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

#ifndef BOINC_NETWORK_H
#define BOINC_NETWORK_H

#ifndef _WIN32
# include <unistd.h>
# include "config.h"
# if HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif
# include <string.h>
#else
# include "boinc_win.h"
#endif

class FDSET_GROUP {
public:
    fd_set read_fds;
    fd_set write_fds;
    fd_set exc_fds;
    int max_fd;

    void zero() {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&exc_fds);
        max_fd = -1;
    }
};

/// Resolve a hostname (IPv4 only).
int resolve_hostname(const char* hostname, int& ip_addr);

int boinc_socket(int& sock);
int boinc_socket_asynch(int sock, bool asynch);
void boinc_close_socket(int sock);
int get_socket_error(int fd);

/// Return a string describing the current network error value.
const char* socket_error_str();

#if defined(_WIN32) && defined(USE_WINSOCK)
typedef int boinc_socklen_t;
#define SHUT_WR SD_SEND
#else
typedef BOINC_SOCKLEN_T boinc_socklen_t;
#endif


#ifndef NETWORK_ALIVE_LAN
#define NETWORK_ALIVE_LAN   0x00000001
#endif

#ifndef NETWORK_ALIVE_WAN
#define NETWORK_ALIVE_WAN   0x00000002
#endif

#ifndef NETWORK_ALIVE_AOL
#define NETWORK_ALIVE_AOL   0x00000004
#endif


#if defined(_WIN32) && defined(USE_WINSOCK)
int WinsockInitialize();
int WinsockCleanup();
#endif

#endif
