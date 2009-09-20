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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "zlib.h"
#else
#include "config.h"
// Somehow having config.h define _FILE_OFFSET_BITS or _LARGE_FILES is
// causing open to be redefined to open64 which somehow, in some versions
// of zlib.h causes gzopen to be redefined as gzopen64 which subsequently gets
// reported as a linker error.  So for this file, we compile in small files
// mode, regardless of these settings
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <cstring>
#include <sstream>
#endif

#include "client_types.h"

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "client_msgs.h"
#include "log_flags.h"
#include "parse.h"
#include "miofile.h"
#include "util.h"
#include "str_util.h"
#include "client_state.h"
#include "pers_file_xfer.h"
#include "sandbox.h"
#include "scheduler_op.h"

using std::string;
using std::vector;

PROJECT::PROJECT() {
    init();
}

void PROJECT::init() {
    master_url.clear();
    strcpy(authenticator, "");
    project_specific_prefs = "";
    gui_urls = "";
    resource_share = 100;
    strcpy(host_venue, "");
    using_venue_specific_prefs = false;
    scheduler_urls.clear();
    strcpy(project_name, "");
    strcpy(symstore, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(email_hash, "");
    strcpy(cross_project_id, "");
    cpid_time = 0;
    user_total_credit = 0;
    user_expavg_credit = 0;
    user_create_time = 0;
    ams_resource_share = 0;
    rpc_seqno = 0;
    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    nrpc_failures = 0;
    master_fetch_failures = 0;
    min_rpc_time = 0;
    possibly_backed_off = true;
    master_url_fetch_pending = false;
    sched_rpc_pending = NO_RPC_REASON;
    next_rpc_time = 0;
    last_rpc_time = 0;
    trickle_up_pending = false;
    anonymous_platform = false;
    non_cpu_intensive = false;
    short_term_debt = 0;
    long_term_debt = 0;
    send_file_list = false;
    send_time_stats_log = 0;
    send_job_log = 0;
    suspended_via_gui = false;
    dont_request_more_work = false;
    detach_when_done = false;
    attached_via_acct_mgr = false;
    ended = false;
    strcpy(code_sign_key, "");
    user_files.clear();
    project_files.clear();
    anticipated_debt = 0;
    wall_cpu_time_this_debt_interval = 0;
    next_runnable_result = NULL;
    work_request = 0;
    work_request_urgency = WORK_FETCH_DONT_NEED;
    duration_correction_factor = 1;
    project_files_downloaded_time = 0;
    use_symlinks = false;

    // Initialize scratch variables.
    rr_sim_status.clear();
    deadlines_missed = 0;
}

/// Return the master URL for this project.
///
/// \return The master URL.
std::string PROJECT::get_master_url() const {
    return master_url;
}

/// Set the master URL for this project.
///
/// \param[in] master_url The new master URL.
void PROJECT::set_master_url(const std::string& master_url) {
    this->master_url = master_url;
}

/// Parse project fields from client_state.xml.
int PROJECT::parse_state(MIOFILE& in) {
    char buf[256];
    std::string sched_url;
    string str1, str2;
    int retval;
    double x;
    bool btemp;

    init();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project>")) {
            if (cpid_time == 0) {
                cpid_time = user_create_time;
            }
            return 0;
        }
        if (parse_str(buf, "<scheduler_url>", sched_url)) {
            scheduler_urls.push_back(sched_url);
            continue;
        }
        if (parse_str(buf, "<master_url>", master_url)) continue;
        if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
        if (parse_str(buf, "<symstore>", symstore, sizeof(symstore))) continue;
        if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
        if (parse_str(buf, "<team_name>", team_name, sizeof(team_name))) continue;
        if (parse_str(buf, "<host_venue>", host_venue, sizeof(host_venue))) continue;
        if (parse_str(buf, "<email_hash>", email_hash, sizeof(email_hash))) continue;
        if (parse_str(buf, "<cross_project_id>", cross_project_id, sizeof(cross_project_id))) continue;
        if (parse_double(buf, "<cpid_time>", cpid_time)) continue;
        if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        if (parse_double(buf, "<user_create_time>", user_create_time)) continue;
        if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        if (parse_int(buf, "<hostid>", hostid)) continue;
        if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        if (parse_double(buf, "<host_create_time>", host_create_time)) continue;
        if (match_tag(buf, "<code_sign_key>")) {
            retval = copy_element_contents(
                in,
                "</code_sign_key>",
                code_sign_key,
                sizeof(code_sign_key)
            );
            if (retval) return retval;
            continue;
        }
        if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        if (parse_double(buf, "<min_rpc_time>", min_rpc_time)) continue;
        if (parse_bool(buf, "master_url_fetch_pending", master_url_fetch_pending)) continue;

        int itmp;
        if (parse_int(buf, "<sched_rpc_pending>", itmp)) {
            sched_rpc_pending = static_cast<rpc_reason>(itmp);
            continue;
        }
        if (parse_double(buf, "<next_rpc_time>", next_rpc_time)) continue;
        if (parse_bool(buf, "trickle_up_pending", trickle_up_pending)) continue;
        if (parse_bool(buf, "send_file_list", send_file_list)) continue;
        if (parse_int(buf, "<send_time_stats_log>", send_time_stats_log)) continue;
        if (parse_int(buf, "<send_job_log>", send_job_log)) continue;
        if (parse_bool(buf, "non_cpu_intensive", non_cpu_intensive)) continue;
        if (parse_bool(buf, "suspended_via_gui", suspended_via_gui)) continue;
        if (parse_bool(buf, "dont_request_more_work", dont_request_more_work)) continue;
        if (parse_bool(buf, "detach_when_done", detach_when_done)) continue;
        if (parse_bool(buf, "ended", ended)) continue;
        if (parse_double(buf, "<short_term_debt>", short_term_debt)) continue;
        if (parse_double(buf, "<long_term_debt>", long_term_debt)) continue;
        if (parse_double(buf, "<resource_share>", x)) continue;    // not authoritative
        if (parse_double(buf, "<duration_correction_factor>", duration_correction_factor)) continue;
        if (parse_bool(buf, "attached_via_acct_mgr", attached_via_acct_mgr)) continue;
        if (parse_double(buf, "<ams_resource_share>", ams_resource_share)) continue;
        if (parse_bool(buf, "scheduler_rpc_in_progress", btemp)) continue;
        if (parse_bool(buf, "use_symlinks", use_symlinks)) continue;
        handle_unparsed_xml_warning("PROJECT::parse_state", buf);
    }
    return ERR_XML_PARSE;
}

