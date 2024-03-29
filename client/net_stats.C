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
/// NET_STATS estimates average network throughput,
/// i.e the average total throughput in both the up and down directions.
///
/// NET_STATUS keeps track of whether we have a physical connection,
/// and whether we need one.

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include "net_stats.h"

#include <cstring>
#include <cmath>
#include <ostream>

#include "parse.h"
#include "miofile.h"
#include "str_util.h"
#include "error_numbers.h"
#include "util.h"
#include "xml_write.h"

#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "pers_file_xfer.h"

#define NET_RATE_HALF_LIFE  (7*86400)

NET_STATUS net_status;

NET_INFO::NET_INFO() {
    clear();
}

/// Updates the variables in this structure.
/// Called after file xfer to update rates.
///
/// \param[in] nbytes Number of bytes transferred
/// \param[in] dt Time in seconds which passed since the file xfer started.
void NET_INFO::update(double nbytes, double dt) {
    if (nbytes == 0.0 || dt == 0.0) {
        return;
    }
    double bytes_sec = nbytes / dt;
    if (max_rate == 0.0) {
        max_rate = bytes_sec;   // first time
    } else {
        // somewhat arbitrary weighting formula
        double w = log(nbytes) / 500.0;
        if (w > 1) {
            w = 1;
        }
        max_rate = w * bytes_sec + (1 - w) * max_rate;
    }
    double start_time = gstate.now - dt;
    update_average(
        start_time,
        nbytes,
        NET_RATE_HALF_LIFE,
        avg_rate,
        avg_time
    );
}

/// Resets all variables to zero.
void NET_INFO::clear() {
    max_rate = 0.0;
    avg_rate = 0.0;
    avg_time = 0.0;
}

NET_STATS::NET_STATS() {
}

void NET_STATS::write(std::ostream& out) const {
    out << "<net_stats>\n"
        << XmlTag<double>("bwup",          up.max_rate)
        << XmlTag<double>("avg_up",        up.avg_rate)
        << XmlTag<double>("avg_time_up",   up.avg_time)
        << XmlTag<double>("bwdown",        down.max_rate)
        << XmlTag<double>("avg_down",      down.avg_rate)
        << XmlTag<double>("avg_time_down", down.avg_time)
        << "</net_stats>\n"
    ;
}

int NET_STATS::parse(MIOFILE& in) {
    char buf[256];

    up.clear();
    down.clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</net_stats>")) return 0;
        if (parse_double(buf, "<bwup>", up.max_rate)) continue;
        if (parse_double(buf, "<avg_up>", up.avg_rate)) continue;
        if (parse_double(buf, "<avg_time_up>", up.avg_time)) continue;
        if (parse_double(buf, "<bwdown>", down.max_rate)) continue;
        if (parse_double(buf, "<avg_down>", down.avg_rate)) continue;
        if (parse_double(buf, "<avg_time_down>", down.avg_time)) continue;
        handle_unparsed_xml_warning("NET_STATS::parse", buf);
    }
    return ERR_XML_PARSE;
}

/// Return:
/// - ONLINE if we have network connections open
/// - WANT_CONNECTION  if we need a physical connection
/// - WANT_DISCONNECT if we don't have any connections, and don't need any
/// - LOOKUP_PENDING if a website lookup is pending (try again later)
///
/// There's a 10-second slop factor:
/// if we've done network communication in the last 10 seconds,
/// we act as if we're doing it now.
/// (so that polling mechanisms have a chance to start other transfers,
/// in the case of a modem connection waiting to be closed by the manager)
///
int NET_STATUS::network_status() {
    int retval;

    if (gstate.http_ops->nops()) {
        last_comm_time = gstate.now;
    }
    if (need_to_contact_reference_site) {
        retval = NETWORK_STATUS_LOOKUP_PENDING;
    } else if (gstate.lookup_website_op.error_num == ERR_IN_PROGRESS) {
        retval = NETWORK_STATUS_LOOKUP_PENDING;
    } else if (gstate.now - last_comm_time < 10) {
        retval = NETWORK_STATUS_ONLINE;
    } else if (need_physical_connection) {
        retval = NETWORK_STATUS_WANT_CONNECTION;
    } else if (gstate.active_tasks.want_network()) {
        retval = NETWORK_STATUS_WANT_CONNECTION;
    } else {
        have_sporadic_connection = false;
        retval = NETWORK_STATUS_WANT_DISCONNECT;
    }
    if (log_flags.network_status_debug) {
        msg_printf(NULL, MSG_INFO, "[network_status_debug] status: %s", network_status_string(retval));
    }
    return retval;
}

