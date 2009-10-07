// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#include "app_ipc.h"

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstring>
#include <sstream>
#include <string>
#endif

#include "parse.h"
#include "error_numbers.h"
#include "str_util.h"
#include "filesys.h"
#include "miofile.h"
#include "miofile_wrap.h"
#include "xml_write.h"

using std::string;

const char* xml_graphics_modes[NGRAPHICS_MSGS] = {
    "<mode_unsupported/>",
    "<mode_hide_graphics/>",
    "<mode_window/>",
    "<mode_fullscreen/>",
    "<mode_blankscreen/>",
    "<reread_prefs/>",
    "<mode_quit/>"
};

GRAPHICS_MSG::GRAPHICS_MSG() : mode(0) {
}

APP_INIT_DATA::APP_INIT_DATA() : project_preferences(0) {
}

APP_INIT_DATA::~APP_INIT_DATA() {
    if (project_preferences) {
        free(project_preferences);
        project_preferences=0;      // paranoia
    }
}

APP_INIT_DATA::APP_INIT_DATA(const APP_INIT_DATA& a) {
    copy(a);
}

APP_INIT_DATA& APP_INIT_DATA::operator=(const APP_INIT_DATA& a) {
    if (this != &a) {
        copy(a);
    }
    return *this;
}

void APP_INIT_DATA::copy(const APP_INIT_DATA& a) {
    major_version = a.major_version;
    minor_version = a.minor_version;
    release = a.release;
    app_version = a.app_version;
    strlcpy(app_name, a.app_name, sizeof(app_name));
    strlcpy(symstore, a.symstore, sizeof(symstore));
    strlcpy(acct_mgr_url, a.acct_mgr_url, sizeof(acct_mgr_url));
    if (a.project_preferences) {
        project_preferences = strdup(a.project_preferences);
    } else {
        project_preferences = 0;
    }
    hostid = a.hostid;
    strlcpy(user_name, a.user_name, sizeof(user_name));
    strlcpy(team_name, a.team_name, sizeof(team_name));
    strlcpy(project_dir, a.project_dir, sizeof(project_dir));
    strlcpy(boinc_dir, a.boinc_dir, sizeof(boinc_dir));
    strlcpy(wu_name, a.wu_name, sizeof(wu_name));
    strlcpy(authenticator, a.authenticator, sizeof(authenticator));
    slot = a.slot;
    user_total_credit = a.user_total_credit;
    user_expavg_credit = a.user_expavg_credit;
    host_total_credit = a.host_total_credit;
    host_expavg_credit = a.host_expavg_credit;
    resource_share_fraction = a.resource_share_fraction;
    host_info = a.host_info;
    proxy_info = a.proxy_info;
    global_prefs = a.global_prefs;

    rsc_fpops_est = a.rsc_fpops_est;
    rsc_fpops_bound = a.rsc_fpops_bound;
    rsc_memory_bound = a.rsc_memory_bound;
    rsc_disk_bound = a.rsc_disk_bound;
    computation_deadline = a.computation_deadline;

    fraction_done_start = a.fraction_done_start;
    fraction_done_end = a.fraction_done_end;

    checkpoint_period = a.checkpoint_period;
#ifdef _WIN32
    strlcpy(shmem_seg_name, a.shmem_seg_name, sizeof(shmem_seg_name));
#else
    shmem_seg_name = a.shmem_seg_name;
#endif
    wu_cpu_time = a.wu_cpu_time;
}

