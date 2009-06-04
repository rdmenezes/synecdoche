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

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif
#ifdef _WIN32
#include "win_util.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cctype>
#ifdef HAVE_ALLOCA_H
#include "alloca.h"
#endif
#endif

#include <string>
#include <sstream>
#include <iomanip>
#include <list>
#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"

#if !defined(HAVE_STRLCPY)
/// Use this instead of strncpy().
/// Result will always be null-terminated, and it's faster.
/// See http://www.gratisoft.us/todd/papers/strlcpy.html
///
/// \param[out] dst Pointer to a char array which will be the destination.
/// \param[in] src Pointer to a char array which will be copied to \a dst.
/// \param[in] size Size of the destination buffer pointed to by \a dst.
/// \return The number of characters written to the destination buffer \a
///         without the null terminator.
size_t strlcpy(char* dst, const char* src, size_t size) {
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size-1 : ret;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }

    return ret;
}
#endif

#if !defined(HAVE_STRLCAT)
/// Use this instead of strncat().
/// Result will always be null-terminated.
/// See http://www.gratisoft.us/todd/papers/strlcpy.html
///
/// \param[out] dst Pointer to a char array to which the string pointed to
///                 by \a src will be appended.
/// \param[in] src Pointer to a char array which will be appended to \a dst
/// \param[in] size Size of the destination buffer pointed to by \a dst
/// \return The number of characters in the destination buffer \a
///         without the null terminator.
size_t strlcat(char* dst, const char* src, size_t size) {
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);

    if (size) {
        size_t len = (src_len >= size-dst_len) ? (size-dst_len-1) : src_len;
        memcpy(&dst[dst_len], src, len);
        dst[dst_len + len] = '\0';
    }

    return dst_len + src_len;
}
#endif // !HAVE_STRLCAT

#if !defined(HAVE_STRCASESTR)
/// Search for a substring while ignoring upper-/lowercase.
///
/// \param[in] s1 The string in which will be searched for \a s2
/// \param[in] s2 The substring that should be searched.
/// \return A pointer pointing to the start of the sequence determined
///         by \a s2 in the string \a s1. If \a s2 cannot be found in
///         \a s1 NULL is returned.
extern char* strcasestr(const char* s1, const char* s2) {
  char *needle, *haystack, *p=NULL;
  // Is alloca() really less likely to fail with out of memory error 
  // than strdup?
#if defined(HAVE_STRDUPA)
  haystack=strdupa(s1);
  needle=strdupa(s2);
#elif defined(HAVE_ALLOCA_H) || defined(HAVE_ALLOCA)
  haystack=(char *)alloca(strlen(s1)+1);
  needle=(char *)alloca(strlen(s2)+1);
  if (needle && haystack) {
    strlcpy(haystack,s1,strlen(s1)+1);
    strlcpy(needle,s2,strlen(s2)+1);
  }
#elif defined(HAVE_STRDUP)
  haystack=strdup(s1);
  needle=strdup(s1)
#else
  haystack=(char *)malloc(strlen(s1)+1);
  needle=(char *)malloc(strlen(s2)+1);
  if (needle && haystack) {
    strlcpy(haystack,s1,strlen(s1)+1);
    strlcpy(needle,s2,strlen(s2)+1);
  }
#endif
  if (needle && haystack) {
    // convert both strings to lower case
    do {
      *haystack=tolower(*haystack);
    } while (*(++haystack));
    do {
      *needle=tolower(*needle);
    } while (*(++needle));
    // find the substring
    p=strstr(haystack,needle);
    // correct the pointer to point to the substring within s1
    if (p) {
      // C++ type checking requires const_cast here, although this
      // is dangerous if s1 points to read only storage.  But the C
      // function definitely takes a const char * as the first parameter
      // and returns a char *.  So that's what we'll do.
      p=const_cast<char *>(s1)+(p-haystack);
    }
  } 
#if !defined(HAVE_STRDUPA) && !defined(HAVE_ALLOCA) && !defined(HAVE_ALLOC_H)
  // If we didn't allocate on the stack free our strings
  if (needle) free(needle);
  if (haystack) free(haystack);
#endif
  return p;
}
#endif

