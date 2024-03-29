// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#endif

#include "log_flags.h"

#include "client_msgs.h"
#include "client_state.h"
#include "common_defs.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "miofile.h"
#include "parse.h"
#include "str_util.h"

LOG_FLAGS log_flags;
CONFIG config;

LOG_FLAGS::LOG_FLAGS() {
    defaults();
}

void LOG_FLAGS::defaults() {
    // on by default
    // (others are off by default)
    task = true;
    file_xfer = true;
    sched_ops = true;

    // off by default; intended for developers and testers
    cpu_sched = false;
    cpu_sched_debug = false;
    rr_simulation = false;
    debt_debug = false;
    task_debug = false;
    work_fetch_debug = false;
    unparsed_xml = false;
    state_debug = false;
    statefile_debug = false;
    file_xfer_debug = false;
    sched_op_debug = false;
    http_debug = false;
    proxy_debug = false;
    time_debug = false;
    http_xfer_debug = false;
    benchmark_debug = false;
    poll_debug = false;
    guirpc_debug = false;
    scrsave_debug = false;
    app_msg_send = false;
    app_msg_receive = false;
    mem_usage_debug = false;
    network_status_debug = false;
    checkpoint_debug = false;
}

/// Parse log flag preferences
int LOG_FLAGS::parse(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/log_flags")) return 0;
        if (xp.parse_bool(tag, "task", task)) continue;
        if (xp.parse_bool(tag, "file_xfer", file_xfer)) continue;
        if (xp.parse_bool(tag, "sched_ops", sched_ops)) continue;
        if (xp.parse_bool(tag, "cpu_sched", cpu_sched)) continue;
        if (xp.parse_bool(tag, "cpu_sched_debug", cpu_sched_debug)) continue;
        if (xp.parse_bool(tag, "rr_simulation", rr_simulation)) continue;
        if (xp.parse_bool(tag, "debt_debug", debt_debug)) continue;
        if (xp.parse_bool(tag, "task_debug", task_debug)) continue;
        if (xp.parse_bool(tag, "work_fetch_debug", work_fetch_debug)) continue;
        if (xp.parse_bool(tag, "unparsed_xml", unparsed_xml)) continue;
        if (xp.parse_bool(tag, "state_debug", state_debug)) continue;
        if (xp.parse_bool(tag, "statefile_debug", statefile_debug)) continue;
        if (xp.parse_bool(tag, "file_xfer_debug", file_xfer_debug)) continue;
        if (xp.parse_bool(tag, "sched_op_debug", sched_op_debug)) continue;
        if (xp.parse_bool(tag, "http_debug", http_debug)) continue;
        if (xp.parse_bool(tag, "proxy_debug", proxy_debug)) continue;
        if (xp.parse_bool(tag, "time_debug", time_debug)) continue;
        if (xp.parse_bool(tag, "http_xfer_debug", http_xfer_debug)) continue;
        if (xp.parse_bool(tag, "benchmark_debug", benchmark_debug)) continue;
        if (xp.parse_bool(tag, "poll_debug", poll_debug)) continue;
        if (xp.parse_bool(tag, "guirpc_debug", guirpc_debug)) continue;
        if (xp.parse_bool(tag, "scrsave_debug", scrsave_debug)) continue;
        if (xp.parse_bool(tag, "app_msg_send", app_msg_send)) continue;
        if (xp.parse_bool(tag, "app_msg_receive", app_msg_receive)) continue;
        if (xp.parse_bool(tag, "mem_usage_debug", mem_usage_debug)) continue;
        if (xp.parse_bool(tag, "network_status_debug", network_status_debug)) continue;
        if (xp.parse_bool(tag, "checkpoint_debug", checkpoint_debug)) continue;
        msg_printf(NULL, MSG_USER_ERROR, "Unrecognized tag in %s: <%s>\n",
            CONFIG_FILE, tag
        );
        xp.skip_unexpected(tag, true, "LOG_FLAGS::parse");
    }
    return ERR_XML_PARSE;
}

