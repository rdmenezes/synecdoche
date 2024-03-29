// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Nicolas Alvarez, Peter Kortschack
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
/// Logic related to general (also known as global) preferences:
/// when to compute, how much disk to use, etc.

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#endif

#include "client_state.h"
#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "parse.h"
#include "file_names.h"
#include "cpu_benchmark.h"
#include "client_msgs.h"
#include "miofile.h"
#include "pers_file_xfer.h"

using std::min;
using std::string;

/// Return the maximum allowed disk usage as determined by user preferences.
/// There are three different settings in the prefs;
/// return the least of the three.
double CLIENT_STATE::allowed_disk_usage(double boinc_total) {
    double limit_pct = host_info.d_total * global_prefs.disk_max_used_pct / 100.0;
    double limit_min_free = boinc_total + host_info.d_free - global_prefs.disk_min_free_gb * GIGA;
    double limit_abs = global_prefs.disk_max_used_gb*(GIGA);

    double size = std::min(std::min(limit_abs, limit_pct), limit_min_free);
    if (size < 0) {
        size = 0;
    }
    return size;
}

int CLIENT_STATE::project_disk_usage(const PROJECT* p, double& size) {
    std::string path = get_project_dir(p);
    dir_size(path.c_str(), size);

    for (size_t i=0; i<active_tasks.active_tasks.size(); i++) {
        const ACTIVE_TASK* atp = active_tasks.active_tasks[i];
        double s;

        if (atp->wup->project != p) continue;
        std::string slot_dir = get_slot_dir(atp->slot);
        dir_size(slot_dir.c_str(), s);
        size += s;
    }

    return 0;
}

int CLIENT_STATE::total_disk_usage(double& size) {
    return dir_size(".", size);
}

/// See if we should suspend processing
int CLIENT_STATE::check_suspend_processing() {
    if (are_cpu_benchmarks_running()) {
        return SUSPEND_REASON_BENCHMARKS;
    }

    if (config.start_delay && now < client_start_time + config.start_delay) {
        return SUSPEND_REASON_INITIAL_DELAY;
    }

    switch(run_mode.get_current()) {
    case RUN_MODE_ALWAYS: break;
    case RUN_MODE_NEVER:
        return SUSPEND_REASON_USER_REQ;
    default:
        if (!global_prefs.run_on_batteries
            && host_info.host_is_running_on_batteries()
        ) {
            return SUSPEND_REASON_BATTERIES;
        }

        if (!global_prefs.run_if_user_active && user_active) {
            return SUSPEND_REASON_USER_ACTIVE;
        }
        if (global_prefs.cpu_times.suspended()) {
            return SUSPEND_REASON_TIME_OF_DAY;
        }
    }

    if (global_prefs.suspend_if_no_recent_input) {
        bool idle = host_info.users_idle(
            check_all_logins, global_prefs.suspend_if_no_recent_input
        );
        if (idle) {
            return SUSPEND_REASON_NO_RECENT_INPUT;
        }
    }

    if (global_prefs.cpu_usage_limit != 100) {
        static double last_time=0, debt=0;
        double diff = now - last_time;
        last_time = now;
        if (diff >= POLL_INTERVAL/2. && diff < POLL_INTERVAL*10.) {
            debt += diff*global_prefs.cpu_usage_limit/100;
            if (debt < 0) {
                return SUSPEND_REASON_CPU_USAGE_LIMIT;
            } else {
                debt -= diff;
            }
        }
    }
    return 0;
}

static string reason_string(int reason) {
    string s_reason;
    if (reason & SUSPEND_REASON_BATTERIES) {
        s_reason += " - on batteries";
    }
    if (reason & SUSPEND_REASON_USER_ACTIVE) {
        s_reason += " - user is active";
    }
    if (reason & SUSPEND_REASON_USER_REQ) {
        s_reason += " - user request";
    }
    if (reason & SUSPEND_REASON_TIME_OF_DAY) {
        s_reason += " - time of day";
    }
    if (reason & SUSPEND_REASON_BENCHMARKS) {
        s_reason += " - running CPU benchmarks";
    }
    if (reason & SUSPEND_REASON_DISK_SIZE) {
        s_reason += " - out of disk space - change global prefs";
    }
    if (reason & SUSPEND_REASON_NO_RECENT_INPUT) {
        s_reason += " - no recent user activity";
    }
    if (reason & SUSPEND_REASON_INITIAL_DELAY) {
        s_reason += " - initial delay";
    }
    return s_reason;
}

