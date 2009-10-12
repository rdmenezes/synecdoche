// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
// Copyright (C) 2009 University of California
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
#include "proc_control.h"
#else
#include "config.h"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <sstream>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <unistd.h>
#include <errno.h>
#endif

#include "client_state.h"

#include "version.h"

#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "filesys.h"
#include "xml_write.h"

#include "file_names.h"
#include "hostinfo.h"
#include "hostinfo_network.h"
#include "network.h"
#include "http_curl.h"
#include "client_msgs.h"
#include "shmem.h"
#include "sandbox.h"
#include "scheduler_op.h"
#include "pers_file_xfer.h"

CLIENT_STATE gstate;

CLIENT_STATE::CLIENT_STATE():
    lookup_website_op(&gui_http)
#ifdef ENABLE_UPDATE_CHECK
    ,
    get_current_version_op(&gui_http)
#endif
{
    http_ops = new HTTP_OP_SET();
    file_xfers = new FILE_XFER_SET(http_ops);
    pers_file_xfers = new PERS_FILE_XFER_SET(file_xfers);
    scheduler_op = new SCHEDULER_OP(http_ops);
    client_state_dirty = false;
    exit_when_idle = false;
    exit_before_start = false;
    exit_after_finish = false;
    check_all_logins = false;
    allow_remote_gui_rpc = false;
    cmdline_gui_rpc_port = 0;
    run_cpu_benchmarks = false;
    skip_cpu_benchmarks = false;
    file_xfer_giveup_period = PERS_GIVEUP;
    contacted_sched_server = false;
    tasks_suspended = false;
    network_suspended = false;
    suspend_reason = 0;
    network_suspend_reason = 0;
    core_client_version.major = SYNEC_MAJOR_VERSION;
    core_client_version.minor = SYNEC_MINOR_VERSION;
    core_client_version.release = SYNEC_RELEASE;
    core_client_version.prerelease = SYNEC_PRERELEASE;
    boinc_compat_version.major = BOINC_MAJOR_VERSION;
    boinc_compat_version.minor = BOINC_MINOR_VERSION;
    boinc_compat_version.release = BOINC_RELEASE;
    boinc_compat_version.prerelease = 0;
    exit_after_app_start_secs = 0;
    app_started = 0;
    exit_before_upload = false;
    proxy_info.clear();
    strcpy(main_host_venue, "");
    run_mode.set(RUN_MODE_AUTO, 0);
    network_mode.set(RUN_MODE_AUTO, 0);
    started_by_screensaver = false;
    requested_exit = false;
    master_fetch_period = MASTER_FETCH_PERIOD;
    retry_cap = RETRY_CAP;
    master_fetch_retry_cap = MASTER_FETCH_RETRY_CAP;
    master_fetch_interval = MASTER_FETCH_INTERVAL;
    sched_retry_delay_min = SCHED_RETRY_DELAY_MIN;
    sched_retry_delay_max = SCHED_RETRY_DELAY_MAX;
    pers_retry_delay_min = PERS_RETRY_DELAY_MIN;
    pers_retry_delay_max = PERS_RETRY_DELAY_MAX;
    pers_giveup = PERS_GIVEUP;
    executing_as_daemon = false;
    redirect_io = false;
    disable_graphics = false;
    work_fetch_no_new_work = false;
    cant_write_state_file = false;

    debt_interval_start = 0;
    total_wall_cpu_time_this_debt_interval = 0;
    retry_shmem_time = 0;
    must_schedule_cpus = true;
    must_enforce_cpu_schedule = true;
    no_gui_rpc = false;
#ifdef ENABLE_UPDATE_CHECK
    new_version_check_time = 0;
#endif
    detach_console = false;
#ifdef SANDBOX
    g_use_sandbox = true; // User can override with -insecure command-line arg
#endif
    launched_by_manager = false;
    initialized = false;
    last_wakeup_time = dtime();
}

void CLIENT_STATE::show_host_info() {
    char buf[256], buf2[256];
    msg_printf(NULL, MSG_INFO,
        "Processor: %d %s %s",
        host_info.p_ncpus, host_info.p_vendor, host_info.p_model
    );
    msg_printf(NULL, MSG_INFO,
        "Processor features: %s", host_info.p_features
    );
    msg_printf(NULL, MSG_INFO,
        "OS: %s: %s", host_info.os_name, host_info.os_version
    );

    nbytes_to_string(host_info.m_nbytes, 0, buf, sizeof(buf));
    nbytes_to_string(host_info.m_swap, 0, buf2, sizeof(buf2));
    msg_printf(NULL, MSG_INFO,
        "Memory: %s physical, %s virtual",
        buf, buf2
    );

    nbytes_to_string(host_info.d_total, 0, buf, sizeof(buf));
    nbytes_to_string(host_info.d_free, 0, buf2, sizeof(buf2));
    msg_printf(NULL, MSG_INFO, "Disk: %s total, %s free", buf, buf2);
    int tz = host_info.timezone/3600;
    msg_printf(0, MSG_INFO, "Local time is UTC %s%d hours",
        tz<0?"":"+", tz
    );
}

void CLIENT_STATE::show_proxy_info() {
    if (proxy_info.use_http_proxy) {
        msg_printf(NULL, MSG_INFO, "Using HTTP proxy %s:%d",
            proxy_info.http_server_name, proxy_info.http_server_port
        );
    } else if (proxy_info.use_socks_proxy) {
        msg_printf(NULL, MSG_INFO, "Using SOCKS proxy %s:%d",
            proxy_info.socks_server_name, proxy_info.socks_server_port
        );
    } else {
        msg_printf(NULL, MSG_INFO, "Not using a proxy");
    }
}