/// \todo Document this function -- NA
/// \todo Add unit tests for this function, maybe parsing the result with
/// parse_init_data_file() and comparing -- NA
void write_init_data_file(std::ostream& out, APP_INIT_DATA& ai) {
    out << "<app_init_data>\n"
        << XmlTag<int>("major_version", ai.major_version)
        << XmlTag<int>("minor_version", ai.minor_version)
        << XmlTag<int>("release",       ai.release)
        << XmlTag<int>("app_version",   ai.app_version)
    ;
    if (strlen(ai.app_name)) {
        out << XmlTag<const char*>("app_name", ai.app_name);
    }
    if (strlen(ai.symstore)) {
        out << XmlTag<const char*>("symstore", ai.symstore);
    }
    if (strlen(ai.acct_mgr_url)) {
        out << XmlTag<const char*>("acct_mgr_url", ai.acct_mgr_url);
    }
    if (ai.project_preferences && strlen(ai.project_preferences)) {
        // Don't replace this with XmlTag!
        // We probably need the newline after open tag,
        // and we definitely can't escape this (ai.project_preferences contains XML)
        out << "<project_preferences>\n" << ai.project_preferences << "</project_preferences>\n";
    }
    if (strlen(ai.team_name)) {
        out << XmlTag<XmlString>("team_name", ai.team_name);
    }
    if (strlen(ai.user_name)) {
        out << XmlTag<XmlString>("user_name", ai.user_name);
    }
    if (strlen(ai.project_dir)) {
        out << XmlTag<const char*>("project_dir", ai.project_dir);
    }
    if (strlen(ai.boinc_dir)) {
        out << XmlTag<const char*>("boinc_dir", ai.boinc_dir);
    }
    if (strlen(ai.authenticator)) {
        out << XmlTag<const char*>("authenticator", ai.authenticator);
    }
    if (strlen(ai.wu_name)) {
        out << XmlTag<const char*>("wu_name", ai.wu_name);
    }
#ifdef _WIN32
    if (strlen(ai.shmem_seg_name)) {
        out << XmlTag<const char*>("comm_obj_name", ai.shmem_seg_name);
    }
#else
    out << XmlTag<int>("shm_key", ai.shmem_seg_name);
#endif
    out << XmlTag<int>   ("slot", ai.slot)
        << XmlTag<double>("wu_cpu_time",            ai.wu_cpu_time)
        << XmlTag<double>("user_total_credit",      ai.user_total_credit)
        << XmlTag<double>("user_expavg_credit",     ai.user_expavg_credit)
        << XmlTag<double>("host_total_credit",      ai.host_total_credit)
        << XmlTag<double>("host_expavg_credit",     ai.host_expavg_credit)
        << XmlTag<double>("resource_share_fraction",ai.resource_share_fraction)
        << XmlTag<double>("checkpoint_period",      ai.checkpoint_period)
        << XmlTag<double>("fraction_done_start",    ai.fraction_done_start)
        << XmlTag<double>("fraction_done_end",      ai.fraction_done_end)
        << XmlTag<double>("rsc_fpops_est",          ai.rsc_fpops_est)
        << XmlTag<double>("rsc_fpops_bound",        ai.rsc_fpops_bound)
        << XmlTag<double>("rsc_memory_bound",       ai.rsc_memory_bound)
        << XmlTag<double>("rsc_disk_bound",         ai.rsc_disk_bound)
        << XmlTag<double>("computation_deadline",   ai.computation_deadline)
    ;
    ai.host_info.write(out, false);
    ai.proxy_info.write(out);
    ai.global_prefs.write(MiofileFromOstream(out));
    out << "</app_init_data>\n";
}

