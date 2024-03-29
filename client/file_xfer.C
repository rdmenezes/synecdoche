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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "file_xfer.h"

#include "util.h"
#include "file_names.h"
#include "client_state.h"
#include "filesys.h"
#include "client_msgs.h"
#include "pers_file_xfer.h"
#include "parse.h"
#include "error_numbers.h"

#include "version.h"

using std::vector;

FILE_XFER::FILE_XFER() {
    file_xfer_done = false;
    file_xfer_retval = 0;
    fip = NULL;
    strcpy(header, "");
    file_size_query = false;
}

FILE_XFER::~FILE_XFER() {
    if (fip && fip->pers_file_xfer) {
        fip->pers_file_xfer->fxp = NULL;
    }
}

int FILE_XFER::init_download(FILE_INFO& file_info) {
    is_upload = false;
    fip = &file_info;
    pathname = get_pathname(fip);

    // if file is already as large or larger than it's supposed to be,
    // something's screwy; start reading it from the beginning.
    if (file_size(pathname.c_str(), starting_size) || starting_size >= fip->nbytes) {
        starting_size = 0;
    }
    bytes_xferred = starting_size;

    const char* url = fip->get_current_url(is_upload);
    if (!url) {
        return ERR_INVALID_URL;
	}
    return HTTP_OP::init_get(url, pathname.c_str(), false, (int)starting_size);
}

/// For uploads, we need to build a header with xml_signature etc.
/// (see wiki/FileUpload)
/// Do this in memory.
///
/// \todo Give priority to unfinished uploads if there are multiple choices.
int FILE_XFER::init_upload(FILE_INFO& file_info) {
    // If upload_offset < 0, we need to query the upload handler
    // for the offset information
    fip = &file_info;
    pathname = get_pathname(fip);

    is_upload = true;

    if (file_info.upload_offset < 0) {
        bytes_xferred = 0;
        sprintf(header,
            "<data_server_request>\n"
            "    <core_client_major_version>%d</core_client_major_version>\n"
            "    <core_client_minor_version>%d</core_client_minor_version>\n"
            "    <core_client_release>%d</core_client_release>\n"
            "    <get_file_size>%s</get_file_size>\n"
            "</data_server_request>\n",
            BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE,
            file_info.name.c_str());

        file_size_query = true;
        const char* url = fip->get_current_url(is_upload);
        if (!url) return ERR_INVALID_URL;
        return HTTP_OP::init_post2(url, header, sizeof(header), NULL, 0);
    } else {
        bytes_xferred = file_info.upload_offset;
        sprintf(header,
            "<data_server_request>\n"
            "    <core_client_major_version>%d</core_client_major_version>\n"
            "    <core_client_minor_version>%d</core_client_minor_version>\n"
            "    <core_client_release>%d</core_client_release>\n"
            "<file_upload>\n"
            "<file_info>\n"
            "%s"
            "<xml_signature>\n"
            "%s"
            "</xml_signature>\n"
            "</file_info>\n"
            "<nbytes>%.0f</nbytes>\n"
            "<md5_cksum>%s</md5_cksum>\n"
            "<offset>%.0f</offset>\n"
            "<data>\n",
            BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE,
            file_info.signed_xml.c_str(),
            file_info.xml_signature.c_str(),
            file_info.nbytes,
            file_info.md5_cksum,
            file_info.upload_offset
        );
        file_size_query = false;
        const char* url = fip->get_current_url(is_upload);
        if (!url) return ERR_INVALID_URL;
        return HTTP_OP::init_post2(
            url, header, sizeof(header), pathname.c_str(), fip->upload_offset
        );
    }
}

/// Parse the file upload handler response in req1.
///
int FILE_XFER::parse_upload_response(double &nbytes) {
    int status = ERR_UPLOAD_TRANSIENT, x;
    char buf[256];

    nbytes = -1;
    parse_double(req1, "<file_size>", nbytes);
    if (parse_int(req1, "<status>", x)) {
        switch (x) {
        case -1: status = ERR_UPLOAD_PERMANENT; break;
        case 0: status = 0; break;
        case 1: status = ERR_UPLOAD_TRANSIENT; break;
        default: status = ERR_UPLOAD_TRANSIENT; break;
        }
    } else {
        status = ERR_UPLOAD_TRANSIENT;
    }

    if (parse_str(req1, "<message>", buf, sizeof(buf))) {
        msg_printf(fip->project, MSG_INTERNAL_ERROR, "Error on file upload: %s", buf);
    }
    if (log_flags.file_xfer_debug) {
        msg_printf(fip->project, MSG_INFO,
            "[file_xfer_debug] parsing upload response: %s", req1
        );
        msg_printf(fip->project, MSG_INFO,
            "[file_xfer_debug] parsing status: %d", status
        );
    }

    return status;
}

