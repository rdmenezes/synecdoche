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
#include <sys/stat.h>
#include <cctype>
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#include "shmem.h"
#endif

#include "filesys.h"
#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "client_msgs.h"
#include "sandbox.h"
#include "client_state.h"

#include "file_names.h"

void get_project_dir(const PROJECT* p, char* path, int len) {
    char buf[1024];
    escape_project_url(p->master_url, buf);
    snprintf(path, len, "%s/%s", PROJECTS_DIR, buf);
}

/// Gets the pathname of a file
void get_pathname(const FILE_INFO* fip, char* path, int len) {
    const PROJECT* p = fip->project;
    char buf[1024];

    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    //
    if (p) {
        get_project_dir(p, buf, sizeof(buf));
        snprintf(path, len, "%s/%s", buf, fip->name);
    } else {
        strlcpy(path, fip->name, len);
    }
}

void get_sched_request_filename(const PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.xml", SCHED_OP_REQUEST_BASE, url);
}

void get_sched_reply_filename(const PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.xml", SCHED_OP_REPLY_BASE, url);
}

void get_master_filename(const PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.xml", MASTER_BASE, url);
}

void job_log_filename(const PROJECT& project, char* buf, int len) {
    char url[1024];

    escape_project_url(project.master_url, url);
    snprintf(buf, len, "%s%s.txt", JOB_LOG_BASE, url);
}

/// Returns the location of a numbered slot directory
void get_slot_dir(int slot, char* path, int len) {
    snprintf(path, len, "%s/%d", SLOTS_DIR, slot);
}

