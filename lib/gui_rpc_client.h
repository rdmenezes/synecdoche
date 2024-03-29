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
/// A C++ interface to BOINC GUI RPC.

#ifndef GUI_RPC_CLIENT_H
#define GUI_RPC_CLIENT_H

#include <locale.h>

#if !defined(_WIN32) || defined (__CYGWIN__)
#include <netinet/in.h>
#endif

#include <vector>
#include <string>

#include "miofile.h"
#include "prefs.h"
#include "common_defs.h"

class HOST_INFO;

struct GUI_URL {
    std::string name;
    std::string description;
    std::string url;

    int parse(MIOFILE& in);
    void print() const;
};

// statistics at a specific day
//
struct DAILY_STATS {
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;
    double host_expavg_credit;
    double day;

    int parse(MIOFILE& in);
};


class PROJECT_LIST_ENTRY {
public:
    std::string name;
    std::string url;
    std::string general_area;
    std::string specific_area;
    std::string description;
    std::string home;
    std::string image;

    PROJECT_LIST_ENTRY();
    ~PROJECT_LIST_ENTRY();

    int parse(XML_PARSER& xp);
    void clear();

    static bool compare_name(const PROJECT_LIST_ENTRY* first, const PROJECT_LIST_ENTRY* second);
};
inline bool PROJECT_LIST_ENTRY::compare_name(const PROJECT_LIST_ENTRY* first, const PROJECT_LIST_ENTRY* second) {
    return first->name < second->name;
}

class PROJECT {
public:
    std::string master_url;
    double resource_share;
    std::string project_name;
    std::string user_name;
    std::string team_name;
    int hostid;
    std::vector<GUI_URL> gui_urls;
    double user_total_credit;
    double user_expavg_credit;
    double host_total_credit;      // as reported by server
    double host_expavg_credit;     // as reported by server
    double disk_usage;
    int nrpc_failures;          ///< # of consecutive times we've failed to
                                ///< contact all scheduling servers
    int master_fetch_failures;
    double min_rpc_time;           ///< earliest time to contact any server
    double short_term_debt;
    double long_term_debt;

    bool master_url_fetch_pending; ///< need to fetch and parse the master URL
    rpc_reason sched_rpc_pending;      ///< need to contact scheduling server
    int rr_sim_deadlines_missed;
    bool non_cpu_intensive;
    bool suspended_via_gui;
    bool dont_request_more_work;
    bool scheduler_rpc_in_progress;
    bool attached_via_acct_mgr;
    bool detach_when_done;
    bool ended;
    /// When the last project file download was finished
    /// (i.e. the time when ALL project files were finished downloading)
    double project_files_downloaded_time;
    /// when the last successful scheduler RPC finished
    double last_rpc_time;
    std::vector<DAILY_STATS> statistics; ///< credit data over the last x days

    // NOTE: if you add any data items above,
    // update parse(), copy() and clear() to include them!!

    PROJECT();
    ~PROJECT();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
    void get_name(std::string& s) const;
    void copy(const PROJECT& p);        ///< copy to this object

    /// temp - keep track of whether or not this record needs to be deleted
    bool flag_for_delete;
};

class APP {
public:
    std::string name;
    std::string user_friendly_name;
    PROJECT* project;