/// Create a new empty FILE_XFER_SET.
///
FILE_XFER_SET::FILE_XFER_SET(HTTP_OP_SET* p) {
    http_ops = p;
    up_active = false;
    down_active = false;
}

/// Start a FILE_XFER going (connect to server etc.)
/// If successful, add to the set.
int FILE_XFER_SET::insert(FILE_XFER* fxp) {
    int retval;

    retval = http_ops->insert(fxp);
    if (retval) return retval;
    file_xfers.push_back(fxp);
    set_bandwidth_limits(fxp->is_upload);
    return 0;
}

/// Remove a FILE_XFER object from the set.
int FILE_XFER_SET::remove(FILE_XFER* fxp) {
    vector<FILE_XFER*>::iterator iter;

    http_ops->remove(fxp);

    iter = file_xfers.begin();
    while (iter != file_xfers.end()) {
        if (*iter == fxp) {
            iter = file_xfers.erase(iter);
            set_bandwidth_limits(fxp->is_upload);
            return 0;
        }
        iter++;
    }
    msg_printf(fxp->fip->project, MSG_INTERNAL_ERROR,
            "File transfer for %s not found", fxp->fip->name.c_str());
    return ERR_NOT_FOUND;
}

/// Run through the FILE_XFER_SET and determine if any of the file
/// transfers are complete or had an error.
bool FILE_XFER_SET::poll() {
    unsigned int i;
    FILE_XFER* fxp;
    bool action = false;
    static double last_time=0;
    double size;

    if (gstate.now - last_time < 1.0) return false;
    last_time = gstate.now;

    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->http_op_done()) continue;

        action = true;
        fxp->file_xfer_done = true;
        if (log_flags.file_xfer_debug) {
            msg_printf(fxp->fip->project, MSG_INFO,
                "[file_xfer_debug] FILE_XFER_SET::poll(): http op done; retval %d\n",
                fxp->http_op_retval);
        }
        fxp->file_xfer_retval = fxp->http_op_retval;
        if (fxp->file_xfer_retval == 0) {
            if (fxp->is_upload) {
                fxp->file_xfer_retval = fxp->parse_upload_response(fxp->fip->upload_offset);
            }

            // If this was a file size query, restart the transfer
            // using the remote file size information
            if (fxp->file_size_query) {
                if (fxp->file_xfer_retval) {
                    fxp->fip->upload_offset = -1;
                } else {

                    // if the server's file size is bigger than ours,
                    // something bad has happened
                    // (like a result got sent to multiple users).
                    // Pretend the file was successfully uploaded
                    if (fxp->fip->upload_offset >= fxp->fip->nbytes) {
                        fxp->file_xfer_done = true;
                        fxp->file_xfer_retval = 0;
                    } else {
                        // Restart the upload, using the newly obtained
                        // upload_offset
                        fxp->close_socket();
                        fxp->file_xfer_retval = fxp->init_upload(*fxp->fip);

                        if (!fxp->file_xfer_retval) {
                            remove(fxp);
                            i--;
                            fxp->file_xfer_retval = insert(fxp);
                            if (!fxp->file_xfer_retval) {
                                fxp->file_xfer_done = false;
                                fxp->file_xfer_retval = 0;
                                fxp->http_op_retval = 0;
                            }
                        }
                    }
                }
            }
        } else if (fxp->file_xfer_retval == HTTP_STATUS_RANGE_REQUEST_ERROR) {
            fxp->fip->error_msg = "Local copy is at least as large as server copy";
        }

        // deal with various error cases for downloads
        if (!fxp->is_upload) {
            std::string pathname = get_pathname(fxp->fip);
            if (file_size(pathname.c_str(), size)) {
                continue;
            }
            double diff = size - fxp->starting_size;
            if (fxp->http_op_retval == 0) {
                // If no HTTP error,
                // see if we read less than 5 KB and file is incomplete.
                // If so truncate the amount read,
                // since it may be a proxy error message
                if (fxp->fip->nbytes) {
                    if (size == fxp->fip->nbytes) continue;
                    if (diff>0 && diff<MIN_DOWNLOAD_INCREMENT) {
                        msg_printf(fxp->fip->project, MSG_INFO,
                            "Incomplete read of %f < 5KB for %s - truncating",
                            diff, fxp->fip->name.c_str());
                        boinc_truncate(pathname.c_str(), fxp->starting_size);
                    }
                }
            } else {
                // got HTTP error; truncate last 5KB of file, since some
                // error-reporting HTML may have been appended
                if (diff < MIN_DOWNLOAD_INCREMENT) {
                    diff = 0;
                } else {
                    diff -= MIN_DOWNLOAD_INCREMENT;
                }
                boinc_truncate(pathname.c_str(), fxp->starting_size + diff);
            }
        }

        // for downloads: if we requested a partial transfer,
        // and the HTTP response is 200,
        // and the file is larger than it should be,
        // the server or proxy must have sent us the entire file
        // (i.e. it doesn't understand Range: requests).
        // In this case, trim off the initial part of the file
        if ((!fxp->is_upload) && (fxp->starting_size) && (fxp->response == HTTP_STATUS_OK)) {
            std::string pathname = get_pathname(fxp->fip);
            if (file_size(pathname.c_str(), size)) {
                continue;
            }
            if (size > fxp->fip->nbytes) {
                FILE* f1 = boinc_fopen(pathname.c_str(), "rb");
                if (!f1) {
                    fxp->file_xfer_retval = ERR_FOPEN;
                    msg_printf(fxp->fip->project, MSG_INTERNAL_ERROR,
                                "File size mismatch, can't open %s", pathname.c_str()); 
                    continue;
                }
                FILE* f2 = boinc_fopen(TEMP_FILE_NAME, "wb");
                if (!f2) {
                    msg_printf(fxp->fip->project, MSG_INTERNAL_ERROR,
                                "File size mismatch, can't open temp %s", TEMP_FILE_NAME);
                    fxp->file_xfer_retval = ERR_FOPEN;
                    fclose(f1);
                    continue;
                }
                fseek(f1, (long)fxp->starting_size, SEEK_SET);
                copy_stream(f1, f2);
                fclose(f1);
                fclose(f2);
                f1 = boinc_fopen(TEMP_FILE_NAME, "rb");
                f2 = boinc_fopen(pathname.c_str(), "wb");
                copy_stream(f1, f2);
                fclose(f1);
                fclose(f2);
            }
        }
    }
    return action;
}

