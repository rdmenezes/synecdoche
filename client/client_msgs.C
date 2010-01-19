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
#include <cstdarg>
#include <cstring>
#include <deque>
#endif

#include "client_msgs.h"

#include "str_util.h"
#include "log_flags.h"
#include "client_types.h"
#include "diagnostics.h"

#ifdef _WIN32
#include "client_state.h"
#endif

#define MAX_SAVED_MESSAGES 1000

/// a dequeue of up to MAX_SAVED_MESSAGES most recent messages,
/// stored in newest-first order.
std::deque<MESSAGE_DESC*> message_descs;

/// Takes a printf style formatted string, inserts the proper values,
/// and passes it to show_message.
/// \todo add translation functionality
void msg_printf(const PROJECT *p, MSG_PRIORITY priority, const char *fmt, ...) {
    char        buf[8192];  // output can be much longer than format
    va_list     ap;

    if (fmt == NULL) return;

    va_start(ap, fmt); // Parses string for variables
    vsnprintf(buf, sizeof(buf), fmt, ap); // And convert symbols To actual numbers
    buf[sizeof(buf) - 1] = 0;
    va_end(ap); // Results are stored in text

    show_message(p, buf, priority);
}

/// Add message to cache and delete old messages if the cache gets too big.
void record_message(const PROJECT* p, MSG_PRIORITY priority, int now, const char* message) {
    MESSAGE_DESC* mdp = new MESSAGE_DESC;
    static int seqno = 1;
    strcpy(mdp->project_name, "");
    if (p) {
        strlcpy(mdp->project_name, p->get_project_name(), sizeof(mdp->project_name));
    }
    mdp->priority = priority;
    mdp->timestamp = now;
    mdp->seqno = seqno++;
    mdp->message = message;
    while (message_descs.size() > MAX_SAVED_MESSAGES) {
        delete message_descs.back();
        message_descs.pop_back();
    }
    message_descs.push_front(mdp);
}

/// Display a message to the user.
/// Depending on the priority, the message may be more or less obtrusive
///
/// \param[in] p Pointer to a project to which the message belongs. May be
///              set to 0 if the message does not belong to any project.
/// \param[in] msg A string containing the message that should be displayed.
/// \param[in] priority A value describing the priority of the
///                     message. See common_defs.h for possible values.
void show_message(const PROJECT *p, const std::string& msg, MSG_PRIORITY priority) {
    const char* x;

    time_t now = time(0);
    std::string time_string = time_to_string((double)now);
#if defined(_WIN32) && defined(_CONSOLE)
    char event_message[2048];
#endif

    // Cycle the log files if we need to
    diagnostics_cycle_logs();

    std::string message(msg);
    if (priority == MSG_INTERNAL_ERROR) {
        message.insert(0, "[error] ");
    }

    // Trim trailing \n's:
    std::string::size_type pos = message.find_last_not_of('\n');
    if (pos == std::string::npos) {
        message.clear();
    } else if (pos < message.length() - 1) {
        message.erase(pos + 1);
    }

    if (p) {
        x = p->get_project_name();
    } else {
        x = "---";
    }

    record_message(p, priority, (int)now, message.c_str());

    printf("%s [%s] %s\n", time_string.c_str(), x, message.c_str());

#if defined(_WIN32) && defined(_CONSOLE)
    if (gstate.executing_as_daemon) {
        stprintf(event_message, TEXT("%s [%s] %s\n"), time_string,  x, message.c_str());
        ::OutputDebugString(event_message);
    }
#endif
}
