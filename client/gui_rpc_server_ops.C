// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez
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

/// \file
/// GUI RPC server side (the actual RPCs)

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifndef _WIN32
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/un.h>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif

#include <cstdio>
#include <vector>
#include <sstream>
#include <ostream>

#include "gui_rpc_server.h"
#include "str_util.h"
#include "client_state.h"
#include "util.h"
#include "error_numbers.h"
#include "parse.h"
#include "miofile.h"
#include "mfile.h"
#include "network.h"
#include "filesys.h"
#include "version.h"
#include "xml_write.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"
#include "pers_file_xfer.h"

/// Maximum size of the write buffer. If this size is exceeded, the connection
/// will be dropped.
const size_t MAX_WRITE_BUFFER=16384;

using std::string;
using std::vector;

static void auth_failure(std::ostream& out) {
    out << "<unauthorized/>\n";
}

/// Handle an authorization request by creating and sending a nonce.
///
/// \param[in] out The output stream where the request will be written.
void GUI_RPC_CONN::handle_auth1(std::ostream& out) {
    std::ostringstream buf;
    buf << dtime();
    nonce = buf.str();
    out << XmlTag<string>("nonce", nonce);
}

/// Check if the response to the challenge sent by handle_auth1 is correct.
///
/// \param[in] buf The string containing the response from the client.
/// \param[in] out The output stream where the request will be written.
void GUI_RPC_CONN::handle_auth2(const char* buf, std::ostream& out) {
    std::string nonce_hash;
    if (!parse_str(buf, "<nonce_hash>", nonce_hash)) {
        auth_failure(out);
        return;
    }
    std::string buf2 = nonce + std::string(gstate.gui_rpcs.password);
    std::string nonce_hash_correct = md5_string(buf2);
    if (nonce_hash != nonce_hash_correct) {
        auth_failure(out);
        return;
    }
    out << "<authorized/>\n";
    auth_needed = false;
}

// client passes its version, but ignore it for now
static void handle_exchange_versions(std::ostream& out) {
    out << "<server_version>\n"
        << XmlTag<int>("major", SYNEC_MAJOR_VERSION)
        << XmlTag<int>("minor", SYNEC_MINOR_VERSION)
        << XmlTag<int>("release", SYNEC_RELEASE)
        << "</server_version>\n"
    ;
}

static void handle_get_simple_gui_info(std::ostream& out) {
    out << "<simple_gui_info>\n";
    for (size_t i=0; i<gstate.projects.size(); i++) {
        const PROJECT* p = gstate.projects[i];
        p->write_state(out, true);
    }
    gstate.write_tasks_gui(out);
    out << "</simple_gui_info>\n";
}

static void handle_get_project_status(std::ostream& out) {
    out << "<projects>\n";
    for (size_t i=0; i<gstate.projects.size(); i++) {
        const PROJECT* p = gstate.projects[i];
        p->write_state(out, true);
    }
    out << "</projects>\n";
}

static void handle_get_disk_usage(std::ostream& out) {
    out << "<disk_usage_summary>\n";
    get_filesystem_info(gstate.host_info.d_total, gstate.host_info.d_free);

    double size, boinc_non_project;
    dir_size(".", boinc_non_project, false);
    dir_size("locale", size, false);
    boinc_non_project += size;
#ifdef __APPLE__
    if (gstate.launched_by_manager) {
        // If launched by Manager, get Manager's size on disk
        ProcessSerialNumber managerPSN;
        FSRef ourFSRef;
        char path[1024];
        double manager_size = 0.0;
        OSStatus err;
        err = GetProcessForPID(getppid(), &managerPSN);
        if (! err) err = GetProcessBundleLocation(&managerPSN, &ourFSRef);
        if (! err) err = FSRefMakePath (&ourFSRef, (UInt8*)path, sizeof(path));
        if (! err) dir_size(path, manager_size, true);
        if (! err) boinc_non_project += manager_size;
    }
#endif
    double boinc_total = boinc_non_project;
    for (size_t i=0; i<gstate.projects.size(); i++) {
        const PROJECT* p = gstate.projects[i];
        gstate.project_disk_usage(p, size);
        out << "<project>\n"
            << XmlTag<string>("master_url", p->get_master_url())
            << XmlTag<double>("disk_usage", size)
            << "</project>\n"
        ;
        boinc_total += size;
    }
    double d_allowed = gstate.allowed_disk_usage(boinc_total);
    out << XmlTag<double>("d_total",   gstate.host_info.d_total)
        << XmlTag<double>("d_free",    gstate.host_info.d_free)
        << XmlTag<double>("d_boinc",   boinc_non_project)
        << XmlTag<double>("d_allowed", d_allowed)
    ;
    out << "</disk_usage_summary>\n";
}