int CLIENT_STATE::init() {
    int retval;
    unsigned int i;
    char buf[256];
    PROJECT* p;

    srand((unsigned int)time(0));
    now = dtime();
    client_start_time = now;
    scheduler_op->url_random = drand();

    detect_platforms();

    std::ostringstream start_msg;
    start_msg << "Starting Synecdoche client version " << core_client_version.major;
    start_msg << '.' << core_client_version.minor << '.' << core_client_version.release;
    if (SYNEC_SVN_VERSION) {
        start_msg << " r" << SYNEC_SVN_VERSION;
    }
    start_msg << " for " << get_primary_platform();
#ifdef _DEBUG
    start_msg << " (DEBUG)";
#endif // _DEBUG

    msg_printf(NULL, MSG_INFO, "%s", start_msg.str().c_str());

    if (core_client_version.prerelease) {
        msg_printf(NULL, MSG_USER_ERROR,
            "This a development version of Synecdoche and may not function properly"
        );
    }

    log_flags.show();

    msg_printf(NULL, MSG_INFO, "Libraries: %s", curl_version());

    if (executing_as_daemon) {
        msg_printf(NULL, MSG_INFO, "Running as a daemon");
    }

    std::string data_dir = relative_to_absolute("");
    msg_printf(NULL, MSG_INFO, "Data directory: %s", data_dir.c_str());

#ifdef _WIN32
    DWORD  buf_size = sizeof(buf);
    LPTSTR pbuf = buf;

    GetUserName(pbuf, &buf_size);
    msg_printf(NULL, MSG_INFO, "Running under account %s", pbuf);
#endif

    parse_account_files();
    parse_statistics_files();

    // check for app_info.xml file in project dirs.
    // If find, read app info from there, set project.anonymous_platform
    // NOTE: this is being done before CPU speed has been read,
 	// so we'll need to patch up avp->flops later;
    check_anonymous();

    // Parse the client state file,
    // ignoring any <project> tags (and associated stuff)
    // for projects with no account file
    host_info.clear_host_info();
    cpu_benchmarks_set_defaults();  // for first time, make sure p_fpops nonzero
    parse_state_file();
    parse_account_files_venue();

    host_info.get_host_info();
    set_ncpus();
    show_host_info();
    show_proxy_info();

    // fill in avp->flops for anonymous project
    for (i = 0; i < app_versions.size(); ++i) {
        APP_VERSION* avp = app_versions[i];
        if (!avp->flops) {
            avp->flops = host_info.p_fpops;
        }
    }

    check_clock_reset();

    // Check to see if we can write the state file.
    retval = write_state_file();
    if (retval) {
        msg_printf(NULL, MSG_USER_ERROR, "Couldn't write state file");
        msg_printf(NULL, MSG_USER_ERROR,
            "Make sure directory permissions are set correctly"
        );
        cant_write_state_file = true;
    }

    // scan user prefs; create file records
    parse_preferences_for_user_files();

    if (log_flags.state_debug) {
        print_summary();
    }

    // Check if version or platform has changed.
    // Either of these is evidence that we're running a different
    // client than previously.
    bool new_client = false;
    if ((core_client_version.major != old_major_version)
        || (core_client_version.minor != old_minor_version)
        || (core_client_version.release != old_release)
    ) {
        msg_printf(NULL, MSG_INFO,
            "Version change (%d.%d.%d -> %d.%d.%d)",
            old_major_version, old_minor_version, old_release,
            core_client_version.major,
            core_client_version.minor,
            core_client_version.release
        );
        new_client = true;
    }
    if ((!statefile_platform_name.empty()) && (statefile_platform_name != get_primary_platform())) {
        msg_printf(0, MSG_INFO, "Platform changed from %s to %s",
            statefile_platform_name.c_str(), get_primary_platform().c_str());
        new_client = true;
    }

    // If new version of client run CPU benchmark and contact
    // reference site (or some project) to trigger firewall alert.
    if (new_client) {
        run_cpu_benchmarks = true;
        if (config.dont_contact_ref_site) {
            if (!projects.empty()) {
                projects[0]->master_url_fetch_pending = true;
            }
        } else {
            net_status.need_to_contact_reference_site = true;
        }
    }

    // show host IDs and venues on various projects
    for (i=0; i<projects.size(); i++) {
        p = projects[i];
        if (p->hostid) {
            sprintf(buf, "%d", p->hostid);
        } else {
            strcpy(buf, "not assigned yet");
        }
        msg_printf(p, MSG_INFO,
            "URL: %s; Computer ID: %s; location: %s; project prefs: %s",
            p->get_master_url().c_str(),
            buf, strlen(p->host_venue)?p->host_venue:"(none)",
            p->using_venue_specific_prefs?p->host_venue:"default"
        );
        if (p->ended) {
            msg_printf(p, MSG_INFO, "Project has ended - OK to detach");
        }
    }

    read_global_prefs();

    // do CPU scheduler and work fetch
    request_schedule_cpus("Startup");
    request_work_fetch("Startup");
    zero_debts_if_requested();
    debt_interval_start = now;

    // set up the project and slot directories
    delete_old_slot_dirs();
    retval = make_project_dirs();
    if (retval) return retval;

    active_tasks.init();
    active_tasks.report_overdue();
    active_tasks.handle_upload_files();

    // Just to be on the safe side; something may have been modified
    set_client_state_dirty("init");

    // initialize GUI RPC data structures before we start accepting
    // GUI RPC's.
    acct_mgr_info.init();
    project_init.init();

    if (!no_gui_rpc) {
        // When we're running at boot time,
        // it may be a few seconds before we can socket/bind/listen.
        // So retry a few times.
        for (i=0; i<30; i++) {
            bool last_time = (i==29);
            retval = gui_rpcs.init(last_time);
            if (!retval) break;
            boinc_sleep(1.0);
        }
        if (retval) return retval;
    }

#ifdef SANDBOX
    get_project_gid();
#endif // SANDBOX

#ifdef _WIN32
    get_sandbox_account_service_token();
    if (sandbox_account_service_token) {
        g_use_sandbox = true;
    }
#endif // _WIN32

    check_file_existence();

    http_ops->cleanup_temp_files();

    initialized = true;
    return 0;
}

static void double_to_timeval(double x, timeval& t) {
    t.tv_sec = (int)x;
    t.tv_usec = (int)(1000000*(x - (int)x));
}


