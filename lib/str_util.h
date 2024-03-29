// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Nicolas Alvarez, Peter Kortschack
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

/// \file
/// Helper functions for string handling and conversion.

#ifndef STR_UTIL_H
#define STR_UTIL_H

#include <algorithm> //for transform
#include <list>
#include <string>

#include <cctype> //for tolower

#ifndef _WIN32
#include "config.h"
#endif

#include "attributes.h"
#include "common_defs.h" //for rpc_reason

#if !defined(HAVE_STRLCPY)
/// Use this instead of strncpy().
size_t strlcpy(char* dst, const char* src, size_t size);
#endif

#if !defined(HAVE_STRLCAT)
/// Use this instead of strncat().
size_t strlcat(char* dst, const char* src, size_t size);
#endif

#if !defined(HAVE_STRCASESTR)
/// Search for a substring while ignoring upper-/lowercase.
const char* strcasestr(const char* s1, const char* s2);
#endif

/// Convert a double precision time into a string.
int ndays_to_string(double x, int smallest_timescale, char* str, size_t len);

/// Convert \a nbytes into a string.
int nbytes_to_string(double nbytes, double total_bytes, char* str, size_t len);

/// Split a string with space separated words into single words.
std::list<std::string> parse_command_line(const char* p);

/// Remove leading and trailing whitespace from a string.
void strip_whitespace(char *str);

/// Remove leading and trailing whitespace from a string.
void strip_whitespace(std::string&);

/// Unescape an URL.
void unescape_url(std::string& url);

/// Escape an URL.
std::string escape_url(const std::string& url);

/// Escape a URL for the project directory
std::string escape_url_readable(const std::string& in);

/// Escape a URL for the project directory
std::string escape_project_url(const std::string& in);

bool valid_master_url(const char* url);

/// Canonicalize a master URL.
void canonicalize_master_url(std::string&);

#define safe_strcpy(x, y) strlcpy(x, y, sizeof(x))

/// Convert a timestamp into a string.
std::string time_to_string(double t);

/// Convert a timestamp with sub-second precision into a string.
std::string precision_time_to_string(double t);

/// Convert a time difference into a descriptive string.
std::string timediff_format(double diff);

/// Check if a string has as specific suffix.
///
/// \param[in] s The string that should be tested.
/// \param[in] suffix The suffix to look for.
/// \return True if \a s ends with \a suffix, false otherwise.
inline bool ends_with(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
        s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/// Check if a string has as specific prefix.
///
/// \param[in] s The string that should be tested.
/// \param[in] prefix The prefix to look for.
/// \return True if \a s starts with \a prefix, false otherwise.
inline bool starts_with(const std::string& s, const std::string& prefix) {
    return s.compare(0, prefix.size(), prefix) == 0;
}

/// Turn all characters in a string into their lower case equivalents.
///
/// \param[in,out] w Reference to the string of which each character should
///                  be replaced by its lowercase equivalent.
inline void downcase_string(std::string& w) {
    std::transform(w.begin(), w.end(), w.begin(), static_cast<int(*)(int)>(tolower));
}

/// Returns short text description of error numbers.
const char* boincerror(int which_error);

/// Return a text-string description of a given network status.
const char* network_status_string(int n);

/// Return a text-string description of a given reason for a rpc request.
const char* rpc_reason_string(rpc_reason reason);

/// Compare two strings in lexicographical order.
bool NoCaseLess(const std::string& a, const std::string& b);

#ifdef _WIN32
#include <windows.h>

/// Get a message for the last error.
char* windows_error_string(char* pszBuf, int iSize);

/// Get a message for a given error.
char* windows_format_error_string(unsigned long dwError, char* pszBuf, int iSize);
#endif

#endif
