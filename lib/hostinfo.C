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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
#include <cstring>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include <ostream>
#include <sstream>

#include "hostinfo.h"

#include "util.h"
#include "parse.h"
#include "miofile.h"
#include "md5_file.h"
#include "error_numbers.h"
#include "xml_write.h"

HOST_INFO::HOST_INFO() {
    clear_host_info();
}

void HOST_INFO::clear_host_info() {
    timezone = 0;
    strcpy(domain_name, "");
    strcpy(serialnum, "");
    strcpy(ip_addr, "");
    strcpy(host_cpid, "");

    p_ncpus = 0;
    strcpy(p_vendor, "");
    strcpy(p_model, "");
    strcpy(p_features, "");
    p_fpops = 0.0;
    p_iops = 0.0;
    p_membw = 0.0;
    p_calculated = 0.0;

    m_nbytes = 0.0;
    m_cache = 0.0;
    m_swap = 0.0;

    d_total = 0.0;
    d_free = 0.0;

    strcpy(os_name, "");
    strcpy(os_version, "");
}

int HOST_INFO::parse(MIOFILE& in) {
    char buf[1024];

    clear_host_info();
    while (in.fgets(buf, sizeof(buf))) {
        if (match_tag(buf, "</host_info>")) return 0;
        else if (parse_int(buf, "<timezone>", timezone)) continue;
        else if (parse_str(buf, "<domain_name>", domain_name, sizeof(domain_name))) continue;
        else if (parse_str(buf, "<ip_addr>", ip_addr, sizeof(ip_addr))) continue;
        else if (parse_str(buf, "<host_cpid>", host_cpid, sizeof(host_cpid))) continue;
        else if (parse_int(buf, "<p_ncpus>", p_ncpus)) continue;
        else if (parse_str(buf, "<p_vendor>", p_vendor, sizeof(p_vendor))) continue;
        else if (parse_str(buf, "<p_model>", p_model, sizeof(p_model))) continue;
        else if (parse_str(buf, "<p_features>", p_features, sizeof(p_features))) continue;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) {
            // fix foolishness that could result in negative value here
            //
            if (p_fpops < 0) p_fpops = -p_fpops;
            continue;
        }
        else if (parse_double(buf, "<p_iops>", p_iops)) {
            if (p_iops < 0) p_iops = -p_iops;
            continue;
        }
        else if (parse_double(buf, "<p_membw>", p_membw)) {
            if (p_membw < 0) p_membw = -p_membw;
            continue;
        }
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_double(buf, "<m_nbytes>", m_nbytes)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
        else if (parse_double(buf, "<m_swap>", m_swap)) continue;
        else if (parse_double(buf, "<d_total>", d_total)) continue;
        else if (parse_double(buf, "<d_free>", d_free)) continue;
        else if (parse_str(buf, "<os_name>", os_name, sizeof(os_name))) continue;
        else if (parse_str(buf, "<os_version>", os_version, sizeof(os_version))) continue;
    }
    return ERR_XML_PARSE;
}

/// Write the host information, to the client state XML file
/// or in a scheduler request message.
void HOST_INFO::write(std::ostream& out, bool suppress_net_info) const {
    out << "<host_info>\n";
    out << XmlTag<int>("timezone", timezone);

    if (!suppress_net_info) {
        out << XmlTag<const char*>("domain_name", domain_name);
        out << XmlTag<const char*>("ip_addr",     ip_addr);
    }
    out << XmlTag<const char*>("host_cpid",       host_cpid)
        << XmlTag<int>        ("p_ncpus",         p_ncpus)
        << XmlTag<const char*>("p_vendor",        p_vendor)
        << XmlTag<const char*>("p_model",         p_model)
        << XmlTag<const char*>("p_features",      p_features)
        << XmlTag<double>     ("p_fpops",         p_fpops)
        << XmlTag<double>     ("p_iops",          p_iops)
        << XmlTag<double>     ("p_membw",         p_membw)
        << XmlTag<double>     ("p_calculated",    p_calculated)
        << XmlTag<double>     ("m_nbytes",        m_nbytes)
        << XmlTag<double>     ("m_cache",         m_cache)
        << XmlTag<double>     ("m_swap",          m_swap)
        << XmlTag<double>     ("d_total",         d_total)
        << XmlTag<double>     ("d_free",          d_free)
        << XmlTag<const char*>("os_name",         os_name)
        << XmlTag<const char*>("os_version",      os_version)
        << "</host_info>\n"
    ;
}

/// Parse CPU benchmarks state file.
///
/// CPU benchmarks are run in a separate process,
/// which communicates its result via a file.
/// The following functions read and write this file.
int HOST_INFO::parse_cpu_benchmarks(FILE* in) {
    char buf[256];

    char* p = fgets(buf, 256, in);
    if (!p) return 0;           // Fixes compiler warning
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<cpu_benchmarks>"));
        else if (match_tag(buf, "</cpu_benchmarks>")) return 0;
        else if (parse_double(buf, "<p_fpops>", p_fpops)) continue;
        else if (parse_double(buf, "<p_iops>", p_iops)) continue;
        else if (parse_double(buf, "<p_membw>", p_membw)) continue;
        else if (parse_double(buf, "<p_calculated>", p_calculated)) continue;
        else if (parse_double(buf, "<m_cache>", m_cache)) continue;
    }
    return 0;
}

void HOST_INFO::write_cpu_benchmarks(FILE* out) {
    fprintf(out,
        "<cpu_benchmarks>\n"
        "    <p_fpops>%f</p_fpops>\n"
        "    <p_iops>%f</p_iops>\n"
        "    <p_membw>%f</p_membw>\n"
        "    <p_calculated>%f</p_calculated>\n"
        "    <m_cache>%f</m_cache>\n"
        "</cpu_benchmarks>\n",
        p_fpops,
        p_iops,
        p_membw,
        p_calculated,
        m_cache
    );
}

/// Make a random string using host info.
/// Not recommended for password generation;
/// use as a last resort if more secure methods fail
///
/// \param[in] salt Some salt that will be included into the random string.
/// \param[out] out Buffer that will receive the generated random string.
void HOST_INFO::make_random_string(const char* salt, char* out) {
    std::ostringstream buf;

    buf << dtime() << domain_name << ip_addr << d_free << salt;
    std::string buf_s = buf.str();
    md5_block((const unsigned char*)buf_s.c_str(), (int)buf_s.length(), out);
}

/// Make a host cross-project ID.
/// Should be unique across hosts with very high probability
void HOST_INFO::generate_host_cpid() {
    make_random_string("", host_cpid);
}