int parse_init_data_file(FILE* f, APP_INIT_DATA& ai) {
    char tag[1024];
    int retval;
    bool flag, is_tag;

    MIOFILE mf;
    mf.init_file(f);
    XML_PARSER xp(&mf);

    if (!xp.parse_start("app_init_data")) {
        fprintf(stderr, "no start tag in app init data\n");
        return ERR_XML_PARSE;
    }

    if (ai.project_preferences) {
        free(ai.project_preferences);
        ai.project_preferences = 0;
    }
    memset(&ai, 0, sizeof(ai));
    ai.fraction_done_start = 0;
    ai.fraction_done_end = 1;

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) {
            fprintf(stderr, "unexpected text in init_data.xml: %s\n", tag);
            continue;
        }
        if (!strcmp(tag, "/app_init_data")) return 0;
        if (!strcmp(tag, "project_preferences")) {
            retval = dup_element(f, "project_preferences", &ai.project_preferences);
            if (retval) return retval;
            continue;
        }
        if (!strcmp(tag, "global_preferences")) {
            GLOBAL_PREFS_MASK mask;
            retval = ai.global_prefs.parse(xp, "", flag, mask);
            if (retval) return retval;
            continue;
        }
        if (!strcmp(tag, "host_info")) {
            ai.host_info.parse(mf);
            continue;
        }
        if (!strcmp(tag, "proxy_info")) {
            ai.proxy_info.parse(mf);
            continue;
        }
        if (xp.parse_int(tag, "major_version", ai.major_version)) continue;
        if (xp.parse_int(tag, "minor_version", ai.minor_version)) continue;
        if (xp.parse_int(tag, "release", ai.release)) continue;
        if (xp.parse_int(tag, "app_version", ai.app_version)) continue;
        if (xp.parse_str(tag, "app_name", ai.app_name, sizeof(ai.app_name))) continue;
        if (xp.parse_str(tag, "symstore", ai.symstore, sizeof(ai.symstore))) continue;
        if (xp.parse_str(tag, "acct_mgr_url", ai.acct_mgr_url, sizeof(ai.acct_mgr_url))) continue;
        if (xp.parse_str(tag, "user_name", ai.user_name, sizeof(ai.user_name))) continue;
        if (xp.parse_str(tag, "team_name", ai.team_name, sizeof(ai.team_name))) continue;
        if (xp.parse_str(tag, "project_dir", ai.project_dir, sizeof(ai.project_dir))) continue;
        if (xp.parse_str(tag, "boinc_dir", ai.boinc_dir, sizeof(ai.boinc_dir))) continue;
        if (xp.parse_str(tag, "authenticator", ai.authenticator, sizeof(ai.authenticator))) continue;
        if (xp.parse_str(tag, "wu_name", ai.wu_name, sizeof(ai.wu_name))) continue;
#ifdef _WIN32
        if (xp.parse_str(tag, "comm_obj_name", ai.shmem_seg_name, sizeof(ai.shmem_seg_name))) continue;
#else
        if (xp.parse_int(tag, "shm_key", ai.shmem_seg_name)) continue;
#endif
        if (xp.parse_int(tag, "slot", ai.slot)) continue;
        if (xp.parse_double(tag, "user_total_credit", ai.user_total_credit)) continue;
        if (xp.parse_double(tag, "user_expavg_credit", ai.user_expavg_credit)) continue;
        if (xp.parse_double(tag, "host_total_credit", ai.host_total_credit)) continue;
        if (xp.parse_double(tag, "host_expavg_credit", ai.host_expavg_credit)) continue;
        if (xp.parse_double(tag, "resource_share_fraction", ai.resource_share_fraction)) continue;
        if (xp.parse_double(tag, "rsc_fpops_est", ai.rsc_fpops_est)) continue;
        if (xp.parse_double(tag, "rsc_fpops_bound", ai.rsc_fpops_bound)) continue;
        if (xp.parse_double(tag, "rsc_memory_bound", ai.rsc_memory_bound)) continue;
        if (xp.parse_double(tag, "rsc_disk_bound", ai.rsc_disk_bound)) continue;
        if (xp.parse_double(tag, "computation_deadline", ai.computation_deadline)) continue;
        if (xp.parse_double(tag, "wu_cpu_time", ai.wu_cpu_time)) continue;
        if (xp.parse_double(tag, "checkpoint_period", ai.checkpoint_period)) continue;
        if (xp.parse_double(tag, "fraction_done_start", ai.fraction_done_start)) continue;
        if (xp.parse_double(tag, "fraction_done_end", ai.fraction_done_end)) continue;
        xp.skip_unexpected(tag, false, "parse_init_data_file");
    }
    fprintf(stderr, "parse_init_data_file: no end tag\n");
    return ERR_XML_PARSE;
}

APP_CLIENT_SHM::APP_CLIENT_SHM() :shm(0) {
}

bool MSG_CHANNEL::get_msg(char *msg) {
    if (!buf[0]) return false;
    strlcpy(msg, buf+1, MSG_CHANNEL_SIZE-1);
    buf[0] = 0;
    return true;
}