static PROJECT* get_project(const char* buf, std::ostream& out) {
    string url;
    if (!parse_str(buf, "<project_url>", url)) {
        out << "<error>Missing project URL</error>\n";
        return 0;
    }
    PROJECT* p = gstate.lookup_project(url.c_str());
    if (!p) {
        out << "<error>No such project</error>\n";
        return 0 ;
    }
    return p;
}

static void handle_result_show_graphics(const char* buf, std::ostream& out) {
    string result_name;
    GRAPHICS_MSG gm;
    ACTIVE_TASK* atp;

    if (match_tag(buf, "<full_screen/>")) {
        gm.mode = MODE_FULLSCREEN;
    } else if (match_tag(buf, "<hide/>")) {
        gm.mode = MODE_HIDE_GRAPHICS;
    } else {
        gm.mode = MODE_WINDOW;
    }

    parse_str(buf, "<window_station>", gm.window_station);
    parse_str(buf, "<desktop>", gm.desktop);
    parse_str(buf, "<display>", gm.display);

    if (parse_str(buf, "<result_name>", result_name)) {
        const PROJECT* p = get_project(buf, out);
        if (!p) {
            out << "<error>No such project</error>\n";
            return;
        }
        RESULT* rp = gstate.lookup_result(p, result_name.c_str());
        if (!rp) {
            out << "<error>No such result</error>\n";
            return;
        }
        atp = gstate.lookup_active_task_by_result(rp);
        if (!atp) {
            out << "<error>no such result</error>\n";
            return;
        }
        atp->request_graphics_mode(gm);
    } else {
        for (size_t i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
            atp = gstate.active_tasks.active_tasks[i];
            if (atp->scheduler_state != CPU_SCHED_SCHEDULED) continue;
            atp->request_graphics_mode(gm);
        }
    }
    out << "<success/>\n";
}


static void handle_project_op(const char* buf, std::ostream& out, const char* op) {
    PROJECT* p = get_project(buf, out);
    if (!p) {
        out << "<error>no such project</error>\n";
        return;
    }
    gstate.set_client_state_dirty("Project modified by user");
    if (!strcmp(op, "reset")) {
        gstate.request_schedule_cpus("project reset by user");
        gstate.request_work_fetch("project reset by user");
        gstate.reset_project(p, false);
    } else if (!strcmp(op, "suspend")) {
        p->suspended_via_gui = true;
        gstate.request_schedule_cpus("project suspended by user");
        gstate.request_work_fetch("project suspended by user");
    } else if (!strcmp(op, "resume")) {
        p->suspended_via_gui = false;
        gstate.request_schedule_cpus("project resumed by user");
        gstate.request_work_fetch("project resumed by user");
    } else if (!strcmp(op, "detach")) {
        if (p->attached_via_acct_mgr) {
            msg_printf(p, MSG_USER_ERROR,
                "This project must be detached using the account manager web site."
            );
            out << "<error>must detach using account manager</error>";
            return;
        }
        gstate.detach_project(p);
        gstate.request_schedule_cpus("project detached by user");
        gstate.request_work_fetch("project detached by user");
    } else if (!strcmp(op, "update")) {
        p->sched_rpc_pending = RPC_REASON_USER_REQ;
        p->min_rpc_time = 0;
        gstate.request_work_fetch("project updated by user");
    } else if (!strcmp(op, "nomorework")) {
        p->dont_request_more_work = true;
    } else if (!strcmp(op, "allowmorework")) {
        p->dont_request_more_work = false;
        gstate.request_work_fetch("project allowed to fetch work by user");
    } else if (!strcmp(op, "detach_when_done")) {
        p->detach_when_done = true;
        p->dont_request_more_work = true;
    } else if (!strcmp(op, "dont_detach_when_done")) {
        p->detach_when_done = false;
        p->dont_request_more_work = false;
    }
    out << "<success/>\n";
}

static void handle_set_run_mode(const char* buf, std::ostream& out) {
    double duration = 0;
    parse_double(buf, "<duration>", duration);

    int mode;
    if (match_tag(buf, "<always")) {
        mode = RUN_MODE_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        mode = RUN_MODE_NEVER;
    } else if (match_tag(buf, "<auto")) {
        mode = RUN_MODE_AUTO;
    } else if (match_tag(buf, "<restore")) {
        mode = RUN_MODE_RESTORE;
    } else {
        out << "<error>Missing mode</error>\n";
        return;
    }
    gstate.run_mode.set(mode, duration);
    out << "<success/>\n";
}

static void handle_set_network_mode(const char* buf, std::ostream& out) {
    double duration = 0;
    parse_double(buf, "<duration>", duration);

    int mode;
    if (match_tag(buf, "<always")) {
        mode = RUN_MODE_ALWAYS;
    } else if (match_tag(buf, "<never")) {
        mode = RUN_MODE_NEVER;
    } else if (match_tag(buf, "<auto")) {
        mode = RUN_MODE_AUTO;
    } else if (match_tag(buf, "<restore")) {
        mode = RUN_MODE_RESTORE;
    } else {
        out << "<error>Missing mode</error>\n";
        return;
    }

    gstate.network_mode.set(mode, duration);
    out << "<success/>\n";
}