/// There's now a network connection, after some period of disconnection.
/// Do all communication that we can.
///
void NET_STATUS::network_available() {
    unsigned int i;

    have_sporadic_connection = true;
    for (i=0; i<gstate.pers_file_xfers->pers_file_xfers.size(); i++) {
        PERS_FILE_XFER* pfx = gstate.pers_file_xfers->pers_file_xfers[i];
        pfx->next_request_time = 0;
    }
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->min_rpc_time = 0;
    }

    // tell active tasks that network is available (for Folding@home)
    //
    gstate.active_tasks.network_available();
}

/// An HTTP operation failed;
/// it could be because there's no physical network connection.
/// Find out for sure by trying to contact Google.
void NET_STATUS::got_http_error() {
    if (gstate.lookup_website_op.error_num == ERR_IN_PROGRESS) {
        return;
    }
    if (need_physical_connection) {
        return;
    }
    if (config.dont_contact_ref_site) {
        return;
    }
    if (log_flags.network_status_debug) {
        msg_printf(0, MSG_INFO, "[network_status_debug] got HTTP error - checking ref site");
    }
    need_to_contact_reference_site = true;
    show_ref_message = true;
}

void NET_STATUS::contact_reference_site() {
    std::string url = "http://www.google.com";
    if (log_flags.network_status_debug) {
        msg_printf(0, MSG_INFO,
            "[network_status_debug] need_phys_conn %d; trying google", need_physical_connection
        );
    }
    gstate.lookup_website_op.do_rpc(url);
    need_to_contact_reference_site = false;
}

int LOOKUP_WEBSITE_OP::do_rpc(std::string& url) {
    int retval;

    if (net_status.show_ref_message) {
        msg_printf(0, MSG_INFO, "Project communication failed: attempting access to reference site");
    }
    retval = gui_http->do_rpc(this, url, LOOKUP_WEBSITE_FILENAME);
    if (retval) {
        error_num = retval;
        net_status.need_physical_connection = true;
        net_status.last_comm_time = 0;
        msg_printf(0, MSG_USER_ERROR,
            "Synecdoche can't access Internet - check network connection or proxy configuration."
        );
    } else {
        error_num = ERR_IN_PROGRESS;
    }
    return retval;
}

void LOOKUP_WEBSITE_OP::handle_reply(int http_op_retval) {
    error_num = http_op_retval;

    // if we couldn't contact a reference web site,
    // we can assume there's a problem that requires user attention
    // (usually no physical network connection).
    // Set a flag that will signal the Manager to that effect
    //
    if (http_op_retval) {
        net_status.need_physical_connection = true;
        net_status.last_comm_time = 0;
        msg_printf(0, MSG_USER_ERROR,
            "Synecdoche can't access Internet - check network connection or proxy configuration."
        );
    } else {
        if (net_status.show_ref_message) {
            msg_printf(0, MSG_INFO,
                "Internet access OK - project servers may be temporarily down."
            );
        }
    }
}

void NET_STATUS::poll() {
    // For 30 seconds after wakeup, the network system (DNS etc.)
    // may still be coming up, so defer the reference site check;
    // otherwise might show spurious "need connection" message
    if (gstate.now < gstate.last_wakeup_time + 30) {
        return;
    }
    if (net_status.need_to_contact_reference_site && gstate.gui_http.state==GUI_HTTP_STATE_IDLE) {
        net_status.contact_reference_site();
    }
}
