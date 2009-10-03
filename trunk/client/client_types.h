// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Nicolas Alvarez, Peter Kortschack
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

// If you change anything, make sure you also change:
// client_types.C  (to write and parse it)
// client_state.C  (to cross-link objects)

#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H

#if !defined(_WIN32) || defined(__CYGWIN32__)
#include <stdio.h>
#include <sys/time.h>
#endif

#include <vector>
#include <set>
#include <string>

#include "common_defs.h"
#include "md5_file.h"
#include "rr_sim.h"

#define P_LOW 1
#define P_MEDIUM 3
#define P_HIGH 5

#define MAX_SIGNATURE_LEN   4096
#define MAX_KEY_LEN         4096

// If the status is neither of these two,
// it will be an error code defined in error_numbers.h,
// indicating an unrecoverable error in the upload or download of the file,
// or that the file was too big and was deleted
#define FILE_NOT_PRESENT 0
#define FILE_PRESENT 1
#define FILE_NOT_PRESENT_NOT_NEEDED 2

class APP;
class MIOFILE;
class PERS_FILE_XFER;
class RESULT;
class PROJECT;
class ACTIVE_TASK;

class FILE_INFO {
public:
    std::string name;
    char md5_cksum[33];
    double max_nbytes;
    double nbytes;
    double upload_offset;
    bool generated_locally; ///< File is produced by application, not downloaded.
    int status;
    bool executable;        ///< Change file protections to make it executable on the client.
    bool uploaded;          ///< File has been uploaded to the project server.
    bool upload_when_present;
    /// Don't delete this file unless instructed to do so. In particular, don't
    /// immediately delete it after the task(s) using it are completed and
    /// reported.
    bool sticky;
    /// Report presence of this file in each scheduler request.
    bool report_on_rpc;
    /// Server requested delete;
    /// if not in use, delete even if sticky is true.
    /// Don't report to server even if report_on_rpc is true.
    bool marked_for_delete;
    bool signature_required;    ///< true iff associated with app version
    bool is_user_file;
    bool is_project_file;
    bool gzip_when_done; ///< for output files: gzip file when done, and append .gz to its name
    PERS_FILE_XFER* pers_file_xfer; ///< nonzero if in the process of being up/downloaded
    RESULT* result;         ///< for upload files (to authenticate)
    PROJECT* project;
    int ref_cnt;
    std::vector<std::string> urls;
    int start_url;
    int current_url;
    /// If the file_info is signed (for uploadable files),
    /// this is the text that is signed.
    /// Otherwise it is the FILE_INFO's XML descriptor
    /// (without enclosing <file_info> tags).
    std::string signed_xml;

    /// If the file_info is signed, this is the digital signature. Otherwise empty (?).
    std::string xml_signature;

    /// If the file itself is signed (for executable files),
    /// this is the signature.
    std::string file_signature;

    /// If a permanent error occurs during file xfer,
    /// it's recorded here.
    std::string error_msg;

    FILE_INFO();
    ~FILE_INFO();
    void reset();
    int set_permissions();
    int parse(MIOFILE& in, bool from_server);
    int write(MIOFILE& out, bool to_server) const;
    int write_gui(std::ostream& out) const;
    int delete_file();      ///< Attempt to delete the underlying file.
    const char* get_init_url(bool is_upload);
    const char* get_next_url(bool is_upload);
    const char* get_current_url(bool is_upload);
    bool is_correct_url_type(bool is_upload, const std::string& url) const;
    bool had_failure(int& failnum) const;

    /// Create a failure message for a failed file-xfer in XML format.
    std::string failure_message() const;

    int merge_info(const FILE_INFO& new_info);
    int verify_file(bool strict, bool show_errors);

    /// Compress the file using zlib (gzip compression).
    int gzip();
};
typedef std::vector<FILE_INFO*> FILE_INFO_PVEC;
typedef std::set<FILE_INFO*> FILE_INFO_PSET;

