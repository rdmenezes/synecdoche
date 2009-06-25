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

#ifndef FILE_NAMES_H
#define FILE_NAMES_H

#include "attributes.h"

#include <string>

class FILE_INFO;
class PROJECT;
class MIOFILE;

/// Gets the pathname (relative to client home dir) of a project file.
std::string get_pathname(const FILE_INFO* fip);

/// Gets the pathname (relative to client home dir) of a project file.
void get_pathname(const FILE_INFO* fip, char* path, int len) __attribute__((deprecated));

/// Get the directory for a given project.
std::string get_project_dir(const PROJECT* p);

/// Get the directory for a given project.
void get_project_dir(const PROJECT* p, char* path, int len) __attribute__((deprecated));

/// get the pathname (relative to client home dir) of the
/// directory used for a particular application "slot".
void get_slot_dir(int slot, char* path, int len) __attribute__((deprecated));

/// Returns the location of a numbered slot directory.
std::string get_slot_dir(int slot);

int make_project_dir(const PROJECT&);
int remove_project_dir(const PROJECT&);
int make_slot_dir(int);

/// Delete unused stuff in the slots/ directory.
void delete_old_slot_dirs();

/// Get the name of the account file for a given master URL.
std::string get_account_filename(const std::string& master_url);

/// Check if the given filename if the name of an account file.
bool is_account_file(const std::string& filename);

/// Check if the given filename if the name of a statistics file.
bool is_statistics_file(const std::string& filename);

/// Get the name of the statistics file for a given master URL.
std::string get_statistics_filename(const std::string& master_url);

/// Check if a file name denotes an image file.
bool is_image_file(std::string filename);

/// Get the scheduler request file name for a project.
std::string get_sched_request_filename(const PROJECT& project);

/// Get the scheduler reply file name for a project.
std::string get_sched_reply_filename(const PROJECT& project);

/// Get the name of the master file for a project.
std::string get_master_filename(const PROJECT& project);

/// Get the job log file name for a project.
std::string job_log_filename(const PROJECT& project);

void send_log_after(const char* filename, double t, MIOFILE& mf);

#define PROJECTS_DIR                "projects"
#define SLOTS_DIR                   "slots"
#define SWITCHER_DIR                "switcher"
#define STATE_FILE_NEXT             "client_state_next.xml"
#define STATE_FILE_NAME             "client_state.xml"
#define STATE_FILE_PREV             "client_state_prev.xml"
#define GLOBAL_PREFS_FILE_NAME      "global_prefs.xml"
#define GLOBAL_PREFS_OVERRIDE_FILE  "global_prefs_override.xml"
#define MASTER_BASE                 "master_"
#define SCHED_OP_REQUEST_BASE       "sched_request_"
#define SCHED_OP_REPLY_BASE         "sched_reply_"
#define CONFIG_FILE                 "cc_config.xml"
#define TEMP_FILE_NAME              "temp.xml"
#define TEMP_STATS_FILE_NAME        "temp_stats.xml"
#define TEMP_TIME_STATS_FILE_NAME   "temp_time_stats.xml"
#define TEMP_ACCT_FILE_NAME         "temp_acct.xml"
#define STDERR_FILE_NAME            "stderr.txt"
#define STDOUT_FILE_NAME            "stdout.txt"
#define CPU_BENCHMARKS_FILE_NAME    "cpu_benchmarks"
#define APP_INFO_FILE_NAME          "app_info.xml"
#define REMOTEHOST_FILE_NAME        "remote_hosts.cfg"
#define ACCT_MGR_REQUEST_FILENAME   "acct_mgr_request.xml"
#define ACCT_MGR_REPLY_FILENAME     "acct_mgr_reply.xml"
#define PROJECT_INIT_FILENAME       "project_init.xml"
#define ACCT_MGR_URL_FILENAME       "acct_mgr_url.xml"
#define ACCT_MGR_LOGIN_FILENAME     "acct_mgr_login.xml"
#define GET_PROJECT_CONFIG_FILENAME "get_project_config.xml"
#define LOOKUP_ACCOUNT_FILENAME     "lookup_account.xml"
#define CREATE_ACCOUNT_FILENAME     "create_account.xml"
#define LOOKUP_WEBSITE_FILENAME     "lookup_website.html"
#define GET_CURRENT_VERSION_FILENAME    "get_current_version.xml"
#define ALL_PROJECTS_LIST_FILENAME "all_projects_list.xml"
#define SWITCHER_FILE_NAME          "switcher"
#define SETPROJECTGRP_FILE_NAME     "setprojectgrp"
#define TIME_STATS_LOG              "time_stats_log"
#define JOB_LOG_BASE                "job_log_"
#define CA_BUNDLE_FILENAME          "ca-bundle.crt"
#define CLIENT_AUTH_FILENAME        "client_auth.xml"

#endif // FILE_NAMES_H
