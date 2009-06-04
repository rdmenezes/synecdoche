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

#ifndef _CLIENT_STATE_
#define _CLIENT_STATE_

#ifndef _WIN32
#include <string>
#include <vector>
#include <ctime>
#endif

#include "acct_mgr.h"
#include "acct_setup.h"
#include "app.h"
#include "client_types.h"
#include "file_xfer.h"
#include "gui_rpc_server.h"
#include "gui_http.h"
#include "hostinfo.h"
#include "net_stats.h"
#include "prefs.h"
#include "time_stats.h"
#include "http_curl.h"

class SCHEDULER_OP;
class PERS_FILE_XFER_SET;
class PERS_FILE_XFER;

// project: suspended, deferred, or no new work (can't ask for more work)
// overall: not work_fetch_ok (from CPU policy)
#define WORK_FETCH_DONT_NEED 0
// project: has more than min queue * share, not suspended/def/nonewwork
// overall: at least min queue, work fetch OK
#define WORK_FETCH_OK        1
// project: less than min queue * resource share of DL/runnable results
// overall: less than min queue
#define WORK_FETCH_NEED      2
// project: no downloading or runnable results
// overall: at least one idle CPU
#define WORK_FETCH_NEED_IMMEDIATELY 3

/// CLIENT_STATE encapsulates the global variables of the core client.
/// If you add anything here, initialize it in the constructor.
class CLIENT_STATE {
public:
    std::vector<PLATFORM> platforms;
    std::vector<PROJECT*> projects;
    std::vector<APP*> apps;
    FILE_INFO_PVEC file_infos;
    std::vector<APP_VERSION*> app_versions;
    WORKUNIT_PVEC workunits;
    RESULT_PVEC results;

    PERS_FILE_XFER_SET* pers_file_xfers;
    HTTP_OP_SET* http_ops;
    FILE_XFER_SET* file_xfers;
    ACTIVE_TASK_SET active_tasks;
    HOST_INFO host_info;
    GLOBAL_PREFS global_prefs;
    NET_STATS net_stats;
    GUI_RPC_CONN_SET gui_rpcs;
    TIME_STATS time_stats;
    PROXY_INFO proxy_info;
    GUI_HTTP gui_http;

    VERSION_INFO core_client_version;
    VERSION_INFO boinc_compat_version;
    std::string statefile_platform_name;
    int file_xfer_giveup_period;
    MODE run_mode;
    MODE network_mode;
    bool started_by_screensaver;
    bool exit_when_idle;
    bool exit_before_start;
    bool exit_after_finish;
    bool check_all_logins;
    bool user_active;       ///< there has been recent mouse/kbd input
    bool allow_remote_gui_rpc;
    int cmdline_gui_rpc_port;
    bool requested_exit;
    /// venue from project that gave us general prefs
    /// or from account manager
    char main_host_venue[256];
    bool exit_before_upload; ///< exit when about to upload a file
#ifndef _WIN32
    gid_t boinc_project_gid;
#endif

    /// \name Backoff-related variables
    ///@{

    /// fetch project's master URL (and stop doing scheduler RPCs)
    /// if get this many successive RPC failures (default 10)
    int master_fetch_period;
    /// cap project->nrpc_failures at this number
    int retry_cap;
    /// after this many master-fetch failures,
    /// move into a state in which we retry master fetch
    /// at the frequency below
    int master_fetch_retry_cap;
    int master_fetch_interval;
    ///@}

    int sched_retry_delay_min;
    int sched_retry_delay_max;
    int pers_retry_delay_min;
    int pers_retry_delay_max;
    int pers_giveup;

