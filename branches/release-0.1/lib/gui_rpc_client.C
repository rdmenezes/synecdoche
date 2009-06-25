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

// This file is the underlying mechanism of GUI RPC client
// (not the actual RPCs)

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_) 
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "../version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <sstream>
#endif

#include "gui_rpc_client.h"
#include "diagnostics.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "network.h"
#include "common_defs.h"

RPC_CLIENT::RPC_CLIENT() {
    sock = -1;
}

RPC_CLIENT::~RPC_CLIENT() {
    close();
}

// if any RPC returns ERR_READ or ERR_WRITE,
// call this and then call init() again.
//
void RPC_CLIENT::close() {
    //fprintf(stderr, "RPC_CLIENT::close called\n");
    if (sock>=0) {
        boinc_close_socket(sock);
        sock = -1;
    }
}

/// Initiate a connection to the core client.
///
/// \param[in] host The host name of the machine the core client is running on.
/// \param[in] port The port the core client is listening on for incoming
///                 connections.
/// \return Zero on success, nonzero if any error occurred.
int RPC_CLIENT::init(const char* host, int port) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (host) {
        hostent* hep = gethostbyname(host);
        if (!hep) {
            return ERR_GETHOSTBYNAME;
        }
        addr.sin_addr.s_addr = *(int*)hep->h_addr_list[0];
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    boinc_socket(sock);
    int retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
#ifdef _WIN32
        BOINCTRACE("RPC_CLIENT::init connect 2: Winsock error '%d'\n", WSAGetLastError());
#endif
        BOINCTRACE("RPC_CLIENT::init connect on %d returned %d\n", sock, retval);
        close();
        return ERR_CONNECT;
    }

    return 0;
}

/// Initiate a connection to the core client using non-blocking operations.
///
/// \param[in] host The host name of the machine the core client is running on.
/// \param[in] _timeout How long to wait until give up
///                     If the caller (i.e. Synecdoche Manager) just launched
///                     the core client, this should be large enough to allow
///                     the process to run and open its listening socket
///                     (e.g. 60 sec).
///                     If connecting to a remote client, it should be large
///                     enough for the user to deal with a "personal firewall"
///                     popup (e.g. 60 sec).
/// \param[in] _retry If true, keep retrying until succeed or timeout.
///                   Use this if just launched the core client.
/// \param[in] port The port the core client is listening on for incoming
///                 connections.
/// \return Zero on success, nonzero if any error occurred.
int RPC_CLIENT::init_asynch(const char* host, double _timeout, bool _retry, int port) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    retry = _retry;
    timeout = _timeout;

    if (host) {
        hostent* hep = gethostbyname(host);
        if (!hep) {
            return ERR_GETHOSTBYNAME;
        }
        addr.sin_addr.s_addr = *(int*)hep->h_addr_list[0];
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    int retval = boinc_socket(sock);
    BOINCTRACE("RPC_CLIENT::init boinc_socket returned %d\n", sock);
    if (retval) {
        return retval;
    }

    retval = boinc_socket_asynch(sock, true);
    if (retval) {
        BOINCTRACE("RPC_CLIENT::init asynch error: %d\n", retval);
    }
    start_time = dtime();
    retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        BOINCTRACE("RPC_CLIENT::init connect returned %d\n", retval);
    }
    BOINCTRACE("RPC_CLIENT::init attempting connect \n");
    return 0;
}

int RPC_CLIENT::init_poll() {
    fd_set read_fds, write_fds, error_fds;
    struct timeval tv;
    int retval;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    FD_SET(sock, &read_fds);
    FD_SET(sock, &write_fds);
    FD_SET(sock, &error_fds);

    BOINCTRACE("RPC_CLIENT::init_poll sock = %d\n", sock);

    tv.tv_sec = tv.tv_usec = 0;
    select(FD_SETSIZE, &read_fds, &write_fds, &error_fds, &tv);
    retval = 0;
    if (FD_ISSET(sock, &error_fds)) {
        retval =  ERR_CONNECT;
    } else if (FD_ISSET(sock, &write_fds)) {
        retval = get_socket_error(sock);
        if (!retval) {
            BOINCTRACE("RPC_CLIENT::init_poll connected to port %d\n", ntohs(addr.sin_port));
            retval = boinc_socket_asynch(sock, false);
            if (retval) {
                BOINCTRACE("asynch error: %d\n", retval);
                return retval;
            }
            return 0;
        } else {
            BOINCTRACE("init_poll: get_socket_error(): %d\n", retval);
        }
    }
    if (dtime() > start_time + timeout) {
        BOINCTRACE("RPC_CLIENT init timed out\n");
        return ERR_CONNECT;
    }
    if (retval) {
        if (retry) {
            boinc_close_socket(sock);
            retval = boinc_socket(sock);
            retval = boinc_socket_asynch(sock, true);
            retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
            BOINCTRACE("RPC_CLIENT::init_poll attempting connect\n");
            return ERR_RETRY;
        } else {
            return ERR_CONNECT;
        }
    }
    return ERR_RETRY;
}