/// Write project information to client state file or GUI RPC reply.
///
int PROJECT::write_state(MIOFILE& out, bool gui_rpc) const {
    unsigned int i;
    char un[2048], tn[2048];

    out.printf("<project>\n");

    xml_escape(user_name, un, sizeof(un));
    xml_escape(team_name, tn, sizeof(tn));
    out.printf(
        "    <master_url>%s</master_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <symstore>%s</symstore>\n"
        "    <user_name>%s</user_name>\n"
        "    <team_name>%s</team_name>\n"
        "    <host_venue>%s</host_venue>\n"
        "    <email_hash>%s</email_hash>\n"
        "    <cross_project_id>%s</cross_project_id>\n"
        "    <cpid_time>%f</cpid_time>\n"
        "    <user_total_credit>%f</user_total_credit>\n"
        "    <user_expavg_credit>%f</user_expavg_credit>\n"
        "    <user_create_time>%f</user_create_time>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <hostid>%d</hostid>\n"
        "    <host_total_credit>%f</host_total_credit>\n"
        "    <host_expavg_credit>%f</host_expavg_credit>\n"
        "    <host_create_time>%f</host_create_time>\n"
        "    <nrpc_failures>%d</nrpc_failures>\n"
        "    <master_fetch_failures>%d</master_fetch_failures>\n"
        "    <min_rpc_time>%f</min_rpc_time>\n"
        "    <next_rpc_time>%f</next_rpc_time>\n"
        "    <short_term_debt>%f</short_term_debt>\n"
        "    <long_term_debt>%f</long_term_debt>\n"
        "    <resource_share>%f</resource_share>\n"
        "    <duration_correction_factor>%f</duration_correction_factor>\n"
        "    <sched_rpc_pending>%d</sched_rpc_pending>\n"
        "    <send_time_stats_log>%d</send_time_stats_log>\n"
        "    <send_job_log>%d</send_job_log>\n"
        "%s%s%s%s%s%s%s%s%s%s%s",
        master_url.c_str(),
        project_name,
        symstore,
        un,
        tn,
        host_venue,
        email_hash,
        cross_project_id,
        cpid_time,
        user_total_credit,
        user_expavg_credit,
        user_create_time,
        rpc_seqno,
        hostid,
        host_total_credit,
        host_expavg_credit,
        host_create_time,
        nrpc_failures,
        master_fetch_failures,
        min_rpc_time,
        next_rpc_time,
        short_term_debt,
        long_term_debt,
        resource_share,
        duration_correction_factor,
        sched_rpc_pending,
        send_time_stats_log,
        send_job_log,
        master_url_fetch_pending?"    <master_url_fetch_pending/>\n":"",
        trickle_up_pending?"    <trickle_up_pending/>\n":"",
        send_file_list?"    <send_file_list/>\n":"",
        non_cpu_intensive?"    <non_cpu_intensive/>\n":"",
        suspended_via_gui?"    <suspended_via_gui/>\n":"",
        dont_request_more_work?"    <dont_request_more_work/>\n":"",
        detach_when_done?"    <detach_when_done/>\n":"",
        ended?"    <ended/>\n":"",
        attached_via_acct_mgr?"    <attached_via_acct_mgr/>\n":"",
        (this == gstate.scheduler_op->cur_proj)?"   <scheduler_rpc_in_progress/>\n":"",
        use_symlinks?"    <use_symlinks/>\n":""
    );
    if (ams_resource_share >= 0) {
        out.printf("    <ams_resource_share>%f</ams_resource_share>\n",
            ams_resource_share
        );
    }
    if (gui_rpc) {
        out.printf("%s", gui_urls.c_str());
        out.printf(
            "    <rr_sim_deadlines_missed>%d</rr_sim_deadlines_missed>\n"
            "    <last_rpc_time>%f</last_rpc_time>\n"
            "    <project_files_downloaded_time>%f</project_files_downloaded_time>\n",
            rr_sim_status.get_deadlines_missed(),
            last_rpc_time,
            project_files_downloaded_time
        );
    } else {
       for (i=0; i<scheduler_urls.size(); i++) {
            out.printf(
                "    <scheduler_url>%s</scheduler_url>\n",
                scheduler_urls[i].c_str()
            );
        }
        if (strlen(code_sign_key)) {
            out.printf(
                "    <code_sign_key>\n%s</code_sign_key>\n", code_sign_key
            );
        }
    }
    out.printf(
        "</project>\n"
    );
    return 0;
}

/// Some project data is stored in account file, other in client_state.xml
/// Copy fields that are stored in client_state.xml from "p" into "this".
///
void PROJECT::copy_state_fields(const PROJECT& p) {
    scheduler_urls = p.scheduler_urls;
    safe_strcpy(project_name, p.project_name);
    safe_strcpy(user_name, p.user_name);
    safe_strcpy(team_name, p.team_name);
    safe_strcpy(host_venue, p.host_venue);
    safe_strcpy(email_hash, p.email_hash);
    safe_strcpy(cross_project_id, p.cross_project_id);
    user_total_credit = p.user_total_credit;
    user_expavg_credit = p.user_expavg_credit;
    user_create_time = p.user_create_time;
    cpid_time = p.cpid_time;
    rpc_seqno = p.rpc_seqno;
    hostid = p.hostid;
    host_total_credit = p.host_total_credit;
    host_expavg_credit = p.host_expavg_credit;
    host_create_time = p.host_create_time;
    nrpc_failures = p.nrpc_failures;
    master_fetch_failures = p.master_fetch_failures;
    min_rpc_time = p.min_rpc_time;
    next_rpc_time = p.next_rpc_time;
    master_url_fetch_pending = p.master_url_fetch_pending;
    sched_rpc_pending = p.sched_rpc_pending;
    trickle_up_pending = p.trickle_up_pending;
    safe_strcpy(code_sign_key, p.code_sign_key);
    short_term_debt = p.short_term_debt;
    long_term_debt = p.long_term_debt;
    send_file_list = p.send_file_list;
    send_time_stats_log = p.send_time_stats_log;
    send_job_log = p.send_job_log;
    non_cpu_intensive = p.non_cpu_intensive;
    suspended_via_gui = p.suspended_via_gui;
    dont_request_more_work = p.dont_request_more_work;
    detach_when_done = p.detach_when_done;
    attached_via_acct_mgr = p.attached_via_acct_mgr;
    ended = p.ended;
    duration_correction_factor = p.duration_correction_factor;
    ams_resource_share = p.ams_resource_share;
    if (ams_resource_share > 0) {
        resource_share = ams_resource_share;
    }
    use_symlinks = p.use_symlinks;
}

/// Write project statistic to project statistics file.
///
int PROJECT::write_statistics(MIOFILE& out, bool /*gui_rpc*/) const {
    out.printf(
        "<project_statistics>\n"
        "    <master_url>%s</master_url>\n",
        master_url.c_str()
    );

    for (std::vector<DAILY_STATS>::const_iterator i=statistics.begin();
        i!=statistics.end(); ++i
    ) {
        out.printf(
            "    <daily_statistics>\n"
            "        <day>%f</day>\n"
            "        <user_total_credit>%f</user_total_credit>\n"
            "        <user_expavg_credit>%f</user_expavg_credit>\n"
            "        <host_total_credit>%f</host_total_credit>\n"
            "        <host_expavg_credit>%f</host_expavg_credit>\n"
            "    </daily_statistics>\n",
            i->day,
            i->user_total_credit,
            i->user_expavg_credit,
            i->host_total_credit,
            i->host_expavg_credit
        );
    }
    out.printf(
        "</project_statistics>\n"
    );
    return 0;
}