/// Create the directory for the project p
int make_project_dir(const PROJECT& p) {
    char buf[1024];
    int retval;

    boinc_mkdir(PROJECTS_DIR);
#ifndef _WIN32
    mode_t old_mask;
    if (g_use_sandbox) {
        old_mask = umask(2);     // Project directories must be world-readable
         chmod(PROJECTS_DIR,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
    }
#endif
    get_project_dir(&p, buf, sizeof(buf));
    retval = boinc_mkdir(buf);
#ifndef _WIN32
    if (g_use_sandbox) {
        old_mask = umask(2);     // Project directories must be world-readable
        chmod(buf,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
        set_to_project_group(buf);
    }
#endif
    return retval;
}

int remove_project_dir(const PROJECT& p) {
    char buf[1024];
    int retval;

    get_project_dir(&p, buf, sizeof(buf));
    retval = client_clean_out_dir(buf);
    if (retval) {
        msg_printf(&p, MSG_INTERNAL_ERROR, "Can't delete file %s", boinc_failed_file);
        return retval;
    }
    return remove_project_owned_dir(buf);
}

/// Create the slot directory for the specified slot #
int make_slot_dir(int slot) {
    char buf[1024];

    if (slot<0) {
        msg_printf(NULL, MSG_INTERNAL_ERROR, "Bad slot number %d", slot);
        return ERR_NEG;
    }
    boinc_mkdir(SLOTS_DIR);
#ifndef _WIN32
    mode_t old_mask;
    if (g_use_sandbox) {
        old_mask = umask(2);     // Slot directories must be world-readable
        chmod(SLOTS_DIR,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
    }
#endif
    get_slot_dir(slot, buf, sizeof(buf));
    int retval = boinc_mkdir(buf);
#ifndef _WIN32
    if (g_use_sandbox) {
        old_mask = umask(2);     // Slot directories must be world-readable
        chmod(buf,
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
        set_to_project_group(buf);
    }
#endif
    return retval;
}

/// Delete unused stuff in the slots/ directory.
void delete_old_slot_dirs() {
    DirScanner dscan(SLOTS_DIR);
    while (1) {
        std::string filename;
        if (!dscan.scan(filename)) {
            break;
        }
        std::string path(SLOTS_DIR);
        path.append("/").append(filename);

        if (is_dir(path.c_str())) {
#ifndef _WIN32
            // If Synecdoche crashes or exits suddenly (e.g., due to
            // being called with --exit_after_finish) it may leave
            // orphan shared memory segments in the system.
            // Clean these up here. (We must do this before deleting the
            // INIT_DATA_FILE, if any, from each slot directory.)
            //
            std::string init_data_path(path);
            path.append("/").append(INIT_DATA_FILE);
            SHMEM_SEG_NAME shmem_seg_name = ftok(init_data_path.c_str(), 1);
            if (shmem_seg_name != -1) {
                destroy_shmem(shmem_seg_name);
            }
#endif
            if (!gstate.active_tasks.is_slot_dir_in_use(path.c_str())) {
                client_clean_out_dir(path.c_str());
                remove_project_owned_dir(path.c_str());
            }
        } else {
            delete_project_owned_file(path.c_str(), false);
        }
    }
}

/// Get the name of the account file for a given master URL.
///
/// \param[in] master_url The master URL for a project for which the account
///                       file name should be generated.
/// \return The name of the account file for the project specified by \a master_url.
std::string get_account_filename(const char* master_url) {
    char buf[1024];
    escape_project_url(master_url, buf);
    std::ostringstream result;
    result << "account_" << buf << ".xml";
    return result.str();
}

/// Print an error message for an invalid account file name.
///
/// \param[in] filename The invalid file name.
/// \return Always returns false.
static bool bad_account_filename(const std::string& filename) {
    msg_printf(NULL, MSG_INTERNAL_ERROR, "Invalid account filename: %s", filename.c_str());
    return false;
}

/// Check if the given filename if the name of an account file.
/// Account filenames are of the form
/// "account_URL.xml",
/// where URL is master URL with slashes replaced by underscores.
///
/// \param[in] filename The name of the file that should be checked.
/// \return True if the file name given by \a filename has the correct
///         syntax for an account file name, false otherwise.
bool is_account_file(const std::string& filename) {
    if (!starts_with(filename, "account_")) {
        // Just return without an error message here because this would
        // generate an error message for each file in the data directory
        // but error messages should only be shown for maleformed
        // account files.
        return false;
    }

    if (!ends_with(filename, ".xml")) {
        return bad_account_filename(filename);
    }

    // Now check if there is an url between "account_" and ".xml":
    if (filename.length() == (strlen("account_") + strlen(".xml"))) {
        return bad_account_filename(filename);
    }

    // All checks passed.
    return true;
}

/// Print an error message for an invalid statistics file name.
///
/// \param[in] filename The invalid file name.
/// \return Always returns false.
static bool bad_statistics_filename(const std::string& filename) {
    msg_printf(NULL, MSG_INTERNAL_ERROR, "Invalid statistics filename: %s", filename.c_str());
    return false;
}

/// Check if the given filename if the name of a statistics file.
/// statistics filenames are of the form
/// "statistics_URL.xml",
/// where URL is master URL with slashes replaced by underscores.
///
/// \param[in] filename The name of the file that should be checked.
/// \return True if the file name given by \a filename has the correct
///         syntax for a statistics file name, false otherwise.
bool is_statistics_file(const std::string& filename) {
    if (!starts_with(filename, "statistics_")) {
        // Just return without an error message here because this would
        // generate an error message for each file in the data directory
        // but error messages should only be shown for maleformed
        // statistics files.
        return false;
    }

    if (!ends_with(filename, ".xml")) {
        return bad_statistics_filename(filename);
    }

    // Now check if there is an url between "account_" and ".xml":
    if (filename.length() == (strlen("statistics_") + strlen(".xml"))) {
        return bad_statistics_filename(filename);
    }

    // All checks passed.
    return true;
}

void get_statistics_filename(const char* master_url, char* path) {
    char buf[256];
    escape_project_url(master_url, buf);
    sprintf(path, "statistics_%s.xml", buf);
}

bool is_image_file(const char* filename) {
    std::string fn = filename;
    downcase_string(fn);
    if (ends_with(fn, std::string(".jpg"))) return true;
    if (ends_with(fn, std::string(".jpeg"))) return true;
    if (ends_with(fn, std::string(".png"))) return true;
    return false;
}
