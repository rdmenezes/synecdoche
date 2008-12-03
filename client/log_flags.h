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
/// flags determining what is written to standard out.
/// (errors go to stderr)

// NOTE: all writes to stdout should have an if (log_flags.*) {} around them.

#ifndef LOGFLAGS_H
#define LOGFLAGS_H

#include <vector>
#include <string>
#include "ticpp/ticpp.h"

#ifndef _WIN32
#include <stdio.h>
#endif

struct LOG_FLAGS {
    // on by default, user-readable
    bool task;              ///< task start and finish
    bool file_xfer;         ///< file transfer start and finish
    bool sched_ops;         ///< interactions with schedulers

    // off by default; intended for developers and testers
    bool cpu_sched;         ///< preemption and resumption
    bool cpu_sched_debug;   ///< explain scheduler decisions
    bool rr_simulation;     ///< results of rr simulator
    bool debt_debug;        ///< changes to debt
    bool task_debug;        ///< task start and control details
                            ///< also prints when apps checkpoint
    bool work_fetch_debug;  ///< work fetch policy

    bool unparsed_xml;      ///< show unparsed XML lines
    bool state_debug;       ///< print textual summary of CLIENT_STATE initially
                            ///< and after each scheduler RPC and garbage collect
                            ///< also show actions of garbage collector
                            ///< also show when state file is read or written
    bool file_xfer_debug;   ///< show completion of FILE_XFER
    bool sched_op_debug;
    bool http_debug;
    bool proxy_debug;
    bool time_debug;        ///< changes in on_frac, active_frac, connected_frac
    bool http_xfer_debug;
    bool benchmark_debug;   ///< debug CPU benchmarks
    bool poll_debug;        ///< show what polls are responding
    bool guirpc_debug;
    bool scrsave_debug;
    bool app_msg_send;      ///< show shared-mem message to apps
    bool app_msg_receive;   ///< show shared-mem message from apps
    bool mem_usage_debug;   ///< memory usage
    bool network_status_debug;
    bool checkpoint_debug;

    LOG_FLAGS();

    /// Reset the log flags to their default values.
    void defaults();

    /// Parse log flag preferences.
    void parse(const ticpp::Element* options);

    void show();
};

struct CONFIG {
    bool dont_check_file_sizes;
    bool http_1_0;
    int save_stats_days;
    int ncpus;
    int max_file_xfers;
    int max_file_xfers_per_project;
    bool suppress_net_info;
    bool disallow_attach;
    bool os_random_only;
    bool no_alt_platform;
    bool simple_gui_only;
    bool dont_contact_ref_site;
    std::vector<std::string> alt_platforms;
    int max_stdout_file_size;
    int max_stderr_file_size;
    bool report_results_immediately;
    double start_delay;
    bool run_apps_manually;
    std::string force_auth;
    bool allow_multiple_clients;

    CONFIG();

    /// Reset the preferences to their default values.
    void defaults();

    /// Read the config file.
    int parse(const std::string& cfg_file_name);

    /// Parse preferences.
    void parse_options(const ticpp::Element* options);
};

extern LOG_FLAGS log_flags;
extern CONFIG config;

/// Read the config file.
extern int read_config_file(bool init);

#endif // LOGFLAGS_H