/// Spend \a sec seconds either doing I/O (if possible) or sleeping.
void CLIENT_STATE::do_io_or_sleep(double sec) {
    int n;
    struct timeval tv;
    now = dtime();
    double end_time = now + sec;
    int loops = 0;
    FDSET_GROUP all_fds;

    while (1) {
        all_fds.zero();
        http_ops->get_fdset(all_fds);
        gui_rpcs.get_fdset(all_fds);
        double_to_timeval(sec, tv);
        n = select(all_fds.max_fd + 1, &all_fds.read_fds,
                   &all_fds.write_fds, &all_fds.exc_fds, &tv);

        // Check if there was an error:
        if (n == -1) {
#ifdef _WIN32
            int err_code = WSAGetLastError();
            int err_code_eintr = WSAEINTR;
#else
            int err_code = errno;
            int err_code_eintr = EINTR;
#endif // _WIN32
            if (err_code == err_code_eintr) {
                // select() was interrupted by a signal - that can be ignored.
                continue;
            } else {
                msg_printf(0, MSG_INTERNAL_ERROR, "select() failed with an unexpected error: %d - quitting.", err_code);
                gstate.requested_exit = true;
                break;
            }
        }

        // Note: curl apparently likes to have curl_multi_perform()
        // (called from net_xfers->got_select())
        // called pretty often, even if no descriptors are enabled.
        // So do the "if (n==0) break" AFTER the http_ops->got_select().
        http_ops->got_select(all_fds, sec);

        if (n == 0) {
            break;
        }

        gui_rpcs.got_select(all_fds);

        // Limit number of times thru this loop.
        // Can get stuck in while loop, if network isn't available,
        // DNS lookups tend to eat CPU cycles.
        //
        if (loops++ > 99) {
            boinc_sleep(.01);
            break;
        }

        now = dtime();
        if (now > end_time) break;
        sec = end_time - now;
    }
}

#define POLL_ACTION(name, func) \
    do { if (func()) { \
            ++actions; \
            if (log_flags.poll_debug) { \
                msg_printf(0, MSG_INFO, "[poll_debug] CLIENT_STATE::poll_slow_events(): " #name "\n"); \
            } \
        } } while(0)

/// Poll the client's finite-state machines
/// possibly triggering state transitions.
/// This function never blocks.
///
/// \return True if something happened (in which case this function should be
/// called again immediately), false otherwise.
bool CLIENT_STATE::poll_slow_events() {
    int actions = 0, retval;
    static int last_suspend_reason=0;
    static bool tasks_restarted = false;
    static bool first=true;
    double old_now = now;
#ifdef __APPLE__
    double idletime;
#endif

    now = dtime();

    if (cant_write_state_file) {
        return false;
    }

    if (now - old_now > POLL_INTERVAL*10) {
        if (log_flags.network_status_debug) {
            msg_printf(0, MSG_INFO,
                "[network_status_debug] woke up after %f seconds",
                now - old_now
            );
        }
        last_wakeup_time = now;
    }

    if (should_run_cpu_benchmarks() && !are_cpu_benchmarks_running()) {
        run_cpu_benchmarks = false;
        start_cpu_benchmarks();
    }

    bool old_user_active = user_active;
    user_active = !host_info.users_idle(
        check_all_logins, global_prefs.idle_time_to_run
#ifdef __APPLE__
         , &idletime
#endif
    );

    if (user_active != old_user_active) {
        request_schedule_cpus("Idle state change");
    }
#ifdef __APPLE__
    // Mac screensaver launches client if not already running.
    // OS X quits screensaver when energy saver puts display to sleep,
    // but we want to keep crunching.
    // Also, user can start Mac screensaver by putting cursor in "hot corner"
    // so idletime may be very small initially.
    // If screensaver started client, this code tells client
    // to exit when user becomes active, accounting for all these factors.
    //
    if (started_by_screensaver && (idletime < 30) && (getppid() == 1)) {
        // pid is 1 if parent has exited
        requested_exit = true;
    }

    // Exit if we were launched by Manager and it crashed.
    //
    if (launched_by_manager && (getppid() == 1)) {
        gstate.requested_exit = true;
    }
#endif

    suspend_reason = check_suspend_processing();

    // suspend or resume activities (but only if already did startup)
    //
    if (tasks_restarted) {
        if (suspend_reason) {
            if (!tasks_suspended) {
                suspend_tasks(suspend_reason);
            }
            last_suspend_reason = suspend_reason;
        } else {
            if (tasks_suspended) {
                resume_tasks(last_suspend_reason);
            }
        }
    } else if (first) {
        // if suspended, show message the first time around
        //
        first = false;
        if (suspend_reason) {
            print_suspend_tasks_message(suspend_reason);
        }
    }
    tasks_suspended = (suspend_reason != 0);

    if (suspend_reason & SUSPEND_REASON_BENCHMARKS) {
        cpu_benchmarks_poll();
    }

    network_suspend_reason = check_suspend_network();

    if (network_suspend_reason) {
        if (!network_suspended) {
            suspend_network(network_suspend_reason);
            network_suspended = true;
        }
    } else {
        if (network_suspended) {
            resume_network();
            network_suspended = false;
        }
    }

    // NOTE:
    // The order of calls in the following lists generally doesn't matter,
    // except for the following:
    // must have:
    //  active_tasks_poll
    //  handle_finished_apps
    //  possibly_schedule_cpus
    //  enforce_schedule
    // in that order (active_tasks_poll() sets must_schedule_cpus,
    // and handle_finished_apps() must be done before possibly_schedule_cpus()

    check_project_timeout();
    POLL_ACTION(active_tasks           , active_tasks.poll      );
    POLL_ACTION(garbage_collect        , garbage_collect        );
    POLL_ACTION(update_results         , update_results         );
    POLL_ACTION(gui_http               , gui_http.poll          );
    POLL_ACTION(gui_rpc_http           , gui_rpcs.poll          );
    if (!network_suspended) {
        net_status.poll();
        POLL_ACTION(acct_mgr               , acct_mgr_info.poll     );
        POLL_ACTION(file_xfers             , file_xfers->poll       );
        POLL_ACTION(pers_file_xfers        , pers_file_xfers->poll  );
        POLL_ACTION(handle_pers_file_xfers , handle_pers_file_xfers );
    }
    POLL_ACTION(handle_finished_apps   , handle_finished_apps   );
    if (!tasks_suspended) {
        POLL_ACTION(possibly_schedule_cpus, possibly_schedule_cpus          );
        POLL_ACTION(enforce_schedule    , enforce_schedule  );
        tasks_restarted = true;
    }
    if (!tasks_suspended && !network_suspended) {
        POLL_ACTION(compute_work_requests, compute_work_requests          );
    }
    if (!network_suspended) {
        POLL_ACTION(scheduler_rpc          , scheduler_rpc_poll     );
    }
    retval = write_state_file_if_needed();
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't write state file: %s; giving up", boincerror(retval)
        );
        exit(EXIT_STATEFILE_WRITE);
    }
    if (log_flags.poll_debug) {
        msg_printf(0, MSG_INFO,
            "[poll_debug] CLIENT_STATE::do_something(): End poll: %d tasks active\n", actions
        );
    }
    if (actions > 0) {
        return true;
    } else {
        time_stats.update(suspend_reason);

        // on some systems, gethostbyname() only starts working
        // a few minutes after system boot.
        // If it didn't work before, try it again.
        //
        if (!strlen(host_info.domain_name)) {
            host_info.get_local_network_info();
        }
        return false;
    }
}