void print_suspend_tasks_message(int reason) {
    string s_reason = reason_string(reason);
    msg_printf(NULL, MSG_INFO, "Suspending computation%s", s_reason.c_str());
}

int CLIENT_STATE::suspend_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_USAGE_LIMIT) {
        if (log_flags.cpu_sched) {
            msg_printf(NULL, MSG_INFO, "[cpu_sched] Suspending - CPU throttle");
        }
        active_tasks.suspend_all(true);
    } else {
        print_suspend_tasks_message(reason);
        active_tasks.suspend_all(global_prefs.leave_apps_in_memory);
    }
    return 0;
}

int CLIENT_STATE::resume_tasks(int reason) {
    if (reason == SUSPEND_REASON_CPU_USAGE_LIMIT) {
        if (log_flags.cpu_sched) {
            msg_printf(NULL, MSG_INFO, "[cpu_sched] Resuming - CPU throttle");
        }
        active_tasks.unsuspend_all();
    } else {
        msg_printf(NULL, MSG_INFO, "Resuming computation");
        active_tasks.unsuspend_all();
        request_schedule_cpus("Resuming computation");
    }
    return 0;
}

int CLIENT_STATE::check_suspend_network() {
    switch(network_mode.get_current()) {
    case RUN_MODE_ALWAYS: return 0;
    case RUN_MODE_NEVER:
        return SUSPEND_REASON_USER_REQ;
    }
    if (!global_prefs.run_if_user_active && user_active) {
        return SUSPEND_REASON_USER_ACTIVE;
    }
    if (global_prefs.net_times.suspended()) {
        return SUSPEND_REASON_TIME_OF_DAY;
    }
    return 0;
}

int CLIENT_STATE::suspend_network(int reason) {
    string s_reason = reason_string(reason);
    msg_printf(NULL, MSG_INFO, "Suspending network activity%s", s_reason.c_str());
    pers_file_xfers->suspend();
    return 0;
}

int CLIENT_STATE::resume_network() {
    msg_printf(NULL, MSG_INFO, "Resuming network activity");
    return 0;
}

/// call this only after parsing global prefs
PROJECT* CLIENT_STATE::global_prefs_source_project() {
    return lookup_project(global_prefs.source_project);
}

void CLIENT_STATE::show_global_prefs_source(bool found_venue) {
    PROJECT* pp = global_prefs_source_project();
    std::string mod_time_string = time_to_string(global_prefs.mod_time);
    if (pp) {
        msg_printf(NULL, MSG_INFO,
            "General prefs: from %s (last modified %s)",
            pp->get_project_name(), mod_time_string.c_str()
        );
    } else {
        msg_printf(NULL, MSG_INFO,
            "General prefs: from %s (last modified %s)",
            global_prefs.source_project,
            mod_time_string.c_str()
        );
    }
    if (strlen(main_host_venue)) {
        msg_printf(NULL, MSG_INFO, "Computer location: %s", main_host_venue);
        if (found_venue) {
            msg_printf(NULL, MSG_INFO,
                "General prefs: using separate prefs for %s", main_host_venue
            );
        } else {
            msg_printf(NULL, MSG_INFO,
                "General prefs: no separate prefs for %s; using your defaults",
                main_host_venue
            );
        }
    } else {
        msg_printf(NULL, MSG_INFO, "Host location: none");
        msg_printf(NULL, MSG_INFO, "General prefs: using your defaults");
    }
}

/// parse user's project preferences,
/// generating FILE_REF and FILE_INFO objects for each <app_file> element.
int PROJECT::parse_preferences_for_user_files() {
    size_t start = 0, end;
    string app_file_xml;
    string timestamp, open_name, url, filename;
    FILE_INFO* fip;
    FILE_REF fr;

    user_files.clear();
    while (1) {
        start = project_specific_prefs.find("<app_file>", start);
        if (start == string::npos) break;
        end = project_specific_prefs.find("</app_file>", start);
        if (end == string::npos) break;

        //copy just until the beginning of </app_file>
        app_file_xml = project_specific_prefs.substr(start, end-start);

        const char* sz_app_file = app_file_xml.c_str();

        if (!parse_str(sz_app_file, "<timestamp>", timestamp)) break;
        if (!parse_str(sz_app_file, "<open_name>", open_name)) break;
        if (!parse_str(sz_app_file, "<url>", url)) break;

        filename = open_name + "_" + timestamp;
        fip = gstate.lookup_file_info(this, filename.c_str());
        if (!fip) {
            fip = new FILE_INFO;
            fip->project = this;
            fip->urls.push_back(url);
            fip->name = filename;
            fip->is_user_file = true;
            gstate.file_infos.push_back(fip);
        }

        fr.file_info = fip;
        strcpy(fr.open_name, open_name.c_str());
        user_files.push_back(fr);

        //set p to just past the end of </app_file>, to parse the
        //next element (if any)
        start = end + strlen("</app_file>");
    }

    return 0;
}

