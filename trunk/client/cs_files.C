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

/// \file
/// The "policy" part of file transfer is here.
/// The "mechanism" part is in pers_file_xfer.C and file_xfer.C

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "client_state.h"
#include "client_types.h"

#include "md5_file.h"
#include "crypt.h"
#include "str_util.h"
#include "filesys.h"
#include "error_numbers.h"

#include "file_names.h"
#include "client_msgs.h"
#include "file_xfer.h"
#include "pers_file_xfer.h"

using std::vector;

/// Decide whether to consider starting a new file transfer.
bool CLIENT_STATE::start_new_file_xfer(PERS_FILE_XFER& pfx) {
    int ntotal=0, nproj=0;

    if (network_suspended) return false;


    // limit the number of file transfers per project
    // (uploads and downloads are limited separately)
    //
    for (size_t i=0; i<file_xfers->file_xfers.size(); i++) {
        const FILE_XFER* fxp = file_xfers->file_xfers[i];
        if (pfx.is_upload == fxp->is_upload) {
            ntotal++;
            if (pfx.fip->project == fxp->fip->project) {
                nproj++;
            }
        }
    }
    if (nproj >= config.max_file_xfers_per_project) return false;
    if (ntotal >= config.max_file_xfers) return false;
    return true;
}

/// Make a directory for each of the projects in the client state.
int CLIENT_STATE::make_project_dirs() {
    unsigned int i;
    int retval;
    for (i=0; i<projects.size(); i++) {
        retval = make_project_dir(*projects[i]);
        if (retval) return retval;
    }
    return 0;
}

/// Check the existence and/or validity of a file.
/// If "strict" is true, check either the digital signature of the file
/// (if signature_required is set) or its MD5 checksum.
/// Otherwise check its size.
///
/// This is called:
/// -# right after download is finished (CLIENT_STATE::handle_pers_file_xfers()).
/// -# if a needed file is already on disk (PERS_FILE_XFER::start_xfer()).
/// -# in checking whether a result's input files are available
///    (CLIENT_STATE::input_files_available()).
///    In this case "strict" is false,
///    and we just check existence and size (no checksum).
///
/// If a failure occurs, set the file's "status" field.
/// This will cause the app_version or workunit that used the file
/// to error out (via APP_VERSION::had_download_failure()
/// WORKUNIT::had_download_failure())
int FILE_INFO::verify_file(bool strict, bool show_errors) {
    char cksum[64];
    bool verified;
    int retval;
    double size, local_nbytes;

    std::string pathname = get_pathname(this);

    // If the file isn't there at all, set status to FILE_NOT_PRESENT;
    // this will trigger a new download rather than erroring out
    if (file_size(pathname.c_str(), size)) {
        status = FILE_NOT_PRESENT;
        return ERR_FILE_MISSING;
    }

    if (gstate.global_prefs.dont_verify_images
        && is_image_file(name)
        && size>0
    ) {
        return 0;
    }

    if (nbytes && (nbytes != size) && (!config.dont_check_file_sizes)) {
        msg_printf(project, MSG_INTERNAL_ERROR, 
                   "File %s has wrong size. Expected %.0f, got %.0f",
                   name.c_str(), nbytes, size);
        status = ERR_WRONG_SIZE;
        return ERR_WRONG_SIZE;
    }

    if (!strict) return 0;

    if (signature_required) {
        if (file_signature.empty()) {
            msg_printf(project, MSG_INTERNAL_ERROR, "Application file %s missing signature", name.c_str());
            msg_printf(project, MSG_INTERNAL_ERROR, "Synecdoche cannot accept this file");
            error_msg = "missing signature";
            status = ERR_NO_SIGNATURE;
            return ERR_NO_SIGNATURE;
        }
        retval = verify_file2(pathname.c_str(), file_signature.c_str(), project->code_sign_key, verified);
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR, "Signature verification error for %s", name.c_str());
            error_msg = "signature verification error";
            status = ERR_RSA_FAILED;
            return ERR_RSA_FAILED;
        }
        if (!verified && show_errors) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                    "Signature verification failed for %s", name.c_str());
            error_msg = "signature verification failed";
            status = ERR_RSA_FAILED;
            return ERR_RSA_FAILED;
        }
    } else if (strlen(md5_cksum)) {
        retval = md5_file(pathname.c_str(), cksum, local_nbytes);
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR, "MD5 computation error for %s: %s\n",
                    name.c_str(), boincerror(retval));
            error_msg = "MD5 computation error";
            status = retval;
            return retval;
        }
        if (strcmp(cksum, md5_cksum)) {
            if (show_errors) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                        "MD5 check failed for %s", name.c_str());
                msg_printf(project, MSG_INTERNAL_ERROR,
                        "expected %s, got %s\n", md5_cksum, cksum);
            }
            error_msg = "MD5 check failed";
            status = ERR_MD5_FAILED;
            return ERR_MD5_FAILED;
        }
    }
    return 0;
}