/// Convert a double precision time (where the value of 1 represents
/// a day) into a string. smallest_timescale determines the smallest
/// unit of time division used
/// smallest_timescale: 0=seconds, 1=minutes, 2=hours, 3=days, 4=years
///
/// \param[in] x The number of days.
/// \param[in] smallest_timescale determines the smallest
///                               unit of time division used.
/// \param[out] str The buffer which will receive the string representing the
///                 number of days given by \a x.
/// \param[in] len The size of the buffer \a str.
/// \return 0 if no error occured. \ref ERR_NULL if \a x is negative or \a str is NULL.
///           \ref ERR_BUFFER_OVERFLOW if \a str is to small to receive the whole string.
int ndays_to_string (double x, int smallest_timescale, char* str, size_t len) {

    if (x < 0 || str == NULL) return ERR_NULL;

    double years = x / 365.25;
    double days = fmod(x, 365.25);
    double hours = fmod(x*24, 24);
    double minutes = fmod(x*24*60, 60);
    double seconds = fmod(x*24*60*60, 60);
    std::ostringstream buf;
    buf.flags(std::ios_base::dec | std::ios_base::fixed);

    if (smallest_timescale==4) {
        buf << std::setprecision(3) << years << " yr ";
    } else if (years > 1 && smallest_timescale < 4) {
        buf << static_cast<int>(years) << " yr ";
    }

    if (smallest_timescale==3) {
        buf << std::setprecision(2) << days << ((days > 1) ? " days ":" day ");
    } else if (days > 1 && smallest_timescale < 3) {
        buf << static_cast<int>(days) << ((days > 1) ? " days ":" day ");
    }

    if (smallest_timescale==2) {
        buf << std::setprecision(2) << hours << " hr ";
    } else if (hours > 1 && smallest_timescale < 2) {
        buf << static_cast<int>(hours) << " hr ";
    }

    if (smallest_timescale==1) {
        buf << std::setprecision(2) << minutes << " min ";
    } else if (minutes > 1 && smallest_timescale < 1) {
        buf << static_cast<int>(minutes) << " min ";
    }

    if (smallest_timescale==0) {
        buf << std::setprecision(2) << seconds << " sec ";
    } else if (seconds > 1 && smallest_timescale < 0) {
        buf << static_cast<int>(seconds) << " sec ";
    }

    std::string result = buf.str();
    if (result.length() >= len) {
        return ERR_BUFFER_OVERFLOW;
    }

    strlcpy(str, result.c_str(), len);
    return 0;
}

/// Convert \a nbytes into a string. If \a total_bytes is non-zero,
/// convert the two into a fractional display (i.e. 4/16 KB)
///
/// \param[in] nbytes The number that should be converted.
/// \param[in] total_bytes Second number, only used for fractional display.
/// \param[out] str The buffer which will receive the resulting string.
/// \param[in] len The size of the buffer \a str.
/// \return 0 if no error occured. \ref ERR_NULL if \a str is \c NULL.
///           \ref ERR_BUFFER_OVERFLOW if \a str is to small to receive the whole string.
int nbytes_to_string(double nbytes, double total_bytes, char* str, size_t len) {
    if (!str)
        return ERR_NULL;

    std::ostringstream buf;
    buf.flags(std::ios_base::dec | std::ios_base::fixed);
    buf.precision(2);
    const double xTera = (1024.0*1024.0*1024.0*1024.0);
    const double xGiga = (1024.0*1024.0*1024.0);
    const double xMega = (1024.0*1024.0);
    const double xKilo = (1024.0);

    if (total_bytes != 0) {
        if (total_bytes >= xTera) {
            buf << (nbytes / xTera) << "/" << (total_bytes / xTera) << " TB";
        } else if (total_bytes >= xGiga) {
            buf << (nbytes / xGiga) << "/" << (total_bytes / xGiga) << " GB";
        } else if (total_bytes >= xMega) {
            buf << (nbytes / xMega) << "/" << (total_bytes / xMega) << " MB";
        } else if (total_bytes >= xKilo) {
            buf << (nbytes / xKilo) << "/" << (total_bytes / xKilo) << " KB";
        } else {
            buf << nbytes << "/" << total_bytes << " Bytes";
        }
    } else {
        if (nbytes >= xTera) {
            buf << (nbytes / xTera) << " TB";
        } else if (nbytes >= xGiga) {
            buf << (nbytes / xGiga) << " GB";
        } else if (nbytes >= xMega) {
            buf << (nbytes / xMega) << " MB";
        } else if (nbytes >= xKilo) {
            buf << (nbytes / xKilo) << " KB";
        } else {
            buf << nbytes << " Bytes";
        }
    }

    std::string result = buf.str();
    if (result.length() >= len) {
        return ERR_BUFFER_OVERFLOW;
    }

    strlcpy(str, result.c_str(), len);
    return 0;
}