const char* PROJECT::get_project_name() const {
    if (strlen(project_name)) {
        return project_name;
    } else {
        return master_url.c_str();
    }
}

const char* PROJECT::get_scheduler_url(int index, double r) const {
    int n = (int) scheduler_urls.size();
    int ir = (int)(r*n);
    int i = (index + ir)%n;
    return scheduler_urls[i].c_str();
}

double PROJECT::next_file_xfer_time(const bool is_upload) const {
    return (is_upload ? next_file_xfer_up : next_file_xfer_down);
}

void PROJECT::file_xfer_failed(const bool is_upload) {
    if (is_upload) {
        file_xfer_failures_up++;
        if (file_xfer_failures_up < FILE_XFER_FAILURE_LIMIT) {
            next_file_xfer_up = 0;
        } else {
            next_file_xfer_up = gstate.now + calculate_exponential_backoff(
                file_xfer_failures_up,
                gstate.pers_retry_delay_min,
                gstate.pers_retry_delay_max
            );
        }
    } else {
        file_xfer_failures_down++;
        if (file_xfer_failures_down < FILE_XFER_FAILURE_LIMIT) {
            next_file_xfer_down = 0;
        } else {
            next_file_xfer_down = gstate.now + calculate_exponential_backoff(
                file_xfer_failures_down,
                gstate.pers_retry_delay_min,
                gstate.pers_retry_delay_max
            );
        }
    }
}

void PROJECT::file_xfer_succeeded(const bool is_upload) {
    if (is_upload) {
        file_xfer_failures_up = 0;
        next_file_xfer_up  = 0;
    } else {
        file_xfer_failures_down = 0;
        next_file_xfer_down = 0;
    }
}

/// Parse project files from a xml file.
///
/// \param[in,out] in Reference to a MIOFILE instance from which the file
///                   contents should be read.
/// \param[in] delete_existing_symlinks If true existing symlinks will be
///                                     deleted before parsing \a in.
/// \return Zero on success, ERR_XML_PARSE otherwise.
int PROJECT::parse_project_files(MIOFILE& in, bool delete_existing_symlinks) {
    if (delete_existing_symlinks) {
        // Delete current sym links.
        // This is done when parsing scheduler reply,
        // to ensure that we get rid of sym links for
        // project files no longer in use
        std::string project_dir = get_project_dir(this);
        for (FILE_REF_VEC::const_iterator it = project_files.begin(); it != project_files.end(); ++it) {
            std::string path(project_dir);
            path.append("/").append((*it).open_name);
            delete_project_owned_file(path.c_str(), false);
        }
    }

    project_files.clear();
    char buf[256];
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project_files>")) {
            return 0;
        }
        if (match_tag(buf, "<file_ref>")) {
            FILE_REF file_ref;
            file_ref.parse(in);
            project_files.push_back(file_ref);
        } else {
            handle_unparsed_xml_warning("PROJECT::parse_project_files", buf);
        }
    }
    return ERR_XML_PARSE;
}

/// Install pointers from FILE_REFs to FILE_INFOs for project files,
/// and flag FILE_INFOs as being project files.
///
/// \param[in] recreate_symlink_files If true symlinks for the files
///                                   belonging to this project will be
///                                   recreated.
void PROJECT::link_project_files(bool recreate_symlink_files) {
    FILE_REF_VEC::iterator fref_iter = project_files.begin();
    while (fref_iter != project_files.end()) {
        FILE_REF& fref = *fref_iter;
        FILE_INFO* fip = gstate.lookup_file_info(this, fref.file_name);
        if (!fip) {
            msg_printf(this, MSG_INTERNAL_ERROR, "project file refers to non-existent %s", fref.file_name);
            fref_iter = project_files.erase(fref_iter);
            continue;
        }
        fref.file_info = fip;
        fip->is_project_file = true;
        ++fref_iter;
    }

    if (recreate_symlink_files) {
        for (std::vector<FILE_INFO*>::const_iterator it = gstate.file_infos.begin(); it != gstate.file_infos.end(); ++it) {
            if (((*it)->project == this) && ((*it)->is_project_file) && ((*it)->status == FILE_PRESENT)) {
                write_symlink_for_project_file(*it);
            }
        }
    }
}

/// Write the XML representation of the project files into a file.
///
/// \param[in] out Reference to a MIOFILE instance representing the target file.
void PROJECT::write_project_files(MIOFILE& out) const {
    if (project_files.empty()) {
        return;
    }
    out.printf("<project_files>\n");
    for (FILE_REF_VEC::const_iterator it = project_files.begin(); it != project_files.end(); ++it) {
        (*it).write(out);
    }
    out.printf("</project_files>\n");
}

/// Write symlinks for project files.
/// Note: it's conceivable that one physical file
/// has several logical names, so try them all.
///
/// \param[in] fip A pointer to a file info instance for which the symbolic
///                links should be created.
/// \return Always returns zero.
int PROJECT::write_symlink_for_project_file(const FILE_INFO* fip) const {
    std::string project_dir = get_project_dir(this);

    for (FILE_REF_VEC::const_iterator it = project_files.begin(); it != project_files.end(); ++it) {
        if ((*it).file_info == fip) {
            std::string path(project_dir);
            path.append("/").append((*it).open_name);
            FILE* f = boinc_fopen(path.c_str(), "w");
            if (f) {
                fprintf(f, "<soft_link>%s/%s</soft_link>\n", project_dir.c_str(), fip->name.c_str());
                fclose(f);
            }
        }
    }
    return 0;
}

/// Update project_files_downloaded_time to the current time.
/// This is called when a project file download finishes.
void PROJECT::update_project_files_downloaded_time() {
    project_files_downloaded_time = gstate.now;
}

/// Get all workunits for this project.
///
/// \return A vector containing all workunits of this project.
WORKUNIT_PVEC PROJECT::get_workunits() const {
    WORKUNIT_PVEC result;
    for (WORKUNIT_PVEC::const_iterator it = gstate.workunits.begin(); it != gstate.workunits.end(); ++it) {
        if ((*it)->project == this) {
            result.push_back(*it);
        }
    }
    return result;
}

int APP::parse(MIOFILE& in) {
    char buf[256];

    strcpy(name, "");
    strcpy(user_friendly_name, "");
    project = NULL;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app>")) {
            if (!strlen(user_friendly_name)) {
                strcpy(user_friendly_name, name);
            }
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<user_friendly_name>", user_friendly_name, sizeof(user_friendly_name))) continue;
        handle_unparsed_xml_warning("APP::parse", buf);
    }
    return ERR_XML_PARSE;
}