static void handle_run_benchmarks(const char* , std::ostream& out) {
    gstate.start_cpu_benchmarks();
    out << "<success/>\n";
}

static void handle_set_proxy_settings(const char* buf, std::ostream& out) {
    MIOFILE in;
    in.init_buf_read(buf);
    gstate.proxy_info.parse(in);
    gstate.set_client_state_dirty("Set proxy settings RPC");
    out << "<success/>\n";
    gstate.show_proxy_info();

    // tell running apps to reread app_info file (for F@h)
    //
    gstate.active_tasks.request_reread_app_info();
}

static void handle_get_proxy_settings(const char*, std::ostream& out) {
    gstate.proxy_info.write(out);
}

// params:
// [ <seqno>n</seqno> ]
//    return only msgs with seqno > n; if absent or zero, return all
//
static void handle_get_messages(const char* buf, std::ostream& out) {
    int seqno=0, i, j;
    unsigned int k;
    MESSAGE_DESC* mdp;

    parse_int(buf, "<seqno>", seqno);

    // messages are stored in descreasing seqno,
    // i.e. newer ones are at the head of the vector.
    // compute j = index of first message to return
    //
    j = (int)message_descs.size()-1;
    for (k=0; k<message_descs.size(); k++) {
        mdp = message_descs[k];
        if (mdp->seqno <= seqno) {
            j = k-1;
            break;
        }
    }

    out << "<msgs>\n";
    for (i=j; i>=0; i--) {
        mdp = message_descs[i];
        out << "<msg>\n"
            << XmlTag<XmlString>("project", mdp->project_name)
            << XmlTag<int>("pri", mdp->priority)
            << XmlTag<int>("seqno", mdp->seqno)
            // putting the message contents in its own line is important!
            // the parser on the other end sucks
            << "<body>\n" << XmlString(mdp->message) << "\n</body>\n" 
            << XmlTag<int>("time", mdp->timestamp)
        ;
        out << "</msg>\n";
    }
    out << "</msgs>\n";
}

static void handle_get_message_count(std::ostream& out) {
    int seqno = 0;
    if (!message_descs.empty()) {
        seqno = message_descs.front()->seqno;
    }
    out << XmlTag<int>("seqno", seqno);
}

// <retry_file_transfer>
//    <project_url>XXX</project_url>
//    <filename>XXX</filename>
// </retry_file_transfer>
//
static void handle_file_transfer_op(const char* buf, std::ostream& out, const char* op) {
    string filename;

    const PROJECT* p = get_project(buf, out);
    if (!p) {
        out << "<error>No such project</error>\n";
        return;
    }

    if (!parse_str(buf, "<filename>", filename)) {
        out << "<error>Missing filename</error>\n";
        return;
    }

    FILE_INFO* f = gstate.lookup_file_info(p, filename.c_str());
    if (!f) {
        out << "<error>No such file</error>\n";
        return;
    }

    PERS_FILE_XFER* pfx = f->pers_file_xfer;
    if (!pfx) {
        out << "<error>No such transfer waiting</error>\n";
        return;
    }

    if (!strcmp(op, "retry")) {
        // leave file-level backoff mode
        pfx->next_request_time = 0;
        // and leave project-level backoff mode
        f->project->file_xfer_succeeded(pfx->is_upload);
    } else if (!strcmp(op, "abort")) {
        f->pers_file_xfer->abort();
    } else {
        out << "<error>unknown op</error>\n";
        return;
    }
    gstate.set_client_state_dirty("File transfer RPC");
    out << "<success/>\n";
}

static void handle_result_op(const char* buf, std::ostream& out, const char* op) {
    PROJECT* p = get_project(buf, out);
    if (!p) {
        out << "<error>No such project</error>\n";
        return;
    }

    char result_name[256];
    if (!parse_str(buf, "<name>", result_name, sizeof(result_name))) {
        out << "<error>Missing result name</error>\n";
        return;
    }

    RESULT* rp = gstate.lookup_result(p, result_name);
    if (!rp) {
        out << "<error>no such result</error>\n";
        return;
    }

    if (!strcmp(op, "abort")) {
        ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
        if (atp) {
            atp->abort_task(ERR_ABORTED_VIA_GUI, "aborted by user");
        } else {
            rp->abort_inactive(ERR_ABORTED_VIA_GUI);
        }
        gstate.request_work_fetch("result aborted by user");
    } else if (!strcmp(op, "suspend")) {
        rp->suspended_via_gui = true;
        gstate.request_work_fetch("result suspended by user");
    } else if (!strcmp(op, "resume")) {
        rp->suspended_via_gui = false;
    }
    gstate.request_schedule_cpus("result suspended, resumed or aborted by user");
    gstate.set_client_state_dirty("Result RPC");
    out << "<success/>\n";
}

