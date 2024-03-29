// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstring>
#include <string>
#endif

using std::string;

#include "proxy_info.h"

#include "parse.h"
#include "miofile.h"
#include "error_numbers.h"
#include "xml_write.h"

int PROXY_INFO::parse(MIOFILE& in) {
    char buf[1024];
    string s5un, s5up, hun, hup, temp;

    memset(this, 0, sizeof(PROXY_INFO));
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</proxy_info>")) return 0;
        else if (parse_bool(buf, "use_http_proxy", use_http_proxy)) continue;
        else if (parse_bool(buf, "use_socks_proxy", use_socks_proxy)) continue;
        else if (parse_bool(buf, "use_http_auth", use_http_auth)) continue;
        else if (parse_int(buf, "<socks_version>", socks_version)) continue;
        else if (parse_str(buf, "<socks_server_name>", socks_server_name, sizeof(socks_server_name))) continue;
        else if (parse_int(buf, "<socks_server_port>", socks_server_port)) continue;
        else if (parse_str(buf, "<http_server_name>", http_server_name, sizeof(http_server_name))) continue;
        else if (parse_int(buf, "<http_server_port>", http_server_port)) continue;
        else if (parse_str(buf, "<socks5_user_name>", socks5_user_name,sizeof(socks5_user_name))) continue;
        else if (parse_str(buf, "<socks5_user_passwd>", socks5_user_passwd,sizeof(socks5_user_passwd))) continue;
        else if (parse_str(buf, "<http_user_name>", http_user_name,sizeof(http_user_name))) continue;
        else if (parse_str(buf, "<http_user_passwd>", http_user_passwd,sizeof(http_user_passwd))) continue;
    }
    return ERR_XML_PARSE;
}

void PROXY_INFO::write(std::ostream& out) const {
    out << "<proxy_info>\n";
    if (use_http_proxy)  out << "<use_http_proxy/>\n";
    if (use_socks_proxy) out << "<use_socks_proxy/>\n";
    if (use_http_auth)   out << "<use_http_auth/>\n";

    out << XmlTag<int>      ("socks_version",      socks_version)
        << XmlTag<XmlString>("socks_server_name",  socks_server_name)
        << XmlTag<int>      ("socks_server_port",  socks_server_port)
        << XmlTag<XmlString>("http_server_name",   http_server_name)
        << XmlTag<int>      ("http_server_port",   http_server_port)
        << XmlTag<XmlString>("socks5_user_name",   socks5_user_name)
        << XmlTag<XmlString>("socks5_user_passwd", socks5_user_passwd)
        << XmlTag<XmlString>("http_user_name",     http_user_name)
        << XmlTag<XmlString>("http_user_passwd",   http_user_passwd)
        << "</proxy_info>\n"
    ;
}

void PROXY_INFO::clear() {
    use_http_proxy = false;
    use_socks_proxy = false;
    use_http_auth = false;
    strcpy(socks_server_name, "");
    strcpy(http_server_name, "");
    socks_server_port = 80;
    http_server_port = 80;
    strcpy(socks5_user_name, "");
    strcpy(socks5_user_passwd, "");
    strcpy(http_user_name, "");
    strcpy(http_user_passwd, "");
    socks_version = 0;
}

PROXY_INFO& PROXY_INFO::operator=(const PROXY_INFO& rhs)
{
    this->use_http_proxy = rhs.use_http_proxy;
    strcpy(this->http_user_name, rhs.http_user_name);
    strcpy(this->http_user_passwd, rhs.http_user_passwd);
    strcpy(this->http_server_name, rhs.http_server_name);
    this->http_server_port = rhs.http_server_port;
    this->use_http_auth = rhs.use_http_auth;
 
    this->use_socks_proxy = rhs.use_socks_proxy;
    strcpy(this->socks5_user_name, rhs.socks5_user_name);
    strcpy(this->socks5_user_passwd, rhs.socks5_user_passwd);
    strcpy(this->socks_server_name, rhs.socks_server_name);
    this->socks_server_port = rhs.socks_server_port;
    this->socks_version = rhs.socks_version;

    return *this;
}
