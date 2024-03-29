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
/// initialization and starting of applications

#include "app.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "win_util.h"
#else
#include "config.h"
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <cerrno>
#include <sys/stat.h>
#endif

#include "version.h"

#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach/machine.h>
#include <libkern/OSByteOrder.h>
#endif

#ifndef _WIN32
#include <fcntl.h>
#endif

#include <cstring>
#include <sstream>
#include <fstream>

#include "filesys.h"
#include "error_numbers.h"
#include "util.h"
#include "str_util.h"
#include "shmem.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "sandbox.h"

#ifdef _WIN32
#include "proc_control.h"
#endif

#ifdef _WIN32
// Dynamically link to these functions at runtime;
// otherwise BOINC cannot run on Win98

// CreateEnvironmentBlock
typedef BOOL (WINAPI *tCEB)(LPVOID *lpEnvironment, HANDLE hToken, BOOL bInherit);
// DestroyEnvironmentBlock
typedef BOOL (WINAPI *tDEB)(LPVOID lpEnvironment);

#endif

#ifndef _WIN32
/// Goes through a list of strings, and prints each string.
///
/// \param[in] argv The list of strings that should get printed.
static void debug_print_argv(const std::list<std::string>& argv) {
    msg_printf(0, MSG_INFO, "[task_debug] Arguments:");
    int count = 0;
    for (std::list<std::string>::const_iterator it = argv.begin();
            it != argv.end(); ++it) {
        msg_printf(0, MSG_INFO,
            "[task_debug]    argv[%d]: %s\n", count++, (*it).c_str()
        );
    }
}
#endif

/// Make a unique key for core/app shared memory segment.
/// Windows: also create and attach to the segment.
///
/// \return 0 on success, #ERR_SHMEM_NAME or #ERR_SHMGET on error.
int ACTIVE_TASK::get_shmem_seg_name() {
#ifdef _WIN32
    int i;
    char seg_name[256];

    bool try_global = (sandbox_account_service_token != NULL);
    for (i=0; i<1024; i++) {
        sprintf(seg_name, "%sboinc_%d", SHM_PREFIX, i);
        shm_handle = create_shmem(seg_name, sizeof(SHARED_MEM), (void**)&app_client_shm.shm,
                                  try_global);
        if (shm_handle) {
            break;
        }
    }
    if (!shm_handle) {
        return ERR_SHMGET;
    }
    sprintf(shmem_seg_name, "boinc_%d", i);
#else
    // shmem_seg_name is not used with mmap() shared memory
    if (app_version->api_major_version() >= 6) {
        shmem_seg_name = -1;
        return 0;
    }
    std::string init_data_path = slot_dir + std::string("/") + std::string(INIT_DATA_FILE);

    // ftok() only works if there's a file at the given location
    //
    FILE* f = boinc_fopen(init_data_path.c_str(), "w");
    if (f) fclose(f);
    shmem_seg_name = ftok(init_data_path.c_str(), 1);
    if (shmem_seg_name == -1) return ERR_SHMEM_NAME;
#endif
    return 0;
}