static void handle_get_host_info(const char*, std::ostream& out) {
    gstate.host_info.write(out, false);
}

static void handle_get_screensaver_tasks(std::ostream& out) {
    out << "<handle_get_screensaver_tasks>\n";
    out << XmlTag<int>("suspend_reason", gstate.suspend_reason);

    for (size_t i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
        const ACTIVE_TASK* atp = gstate.active_tasks.active_tasks[i];
        if ((atp->task_state() == PROCESS_EXECUTING) ||
                ((atp->task_state() == PROCESS_SUSPENDED) &&
                        (gstate.suspend_reason & SUSPEND_REASON_CPU_USAGE_LIMIT))) {
            atp->result->write_gui(out);
        }
    }
    out << "</handle_get_screensaver_tasks>\n";
}

static void handle_quit(const char*, std::ostream& out) {
    gstate.requested_exit = true;
    out << "<success/>\n";
}

static void handle_acct_mgr_info(const char*, std::ostream& out) {
    out << "<acct_mgr_info>\n"
        << XmlTag<const char*>("acct_mgr_url",  gstate.acct_mgr_info.acct_mgr_url)
        << XmlTag<const char*>("acct_mgr_name", gstate.acct_mgr_info.acct_mgr_name)
    ;
    if (strlen(gstate.acct_mgr_info.login_name)) {
        out << "    <have_credentials/>\n";
    }
    out << "</acct_mgr_info>\n";
}

static void handle_get_statistics(const char*, std::ostream& out) {
    out << "<statistics>\n";
    for (std::vector<PROJECT*>::const_iterator i=gstate.projects.begin();
        i != gstate.projects.end(); ++i
    ) {
        (*i)->write_statistics(out, true);
    }
    out << "</statistics>\n";
}

static void handle_get_cc_status(std::ostream& out) {
    out << "<cc_status>\n"
        << XmlTag<int>   ("network_status",         net_status.network_status())
        << XmlTag<int>   ("ams_password_error",     gstate.acct_mgr_info.password_error?1:0)
        << XmlTag<int>   ("task_suspend_reason",    gstate.suspend_reason)
        << XmlTag<int>   ("network_suspend_reason", gstate.network_suspend_reason)
        << XmlTag<int>   ("task_mode",              gstate.run_mode.get_current())
        << XmlTag<int>   ("network_mode",           gstate.network_mode.get_current())
        << XmlTag<int>   ("task_mode_perm",         gstate.run_mode.get_perm())
        << XmlTag<int>   ("network_mode_perm",      gstate.network_mode.get_perm())
        << XmlTag<double>("task_mode_delay",        gstate.run_mode.delay())
        << XmlTag<double>("network_mode_delay",     gstate.network_mode.delay())
        << XmlTag<int>   ("disallow_attach",        config.disallow_attach?1:0)
        << XmlTag<int>   ("simple_gui_only",        config.simple_gui_only?1:0)
        << "</cc_status>\n"
    ;
}

static void handle_network_available(const char*, std::ostream& out) {
    net_status.network_available();
    out << "<success/>\n";
}

static void handle_get_project_init_status(const char*, std::ostream& out) {
    out << "<get_project_init_status>\n"
        << XmlTag<string>("url",  gstate.project_init.url)
        << XmlTag<string>("name", gstate.project_init.name)
    ;
    if (strlen(gstate.project_init.account_key)) {
        out << "    <has_account_key/>\n";
    }
    out << "</get_project_init_status>\n";
}

void GUI_RPC_CONN::handle_get_project_config(const char* buf, std::ostream& out) {
    string url;

    parse_str(buf, "<url>", url);

    canonicalize_master_url(url);
    get_project_config_op.do_rpc(url);
    out << "<success/>\n";
}

void GUI_RPC_CONN::handle_get_project_config_poll(const char*, std::ostream& out) {
    if (get_project_config_op.error_num) {
        out << "<project_config>\n"
            << XmlTag<int>("error_num", get_project_config_op.error_num)
            << "</project_config>\n"
        ;
    } else {
        out << get_project_config_op.reply;
    }
}

void GUI_RPC_CONN::handle_lookup_account(const char* buf, std::ostream& out) {
    ACCOUNT_IN ai;

    ai.parse(buf);
    if (ai.url.empty() || ai.email_addr.empty() || ai.passwd_hash.empty()) {
        out << "<error>missing URL, email address, or password</error>\n";
        return;
    }

    lookup_account_op.do_rpc(ai);
    out << "<success/>\n";
}

void GUI_RPC_CONN::handle_lookup_account_poll(const char*, std::ostream& out) {
    if (lookup_account_op.error_num) {
        out << "<account_out>\n"
            << XmlTag<int>("error_num", lookup_account_op.error_num)
            << "</account_out>\n"
        ;
    } else {
        out << lookup_account_op.reply;
    }
}

