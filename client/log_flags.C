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

#include "cpp.h"

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
#include "error_numbers.h"
#include "common_defs.h"
#include "file_names.h"
#include "client_msgs.h"
#include "miofile.h"
#include "str_util.h"
#include "filesys.h"
#include "ticpp/ticpp.h"

LOG_FLAGS log_flags;
CONFIG config;

LOG_FLAGS::LOG_FLAGS() {
    defaults();
}

/// Reset the log flags to their default values.
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

/// Parse log flag preferences.
///
/// \param[in] log_flags Pointer to the TinyXML++ Element that contains the
///                      <log_flags>-tag.
void LOG_FLAGS::parse(const ticpp::Element* log_flags) {
    std::string child_name;
    for (ticpp::Element* child = log_flags->FirstChildElement(false); child; child = child->NextSiblingElement(false)) {
        child->GetValue(&child_name);

        if      (child_name == "task")                 child->GetText(&task);
        else if (child_name == "file_xfer")            child->GetText(&file_xfer);
        else if (child_name == "sched_ops")            child->GetText(&sched_ops);
        else if (child_name == "cpu_sched")            child->GetText(&cpu_sched);
        else if (child_name == "cpu_sched_debug")      child->GetText(&cpu_sched_debug);
        else if (child_name == "rr_simulation")        child->GetText(&rr_simulation);
        else if (child_name == "debt_debug")           child->GetText(&debt_debug);
        else if (child_name == "task_debug")           child->GetText(&task_debug);
        else if (child_name == "work_fetch_debug")     child->GetText(&work_fetch_debug);
        else if (child_name == "unparsed_xml")         child->GetText(&unparsed_xml);
        else if (child_name == "state_debug")          child->GetText(&state_debug);
        else if (child_name == "file_xfer_debug")      child->GetText(&file_xfer_debug);
        else if (child_name == "sched_op_debug")       child->GetText(&sched_op_debug);
        else if (child_name == "http_debug")           child->GetText(&http_debug);
        else if (child_name == "proxy_debug")          child->GetText(&proxy_debug);
        else if (child_name == "time_debug")           child->GetText(&time_debug);
        else if (child_name == "http_xfer_debug")      child->GetText(&http_xfer_debug);
        else if (child_name == "benchmark_debug")      child->GetText(&benchmark_debug);
        else if (child_name == "poll_debug")           child->GetText(&poll_debug);
        else if (child_name == "guirpc_debug")         child->GetText(&guirpc_debug);
        else if (child_name == "scrsave_debug")        child->GetText(&scrsave_debug);
        else if (child_name == "app_msg_send")         child->GetText(&app_msg_send);
        else if (child_name == "app_msg_receive")      child->GetText(&app_msg_receive);
        else if (child_name == "mem_usage_debug")      child->GetText(&mem_usage_debug);
        else if (child_name == "network_status_debug") child->GetText(&network_status_debug);
        else if (child_name == "checkpoint_debug")     child->GetText(&checkpoint_debug);
        else {
            msg_printf(NULL, MSG_USER_ERROR, "Unrecognized tag in %s: <%s>\n", log_flags->GetDocument()->Value().c_str(), child_name.c_str());
        }
    }
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

/// Reset the preferences to their default values.
void CONFIG::defaults() {
    dont_check_file_sizes = false;
    http_1_0 = false;
    save_stats_days = 30;
    ncpus = 0;
    max_file_xfers = 8;
    max_file_xfers_per_project = 2;
    suppress_net_info = false;
    disallow_attach = false;
    os_random_only = false;
    no_alt_platform = false;
    simple_gui_only = false;
    dont_contact_ref_site = false;
    max_stdout_file_size = 0;
    max_stderr_file_size = 0;
    alt_platforms.clear();
    report_results_immediately = false;
    start_delay = 0;
    run_apps_manually = false;
    force_auth = "default";
}

/// Parse preferences.
///
/// \param[in] log_flags Pointer to the TinyXML++ Element that contains the
///                      <options>-tag.
void CONFIG::parse_options(const ticpp::Element* options) {
    std::string child_name;
    for (ticpp::Element* child = options->FirstChildElement(false); child; child = child->NextSiblingElement(false)) {
        child->GetValue(&child_name);

        if      (child_name == "save_stats_days")            child->GetText(&save_stats_days);
        else if (child_name == "dont_check_file_sizes")      child->GetText(&dont_check_file_sizes);
        else if (child_name == "http_1_0")                   child->GetText(&http_1_0);
        else if (child_name == "ncpus")                      child->GetText(&ncpus);
        else if (child_name == "max_file_xfers")             child->GetText(&max_file_xfers);
        else if (child_name == "max_file_xfers_per_project") child->GetText(&max_file_xfers_per_project);
        else if (child_name == "suppress_net_info")          child->GetText(&suppress_net_info);
        else if (child_name == "disallow_attach")            child->GetText(&disallow_attach);
        else if (child_name == "os_random_only")             child->GetText(&os_random_only);
        else if (child_name == "no_alt_platform")            child->GetText(&no_alt_platform);
        else if (child_name == "simple_gui_only")            child->GetText(&simple_gui_only);
        else if (child_name == "dont_contact_ref_site")      child->GetText(&dont_contact_ref_site);
        else if (child_name == "max_stdout_file_size")       child->GetText(&max_stdout_file_size);
        else if (child_name == "max_stderr_file_size")       child->GetText(&max_stderr_file_size);
        else if (child_name == "report_results_immediately") child->GetText(&report_results_immediately);
        else if (child_name == "start_delay")                child->GetText(&start_delay);
        else if (child_name == "run_apps_manually")          child->GetText(&run_apps_manually);
        else if (child_name == "alt_platform") {
            std::string pf;
            child->GetText(&pf);
            alt_platforms.push_back(pf);
        } else if (child_name == "data_dir") {
            std::string path;
            child->GetText(&path);
            if (chdir(path.c_str())) {
                perror("chdir");
                exit(1);
            }
        } else if (child_name == "force_auth") {
            child->GetText(&force_auth);
            downcase_string(force_auth);
        } else {
            msg_printf(NULL, MSG_USER_ERROR, "Unrecognized tag in %s: <%s>\n", options->GetDocument()->Value().c_str(), child_name.c_str());
        }
    }
}

int CONFIG::parse(const std::string& cfg_file_name) {
    try {
        ticpp::Document doc(cfg_file_name);
        doc.LoadFile();
        ticpp::Element* root_node = doc.FirstChildElement();

        ticpp::Element* log_flags_xml = root_node->FirstChildElement("log_flags");
        log_flags.parse(log_flags_xml);

        ticpp::Element* options_xml = root_node->FirstChildElement("options");
        parse_options(options_xml);
        return 0;
    } catch(ticpp::Exception& ex) {
        msg_printf(NULL, MSG_USER_ERROR, "Error while parsing config file \"%s\": %s", cfg_file_name.c_str(), ex.what());
        return ERR_XML_PARSE;
    }
}

/// Read the config file.
///
/// \param[in] init Set this parameter to true when reading the config file
///                 for the first time and false when re-reading it.
/// \return Zero on success, nonzero if an error occured while parsing the
///         config file.
int read_config_file(bool init) {
    log_flags.defaults();
    config.defaults();

    if (!init) {
        msg_printf(NULL, MSG_INFO, "Re-reading cc_config.xml");
    }
    int ret_val = config.parse(CONFIG_FILE);
    return ret_val;
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
