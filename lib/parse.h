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

#ifndef PARSE_H
#define PARSE_H

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif // HAVE_STRING_H

#include <math.h>
#include <errno.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
extern "C" {
int finite(double);
}
#endif
#endif

#include <string>

class MIOFILE;

class XML_PARSER {
    MIOFILE* f;
    bool scan_nonws(int& first_char);
    int scan_comment();
    int scan_tag(char* tag_buf, size_t tag_len, char* attr_buf = 0, size_t attr_len = 0);
    bool copy_until_tag(char* buf, int len);
public:
    XML_PARSER(MIOFILE*);
    bool get(char* buf, size_t len, bool& is_tag, char* attr_buf = 0, size_t attr_len = 0);
    bool parse_start(const char* start_tag);

    bool parse_str   (char* parsed_tag, const char* start_tag, char* buf, size_t len);
    bool parse_string(char* parsed_tag, const char* start_tag, std::string& str);
    bool parse_int   (char* parsed_tag, const char* start_tag, int& result);
    bool parse_double(char* parsed_tag, const char* start_tag, double& result);
    bool parse_bool  (char* parsed_tag, const char* start_tag, bool& result);

    int element_contents(const char* end_tag, char* buf, size_t buflen);
    void skip_unexpected(const char* start_tag, bool verbose, const char* where);

    MIOFILE& get_miofile() const;
};

/// \name DEPRECATED XML PARSER
/// Deprecated because it makes assumptions about
/// the format of the XML being parsed

/// @{

/// Check if the given line contains the given tag.
///
/// \param[in] s Line of text to search in.
/// \param[in] tag Tag to search for.
/// \return True if \a tag exists in \a s.
inline bool match_tag(const std::string& s, const std::string& tag) {
    return (s.find(tag) != std::string::npos);
}

/// parse an integer of the form <tag>1234</tag>.
/// return true if it's there.
/// Note: this doesn't check for the end tag.
inline bool parse_int(const char* buf, const char* tag, int& result) {
    const char* p = strstr(buf, tag);
    if (!p) return false;
    int temp_result = strtol(p + strlen(tag), 0, 0);      // This respects hex and octal prefixes.
    if (errno == ERANGE) return false;
    result = temp_result;
    return true;
}

/// parse double of the form <tag>1234</tag>.
/// return true if it's there.
/// Note: this doesn't check for the end tag.
inline bool parse_double(const char* buf, const char* tag, double& result) {
    double temp_result;
    const char* p = strstr(buf, tag);
    if (!p) return false;
    temp_result = atof(p+strlen(tag));
#if defined (HPUX_SOURCE)
    if (_Isfinite(temp_result)) {
#else
    if (finite(temp_result)) {
#endif
        result = temp_result;
        return true;
    }
    return false;
}

bool parse_str(const char* buf, const char* tag, char* dest, size_t destlen);
bool parse_str(const char* buf, const char* tag, std::string& dest);
void parse_attr(const char* buf, const char* attrname, char* out, size_t len);
bool parse_bool(const char* buf, const char* tag, bool& result);

// END DEPRECATED XML PARSER

/// @}

int strcatdup(char*& p, char* buf);
int dup_element_contents(FILE* in, const char* end_tag, char** pp);
int dup_element(FILE* in, const char* end_tag, char** pp);
int copy_element_contents(FILE* in, const char* end_tag, char* p, size_t len);
int copy_element_contents(FILE* in, const char* end_tag, std::string&);
void replace_element_contents(
    char* buf, const char* start, const char* end, const char* replacement
);
bool remove_element(char* buf, const char* start, const char* end);
bool str_replace(char* str, const char* old, const char* neww);
char* sgets(char* buf, size_t len, char* &in);

/// Escape XML.
void xml_escape(const char* in, char* out, int len);

/// Unescape XML.
void xml_unescape(const char* in, char* out, int len);

void extract_venue(const char* in, const char* venue_name, char* out);

/// Skip unrecognized line.
int skip_unrecognized(char* buf, MIOFILE& fin);

#endif