/// Describes a connection between a file and a workunit, result, or
/// application. In the first two cases, the app will either use open() or
/// fopen() to access the file (in which case \ref open_name is the name it
/// will use) or the app will be connected by the given fd (in which case fd is
/// nonzero).
class FILE_REF {
public:
    char file_name[256];    ///< physical name
    char open_name[256];    ///< logical name
    bool main_program;
    FILE_INFO* file_info;
    /// If true, core client will copy the file instead of making a soft link.
    /// Works both for input and output files.
    bool copy_file;
    /// If true, don't treat as an error if the file is missing when the task
    /// ends.
    bool optional;

public:
    int parse(MIOFILE& in);
    int write(MIOFILE& out) const;
};
typedef std::vector<FILE_REF> FILE_REF_VEC;


/// Statistics at a specific day.
struct DAILY_STATS {
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double day;

    void clear();
    DAILY_STATS() { clear(); }
    int parse(FILE* in);
};
bool operator < (const DAILY_STATS& lhs, const DAILY_STATS& rhs);

class WORKUNIT {
public:
    char name[256];
    char app_name[256];
    /// Deprecated, but need to keep around to let people revert
    /// to versions before multi-platform support.
    int version_num;
    std::string command_line;
    //char env_vars[256];         ///< environment vars in URL format
    std::vector<FILE_REF> input_files;
    PROJECT* project;
    APP* app;
    int ref_cnt;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;

public:
    WORKUNIT(){}
    ~WORKUNIT(){}
    int parse(MIOFILE& in);
    int write(MIOFILE& out) const;
    bool had_download_failure(int& failnum) const;
    void get_file_errors(std::string& str) const;
    void clear_errors();
};
typedef std::vector<WORKUNIT*> WORKUNIT_PVEC;

class PROJECT {
private:
    /// @name Account file
    /// The following items come from the account file.
    /// They are a function only of the user and the project.
    /// @{

    /// URL of site that contains scheduler tags for this project.
    std::string master_url;
    
public:
    char authenticator[256]; ///< User's authenticator on this project.

    /// Project preferences without the enclosing <project_preferences> tags.
    /// May include <venue> elements
    /// This field is used only briefly: between handling a
    /// scheduler RPC reply and writing the account file.
    std::string project_prefs;

    /// Project-specific preferences without enclosing <project_specific> tags.
    /// Does not include <venue> elements.
    std::string project_specific_prefs;

    std::string gui_urls;  ///< GUI URLs, with enclosing <gui_urls> tags.
    double resource_share; ///< Project's resource share relative to other projects.

    // Logically, this belongs in the client state file
    // rather than the account file.
    // But we need it in the latter in order to parse prefs.
    char host_venue[256];

    bool using_venue_specific_prefs;
    /// @}

    /// @name client_state
    /// The following items come from client_state.xml.
    /// They may depend on the host as well as user and project.
    /// \note if you add anything, add it to copy_state_fields() also!!!
    /// @{

    std::vector<std::string> scheduler_urls; ///< where to find scheduling servers
    char project_name[256];             ///< descriptive.  not unique
    char symstore[256];             ///< URL of symbol server (Windows)
    char user_name[256];
    char team_name[256];
    char email_hash[MD5_LEN];
    char cross_project_id[MD5_LEN];
    double cpid_time;
    double user_total_credit;
    double user_expavg_credit;
    double user_create_time;
    int hostid;
    double host_total_credit;
    double host_expavg_credit;
    double host_create_time;
    /// Resource share according to account manager. Overrides project's.
    double ams_resource_share;
    /// @}

    /// @name Scheduler RPCs
    /// Stuff related to scheduler RPCs and master fetch
    /// @{

    int rpc_seqno;

    /// Number of consecutive times we've failed to contact all scheduling
    /// servers.
    int nrpc_failures;
    /// Number of consecutive(?) times we've failed to download the master file.
    int master_fetch_failures;
    /// Earliest time to contact any server of this project (or zero). In other
    /// words, don't contact any server before this timestamp.
    double min_rpc_time;
    void set_min_rpc_time(double future_time, const char* reason);
    bool waiting_until_min_rpc_time(); ///< Returns true if min_rpc_time > now
    bool master_url_fetch_pending;  ///< need to fetch and parse the master URL

    /// We need to do a scheduler RPC, for various possible reasons:
    /// user request, propagate host CPID, time-based, etc.
    /// Reasons are enumerated in scheduler_op.h.
    rpc_reason sched_rpc_pending;