/// Read global preferences into the global_prefs structure.
/// -# read the override file to get venue in case it's there
/// -# read global_prefs.xml
/// -# read the override file again
///
/// This is called:
/// - on startup
/// - on completion of a scheduler or AMS RPC, if they sent prefs
/// - in response to read_global_prefs_override GUI RPC
void CLIENT_STATE::read_global_prefs() {
    bool found_venue;
    int retval;
    FILE* f;
    string foo;

    retval = read_file_string(GLOBAL_PREFS_OVERRIDE_FILE, foo);
    if (!retval) {
        parse_str(foo.c_str(), "<host_venue>", main_host_venue, sizeof(main_host_venue));
    }

    retval = global_prefs.parse_file(
        GLOBAL_PREFS_FILE_NAME, main_host_venue, found_venue
    );
    if (retval) {
        if (retval == ERR_FOPEN) {
            msg_printf(NULL, MSG_INFO, "No general preferences found - using defaults");
        } else {
            msg_printf(NULL, MSG_INFO, "Couldn't parse preference file - using BOINC defaults");
            boinc_delete_file(GLOBAL_PREFS_FILE_NAME);
            global_prefs.init();
        }
    } else {
        // check that the source project's venue matches main_host_venue.
        // If not, read file again.
        // This is a fix for cases where main_host_venue is out of synch
        //
        PROJECT* p = global_prefs_source_project();
        if (p && strcmp(main_host_venue, p->host_venue)) {
            strcpy(main_host_venue, p->host_venue);
            global_prefs.parse_file(GLOBAL_PREFS_FILE_NAME, main_host_venue, found_venue);
        }
        show_global_prefs_source(found_venue);
    }

    // read the override file
    //
    f = fopen(GLOBAL_PREFS_OVERRIDE_FILE, "r");
    if (f) {
        MIOFILE mf;
        GLOBAL_PREFS_MASK mask;
        mf.init_file(f);
        XML_PARSER xp(&mf);
        global_prefs.parse_override(xp, "", found_venue, mask);
        msg_printf(NULL, MSG_INFO, "Reading preferences override file");
        fclose(f);
    }

    msg_printf(NULL, MSG_INFO, "Preferences limit memory usage when active to %.2fMB",
                (host_info.m_nbytes*global_prefs.ram_max_used_busy_frac) / MEGA);
    msg_printf(NULL, MSG_INFO, "Preferences limit memory usage when idle to %.2fMB",
                (host_info.m_nbytes*global_prefs.ram_max_used_idle_frac) / MEGA);

    double x;
    total_disk_usage(x);
    msg_printf(NULL, MSG_INFO, "Preferences limit disk usage to %.2fGB",
                    allowed_disk_usage(x) / GIGA);

    // max_cpus, bandwidth limits may have changed
    set_ncpus();
    file_xfers->set_bandwidth_limits(true);
    file_xfers->set_bandwidth_limits(false);
    request_schedule_cpus("Prefs update");
    request_work_fetch("Prefs update");
}

int CLIENT_STATE::save_global_prefs(const char* global_prefs_xml, const char* master_url, const char* scheduler_url) {
    FILE* f = boinc_fopen(GLOBAL_PREFS_FILE_NAME, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f,
        "<global_preferences>\n"
    );

    // tag with the project and scheduler URL,
    // but only if not already tagged
    //
    if (!strstr(global_prefs_xml, "<source_project>")) {
        fprintf(f,
            "    <source_project>%s</source_project>\n"
            "    <source_scheduler>%s</source_scheduler>\n",
            master_url,
            scheduler_url
        );
    }
    fprintf(f,
        "%s"
        "</global_preferences>\n",
        global_prefs_xml
    );
    fclose(f);
    return 0;
}

/// amount of RAM usable now
double CLIENT_STATE::available_ram() {
    if (user_active) {
        return host_info.m_nbytes * global_prefs.ram_max_used_busy_frac;
    } else {
        return host_info.m_nbytes * global_prefs.ram_max_used_idle_frac;
    }
}

/// max amount that will ever be usable
double CLIENT_STATE::max_available_ram() {
    return host_info.m_nbytes*std::max(
        global_prefs.ram_max_used_busy_frac, global_prefs.ram_max_used_idle_frac
    );
}