    bool tasks_suspended; ///< Don't do CPU. See check_suspend_activities for logic
    bool network_suspended; ///< Don't do network. See check_suspend_network for logic
    int suspend_reason;
    int network_suspend_reason;
    /// \c true if \c --daemon is on the commandline.
    /// This means we are running as a daemon on Unix,
    /// or as a service on Windows.
    bool executing_as_daemon;
    bool redirect_io; ///< redirect stdout, stderr to log files
    /// A condition has occurred in which we know graphics will
    /// not be displayable.
    /// So GUIs shouldn't offer graphics.
    bool disable_graphics;
    bool detach_console;
    bool launched_by_manager;
    double now;
    double client_start_time;
    double last_wakeup_time;
    bool initialized;
    /// Failed to write state file.
    /// In this case we continue to run for 1 minute,
    /// handling GUI RPCs but doing nothing else,
    /// so that the Manager can tell the user what the problem is.
    bool cant_write_state_file;
private:
    bool client_state_dirty;
    int old_major_version;
    int old_minor_version;
    int old_release;
    bool skip_cpu_benchmarks; ///< if set, use hardwired numbers rather than running benchmarks
    bool run_cpu_benchmarks; ///< if set, run benchmarks on client startup
    /// Set if a benchmark fails to start because of a process that doesn't stop.
    /// Persists so that benchmarks are run at the next start.
    bool cpu_benchmarks_pending;

    int exit_after_app_start_secs; ///< if nonzero, exit this many seconds after starting an app
    double app_started; ///< when the most recent app was started

/// @name acct_mgr.C
public:
    ACCT_MGR_OP acct_mgr_op;
    ACCT_MGR_INFO acct_mgr_info;
/// @}

/// @name acct_setup.C
public:
    PROJECT_INIT project_init;
    PROJECT_ATTACH project_attach;
    LOOKUP_WEBSITE_OP lookup_website_op;

#ifdef ENABLE_UPDATE_CHECK
    GET_CURRENT_VERSION_OP get_current_version_op;
    void new_version_check();
    double new_version_check_time;
    std::string newer_version;
#endif

/// @}

/// @name client_state.C
public:
    CLIENT_STATE();
    void show_host_info();
    void show_proxy_info();
    int init();

    /// Poll the client's finite-state machines.
    bool poll_slow_events();

    void do_io_or_sleep(double sec);
    bool time_to_exit() const;
    PROJECT* lookup_project(const char* master_url);
    APP* lookup_app(const PROJECT* project, const char* name);
    FILE_INFO* lookup_file_info(const PROJECT* project, const std::string& name);
    RESULT* lookup_result(const PROJECT* project, const char* name);
    WORKUNIT* lookup_workunit(const PROJECT* project, const char* name);
    APP_VERSION* lookup_app_version(const APP* app, const char* platform, int ver, const char* plan_class);

    /// "Detach" a project.
    int detach_project(PROJECT* project);

    /// Call this when a result has a nonrecoverable error.
    int report_result_error(RESULT& res, const char *format, ...);

    int reset_project(PROJECT* project, bool detaching);
    bool no_gui_rpc;
private:
    int link_app(PROJECT* p, APP* app);
    int link_file_info(PROJECT* p, FILE_INFO* fip);
    int link_file_ref(PROJECT* p, FILE_REF* file_refp);
    int link_app_version(PROJECT* p, APP_VERSION* avp);
    int link_workunit(PROJECT* p, WORKUNIT* wup);
    int link_result(PROJECT* p, RESULT* rp);
    void print_summary() const;
    bool garbage_collect();
    bool garbage_collect_always();

    /// Perform state transitions for results.
    bool update_results();

    int nresults_for_project(const PROJECT* project) const;
    void check_clock_reset();
    
