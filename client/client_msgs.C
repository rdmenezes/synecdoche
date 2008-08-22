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

using std::deque;

#include "log_flags.h"
#include "client_types.h"
#include "client_msgs.h"

#define MAX_SAVED_MESSAGES 1000

// a dequeue of up to MAX_SAVED_MESSAGES most recent messages,
// stored in newest-first order
//
deque<MESSAGE_DESC*> message_descs;

// Takes a printf style formatted string, inserts the proper values,
// and passes it to show_message
// TODO: add translation functionality
//
void msg_printf(const PROJECT *p, int priority, const char *fmt, ...) {
    char        buf[8192];  // output can be much longer than format
    va_list     ap;

    if (fmt == NULL) return;

    va_start(ap, fmt); // Parses string for variables
    vsprintf(buf, fmt, ap); // And convert symbols To actual numbers
    va_end(ap); // Results are stored in text

    show_message(p, buf, priority);
}

// stash message in memory
//
void record_message(const PROJECT* p, int priority, int now, const char* message) {
    MESSAGE_DESC* mdp = new MESSAGE_DESC;
    static int seqno = 1;
    strcpy(mdp->project_name, "");
    if (p) {
        strcpy(mdp->project_name, p->get_project_name());
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

const char *BOINC_RCSID_9572274f4f = "$Id: client_msgs.C 14436 2007-12-21 21:09:40Z davea $";