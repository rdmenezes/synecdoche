// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2010 University of California
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

// boinc_win.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#ifndef _BOINC_WIN_
#define _BOINC_WIN_

// Under CYGWIN we need to include config.h first.
#ifdef __CYGWIN32__
#include "config.h"
#endif

// Windows System Libraries
//

// Visual Studio 2005 has extended the C Run-Time Library by including "secure"
// runtime functions and deprecating the previous function prototypes.  Since
// we need to use the previous prototypes to maintain compatibility with other
// platforms we are going to disable the deprecation warnings if we are compiling
// on Visual Studio 2005
#if _MSC_VER >= 1400
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
// And this one is for the C++ library "secure" functions.
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Allow use of features specific to Windows 2000 or later.
#define WINVER 0x0500
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows 2000 or later.
#define _WIN32_WINNT 0x0500
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 5.1 or later.
#define _WIN32_IE 0x0501
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // This trims down the windows libraries.
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN        // Trims even farther.
#endif

#include <windows.h>
#include <share.h>
#include <shlobj.h>
#include <userenv.h>
#include <aclapi.h>

#if !defined(__CYGWIN32__) || defined(USE_WINSOCK)

/* If we're not running under CYGWIN use windows networking */
#undef USE_WINSOCK
#define USE_WINSOCK 1
#include <winsock.h>
#include <wininet.h>

typedef size_t socklen_t;

#else

/* Under cygwin, curl was probably compiled to use <sys/socket.h> */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#define _WINSOCK_H
#define _WINSOCKAPI_
#define _WINSOCK2_H
#define _WININET_H
#define _WININETAPI_

#endif

#include <process.h>
#if defined(__MINGW32__) || defined(__CYGWIN32__)
#include <pbt.h>
#endif

#include <commctrl.h>
#include <raserror.h>
#if defined(__MINGW32__)
#include <stdint.h>
#include <imagehlp.h>
#else
#include <dbghelp.h>
#endif
#include <tlhelp32.h>

#include <io.h>
#if !defined(__CYGWIN32__)
#include <direct.h>
#endif

#if !defined(__CYGWIN32__)
#include <tchar.h>
#else
#ifndef _TCHAR_DEFINED
typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE , *PTBYTE ;
#define _TCHAR_DEFINED
#endif /* !_TCHAR_DEFINED */
typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
#define __TEXT(quote) quote
#endif

// All projects should be using std::min and std::max instead of the Windows
//   version of the symbols.
#undef min
#undef max

// Standard Libraries
//

// C headers
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <malloc.h>

#if !defined(__MINGW32__) && !defined(__CYGWIN32__)
#include <crtdbg.h>
#include <delayimp.h>
#endif

#ifdef __cplusplus
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cfloat>
#include <locale>
#else
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <locale.h>
#endif

// C++ headers
//
#ifdef __cplusplus
#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#endif


#ifndef SIGRTMAX
#if defined(_SIGRTMAX)
#define SIGRTMAX _SIGRTMAX
#elif defined(NSIG)
#define SIGRTMAX (NSIG-1)
#else
#define SIGRTMAX 32
#endif
#endif


#ifndef __CYGWIN__

#define vsnprintf               _vsnprintf
#define snprintf                _snprintf
#define stprintf                _stprintf
#define stricmp                 _stricmp
#define strdup                  _strdup
#define fdopen                  _fdopen
#define dup                     _dup
#define unlink                  _unlink
#define read                    _read
#define stat                    _stat
#define chdir                   _chdir
#define finite                  _finite
#define strdate                 _strdate
#define strtime                 _strtime
#define getcwd                  _getcwd

#endif

#ifdef __MINGW32__
#ifdef __cplusplus
extern "C" {
#endif
void __cdecl _fpreset (void);
void __cdecl fpreset (void);
#define SetClassLongPtr SetClassLong
#define GCLP_HICON GCL_HICON
#define GCLP_HICONSM GCL_HICONSM
#ifdef __cplusplus
}
#endif
#endif

// On the Win32 platform include file and line number information for each
//   memory allocation/deallocation
#ifdef _DEBUG

#define malloc(s)                             _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define calloc(c, s)                          _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define realloc(p, s)                         _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _expand(p, s)                         _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define free(p)                               _free_dbg(p, _NORMAL_BLOCK)
#define _msize(p)                             _msize_dbg(p, _NORMAL_BLOCK)
#define _aligned_malloc(s, a)                 _aligned_malloc_dbg(s, a, __FILE__, __LINE__)
#define _aligned_realloc(p, s, a)             _aligned_realloc_dbg(p, s, a, __FILE__, __LINE__)
#define _aligned_offset_malloc(s, a, o)       _aligned_offset_malloc_dbg(s, a, o, __FILE__, __LINE__)
#define _aligned_offset_realloc(p, s, a, o)   _aligned_offset_realloc_dbg(p, s, a, o, __FILE__, __LINE__)
#define _aligned_free(p)                      _aligned_free_dbg(p)

#ifndef DEBUG_NEW
#define DEBUG_NEW                             new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#define SET_CRT_DEBUG_FIELD(a)                _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define CLEAR_CRT_DEBUG_FIELD(a)              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#else //_DEBUG

#ifndef DEBUG_NEW
#define DEBUG_NEW                             new
#endif

#define SET_CRT_DEBUG_FIELD(a)                ((void) 0)
#define CLEAR_CRT_DEBUG_FIELD(a)              ((void) 0)

#endif //_DEBUG

#define new DEBUG_NEW

#endif //_BOINC_WIN_
