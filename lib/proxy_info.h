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

#ifndef _PROXY_INFO_
#define _PROXY_INFO_

class MIOFILE;

struct PROXY_INFO {
    bool use_http_proxy;
    bool use_socks_proxy;
    bool use_http_auth;
    int socks_version;
    char socks_server_name[256];
    char http_server_name[256];
    int socks_server_port;
    int http_server_port;
    char http_user_name[256];
    char http_user_passwd[256];
    char socks5_user_name[256];
    char socks5_user_passwd[256];

    int parse(MIOFILE& in);
    int write(MIOFILE& out) const;
    void clear();

    PROXY_INFO& operator=(const PROXY_INFO& rhs);
};

#endif