/// Write the app init file.
/// This is done before starting the app,
/// and when project prefs have changed during app execution.
///
/// \return 0 on success, #ERR_FOPEN if the file could not be opened.
///
/// \todo This function was modified to use ofstream instead of boinc_fopen().
/// This means the error checking and retry mechanisms of boinc_fopen() aren't
/// used now. We may need to restore them. --NA
int ACTIVE_TASK::write_app_init_file() {
    APP_INIT_DATA aid;

    memset(&aid, 0, sizeof(aid));

    aid.major_version = BOINC_MAJOR_VERSION;
    aid.minor_version = BOINC_MINOR_VERSION;
    aid.release = BOINC_RELEASE;
    aid.app_version = app_version->version_num;
    safe_strcpy(aid.app_name, wup->app->name);
    safe_strcpy(aid.symstore, wup->project->symstore);
    safe_strcpy(aid.acct_mgr_url, gstate.acct_mgr_info.acct_mgr_url);
    if (wup->project->project_specific_prefs.length()) {
        aid.project_preferences = strdup(wup->project->project_specific_prefs.c_str());
    }
    aid.hostid = wup->project->hostid;
    safe_strcpy(aid.user_name, wup->project->user_name);
    safe_strcpy(aid.team_name, wup->project->team_name);

    std::string project_path = relative_to_absolute(get_project_dir(wup->project));
    strlcpy(aid.project_dir, project_path.c_str(), sizeof(aid.project_dir));

    std::string buf = relative_to_absolute("");
    strlcpy(aid.boinc_dir, buf.c_str(), sizeof(aid.boinc_dir));

    strcpy(aid.authenticator, wup->project->authenticator);
    aid.slot = slot;
    strcpy(aid.wu_name, wup->name);
    aid.user_total_credit = wup->project->user_total_credit;
    aid.user_expavg_credit = wup->project->user_expavg_credit;
    aid.host_total_credit = wup->project->host_total_credit;
    aid.host_expavg_credit = wup->project->host_expavg_credit;
    double rrs = gstate.runnable_resource_share();
    if (rrs) {
        aid.resource_share_fraction = wup->project->resource_share/rrs;
    } else {
        aid.resource_share_fraction = 1;
    }
    aid.rsc_fpops_est = wup->rsc_fpops_est;
    aid.rsc_fpops_bound = wup->rsc_fpops_bound;
    aid.rsc_memory_bound = wup->rsc_memory_bound;
    aid.rsc_disk_bound = wup->rsc_disk_bound;
    aid.computation_deadline = result->computation_deadline();
    aid.checkpoint_period = gstate.ncpus * gstate.global_prefs.disk_interval;
    aid.fraction_done_start = 0;
    aid.fraction_done_end = 1;
#ifdef _WIN32
    strcpy(aid.shmem_seg_name, shmem_seg_name);
#else
    aid.shmem_seg_name = shmem_seg_name;
#endif
    // wu_cpu_time is the CPU time at start of session,
    // not the checkpoint CPU time
    // At the start of an episode these are equal, but not in the middle!
    //
    aid.wu_cpu_time = episode_start_cpu_time;

    std::string init_data_path = slot_dir + std::string("/") + std::string(INIT_DATA_FILE);
    std::ofstream f(init_data_path.c_str(), std::ios::out);
    if (!f.is_open()) {
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "Failed to open init file %s",
            init_data_path.c_str()
        );
        return ERR_FOPEN;
    }

    aid.host_info = gstate.host_info;
    aid.global_prefs = gstate.global_prefs;
    aid.proxy_info = gstate.proxy_info;
    write_init_data_file(f, aid);
    return 0;
}

static int make_soft_link(const PROJECT* project, const char* link_path, const char* rel_file_path) {
    FILE *fp = boinc_fopen(link_path, "w");
    if (!fp) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "Can't create link file %s", link_path
        );
        return ERR_FOPEN;
    }
    fprintf(fp, "<soft_link>%s</soft_link>\n", rel_file_path);
    fclose(fp);
    return 0;
}

/// Set up a file reference, given a slot dir and project dir.
/// This means:
/// -# copy the file to slot dir, if reference is by copy
/// -# else make a soft link
static int setup_file(
    const PROJECT* project, const FILE_INFO* fip, const FILE_REF& fref,
    const std::string& file_path, const std::string& slot_dir, bool input
) {
    int retval;

    std::string link_path = slot_dir + std::string("/");
    if (strlen(fref.open_name)) {
        link_path += std::string(fref.open_name);
    } else {
        link_path += std::string(fip->name);
    }
    std::string rel_file_path = std::string("../../") + file_path;

    // if anonymous platform, this is called even if not first time,
    // so link may already be there
    //
    if (input && project->anonymous_platform && boinc_file_exists(link_path.c_str())) {
        return 0;
    }

    if (fref.copy_file) {
        if (input) {
            retval = boinc_copy(file_path.c_str(), link_path.c_str());
            if (retval) {
                msg_printf(project, MSG_INTERNAL_ERROR,
                    "Can't copy %s to %s: %s", file_path.c_str(), link_path.c_str(),
                    boincerror(retval)
                );
                return retval;
            }
        }
        return 0;
    }

#ifdef _WIN32
    retval = make_soft_link(project, link_path.c_str(), rel_file_path.c_str());
    if (retval) return retval;
#else
    if (project->use_symlinks) {
        retval = symlink(rel_file_path.c_str(), link_path.c_str());
    } else {
        retval = make_soft_link(project, link_path.c_str(), rel_file_path.c_str());
    }
    if (retval) return retval;
#endif
#ifdef SANDBOX
    return set_to_project_group(link_path.c_str());
#endif
    return 0;
}