    /// If nonzero, specifies a time when another scheduler RPC
    /// should be done (as requested by server).
    double next_rpc_time;

    /// We need to call request_work_fetch() when a project
    /// transitions from being backed off to not.
    /// This (slightly misnamed) keeps track of whether this
    /// may still need to be done for given project.
    bool possibly_backed_off;

    bool trickle_up_pending;    ///< have trickle up to send
    double last_rpc_time;       ///< when last RPC finished
    /// @}

    /// @name Others
    /// @{

    /// Use anonymous platform for this project. Set if app_versions.xml file
    /// is found in the project directory. Will use those apps rather than
    /// getting apps from the server.
    bool anonymous_platform;

    bool non_cpu_intensive;
    bool use_symlinks;
    /// @}

    /// @name Server requests for data
    /// Items send in scheduler replies, requesting that
    /// various things be sent in the next request
    /// @{

    /// Send the list of permanent files associated with the project
    /// in the next scheduler reply.
    bool send_file_list;

    int send_time_stats_log;  ///< If nonzero, send time stats log from that point on
    int send_job_log; ///< if nonzero, send this project's job log from that point on
    /// @}

    bool suspended_via_gui;

    /// If true, don't request work from this project (but still do scheduler
    /// requests, like to return finished work). Used for a clean exit to a
    /// project, or if a user wants to pause doing work for the project.
    bool dont_request_more_work;

    bool attached_via_acct_mgr;
    /// If true, detach this project as soon as there are no more results for
    /// it.
    bool detach_when_done;
    bool ended;                 ///< Project has ended; advise user to detach.
    char code_sign_key[MAX_KEY_LEN];
    FILE_REF_VEC user_files;
    FILE_REF_VEC project_files; ///< Files not specific to apps or work, like icons.

    int parse_preferences_for_user_files();

    /// Parse project files from a xml file.
    int parse_project_files(MIOFILE& in, bool delete_existing_symlinks);

    /// Write the XML representation of the project files into a file.
    void write_project_files(MIOFILE& out) const;

    /// Install pointers from FILE_REFs to FILE_INFOs for project files.
    void link_project_files(bool recreate_symlink_files);

    /// Write symlinks for project files.
    int write_symlink_for_project_file(const FILE_INFO* fip) const;

    double project_files_downloaded_time; ///< When last project file download finished.

    /// Update project_files_downloaded_time to the current time.
    void update_project_files_downloaded_time();

    /// Multiply by this when estimating the CPU time of a result
    /// (based on FLOPs estimated and benchmarks).
    /// This is dynamically updated in a way that maintains an upper bound.
    /// it goes down slowly but if a new estimate X is larger,
    /// the factor is set to X.
    double duration_correction_factor;

    void update_duration_correction_factor(const RESULT* result);

    /// @name CPU scheduler and work fetch
    /// Fields used by CPU scheduler and work fetch.
    /// everything from here on applies only to CPU intensive projects.
    /// @{

    /// Not suspended and not deferred and not no more work.
    bool contactable() const;

    /// Has a runnable result.
    bool runnable() const;

    /// Has a result in downloading state.
    bool downloading() const;

    /// Runnable or contactable or downloading.
    bool potentially_runnable() const;

    /// Runnable or downloading.
    bool nearly_runnable() const;

    /// The project has used too much CPU time recently.
    bool overworked() const;

    /// A download is backed off.
    bool some_download_stalled() const;

    bool some_result_suspended() const;
    /// @}

    /// temps used in CLIENT_STATE::rr_simulation();
    RR_SIM_PROJECT_STATUS rr_sim_status;
    void set_rrsim_proc_rate(double rrs);

    int deadlines_missed;   ///< used as scratch by scheduler, enforcer

    /// @name Debt
    /// "debt" is how much CPU time we owe this project relative to others.
    /// @{

    /// Computed over runnable projects.
    /// Used for CPU scheduling.
    double short_term_debt;

    /// Computed over potentially runnable projects
    /// (defined for all projects, but doesn't change if
    /// not potentially runnable).
    /// Normalized so mean over all projects is zero.
    double long_term_debt;

    /// Expected debt by the end of the preemption period.
    double anticipated_debt;

