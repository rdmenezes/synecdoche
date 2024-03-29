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

/// \file
/// command-line parsing

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <stdio.h>
#include <unistd.h>
#endif

#include <cstring>

#include "version.h"

#include "str_util.h"
#include "util.h"
#include "client_msgs.h"
#include "client_state.h"
#include "sandbox.h"

static void print_options(const char* prog) {
    printf(
        "The command-line options for %s are intended for debugging.\n"
        "The recommended command-line interface is a separate program, 'syneccmd'.\n"
        "Run syneccmd in the same directory as %s.\n"
        "\n"
        "Usage: %s [options]\n"
        "    --allow_multiple_clients        allow >1 instances per host\n"
        "    --allow_remote_gui_rpc          allow remote GUI RPC connections\n"
        "    --check_all_logins              for idle detection, check remote logins too\n"
#ifdef _WIN32
        "    --daemon                        run as system service\n"
        "    --detach                        detach from console\n"
#else
        "    --daemon                        run as daemon\n"
#endif
        "    --dir <path>                    use given dir as Synecdoche home\n"
        "    --gui_rpc_port <port>           port for GUI RPCs\n"
        "    --help                          show options\n"
#ifdef SANDBOX
        "    --insecure                      disable app sandboxing\n"
#endif
        "    --launched_by_manager           core client was launched by Manager\n"
        "    --no_gui_rpc                    don't allow GUI RPC, don't make socket\n"
        "    --redirectio                    redirect stdout and stderr to log files\n"
        "    --run_cpu_benchmarks            run the CPU benchmarks\n"
        "    --start_delay X                 delay starting apps for X secs\n"
        "    --version                       show version info\n", 
        prog, prog, prog
    );
}

#define ARG(s) (!strcmp(argv[i], "-" s)||!strcmp(argv[i], "--" s))