int APP::write(MIOFILE& out) const {
    out.printf(
        "<app>\n"
        "    <name>%s</name>\n"
        "    <user_friendly_name>%s</user_friendly_name>\n"
        "</app>\n",
        name, user_friendly_name
    );
    return 0;
}

FILE_INFO::FILE_INFO() {
    strcpy(md5_cksum, "");
    max_nbytes = 0;
    nbytes = 0;
    upload_offset = -1;
    generated_locally = false;
    status = FILE_NOT_PRESENT;
    executable = false;
    uploaded = false;
    upload_when_present = false;
    sticky = false;
    marked_for_delete = false;
    report_on_rpc = false;
    gzip_when_done = false;
    signature_required = false;
    is_user_file = false;
    is_project_file = false;
    pers_file_xfer = NULL;
    result = NULL;
    project = NULL;
    urls.clear();
    start_url = -1;
    current_url = -1;
}

FILE_INFO::~FILE_INFO() {
    if (pers_file_xfer) {
        msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Deleting file %s while in use", name.c_str());
        pers_file_xfer->fip = NULL;
    }
}

void FILE_INFO::reset() {
    status = FILE_NOT_PRESENT;
    delete_file();
    error_msg = "";
}

/// Set the appropriate permissions depending on whether
/// it's an executable file.
/// This doesn't seem to exist in Windows.
///
int FILE_INFO::set_permissions() {
#ifdef _WIN32
    return 0;
#else
    int retval;
    std::string pathname = get_pathname(this);

    if (g_use_sandbox) {
        // give exec permissions for user, group and others but give
        // read permissions only for user and group to protect account keys
        retval = set_to_project_group(pathname.c_str());
        if (retval) return retval;
        if (executable) {
            retval = chmod(pathname.c_str(),
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IXOTH
            );
        } else {
            retval = chmod(pathname.c_str(),
                S_IRUSR|S_IWUSR
                |S_IRGRP|S_IWGRP
            );
        }
    } else {
        // give read/exec permissions for user, group and others
        // in case someone runs Synecdoche from different user
        if (executable) {
            retval = chmod(pathname.c_str(),
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
        } else {
            retval = chmod(pathname.c_str(),
                S_IRUSR|S_IWUSR
                |S_IRGRP
                |S_IROTH
            );
        }
    }
    return retval;
#endif
}

/// If from server, make an exact copy of everything
/// except the start/end tags and the <xml_signature> element.
///
int FILE_INFO::parse(MIOFILE& in, bool from_server) {
    char buf[256], buf2[1024];
    std::string url;
    PERS_FILE_XFER *pfxp;
    int retval;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_info>")) {
            if (name.empty()) return ERR_BAD_FILENAME;
            if (name.find("..") != std::string::npos) return ERR_BAD_FILENAME;
            if (name.find("%") != std::string::npos) return ERR_BAD_FILENAME;
            return 0;
        }
        if (match_tag(buf, "<xml_signature>")) {
            retval = copy_element_contents(in, "</xml_signature>", xml_signature);
            if (retval) {
                return retval;
            }
            continue;
        }
        if (match_tag(buf, "<file_signature>")) {
            retval = copy_element_contents(in, "</file_signature>", file_signature);
            if (retval) {
                return retval;
            }
            if (from_server) {
                signed_xml += "<file_signature>\n";
                signed_xml += file_signature;
                signed_xml += "</file_signature>\n";
            }
            continue;
        }
        signed_xml += buf;
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<url>", url)) {
            urls.push_back(url);
            continue;
        }
        if (parse_str(buf, "<md5_cksum>", md5_cksum, sizeof(md5_cksum))) continue;
        if (parse_double(buf, "<nbytes>", nbytes)) continue;
        if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        if (parse_bool(buf, "generated_locally", generated_locally)) continue;
        if (parse_int(buf, "<status>", status)) continue;
        if (parse_bool(buf, "executable", executable)) continue;
        if (parse_bool(buf, "uploaded", uploaded)) continue;
        if (parse_bool(buf, "upload_when_present", upload_when_present)) continue;
        if (parse_bool(buf, "sticky", sticky)) continue;
        if (parse_bool(buf, "marked_for_delete", marked_for_delete)) continue;
        if (parse_bool(buf, "report_on_rpc", report_on_rpc)) continue;
        if (parse_bool(buf, "gzip_when_done", gzip_when_done)) continue;
        if (parse_bool(buf, "signature_required", signature_required)) continue;
        if (parse_bool(buf, "is_project_file", is_project_file)) continue;
        if (match_tag(buf, "<no_delete")) continue;
        if (match_tag(buf, "<persistent_file_xfer>")) {
            pfxp = new PERS_FILE_XFER;
            retval = pfxp->parse(in);
            if (!retval) {
                pers_file_xfer = pfxp;
            } else {
                delete pfxp;
            }
            continue;
        }
        if (!from_server && match_tag(buf, "<signed_xml>")) {
            retval = copy_element_contents(in, "</signed_xml>", signed_xml);
            if (retval) {
                return retval;
            }
            continue;
        }
        if (match_tag(buf, "<file_xfer>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</file_xfer>")) break;
            }
            continue;
        }
        if (match_tag(buf, "<error_msg>")) {
            retval = copy_element_contents(
                in, "</error_msg>", buf2, sizeof(buf2)
            );
            if (retval) return retval;
            error_msg = buf2;
            continue;
        }
        handle_unparsed_xml_warning("FILE_INFO::parse", buf);
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::write(MIOFILE& out, bool to_server) const {
    unsigned int i;
    int retval;

    out.printf(
        "<file_info>\n"
        "    <name>%s</name>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n",
        name.c_str(), nbytes, max_nbytes);

    if (strlen(md5_cksum)) {
        out.printf("    <md5_cksum>%s</md5_cksum>\n", md5_cksum);
    }
    if (!to_server) {
        if (generated_locally) out.printf("    <generated_locally/>\n");
        out.printf("    <status>%d</status>\n", status);
        if (executable) out.printf("    <executable/>\n");
        if (uploaded) out.printf("    <uploaded/>\n");
        if (upload_when_present) out.printf("    <upload_when_present/>\n");
        if (sticky) out.printf("    <sticky/>\n");
        if (marked_for_delete) out.printf("    <marked_for_delete/>\n");
        if (report_on_rpc) out.printf("    <report_on_rpc/>\n");
        if (gzip_when_done) out.printf("    <gzip_when_done/>\n");
        if (signature_required) out.printf("    <signature_required/>\n");
        if (is_user_file) out.printf("    <is_user_file/>\n");
        if (!file_signature.empty()) out.printf("    <file_signature>\n%s</file_signature>\n", file_signature.c_str());
    }
    for (i=0; i<urls.size(); i++) {
        out.printf("    <url>%s</url>\n", urls[i].c_str());
    }
    if (!to_server && pers_file_xfer) {
        retval = pers_file_xfer->write(out);
        if (retval) return retval;
    }
    if (!to_server) {
        if ((!signed_xml.empty()) && (!xml_signature.empty())) {
            out.printf(
                "    <signed_xml>\n%s    </signed_xml>\n"
                "    <xml_signature>\n%s    </xml_signature>\n",
                signed_xml.c_str(), xml_signature.c_str());
        }
    }
    if (!error_msg.empty()) {
        std::string error_msg_nows = error_msg;
        strip_whitespace(error_msg_nows);
        out.printf("    <error_msg>\n%s\n</error_msg>\n", error_msg_nows.c_str());
    }
    out.printf("</file_info>\n");
    return 0;
}