/// See if the project specified by \a master_url already exists
/// in the client state record. Ignore any trailing "/" characters
PROJECT* CLIENT_STATE::lookup_project(const std::string& master_url) {
    std::string mu1 = master_url;
    if (ends_with(master_url, "/")) {
        mu1.erase(mu1.end() - 1);
    }
    for (std::vector<PROJECT*>::const_iterator p = projects.begin(); p != projects.end(); ++p) {
        std::string mu2 = (*p)->get_master_url();
        if (ends_with(mu2, "/")) {
            mu2.erase(mu2.end() - 1);
        }
        if (mu1 == mu2) {
            return *p;
        }
    }
    return 0;
}

APP* CLIENT_STATE::lookup_app(const PROJECT* project, const char* name) {
    for (unsigned int i=0; i<apps.size(); i++) {
        APP* app = apps[i];
        if (app->project == project && !strcmp(name, app->name)) return app;
    }
    return 0;
}

RESULT* CLIENT_STATE::lookup_result(const PROJECT* project, const char* name) {
    for (unsigned int i=0; i<results.size(); i++) {
        RESULT* rp = results[i];
        if (rp->project == project && !strcmp(name, rp->name)) return rp;
    }
    return 0;
}

WORKUNIT* CLIENT_STATE::lookup_workunit(const PROJECT* project, const char* name) {
    for (unsigned int i=0; i<workunits.size(); i++) {
        WORKUNIT* wup = workunits[i];
        if (wup->project == project && !strcmp(name, wup->name)) return wup;
    }
    return 0;
}

APP_VERSION* CLIENT_STATE::lookup_app_version(const APP* app, const char* platform,
                                              int version_num, const char* plan_class) {
    for (size_t i = 0; i < app_versions.size(); ++i) {
        APP_VERSION* avp = app_versions[i];
        if (avp->app != app) continue;
        if (version_num != avp->version_num) continue;
        if (strcmp(avp->platform, platform)) continue;
        if (strcmp(avp->plan_class, plan_class)) continue;
        return avp;
    }
    return 0;
}

FILE_INFO* CLIENT_STATE::lookup_file_info(const PROJECT* p, const std::string& name) {
    for (unsigned int i=0; i<file_infos.size(); i++) {
        FILE_INFO* fip = file_infos[i];
        if ((fip->project == p) && (fip->name == name)) {
            return fip;
        }
    }
    return 0;
}

// functions to create links between state objects
// (which, in their XML form, reference one another by name)
// Return nonzero if already in client state.
int CLIENT_STATE::link_app(PROJECT* p, APP* app) {
    if (lookup_app(p, app->name)) return ERR_NOT_UNIQUE;
    app->project = p;
    return 0;
}

int CLIENT_STATE::link_file_info(PROJECT* p, FILE_INFO* fip) {
    if (lookup_file_info(p, fip->name)) return ERR_NOT_UNIQUE;
    fip->project = p;
    return 0;
}

int CLIENT_STATE::link_app_version(PROJECT* p, APP_VERSION* avp) {
    APP* app;
    FILE_INFO* fip;
    unsigned int i;

    avp->project = p;
    app = lookup_app(p, avp->app_name);
    if (!app) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: bad application name %s",
            avp->app_name
        );
        return ERR_NOT_FOUND;
    }
    avp->app = app;

    if (lookup_app_version(app, avp->platform, avp->version_num, avp->plan_class)) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: duplicate app version: %s %s %d %s",
            avp->app_name, avp->platform, avp->version_num, avp->plan_class
        );
        return ERR_NOT_UNIQUE;
    }

    avp->graphics_exec_path[0] = 0;

    for (i=0; i<avp->app_files.size(); i++) {
        FILE_REF& file_ref = avp->app_files[i];
        fip = lookup_file_info(p, file_ref.file_name);
        if (!fip) {
            msg_printf(p, MSG_INTERNAL_ERROR,
                "State file error: missing application file %s",
                file_ref.file_name
            );
            return ERR_NOT_FOUND;
        }

        if (!strcmp(file_ref.open_name, GRAPHICS_APP_FILENAME)) {
            std::string path = relative_to_absolute(get_pathname(fip));
            strlcpy(avp->graphics_exec_path, path.c_str(), sizeof(avp->graphics_exec_path));
        }

        // any file associated with an app version must be signed
        fip->signature_required = true;
        file_ref.file_info = fip;
    }
    return 0;
}

int CLIENT_STATE::link_file_ref(PROJECT* p, FILE_REF* file_refp) {
    FILE_INFO* fip;

    fip = lookup_file_info(p, file_refp->file_name);
    if (!fip) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: missing file %s",
            file_refp->file_name
        );
        return ERR_NOT_FOUND;
    }
    file_refp->file_info = fip;
    return 0;
}

int CLIENT_STATE::link_workunit(PROJECT* p, WORKUNIT* wup) {
    APP* app;
    unsigned int i;
    int retval;

    app = lookup_app(p, wup->app_name);
    if (!app) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: missing application %s",
            wup->app_name
        );
        return ERR_NOT_FOUND;
    }
    wup->project = p;
    wup->app = app;
    for (i=0; i<wup->input_files.size(); i++) {
        retval = link_file_ref(p, &wup->input_files[i]);
        if (retval) {
            msg_printf(p, MSG_INTERNAL_ERROR,
                "State file error: missing input file %s\n",
                wup->input_files[i].file_name
            );
            return retval;
        }
    }
    return 0;
}

int CLIENT_STATE::link_result(PROJECT* p, RESULT* rp) {
    WORKUNIT* wup;
    unsigned int i;
    int retval;

    wup = lookup_workunit(p, rp->wu_name);
    if (!wup) {
        msg_printf(p, MSG_INTERNAL_ERROR,
            "State file error: missing task %s\n", rp->wu_name
        );
        return ERR_NOT_FOUND;
    }
    rp->project = p;
    rp->wup = wup;
    rp->app = wup->app;
    for (i=0; i<rp->output_files.size(); i++) {
        retval = link_file_ref(p, &rp->output_files[i]);
        if (retval) return retval;
    }
    return 0;
}

