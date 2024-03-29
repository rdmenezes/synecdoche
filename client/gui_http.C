// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#endif

#include <string>

#include "gui_http.h"

#include "client_state.h"
#include "filesys.h"
#include "error_numbers.h"

int GUI_HTTP::do_rpc(GUI_HTTP_OP* op, std::string url, std::string output_file) {
    int retval;

    if (state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    http_op.set_proxy(&gstate.proxy_info);
    boinc_delete_file(output_file.c_str());
    retval = http_op.init_get(url.c_str(), output_file.c_str(), true);
    if (!retval) retval = gstate.http_ops->insert(&http_op);
    if (!retval) {
        gui_http_op = op;
        state = GUI_HTTP_STATE_BUSY;
    }
    return retval;
}

int GUI_HTTP::do_rpc_post(GUI_HTTP_OP* op, std::string url, std::string input_file, std::string output_file) {
    int retval;

    if (state != GUI_HTTP_STATE_IDLE) {
        return ERR_RETRY;
    }

    http_op.set_proxy(&gstate.proxy_info);
    boinc_delete_file(output_file.c_str());
    retval = http_op.init_post(url.c_str(), input_file.c_str(), output_file.c_str());
    if (!retval) retval = gstate.http_ops->insert(&http_op);
    if (!retval) {
        gui_http_op = op;
        state = GUI_HTTP_STATE_BUSY;
    }
    return retval;
}

bool GUI_HTTP::poll() {
    if (state == GUI_HTTP_STATE_IDLE) return false;
    static double last_time=0;
    if (gstate.now-last_time < 1) return false;
    last_time = gstate.now;

    if (http_op.http_op_state == HTTP_STATE_DONE) {
        gstate.http_ops->remove(&http_op);
        gui_http_op->handle_reply(http_op.http_op_retval);
        gui_http_op = NULL;
        state = GUI_HTTP_STATE_IDLE;
    }
    return true;
}
