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
#else
#include "config.h"
#include <sstream>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "util.h"
#include "str_util.h"
#include "sandbox.h"
#include "client_state.h"

/// Scan project dir for file names of the form trickle_up_X_Y
/// where X is a result name and Y is a timestamp.
/// Convert them to XML (for sched request message).
///
/// \param[in] project Pointer to a PROJECT instance for the project for which
///                    trickle files should be read.
/// \param[in] f Pointer to a file that should receive the xml-version of the
///              content of the trickle files.
/// \return Always returns zero.
int CLIENT_STATE::read_trickle_files(const PROJECT* project, FILE* f) {
    char project_dir[256];
    get_project_dir(project, project_dir, sizeof(project_dir));

    DirScanner ds(project_dir);
    std::string fn;

    // Trickle-up filenames are of the form trickle_up_RESULTNAME_TIME[.sent]
    while (ds.scan(fn)) {
        // Check if the current file is a trickle file:
        if (!starts_with(fn, "trickle_up_")) {
            continue;
        }
        std::string::size_type res_begin = strlen("trickle_up_");
        std::string::size_type res_end = fn.rfind('_');
        if (res_end <= res_begin) {
            continue;
        }

        // Extract the result name:
        std::string result_name = fn.substr(res_begin, res_end - res_begin);

        // Extract the timestamp:
        std::stringstream tmp(fn.substr(res_end + 1));
        time_t t;
        tmp >> t;

        // Read the content of the trickle file:
        std::string path(project_dir);
        path.append("/").append(fn);
        char* file_contents;
        if (read_file_malloc(path.c_str(), file_contents)) {
            continue;
        }
        fprintf(f,
            "  <msg_from_host>\n"
            "      <result_name>%s</result_name>\n"
            "      <time>%d</time>\n"
            "%s\n"
            "  </msg_from_host>\n",
            result_name.c_str(),
            (int)t,
            file_contents
        );
        free(file_contents);

        // Append .sent to filename, so we'll know which ones to delete later.
        if (!ends_with(fn, ".sent")) {
            std::ostringstream newpath;
            newpath << project_dir << '/' << fn << ".sent";
            boinc_rename(path.c_str(), newpath.str().c_str());
        }
    }
    return 0;
}

/// Remove trickle files when ack has been received.
/// Remove only this ending with ".sent"
/// (others arrived from application while RPC was happening).
///
/// \param[in] project Pointer to a PROJECT instance of the project for which
///                    the trickle files should be removed.
/// \return Always returns zero.
int CLIENT_STATE::remove_trickle_files(const PROJECT* project) {
    char project_dir[256];
    get_project_dir(project, project_dir, sizeof(project_dir));

    DirScanner ds(project_dir);
    std::string fn;
    while (ds.scan(fn)) {
        if ((starts_with(fn, "trickle_up")) && (ends_with(fn, ".sent"))) {
            std::string path(project_dir);
            path.append("/").append(fn);
            delete_project_owned_file(path.c_str(), true);
        }
    }
    return 0;
}

/// Parse a trickle-down message in a scheduler reply.
/// Locate the corresponding active task,
/// write a file in the slot directory,
/// and notify the task.
///
/// \param[in] project A pointer to the PROJECT instance of the project for
///                    which a trickle-down message was received.
/// \param[in] in A pointer to the file containing the scheduler reply with
///               the trickle-down message.
/// \return Zero on success, ERR_NULL if the result for the trickle-down
///         message could not be found, ERR_FOPEN if creating the trickle-down
///         file failed, ERR_XML_PARSE if the input was malformed.
int CLIENT_STATE::handle_trickle_down(const PROJECT* project, FILE* in) {
    char buf[256];
    char result_name[256];
    std::string body;
    int send_time = 0;

    result_name[0] = 0;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</trickle_down>")) {
            RESULT* rp = lookup_result(project, result_name);
            if (!rp) {
                return ERR_NULL;
            }
            ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
            if (!atp) {
                return ERR_NULL;
            }
            std::ostringstream path;
            path << atp->slot_dir << "/trickle_down_" << send_time;
            FILE* f = fopen(path.str().c_str(), "w"); // Shouldn't this use boinc_fopen?
            if (!f) {
                return ERR_FOPEN;
            }
            fputs(body.c_str(), f);
            fclose(f);
            atp->have_trickle_down = true;
            return 0;
        } else if (parse_str(buf, "<result_name>", result_name, 256)) {
            continue;
        } else if (parse_int(buf, "<time>", send_time)) {
            continue;
        } else {
            body += buf;
        }
    }
    return ERR_XML_PARSE;
}