int FILE_INFO::write_gui(MIOFILE& out) const {
    out.printf(
        "<file_transfer>\n"
        "    <project_url>%s</project_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <name>%s</name>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n"
        "    <status>%d</status>\n",
        project->get_master_url().c_str(), project->project_name, name.c_str(),
        nbytes, max_nbytes, status);

    if (generated_locally) out.printf("    <generated_locally/>\n");
    if (uploaded) out.printf("    <uploaded/>\n");
    if (upload_when_present) out.printf("    <upload_when_present/>\n");
    if (sticky) out.printf("    <sticky/>\n");
    if (marked_for_delete) out.printf("    <marked_for_delete/>\n");

    if (pers_file_xfer) {
        pers_file_xfer->write(out);
    }
    out.printf("</file_transfer>\n");
    return 0;
}

/// Delete physical underlying file associated with FILE_INFO.
int FILE_INFO::delete_file() {
    std::string path = get_pathname(this);
    int retval = delete_project_owned_file(path.c_str(), true);
    if (retval && status != FILE_NOT_PRESENT) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Couldn't delete file %s", path.c_str());
    }
    status = FILE_NOT_PRESENT;
    return retval;
}

/// Files may have URLs for both upload and download.
/// Call this to get the initial url,
/// The is_upload arg says which kind you want.
/// NULL return means there is no URL of the requested type.
///
/// \todo If there are multiple URLs, this function tries them in order. There
/// is commented out code that picks them randomly. There should be a way for a
/// project to choose what mode to use.
const char* FILE_INFO::get_init_url(bool is_upload) {
    if (urls.empty()) {
        return NULL;
    }

    /// if a project supplies multiple URLs, try them in order
    /// (e.g. in Einstein@home they're ordered by proximity to client).
    /// The commented-out code tries them starting from random place.
    /// This is appropriate if replication is for load-balancing.
    /// TODO: add a flag saying which mode to use.
#if 1
    current_url = 0;
#else
    double temp;
    temp = rand();
    temp *= urls.size();
    temp /= RAND_MAX;
    current_url = (int)temp;
#endif
    start_url = current_url;
    while (true) {
        if (!is_correct_url_type(is_upload, urls[current_url])) {
            current_url = (current_url + 1) % ((int)urls.size());
            if (current_url == start_url) {
                msg_printf(project, MSG_INTERNAL_ERROR, "Couldn't find suitable URL for %s", name.c_str());
                return NULL;
            }
        } else {
            start_url = current_url;
            return urls[current_url].c_str();
        }
    }
}

/// Call this to get the next URL of the indicated type.
/// NULL return means you've tried them all.
const char* FILE_INFO::get_next_url(bool is_upload) {
    if (urls.empty()) return NULL;
    while (true) {
        current_url = (current_url + 1) % ((int)urls.size());
        if (current_url == start_url) {
            return NULL;
        }
        if (is_correct_url_type(is_upload, urls[current_url])) {
            return urls[current_url].c_str();
        }
    }
}

const char* FILE_INFO::get_current_url(bool is_upload) {
    if (current_url < 0) {
        return get_init_url(is_upload);
    }
    if (current_url >= (int)urls.size()) {
        msg_printf(project, MSG_INTERNAL_ERROR, "File %s has no URL", name.c_str());
        return NULL;
    }
    return urls[current_url].c_str();
}

/// Checks if the URL includes the phrase "file_upload_handler".
/// This indicates the URL is an upload url.
bool FILE_INFO::is_correct_url_type(bool is_upload, const std::string& url) const {
    const char* has_str = strstr(url.c_str(), "file_upload_handler");
    if ((is_upload && !has_str) || (!is_upload && has_str)) {
        return false;
    } else {
        return true;
    }
}

/// Merges information from a new FILE_INFO that has the same name as one
/// that is already present in the client state file.
/// Potentially changes upload_when_present, max_nbytes, and signed_xml.
int FILE_INFO::merge_info(const FILE_INFO& new_info) {
    char buf[256];
    unsigned int i;

    upload_when_present = new_info.upload_when_present;

    if (max_nbytes <= 0 && new_info.max_nbytes) {
        max_nbytes = new_info.max_nbytes;
        sprintf(buf, "    <max_nbytes>%.0f</max_nbytes>\n", new_info.max_nbytes);
        signed_xml += buf;
    }

    // replace existing URLs with new ones
    urls.clear();
    for (i=0; i<new_info.urls.size(); i++) {
        urls.push_back(new_info.urls[i]);
    }

    // replace signature
    if (!new_info.file_signature.empty()) {
        file_signature = new_info.file_signature;
    }

    return 0;
}

/// Returns true if the file had an unrecoverable error
/// (couldn't download, RSA/MD5 check failed, etc).
bool FILE_INFO::had_failure(int& failnum) const {
    if (status != FILE_NOT_PRESENT
                && status != FILE_PRESENT
                && status != FILE_NOT_PRESENT_NOT_NEEDED) {
        failnum = status;
        return true;
    }
    return false;
}

/// Create a failure message for a failed file-xfer in XML format.
///
/// \return A string containing error information in XML format.
std::string FILE_INFO::failure_message() const {
    std::ostringstream buf;
    buf << "<file_xfer_error>\n"
        << "  <file_name>" << name << "</file_name>\n"
        << "  <error_code>" << status << "</error_code>\n";
    if (!error_msg.empty()) {
        buf << "  <error_message>" << error_msg << "</error_message>\n";
    }
    buf << "</file_xfer_error>\n";
    return buf.str();
}

/// Compress the file using zlib (gzip compression).
///
/// \return Zero on success, ERR_FOPEN or ERR_WRITE on error.
/// \todo Replace this with a gzip streambuf? (btw, there is one in boost::iostream)
int FILE_INFO::gzip() {
    const size_t BUFSIZE = 16384;
    char buf[BUFSIZE];

    std::string inpath = get_pathname(this);
    std::string outpath(inpath);
    outpath.append(".gz");
    FILE* in = boinc_fopen(inpath.c_str(), "rb");
    if (!in) {
        return ERR_FOPEN;
    }

    gzFile out = gzopen(outpath.c_str(), "wb");
    while (1) {
        int n = (int)fread(buf, 1, BUFSIZE, in);
        if (n <= 0) {
            break;
        }
        int m = gzwrite(out, buf, n);
        if (m != n) {
            fclose(in);
            gzclose(out);
            return ERR_WRITE;
        }
    }
    fclose(in);
    gzclose(out);
    delete_project_owned_file(inpath.c_str(), true);
    boinc_rename(outpath.c_str(), inpath.c_str());
    return 0;
}

