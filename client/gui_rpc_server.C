// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez
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
/// The plumbing of GUI %RPC, server side
/// (but not the actual RPCs)

#include "gui_rpc_server.h"

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include <cerrno>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#include "client_msgs.h"
#include "log_flags.h"
#include "client_state.h"
#include "file_names.h"
#include "filesys.h"
#include "sandbox.h"
#include "str_util.h"
#include "network.h"
#include "md5_file.h"
#include "hostinfo.h"

#include <cstdio>
#include <cstring>
#include <vector>
#include <set>

// This function is actually declared in gui_rpc_client.h, but we can't include
// this file here because it re-defines some classes used by the gui-rpc-server.
// TODO: Clean this mess up!
std::string read_gui_rpc_password(const std::string& file_name = GUI_RPC_PASSWD_FILE);

GUI_RPC_CONN::GUI_RPC_CONN(int fd):
    sock(fd),
    auth_needed(false),

    get_project_config_op(&gui_http),
    lookup_account_op(&gui_http),
    create_account_op(&gui_http)
{
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
    boinc_close_socket(sock);
}

GUI_RPC_CONN_SET::GUI_RPC_CONN_SET() {
    lsock = -1;
}

bool GUI_RPC_CONN_SET::poll() {
    unsigned int i;
    bool action = false;
    for (i=0; i<gui_rpcs.size(); i++) {
        if (gui_rpcs[i]->gui_http.poll()) {
            action = true;
        }
    }
    return action;
}

int GUI_RPC_CONN_SET::get_password() {
    strcpy(password, "");

    if (boinc_file_exists(GUI_RPC_PASSWD_FILE)) {
        std::string buf = read_gui_rpc_password();
        strlcpy(password, buf.c_str(), sizeof(password));
    } else {
        // if no password file, make a random password
        //
        int retval = make_random_string(password);
        if (retval) {
            if (config.os_random_only) {
                msg_printf(
                    NULL, MSG_INTERNAL_ERROR,
                    "OS random string generation failed, exiting"
                );
                exit(1);
            }
            gstate.host_info.make_random_string("guirpc", password);
        }
        FILE* f = fopen(GUI_RPC_PASSWD_FILE, "w");
        if (f) {
            fputs(password, f);
            fclose(f);
#ifndef _WIN32
            // if someone can read the password,
            // they can cause code to execute as this user.
            // So better protect it.
            //
            if (g_use_sandbox) {
                // Allow group access so authorized administrator can modify it
                chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR | S_IRGRP | S_IWGRP);
            } else {
                chmod(GUI_RPC_PASSWD_FILE, S_IRUSR|S_IWUSR);
            }
#endif
        }
    }
    return 0;
}

int GUI_RPC_CONN_SET::get_allowed_hosts() {
    int ipaddr, retval;
    char buf[256];

    allowed_remote_ip_addresses.clear();
    remote_hosts_file_exists = false;

    // scan remote_hosts.cfg, convert names to IP addresses
    //
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f) {
        remote_hosts_file_exists = true;
        if (log_flags.guirpc_debug) {
            msg_printf(0, MSG_INFO,
                "[guirpc_debug] found allowed hosts list"
            );
        }

        // read in each line, if it is not a comment
        // then resolve the address and add to our allowed list
        //
        memset(buf,0,sizeof(buf));
        while (fgets(buf, 256, f)) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                retval = resolve_hostname(buf, ipaddr);
                if (retval) {
                    msg_printf(0, MSG_USER_ERROR,
                        "Can't resolve hostname %s in %s",
                        buf, REMOTEHOST_FILE_NAME
                    );
                } else {
                    allowed_remote_ip_addresses.insert(ntohl(ipaddr));
                }
            }
        }
        fclose(f);
    }
    return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* rpc_conn) {
    gui_rpcs.push_back(rpc_conn);
    return 0;
}

// If the core client runs at boot time,
// it may be a while (~10 sec) before the DNS system is working.
// If this returns an error, it will get called once a second
// for up to 30 seconds.
// On the last call, "last_time" is set; print error messages then.
//
int GUI_RPC_CONN_SET::init(bool last_time) {
    sockaddr_in addr;
    int retval;

    get_password();
    get_allowed_hosts();

    retval = boinc_socket(lsock);
    if (retval) {
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "GUI RPC failed to create socket: %d", lsock
            );
        }
        return retval;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    if (gstate.cmdline_gui_rpc_port) {
        addr.sin_port = htons(gstate.cmdline_gui_rpc_port);
    } else {
        addr.sin_port = htons(GUI_RPC_PORT);
    }

#ifdef __APPLE__
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
#else
    if (gstate.allow_remote_gui_rpc || remote_hosts_file_exists) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (log_flags.guirpc_debug) {
            msg_printf(NULL, MSG_INFO, "[guirpc_debug] Remote control allowed");
        }
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (log_flags.guirpc_debug) {
            msg_printf(NULL, MSG_INFO, "[guirpc_debug] Local control only allowed");
        }
    }
