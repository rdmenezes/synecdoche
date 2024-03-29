// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#ifndef STDWX_H
#define STDWX_H


#ifdef _WIN32

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER                  // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400           // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT            // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0500     // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS          // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0500   // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE               // Allow use of features specific to IE 5.01 or later.
#define _WIN32_IE 0x0501        // Change this to the appropriate value to target IE 6.0 or later.
#endif

#endif

#ifdef __APPLE__

// Use localtime_r and fmtime_r in wxWidgets and eliminate compiler warnings
#define HAVE_LOCALTIME_R 1
#define HAVE_GMTIME_R 1
#endif

#include <wx/wx.h>

#include <wx/accel.h>           // accelerator support
#include <wx/app.h>
#include <wx/bitmap.h>          // bitmap modification support
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/clipbrd.h>         // clipboard support
#include <wx/cmdline.h>         // command line support
#include <wx/colour.h>
#include <wx/combo.h>
#include <wx/config.h>          // configuration support
#include <wx/control.h>
#include <wx/cshelp.h>
#include <wx/datetime.h>        // date/time support
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <wx/debug.h>           // diagnostics support
#include <wx/dialup.h>
#include <wx/dir.h>
#include <wx/dynlib.h>
#include <wx/event.h>
#include <wx/filename.h>        // filesystem support
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/gifdecod.h>
#include <wx/grid.h>
#include <wx/hyperlink.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/intl.h>            // internationalization support
#include <wx/list.h>
#include <wx/listctrl.h>        // list control support
#include <wx/log.h>             // logging support
#include <wx/mimetype.h>
#include <wx/msgdlg.h>          // messagebox dialog support
#include <wx/notebook.h>        // notebook support
#include <wx/odcombo.h>
#include <wx/panel.h>           // panel support
#include <wx/process.h>
#include <wx/radiobut.h>
#include <wx/regex.h>           // regular expression support
#include <wx/settings.h>        // system settings support
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>        // static line support
#include <wx/statbmp.h>         // static bitmap support
#include <wx/statbox.h>
#include <wx/stattext.h>        // static text support
#include <wx/string.h>
#include <wx/sysopt.h>
#include <wx/taskbar.h>         // taskbar support
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/timer.h>           // timer support
#include <wx/tipwin.h>
#include <wx/tokenzr.h>
#include <wx/toolbar.h>         // toolbars support
#include <wx/tooltip.h>
#include <wx/treectrl.h>
#include <wx/url.h>
#include <wx/utils.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/wfstream.h>
#include <wx/wizard.h>
#include <wx/xml/xml.h>

#ifdef _WIN32

// Visual Studio 2005 has extended the C Run-Time Library by including "secure"
// runtime functions and deprecating the previous function prototypes.  Since
// we need to use the previous prototypes to maintain compatibility with other
// platforms we are going to disable the deprecation warnings if we are compiling
// on Visual Studio 2005
#if _MSC_VER >= 1400
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#endif

// Windows Headers
//
#include <windows.h>
#if !defined(__CYGWIN32__) && !defined(__MINGW32__)
#include <crtdbg.h>
#endif
#include <dbghelp.h>
#include <tlhelp32.h>
#include <share.h>
#include <ole2.h>
#include <oleauto.h>
#include <wininet.h>
#include <shlobj.h>

#if wxUSE_ACCESSIBILITY
    #include <oleacc.h>
#else
    #pragma message("")
    #pragma message("wxUSE_ACCESSIBILITY is not defined in setup.h for the wxWidgets library.")
    #pragma message("***** Accessibility features are disabled *****")
    #pragma message("")
#endif // wxUSE_ACCESSIBILITY

#include <wx/msw/ole/oleutils.h>
#include <wx/msw/winundef.h>

#ifndef OBJID_CLIENT
    #define OBJID_CLIENT 0xFFFFFFFC
#endif

#endif

// Standard Libraries
//
#ifndef __APPLE__
#include <malloc.h>
#endif

// C headers
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <csetjmp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <locale>


// C++ headers
#if defined(_WIN32) && !defined(__CYGWIN32__) && !defined(__MINGW32__)
#include <xdebug>
#endif
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <stdexcept>
#include <vector>


#ifdef _WIN32

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

// On the Win32 platform include file and line number information for each
//   memory allocation/deallocation
#if (defined(__WIN32__) && defined(__VISUALC__) && !defined(__AFX_H__))

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

#define DEBUG_NEW                             new(_NORMAL_BLOCK, __FILE__, __LINE__)

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#define SET_CRT_DEBUG_FIELD(a)                _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define CLEAR_CRT_DEBUG_FIELD(a)              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#else //_DEBUG

#define DEBUG_NEW                             new

#define SET_CRT_DEBUG_FIELD(a)                ((void) 0)
#define CLEAR_CRT_DEBUG_FIELD(a)              ((void) 0)

#endif //_DEBUG

#define new DEBUG_NEW

#endif //__WIN32__ && __VISUALC && !__AFX_H__

#endif // STDWX_H