/// Return true if an upload is currently in progress
/// or has been since the last call to this.
/// Similar for download.
///
void FILE_XFER_SET::check_active(bool& up, bool& down) {
    unsigned int i;
    FILE_XFER* fxp;

    up = up_active;
    down = down_active;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        fxp->is_upload?up=true:down=true;
    }
    up_active = false;
    down_active = false;
}

/// Adjust bandwidth limits.
///
void FILE_XFER_SET::set_bandwidth_limits(bool is_upload) {
    double max_bytes_sec;
    unsigned int i;
    FILE_XFER* fxp;

    if (is_upload) {
        max_bytes_sec = gstate.global_prefs.max_bytes_sec_up;
    } else {
        max_bytes_sec = gstate.global_prefs.max_bytes_sec_down;
    }
    if (!max_bytes_sec) return;
    int n = 0;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->is_active()) continue;
        if (is_upload) {
            if (!fxp->is_upload) continue;
        } else {
            if (fxp->is_upload) continue;
        }
        n++;
    }
    if (!n) return;
    max_bytes_sec /= n;
    for (i=0; i<file_xfers.size(); i++) {
        fxp = file_xfers[i];
        if (!fxp->is_active()) continue;
        if (is_upload) {
            if (!fxp->is_upload) continue;
            fxp->set_speed_limit(true, max_bytes_sec);
        } else {
            if (fxp->is_upload) continue;
            fxp->set_speed_limit(false, max_bytes_sec);
        }
    }
}