void GUI_RPC_CONN::handle_create_account(const char* buf, std::ostream& out) {
    ACCOUNT_IN ai;

    ai.parse(buf);

    create_account_op.do_rpc(ai);
    out << "<success/>\n";
}

void GUI_RPC_CONN::handle_create_account_poll(const char*, std::ostream& out) {
    if (create_account_op.error_num) {
        out << "<account_out>\n"
            << XmlTag<int>("error_num", create_account_op.error_num)
            << "</account_out>\n"
        ;
    } else {
        out << create_account_op.reply;
    }
}

static void handle_project_attach(const char* buf, std::ostream& out) {
    string url, authenticator, project_name;
    bool use_config_file = false;

    // Get URL/auth from project_init.xml?
    //
    if (parse_bool(buf, "use_config_file", use_config_file)) {
        if (gstate.project_init.url.empty()) {
            out << "<error>Missing URL</error>\n";
            return;
        }

        if (!strlen(gstate.project_init.account_key)) {
            out << "<error>Missing authenticator</error>\n";
            return;
        }

        url = gstate.project_init.url;
        authenticator = gstate.project_init.account_key;
    } else {
        if (!parse_str(buf, "<project_url>", url)) {
            out << "<error>Missing URL</error>\n";
            return;
        }
        if (!parse_str(buf, "<authenticator>", authenticator)) {
            out << "<error>Missing authenticator</error>\n";
            return;
        }

        if (authenticator.empty()) {
            out << "<error>Missing authenticator</error>\n";
            return;
        }
        parse_str(buf, "<project_name>", project_name);
    }

    if (gstate.lookup_project(url)) {
        out << "<error>Already attached to project</error>\n";
        return;
    }

    // clear messages from previous attach to project.
    gstate.project_attach.messages.clear();
    gstate.project_attach.error_num = gstate.add_project(
        url.c_str(), authenticator.c_str(), project_name.c_str(), false
    );

    // if project_init.xml refers to this project,
    // delete the file, otherwise we'll just
    // reattach the next time the core client starts
    if (url == gstate.project_init.url) {
        int retval = gstate.project_init.remove();
        if (retval) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Can't delete project init file: %s", boincerror(retval)
            );
        }
    }

    out << "<success/>\n";
}

static void handle_project_attach_poll(const char*, std::ostream& out) {
    out << "<project_attach_reply>\n";
    for (size_t i=0; i<gstate.project_attach.messages.size(); i++) {
        out << XmlTag<string>("message", gstate.project_attach.messages[i]);
    }
    out << XmlTag<int>("error_num", gstate.project_attach.error_num);
    out << "</project_attach_reply>\n";
}

static void handle_acct_mgr_rpc(const char* buf, std::ostream& out) {
    std::string url, name, password;
    std::string password_hash, name_lc;
    bool use_config_file = false;
    bool bad_arg = false;
    if (!parse_bool(buf, "use_config_file", use_config_file)) {
        if (!parse_str(buf, "<url>", url)) bad_arg = true;
        if (!parse_str(buf, "<name>", name)) bad_arg = true;
        if (!parse_str(buf, "<password>", password)) bad_arg = true;
        if (!bad_arg) {
            name_lc = name;
            downcase_string(name_lc);
            password_hash = md5_string(password+name_lc);
        }
    } else {
        if (!strlen(gstate.acct_mgr_info.acct_mgr_url) || !strlen(gstate.acct_mgr_info.acct_mgr_url) || !strlen(gstate.acct_mgr_info.acct_mgr_url)) {
            bad_arg = true;
        } else {
            url = gstate.acct_mgr_info.acct_mgr_url;
            name = gstate.acct_mgr_info.login_name;
            password_hash = gstate.acct_mgr_info.password_hash;
        }
    }
    if (bad_arg) {
        out << "<error>bad arg</error>\n";
    } else {
        gstate.acct_mgr_op.do_rpc(url, name, password_hash, true);
        out << "<success/>\n";
    }
}

static void handle_acct_mgr_rpc_poll(const char*, std::ostream& out) {
    out << "<acct_mgr_rpc_reply>\n";
    if (!gstate.acct_mgr_op.error_str.empty()) {
        out << XmlTag<string>("message", gstate.acct_mgr_op.error_str);
    }
    out << XmlTag<int>("error_num", gstate.acct_mgr_op.error_num);
    out << "</acct_mgr_rpc_reply>\n";
}

#ifdef ENABLE_UPDATE_CHECK
static void handle_get_newer_version(std::ostream& out) {
    out << XmlTag<string>("newer_version", gstate.newer_version);
}
#endif

static void handle_get_global_prefs_file(std::ostream& out) {
    GLOBAL_PREFS p;
    bool found;
    int retval = p.parse_file(
        GLOBAL_PREFS_FILE_NAME, gstate.main_host_venue, found
    );
    if (retval) {
        out << XmlTag<int>("error", retval);
        return;
    }
    p.write(out);
}