    /// Abort all jobs that are not started yet but already missed their deadline.
    bool abort_unstarted_late_jobs();
/// @}

/// @name cpu_sched.C
private:
    double debt_interval_start;
    double total_wall_cpu_time_this_debt_interval; ///< "wall CPU time" accumulated since last adjust_debts()
    double total_cpu_time_this_debt_interval;
    double cpu_shortfall;
    bool work_fetch_no_new_work;
    bool must_enforce_cpu_schedule;
    bool must_schedule_cpus;
    bool must_check_work_fetch;
    std::vector <RESULT*> ordered_scheduled_results;
    void assign_results_to_projects();
    RESULT* largest_debt_project_best_result();
    RESULT* earliest_deadline_result();
    void reset_debt_accounting();
    void adjust_debts();
    bool possibly_schedule_cpus();
    void schedule_cpus();
    bool enforce_schedule();
    bool no_work_for_a_cpu();
    void make_running_task_heap(std::vector<ACTIVE_TASK*>& running_tasks, double& ncpus_used);
public:
    /// If we fail to start a task due to no shared-mem segments,
    /// wait until at least this time to try running
    /// another task that needs a shared-mem seg
    double retry_shmem_time;
    inline double work_buf_min() {
        return global_prefs.work_buf_min_days * 86400;
    }
    inline double work_buf_additional() {
        return global_prefs.work_buf_additional_days *86400;
    }
    inline double work_buf_total() {
        double x = work_buf_min() + work_buf_additional();
        if (x < 1) x = 1;
        return x;
    }
    void request_enforce_schedule(const char* where);
    /// Check for reschedule CPUs ASAP.  Called when:
    /// - core client starts (CLIENT_STATE::init())
    /// - an app exits (ACTIVE_TASK_STATE::check_app_exited())
    /// - Tasks are killed (ACTIVE_TASK_STATE::exit_tasks())
    /// - a result's input files finish downloading (CLIENT_STATE::update_results())
    /// - an app fails to start (CLIENT_STATE::schedule_cpus())
    /// - any project op is done via RPC (suspend/resume)
    /// - any result op is done via RPC (suspend/resume)
    void request_schedule_cpus(const char* where);
    ACTIVE_TASK* lookup_active_task_by_result(const RESULT* result);
/// @}

/// @name cs_account.C
public:
    /// Add a project.
    int add_project(const char* master_url, const char* _auth, const char* project_name, bool attached_via_acct_mgr);
private:
    int parse_account_files();
    int parse_account_files_venue();
    int parse_preferences_for_user_files();
    int parse_statistics_files();
        // should be move to a new file, but this will do it for testing
/// @}

/// @name cs_apps.C
private:
    double total_resource_share();
    double potentially_runnable_resource_share();
    double nearly_runnable_resource_share();
    double fetchable_resource_share();
public:
    double runnable_resource_share();
    int quit_activities();
    void set_ncpus();
    double get_fraction_done(RESULT* result);

    /// Check if all the input files for a result are present.
    int input_files_available(const RESULT* rp, bool verify, FILE_INFO_PSET* fip_set = 0);

    int ncpus; ///< number of usable cpus
private:
    int nslots;

    int latest_version(APP* app, const char* platform);
    int app_finished(ACTIVE_TASK& at);
    bool start_apps();
    bool handle_finished_apps();
public:
    ACTIVE_TASK* get_task(RESULT*);
/// @}

/// @name cs_benchmark.C
public:
    bool should_run_cpu_benchmarks();
    void start_cpu_benchmarks();
    bool cpu_benchmarks_poll();
    void abort_cpu_benchmarks();
    bool are_cpu_benchmarks_running();
    bool cpu_benchmarks_done();
    void cpu_benchmarks_set_defaults();
    void print_benchmark_results();
/// @}

/// @name cs_cmdline.C
public:
    void parse_cmdline(int argc, const char* const* argv);
    void parse_env_vars();
/// @}

/// @name cs_files.C
public:
    void check_file_existence();
    bool start_new_file_xfer(PERS_FILE_XFER& pfx);
private:
    int make_project_dirs();
    bool handle_pers_file_xfers();
/// @}

/// @name cs_platforms.C
public:
    std::string get_primary_platform() const;
private:
    void add_platform(const char* platform);
    void detect_platforms();
    void write_platforms(PROJECT* p, MIOFILE& mf);
    bool is_supported_platform(const char* p);
/// @}

/// @name cs_prefs.C
public:
    int project_disk_usage(PROJECT* p, double& size);
    int total_disk_usage(double& size); ///< returns the total disk usage of Synecdoche on this host
    double allowed_disk_usage(double boinc_total);
    int suspend_tasks(int reason);
    int resume_tasks(int reason=0);
    int suspend_network(int reason);
    int resume_network();
    void read_global_prefs();
    int save_global_prefs(const char* global_prefs_xml, const char* master_url, const char* scheduler_url);
    double available_ram();
    double max_available_ram();
private:
    int check_suspend_processing();
    int check_suspend_network();
    void install_global_prefs();
    PROJECT* global_prefs_source_project();
    void show_global_prefs_source(bool found_venue);
/// @}

/// @name cs_scheduler.C
public:
    int make_scheduler_request(PROJECT* p);

