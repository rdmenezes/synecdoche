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

#ifndef CLIENT_MSG_LOG_H
#define CLIENT_MSG_LOG_H

// write messages ONLY as follows:
// if (log_flags.X) {
//     msg_printf();
// }

#include <algorithm>
#include <deque>
#include <string>

#include "log_flags.h"

class PROJECT;

// the following stores a message in memory, where it can be retrieved via RPC
//
struct MESSAGE_DESC {
    char project_name[256];
    int priority;
    int timestamp;
    int seqno;
    std::string message;
};

extern std::deque<MESSAGE_DESC*> message_descs;
extern void record_message(const PROJECT *p, int priority, int now, const char* message);
extern void show_message(const PROJECT *p, const char* message, int priority);

// the __attribute((format...)) tags are GCC extensions that let the compiler
// do like-checking on printf-like arguments
//
#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x) /*nothing*/
#endif

// Show a message, preceded by timestamp and project name
//
extern void msg_printf(const PROJECT *p, int priority, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

#endif
