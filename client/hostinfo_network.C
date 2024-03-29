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
#include <cstdio>
#include <cstring>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "str_util.h"
#include "parse.h"
#include "file_names.h"
#include "client_msgs.h"
#include "error_numbers.h"

#include "hostinfo.h"

/// get domain name and IP address of this host
///
int HOST_INFO::get_local_network_info() {
    struct in_addr addr;
    struct hostent* he;

    strcpy(domain_name, "");
    strcpy(ip_addr, "");

    // it seems like we should use getdomainname() instead of gethostname(),
    // but on FC6 it returns "(none)".
    //
    if (gethostname(domain_name, 256)) return ERR_GETHOSTBYNAME;
    he = gethostbyname(domain_name);
    if (!he || !he->h_addr_list[0]) {
        //msg_printf(NULL, MSG_ERROR, "gethostbyname (%s) failed", domain_name);
        return ERR_GETHOSTBYNAME;
    }
    memcpy(&addr, he->h_addr_list[0], sizeof(addr));
    strlcpy(ip_addr, inet_ntoa(addr), sizeof(ip_addr));
    return 0;
}
