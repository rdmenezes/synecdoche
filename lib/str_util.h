// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez, Peter Kortschack
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
/// Helper functions for string handling and conversion.

#ifndef STR_UTIL_H
#define STR_UTIL_H

#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cstring>
#include <string>
#include <list>

#define KILO (1024.0)
#define MEGA (1048576.0)
#define GIGA (1024.*1048576.0)

#if !defined(HAVE_STRLCPY)
/// Use this instead of strncpy().
extern size_t strlcpy(char* dst, const char* src, size_t size);
#endif

#if !defined(HAVE_STRLCAT)
/// Use this instead of strncat().
extern size_t strlcat(char* dst, const char* src, size_t size);
#endif

#if !defined(HAVE_STRCASESTR)
/// Search for a substring while ignoring upper-/lowercase.
extern char* strcasestr(const char* s1, const char* s2);
#endif

/// Convert a double precision time into a string.
extern int ndays_to_string(double x, int smallest_timescale, char* str, size_t len);

/// Convert \a nbytes into a string.
extern int nbytes_to_string(double nbytes, double total_bytes, char* str, size_t len);

/// Split a string with space separated words into single words.
extern std::list<std::string> parse_command_line(const char* p);

extern void c2x(char *what);

/// Remove leading and trailing whitespace froma string
extern void strip_whitespace(char *str);

/// Remove leading and trailing whitespace froma string
extern void strip_whitespace(std::string&);

extern void unescape_url(std::string& url);
extern void unescape_url(char *url);
extern void escape_url(std::string& url);
extern void escape_url(const char *in, char*out);
extern void escape_url_readable(const char* in, char* out);
extern void escape_project_url(const char *in, char* out);
extern bool valid_master_url(const char*);

/// Canonicalize a master URL.
extern void canonicalize_master_url(char *url);

/// Canonicalize a master URL.
extern void canonicalize_master_url(std::string&);

#define safe_strcpy(x, y) strlcpy(x, y, sizeof(x))
#define safe_strcat(x, y) if (strlen(x)+strlen(y)<sizeof(x)) strcat(x, y)

/// Convert a timestamp into a string.
extern std::string time_to_string(double t);

/// Convert a timestamp with sub-second precision into a string.
extern std::string precision_time_to_string(double t);

/// Convert a time difference into a descriptive string.
extern std::string timediff_format(double);

inline bool ends_with(std::string const& s, std::string const& suffix) {
    return
        s.size()>=suffix.size() &&
        s.substr(s.size()-suffix.size()) == suffix;
}

inline bool starts_with(std::string const& s, std::string const& prefix) {
    return s.substr(0, prefix.size()) == prefix;
}

inline void downcase_string(std::string& w) {
    for (std::string::iterator p = w.begin(); p != w.end(); ++p) {
        *p = tolower(*p);
    }
}

/// Convert UNIX time to MySQL timestamp (yyyymmddhhmmss).
extern std::string mysql_timestamp(double dt);

/// Convert UNIX time to MySQL timestamp (yyyymmddhhmmss).
extern int mysql_timestamp(double dt, char* p, size_t len);

// returns short text description of error corresponding to
// int errornumber from error_numbers.h
//
extern const char* boincerror(int which_error);
extern const char* network_status_string(int);
extern const char* rpc_reason_string(int);

#ifdef _WIN32
#include <windows.h>

extern char* windows_error_string(char* pszBuf, int iSize);
extern char* windows_format_error_string(
    unsigned long dwError, char* pszBuf, int iSize
);
#endif

#endif