/// This enum will be used by the state machine in \ref parse_command_line.
enum cmd_line_parser_state {
    NOT_IN_TOKEN,
    IN_SINGLE_QUOTED_TOKEN,
    IN_DOUBLE_QUOTED_TOKEN,
    IN_UNQUOTED_TOKEN
};

/// Take a string containing some space separated words.
/// Return an array of pointers to the null-terminated words.
/// \todo use strtok here
///
/// \param[in] p A string with space separated words.
/// \return A list of strings with all space separated words taken from \a p.
std::list<std::string> parse_command_line(const char* p) {
    cmd_line_parser_state state = NOT_IN_TOKEN;
    std::list<std::string> result;
    const char* start = p;

    while (*p) {
        switch(state) {
        case NOT_IN_TOKEN:
            if (!isspace(*p)) {
                if (*p == '\'') {
                    start = ++p;
                    state = IN_SINGLE_QUOTED_TOKEN;
                } else if (*p == '\"') {
                    start = ++p;
                    state = IN_DOUBLE_QUOTED_TOKEN;
                } else {
                    start = p;
                    state = IN_UNQUOTED_TOKEN;
                }
            }
            break;
        case IN_SINGLE_QUOTED_TOKEN:
            if (*p == '\'') {
                result.push_back(std::string(start, p));
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_DOUBLE_QUOTED_TOKEN:
            if (*p == '\"') {
                result.push_back(std::string(start, p));
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_UNQUOTED_TOKEN:
            if (isspace(*p)) {
                result.push_back(std::string(start, p));
                state = NOT_IN_TOKEN;
            }
            break;
        }
        // We need this double check here as p may already be moved to
        // the next position. If we now advance it once more without checking
        // the value p points at we might miss the terminating '\0'.
        if (!p++) {
            break;
        }
    }

    // There may be one argument left:
    if ((state == IN_UNQUOTED_TOKEN) && ((p - start) > 1)) {
        result.push_back(std::string(start, p));
    }
    return result;
}

/// Remove leading and trailing whitespace from a string.
///
/// \param[in,out] str Pointer to the null-terminated C string that should get trimmed.
void strip_whitespace(char* str) {
    std::string buf(str);
    strip_whitespace(buf);

    // This should be safe as strip_whitespace only shortens the string.
    strcpy(str, buf.c_str());
}

/// Remove leading and trailing whitespace from a string.
///
/// \param[in,out] str Reference to the string that should get trimmed.
void strip_whitespace(std::string& str) {
    std::string::size_type pos = str.find_first_not_of(" \f\t\v\r\n");
    str.erase(0, pos);
    pos = str.find_last_not_of(" \f\t\v\r\n");
    str.erase(pos + 1);
}

/// Convert a number in hexadecimal representation into an integer.
/// This is a helper function for unescape_url(std::string&).
///
/// \param[in] what Array of char containing exactly two hexadecimal digits.
/// \return The character of which the ASCII code matches the value described
///         by the hexadecimal number in \a what.
static char x2c(const char* what) {
    register char digit;
    digit = ((what[0] >= 'A') ? (((what[0] & 0xdf) - 'A') + 10) : (what[0] - '0'));
    digit *= 16;
    digit += ((what[1] >= 'A') ? (((what[1] & 0xdf) - 'A') + 10) : (what[1] - '0'));
    return digit;
}

/// Unescape an URL.
///
/// \param[in,out] url Reference to a string containing the URL that should
///                    get unescaped.
void unescape_url(std::string& url) {
    std::string result;
    result.reserve(url.size());
    for (std::string::const_iterator c = url.begin(); c != url.end(); ++c) {
        if (*c == '%') {
            // Only decode if the input is complete!
            if (c + 2 >= url.end()) {
                break;
            }
            char buf[2];
            std::copy(c + 1, c + 3, buf);
            result += x2c(buf);
            c += 3;
        } else {
            result += *c;
        }
    }
    url = result;
}

/// Escape an URL.
///
/// \param[in,out] url Reference to a string containing the URL that should
///                    get escaped.
void escape_url(std::string& url) {
    std::string result;
    result.reserve(url.size());
    for (std::string::const_iterator c = url.begin(); c != url.end(); ++c) {
        if (isalnum(*c)) {
            result += *c;
        } else {
            result += '%';
            char buf[4];
            snprintf(buf, 4, "%X", *c);
            result += std::string(buf, buf + 2);
        }
    }
    url = result;
}

/// Escape a URL for the project directory, cutting off the "http://",
/// converting everthing other than alphanumbers, ., - and _ to "_".
void escape_url_readable(const char *in, char* out) {
    int x, y;
    const char *temp;

    temp = strstr(in,"://");
    if (temp) {
        in = temp + strlen("://");
    }
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x]) || in[x]=='.' || in[x]=='-' || in[x]=='_') {
            out[y] = in[x];
            ++y;
        } else {
            out[y] = '_';
            ++y;
        }
    }
    out[y] = 0;
}