static void show_flag(std::string& buf, bool flag, const char* flag_name) {
    if (!flag) {
        return;
    }
    if (buf.empty()) {
        buf.append(flag_name);
        return;
    }
    buf.append(", ").append(flag_name);
    if (buf.size() > 60) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf.c_str());
        buf.clear();
    }
}

/// Print a message containing all log flags that are set to true.
void LOG_FLAGS::show() {
    std::string buf;
    show_flag(buf, task, "task");
    show_flag(buf, file_xfer, "file_xfer");
    show_flag(buf, sched_ops, "sched_ops");
    show_flag(buf, cpu_sched, "cpu_sched");
    show_flag(buf, cpu_sched_debug, "cpu_sched_debug");
    show_flag(buf, rr_simulation, "rr_simulation");
    show_flag(buf, debt_debug, "debt_debug");
    show_flag(buf, task_debug, "task_debug");
    show_flag(buf, work_fetch_debug, "work_fetch_debug");
    show_flag(buf, unparsed_xml, "unparsed_xml");
    show_flag(buf, state_debug, "state_debug");
    show_flag(buf, statefile_debug, "statefile_debug");
    show_flag(buf, file_xfer_debug, "file_xfer_debug");
    show_flag(buf, sched_op_debug, "sched_op_debug");
    show_flag(buf, http_debug, "http_debug");
    show_flag(buf, proxy_debug, "proxy_debug");
    show_flag(buf, time_debug, "time_debug");
    show_flag(buf, http_xfer_debug, "http_xfer_debug");
    show_flag(buf, benchmark_debug, "benchmark_debug");
    show_flag(buf, poll_debug, "poll_debug");
    show_flag(buf, guirpc_debug, "guirpc_debug");
    show_flag(buf, scrsave_debug, "scrsave_debug");
    show_flag(buf, app_msg_send, "app_msg_send");
    show_flag(buf, app_msg_receive, "app_msg_receive");
    show_flag(buf, mem_usage_debug, "mem_usage_debug");
    show_flag(buf, network_status_debug, "network_status_debug");
    show_flag(buf, checkpoint_debug, "checkpoint_debug");
    if (!buf.empty()) {
        msg_printf(NULL, MSG_INFO, "log flags: %s", buf.c_str());
    }
}

CONFIG::CONFIG() {
    defaults();
}

void CONFIG::defaults() {
    dont_check_file_sizes = false;
    http_1_0 = false;
    save_stats_days = 30;
    ncpus = 0;
    max_file_xfers = MAX_FILE_XFERS;
    max_file_xfers_per_project = MAX_FILE_XFERS_PER_PROJECT;
    suppress_net_info = false;
    disallow_attach = false;
    os_random_only = false;
    no_alt_platform = false;
    simple_gui_only = false;
    dont_contact_ref_site = false;
    alt_platforms.clear();
    max_stdout_file_size = 0;
    max_stderr_file_size = 0;
    report_results_immediately = false;
    start_delay = 0;
    run_apps_manually = false;
    force_auth = "default";
    allow_multiple_clients = false;
    zero_debts = false;
}

