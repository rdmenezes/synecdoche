// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "time_stats.h"

#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstring>

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <sstream>

#include "parse.h"
#include "miofile.h"
#include "util.h"
#include "filesys.h"
#include "error_numbers.h"
#include "client_msgs.h"
#include "file_names.h"
#include "client_state.h"
#include "network.h"
#include "log_flags.h"
#include "xml_write.h"

#include "version.h"

#define CONNECTED_STATE_UNINITIALIZED   -1
#define CONNECTED_STATE_NOT_CONNECTED   0
#define CONNECTED_STATE_CONNECTED       1
#define CONNECTED_STATE_UNKNOWN         2

#ifdef _WIN32
#include <sensapi.h>

int get_connected_state() {
    DWORD flags;
    return IsNetworkAlive(&flags)?CONNECTED_STATE_CONNECTED:CONNECTED_STATE_NOT_CONNECTED;
}
#else

// anyone know how to see if this host has physical network connection?
//
int get_connected_state() {
    return CONNECTED_STATE_UNKNOWN;
}
#endif

/// exponential decay constant.
/// The last 10 days have a weight of 1/e;
/// everything before that has a weight of (1-1/e)

const float ALPHA = (SECONDS_PER_DAY*10);
//const float ALPHA = 60;   // for testing

TIME_STATS::TIME_STATS() {
    last_update = 0;
    first = true;
    on_frac = 1;
    connected_frac = 1;
    active_frac = 1;
    cpu_efficiency = 1;
    previous_connected_state = CONNECTED_STATE_UNINITIALIZED;
    inactive_start = 0;
}

/// Update time statistics based on current activities.
/// NOTE: we don't set the state-file dirty flag here,
/// so these get written to disk only when other activities
/// cause this to happen.  Maybe should change this.
///
void TIME_STATS::update(int suspend_reason) {
    double dt, w1, w2;

    bool is_active = !(suspend_reason & ~SUSPEND_REASON_CPU_USAGE_LIMIT);
    if (last_update == 0) {
        // this is the first time this client has executed.
        // Assume that everything is active

        on_frac = 1;
        connected_frac = 1;
        active_frac = 1;
        first = false;
        last_update = gstate.now;
    } else {
        dt = gstate.now - last_update;
        if (dt <= 10) return;
        w1 = 1 - exp(-dt/ALPHA);    // weight for recent period
        w2 = 1 - w1;                // weight for everything before that
                                    // (close to zero if long gap)

        int connected_state = get_connected_state();
        if (gstate.network_suspend_reason) {
            connected_state = CONNECTED_STATE_NOT_CONNECTED;
        }

        if (first) {
            // the client has just started; this is the first call.
            on_frac *= w2;
            first = false;
        } else {
            on_frac = w1 + w2*on_frac;
            if (connected_frac < 0) connected_frac = 0;
            switch (connected_state) {
            case CONNECTED_STATE_NOT_CONNECTED:
                connected_frac *= w2;
                break;
            case CONNECTED_STATE_CONNECTED:
                connected_frac *= w2;
                connected_frac += w1;
                break;
            case CONNECTED_STATE_UNKNOWN:
                connected_frac = -1;
            }
            if (connected_state != previous_connected_state) {
                previous_connected_state = connected_state;
            }
            active_frac *= w2;
            if (is_active) {
                active_frac += w1;
                if (inactive_start) {
                    inactive_start = 0;
                }
            } else if (inactive_start == 0){
                inactive_start = gstate.now;
            }
            //msg_printf(NULL, MSG_INFO, "is_active %d, active_frac %f", is_active, active_frac);
        }
        last_update = gstate.now;
        if (log_flags.time_debug) {
            msg_printf(0, MSG_INFO,
                "[time_debug] dt %f w2 %f on %f; active %f; conn %f",
                dt, w2, on_frac, active_frac, connected_frac
            );
        }
    }
}

void TIME_STATS::update_cpu_efficiency(double cpu_wall_time, double cpu_time) {
    double old_cpu_efficiency = cpu_efficiency;
    if (cpu_wall_time < .01) return;
    double w = exp(-cpu_wall_time/SECONDS_PER_DAY);
    double e = cpu_time/cpu_wall_time;
    if (e<0) {
        return;
    }
    cpu_efficiency = w*cpu_efficiency + (1-w)*e;
    if (log_flags.cpu_sched_debug){
        msg_printf(0, MSG_INFO,
            "[cpu_sched_debug] CPU efficiency old %f new %f wall %f CPU %f w %f e %f",
            old_cpu_efficiency, cpu_efficiency, cpu_wall_time,
            cpu_time, w, e
        );
    }
}

/// Write XML based time statistics
///
void TIME_STATS::write(std::ostream& out, bool to_server) const {
    out << "<time_stats>\n"
        << XmlTag<double>("on_frac",        on_frac)
        << XmlTag<double>("connected_frac", connected_frac)
        << XmlTag<double>("active_frac",    active_frac)
        << XmlTag<double>("cpu_efficiency", cpu_efficiency)
    ;
    if (!to_server) {
        out << XmlTag<double>("last_update", last_update);
    }
    out << "</time_stats>\n";
}

/// Parse XML based time statistics, usually from client_state.xml
int TIME_STATS::parse(MIOFILE& in) {
    char buf[256];
    double x;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</time_stats>")) return 0;
        else if (parse_double(buf, "<last_update>", x)) {
            if ((x < 0.0) || (x > gstate.now)) {
                msg_printf(0, MSG_INTERNAL_ERROR, "bad value %f of time stats last update; ignoring", x);
            } else {
                last_update = x;
            }
            continue;
        } else if (parse_double(buf, "<on_frac>", x)) {
            if ((x <= 0.0) || (x > 1.0)) {
                msg_printf(0, MSG_INTERNAL_ERROR, "bad value %f of time stats on_frac; ignoring", x);
            } else {
                on_frac = x;
            }
            continue;
        } else if (parse_double(buf, "<connected_frac>", x)) {
            if (x > 1.0) {
                msg_printf(0, MSG_INTERNAL_ERROR, "bad value %f of time stats connected_frac; ignoring", x);
            } else {
                connected_frac = x;
            }
            continue;
        } else if (parse_double(buf, "<active_frac>", x)) {
            if ((x <= 0.0) || (x > 1.0)) {
                msg_printf(0, MSG_INTERNAL_ERROR, "bad value %f of time stats active_frac; ignoring", x);
            } else {
                active_frac = x;
            }
            continue;
        } else if (parse_double(buf, "<cpu_efficiency>", x)) {
            if ((x < 0.0) || (x > 1.0)) {
                msg_printf(0, MSG_INTERNAL_ERROR, "bad value %f of time stats cpu_efficiency; ignoring", x);
            } else {
                active_frac = x;
            }
            continue;
        } else {
            handle_unparsed_xml_warning("TIME_STATS::parse", buf);
        }
    }
    return ERR_XML_PARSE;
}
