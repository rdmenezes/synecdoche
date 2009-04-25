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

#ifndef _SCHEDULER_OP_
#define _SCHEDULER_OP_

#include <vector>
#include <string>

#include "client_types.h"
#include "http_curl.h"

// invariant: in this state, our HTTP_OP is not in the HTTP_OP_SET
#define SCHEDULER_OP_STATE_IDLE         0

#define SCHEDULER_OP_STATE_GET_MASTER   1

#define SCHEDULER_OP_STATE_RPC          2

// defaults related to scheduler RPC policy
// See client_state.h for definitions

// fetch and parse master URL if nrpc_failures is a multiple of this
#define MASTER_FETCH_PERIOD     10
// cap on nrpc_failures
#define RETRY_CAP               10
// after this many master-fetch failures,
// move into a state in which we retry master fetch
// at the frequency below
#define MASTER_FETCH_RETRY_CAP 3
// See above
#define MASTER_FETCH_INTERVAL (86400)    // 1 day

// constants used to bound RPC backoff
#define SCHED_RETRY_DELAY_MIN    60                // 1 minute
#define SCHED_RETRY_DELAY_MAX    (60*60*4)         // 4 hours


/// SCHEDULER_OP encapsulates the mechanism for
/// -# fetching master files
/// -# communicating with scheduling servers
///
/// Only one such operation can be in progress at once.

class SCHEDULER_OP {
private:
    int scheduler_op_retval;
    HTTP_OP http_op;
    HTTP_OP_SET* http_ops;
    char scheduler_url[256];
    int url_index;                  ///< index within project's URL list
public:
    PROJECT* cur_proj;               ///< project we're currently contacting
    int state;
    rpc_reason reason;
    double url_random;              ///< used to randomize order

public:
    SCHEDULER_OP(HTTP_OP_SET*);
    
    /// Poll routine. If an operation is in progress, check for completion.
    bool poll();
    
    int init_get_work();

    /// Try to initiate an RPC to the given project.
    int init_op_project(PROJECT* p, rpc_reason r);

    int init_master_fetch(PROJECT* p);
    bool check_master_fetch_start();

    /// Back off contacting this project's schedulers.
    void backoff(PROJECT* p, const std::string& reason_msg);

    /// if we're doing an op to this project, abort it
    void abort(PROJECT* p);
private:
    bool update_urls(PROJECT* p, std::vector<std::string>& urls);
    int start_op(PROJECT* p);
    int start_rpc(PROJECT* p);
    
    /// Parse a master file.
    std::vector<std::string> parse_master_file(PROJECT* p) const;
};

struct USER_MESSAGE {
    std::string message;
    std::string priority;
    USER_MESSAGE(const char* msg, const char* prio);
};

inline USER_MESSAGE::USER_MESSAGE(const char* msg, const char* prio):
    message(msg),
    priority(prio)
{
}

struct SCHEDULER_REPLY {
    int hostid;
    double request_delay;
    double next_rpc_delay;
    std::vector<USER_MESSAGE> messages;

    // not including <global_preferences> tags;
    // may include <venue> elements
    char* global_prefs_xml;
    // not including <project_preferences> tags
    // may include <venue> elements
    char* project_prefs_xml;

    char master_url[256];
    char host_venue[256];

    unsigned int user_create_time;
    std::vector<APP> apps;
    std::vector<FILE_INFO> file_infos;
    std::vector<std::string> file_deletes;
    std::vector<APP_VERSION> app_versions;
    std::vector<WORKUNIT> workunits;
    std::vector<RESULT> results;
    std::vector<RESULT> result_acks;
    std::vector<RESULT> result_abort;
    std::vector<RESULT> result_abort_if_not_started;
    char* code_sign_key;
    char* code_sign_key_signature;
    bool message_ack;
    bool project_is_down;
    bool send_file_list;
    int send_time_stats_log;
    int send_job_log;
    int scheduler_version;

    SCHEDULER_REPLY();
    ~SCHEDULER_REPLY();
    int parse(FILE* in, PROJECT* project);
};

#endif