/// Answer an authorization request sent by the server.
/// The answer is the response to the nonce sent as challenge by the server.
/// It is the md5sum from the concatenation of the nonce and the
/// GUI-RPC password.
///
/// \param[in] passwd The GUI-RPC password.
/// \return Zero if the authorization was acknowledged by the server,
///         nonzero if the server rejected the request or if any other error
///         occurred.
int RPC_CLIENT::authorize(const char* passwd) {
    RPC rpc(this);
    int retval = rpc.do_rpc("<auth1/>\n");
    if (retval) {
        return retval;
    }

    XML_PARSER xp(&rpc.fin);
    char nonce[256];
    bool found = false;
    char chr_buf[256];
    bool is_tag;
    while (!xp.get(chr_buf, sizeof(chr_buf), is_tag)) {
        if (!is_tag) {
            continue;
        }
        if (xp.parse_str(chr_buf, "nonce", nonce, sizeof(nonce))) {
            found = true;
            break;
        }
    }
    if (!found) {
        //fprintf(stderr, "Nonce not found\n");
        return ERR_AUTHENTICATOR;
    }

    std::ostringstream input;
    input << nonce << passwd;
    std::string nonce_hash = md5_string(input.str());
    std::ostringstream buf;
    buf << "<auth2>\n<nonce_hash>" << nonce_hash << "</nonce_hash>\n</auth2>\n";
    retval = rpc.do_rpc(buf.str().c_str());
    if (retval) {
        return retval;
    }

    while (!xp.get(chr_buf, sizeof(chr_buf), is_tag)) {
        if (!is_tag) {
            continue;
        }
        bool authorized;
        if (xp.parse_bool(chr_buf, "authorized", authorized)) {
            if (authorized) {
                return 0;
            }
        }
    }
    return ERR_AUTHENTICATOR;
}

/// Send a rpc-request to the rpc-server.
///
/// \param[in] p The rpc-request.
/// \return Zero on success, ERR_WRITE on error.
int RPC_CLIENT::send_request(const char* p) {
    std::string buf("<boinc_gui_rpc_request>\n");
    buf.append(p).append("</boinc_gui_rpc_request>\n\003");
    int n = send(sock, buf.c_str(), static_cast<int>(buf.size()), 0);
    if (n < 0) {
        return ERR_WRITE;
    }
    return 0;
}

/// Get reply from server. 
/// Caller must free the buffer.
///
/// \param[in,out] mbuf Reference to a pointer which will point to the
///                     string received from the rpc-server.
/// \return Zero on success, ERR_READ on error.
int RPC_CLIENT::get_reply(char*& mbuf) {
    MFILE mf;
    int n;

    while (true) {
        char buf[8193];
        n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            return ERR_READ;
        }
        buf[n]=0;
        mf.puts(buf);
        if (strchr(buf, '\003')) {
            break;
        }
    }
    mf.get_buf(mbuf, n);
    return 0;
}

RPC::RPC(RPC_CLIENT* rc) {
    mbuf = 0;
    rpc_client = rc;
}

RPC::~RPC() {
    if (mbuf) free(mbuf);
}

int RPC::do_rpc(const char* req) {
    int retval;

    //fprintf(stderr, "RPC::do_rpc rpc_client->sock = '%d'", rpc_client->sock);
    if (rpc_client->sock == -1) return ERR_CONNECT;
#ifdef DEBUG
    puts(req);
#endif
    retval = rpc_client->send_request(req);
    if (retval) return retval;
    retval = rpc_client->get_reply(mbuf);
    if (retval) return retval;
    fin.init_buf_read(mbuf);
#ifdef DEBUG
    puts(mbuf);
#endif
    return 0;
}

int RPC::parse_reply() {
    char buf[256];
    while (fin.fgets(buf, 256)) {
        if (strstr(buf, "unauthorized")) return ERR_AUTHENTICATOR;
        if (strstr(buf, "Missing authenticator")) return ERR_AUTHENTICATOR;
        if (strstr(buf, "Missing URL")) return ERR_INVALID_URL;
        if (strstr(buf, "Already attached to project")) return ERR_ALREADY_ATTACHED;
        if (strstr(buf, "success")) return BOINC_SUCCESS;
    }
    return ERR_NOT_FOUND;
}

/// Read the GUI-RPC-password from a file.
///
/// \param[in] file_name The file name containing the password. Defaults to
///                      GUI_RPC_PASSWD_FILE if omitted.
/// \return The GUI-RPC-password read from the file specified by \a file_name.
std::string read_gui_rpc_password(const std::string& file_name) {
    std::ifstream in(file_name.c_str());
    if (!in) {
        throw std::runtime_error(std::string("Can't open GUI-RPC-password file \"")
                                    + file_name + std::string("\"."));
    }
    std::string password;
    std::getline(in, password);
    return password;
}
