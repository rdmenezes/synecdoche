// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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

/// \file
/// Graphics-related interaction with running apps.
///
/// NOTE: This code is deprecated.
/// We're keeping it in so that "Show Graphics" will work
/// when pre-V6 apps run on a V6 client.
/// At some point (when all projects have V6 apps,
/// and all old CPDN jobs are finished) we can remove this.

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <sstream>
#endif

#include "diagnostics.h"
#include "client_state.h"
#include "client_msgs.h"

#include "app.h"
#include "util.h"


void ACTIVE_TASK::request_graphics_mode(GRAPHICS_MSG& msg) {
    if (!app_client_shm.shm) {
        return;
    }

    if (msg.mode == MODE_FULLSCREEN) {
        // Remember mode before screensaver
        graphics_mode_before_ss = graphics_mode_acked;
    } else if (graphics_mode_acked != MODE_FULLSCREEN) {
        graphics_mode_before_ss = MODE_HIDE_GRAPHICS;
    }

    if ((msg.mode == MODE_HIDE_GRAPHICS) && (graphics_mode_acked == MODE_FULLSCREEN)) {
        // Restore mode from before screensaver
        msg.mode = graphics_mode_before_ss;
    }

    graphics_msg = msg;       // save graphics_station, desktop, display

    std::ostringstream buf;
    buf << xml_graphics_modes[msg.mode];

    if (!msg.window_station.empty()) {
        buf << "<window_station>" << msg.window_station << "</window_station>";
    }
    if (!msg.desktop.empty()) {
        buf << "<desktop>" << msg.desktop << "</desktop>";
    }
    if (!msg.display.empty()) {
        buf << "<display>" << msg.display << "</display>";
    }

    if (log_flags.scrsave_debug) {
        msg_printf(wup->project, MSG_INFO, "[scrsave_debug] ACTIVE_TASK::request_graphics_mode(): requesting graphics mode %s for %s",
            xml_graphics_modes[msg.mode], result->name);
    }
    graphics_request_queue.msg_queue_send(buf.str().c_str(), app_client_shm.shm->graphics_request);
}


/// handle messages on the "graphics_reply" channel
void ACTIVE_TASK::check_graphics_mode_ack() {
    char buf[MSG_CHANNEL_SIZE];
#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    if (powerpc_emulated_on_i386) {
        graphics_mode_acked = MODE_UNSUPPORTED;
        return;
    }
#endif

    if (!app_client_shm.shm) return;
    if (app_client_shm.shm->graphics_reply.get_msg(buf)) {
        GRAPHICS_MSG gm;
        app_client_shm.decode_graphics_msg(buf, gm);
        if (log_flags.scrsave_debug) {
            msg_printf(wup->project, MSG_INFO,
                "[scrsave_debug] ACTIVE_TASK::check_graphics_mode_ack(): got graphics ack %s for %s, previous mode %s",
                buf, result->name, xml_graphics_modes[graphics_mode_acked]
            );
        }

        if (gm.mode != MODE_REREAD_PREFS) {
            graphics_mode_acked = gm.mode;
        }
    }
}

void ACTIVE_TASK_SET::graphics_poll() {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (!atp->process_exists()) continue;
        atp->graphics_request_queue.msg_queue_poll(
            atp->app_client_shm.shm->graphics_request
        );
        atp->check_graphics_mode_ack();
    }
}

bool ACTIVE_TASK::supports_graphics() const {
#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    if (powerpc_emulated_on_i386)
        return false;
#endif
    if (graphics_mode_acked == MODE_UNSUPPORTED) return false;
    if (scheduler_state != CPU_SCHED_SCHEDULED) return false;
    return true;
}