/// Escape a URL for the project directory, cutting off the "http://",
/// converting everything other than letters, numbers, ., - and _ to "_".
///
/// \param[in] in The URL to escape.
/// \return The escaped version of \a in.
std::string escape_url_readable(const std::string& in) {
    std::string::const_iterator it = in.begin();

    // Skip http:// and similar things:
    std::string::size_type pos = in.find("://");
    if (pos != std::string::npos) {
        it += pos + 3;
    }

    std::string result;
    result.reserve(in.end() - it);
    for (; it != in.end(); ++it) {
        if ((isalnum(*it)) || ((*it) == '.') || ((*it) == '-') || ((*it) == '_')) {
            result += *it;
        } else {
            result += '_';
        }
    }
    return result;
}

/// Canonicalize a master URL.
///   - Convert the first part of a URL (before the "://") to http://,
/// or prepend it
///   - Remove double slashes in the rest
///   - Add a trailing slash if necessary
///
/// \param[in,out] url The url that should get canonicalized.
void canonicalize_master_url(std::string& url) {
    std::string buf(url);
    bool bSSL = false; // keep track if they sent in https://

    std::string::size_type pos = buf.find("://");
    if (pos != std::string::npos) {
        if (buf.substr(0, 8) == std::string("https://")) {
            bSSL = true;
        }
        buf.erase(0, pos + 3);
    }
    while ((pos = buf.find("//")) != std::string::npos) {
        buf.erase(pos, 1);
    }
    if (*(buf.end() - 1) != '/') {
        buf += "/";
    }

    url = std::string("http") + (bSSL ? "s://" : "://") + buf;
}

/// Canonicalize a master URL.
///   - Convert the first part of a URL (before the "://") to http://,
/// or prepend it
///   - Remove double slashes in the rest
///   - Add a trailing slash if necessary
///
/// \param[in,out] url The url that should get canonicalized.
/// \deprecated Use canonicalize_master_url(std::string&) instead which
///             is more secure.
void canonicalize_master_url(char* url) {
    std::string buf(url);
    canonicalize_master_url(buf);
    strcpy(url, buf.c_str());
}

// is the string a valid master URL, in canonical form?
//
bool valid_master_url(const char* url) {
    const char *p, *q;
    size_t n;
    bool bSSL = false;

    p = strstr(url, "http://");
    if (p != url) {
        // allow https
        p = strstr(url, "https://");
        if (p == url) {
            bSSL = true;
        } else {
            return false; // no http or https, it's bad!
        }
    }
    q = p+strlen(bSSL ? "https://" : "http://");
    p = strstr(q, ".");
    if (!p) return false;
    if (p == q) return false;
    q = p+1;
    p = strstr(q, "/");
    if (!p) return false;
    if (p == q) return false;
    n = strlen(url);
    if (url[n-1] != '/') return false;
    return true;
}