int ACTIVE_TASK::link_user_files() {
    PROJECT* project = wup->project;
    unsigned int i;
    FILE_REF fref;
    FILE_INFO* fip;

    for (i=0; i<project->user_files.size(); i++) {
        fref = project->user_files[i];
        fip = fref.file_info;
        if (fip->status != FILE_PRESENT) continue;
        std::string file_path = get_pathname(fip);
        setup_file(project, fip, fref, file_path, slot_dir, true);
    }
    return 0;
}

int ACTIVE_TASK::copy_output_files() {
    for (size_t i = 0; i < result->output_files.size(); ++i) {
        FILE_REF& fref = result->output_files[i];
        if (!fref.copy_file) {
            continue;
        }
        const FILE_INFO* fip = fref.file_info;
        std::string slotfile = slot_dir + std::string("/") + std::string(fref.open_name);
        std::string projfile = get_pathname(fip);
        int retval = boinc_rename(slotfile.c_str(), projfile.c_str());
        if (retval) {
            msg_printf(wup->project, MSG_INTERNAL_ERROR, "Can't rename output file %s to %s: %s",
                    fip->name.c_str(), projfile.c_str(), boincerror(retval));
        }
    }
    return 0;
}

/// Start a task in a slot directory.
/// This includes setting up soft links,
/// passing preferences, and starting the process.
///
/// Current dir is top-level Synecdoche dir.
///
/// \post
/// - If any error occurs
///   - #task_state is #PROCESS_COULDNT_START
///   - CLIENT_STATE::report_result_error() is called
/// - else
///   - #task_state is #PROCESS_EXECUTING
///
/// \return 0 on success, nonzero otherwise.
int ACTIVE_TASK::start() {
    char exec_name[256], exec_path[256];
    unsigned int i;
    FILE_REF fref;
    int retval;
    // F*** goto, need to define some variables here instead of where they are used!
    std::ostringstream err_stream;
#ifdef _WIN32
    std::string cmd_line;
    std::string slotdirpath;
#else
    // Needs to be defined here because those gotos would skip the
    // initialization of 'cmdline' and 'argv' if it would be defined later.
    std::ostringstream cmdline;
    std::list<std::string> argv;
#endif
    if ((!full_init_done) && (log_flags.task)) {
        msg_printf(wup->project, MSG_INFO,
            "Starting %s", result->name
        );
    }
    if (log_flags.cpu_sched) {
        msg_printf(wup->project, MSG_INFO,
            "[cpu_sched] Starting %s%s", result->name, (full_init_done) ? " (resume)" : " (initial)"
        );
    }

    // Always check if all required files are present. If not, trigger
    // re-downloads and don't start the science application.
    FILE_INFO_PSET missing_file_infos;
    retval = gstate.input_files_available(result, true, &missing_file_infos);
    if (retval) {
        for (FILE_INFO_PSET::iterator it = missing_file_infos.begin(); it != missing_file_infos.end(); ++it) {
            FILE_INFO* fip = *it;
            if (fip) {
                err_stream << "Input file " << fip->name
                           << " missing or invalid: " << retval;
            } else {
                err_stream << "Input file missing or invalid";
                // We can't trigger a new download if we don't have
                // any file information. Just fail here as before.
                goto error;
            }
            fip->status = FILE_NOT_PRESENT;
        }
    }
    if (!missing_file_infos.empty()) {
        // Some files are missing and are set for re-transfer.
        // Update status and return without error.
        result->set_state(RESULT_FILES_DOWNLOADING, "start");
        set_task_state(PROCESS_UNINITIALIZED, "start");
        next_scheduler_state = PROCESS_UNINITIALIZED;
        return 0;
    }

    if (!full_init_done) {
        checkpoint_cpu_time = 0;
        checkpoint_wall_time = gstate.now;
    }
    current_cpu_time = checkpoint_cpu_time;
    episode_start_cpu_time = checkpoint_cpu_time;
    debt_interval_start_cpu_time = checkpoint_cpu_time;

    graphics_request_queue.init(result->name);        // reset message queues
    process_control_queue.init(result->name);

    if (!app_client_shm.shm) {
        retval = get_shmem_seg_name();
        if (retval) {
            err_stream << "Can't get shared memory segment name: " << boincerror(retval);
            goto error;
        }
    }

    // this must go AFTER creating shmem name,
    // since the shmem name is part of the file
    //
    retval = write_app_init_file();
    if (retval) {
        err_stream << "Can't write init file: " << retval;
        goto error;
    }

    // set up applications files
    //
    strcpy(exec_name, "");
    for (i=0; i<app_version->app_files.size(); i++) {
        fref = app_version->app_files[i];
        FILE_INFO* fip = fref.file_info;
        std::string file_path = get_pathname(fip);
        if (fref.main_program) {
            if (is_image_file(fip->name)) {
                err_stream << "Main program " << fip->name << " is an image file";
                retval = ERR_NO_SIGNATURE;
                goto error;
            }
            if (!fip->executable && !wup->project->anonymous_platform) {
                err_stream << "Main program " << fip->name << " is not executable";
                retval = ERR_NO_SIGNATURE;
                goto error;
            }
            safe_strcpy(exec_name, fip->name.c_str());
            safe_strcpy(exec_path, file_path.c_str());
        }
        // anonymous platform may use different files than
        // when the result was started, so link files even if not first time
        if ((!full_init_done) || (wup->project->anonymous_platform)) {
            retval = setup_file(result->project, fip, fref, file_path, slot_dir, true);
            if (retval) {
                err_stream << "Can't link input file";
                goto error;
            }
        }
    }
    if (!strlen(exec_name)) {
        err_stream << "No main program specified";
        retval = ERR_NOT_FOUND;
        goto error;
    }

    // set up input, output files
    if (!full_init_done) {
        for (i=0; i<wup->input_files.size(); i++) {
            fref = wup->input_files[i];
            const FILE_INFO* fip = fref.file_info;
            std::string file_path = get_pathname(fref.file_info);
            retval = setup_file(result->project, fip, fref, file_path, slot_dir, true);
            if (retval) {
                err_stream << "Can't link input file";
                goto error;
            }
        }
        for (i=0; i<result->output_files.size(); i++) {
            fref = result->output_files[i];
            if (fref.copy_file) continue;
            const FILE_INFO* fip = fref.file_info;
            std::string file_path = get_pathname(fref.file_info);
            retval = setup_file(result->project, fip, fref, file_path, slot_dir, false);
            if (retval) {
                err_stream << "Can't link output file";
                goto error;
            }
        }
        full_init_done = true;
    }

    link_user_files();

    if (gstate.exit_before_start) {
        exit(0);
    }

#ifdef _WIN32
    PROCESS_INFORMATION process_info;
    STARTUPINFO startup_info;
    LPVOID environment_block = NULL;
    char error_msg[1024];
    char error_msg2[1024];

    memset(&process_info, 0, sizeof(process_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    // suppress 2-sec rotating hourglass cursor on startup
    //
    startup_info.dwFlags = STARTF_FORCEOFFFEEDBACK;

    app_client_shm.reset_msgs();

    if (config.run_apps_manually) {
        // fill in core client's PID so we won't think app has exited
        pid = GetCurrentProcessId();
        pid_handle = GetCurrentProcess();
        set_task_state(PROCESS_EXECUTING, "start");
        return 0;
    }
    // NOTE: in Windows, stderr is redirected in boinc_init_diagnostics();

    cmd_line = exec_path + std::string(" ") + wup->command_line;
    if (strlen(app_version->cmdline)) {
        cmd_line += std::string(" ") + app_version->cmdline;
    }
    slotdirpath = relative_to_absolute(slot_dir);
    bool success = false;

    for (i=0; i<5; i++) {
        if (sandbox_account_service_token != NULL) {
            // Find CreateEnvironmentBlock/DestroyEnvironmentBlock pointers
            tCEB    pCEB = NULL;
            tDEB    pDEB = NULL;
            HMODULE hUserEnvLib = NULL;

            hUserEnvLib = LoadLibrary("userenv.dll");
            if (hUserEnvLib) {
                pCEB = (tCEB) GetProcAddress(hUserEnvLib, "CreateEnvironmentBlock");
                pDEB = (tDEB) GetProcAddress(hUserEnvLib, "DestroyEnvironmentBlock");
            }

            if (!pCEB(&environment_block, sandbox_account_service_token, FALSE)) {
                if (log_flags.task) {
                    windows_error_string(error_msg, sizeof(error_msg));
                    msg_printf(wup->project, MSG_INFO,
                        "Process environment block creation failed: %s", error_msg
                    );
                }
            }

            if (CreateProcessAsUser(
                sandbox_account_service_token,
                exec_path,
                (LPSTR)cmd_line.c_str(),
                NULL,
                NULL,
                FALSE,
                CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS|CREATE_UNICODE_ENVIRONMENT,
                environment_block,
                slotdirpath.c_str(),
                &startup_info,
                &process_info
            )) {
                success = true;
                break;
            } else {
                windows_error_string(error_msg, sizeof(error_msg));
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Process creation failed: %s", error_msg
                );
            }

            if (!pDEB(environment_block)) {
                if (log_flags.task) {
                    windows_error_string(error_msg, sizeof(error_msg2));
                    msg_printf(wup->project, MSG_INFO,
                        "Process environment block cleanup failed: %s",
                        error_msg2
                    );
                }
            }

            if (hUserEnvLib) {
                pCEB = NULL;
                pDEB = NULL;
                FreeLibrary(hUserEnvLib);
            }

        } else {
            if (CreateProcess(
                exec_path,
                (LPSTR)cmd_line.c_str(),
                NULL,
                NULL,
                FALSE,
                CREATE_NEW_PROCESS_GROUP|CREATE_NO_WINDOW|IDLE_PRIORITY_CLASS,
                NULL,
                slotdirpath.c_str(),
                &startup_info,
                &process_info
            )) {
                success = true;
                break;
            } else {
                windows_error_string(error_msg, sizeof(error_msg));
                msg_printf(wup->project, MSG_INTERNAL_ERROR,
                    "Process creation failed: %s", error_msg
                );
            }
        }
        boinc_sleep(drand());
    }

    if (!success) {
        err_stream << "CreateProcess() failed - " << error_msg;
        retval = ERR_EXEC;
        goto error;
    }
    pid = process_info.dwProcessId;
    pid_handle = process_info.hProcess;
    CloseHandle(process_info.hThread);  // thread handle is not used
#else
    // Unix/Linux/Mac case

    // Set up core/app shared memory seg if needed
    //
    if (!app_client_shm.shm) {
        if (app_version->api_major_version() >= 6) {
            // Use mmap() shared memory
            std::string buf = slot_dir + std::string("/") + std::string(MMAPPED_FILE_NAME);
            if (g_use_sandbox) {
                if (!boinc_file_exists(buf.c_str())) {
                    int fd = open(buf.c_str(), O_RDWR | O_CREAT, 0660);
                    if (fd >= 0) {
                        close (fd);
#ifdef SANDBOX
                        set_to_project_group(buf.c_str());
#endif
                    }
                }
            }
            retval = create_shmem_mmap(
                buf.c_str(), sizeof(SHARED_MEM), (void**)&app_client_shm.shm
            );
        } else {
            // Use shmget() shared memory
            retval = create_shmem(
                shmem_seg_name, sizeof(SHARED_MEM), gstate.boinc_project_gid,
                (void**)&app_client_shm.shm
            );

            if (retval) {
                needs_shmem = true;
                destroy_shmem(shmem_seg_name);
                return retval;
            }
        }
        needs_shmem = false;
    }
    app_client_shm.reset_msgs();

#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
    // PowerPC apps emulated on i386 Macs crash if running graphics
    powerpc_emulated_on_i386 = ! is_native_i386_app(exec_path);
#endif
    if (config.run_apps_manually) {
        pid = getpid();     // use the client's PID
        set_task_state(PROCESS_EXECUTING, "start");
        return 0;
    }

    // Prepare command line for the science app:
    cmdline << wup->command_line;
    if (strlen(app_version->cmdline)) {
        cmdline << ' ' << app_version->cmdline;
    }
    argv = parse_command_line(cmdline.str().c_str());
    if (log_flags.task_debug) {
        debug_print_argv(argv);
    }

    pid = fork();
    if (pid == -1) {
        err_stream << "fork() failed: " << strerror(errno);
        retval = ERR_FORK;
        goto error;
    }
    if (pid == 0) {
        // from here on we're running in a new process.
        // If an error happens,
        // exit nonzero so that the core client knows there was a problem.

        // don't pass stdout to the app
        //
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDOUT_FILENO);
        close(fd);

        // add to library path:
        // - the project dir (../../projects/X)
        // - the slot dir (.)
        // - the Synecdoche dir (../..)
        // We use relative paths in case higher-level dirs
        // are not readable to the account under which app runs
        //
        std::string pdir = get_project_dir(wup->project);

        std::ostringstream libpath;
        const char* env_lib_path = getenv("LD_LIBRARY_PATH");
        if (env_lib_path) {
            libpath << env_lib_path << ':';
        }
        libpath << "../../" << pdir << ":.:../..";
        setenv("LD_LIBRARY_PATH", libpath.str().c_str(), 1);

        retval = chdir(slot_dir.c_str());
        if (retval) {
            perror("chdir");
            fflush(NULL);
            _exit(errno);
        }

#if 0
        // set stack size limit to the max.
        // Some BOINC apps have reported problems with exceeding
        // small stack limits (e.g. 8 MB)
        // and it seems like the best thing to raise it as high as possible
        //
        struct rlimit rlim;
#define MIN_STACK_LIMIT 64000000
        getrlimit(RLIMIT_STACK, &rlim);
        if (rlim.rlim_cur != RLIM_INFINITY && rlim.rlim_cur <= MIN_STACK_LIMIT) {
            if (rlim.rlim_max == RLIM_INFINITY || rlim.rlim_max > MIN_STACK_LIMIT) {
                rlim.rlim_cur = MIN_STACK_LIMIT;
            } else {
                rlim.rlim_cur = rlim.rlim_max;
            }
            setrlimit(RLIMIT_STACK, &rlim);
        }
#endif

        // hook up stderr to a specially-named file
        //
        freopen(STDERR_FILE, "a", stderr);

        // set idle process priority
#ifdef HAVE_SETPRIORITY
        if (setpriority(PRIO_PROCESS, 0, PROCESS_IDLE_PRIORITY)) {
            perror("setpriority");
        }
#endif
        std::string path = std::string("../../") + std::string(exec_path);
        if (g_use_sandbox) {
            std::ostringstream switcher_path;
            switcher_path << "../../" << SWITCHER_DIR << '/' << SWITCHER_FILE_NAME;
            argv.push_front(exec_name);
            argv.push_front(path);
            argv.push_front(SWITCHER_FILE_NAME);
            // Files written by projects have user boinc_project and group boinc_project,
            // so they must be world-readable so Synecdoche can read them.
            umask(2);
            retval = do_execv(switcher_path.str(), argv);
        } else {
            argv.push_front(exec_name);
            retval = do_execv(path, argv);
        }
        msg_printf(wup->project, MSG_INTERNAL_ERROR,
            "Process creation (%s) failed: %s, errno=%d\n", path.c_str(), boincerror(retval), errno
        );
        perror("execv");
        fflush(NULL);
        _exit(errno);
    }

    if (log_flags.task_debug) {
        msg_printf(wup->project, MSG_INFO,
            "[task_debug] ACTIVE_TASK::start(): forked process: pid %d\n", pid
        );
    }

#endif
    set_task_state(PROCESS_EXECUTING, "start");
    return 0;

    // go here on error; "error_msg" contains error message, "retval" is nonzero
    //
error:
    // if something failed, it's possible that the executable was munged.
    // Verify it to trigger another download.
    //
    gstate.input_files_available(result, true);
    gstate.report_result_error(*result, "%s", err_stream.str().c_str());
    set_task_state(PROCESS_COULDNT_START, "start");
    return retval;
}