    /// How much "wall CPU time" has been devoted to this
    /// project in the current debt interval.
    double wall_cpu_time_this_debt_interval;
    /// @}

    /// The next result to run for this project.
    RESULT *next_runnable_result;

    /// Number of results in UPLOADING state.
    /// Don't start new results if these exceeds 2*ncpus.
    int nuploading_results;

    /// The unit is "project-normalized CPU seconds",
    /// i.e. the work should take 1 CPU on this host
    /// X seconds of wall-clock time to complete,
    /// taking into account:
    /// -# this project's fractional resource share.
    /// -# on_frac, active_frac, and cpu_effiency.
    ///
    /// See http://boinc.berkeley.edu/trac/wiki/CpuSched
    double work_request;

    int work_request_urgency;

    /// Number of results being returned in current scheduler operation.
    int nresults_returned;

    /// Get scheduler URL with random offset \a r.
    const char* get_scheduler_url(int index, double r) const;

    /// Temporary used when scanning projects.
    bool checked;

    /// @name File transfer backoff.
    /// Vars related to file-transfer backoff.
    /// file_xfer_failures_up: count of consecutive upload failures.
    /// next_file_xfer_up: when to start trying uploads again.
    ///
    /// If file_xfer_failures_up exceeds FILE_XFER_FAILURE_LIMIT,
    /// we switch from a per-file to a project-wide backoff policy
    /// (separately for the up/down directions).
    ///
    /// NOTE: all this refers to transient failures, not permanent.
    /// Also, none of this is used right now (commented out)
    /// @{
#define FILE_XFER_FAILURE_LIMIT 3
    int file_xfer_failures_up;
    int file_xfer_failures_down;
    double next_file_xfer_up;
    double next_file_xfer_down;

    double next_file_xfer_time(const bool is_upload) const;
    void file_xfer_failed(const bool is_upload);
    void file_xfer_succeeded(const bool is_upload);
    /// @}

    PROJECT();
    ~PROJECT(){}
    void init();
    void copy_state_fields(const PROJECT& p);
    const char *get_project_name() const;

    /// Return the master URL for this project.
    std::string get_master_url() const;
    
    /// Set the master URL for this project.
    void set_master_url(const std::string& master_url);

    /// Write account_*.xml file.
    int write_account_file() const;

    int parse_account(FILE*);

    /// Scan and parse an account_*.xml file, looking for a <venue> element.
    int parse_account_file_venue();

    int parse_account_file();
    int parse_state(MIOFILE& in);
    int write_state(std::ostream& out, bool gui_rpc=false) const;

    std::vector<DAILY_STATS> statistics; ///< Statistics of the last x days.
    int parse_statistics(FILE* in);
    int write_statistics(std::ostream& out, bool gui_rpc=false) const;

    /// Write the statistics file.
    int write_statistics_file() const;

    /// Get all workunits for this project.
    WORKUNIT_PVEC get_workunits() const;
};

class APP {
public:
    char name[256];
    char user_friendly_name[256];
    PROJECT* project;

    int parse(MIOFILE& in);
    int write(MIOFILE& out) const;
};

class APP_VERSION {
public:
    char app_name[256];
    int version_num;
    char platform[256];
    char plan_class[64];
    char api_version[16];
    double avg_ncpus;
    double max_ncpus;
    double flops;
    char cmdline[256]; ///< Additional command-line arguments.

    APP* app;
    PROJECT* project;
    std::vector<FILE_REF> app_files;
    int ref_cnt;
    char graphics_exec_path[512];

public:
    APP_VERSION(){}
    ~APP_VERSION(){}
    int parse(MIOFILE& in);
    int write(MIOFILE& out) const;
    bool had_download_failure(int& failnum) const;
    void get_file_errors(std::string& str);
    void clear_errors();
    int api_major_version() const;
};