#endif

    int one = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (char*)&one, 4);

    retval = bind(lsock, (const sockaddr*)(&addr), (boinc_socklen_t)sizeof(addr));
    if (retval) {
#ifdef _WIN32
        retval = WSAGetLastError(); // Display the real error code
#else
        retval = errno;             // Display the real error code
#endif // _WIN32
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "GUI RPC bind to port %d failed: %d", ntohs(addr.sin_port), retval);
        }
        boinc_close_socket(lsock);
        lsock = -1;
        return ERR_BIND;
    }
    if (log_flags.guirpc_debug) {
        msg_printf(NULL, MSG_INFO, "[guirpc_debug] Listening on port %d", ntohs(addr.sin_port));
    }

    retval = listen(lsock, 999);
    if (retval) {
        if (last_time) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "GUI RPC listen failed: %d", retval
            );
        }
        boinc_close_socket(lsock);
        lsock = -1;
        return ERR_LISTEN;
    }
    return 0;
}

static void show_connect_error(in_addr ia) {
    static double last_time=0;
    static int count=0;

    if (last_time == 0) {
        last_time = gstate.now;
        count = 1;
    } else {
        if (gstate.now - last_time < 600) {
            count++;
            return;
        }
        last_time = gstate.now;
    }
    msg_printf(
        NULL, MSG_USER_ERROR,
        "GUI RPC request from non-allowed address %s",
        inet_ntoa(ia)
    );
    if (count > 1) {
        msg_printf(
            NULL, MSG_USER_ERROR,
            "%d connections rejected in last 10 minutes",
            count
        );
    }
    count = 0;
}
bool GUI_RPC_CONN::needs_write() const {
    return !write_buffer.empty();
}

void GUI_RPC_CONN_SET::get_fdset(FDSET_GROUP& fds) const {
    const GUI_RPC_CONN* gr;

    if (lsock < 0) return;
    for (size_t i=0; i<gui_rpcs.size(); i++) {
        gr = gui_rpcs[i];
        int s = gr->sock;
        FD_SET(s, &fds.read_fds);
        FD_SET(s, &fds.exc_fds);
        if (gr->needs_write()) {
            FD_SET(s, &fds.write_fds);
        }
        if (s > fds.max_fd) fds.max_fd = s;
    }
    FD_SET(lsock, &fds.read_fds);
    if (lsock > fds.max_fd) fds.max_fd = lsock;
}

bool GUI_RPC_CONN_SET::check_allowed_list(unsigned long ip_addr) const {
    return allowed_remote_ip_addresses.count(ip_addr) == 1;
}

void GUI_RPC_CONN_SET::got_select(FDSET_GROUP& fds) {
    int retval;
    std::vector<GUI_RPC_CONN*>::iterator iter;

    if (lsock < 0) return;

    if (FD_ISSET(lsock, &fds.read_fds)) {
        struct sockaddr_in addr;
        boinc_socklen_t addr_len = sizeof(addr);
        int sock = accept(lsock, (struct sockaddr*)&addr, (boinc_socklen_t*)&addr_len);
        if (sock == -1) {
            return;
        }

        // apps shouldn't inherit the socket!
#ifndef _WIN32
        fcntl(sock, F_SETFD, FD_CLOEXEC);
#endif

        int peer_ip = (int) ntohl(addr.sin_addr.s_addr);
        bool allowed = false, is_local = false;

        // accept the connection if:
        // 1) allow_remote_gui_rpc is set or
        // 2) client host is included in "remote_hosts" file or
        // 3) client is on localhost
        //
        if (peer_ip == 0x7f000001) {
            allowed = true;
            is_local = true;
        } else {
            // reread host file because IP addresses might have changed
            //
            get_allowed_hosts();
            allowed = check_allowed_list(peer_ip);
        }

        if (!(gstate.allow_remote_gui_rpc) && !allowed) {
            in_addr ia;
            ia.s_addr = htonl(peer_ip);
            show_connect_error(ia);
            boinc_close_socket(sock);
        } else {
            GUI_RPC_CONN* gr = new GUI_RPC_CONN(sock);
            if (strlen(password)) {
                gr->auth_needed = true;
            }
            gr->is_local = is_local;
            insert(gr);
        }
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        GUI_RPC_CONN* gr = *iter;
        if (FD_ISSET(gr->sock, &fds.exc_fds)) {
            delete gr;
            iter = gui_rpcs.erase(iter);
            continue;
        }
        ++iter;
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        GUI_RPC_CONN* gr = *iter;
        if (FD_ISSET(gr->sock, &fds.read_fds)) {
            retval = gr->handle_rpc();
            if (retval) {
                if (log_flags.guirpc_debug) {
                    msg_printf(NULL, MSG_INFO,
                        "[guirpc_debug] error %d from handler, closing socket\n",
                        retval
                    );
                }
                delete gr;
                iter = gui_rpcs.erase(iter);
                continue;
            }
        }
        ++iter;
    }
    iter = gui_rpcs.begin();
    while (iter != gui_rpcs.end()) {
        GUI_RPC_CONN* gr = *iter;
        if (FD_ISSET(gr->sock, &fds.write_fds)) {
            retval = gr->handle_write();
            if (retval) {
                if (log_flags.guirpc_debug) {
                    msg_printf(NULL, MSG_INFO,
                        "[guirpc_debug] error %d from write handler, closing socket\n",
                        retval
                    );
                }
                delete gr;
                iter = gui_rpcs.erase(iter);
                continue;
            }
        }
        ++iter;
    }
}

void GUI_RPC_CONN_SET::close() {
    if (log_flags.guirpc_debug) {
        msg_printf(NULL, MSG_INFO,
            "[guirpc_debug] closing GUI RPC listening socket %d\n", lsock
        );
    }
    if (lsock >= 0) {
        boinc_close_socket(lsock);
        lsock = -1;
    }
}