/// Resume the task if it was previously running; otherwise start it.
///
/// \post #task_state is set correctly.
///
/// \param[in] first_time Set this to true if the app
///                       will be started for the first time.
/// \return 0 on success, nonzero otherwise.
int ACTIVE_TASK::resume_or_start(bool first_time) {
    const char* str = "??";
    int retval;

    switch (task_state()) {
    case PROCESS_UNINITIALIZED:
        if (first_time) {
            retval = start();
            str = "Starting";
        } else {
            retval = start();
            str = "Restarting";
        }
        if (task_state() == PROCESS_UNINITIALIZED) {
            // Starting failed because of missing files. Don't treat this
            // as an error because this would terminate the result. Just
            // return here to suppress the start-message.
            return 0;
        }
        if ((retval == ERR_SHMGET) || (retval == ERR_SHMAT)) {
            return retval;
        }
        if (retval) {
            set_task_state(PROCESS_COULDNT_START, "resume_or_start1");
            return retval;
        }
        break;
    case PROCESS_SUSPENDED:
        retval = unsuspend();
        if (retval) {
            msg_printf(wup->project, MSG_INTERNAL_ERROR,
                "Couldn't resume task %s", result->name
            );
            set_task_state(PROCESS_COULDNT_START, "resume_or_start2");
            return retval;
        }
        str = "Resuming";
        break;
    default:
        msg_printf(result->project, MSG_INTERNAL_ERROR,
            "Unexpected state %d for task %s", task_state(), result->name
        );
        return 0;
    }
    if (log_flags.task) {
        msg_printf(result->project, MSG_INFO,
            "%s task %s using %s version %d",
            str,
            result->name,
            app_version->app->name,
            app_version->version_num
        );
    }
    return 0;
}