/// Parse the command line arguments passed to the client.
/// NOTE: init() has not been called at this point
/// (i.e. client_state.xml has not been parsed),
/// so just record the args here.
///
/// Check for both -X (deprecated) and --X
///
void CLIENT_STATE::parse_cmdline(int argc, const char* const* argv) {
    int i;
    bool show_options = false;

    for (i=1; i<argc; i++) {
        if (ARG("exit_when_idle")) {
            exit_when_idle = true;
            config.report_results_immediately = true;
        } else if (ARG("exit_before_start")) {
            exit_before_start = true;
        } else if (ARG("exit_after_finish")) {
            exit_after_finish = true;
        } else if (ARG("check_all_logins")) {
            check_all_logins = true;
        } else if (ARG("daemon")) {
            executing_as_daemon = true;
        } else if (ARG("skip_cpu_benchmarks")) {
            skip_cpu_benchmarks = true;
        } else if (ARG("exit_after_app_start")) {
            if (i == argc-1) show_options = true;
            else exit_after_app_start_secs = atoi(argv[++i]);
        } else if (ARG("file_xfer_giveup_period")) {
            if (i == argc-1) show_options = true;
            else file_xfer_giveup_period = atoi(argv[++i]);
        } else if (ARG("saver")) {
            started_by_screensaver = true;
        } else if (!strncmp(argv[i], "-psn_", strlen("-psn_"))) {
            // ignore -psn argument on Mac OS X
        } else if (ARG("exit_before_upload")) {
            exit_before_upload = true;
        // The following are only used for testing to alter scheduler/file transfer
        // backoff rates
        } else if (ARG("master_fetch_period")) {
            if (i == argc-1) show_options = true;
            else master_fetch_period = atoi(argv[++i]);
        } else if (ARG("retry_cap")) {
            if (i == argc-1) show_options = true;
            else retry_cap = atoi(argv[++i]);
        } else if (ARG("master_fetch_retry_cap")) {
            if (i == argc-1) show_options = true;
            else master_fetch_retry_cap = atoi(argv[++i]);
        } else if (ARG("master_fetch_interval")) {
            if (i == argc-1) show_options = true;
            else master_fetch_interval = atoi(argv[++i]);
        } else if (ARG("sched_retry_delay_min")) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_min = atoi(argv[++i]);
        } else if (ARG("sched_retry_delay_max")) {
            if (i == argc-1) show_options = true;
            else sched_retry_delay_max = atoi(argv[++i]);
        } else if (ARG("pers_retry_delay_min")) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_min = atoi(argv[++i]);
        } else if (ARG("pers_retry_delay_max")) {
            if (i == argc-1) show_options = true;
            else pers_retry_delay_max = atoi(argv[++i]);
        } else if (ARG("pers_giveup")) {
            if (i == argc-1) show_options = true;
            else pers_giveup = atoi(argv[++i]);
        } else if (ARG("detach_phase_two")) {
            detach_console = true;

        // the above options are private (i.e. not shown by -help)
        // Public options follow.
        // NOTE: if you change or add anything, make the same change
        // in show_options() (above) and in doc/client.php

        } else if (ARG("run_cpu_benchmarks")) {
            run_cpu_benchmarks = true;
        } else if (ARG("version")) {
            detect_platforms();
            printf("%s %s\n", BOINC_VERSION_STRING, get_primary_platform().c_str());
            exit(0);
        } else if (ARG("allow_remote_gui_rpc")) {
            allow_remote_gui_rpc = true;
        } else if (ARG("gui_rpc_port")) {
            cmdline_gui_rpc_port = atoi(argv[++i]);
        } else if (ARG("redirectio")) {
            redirect_io = true;
        } else if (ARG("help")) {
            print_options(argv[0]);
            exit(0);
        } else if (ARG("dir")) {
            if (i == argc-1) {
                show_options = true;
            } else {
                data_directory = argv[++i];
            }
        } else if (ARG("no_gui_rpc")) {
            no_gui_rpc = true;
        } else if (ARG("insecure")) {
#ifdef SANDBOX
            g_use_sandbox = false;
#endif
        } else if (ARG("launched_by_manager")) {
            launched_by_manager = true;
        } else if (ARG("start_delay")) {
            if (i == argc-1) show_options = true;
            else config.start_delay = atof(argv[++i]);
        } else if (ARG("allow_multiple_clients")) {
            config.allow_multiple_clients = true;
        } else {
            printf("Unknown option: %s\n", argv[i]);
            show_options = true;
        }
    }
    if (show_options) {
        print_options(argv[0]);
        exit(1);
    }
}

#undef ARG
#undef ARGX2

void CLIENT_STATE::parse_env_vars() {
    char *p, temp[256];

    p = getenv("HTTP_PROXY");
    if (p && strlen(p) > 0) {
        proxy_info.use_http_proxy = true;
        parse_url(p, proxy_info.http_server_name, proxy_info.http_server_port, temp);
    }
    p = getenv("HTTP_USER_NAME");
    if (p) {
        proxy_info.use_http_auth = true;
        safe_strcpy(proxy_info.http_user_name, p);
        p = getenv("HTTP_USER_PASSWD");
        if (p) {
            safe_strcpy(proxy_info.http_user_passwd, p);
        }
    }

    proxy_info.socks_version = SOCKS_VERSION_5;
    if (getenv("SOCKS4_SERVER")) {
        proxy_info.socks_version = SOCKS_VERSION_4;
    }

    p = getenv("SOCKS4_SERVER");
    if (p && strlen(p)) {
        proxy_info.use_socks_proxy = true;
        parse_url(p, proxy_info.socks_server_name, proxy_info.socks_server_port, temp);
    }

    p = getenv("SOCKS_SERVER");
    if (!p) p = getenv("SOCKS5_SERVER");
    if (p && strlen(p)) {
        proxy_info.use_socks_proxy = true;
        parse_url(p, proxy_info.socks_server_name, proxy_info.socks_server_port, temp);
    }

    p = getenv("SOCKS5_USER");
    if (!p) p = getenv("SOCKS_USER");
    if (p) {
        safe_strcpy(proxy_info.socks5_user_name, p);
    }

    p = getenv("SOCKS5_PASSWD");
    if (p) {
        safe_strcpy(proxy_info.socks5_user_passwd, p);
    }
}