/// Convert a timestamp into a string.
///
/// \param[in] t A timestamp in seconds.
/// \return A string representing the given timestamp.
std::string time_to_string(double t) {
    char buf[256];
    time_t x = (time_t)t;
    struct tm* tm = localtime(&x);
    if (strftime(buf, sizeof(buf), "%d-%b-%Y %H:%M:%S", tm)) {
        return std::string(buf);
    } else { // Actually this should never happen.
        return std::string("");
    }
}

/// Convert a timestamp with sub-second precision into a string.
///
/// \param[in] t A timestamp in seconds which should get converted.
/// \return A string representing the given timestamp with a
///         precision of 0.0001 seconds.
std::string precision_time_to_string(double t) {
    char buf[256];
    int hundreds_of_microseconds=(int)(10000*(t-(int)t));
    if (hundreds_of_microseconds == 10000) {
        // paranoia -- this should never happen!
        //
        hundreds_of_microseconds=0;
        t+=1.0;
    }
    time_t x = (time_t)t;
    struct tm* tm = localtime(&x);

    if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm)) {
        std::ostringstream finer;
        finer << buf << "." << std::setw(4) << std::setfill('0') << hundreds_of_microseconds;
        return finer.str();
    } else { // Actually this should never happen.
        return std::string("");
    }
}

/// Convert a time difference into a descriptive string.
///
/// \param[in] diff The time difference in seconds.
/// \return A descriptive string representing the given time difference.
std::string timediff_format(double diff) {
    std::ostringstream buf;
    int tdiff = static_cast<int>(diff);

    int sex = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        buf << sex << " sec";
        return buf.str();
    }

    int min = tdiff % 60;
    tdiff /= 60;
    if (!tdiff) {
        buf << min << " min " << sex << " sec";
        return buf.str();
    }

    int hours = tdiff % 24;
    tdiff /= 24;
    if (!tdiff) {
        buf << hours << " hr " << min << " min " << sex << " sec";
        return buf.str();
    }

    int days = tdiff % 7;
    tdiff /= 7;
    if (!tdiff) {
        buf << days << " days " << hours << " hr " << min << " min " << sex << " sec";
        return buf.str();
    }

    buf << tdiff << " weeks " << days << " days " << hours << " hr " << min << " min " << sex << " sec";
    return buf.str();
}

void escape_project_url(const char *in, char* out) {
    escape_url_readable(in, out);
    char& last = out[strlen(out)-1];
    // remove trailing _
    if (last == '_') {
        last = '\0';
    }
}

/// Escape a URL for the project directory
std::string escape_project_url(const std::string& in) {
    std::string result = escape_url_readable(in);

    // Remove trailing '_':
    if (ends_with(result, "_")) {
        result.erase(result.end() - 1);
    }
    return result;
}

/// Convert UNIX time to MySQL timestamp (yyyymmddhhmmss).
///
/// \param[in] dt UNIX timestamp.
/// \return The MySQL timestamp equivalent of the given timestamp as string.
std::string mysql_timestamp(double dt) {
    struct tm* tmp;
    time_t t = (time_t)dt;
    tmp = localtime(&t);     // MySQL timestamps are in local time
    std::ostringstream res;
    res << std::setw(4) << (tmp->tm_year + 1900);
    res.fill('0');
    res.width(2);
    res << (tmp->tm_mon + 1) << tmp->tm_mday << tmp->tm_hour;
    res << tmp->tm_min << tmp->tm_sec;
    return res.str();
}