/// scan all FILE_INFOs and PERS_FILE_XFERs.
/// start and finish downloads and uploads as needed.
bool CLIENT_STATE::handle_pers_file_xfers() {
    unsigned int i;
    FILE_INFO* fip;
    PERS_FILE_XFER *pfx;
    bool action = false;
    int retval;
    static double last_time;

    if (now - last_time < 1.0) return false;
    last_time = now;

    // Look for FILE_INFOs for which we should start a transfer,
    // and make PERS_FILE_XFERs for them
    for (i=0; i<file_infos.size(); i++) {
        fip = file_infos[i];
        pfx = fip->pers_file_xfer;
        if (pfx) continue;
        if (!fip->generated_locally && fip->status == FILE_NOT_PRESENT) {
            pfx = new PERS_FILE_XFER;
            pfx->init(fip, false);
            fip->pers_file_xfer = pfx;
            pers_file_xfers->insert(fip->pers_file_xfer);
            action = true;
        } else if (fip->upload_when_present && fip->status == FILE_PRESENT && !fip->uploaded) {
            pfx = new PERS_FILE_XFER;
            pfx->init(fip, true);
            fip->pers_file_xfer = pfx;
            pers_file_xfers->insert(fip->pers_file_xfer);
            action = true;
        }
    }

    // Scan existing PERS_FILE_XFERs, looking for those that are done,
    // and deleting them
    vector<PERS_FILE_XFER*>::iterator iter;
    iter = pers_file_xfers->pers_file_xfers.begin();
    while (iter != pers_file_xfers->pers_file_xfers.end()) {
        pfx = *iter;

        // If the transfer finished, remove the PERS_FILE_XFER object
        // from the set and delete it
        if (pfx->pers_xfer_done) {
            fip = pfx->fip;
            if (fip->generated_locally || fip->upload_when_present) {
                // file has been uploaded - delete if not sticky
                if (!fip->sticky) {
                    fip->delete_file();
                }
                fip->uploaded = true;
                active_tasks.upload_notify_app(fip);
            } else if (fip->status >= 0) {
                // file transfer did not fail (non-negative status)

                // verify the file with RSA or MD5, and change permissions
                retval = fip->verify_file(true, true);
                if (retval) {
                    msg_printf(fip->project, MSG_INTERNAL_ERROR,
                            "Checksum or signature error for %s", fip->name.c_str());
                    fip->status = retval;
                } else {
                    // Set the appropriate permissions depending on whether
                    // it's an executable or normal file
                    retval = fip->set_permissions();
                    fip->status = FILE_PRESENT;
                }

                // if it's a user file, tell running apps to reread prefs
                if (fip->is_user_file) {
                    active_tasks.request_reread_prefs(fip->project);
                }

                // if it's a project file, make a link in project dir
                if (fip->is_project_file) {
                    PROJECT* p = fip->project;
                    p->write_symlink_for_project_file(fip);
                    p->update_project_files_downloaded_time();
                }
            }
            iter = pers_file_xfers->pers_file_xfers.erase(iter);
            delete pfx;
            action = true;
            // `delete pfx' should have set pfx->fip->pfx to NULL
            assert (fip == NULL || fip->pers_file_xfer == NULL);
        } else {
            iter++;
        }
    }

    return action;
}

/// called at startup to ensure that if the core client
/// thinks a file is there, it's actually there.
void CLIENT_STATE::check_file_existence() {
    for (std::vector<FILE_INFO*>::iterator it = file_infos.begin(); it != file_infos.end(); ++it) {
        FILE_INFO* fip = *it;
        if (fip->status == FILE_PRESENT) {
            std::string path = get_pathname(fip);
            if (!boinc_file_exists(path)) {
                // The missing file is still referenced by a project or a workunit,
                // otherwise it would have been removed by garbage collection.
                // But if the associated project doesn't have any workunit, there is
                // no need to re-transfer this missing file as it may never be
                // needed again. If it is needed again at some point in the
                // future, it can re-transfered then.
                // However, there is one exception to this rule: Files that
                // are directly associated with the project and not with an
                // app_version or workunit/result aren't checked at any later
                // point because they are not required as input files for results.
                // These files (like icon, slideshow, etc.) are mainly used in
                // the manager and/or screensaver. Therefore we have to
                // download these files immediately.
                WORKUNIT_PVEC wus = fip->project->get_workunits();
                bool file_required = !wus.empty();
                if (!file_required) {
                    // No WUs for this project, now check if this file is a
                    // project file:
                    const FILE_REF_VEC& pfiles = fip->project->project_files;
                    for (FILE_REF_VEC::const_iterator it = pfiles.begin(); it != pfiles.end(); ++it) {
                        if ((*it).file_info == fip) {
                            // The missing file is a project file. => We need it now.
                            file_required = true;
                            break;
                        }
                    }
                }
                if (file_required) {
                    // OK, the file is required, mark as missing.
                    fip->status = FILE_NOT_PRESENT;
                    msg_printf(fip->project, MSG_INFO, "File %s not found", path.c_str());
                } else {
                    // Although the file is currently not required, we
                    // can't delete the FILE_INFO instance because this would
                    // prevent re-downloading the file once it is needed.
                    // Instead mark it as missing but not required.
                    fip->status = FILE_NOT_PRESENT_NOT_NEEDED;
                    msg_printf(fip->project, MSG_INFO,
                        "File %s not found. Currently not required, skipping download.",
                        path.c_str());
                }
            }
        }
    }
}