    /// Handle the reply from a scheduler.
    int handle_scheduler_reply(PROJECT* project, const char* scheduler_url, int& nresults);

    SCHEDULER_OP* scheduler_op;
private:
    bool contacted_sched_server;
    int overall_work_fetch_urgency;

    bool scheduler_rpc_poll();
    double avg_proc_rate();
    bool should_get_work();
/// @}

/// @name cs_statefile.C
public:
    void set_client_state_dirty(const char* source);
    int parse_state_file();
    int write_state(MIOFILE& f) const;
    int write_state_file() const;
    int write_state_file_if_needed();
    void check_anonymous();
    int parse_app_info(PROJECT* p, FILE* in);
    int write_state_gui(MIOFILE& f) const;
    int write_file_transfers_gui(MIOFILE& f) const;
    int write_tasks_gui(MIOFILE& f) const;
/// @}

/// @name cs_trickle.C
private:
    /// Scan project dir for trickle files and convert them to XML.
    int read_trickle_files(const PROJECT* project, FILE* f);

    /// Remove trickle files when ack has been received.
    int remove_trickle_files(const PROJECT* project);

public:
    /// Parse a trickle-down message in a scheduler reply.
    int handle_trickle_down(const PROJECT* project, FILE* in);
/// @}

/// @name check_state.C
/// stuff related to data-structure integrity checking
///
public:
    void check_project_pointer(PROJECT*);
    void check_app_pointer(APP*);
    void check_file_info_pointer(FILE_INFO*);
    void check_app_version_pointer(APP_VERSION*);
    void check_workunit_pointer(WORKUNIT*);
    void check_result_pointer(RESULT*);
    void check_pers_file_xfer_pointer(PERS_FILE_XFER*);
    void check_file_xfer_pointer(FILE_XFER*);

    void check_app(APP&);
    void check_file_info(FILE_INFO&);
    void check_file_ref(FILE_REF&);
    void check_app_version(APP_VERSION&);
    void check_workunit(WORKUNIT&);
    void check_result(RESULT&);
    void check_active_task(ACTIVE_TASK&);
    void check_pers_file_xfer(PERS_FILE_XFER&);
    void check_file_xfer(FILE_XFER&);

    void check_all();
    void free_mem();
/// @}

/// @name work_fetch.C
public:
    PROJECT* next_project_master_pending();
    PROJECT* next_project_need_work();
    double overall_cpu_frac();

    /// Reset all debts to zero if "zero_debts" is set in the config file.
    void zero_debts_if_requested();

    /// Check if work fetch needed.  Called when:
    /// - core client starts (CLIENT_STATE::init())
    /// - task is completed or fails
    /// - tasks are killed
    /// - an RPC completes
    /// - project suspend/detach/attach/reset GUI RPC
    /// - result suspend/abort GUI RPC
    void request_work_fetch(const char* where);

private:
    int proj_min_results(PROJECT* p, double subset_resource_share);
    void check_project_timeout();
    PROJECT* next_project_sched_rpc_pending();
    PROJECT* next_project_trickle_up_pending();
    PROJECT* find_project_with_overdue_results();
    double time_until_work_done(PROJECT* p, int k, double subset_resource_share);
    bool compute_work_requests();
    void scale_duration_correction_factors(double factor);
    void generate_new_host_cpid();
    void compute_nuploading_results();
/// @}

/// @name rr_sim.cpp
private:
    void rr_simulation();
    void print_deadline_misses();
/// @}

};

extern CLIENT_STATE gstate;

/// return a random double in the range [MIN,min(e^n,MAX))
double calculate_exponential_backoff(
    int n, double MIN, double MAX
);

void print_suspend_tasks_message(int reason);

/// the client will handle I/O (including GUI RPCs)
/// for up to POLL_INTERVAL seconds before calling poll_slow_events()
/// to call the polling functions
#define POLL_INTERVAL   1.0

#endif
