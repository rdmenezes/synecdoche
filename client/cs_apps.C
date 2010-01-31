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

/// \file
/// The "policy" part of task execution is here.
/// The "mechanism" part is in app.C

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cassert>
#include <csignal>
#endif

#include "client_state.h"
#include "md5_file.h"
#include "util.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "log_flags.h"
#include "client_msgs.h"

using std::vector;

/// clean up after finished apps.
bool CLIENT_STATE::handle_finished_apps() {
    ACTIVE_TASK* atp;
    bool action = false;
    static double last_time = 0;
    if (now - last_time < 1.0) return false;
    last_time = now;

    vector<ACTIVE_TASK*>::iterator iter;

    iter = active_tasks.active_tasks.begin();
    while (iter != active_tasks.active_tasks.end()) {
        atp = *iter;
        switch (atp->task_state()) {
        case PROCESS_EXITED:
        case PROCESS_WAS_SIGNALED:
        case PROCESS_EXIT_UNKNOWN:
        case PROCESS_COULDNT_START:
        case PROCESS_ABORTED:
            if (log_flags.task) {
                msg_printf(atp->wup->project, MSG_INFO,
                    "Computation for task %s finished", atp->result->name
                );
            }
            app_finished(*atp);
            iter = active_tasks.active_tasks.erase(iter);
            delete atp;
            set_client_state_dirty("handle_finished_apps");

            // the following is critical; otherwise the result is
            // still in the "scheduled" list and enforce_schedule()
            // will try to run it again.
            //
            request_schedule_cpus("handle_finished_apps");
            action = true;
            break;
        default:
            iter++;
        }
    }
    return action;
}

/// Handle a task that has finished.
/// Mark its output files as present, and delete scratch files.
/// Don't delete input files because they might be shared with other WUs.
/// Update state of result record.
int CLIENT_STATE::app_finished(ACTIVE_TASK& at) {
    RESULT* rp = at.result;
    bool had_error = false;

    FILE_INFO* fip;
    unsigned int i;
    int retval;
    double size;

    // scan the output files, check if missing or too big.
    // Don't bother doing this if result was aborted via GUI or by project
    //
    switch (rp->exit_status) {
    case ERR_ABORTED_VIA_GUI:
    case ERR_ABORTED_BY_PROJECT:
        break;
    default:
        for (i=0; i<rp->output_files.size(); i++) {
            FILE_REF& fref = rp->output_files[i];
            fip = fref.file_info;
            if (fip->uploaded) {
                continue;
            }
            std::string path = get_pathname(fip);
            retval = file_size(path.c_str(), size);
            if (retval) {
                if (fref.optional) {
                    fip->upload_when_present = false;
                    continue;
                }

                // an output file is unexpectedly absent.
                //
                fip->status = retval;
                had_error = true;
                msg_printf(rp->project, MSG_INFO, "Output file %s for task %s absent",
                        fip->name.c_str(), rp->name);
            } else if (size > fip->max_nbytes) {
                // Note: this is only checked when the application finishes.
                // The total disk space is checked while the application is running.
                msg_printf(rp->project, MSG_INFO, "Output file %s for task %s exceeds size limit.",
                        fip->name.c_str(), rp->name);
                msg_printf(rp->project, MSG_INFO, "File size: %f bytes.  Limit: %f bytes",
                        size, fip->max_nbytes);

                fip->delete_file();
                fip->status = ERR_FILE_TOO_BIG;
                had_error = true;
            } else {
                if (!fip->upload_when_present && !fip->sticky) {
                    fip->delete_file();     // sets status to NOT_PRESENT
                } else {
                    retval = 0;
                    if (fip->gzip_when_done) {
                        retval = fip->gzip();
                    }
                    if (!retval) {
                        retval = md5_file(path.c_str(), fip->md5_cksum, fip->nbytes);
                    }
                    if (retval) {
                        fip->status = retval;
                        had_error = true;
                    } else {
                        fip->status = FILE_PRESENT;
                    }
                }
            }
        }
    }

    if (rp->exit_status != 0) {
        had_error = true;
    }

    if (had_error) {
        switch (rp->exit_status) {
        case ERR_ABORTED_VIA_GUI:
        case ERR_ABORTED_BY_PROJECT:
            rp->set_state(RESULT_ABORTED, "CS::app_finished");
            break;
        default:
            rp->set_state(RESULT_COMPUTE_ERROR, "CS::app_finished");
        }
    } else {
        rp->set_state(RESULT_FILES_UPLOADING, "CS::app_finished");
        rp->project->update_duration_correction_factor(rp);
    }

    double wall_cpu_time = now - debt_interval_start;
    at.result->project->wall_cpu_time_this_debt_interval += wall_cpu_time;
    total_wall_cpu_time_this_debt_interval += wall_cpu_time;
    total_cpu_time_this_debt_interval += at.current_cpu_time - at.debt_interval_start_cpu_time;

    return 0;
}