static void handle_get_global_prefs_working(std::ostream& out) {
    gstate.global_prefs.write(out);
}

static void handle_get_global_prefs_override(std::ostream& out) {
    string s;
    int retval = read_file_string(GLOBAL_PREFS_OVERRIDE_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        out << s << "\n";
    } else {
        out << "<error>no prefs override file</error>\n";
    }
}

static void handle_set_global_prefs_override(/* const */ char* buf, std::ostream& out) {
    char *p, *q=0;
    int retval = ERR_XML_PARSE;

    // strip off outer tags
    //
    p = strstr(buf, "<set_global_prefs_override>\n");
    if (p) {
        p += strlen("<set_global_prefs_override>\n");
        q = strstr(p, "</set_global_prefs_override");
    }
    if (q) {
        *q = 0;
        strip_whitespace(p);
        if (strlen(p)) {
            FILE* f = boinc_fopen(GLOBAL_PREFS_OVERRIDE_FILE, "w");
            if (f) {
                fprintf(f, "%s\n", p);
                fclose(f);
                retval = 0;
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = boinc_delete_file(GLOBAL_PREFS_OVERRIDE_FILE);
        }
    }
    out << "<set_global_prefs_override_reply>\n"
        << XmlTag<int>("status", retval)
        << "</set_global_prefs_override_reply>\n"
    ;
}

static void handle_get_cc_config(std::ostream& out) {
    string s;
    int retval = read_file_string(CONFIG_FILE, s);
    if (!retval) {
        strip_whitespace(s);
        out << s;
    }
}

static void read_all_projects_list_file(std::ostream& out) {
    string s;
    int retval = read_file_string(ALL_PROJECTS_LIST_FILENAME, s);
    if (!retval) {
        strip_whitespace(s);
        out << s;
    }
}

static int set_debt(XML_PARSER& xp) {
    bool is_tag;
    char tag[256];
    std::string url;
    double short_term_debt = 0.0, long_term_debt = 0.0;
    bool got_std=false, got_ltd=false;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!strcmp(tag, "/project")) {
            if (url.empty()) return ERR_XML_PARSE;
            canonicalize_master_url(url);
            PROJECT* p = gstate.lookup_project(url.c_str());
            if (!p) return ERR_NOT_FOUND;
            if (got_std) p->short_term_debt = short_term_debt;
            if (got_ltd) p->long_term_debt = long_term_debt;
            return 0;
        }
        if (xp.parse_string(tag, "master_url", url)) continue;
        if (xp.parse_double(tag, "short_term_debt", short_term_debt)) {
            got_std = true;
            continue;
        }
        if (xp.parse_double(tag, "long_term_debt", long_term_debt)) {
            got_ltd = true;
            continue;
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "set_debt");
    }
    return 0;
}

static void handle_set_debts(const char* buf, std::ostream& out) {
    MIOFILE in;
    XML_PARSER xp(&in);
    bool is_tag;
    char tag[256];
    int retval;

    in.init_buf_read(buf);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "boinc_gui_rpc_request")) continue;
        if (!strcmp(tag, "set_debts")) continue;
        if (!strcmp(tag, "/set_debts")) {
            out << "<success/>\n";
            gstate.set_client_state_dirty("set_debt RPC");
            return;
        }
        if (!strcmp(tag, "project")) {
            retval = set_debt(xp);
            if (retval) {
                out << XmlTag<int>("error", retval);
                return;
            }
            continue;
        }
        xp.skip_unexpected(tag, log_flags.unparsed_xml, "handle_set_debts");
    }
    out << "<error>No end tag</error>\n";
}

static void handle_set_cc_config(/* const */ char* buf, std::ostream& out) {
    char *p, *q=0;
    int retval = ERR_XML_PARSE;

    // strip off outer tags
    //
    p = strstr(buf, "<set_cc_config>\n");
    if (p) {
        p += strlen("<set_cc_config>\n");
        q = strstr(p, "</set_cc_config");
    }
    if (q) {
        *q = 0;
        strip_whitespace(p);
        if (strlen(p)) {
            FILE* f = boinc_fopen(CONFIG_FILE, "w");
            if (f) {
                fprintf(f, "%s\n", p);
                fclose(f);
                retval = 0;
            } else {
                retval = ERR_FOPEN;
            }
        } else {
            retval = boinc_delete_file(CONFIG_FILE);
        }
    }
    out << "<set_cc_config_reply>\n"
        << XmlTag<int>("status", retval)
        << "</set_cc_config_reply>\n"
    ;
}