/// Print debugging information about how many projects/files/etc
/// are currently in the client state record.
void CLIENT_STATE::print_summary() const {
    unsigned int i;
    double t;

    msg_printf(0, MSG_INFO, "[state_debug] Client state summary:\n");
    msg_printf(0, MSG_INFO, "%lu projects:", projects.size());
    for (i=0; i<projects.size(); i++) {
        t = projects[i]->min_rpc_time;
        if (t) {
            msg_printf(0, MSG_INFO, "    %s min RPC %f.0 seconds from now", projects[i]->get_master_url().c_str(), t-now);
        } else {
            msg_printf(0, MSG_INFO, "    %s", projects[i]->get_master_url().c_str());
        }
    }
    msg_printf(0, MSG_INFO, "%lu file_infos:", file_infos.size());
    for (i=0; i<file_infos.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s status:%d %s", file_infos[i]->name.c_str(), file_infos[i]->status, file_infos[i]->pers_file_xfer?"active":"inactive");
    }
    msg_printf(0, MSG_INFO, "%lu app_versions", app_versions.size());
    for (i=0; i<app_versions.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s %d", app_versions[i]->app_name, app_versions[i]->version_num);
    }
    msg_printf(0, MSG_INFO, "%lu workunits", workunits.size());
    for (i=0; i<workunits.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s", workunits[i]->name);
    }
    msg_printf(0, MSG_INFO, "%lu results", results.size());
    for (i=0; i<results.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s state:%d", results[i]->name, results[i]->state());
    }
    msg_printf(0, MSG_INFO, "%lu persistent file xfers", pers_file_xfers->pers_file_xfers.size());
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s http op state: %d", pers_file_xfers->pers_file_xfers[i]->fip->name.c_str(), (pers_file_xfers->pers_file_xfers[i]->fxp?pers_file_xfers->pers_file_xfers[i]->fxp->http_op_state:-1));
    }
    msg_printf(0, MSG_INFO, "%lu active tasks", active_tasks.active_tasks.size());
    for (i=0; i<active_tasks.active_tasks.size(); i++) {
        msg_printf(0, MSG_INFO, "    %s", active_tasks.active_tasks[i]->result->name);
    }
}

int CLIENT_STATE::nresults_for_project(const PROJECT* project) const {
    int n=0;
    for (unsigned int i=0; i<results.size(); i++) {
        if (results[i]->project == project) n++;
    }
    return n;
}

/// Abort all jobs that are not started yet but already missed their deadline.
///
/// \return True if at least one result was aborted.
bool CLIENT_STATE::abort_unstarted_late_jobs() {
    bool action = false;
    for (RESULT_PVEC::iterator p = results.begin(); p != results.end(); ++p) {
        if (((*p)->not_started()) && ((*p)->report_deadline <= now)) {
            // This task is not running yet but already has missed its deadline. Abort it:
            (*p)->abort_inactive(ERR_UNSTARTED_LATE);

            if (log_flags.task) {
                msg_printf((*p)->get_project(), MSG_INFO,
                    "Result %s was aborted because it was not started yet and already missed its deadline.",
                    (*p)->get_name().c_str());
            }

            action = true;
        }
    }
    return action;
}

bool CLIENT_STATE::garbage_collect() {
    static double last_time=0;
    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    // Shortcut evaluation prevents the second line from executing when the first line
    // already returned true. This is the desired behaviour and much less verbose than checking
    // the return value of each function with an extra if statement.
    bool action =  abort_unstarted_late_jobs()
                || garbage_collect_always();

    if (action) {
        return true;
    }

    // Detach projects that are marked for detach when done
    // and are in fact done (have no results).
    // This is done here (not in garbage_collect_always())
    // because detach_project() calls garbage_collect_always(),
    // and we need to avoid infinite recursion
    for (unsigned i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        if (p->detach_when_done && !nresults_for_project(p)) {
            detach_project(p);
            action = true;
        }
    }
    return action;
}