/// Convert UNIX time to MySQL timestamp (yyyymmddhhmmss).
///
/// \param[in] dt UNIX timestamp.
/// \param[out] p Pointer to a char array that will receive the string
///               with the MySQL timestamp equivalent to the given UNIX
///               timestamp.
/// \param[in] len Size of the buffer \a p.
/// \return 0 if no error occured, \ref ERR_BUFFER_OVERFLOW if the given buffer
///         \a p is too small.
int mysql_timestamp(double dt, char* p, size_t len) {
    std::string buf = mysql_timestamp(dt);
    if (buf.length() >= len) {
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(p, buf.c_str(), len);
    return 0;
}

/// Return a text-string description of a given error.
/// Must be kept consistent with error_numbers.h
///
/// \param[in] which_error The error number for which the string should
///                        be returned.
/// \return A string containing the error message corresponding to the
///         error number in \a which_error.
const char* boincerror(int which_error) {
    switch (which_error) {
        case BOINC_SUCCESS: return "Success";
        case ERR_SELECT: return "select() failed";
        case ERR_MALLOC: return "malloc() failed";
        case ERR_READ: return "read() failed";
        case ERR_WRITE: return "write() failed";
        case ERR_FREAD: return "fread() failed";
        case ERR_FWRITE: return "fwrite() failed";
        case ERR_IO: return "system I/O error";
        case ERR_CONNECT: return "connect() failed";
        case ERR_FOPEN: return "fopen() failed";
        case ERR_RENAME: return "rename() failed";
        case ERR_UNLINK: return "unlink() failed";
        case ERR_OPENDIR: return "opendir() failed";
        case ERR_XML_PARSE: return "unexpected XML tag or syntax";
        case ERR_GETHOSTBYNAME: return "can't resolve hostname";
        case ERR_GIVEUP_DOWNLOAD: return "file download timed out";
        case ERR_GIVEUP_UPLOAD: return "file upload timed out";
        case ERR_NULL: return "unexpected null pointer";
        case ERR_NEG: return "unexpected negative value";
        case ERR_BUFFER_OVERFLOW: return "buffer overflow";
        case ERR_MD5_FAILED: return "md5 checksum failed for file";
        case ERR_RSA_FAILED: return "RSA key check failed for file";
        case ERR_OPEN: return "open() failed";
        case ERR_DUP2: return "dup() failed";
        case ERR_NO_SIGNATURE: return "no signature";
        case ERR_THREAD: return "thread failure";
        case ERR_SIGNAL_CATCH: return "caught signal";
        case ERR_UPLOAD_TRANSIENT: return "transient upload error";
        case ERR_UPLOAD_PERMANENT: return "permanent upload error";
        case ERR_IDLE_PERIOD: return "user preferences say can't start work";
        case ERR_ALREADY_ATTACHED: return "already attached to project";
        case ERR_FILE_TOO_BIG: return "file size too big";
        case ERR_GETRUSAGE: return "getrusage() failed";
        case ERR_BENCHMARK_FAILED: return "benchmark failed";
        case ERR_BAD_HEX_FORMAT: return "hex format key data bad";
        case ERR_DB_NOT_FOUND: return "no database rows found in lookup/enumerate";
        case ERR_DB_NOT_UNIQUE: return "database lookup not unique";
        case ERR_DB_CANT_CONNECT: return "can't connect to database";
        case ERR_GETS: return "gets()/fgets() failedj";
        case ERR_SCANF: return "scanf()/fscanf() failed";
        case ERR_READDIR: return "readdir() failed";
        case ERR_SHMGET: return "shmget() failed";
        case ERR_SHMCTL: return "shmctl() failed";
        case ERR_SHMAT: return "shmat() failed";
        case ERR_FORK: return "fork() failed";
        case ERR_EXEC: return "exec() failed";
        case ERR_NOT_EXITED: return "process didn't exit";
        case ERR_NOT_IMPLEMENTED: return "system call not implemented";
        case ERR_GETHOSTNAME: return "gethostname() failed";
        case ERR_NETOPEN: return "netopen() failed";
        case ERR_SOCKET: return "socket() failed";
        case ERR_FCNTL: return "fcntl() failed";
        case ERR_AUTHENTICATOR: return "authentication error";
        case ERR_SCHED_SHMEM: return "scheduler shared memory contents bad";
        case ERR_ASYNCSELECT: return "async select() failed";
        case ERR_BAD_RESULT_STATE: return "bad result state";
        case ERR_DB_CANT_INIT: return "can't init database";
        case ERR_NOT_UNIQUE: return "state files have redundant entries";
        case ERR_NOT_FOUND: return "not found";
        case ERR_NO_EXIT_STATUS: return "no exit status in scheduler request";
        case ERR_FILE_MISSING: return "file missing";
        case ERR_SEMGET: return "semget() failed";
        case ERR_SEMCTL: return "semctl() failed";
        case ERR_SEMOP: return "semop() failed";
        case ERR_FTOK: return "ftok() failed";
        case ERR_SOCKS_UNKNOWN_FAILURE: return "SOCKS: unknown error";
        case ERR_SOCKS_REQUEST_FAILED: return "SOCKS: request failed";
        case ERR_SOCKS_BAD_USER_PASS: return "SOCKS: bad user password";
        case ERR_SOCKS_UNKNOWN_SERVER_VERSION: return "SOCKS: unknown server version";
        case ERR_SOCKS_UNSUPPORTED: return "SOCKS: unsupported";
        case ERR_SOCKS_CANT_REACH_HOST: return "SOCKS: can't reach host";
        case ERR_SOCKS_CONN_REFUSED: return "SOCKS: connection refused";
        case ERR_TIMER_INIT: return "timer init";
        case ERR_RSC_LIMIT_EXCEEDED: return "resource limit exceeded";
        case ERR_INVALID_PARAM: return "invalid parameter";
        case ERR_SIGNAL_OP: return "signal op";
        case ERR_BIND: return "bind() failed";
        case ERR_LISTEN: return "listen() failed";
        case ERR_TIMEOUT: return "timeout";
        case ERR_PROJECT_DOWN: return "project down";
        case ERR_HTTP_ERROR: return "HTTP error";
        case ERR_RESULT_START: return "result start failed";
        case ERR_RESULT_DOWNLOAD: return "result download failed";
        case ERR_RESULT_UPLOAD: return "result upload failed";
        case ERR_INVALID_URL: return "invalid URL";
        case ERR_MAJOR_VERSION: return "bad major version";
        case ERR_NO_OPTION: return "no option";
        case ERR_MKDIR: return "mkdir() failed";
        case ERR_INVALID_EVENT: return "invalid event";
        case ERR_ALREADY_RUNNING: return "already running";
        case ERR_NO_APP_VERSION: return "no app version";
        case ERR_WU_USER_RULE: return "user already did result for this workunit";
        case ERR_ABORTED_VIA_GUI: return "result aborted via GUI";
        case ERR_INSUFFICIENT_RESOURCE: return "insufficient resources";
        case ERR_RETRY: return "retry";
        case ERR_WRONG_SIZE: return "wrong size";
        case ERR_USER_PERMISSION: return "user permission";
        case ERR_BAD_EMAIL_ADDR: return "bad email address";
        case ERR_BAD_PASSWD: return "bad password";
        case ERR_SHMEM_NAME: return "can't get shared mem segment name";
        case ERR_NO_NETWORK_CONNECTION: return "no available network connection";
        case ERR_IN_PROGRESS: return "operation in progress";
        case ERR_ACCT_CREATION_DISABLED: return "account creation disabled";
        case ERR_ATTACH_FAIL_INIT: return "Couldn't start master page download";
        case ERR_ATTACH_FAIL_DOWNLOAD: return "Couldn't download master page";
        case ERR_ATTACH_FAIL_PARSE: return "Couldn't parse master page";
        case ERR_ATTACH_FAIL_BAD_KEY: return "Invalid account key";
        case ERR_ATTACH_FAIL_FILE_WRITE: return "Couldn't write account file";
        case ERR_FFLUSH: return "fflush() failed";
        case ERR_FSYNC: return "fsync() failed";
        case ERR_TRUNCATE: return "truncate() failed";
        case ERR_ABORTED_BY_PROJECT: return "Aborted by project";
        case ERR_GETGRNAM: return "getgrnam() failed";
        case ERR_CHOWN: return "chown() failed";
        case ERR_FILE_NOT_FOUND: return "file not found";
        case ERR_BAD_FILENAME: return "file name is empty or has '..'";
        case ERR_TOO_MANY_EXITS: return "application exited too many times";
        case ERR_RMDIR: return "rmdir() failed";
        case ERR_SYMLINK: return "symlink() failed";
        case ERR_DB_CONN_LOST: return "DB connection lost during enumeration";
        case ERR_CRYPTO: return "encryption error";
        case ERR_UNSTARTED_LATE: return "job is unstarted and past deadline";
        case 404: return "HTTP file not found";
        case 407: return "HTTP proxy authentication failure";
        case 416: return "HTTP range request error";
        case 500: return "HTTP internal server error";
        case 501: return "HTTP not implemented";
        case 502: return "HTTP bad gateway";
        case 503: return "HTTP service unavailable";
        case 504: return "HTTP gateway timeout";
    }
    static char buf[128];
    sprintf(buf, "Error %d", which_error);
    return buf;
}

/// Return a text-string description of a given network status.
/// Must be kept consistent with common_defs.h
///
/// \param[in] n The network status identifier for which the string should
///              be returned.
/// \return A string containing the network status message corresponding to
///         the network status number in \a n.
const char* network_status_string(int n) {
    switch (n) {
    case NETWORK_STATUS_ONLINE: return "online";
    case NETWORK_STATUS_WANT_CONNECTION: return "need connection";
    case NETWORK_STATUS_WANT_DISCONNECT: return "don't need connection";
    case NETWORK_STATUS_LOOKUP_PENDING: return "reference site lookup pending";
    default: return "unknown";
    }
}

/// Return a text-string description of a given reason for a rpc request.
/// Must be kept consistent with common_defs.h
///
/// \param[in] n The rpc request reason identifier for which the string should
///              be returned.
/// \return A string containing the rpc request reason corresponding to
///         the rpc request reason number in \a reason.
const char* rpc_reason_string(rpc_reason reason) {
    switch (reason) {
    case RPC_REASON_USER_REQ: return "Requested by user";
    case RPC_REASON_NEED_WORK: return "To fetch work";
    case RPC_REASON_RESULTS_DUE: return "To report completed tasks";
    case RPC_REASON_TRICKLE_UP: return "To send trickle-up message";
    case RPC_REASON_ACCT_MGR_REQ: return "Requested by account manager";
    case RPC_REASON_INIT: return "Project initialization";
    case RPC_REASON_PROJECT_REQ: return "Requested by project";
    default: return "Unknown reason";
    }
}

#ifdef WIN32

/// Get a message for the last error.
///
/// \param[out] pszBuf Pointer to a buffer that will receive the error
///                    message. If the buffer is too small it will be cleared.
/// \param[in] iSize Size of the buffer pointed to by \a pszBuf.
/// \return Always returns \a pszBuf.
char* windows_error_string(char* pszBuf, int iSize) {
    LPTSTR lpszTemp = NULL;
    DWORD dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL, GetLastError(), LANG_NEUTRAL, (LPTSTR)&lpszTemp, 0, NULL);

    if ((!dwRet) || ((long)iSize < (long)dwRet + 14)) {
        // Supplied buffer is not long enough.
        pszBuf[0] = TEXT('\0');
    } else {
        lpszTemp[lstrlen(lpszTemp) - 2] = TEXT('\0'); // Remove cr and newline character.
        sprintf(pszBuf, TEXT("%s (0x%x)"), lpszTemp, GetLastError());
    }

    if (lpszTemp) {
        LocalFree((HLOCAL)lpszTemp );
    }
    return pszBuf;
}

/// Get a message for a given error.
///
/// \param[in] dwError The error number for which the string representation
///                    should be returned.
/// \param[out] pszBuf Pointer to a buffer that will receive the error
///                    message. If the buffer is too small it will be cleared.
/// \param[in] iSize Size of the buffer pointed to by \a pszBuf.
/// \return Always returns \a pszBuf.
char* windows_format_error_string(unsigned long dwError, char* pszBuf, int iSize) {
    LPTSTR lpszTemp = NULL;
    DWORD dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                NULL, dwError, LANG_NEUTRAL, (LPTSTR)&lpszTemp, 0, NULL);

    if ((!dwRet) || ((long)iSize < (long)dwRet + 14)) {
        // Supplied buffer is not long enough.
        pszBuf[0] = TEXT('\0');
    } else {
        lpszTemp[lstrlen(lpszTemp) - 2] = TEXT('\0'); // Remove cr and newline character.
        sprintf(pszBuf, TEXT("%s (0x%x)"), lpszTemp, dwError);
    }

    if (lpszTemp) {
        LocalFree((HLOCAL)lpszTemp);
    }
    return pszBuf;
}
#endif
