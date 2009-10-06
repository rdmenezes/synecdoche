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
/// keep track of the network performance of this host,
/// namely exponentially weighted averages of upload and download speeds

#ifndef _NET_STATS_
#define _NET_STATS_

#include <cstdio>
#include <string>

#include "gui_http.h"
#include "error_numbers.h"

class FILE_XFER_SET;
class HTTP_OP_SET;
class MIOFILE;

// there's one of these each for upload and download
class NET_INFO {
public:
    /// estimate of max transfer rate; computed as an average of
    /// the rates of recent file transfers, weighted by file size.
    /// This ignores concurrency of transfers.
    double max_rate;
    double avg_rate;        ///< recent average transfer rate
    double avg_time;        ///< when avg_rate was last updated
    
    NET_INFO();
    
    /// Updates the variables in this structure.
    void update(double nbytes, double dt);

    /// Resets all variables to zero.
    void clear();
};

class NET_STATS {
public:
    //double last_time;
    NET_INFO up;
    NET_INFO down;

    NET_STATS();
    //void poll(FILE_XFER_SET&, HTTP_OP_SET&);

    void write(MIOFILE&) const;
    int parse(MIOFILE&);
};

class NET_STATUS {
public:
    /// contact the reference site as soon as GUI_HTTP is idle.
    /// polled from NET_STATS::poll(), for want of a better place.
    bool need_to_contact_reference_site;
    void contact_reference_site();
    bool show_ref_message;
    /// client wants to do network comm and no physical connection exists.
    /// Initially false; set whenever a Curl operation
    /// returns CURLE_COULDNT_RESOLVE_HOST,
    /// and a subsequent request to a highly-available site
    /// also returns CURLE_COULDNT_RESOLVE_HOST.
    /// cleared whenever we transfer data,
    /// or an operation returns some other value
    ///
    bool need_physical_connection;
    /// we have a network connection, but it's likely to go away soon,
    /// so do as much network comm as possible
    /// (e.g. report completed results)
    ///
    bool have_sporadic_connection;
    double last_comm_time;

    int network_status();
    void network_available();
    void got_http_error();
    NET_STATUS() {
        need_physical_connection = false;
        have_sporadic_connection = false;
        need_to_contact_reference_site = false;
        show_ref_message = false;
        last_comm_time = 0;
    }
    void poll();
};

/// This is used to access a reference website (like Yahoo or Google)
/// that is assumed to be 100% available.
/// It is used ONLY from the HTTP code, when a transaction fails
///
struct LOOKUP_WEBSITE_OP: public GUI_HTTP_OP {
    int error_num;

    virtual ~LOOKUP_WEBSITE_OP(){}
    int do_rpc(std::string&);
    virtual void handle_reply(int http_op_retval);
    LOOKUP_WEBSITE_OP(GUI_HTTP* p){
        error_num = BOINC_SUCCESS;
        gui_http = p;
    }
};

extern NET_STATUS net_status;

#endif