class RESULT {
public:
    char name[256];
    char wu_name[256];
    double report_deadline;
    int version_num;        ///< Identifies the app used.
    char plan_class[64];
    char platform[256];
    APP_VERSION* avp;
    std::vector<FILE_REF> output_files;
    /// We're ready to report this result to the server;
    /// either computation is done and all the files have been uploaded,
    /// or there was an error.
    bool ready_to_report;
    /// Time when ready_to_report was set.
    double completed_time;
    /// We've received the ack for this result from the server.
    bool got_server_ack;
    double final_cpu_time;
    double fpops_per_cpu_sec;   // nonzero if reported by app
    double fpops_cumulative;    // nonzero if reported by app
    double intops_per_cpu_sec;   // nonzero if reported by app
    double intops_cumulative;    // nonzero if reported by app
    int exit_status;            ///< Return value from the application.
    /// The concatenation of:
    ///
    /// - if report_result_error() is called for this result:
    ///   - <message>x</message>
    ///   - <exit_status>x</exit_status>
    ///   - <signal>x</signal>
    /// - if called in FILES_DOWNLOADED state:
    ///   - <couldnt_start>x</couldnt_start>
    /// - if called in NEW state:
    ///   - <download_error>x</download_error> for each failed download
    /// - if called in COMPUTE_DONE state:
    ///   - <upload_error>x</upload_error> for each failed upload
    /// - <stderr_txt>X</stderr_txt>, where X is the app's stderr output
    std::string stderr_out;
    bool suspended_via_gui;

    APP* app;
    WORKUNIT* wup; ///< this may be NULL after result is finished
    PROJECT* project;

private:
    int _state;                  ///< State of this result: see lib/common_defs.h
    double received_time; ///< when we got this from server

public:
    RESULT(){}
    ~RESULT(){}
    void clear();
    int parse_server(MIOFILE&);
    int parse_state(MIOFILE&);
    int parse_name(FILE* in, const char* end_tag);
    int write(MIOFILE& out, bool to_server) const;
    int write_gui(std::ostream& out) const;
    bool is_upload_done() const;    ///< files uploaded?
    void clear_uploaded_flags();
    const FILE_REF* lookup_file(const FILE_INFO* fip) const;
    FILE_INFO* lookup_file_logical(const char* lname);
    /// Abort the result if it hasn't started computing yet.
    /// Called only for results with no active task
    /// (otherwise you need to abort the active task).
    void abort_inactive(int status);
    void append_log_record(ACTIVE_TASK& at);

    inline int state() const { return _state; }
    void set_state(int val, const char* where);

    // stuff related to CPU scheduling

    double estimated_cpu_time() const;
    double estimated_cpu_time_uncorrected() const;
    double estimated_cpu_time_remaining() const;
    bool computing_done() const;

    /// Check if the result was started yet.
    bool not_started() const;

    /// Downloaded, not finished, not suspended, project not suspended.
    bool runnable() const;
    /// Downloading or downloaded,
    /// not finished, suspended, project not suspended.
    bool nearly_runnable() const;
    /// Downloading, not downloaded, not suspended, project not suspended.
    bool downloading() const;
    /// Some input or app file is downloading, and backed off.
    /// That is, it may be a long time before we can run this result.
    bool some_download_stalled() const;

    /// Get the project this result belongs to.
    PROJECT* get_project() const;

    /// Get the name of this result.
    std::string get_name() const;

    /// Get the time when this result was received from the server.
    double get_received_time() const;
    
    /// Set the time when this result was received from the server.
    void set_received_time(double received_time);

    // temporaries used in CLIENT_STATE::rr_simulation():
    double rrsim_cpu_left;
    double rrsim_finish_delay;
    /// Result already selected by schedule_cpus(). Used to keep cpu scheduler
    /// from scheduling a result twice. Transient variable.
    bool already_selected;
    /// report deadline - prefs.work_buf_min - time slice
    double computation_deadline() const;
    bool rr_sim_misses_deadline;
    bool last_rr_sim_missed_deadline;

    /// Temporary used to tell GUI that this result is deadline-scheduled.
    bool edf_scheduled;
};
typedef std::vector<RESULT*> RESULT_PVEC;

/// Sepresents an always/auto/never value, possibly temporarily overridden.
class MODE {
private:
    int perm_mode;
    int temp_mode;
    double temp_timeout;
public:
    MODE();
    void set(int mode, double duration);
    int get_perm() const;
    int get_current() const;
    double delay() const;
};

/// A platform supported by the client.
class PLATFORM {
public:
    std::string name;
};

#endif // CLIENT_TYPES_H