int CONFIG::parse_options(XML_PARSER& xp) {
    char tag[1024];
    bool is_tag;
    std::string s;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/options")) {
            return 0;
        }
        if (xp.parse_int(tag, "save_stats_days", save_stats_days)) continue;
        if (xp.parse_bool(tag, "dont_check_file_sizes", dont_check_file_sizes)) continue;
        if (xp.parse_bool(tag, "http_1_0", http_1_0)) continue;
        if (xp.parse_int(tag, "ncpus", ncpus)) continue;
        if (xp.parse_int(tag, "max_file_xfers", max_file_xfers)) continue;
        if (xp.parse_int(tag, "max_file_xfers_per_project", max_file_xfers_per_project)) continue;
        if (xp.parse_bool(tag, "suppress_net_info", suppress_net_info)) continue;
        if (xp.parse_bool(tag, "disallow_attach", disallow_attach)) continue;
        if (xp.parse_bool(tag, "os_random_only", os_random_only)) continue;
        if (xp.parse_bool(tag, "no_alt_platform", no_alt_platform)) continue;
        if (xp.parse_bool(tag, "simple_gui_only", simple_gui_only)) continue;
        if (xp.parse_bool(tag, "dont_contact_ref_site", dont_contact_ref_site)) continue;
        if (xp.parse_string(tag, "alt_platform", s)) {
            alt_platforms.push_back(s);
            continue;
        }
        if (xp.parse_int(tag, "max_stdout_file_size", max_stdout_file_size)) continue;
        if (xp.parse_int(tag, "max_stderr_file_size", max_stderr_file_size)) continue;
        if (xp.parse_bool(tag, "report_results_immediately", report_results_immediately)) continue;
        if (xp.parse_double(tag, "start_delay", start_delay)) continue;
        if (xp.parse_bool(tag, "run_apps_manually", run_apps_manually)) continue;
        if (xp.parse_string(tag, "force_auth", force_auth)) {
            downcase_string(force_auth);
            continue;
        }
        if (xp.parse_bool(tag, "zero_debts", zero_debts)) continue;
        if (!strncmp(tag, "proxy_info", sizeof(tag))) {
            int retval = gstate.proxy_info.parse(xp.get_miofile());
            if (retval) {
                return retval;
            }
        }
        msg_printf(NULL, MSG_USER_ERROR, "Unrecognized tag in %s: <%s>\n",
            CONFIG_FILE, tag
        );
        xp.skip_unexpected(tag, true, "CONFIG::parse_options");
    }
    return ERR_XML_PARSE;
}

int CONFIG::parse(FILE* f) {
    char tag[256];
    MIOFILE mf;
    XML_PARSER xp(&mf);
    bool is_tag;

    mf.init_file(f);
    if (!xp.parse_start("cc_config")) {
        msg_printf(NULL, MSG_USER_ERROR, "Missing start tag in %s", CONFIG_FILE);
        return ERR_XML_PARSE;
    }
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            msg_printf(NULL, MSG_USER_ERROR,
               "Unexpected text %s in %s", tag, CONFIG_FILE
            );
            continue;
        }
        if (!strcmp(tag, "/cc_config")) return 0;
        if (!strcmp(tag, "log_flags")) {
            log_flags.parse(xp);
            continue;
        }
        if (!strcmp(tag, "options")) {
            parse_options(xp);
            continue;
        }
        msg_printf(NULL, MSG_USER_ERROR, "Unparsed tag in %s: <%s>\n",
            CONFIG_FILE, tag
        );
        xp.skip_unexpected(tag, true, "CONFIG.parse");
    }
    msg_printf(NULL, MSG_USER_ERROR, "Missing end tag in %s", CONFIG_FILE);
    return ERR_XML_PARSE;
}

/// Read the config file.
///
/// \param[in] init Set this parameter to true when reading the config file
///                 for the first time and false when re-reading it.
/// \return Zero on success, ERR_FOPEN if opening the config file failed.
int read_config_file(bool init) {
    log_flags.defaults();
    config.defaults();

    if (!init) {
        msg_printf(NULL, MSG_INFO, "Re-reading cc_config.xml");
    }
    FILE* f = boinc_fopen(CONFIG_FILE, "r");
    if (!f) {
        return ERR_FOPEN;
    }
    config.parse(f);
    fclose(f);
    return 0;
}

/// Print a message about unparsed xml.
/// If log_flags.unparsed_xml is not set this function does nothing.
///
/// \param[in] in_func The name of the parsing function which discovered the
///                    unparsed xml data.
/// \param[in] buf The unparsed xml data that should be included in the message.
void handle_unparsed_xml_warning(const std::string& in_func, const std::string& buf) {
    if (log_flags.unparsed_xml) {
        // First check if buf only contains whitespaces. If yes ignore it to
        // prevent printing empty messages.
        std::string buf_copy(buf);
        strip_whitespace(buf_copy);
        if (!buf_copy.empty()) {
            msg_printf(0, MSG_INFO, "[unparsed_xml] %s: unrecognized: %s",
                       in_func.c_str(), buf_copy.c_str());
        }
    }
}