/// Delete unneeded records and files.
///
/// \return True if some elements were removed, false otherwise.
bool CLIENT_STATE::garbage_collect_always() {
    unsigned int i, j;
    int failnum;
    bool action = false, found;
    std::string error_msgs;

    // zero references counts on WUs, FILE_INFOs and APP_VERSIONs
    for (i=0; i<workunits.size(); i++) {
        workunits[i]->ref_cnt = 0;
    }
    for (i=0; i<file_infos.size(); i++) {
        file_infos[i]->ref_cnt = 0;
    }
    for (i=0; i<app_versions.size(); i++) {
        app_versions[i]->ref_cnt = 0;
    }

    // reference-count user and project files
    for (i=0; i<projects.size(); i++) {
        PROJECT* project = projects[i];
        for (j=0; j<project->user_files.size(); j++) {
            project->user_files[j].file_info->ref_cnt++;
        }
        for (j=0; j<project->project_files.size(); j++) {
            project->project_files[j].file_info->ref_cnt++;
        }
    }

    // Scan through RESULTs.
    // delete RESULTs that have been reported and acked.
    // Check for results whose WUs had download failures
    // Check for results that had upload failures
    // Reference-count output files
    // Reference-count WUs
    std::vector<RESULT*>::iterator result_iter = results.begin();
    while (result_iter != results.end()) {
        RESULT* rp = *result_iter;
        if (rp->got_server_ack) {
            // see if - for some reason - there's an active task
            // for this result.  don't want to create dangling ptr.
            ACTIVE_TASK* atp = active_tasks.lookup_result(rp);
            if (atp) {
                msg_printf(rp->project, MSG_INTERNAL_ERROR,
                    "garbage_collect(); still have active task for acked result %s; state %d",
                    rp->name, atp->task_state());
                atp->abort_task(EXIT_ABORTED_BY_CLIENT, "Got ack for job that's till active");
            } else {
                if (log_flags.state_debug) {
                    msg_printf(0, MSG_INFO, "[state_debug] garbage_collect: deleting result %s\n", rp->name);
                }
                delete rp;
                result_iter = results.erase(result_iter);
                action = true;
                continue;
            }
        }
        // See if the files for this result's workunit had
        // any errors (download failure, MD5, RSA, etc)
        // and we don't already have an error for this result
        if (!rp->ready_to_report) {
            WORKUNIT* wup = rp->wup;
            if (wup->had_download_failure(failnum)) {
                wup->get_file_errors(error_msgs);
                report_result_error(*rp, "WU download error: %s", error_msgs.c_str());
            } else if (rp->avp && rp->avp->had_download_failure(failnum)) {
                rp->avp->get_file_errors(error_msgs);
                report_result_error(*rp, "app_version download error: %s", error_msgs.c_str());
            }
        }
        bool found_error = false;
        std::string error_str;
        for (i=0; i<rp->output_files.size(); i++) {
            // If one of the output files had an upload failure,
            // mark the result as done and report the error.
            if (!rp->ready_to_report) {
                FILE_INFO* fip = rp->output_files[i].file_info;
                if (fip->had_failure(failnum)) {
                    found_error = true;
                    error_str += fip->failure_message();
                }
            }
            rp->output_files[i].file_info->ref_cnt++;
        }
        if (found_error) {
            // Check for process still running; this can happen
            // e.g. if an intermediate upload fails.
            ACTIVE_TASK* atp = active_tasks.lookup_result(rp);
            if (atp) {
                int task_state = atp->task_state();
                if ((task_state == PROCESS_EXECUTING) || (task_state == PROCESS_SUSPENDED)) {
                    atp->abort_task(EXIT_ABORTED_BY_CLIENT, "Got ack for job that's till active");
                }
            }
            report_result_error(*rp, "%s", error_str.c_str());
        }
        rp->avp->ref_cnt++;
        rp->wup->ref_cnt++;
        result_iter++;
    }

    // delete WORKUNITs not referenced by any in-progress result;
    // reference-count files and APP_VERSIONs referred to by other WUs
    std::vector<WORKUNIT*>::iterator wu_iter = workunits.begin();
    while (wu_iter != workunits.end()) {
        WORKUNIT* wup = *wu_iter;
        if (wup->ref_cnt == 0) {
            if (log_flags.state_debug) {
                msg_printf(0, MSG_INFO,
                    "[state_debug] CLIENT_STATE::garbage_collect(): deleting workunit %s\n",
                    wup->name);
            }
            delete wup;
            wu_iter = workunits.erase(wu_iter);
            action = true;
        } else {
            for (i=0; i<wup->input_files.size(); i++) {
                wup->input_files[i].file_info->ref_cnt++;
            }
            wu_iter++;
        }
    }

    // go through APP_VERSIONs;
    // delete any not referenced by any WORKUNIT
    // and superceded by a more recent version.
    std::vector<APP_VERSION*>::iterator avp_iter = app_versions.begin();
    while (avp_iter != app_versions.end()) {
        APP_VERSION* avp = *avp_iter;
        if (avp->ref_cnt == 0) {
            found = false;
            for (j=0; j<app_versions.size(); j++) {
                APP_VERSION* avp2 = app_versions[j];
                if ((avp2->app == avp->app)
                        && (avp2->version_num > avp->version_num)
                        && (!strcmp(avp2->plan_class, avp->plan_class))) {
                    found = true;
                    break;
                }
            }
            if (found) {
                delete avp;
                avp_iter = app_versions.erase(avp_iter);
                action = true;
            } else {
                avp_iter++;
            }
        } else {
            avp_iter++;
        }
    }

    // Then go through remaining APP_VERSIONs,
    // bumping refcnt of associated files.
    for (i=0; i<app_versions.size(); i++) {
        APP_VERSION* avp = app_versions[i];
        for (j=0; j<avp->app_files.size(); j++) {
            avp->app_files[j].file_info->ref_cnt++;
        }
    }

    // reference count files involved in PERS_FILE_XFER or FILE_XFER
    // (this seems redundant, but apparently not)
    for (i=0; i<file_xfers->file_xfers.size(); i++) {
        file_xfers->file_xfers[i]->fip->ref_cnt++;
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        pers_file_xfers->pers_file_xfers[i]->fip->ref_cnt++;
    }

    // delete FILE_INFOs (and corresponding files) that are not referenced
    // Don't do this if sticky and not marked for delete
    std::vector<FILE_INFO*>::iterator fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        FILE_INFO* fip = *fi_iter;
        bool exempt = fip->sticky;
        if (fip->status < 0) exempt = false;
        if (fip->marked_for_delete) exempt = false;
        if (fip->ref_cnt==0 && !exempt) {
            if (fip->pers_file_xfer) {
                pers_file_xfers->remove(fip->pers_file_xfer);
                delete fip->pers_file_xfer;
                fip->pers_file_xfer = 0;
            }
            fip->delete_file();
            if (log_flags.state_debug) {
                msg_printf(0, MSG_INFO,
                        "[state_debug] CLIENT_STATE::garbage_collect(): deleting file %s\n",
                        fip->name.c_str());
            }
            delete fip;
            fi_iter = file_infos.erase(fi_iter);
            action = true;
        } else {
            fi_iter++;
        }
    }

    if ((action) && (log_flags.state_debug)) {
        print_summary();
    }
    return action;
}

