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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cctype>
#include <cstdio>
#include <sstream>
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#include <sys/stat.h>
#include "shmem.h"
#endif

#include "file_names.h"

#include "filesys.h"
#include "error_numbers.h"
#include "str_util.h"
#include "util.h"
#include "client_msgs.h"
#include "sandbox.h"
#include "client_state.h"

/// Get the directory for a given project.
///
/// \param[in] p Pointer to a project instance for which the directory
///              should be generated.
/// \return The directory for the given project.
std::string get_project_dir(const PROJECT* p) {
    std::ostringstream result;
    result << PROJECTS_DIR << '/' << escape_project_url(p->get_master_url());
    return result.str();
}

/// Gets the pathname (relative to client home dir) of a project file.
///
/// \param[in] fip Pointer to a FILE_INFO instance for which the physical file
///                name should be returned.
/// \return File name for the given FILE_INFO instance.
std::string get_pathname(const FILE_INFO* fip) {
    const PROJECT* p = fip->project;

    // for testing purposes, it's handy to allow a FILE_INFO without
    // an associated PROJECT.
    if (p) {
        std::ostringstream result;
        result << get_project_dir(p) << '/' << fip->name;
        return result.str();
    } else {
        return fip->name;
    }
}

/// Get the scheduler request file name for a project.
///
/// \param[in] project The project for which the file name should be returned.
/// \return The file name of the scheduler request file for the given project.
std::string get_sched_request_filename(const PROJECT& project) {
    std::ostringstream result;
    result << SCHED_OP_REQUEST_BASE << escape_project_url(project.get_master_url()) << ".xml";
    return result.str();
}

/// Get the scheduler reply file name for a project.
///
/// \param[in] project The project for which the file name should be returned.
/// \return The file name of the scheduler reply file for the given project.
std::string get_sched_reply_filename(const PROJECT& project) {
    std::ostringstream result;
    result << SCHED_OP_REPLY_BASE << escape_project_url(project.get_master_url()) << ".xml";
    return result.str();
}

/// Get the name of the master file for a project.
///
/// \param[in] project The project for which the file name should be returned.
/// \return The file name of the master file for the given project.
std::string get_master_filename(const PROJECT& project) {
    std::ostringstream result;
    result << MASTER_BASE << escape_project_url(project.get_master_url()) << ".xml";
    return result.str();
}

/// Returns the location of a numbered slot directory.
///
/// \param[in] slot The number of the slot.
/// \return The (relative) path to the requested slot directory.
std::string get_slot_dir(int slot) {
    std::ostringstream path;
    path << SLOTS_DIR << '/' << slot;
    return path.str();
}

/// Create the directory for the project \a p.
int make_project_dir(const PROJECT& p) {
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
    std::string project_dir = get_project_dir(&p);
    retval = boinc_mkdir(project_dir.c_str());
#ifndef _WIN32
    if (g_use_sandbox) {
        old_mask = umask(2);     // Project directories must be world-readable
        chmod(project_dir.c_str(),
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
        set_to_project_group(project_dir.c_str());
    }
#endif
    return retval;
}

int remove_project_dir(const PROJECT& p) {
    int retval;

    std::string project_dir = get_project_dir(&p);
    retval = client_clean_out_dir(project_dir.c_str());
    if (retval) {
        msg_printf(&p, MSG_INTERNAL_ERROR, "Can't delete file %s", boinc_failed_file);
        return retval;
    }
    return remove_project_owned_dir(project_dir.c_str());
}

/// Create the slot directory for the specified slot number.
int make_slot_dir(int slot) {
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
    std::string slot_dir = get_slot_dir(slot);
    int retval = boinc_mkdir(slot_dir.c_str());
#ifndef _WIN32
    if (g_use_sandbox) {
        old_mask = umask(2);     // Slot directories must be world-readable
        chmod(slot_dir.c_str(),
            S_IRUSR|S_IWUSR|S_IXUSR
            |S_IRGRP|S_IWGRP|S_IXGRP
            |S_IROTH|S_IXOTH
        );
        umask(old_mask);
        set_to_project_group(slot_dir.c_str());
    }
#endif
    return retval;
}

/// Delete unused subdirectories in the slots/ directory.
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
            if (!gstate.active_tasks.is_slot_dir_in_use(path)) {
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
std::string get_account_filename(const std::string& master_url) {
    std::ostringstream result;
    result << "account_" << escape_project_url(master_url) << ".xml";
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
        // generate an error message for each file in the data directory;
        // but error messages should only be shown for malformed
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
        // but error messages should only be shown for malformed
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

/// Get the name of the statistics file for a given master URL.
///
/// \param[in] master_url The master URL for a project for which the statistics
///                       file name should be generated.
/// \return The name of the statistics file for the project specified by \a master_url.
std::string get_statistics_filename(const std::string& master_url) {
    std::ostringstream result;
    result << "statistics_" << escape_project_url(master_url) << ".xml";
    return result.str();
}

/// Check if a file name denotes an image file.
/// This function checks the file extension and returns true for files
/// ending with ".jpg", ".jpeg" or ".png".
///
/// \param[in] filename The file name that should be checked.
/// \return True if the given file name denotes an image file.
bool is_image_file(std::string filename) {
    downcase_string(filename);
    if (ends_with(filename, ".jpg")) return true;
    if (ends_with(filename, ".jpeg")) return true;
    if (ends_with(filename, ".png")) return true;
    return false;
}