int APP_VERSION::parse(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    strcpy(app_name, "");
    strcpy(api_version, "");
    version_num = 0;
    strcpy(platform, "");
    strcpy(plan_class, "");
    strcpy(cmdline, "");
    avg_ncpus = 1;
    max_ncpus = 1;
    app = NULL;
    project = NULL;
    flops = gstate.host_info.p_fpops;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app_version>")) return 0;
        if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            app_files.push_back(file_ref);
            continue;
        }
        if (parse_int(buf, "<version_num>", version_num)) continue;
        if (parse_str(buf, "<api_version>", api_version, sizeof(api_version))) continue;
        if (parse_str(buf, "<platform>", platform, sizeof(platform))) continue;
        if (parse_str(buf, "<plan_class>", plan_class, sizeof(plan_class))) continue;
        if (parse_double(buf, "<avg_ncpus>", avg_ncpus)) continue;
        if (parse_double(buf, "<max_ncpus>", max_ncpus)) continue;
        if (parse_double(buf, "<flops>", flops)) continue;
        if (parse_str(buf, "<cmdline>", cmdline, sizeof(cmdline))) continue;
        handle_unparsed_xml_warning("APP_VERSION::parse", buf);
    }
    return ERR_XML_PARSE;
}

int APP_VERSION::write(MIOFILE& out) const {
    unsigned int i;
    int retval;

    out.printf(
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        "    <platform>%s</platform>\n"
        "    <avg_ncpus>%f</avg_ncpus>\n"
        "    <max_ncpus>%f</max_ncpus>\n"
        "    <flops>%f</flops>\n",
        app_name,
        version_num,
        platform,
        avg_ncpus,
        max_ncpus,
        flops
    );
    if (strlen(plan_class)) {
        out.printf("    <plan_class>%s</plan_class>\n", plan_class);
    }
    if (strlen(api_version)) {
        out.printf("    <api_version>%s</api_version>\n", api_version);
    }
    if (strlen(cmdline)) {
        out.printf("    <cmdline>%s</cmdline>\n", cmdline);
    }
    for (i=0; i<app_files.size(); i++) {
        retval = app_files[i].write(out);
        if (retval) return retval;
    }

    out.printf(
        "</app_version>\n"
    );
    return 0;
}

bool APP_VERSION::had_download_failure(int& failnum) const {
    unsigned int i;

    for (i=0; i<app_files.size();i++) {
        if (app_files[i].file_info->had_failure(failnum)) {
            return true;
        }
    }
    return false;
}

void APP_VERSION::get_file_errors(std::string& str) {
    int errnum;
    unsigned int i;
    const FILE_INFO* fip;

    str = "couldn't get input files:\n";
    for (i=0; i<app_files.size();i++) {
        fip = app_files[i].file_info;
        if (fip->had_failure(errnum)) {
            str += fip->failure_message();
        }
    }
}

void APP_VERSION::clear_errors() {
    int x;
    unsigned int i;
    for (i=0; i<app_files.size();i++) {
        FILE_INFO* fip = app_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->reset();
        }
    }
}

int APP_VERSION::api_major_version() const {
    int v, n;
    n = sscanf(api_version, "%d", &v);
    if (n != 1) return 0;
    return v;
}

int FILE_REF::parse(MIOFILE& in) {
    char buf[256];
    bool temp;

    strcpy(file_name, "");
    strcpy(open_name, "");
    main_program = false;
    copy_file = false;
    optional = false;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_ref>")) return 0;
        if (parse_str(buf, "<file_name>", file_name, sizeof(file_name))) continue;
        if (parse_str(buf, "<open_name>", open_name, sizeof(open_name))) continue;
        if (parse_bool(buf, "main_program", main_program)) continue;
        if (parse_bool(buf, "copy_file", copy_file)) continue;
        if (parse_bool(buf, "optional", optional)) continue;
        if (parse_bool(buf, "no_validate", temp)) continue;
        handle_unparsed_xml_warning("FILE_REF::parse", buf);
    }
    return ERR_XML_PARSE;
}

int FILE_REF::write(MIOFILE& out) const {

    out.printf(
        "    <file_ref>\n"
        "        <file_name>%s</file_name>\n",
        file_name
    );
    if (strlen(open_name)) {
        out.printf("        <open_name>%s</open_name>\n", open_name);
    }
    if (main_program) {
        out.printf("        <main_program/>\n");
    }
    if (copy_file) {
        out.printf("        <copy_file/>\n");
    }
    if (optional) {
        out.printf("        <optional/>\n");
    }
    out.printf("    </file_ref>\n");
    return 0;
}

int WORKUNIT::parse(MIOFILE& in) {
    char buf[4096];
    FILE_REF file_ref;
    double dtemp;

    strcpy(name, "");
    strcpy(app_name, "");
    version_num = 0;
    command_line = "";
    //strcpy(env_vars, "");
    app = NULL;
    project = NULL;
    // Default these to very large values (1 week on a 1 cobblestone machine)
    // so we don't keep asking the server for more work
    rsc_fpops_est = 1e9*SECONDS_PER_DAY*7;
    rsc_fpops_bound = 4e9*SECONDS_PER_DAY*7;
    rsc_memory_bound = 1e8;
    rsc_disk_bound = 1e9;
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</workunit>")) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
        if (match_tag(buf, "<command_line>")) {
            if (strstr(buf, "</command_line>")) {
                parse_str(buf, "<command_line>", command_line);
            } else {
                bool found=false;
                while (in.fgets(buf, sizeof(buf))) {
                    if (strstr(buf, "</command_line")) {
                        found = true;
                        break;
                    }
                    command_line += buf;
                }
                if (!found) {
                    msg_printf(NULL, MSG_INTERNAL_ERROR,
                        "Task %s: bad command line",
                        name
                    );
                    return ERR_XML_PARSE;
                }
            }
            strip_whitespace(command_line);
            continue;
        }
        //if (parse_str(buf, "<env_vars>", env_vars, sizeof(env_vars))) continue;
        if (parse_double(buf, "<rsc_fpops_est>", rsc_fpops_est)) continue;
        if (parse_double(buf, "<rsc_fpops_bound>", rsc_fpops_bound)) continue;
        if (parse_double(buf, "<rsc_memory_bound>", rsc_memory_bound)) continue;
        if (parse_double(buf, "<rsc_disk_bound>", rsc_disk_bound)) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            input_files.push_back(file_ref);
            continue;
        }
        // unused stuff
        if (parse_double(buf, "<credit>", dtemp)) continue;
        handle_unparsed_xml_warning("WORKUNIT::parse", buf);
    }
    return ERR_XML_PARSE;
}