    APP();
    ~APP();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class APP_VERSION {
public:
    std::string app_name;
    int version_num;
    std::string plan_class;
    APP* app;
    PROJECT* project;
    double duration_correction_factor;

    APP_VERSION();
    ~APP_VERSION();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class WORKUNIT {
public:
    std::string name;
    std::string app_name;
    int version_num;
    double rsc_fpops_est;
    double rsc_fpops_bound;
    double rsc_memory_bound;
    double rsc_disk_bound;
    PROJECT* project;
    APP* app;
    APP_VERSION* avp;

    WORKUNIT();
    ~WORKUNIT();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class RESULT {
public:
    std::string name;
    std::string wu_name;
    std::string project_url;
    double received_time;
    double report_deadline;
    bool ready_to_report;
    bool got_server_ack;
    double final_cpu_time;
    int state;
    int scheduler_state;
    int exit_status;
    int signal;
    std::string stderr_out;
    bool suspended_via_gui;
    bool project_suspended_via_gui;

    // the following defined if active
    bool active_task;
    int active_task_state;
    int app_version_num;
    double checkpoint_cpu_time;
    double current_cpu_time;
    double fraction_done;
    double swap_size;
    double working_set_size_smoothed;
    double estimated_cpu_time_remaining;
    bool supports_graphics;
    int graphics_mode_acked;
    bool too_large;
    bool needs_shmem;
    bool edf_scheduled;
    std::string graphics_exec_path;
    std::string slot_path;

    APP* app;
    WORKUNIT* wup;
    PROJECT* project;

    RESULT();
    ~RESULT();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class FILE_TRANSFER {
public:
    std::string name;
    std::string project_url;
    std::string project_name;
    double nbytes;
    bool generated_locally;
    bool uploaded;
    bool upload_when_present;
    bool sticky;
    bool pers_xfer_active;
    bool xfer_active;
    int num_retries;
    int first_request_time;
    int next_request_time;
    int status;
    double time_so_far;
    double bytes_xferred;
    double file_offset;
    double xfer_speed;
    std::string hostname;
    PROJECT* project;

    FILE_TRANSFER();
    ~FILE_TRANSFER();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class MESSAGE {
public:
    std::string project;
    MSG_PRIORITY priority;
    int seqno;
    int timestamp;
    std::string body;

    MESSAGE();
    ~MESSAGE();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class GR_PROXY_INFO {
public:
    bool use_http_proxy;
    bool use_socks_proxy;
    bool use_http_authentication;
    int socks_version;
    std::string socks_server_name;
    std::string http_server_name;
    int socks_server_port;
    int http_server_port;
    std::string http_user_name;
    std::string http_user_passwd;
    std::string socks5_user_name;
    std::string socks5_user_passwd;

    GR_PROXY_INFO();
    ~GR_PROXY_INFO();

    int parse(MIOFILE& in);
    void print() const;
    void clear();
};

class CC_STATE {
public:
    std::vector<PROJECT*> projects;
    std::vector<APP*> apps;
    std::vector<APP_VERSION*> app_versions;
    std::vector<WORKUNIT*> wus;
    std::vector<RESULT*> results;

    GLOBAL_PREFS global_prefs;  // working prefs, i.e. network + override
    VERSION_INFO version_info;  // populated only if talking to pre-5.6 CC
    bool executing_as_daemon;   // true if Client is running as a service / daemon

    CC_STATE();
    ~CC_STATE();

    PROJECT* lookup_project(const std::string& url);
    APP* lookup_app(const std::string& project_url, const std::string& name);
    APP* lookup_app(const PROJECT* project, const std::string& name);
    APP_VERSION* lookup_app_version(const std::string& project_url, const std::string& name, int version_num);
    APP_VERSION* lookup_app_version(const PROJECT* project, const std::string& name, int version_num);
    WORKUNIT* lookup_wu(const std::string& project_url, const std::string& name);
    WORKUNIT* lookup_wu(const PROJECT* project, const std::string& name);
    RESULT* lookup_result(const std::string& project_url, const std::string& name);
    RESULT* lookup_result(const PROJECT* project, const std::string& name);

    void print() const;
    void clear();
};

class ALL_PROJECTS_LIST {
public:
    std::vector<PROJECT_LIST_ENTRY*> projects;

    ALL_PROJECTS_LIST();
    ~ALL_PROJECTS_LIST();

    void clear();
};

class PROJECTS {
public:
    std::vector<PROJECT*> projects;

    PROJECTS(){}
    ~PROJECTS();

    void print() const;
    void clear();
};

struct DISK_USAGE {
    std::vector<PROJECT*> projects;
    double d_total;
    double d_free;
    double d_boinc;     // amount used by Synecdoche itself, not projects
    double d_allowed;   // amount Synecdoche is allowed to use, total

    DISK_USAGE(){clear();}
    ~DISK_USAGE();

    void print() const;
    void clear();
};

class RESULTS {
public:
    std::vector<RESULT*> results;

    RESULTS(){}
    ~RESULTS();

    void print() const;
    void clear();
};

class FILE_TRANSFERS {
public:
    std::vector<FILE_TRANSFER*> file_transfers;

    FILE_TRANSFERS();
    ~FILE_TRANSFERS();

    void print() const;
    void clear();
};

class MESSAGES {
public:
    std::vector<MESSAGE*> messages;

    MESSAGES();
    ~MESSAGES();

    void print() const;
    void clear();
};

struct DISPLAY_INFO {
    std::string window_station;   // windows
    std::string desktop;          // windows
    std::string display;          // X11

    DISPLAY_INFO();
    void clear();
};

std::ostream& operator <<(std::ostream& out, const DISPLAY_INFO& in);

struct ACCT_MGR_INFO {
    std::string acct_mgr_name;
    std::string acct_mgr_url;
    bool have_credentials;

    ACCT_MGR_INFO();
    ~ACCT_MGR_INFO(){}

    int parse(MIOFILE& in);
    void clear();
};

struct PROJECT_ATTACH_REPLY {
    int error_num;
    std::vector<std::string>messages;

    PROJECT_ATTACH_REPLY();
    ~PROJECT_ATTACH_REPLY(){}

    int parse(MIOFILE& in);
    void clear();
};

struct ACCT_MGR_RPC_REPLY {
    int error_num;
    std::vector<std::string>messages;

    ACCT_MGR_RPC_REPLY();
    ~ACCT_MGR_RPC_REPLY(){}

    int parse(MIOFILE& in);
    void clear();
};

struct PROJECT_INIT_STATUS {
    std::string url;
    std::string name;
    bool has_account_key;

    PROJECT_INIT_STATUS();
    ~PROJECT_INIT_STATUS(){}

    int parse(MIOFILE& in);
    void clear();
};

struct PROJECT_CONFIG {
    int error_num;
    std::string name;
    std::string master_url;
    int local_revision; ///< SVN revision of the server software
    int min_passwd_length;
    bool account_manager;
    bool uses_username;
    bool account_creation_disabled;
    bool client_account_creation_disabled;
    std::vector<std::string> messages;

    PROJECT_CONFIG();
    ~PROJECT_CONFIG();

    int parse(MIOFILE& in);
    void clear();
    void print() const;
};

struct ACCOUNT_IN {
    std::string url;
    /// account identifier (email address or user name)
    std::string email_addr;
    std::string user_name;
    std::string passwd;

    ACCOUNT_IN();
    ~ACCOUNT_IN();

    void clear();
};

struct ACCOUNT_OUT {
    int error_num;
    std::string error_msg;
    std::string authenticator;

    ACCOUNT_OUT();
    ~ACCOUNT_OUT();

    int parse(MIOFILE& in);
    void clear();
    void print() const;
};

class CC_STATUS {
public:
    int network_status;         // values: NETWORK_STATUS_*
    bool ams_password_error;
    int task_suspend_reason;    // bitmap, see common_defs.h
    int network_suspend_reason;
    int task_mode;              // always/auto/never; see common_defs.h
    int network_mode;
    int task_mode_perm;         // same, but permanent version
    int network_mode_perm;
    double task_mode_delay;     // time until perm becomes actual
    double network_mode_delay;
    bool disallow_attach;
    bool simple_gui_only;

    CC_STATUS();
    ~CC_STATUS();

    int parse(MIOFILE& in);
    void clear();
    void print() const;
};

struct SIMPLE_GUI_INFO {
    std::vector<PROJECT*> projects;
    std::vector<RESULT*> results;
    void print() const;
};

class RPC_CLIENT {
public:
    int sock;
    double start_time;
    double timeout;
    bool retry;
    sockaddr_in addr;

    /// Send a rpc-request to the rpc-server.
    int send_request(const char* p);

    /// Get reply from server.
    int get_reply(char*& mbuf);

    RPC_CLIENT();
    ~RPC_CLIENT();

    /// Initiate a connection to the core client.
    int init(const char* host, int port = GUI_RPC_PORT);

    /// Initiate a connection to the core client using non-blocking operations.
    int init_asynch(const char* host, double timeout, bool retry, int port = GUI_RPC_PORT);

    int init_poll();
    void close();

    /// Answer an authorization request sent by the server.
    int authorize(const char* passwd);

    int exchange_versions(VERSION_INFO& server);
    int get_state(CC_STATE& state);
    int get_results(RESULTS& t);
    int get_file_transfers(FILE_TRANSFERS& t);
    int get_simple_gui_info(SIMPLE_GUI_INFO& sgi);
    int get_simple_gui_info(CC_STATE& state, RESULTS& results);
    int get_project_status(CC_STATE& state);
    int get_project_status(PROJECTS& p);
    int get_all_projects_list(ALL_PROJECTS_LIST& pl);
    int get_disk_usage(DISK_USAGE& du);
    int show_graphics(
        const char* project, const char* result_name, int graphics_mode,
        DISPLAY_INFO& display
    );
    int project_op(PROJECT& project, const char* op);
    int set_run_mode(int mode, double duration);
        // if duration is zero, change is permanent.
        // otherwise, after duration expires,
        // restore last permanent mode
    int set_network_mode(int mode, double duration);
    int get_screensaver_tasks(int& suspend_reason, RESULTS& t);
    int run_benchmarks();
    int set_proxy_settings(const GR_PROXY_INFO& pi);
    int get_proxy_settings(GR_PROXY_INFO& pi);
    int get_messages(int seqno, MESSAGES& msgs);
    int get_message_count(int& msg_count);
    int file_transfer_op(const FILE_TRANSFER& ft, const char* op);
    int result_op(RESULT& result, const char* op);
    int get_host_info(HOST_INFO& host);
    int quit();
    int acct_mgr_info(ACCT_MGR_INFO& ami);
    const char* mode_name(int mode);
    int get_statistics(PROJECTS& p);
    int network_available();
    int get_project_init_status(PROJECT_INIT_STATUS& pis);

    // the following are asynch operations.
    // Make the first call to start the op,
    // call the second one periodically until it returns zero.
    // TODO: do project update
    //
    int get_project_config(const std::string& url);
    int get_project_config_poll(PROJECT_CONFIG& pc);
    int lookup_account(ACCOUNT_IN& ai);
    int lookup_account_poll(ACCOUNT_OUT& ao);
    int create_account(ACCOUNT_IN& ai);
    int create_account_poll(ACCOUNT_OUT& ao);
    int project_attach(
        const char* url, const char* auth, const char* project_name
    );
    int project_attach_from_file();
    int project_attach_poll(PROJECT_ATTACH_REPLY& reply);
    int acct_mgr_rpc(
        const char* url, const char* name, const char* passwd,
        bool use_config_file=false
    );
    int acct_mgr_rpc_poll(ACCT_MGR_RPC_REPLY& reply);

#ifdef ENABLE_UPDATE_CHECK
    int get_newer_version(std::string&);
#endif
    int read_global_prefs_override();
    int read_cc_config();
    int get_cc_status(CC_STATUS& status);
    int get_global_prefs_file(std::string&);
    int get_global_prefs_working(std::string&);
    int get_global_prefs_working_struct(GLOBAL_PREFS&, GLOBAL_PREFS_MASK&);
    int get_global_prefs_override(std::string&);
    int set_global_prefs_override(const std::string&);
    int get_global_prefs_override_struct(GLOBAL_PREFS&, GLOBAL_PREFS_MASK&);
    int set_global_prefs_override_struct(GLOBAL_PREFS&, GLOBAL_PREFS_MASK&);
    int set_debts(const std::vector<PROJECT>&);
};

struct RPC {
    char* mbuf;
    MIOFILE fin;
    RPC_CLIENT* rpc_client;

    RPC(RPC_CLIENT* rc);
    ~RPC();
    int do_rpc(const char* req);
    int parse_reply();
};

struct SET_LOCALE {
    std::string locale;
    inline SET_LOCALE() {
        locale = setlocale(LC_ALL, NULL);
        setlocale(LC_ALL, "C");
    }
    inline ~SET_LOCALE() {
        setlocale(LC_ALL, locale.c_str());
    }
};

std::string read_gui_rpc_password(const std::string& file_name = GUI_RPC_PASSWD_FILE);

#endif // GUI_RPC_CLIENT_H