/// Check if all the input files for a result are present
/// (both WU and app version).
/// Called from CLIENT_STATE::update_results (with verify=false)
/// to transition result from DOWNLOADING to DOWNLOADED.
/// Called from ACTIVE_TASK::start() (with verify=true) to check
/// if all required files are in place before starting the science
/// application.
///
/// If fip_vec is non-null, store all missing files in this vector.
/// Otherwise, stop as soon as the first missing file was discovered.
///
/// \param[in] rp Result for which the files should be checked.
/// \param[in] verify If true, strict validation (i.e. signature checking)
///                   is performed
/// \param[out] fip_set Optional pointer to a set that will receive
///                     a FILE_INFO pointer for each missing file.
int CLIENT_STATE::input_files_available(const RESULT* rp, bool verify, FILE_INFO_PSET* fip_set) {
    WORKUNIT* wup = rp->wup;
    FILE_INFO* fip;
    FILE_REF fr;
    PROJECT* project = rp->project;
    int result = 0;

    APP_VERSION* avp = rp->avp;
    for (size_t i = 0; i < avp->app_files.size(); ++i) {
        fr = avp->app_files[i];
        fip = fr.file_info;
        if (fip->status != FILE_PRESENT) {
            if (fip_set) {
                fip_set->insert(fip);
                result = ERR_FILE_MISSING;
            } else {
                return ERR_FILE_MISSING;
            }
        } else if (!project->anonymous_platform) {
            // Only verify files marked as present.
            // Don't verify app files if using anonymous platform.
            int retval = fip->verify_file(verify, true);
            if (retval) {
                if (fip_set) {
                    fip_set->insert(fip);
                    result = retval;
                } else {
                    return retval;
                }
            }
        }
    }

    for (size_t i = 0; i < wup->input_files.size(); ++i) {
        fip = wup->input_files[i].file_info;
        if (fip->generated_locally) continue;
        if (fip->status != FILE_PRESENT) {
            if (fip_set) {
                fip_set->insert(fip);
                result = ERR_FILE_MISSING;
            } else {
                return ERR_FILE_MISSING;
            }
        } else {
            // Only verify files marked as present.
            int retval = fip->verify_file(verify, true);
            if (retval) {
                if (fip_set) {
                    fip_set->insert(fip);
                    result = retval;
                } else {
                    return retval;
                }
            }
        }
    }
    return result;
}

inline double force_fraction(double f) {
    if (f < 0) return 0;
    if (f > 1) return 1;
    return f;
}

double CLIENT_STATE::get_fraction_done(const RESULT* result) {
    const ACTIVE_TASK* atp = active_tasks.lookup_result(result);
    return atp ? force_fraction(atp->fraction_done) : 0.0;
}

/// Find latest version of app for given platform
/// or -1 if can't find one.
///
/// \param[in] app Pointer to an APP instance which contains the requested application.
/// \param[in] platform A string containing the name of the requested platform.
/// \return The latest version of the requested application or -1 if no version
///         was found for the requested platform.
int CLIENT_STATE::latest_version(const APP* app, const std::string& platform) {
    unsigned int i;
    int best = -1;

    for (i=0; i<app_versions.size(); i++) {
        const APP_VERSION* avp = app_versions[i];
        if (avp->app != app) continue;
        if (platform != std::string(avp->platform)) continue;
        if (avp->version_num < best) continue;
        best = avp->version_num;
    }
    return best;
}

/// Find the ACTIVE_TASK in the current set with the matching PID.
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return NULL;
}

/// Find the ACTIVE_TASK in the current set with the matching result.
ACTIVE_TASK* ACTIVE_TASK_SET::lookup_result(const RESULT* result) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->result == result) {
            return atp;
        }
    }
    return NULL;
}