int WORKUNIT::write(MIOFILE& out) const {
    unsigned int i;

    out.printf(
        "<workunit>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        //"    <env_vars>%s</env_vars>\n"
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n",
        name,
        app_name,
        version_num,
        //env_vars,
        rsc_fpops_est,
        rsc_fpops_bound,
        rsc_memory_bound,
        rsc_disk_bound
    );
    if (!command_line.empty()) {
        out.printf(
            "    <command_line>\n"
            "%s\n"
            "    </command_line>\n",
            command_line.c_str()
        );
    }
    for (i=0; i<input_files.size(); i++) {
        input_files[i].write(out);
    }
    out.printf("</workunit>\n");
    return 0;
}

bool WORKUNIT::had_download_failure(int& failnum) const {
    unsigned int i;

    for (i=0;i<input_files.size();i++) {
        if (input_files[i].file_info->had_failure(failnum)) {
            return true;
        }
    }
    return false;
}

void WORKUNIT::get_file_errors(std::string& str) const {
    int x;
    unsigned int i;
    const FILE_INFO* fip;

    str = "couldn't get input files:\n";
    for (i=0;i<input_files.size();i++) {
        fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            str += fip->failure_message();
        }
    }
}

/// If any input files had download error from previous WU,
/// reset them to try download again.
///
void WORKUNIT::clear_errors() {
    int x;
    unsigned int i;
    for (i=0; i<input_files.size();i++) {
        FILE_INFO* fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->reset();
        }
    }
}

int RESULT::parse_name(FILE* in, const char* end_tag) {
    char buf[256];

    strcpy(name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, end_tag)) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        handle_unparsed_xml_warning("RESULT::parse_name", buf);
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    completed_time = 0.0;
    report_deadline = 0;
    output_files.clear();
    _state = RESULT_NEW;
    ready_to_report = false;
    completed_time = 0;
    got_server_ack = false;
    final_cpu_time = 0;
    exit_status = 0;
    stderr_out = "";
    suspended_via_gui = false;
    rr_sim_misses_deadline = false;
    last_rr_sim_missed_deadline = false;
    fpops_per_cpu_sec = 0;
    fpops_cumulative = 0;
    intops_per_cpu_sec = 0;
    intops_cumulative = 0;
    app = NULL;
    wup = NULL;
    project = NULL;
    version_num = 0;
    strcpy(platform, "");
    strcpy(plan_class, "");
}

/// Parse a <result> element from scheduling server.
///
int RESULT::parse_server(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_double(buf, "<report_deadline>", report_deadline)) continue;
        if (parse_str(buf, "<platform>", platform, sizeof(platform))) continue;
        if (parse_str(buf, "<plan_class>", plan_class, sizeof(plan_class))) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        handle_unparsed_xml_warning("RESULT::parse_server", buf);
    }
    return ERR_XML_PARSE;
}

/// parse a <result> element from state file
///
int RESULT::parse_state(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) {
            // set state to something reasonable in case of bad state file
            //
            if (got_server_ack || ready_to_report) {
                switch (state()) {
                case RESULT_NEW:
                case RESULT_FILES_DOWNLOADING:
                case RESULT_FILES_DOWNLOADED:
                case RESULT_FILES_UPLOADING:
                    set_state(RESULT_FILES_UPLOADED, "RESULT::parse_state");
                    break;
                }
            }
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_double(buf, "<received_time>", received_time)) continue;
        if (parse_double(buf, "<report_deadline>", report_deadline)) {
            continue;
        }
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        if (parse_int(buf, "<exit_status>", exit_status)) continue;
        if (parse_bool(buf, "got_server_ack", got_server_ack)) continue;
        if (parse_bool(buf, "ready_to_report", ready_to_report)) continue;
        if (parse_double(buf, "<completed_time>", completed_time)) continue;
        if (parse_bool(buf, "suspended_via_gui", suspended_via_gui)) continue;
        if (parse_int(buf, "<state>", _state)) continue;
        if (match_tag(buf, "<stderr_out>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</stderr_out>")) break;
                if (strstr(buf, "<![CDATA[")) continue;
                if (strstr(buf, "]]>")) continue;
                stderr_out.append(buf);
            }
            continue;
        }
        if (parse_double(buf, "<fpops_per_cpu_sec>", fpops_per_cpu_sec)) continue;
        if (parse_double(buf, "<fpops_cumulative>", fpops_cumulative)) continue;
        if (parse_double(buf, "<intops_per_cpu_sec>", intops_per_cpu_sec)) continue;
        if (parse_double(buf, "<intops_cumulative>", intops_cumulative)) continue;
        if (parse_str(buf, "<platform>", platform, sizeof(platform))) continue;
        if (parse_str(buf, "<plan_class>", plan_class, sizeof(plan_class))) continue;
        if (parse_int(buf, "<version_num>", version_num)) continue;
        handle_unparsed_xml_warning("RESULT::parse", buf);
    }
    return ERR_XML_PARSE;
}

int RESULT::write(MIOFILE& out, bool to_server) const {
    unsigned int i;
    const FILE_INFO* fip;
    int n, retval;

    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <platform>%s</platform>\n"
        "    <version_num>%d</version_num>\n",
        name,
        final_cpu_time,
        exit_status,
        state(),
        platform,
        version_num
    );
    if (strlen(plan_class)) {
        out.printf("    <plan_class>%s</plan_class>\n", plan_class);
    }
    if (fpops_per_cpu_sec) {
        out.printf("    <fpops_per_cpu_sec>%f</fpops_per_cpu_sec>\n", fpops_per_cpu_sec);
    }
    if (fpops_cumulative) {
        out.printf("    <fpops_cumulative>%f</fpops_cumulative>\n", fpops_cumulative);
    }
    if (intops_per_cpu_sec) {
        out.printf("    <intops_per_cpu_sec>%f</intops_per_cpu_sec>\n", intops_per_cpu_sec);
    }
    if (intops_cumulative) {
        out.printf("    <intops_cumulative>%f</intops_cumulative>\n", intops_cumulative);
    }
    if (to_server) {
        out.printf(
            "    <app_version_num>%d</app_version_num>\n",
            wup->version_num
        );
    }
    n = (int)stderr_out.length();
    if (n || to_server) {
        out.printf("<stderr_out>\n");

        // the following is here so that it gets recorded on server
        // (there's no core_client_version field of result table)
        //
        if (to_server) {
            out.printf(
                "<core_client_version>%d.%d.%d</core_client_version>\n",
                gstate.core_client_version.major,
                gstate.core_client_version.minor,
                gstate.core_client_version.release
            );
        }
        if (n) {
            out.printf("<![CDATA[\n");
            out.printf("%s",stderr_out.c_str());
            if (stderr_out[n-1] != '\n') {
                out.printf("\n");
            }
            out.printf("]]>\n");
        }
        out.printf("</stderr_out>\n");
    }
    if (to_server) {
        for (i=0; i<output_files.size(); i++) {
            fip = output_files[i].file_info;
            if (fip->uploaded) {
                retval = fip->write(out, true);
                if (retval) return retval;
            }
        }
    } else {
        if (got_server_ack) out.printf("    <got_server_ack/>\n");
        if (ready_to_report) out.printf("    <ready_to_report/>\n");
        if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
        if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
        out.printf(
            "    <wu_name>%s</wu_name>\n"
            "    <received_time>%f</received_time>\n"
            "    <report_deadline>%f</report_deadline>\n",
            wu_name,
            received_time,
            report_deadline
        );
        for (i=0; i<output_files.size(); i++) {
            retval = output_files[i].write(out);
            if (retval) return retval;
        }
    }
    out.printf("</result>\n");
    return 0;
}