int GUI_RPC_CONN::handle_rpc() {
    char request_msg[4096];
    int n;
    std::ostringstream reply;

    // read the request message in one read()
    // so that the core client won't hang because
    // of malformed request msgs
    //
#ifdef _WIN32
        n = recv(sock, request_msg, 4095, 0);
#else
        n = read(sock, request_msg, 4095);
#endif
    if (n <= 0) return ERR_READ;
    request_msg[n-1] = 0;   // replace 003 with NULL

    if (log_flags.guirpc_debug) {
        msg_printf(0, MSG_INFO,
            "[guirpc_debug] GUI RPC Command = '%s'\n", request_msg
        );
    }

    reply << "<boinc_gui_rpc_reply>\n";
    if (match_tag(request_msg, "<auth1")) {
        handle_auth1(reply);
    } else if (match_tag(request_msg, "<auth2")) {
        handle_auth2(request_msg, reply);
    } else if (auth_needed && !is_local) {
        auth_failure(reply);

    // operations that require authentication only for non-local clients start here.
    // Use this only for information that should be available to people
    // sharing this computer (e.g. what jobs are running)
    // but not for anything sensitive (passwords etc.)

    } else if (match_tag(request_msg, "<exchange_versions")) {
        handle_exchange_versions(reply);
    } else if (match_tag(request_msg, "<get_state")) {
        gstate.write_state_gui(reply);
    } else if (match_tag(request_msg, "<get_results")) {
        reply << "<results>\n";
        gstate.write_tasks_gui(reply);
        reply << "</results>\n";
    } else if (match_tag(request_msg, "<get_screensaver_tasks")) {
        handle_get_screensaver_tasks(reply);
    } else if (match_tag(request_msg, "<result_show_graphics")) {
        handle_result_show_graphics(request_msg, reply);
    } else if (match_tag(request_msg, "<get_file_transfers")) {
        gstate.write_file_transfers_gui(reply);
    } else if (match_tag(request_msg, "<get_simple_gui_info")) {
        handle_get_simple_gui_info(reply);
    } else if (match_tag(request_msg, "<get_project_status")) {
        handle_get_project_status(reply);
    } else if (match_tag(request_msg, "<get_disk_usage")) {
        handle_get_disk_usage(reply);
    } else if (match_tag(request_msg, "<get_messages")) {
        handle_get_messages(request_msg, reply);
    } else if (match_tag(request_msg, "<get_message_count")) {
        handle_get_message_count(reply);
    } else if (match_tag(request_msg, "<get_host_info")) {
        handle_get_host_info(request_msg, reply);
    } else if (match_tag(request_msg, "<get_statistics")) {
        handle_get_statistics(request_msg, reply);
#ifdef ENABLE_UPDATE_CHECK
    } else if (match_tag(request_msg, "<get_newer_version>")) {
        handle_get_newer_version(reply);
#endif
    } else if (match_tag(request_msg, "<get_cc_status")) {
        handle_get_cc_status(reply);

    // Operations that require authentication start here

    } else if (auth_needed) {
        auth_failure(reply);
    } else if (match_tag(request_msg, "<project_nomorework")) {
        handle_project_op(request_msg, reply, "nomorework");
    } else if (match_tag(request_msg, "<project_allowmorework")) {
        handle_project_op(request_msg, reply, "allowmorework");
    } else if (match_tag(request_msg, "<project_detach_when_done")) {
        handle_project_op(request_msg, reply, "detach_when_done");
    } else if (match_tag(request_msg, "<project_dont_detach_when_done")) {
        handle_project_op(request_msg, reply, "dont_detach_when_done");
    } else if (match_tag(request_msg, "<set_network_mode")) {
        handle_set_network_mode(request_msg, reply);
    } else if (match_tag(request_msg, "<run_benchmarks")) {
        handle_run_benchmarks(request_msg, reply);
    } else if (match_tag(request_msg, "<get_proxy_settings")) {
        handle_get_proxy_settings(request_msg, reply);
    } else if (match_tag(request_msg, "<set_proxy_settings")) {
        handle_set_proxy_settings(request_msg, reply);
    } else if (match_tag(request_msg, "<network_available")) {
        handle_network_available(request_msg, reply);
    } else if (match_tag(request_msg, "<abort_file_transfer")) {
        handle_file_transfer_op(request_msg, reply, "abort");
    } else if (match_tag(request_msg, "<project_detach")) {
        handle_project_op(request_msg, reply, "detach");
    } else if (match_tag(request_msg, "<abort_result")) {
        handle_result_op(request_msg, reply, "abort");
    } else if (match_tag(request_msg, "<suspend_result")) {
        handle_result_op(request_msg, reply, "suspend");
    } else if (match_tag(request_msg, "<resume_result")) {
        handle_result_op(request_msg, reply, "resume");
    } else if (match_tag(request_msg, "<project_suspend")) {
        handle_project_op(request_msg, reply, "suspend");
    } else if (match_tag(request_msg, "<project_resume")) {
        handle_project_op(request_msg, reply, "resume");
    } else if (match_tag(request_msg, "<set_run_mode")) {
        handle_set_run_mode(request_msg, reply);
    } else if (match_tag(request_msg, "<quit")) {
        handle_quit(request_msg, reply);
    } else if (match_tag(request_msg, "<acct_mgr_info")) {
        handle_acct_mgr_info(request_msg, reply);
    } else if (match_tag(request_msg, "<read_global_prefs_override/>")) {
        reply << "<success/>\n";
        gstate.read_global_prefs();
        gstate.request_schedule_cpus("Preferences override");
        gstate.request_work_fetch("Preferences override");
    } else if (match_tag(request_msg, "<get_project_init_status")) {
        handle_get_project_init_status(request_msg, reply);
    } else if (match_tag(request_msg, "<get_global_prefs_file")) {
        handle_get_global_prefs_file(reply);
    } else if (match_tag(request_msg, "<get_global_prefs_working")) {
        handle_get_global_prefs_working(reply);
    } else if (match_tag(request_msg, "<get_global_prefs_override")) {
        handle_get_global_prefs_override(reply);
    } else if (match_tag(request_msg, "<set_global_prefs_override")) {
        handle_set_global_prefs_override(request_msg, reply);
    } else if (match_tag(request_msg, "<get_cc_config")) {
        handle_get_cc_config(reply);
    } else if (match_tag(request_msg, "<set_cc_config")) {
        handle_set_cc_config(request_msg, reply);
    } else if (match_tag(request_msg, "<read_cc_config/>")) {
        reply << "<success/>\n";
        read_config_file(false);
        msg_printf(0, MSG_INFO, "Re-read config file");
        log_flags.show();
        gstate.zero_debts_if_requested();
        gstate.set_ncpus();
        gstate.request_schedule_cpus("Core client configuration");
        gstate.request_work_fetch("Core client configuration");
    } else if (match_tag(request_msg, "<get_all_projects_list/>")) {
        read_all_projects_list_file(reply);
    } else if (match_tag(request_msg, "<set_debts")) {
        handle_set_debts(request_msg, reply);
    } else if (match_tag(request_msg, "<retry_file_transfer")) {
        handle_file_transfer_op(request_msg, reply, "retry");
    } else if (match_tag(request_msg, "<project_reset")) {
        handle_project_op(request_msg, reply, "reset");
    } else if (match_tag(request_msg, "<project_update")) {
        handle_project_op(request_msg, reply, "update");
    } else if (match_tag(request_msg, "<get_project_config>")) {
        handle_get_project_config(request_msg, reply);
    } else if (match_tag(request_msg, "<get_project_config_poll")) {
        handle_get_project_config_poll(request_msg, reply);
    } else if (match_tag(request_msg, "<lookup_account>")) {
        handle_lookup_account(request_msg, reply);
    } else if (match_tag(request_msg, "<lookup_account_poll")) {
        handle_lookup_account_poll(request_msg, reply);
    } else if (match_tag(request_msg, "<create_account>")) {
        handle_create_account(request_msg, reply);
    } else if (match_tag(request_msg, "<create_account_poll")) {
        handle_create_account_poll(request_msg, reply);
    } else if (match_tag(request_msg, "<project_attach>")) {
        handle_project_attach(request_msg, reply);
    } else if (match_tag(request_msg, "<project_attach_poll")) {
        handle_project_attach_poll(request_msg, reply);
    } else if (match_tag(request_msg, "<acct_mgr_rpc>")) {
        handle_acct_mgr_rpc(request_msg, reply);
    } else if (match_tag(request_msg, "<acct_mgr_rpc_poll")) {
        handle_acct_mgr_rpc_poll(request_msg, reply);

    // DON'T JUST ADD NEW RPCS HERE - THINK ABOUT THEIR
    // AUTHENTICATION AND NETWORK REQUIREMENTS FIRST

    } else {
        reply << "<error>unrecognized op</error>\n";
    }

    reply << "</boinc_gui_rpc_reply>\n\003";

    std::string s_reply = reply.str();
    if (write_buffer.length() > MAX_WRITE_BUFFER) {
        return ERR_BUFFER_OVERFLOW;
    }
    write_buffer.append(s_reply);

    // strip final \003
    if (s_reply[s_reply.size()-1] == '\003') {
        s_reply.resize(s_reply.size()-1);
    }

    if (log_flags.guirpc_debug) {
        msg_printf(0, MSG_INFO,
            "[guirpc_debug] GUI RPC reply: '%.50s'\n", s_reply.c_str()
        );
    }
    return 0;
}

/// Writes as much as possible from the send buffer. The remaining data (if
/// any) is left in the buffer. If send returns an error, this function returns
/// -1.
int GUI_RPC_CONN::handle_write() {
    int retval = send(sock, &write_buffer[0], write_buffer.length(), 0);
    if (retval < 0) {
        return retval;
    } else {
        write_buffer.erase(0, retval);
    }
    return 0;
}