#if (defined (__APPLE__) && (defined(__i386__) || defined(__x86_64__)))

union headeru {
    fat_header fat;
    mach_header mach;
};

/// Read the mach-o headers to determine the architectures
/// supported by executable file.
/// Returns 1 if application can run natively on i386 / x86_64 Macs, else returns 0.
int ACTIVE_TASK::is_native_i386_app(const char* exec_path) const {
    FILE *f;
    int result = 0;

    headeru myHeader;
    fat_arch fatHeader;

    uint32_t n, i, len;
    uint32_t theMagic;
    integer_t theType;

    f = boinc_fopen(exec_path, "rb");
    if (!f) {
        return result;          // Should never happen
    }

    myHeader.fat.magic = 0;
    myHeader.fat.nfat_arch = 0;

    fread(&myHeader, 1, sizeof(fat_header), f);
    theMagic = myHeader.mach.magic;
    switch (theMagic) {
    case MH_CIGAM:
    case MH_MAGIC:
    case MH_MAGIC_64:
    case MH_CIGAM_64:
       theType = myHeader.mach.cputype;
        if ((theMagic == MH_CIGAM) || (theMagic == MH_CIGAM_64)) {
            theType = OSSwapInt32(theType);
        }
        if ((theType == CPU_TYPE_I386) || (theType == CPU_TYPE_X86_64)) {
            result = 1;        // Single-architecture i386or x86_64 file
        }
        break;
    case FAT_MAGIC:
    case FAT_CIGAM:
        n = myHeader.fat.nfat_arch;
        if (theMagic == FAT_CIGAM) {
            n = OSSwapInt32(myHeader.fat.nfat_arch);
        }
           // Multiple architecture (fat) file
        for (i=0; i<n; i++) {
            len = fread(&fatHeader, 1, sizeof(fat_arch), f);
            if (len < sizeof(fat_arch)) {
                break;          // Should never happen
            }
            theType = fatHeader.cputype;
            if (theMagic == FAT_CIGAM) {
                theType = OSSwapInt32(theType);
            }
            if ((theType == CPU_TYPE_I386) || (theType == CPU_TYPE_X86_64)) {
                result = 1;
                break;
            }
        }
        break;
    default:
        break;
    }

    fclose (f);
    return result;
}
#endif