/// Perform state transitions for results.
/// For results that are waiting for file transfer,
/// check if the transfer is done,
/// and if so switch to new state and take other actions.
/// Also set some fields for newly-aborted results.
///
/// \return True if there were some changes.
bool CLIENT_STATE::update_results() {
    bool action = false;
    static double last_time=0;

    if (gstate.now - last_time < 1.0) {
        return false;
    }
    last_time = gstate.now;

    RESULT_PVEC::iterator result_iter = results.begin();
    while (result_iter != results.end()) {
        RESULT* rp = *result_iter;

        switch (rp->state()) {
        case RESULT_NEW:
            rp->set_state(RESULT_FILES_DOWNLOADING, "CS::update_results");
            action = true;
            break;
        case RESULT_FILES_DOWNLOADING: {
            FILE_INFO_PSET missing_files;
            int retval = input_files_available(rp, false, &missing_files);
            if (!retval) {
                rp->set_state(RESULT_FILES_DOWNLOADED, "CS::update_results");
                if (rp->avp->app_files.empty()) {
                    // if this is a file-transfer app, start the upload phase
                    rp->set_state(RESULT_FILES_UPLOADING, "CS::update_results");
                    rp->clear_uploaded_flags();
                } else {
                    // else try to start the computation
                    request_schedule_cpus("files downloaded");
                }
                action = true;
            } else {
                // Some files are still missing. Make sure there is a download
                // running for all of them:
                for (FILE_INFO_PSET::iterator fip = missing_files.begin(); fip != missing_files.end(); ++fip) {
                    if ((*fip)->status == FILE_NOT_PRESENT_NOT_NEEDED) {
                        // The file is required now, therefore trigger a download.
                        (*fip)->status = FILE_NOT_PRESENT;
                    }
                }
            }
            break;
        }
        case RESULT_FILES_UPLOADING:
            if (rp->is_upload_done()) {
                rp->ready_to_report = true;
                rp->completed_time = gstate.now;
                rp->set_state(RESULT_FILES_UPLOADED, "CS::update_results");
                action = true;
            }
            break;
        case RESULT_FILES_UPLOADED:
            break;
        case RESULT_ABORTED:
            if (!rp->ready_to_report) {
                rp->ready_to_report = true;
                rp->completed_time = now;
                action = true;
            }
            break;
        }
        result_iter++;
    }
    return action;
}

/// Returns true if client should exit because of debugging criteria
/// (timeout or idle)
bool CLIENT_STATE::time_to_exit() const {
    if (exit_after_app_start_secs
        && (app_started>0)
        && ((now - app_started) >= exit_after_app_start_secs)
    ) {
        msg_printf(NULL, MSG_INFO,
            "Exiting because time is up: %d", exit_after_app_start_secs
        );
        return true;
    }
    if (exit_when_idle && results.empty() && contacted_sched_server) {
        msg_printf(NULL, MSG_INFO, "exiting because no more results");
        return true;
    }
    if (cant_write_state_file) {
        static bool first = true;
        double t = now - last_wakeup_time;
        if (first && t > 50) {
            first = false;
            msg_printf(NULL, MSG_INFO,
                "Can't write state file, exiting in 10 seconds"
            );
        }
        if (t > 60) {
            msg_printf(NULL, MSG_INFO,
                "Can't write state file, exiting now"
            );
            return true;
        }
    }
    return false;
}

/// Call this when a result has a nonrecoverable error.
/// - back off on contacting the project's scheduler
///   (so don't crash over and over)
/// - Append a description of the error to result.stderr_out
/// - If result state is FILES_DOWNLOADED, change it to COMPUTE_ERROR
///   so that we don't try to run it again.
///
/// \param[in,out] res Reference to the failed result.
/// \param[in] format Format string to format the error message.
/// \return Always returns Zero.
///
/// \todo This whole message-formatting with variable arguments like printf is
/// just insane and should be removed as soon as possible.
int CLIENT_STATE::report_result_error(RESULT& res, const char* format, ...) {
    // Only do this once per result.
    if (res.ready_to_report) {
        return 0;
    }

    res.ready_to_report = true;
    res.completed_time = now;

    char err_msg[4096];
    // The above store 1-line messages and short XML snippets.
    // Shouldn't exceed a few hundred bytes.

    va_list va;
    va_start(va, format);
    vsnprintf(err_msg, sizeof(err_msg), format, va);
    va_end(va);
    err_msg[sizeof(err_msg) - 1] = 0; // Yes, I'm paranoid. But it's vsnprintf, which *can* leave the string without '\0' at the end.

    std::ostringstream backoff_buf;
    backoff_buf << "Unrecoverable error for result " << res.name << " (" << err_msg << ")";
    scheduler_op->backoff(res.project, backoff_buf.str().c_str());

    std::ostringstream stderr_buf;
    stderr_buf << "<message>\n" << err_msg << "\n</message>\n";
    res.stderr_out.append(stderr_buf.str());

    switch(res.state()) {
    case RESULT_NEW:
    case RESULT_FILES_DOWNLOADING:
        // called from:
        // CLIENT_STATE::garbage_collect()
        //   if WU or app_version had a download failure
        if (!res.exit_status) {
            res.exit_status = ERR_RESULT_DOWNLOAD;
        }
        break;

    case RESULT_FILES_DOWNLOADED:
        // called from:
        // ACTIVE_TASK::start (if couldn't start app)
        // ACTIVE_TASK::restart (if files missing)
        // ACITVE_TASK_SET::restart_tasks (catch other error returns)
        // ACTIVE_TASK::handle_exited_app (on nonzero exit or signal)
        // ACTIVE_TASK::abort_task (if exceeded resource limit)
        // CLIENT_STATE::schedule_cpus (catch-all for resume/start errors)
        res.set_state(RESULT_COMPUTE_ERROR, "CS::report_result_error");
        if (!res.exit_status) {
            res.exit_status = ERR_RESULT_START;
        }
        break;

    case RESULT_FILES_UPLOADING:
        // called from
        // CLIENT_STATE::garbage_collect() if result had an upload error
        for (FILE_REF_VEC::const_iterator it = res.output_files.begin(); it != res.output_files.end(); ++it) {
            const FILE_INFO* cur_finfo = (*it).file_info;
            int failnum;
            if (cur_finfo->had_failure(failnum)) {
                std::ostringstream buf;
                buf << "<upload_error>\n"
                    << XmlTag<std::string>("file_name", cur_finfo->name)
                    << XmlTag<int> ("error_code", failnum)
                    << "</upload_error>\n";
                res.stderr_out.append(buf.str());
            }
        }
        if (!res.exit_status) {
            res.exit_status = ERR_RESULT_UPLOAD;
        }
        break;
    case RESULT_FILES_UPLOADED:
        msg_printf(res.project, MSG_INTERNAL_ERROR,
            "Error reported for completed task %s", res.name
        );
        break;
    }

    // Prevent the stderr_out contents from beeing longer than MAX_STDERR_LEN:
    if (res.stderr_out.size() > MAX_STDERR_LEN) {
        res.stderr_out.erase(MAX_STDERR_LEN);
    }
    return 0;
}