int RESULT::write_gui(MIOFILE& out) const {
    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <wu_name>%s</wu_name>\n"
        "    <project_url>%s</project_url>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n"
        "    <received_time>%f</received_time>\n"
        "    <report_deadline>%f</report_deadline>\n"
        "    <estimated_cpu_time_remaining>%f</estimated_cpu_time_remaining>\n",
        name,
        wu_name,
        project->get_master_url().c_str(),
        final_cpu_time,
        exit_status,
        state(),
        received_time,
        report_deadline,
        estimated_cpu_time_remaining()
    );
    if (got_server_ack) out.printf("    <got_server_ack/>\n");
    if (ready_to_report) out.printf("    <ready_to_report/>\n");
    if (completed_time) out.printf("    <completed_time>%f</completed_time>\n", completed_time);
    if (suspended_via_gui) out.printf("    <suspended_via_gui/>\n");
    if (project->suspended_via_gui) out.printf("    <project_suspended_via_gui/>\n");
    if (edf_scheduled) out.printf("    <edf_scheduled/>\n");
    const ACTIVE_TASK* atp = gstate.active_tasks.lookup_result(this);
    if (atp) {
        atp->write_gui(out);
    }
    out.printf("</result>\n");
    return 0;
}

/// Returns true if the result's output files are all either
/// successfully uploaded or have unrecoverable errors.
///
bool RESULT::is_upload_done() const {
    unsigned int i;
    const FILE_INFO* fip;
    int retval;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->upload_when_present) {
            if (fip->had_failure(retval)) continue;
            if (!fip->uploaded) {
                return false;
            }
        }
    }
    return true;
}

/// Resets all FILE_INFO's in result to uploaded = false
/// if upload_when_present is true.
///
void RESULT::clear_uploaded_flags() {
    unsigned int i;
    FILE_INFO* fip;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->upload_when_present) {
            fip->uploaded = false;
        }
    }
}

bool PROJECT::some_download_stalled() const {
    unsigned int i;
    for (i=0; i<gstate.pers_file_xfers->pers_file_xfers.size(); i++) {
        const PERS_FILE_XFER* pfx = gstate.pers_file_xfers->pers_file_xfers[i];
        if (pfx->fip->project != this) continue;
        if (pfx->is_upload) continue;
        if (pfx->next_request_time > gstate.now) return true;
    }
    return false;
}

/// Return true if some file needed by this result (input or application)
/// is downloading and backed off.
bool RESULT::some_download_stalled() const {
    unsigned int i;
    const FILE_INFO* fip;
    const PERS_FILE_XFER* pfx;

    for (i=0; i<wup->input_files.size(); i++) {
        fip = wup->input_files[i].file_info;
        pfx = fip->pers_file_xfer;
        if (pfx && pfx->next_request_time > gstate.now) {
            return true;
        }
    }
    for (i=0; i<avp->app_files.size(); i++) {
        fip = avp->app_files[i].file_info;
        pfx = fip->pers_file_xfer;
        if (pfx && pfx->next_request_time > gstate.now) {
            return true;
        }
    }
    return false;
}

/// Get the project this result belongs to.
///
/// \return Pointer to the project the current result belongs to.
PROJECT* RESULT::get_project() const {
    return project;
}

/// Get the name of this result.
///
/// \return The name of this result.
std::string RESULT::get_name() const {
    return name;
}

/// Get the time when this result was received from the server.
///
/// \return The time when this result was received from the server.
double RESULT::get_received_time() const {
    return received_time;
}

/// Set the time when this result was received from the server.
///
/// \param[int] received_time The time when this result was received from the server.
void RESULT::set_received_time(double received_time) {
    this->received_time = received_time;
}

const FILE_REF* RESULT::lookup_file(const FILE_INFO* fip) const {
    for (unsigned int i=0; i<output_files.size(); i++) {
        const FILE_REF& fr = output_files[i];
        if (fr.file_info == fip) return &fr;
    }
    return 0;
}

FILE_INFO* RESULT::lookup_file_logical(const char* lname) {
    for (unsigned int i=0; i<output_files.size(); i++) {
        FILE_REF& fr = output_files[i];
        if (!strcmp(lname, fr.open_name)) {
            return fr.file_info;
        }
    }
    return 0;
}

void RESULT::append_log_record(ACTIVE_TASK& at) {
    std::string filename = job_log_filename(*project);
    FILE* f = fopen(filename.c_str(), "ab");
    if (!f) return;
    fprintf(f, "%f ue %f ct %f fe %f sm %f sp %f sf %f sd %f sc %d nm %s\n",
        gstate.now,
        estimated_cpu_time_uncorrected(),
        final_cpu_time,
        wup->rsc_fpops_est,
        at.stats_mem,
        at.stats_page,
        at.stats_pagefault_rate,
        at.stats_disk,
        at.stats_checkpoint,
        name
    );
    fclose(f);
}

/// Abort a result that's not currently running.
///
void RESULT::abort_inactive(int status) {
    if (state() >= RESULT_COMPUTE_ERROR) return;
    set_state(RESULT_ABORTED, "RESULT::abort_inactive");
    exit_status = status;
}

MODE::MODE() {
    perm_mode = 0;
    temp_mode = 0;
    temp_timeout = 0;
}

void MODE::set(int mode, double duration) {
    if (mode == RUN_MODE_RESTORE) {
        temp_timeout = 0;
        temp_mode = perm_mode;
        return;
    }
    if (duration) {
        temp_mode = mode;
        temp_timeout = gstate.now + duration;
    } else {
        temp_timeout = 0;
        temp_mode = mode;
        perm_mode = mode;
        gstate.set_client_state_dirty("Set mode");
    }
}

int MODE::get_perm() const {
    return perm_mode;
}

int MODE::get_current() const {
    if (temp_timeout > gstate.now) {
        return temp_mode;
    } else {
        return perm_mode;
    }
}

double MODE::delay() const {
    if (temp_timeout > gstate.now) {
        return temp_timeout - gstate.now;
    } else {
        return 0;
    }
}