bool MSG_CHANNEL::has_msg() {
    if (buf[0]) return true;
    return false;
}

bool MSG_CHANNEL::send_msg(const char *msg) {
    if (buf[0]) return false;
    strlcpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
    return true;
}

void MSG_CHANNEL::send_msg_overwrite(const char* msg) {
    strlcpy(buf+1, msg, MSG_CHANNEL_SIZE-1);
    buf[0] = 1;
}

int APP_CLIENT_SHM::decode_graphics_msg(const char* msg, GRAPHICS_MSG& m) {
    int i;

    parse_str(msg, "<window_station>", m.window_station);
    parse_str(msg, "<desktop>", m.desktop);
    parse_str(msg, "<display>", m.display);

    m.mode = 0;
    for (i=0; i<NGRAPHICS_MSGS; i++) {
        if (match_tag(msg, xml_graphics_modes[i])) {
            m.mode = i;
        }
    }
    return 0;
}

void APP_CLIENT_SHM::reset_msgs() {
    memset(shm, 0, sizeof(SHARED_MEM));
}

/// Resolve virtual name (in slot dir) to physical path (in project dir).
/// This function is a C-version of boinc_resolve_filename_s and only exists
/// for compatibility reasons. See boinc_resolve_filename_s for more
/// information.
///
/// \param[in] virtual_name String describing the virtual file name which
///                         should get resolved.
/// \param[out] physical_name Pointer to a buffer that should receive the
///                           resolved file name belonging to the virtual
///                           file name in \a virtual_name.
/// \param[out] len Size of the output buffer \a physical_name.
/// \return Zero on success, ERR_NULL if \a virtual_name is zero,
///         ERR_BUFFER_OVERFLOW if the buffer pointed to by \a physical_name
///         is too small.
int boinc_resolve_filename(const char* virtual_name, char* physical_name, int len) {
    std::string buf;
    int ret_val = boinc_resolve_filename_s(virtual_name, buf);
    if (ret_val == 0) {
        // Check if the output buffer is big enough:
        if (buf.size() + 1 >= static_cast<std::string::size_type>(len)) {
            return ERR_BUFFER_OVERFLOW;
        }
        strlcpy(physical_name, buf.c_str(), len);
    }
    return ret_val;
}


/// Resolve virtual name (in slot dir) to physical path (in project dir).
/// Cases:
/// - Windows and pre-6.12 Unix:
///   virtual name refers to a "soft link" (XML file acting as symbolic link)
/// - 6.12+ Unix:
///   virtual name is a symbolic link
/// - Standalone: physical path is same as virtual name
///
/// \param[in] virtual_name String describing the virtual file name which
///                         should get resolved.
/// \param[out] physical_name Reference to a string instance that should
///                           receive the resolved file name belonging to
///                           the virtual file name in \a virtual_name.
/// \return Zero on success, ERR_NULL if \a virtual_name is zero.
int boinc_resolve_filename_s(const char *virtual_name, string& physical_name) {
    if (!virtual_name) {
        return ERR_NULL;
    }
    physical_name = virtual_name;
#ifndef _WIN32
    if (is_symlink(virtual_name)) {
        return 0;
    }
#endif
    // Open the link file and read the first line
    FILE *fp = boinc_fopen(virtual_name, "r");
    if (!fp) {
        return 0;
    }

    // Must initialize buf since fgets() on an empty file won't do anything.
    char buf[512] = {0};
    char* p = fgets(buf, sizeof(buf), fp);
    fclose(fp);
    if (p) {
        parse_str(buf, "<soft_link>", physical_name);
    }
    return 0;
}

/// Get the directory for a project denoted by its master-url.
///
/// \param[in] url The master-url of the project.
/// \return The directory used for the project denoted by the given master-url.
std::string url_to_project_dir(const std::string& url) {
    std::ostringstream result;
    result << PROJECT_DIR << '/' << escape_project_url(url);
    return result.str();
}