/// "Reset" a project: (clear error conditions)
/// - stop all active tasks
/// - stop all file transfers
/// - stop scheduler RPC if any
/// - delete all workunits and results
/// - delete all apps and app_versions
/// - garbage collect to delete unneeded files
///
/// Note: does NOT delete persistent files or user-supplied files;
/// does not delete project dir
int CLIENT_STATE::reset_project(PROJECT* project, bool detaching) {
    unsigned int i;
    APP_VERSION* avp;
    APP* app;
    std::vector<APP*>::iterator app_iter;
    std::vector<APP_VERSION*>::iterator avp_iter;
    RESULT* rp;
    PERS_FILE_XFER* pxp;

    msg_printf(project, MSG_INFO, "Resetting project");
    active_tasks.abort_project(project);

    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        pxp = pers_file_xfers->pers_file_xfers[i];
        if (pxp->fip->project == project) {
            if (pxp->fxp) {
                file_xfers->remove(pxp->fxp);
                delete pxp->fxp;
            }
            pers_file_xfers->remove(pxp);
            delete pxp;
            i--;
        }
    }

    // if we're in the middle of a scheduler op to the project, abort it
    //
    scheduler_op->abort(project);

    // mark results as server-acked.
    // This will cause garbage_collect to delete them,
    // and in turn their WUs will be deleted
    //
    for (i=0; i<results.size(); i++) {
        rp = results[i];
        if (rp->project == project) {
            rp->got_server_ack = true;
        }
    }

    project->user_files.clear();
    project->project_files.clear();

    garbage_collect_always();

    // "ordered_scheduled_results" may contain pointers (now dangling)
    // to tasks of this project.
    //
    ordered_scheduled_results.clear();

    // remove apps and app_versions (but not if anonymous platform)
    //
    if (!project->anonymous_platform || detaching) {
        avp_iter = app_versions.begin();
        while (avp_iter != app_versions.end()) {
            avp = *avp_iter;
            if (avp->project == project) {
                avp_iter = app_versions.erase(avp_iter);
                delete avp;
            } else {
                avp_iter++;
            }
        }

        app_iter = apps.begin();
        while (app_iter != apps.end()) {
            app = *app_iter;
            if (app->project == project) {
                app_iter = apps.erase(app_iter);
                delete app;
            } else {
                app_iter++;
            }
        }
        garbage_collect_always();
    }

    project->duration_correction_factor = 1;
    project->ams_resource_share = -1;
    project->min_rpc_time = 0;
    write_state_file();
    return 0;
}

/// "Detach" a project:
/// - Reset (see above)
/// - delete all file infos
/// - delete account file
/// - delete account directory
///
/// \param[in] project Pointer to a PROJECT instance for the project that
///                    should be detached from.
/// \return Always returns Zero.
int CLIENT_STATE::detach_project(PROJECT* project) {
    reset_project(project, true);
    msg_printf(project, MSG_INFO, "Detaching from project");

    // Delete all FILE_INFOs associated with this project:
    std::vector<FILE_INFO*>::iterator fi_iter = file_infos.begin();
    while (fi_iter != file_infos.end()) {
        FILE_INFO* fip = *fi_iter;
        if (fip->project == project) {
            fi_iter = file_infos.erase(fi_iter);
            delete fip;
        } else {
            fi_iter++;
        }
    }

    // If global prefs came from this project, delete file and reinit:
    const PROJECT* p = lookup_project(global_prefs.source_project);
    if (p == project) {
        boinc_delete_file(GLOBAL_PREFS_FILE_NAME);
        global_prefs.defaults();
    }

    // Find project and remove it from the vector:
    for (std::vector<PROJECT*>::iterator project_iter = projects.begin(); project_iter != projects.end(); project_iter++) {
        if ((*project_iter) == project) {
            project_iter = projects.erase(project_iter);
            break;
        }
    }

    // Delete statistics file:
    std::string path = get_statistics_filename(project->get_master_url());
    int retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Can't delete statistics file: %s", boincerror(retval));
    }

    // Delete account file:
    path = get_account_filename(project->get_master_url());
    retval = boinc_delete_file(path);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Can't delete account file: %s", boincerror(retval));
    }

    // Delete scheduler request file:
    path = get_sched_request_filename(*project);
    boinc_delete_file(path);

    // Delete scheduler reply file:
    path = get_sched_reply_filename(*project);
    boinc_delete_file(path);

    // Delete master file:
    path = get_master_filename(*project);
    boinc_delete_file(path);

    // Remove project directory and its contents:
    retval = remove_project_dir(*project);
    if (retval) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Can't delete project directory: %s", boincerror(retval));
    }

    // Finally delete the project:
    delete project;
    write_state_file();
    return 0;
}

/// Quit running applications, quit benchmarks,
/// write the client_state.xml file
/// (in principle we could also terminate net_xfers here,
/// e.g. flush buffers, but why bother)
int CLIENT_STATE::quit_activities() {
    int retval;

    // calculate long-term debts (for state file)
    //
    adjust_debts();

    retval = active_tasks.exit_tasks();
    if (retval) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
            "Couldn't exit tasks: %s", boincerror(retval)
        );
    }
    write_state_file();
    gui_rpcs.close();
    abort_cpu_benchmarks();
    return 0;
}

/// Return a random double in the range [rmin,rmax)
static inline double rand_range(double rmin, double rmax) {
    if (rmin < rmax) {
        return drand() * (rmax-rmin) + rmin;
    } else {
        return rmin;
    }
}

/// Return a random double in the range [MIN,min(e^n,MAX))
double calculate_exponential_backoff( int n, double MIN, double MAX) {
    double rmax = std::min(MAX, exp((double)n));
    return rand_range(MIN, rmax);
}

/// See if a timestamp in the client state file
/// is later than the current time.
/// If so, the user must have decremented the system clock.
/// Clear all timeout variables.
void CLIENT_STATE::check_clock_reset() {
    now = time(0);
    if (!time_stats.last_update) return;
    if (time_stats.last_update <= now) return;

    msg_printf(NULL, MSG_INFO,
        "System clock was turned backwards; clearing timeouts"
    );
#ifdef ENABLE_UPDATE_CHECK
    new_version_check_time = now;
#endif

    unsigned int i;
    for (i=0; i<projects.size(); i++) {
        PROJECT* p = projects[i];
        p->min_rpc_time = 0;
        if (p->next_rpc_time) {
            p->next_rpc_time = now;
        }
        p->next_file_xfer_up = 0;
        p->next_file_xfer_down = 0;
    }
    for (i=0; i<pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = pers_file_xfers->pers_file_xfers[i];
        pfx->next_request_time = 0;
    }

    // RESULT: could change report_deadline, but not clear how
}
