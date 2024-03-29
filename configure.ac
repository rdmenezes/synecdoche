dnl -*- autoconf -*-

dnl not sure exactly what the minimum version is (but 2.13 wont work)
AC_PREREQ(2.57)

dnl Set the Synecdoche version here.
AC_INIT(Synecdoche, 0.2.0)

LIBBOINC_VERSION=`echo ${PACKAGE_VERSION} | sed 's/\./:/g'`
AC_SUBST([LIBBOINC_VERSION])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([autotools-aux])

AC_ARG_ENABLE(debug, 
    AS_HELP_STRING([--enable-debug],
                   [enable tracing and debugging flags for all components]),
    [enable_debug="$enableval"],
    [])

AC_ARG_ENABLE(unicode, 
    AS_HELP_STRING([--enable-unicode],
                   [enable building the manager with unicode support]),
    [enable_unicode="$enableval"],
    [])

if test "${enable_debug}" = yes ; then
    build_state='Debug'
else
    build_state='Release'
fi

echo "--- Configuring AC_PACKAGE_NAME AC_PACKAGE_VERSION (${build_state}) ---"

AC_CANONICAL_TARGET

dnl generate .tar.gz, .tar.bz2, .zip
dnl AM_INIT_AUTOMAKE(dist-bzip2 dist-zip)
AM_INIT_AUTOMAKE(dist-zip)

AC_CONFIG_SRCDIR(lib/shmem.C)

AC_COPYRIGHT([
Synecdoche
http://synecdoche.googlecode.com/
Copyright (C) 2008 David Barnard
Copyright (C) 2008 Nicolas Alvarez
Copyright (C) 2008 University of California

Synecdoche is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Synecdoche is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.
])

AC_REVISION([$Revision$])

AC_SUBST([TOP_BUILD_DIR], [`pwd`])
AC_SUBST([TOP_SOURCE_DIR], [`(cd \`dirname "$0"\` && pwd)`])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O

dnl Check for docbook2x-man, which we use to create the manpages in
dnl doc/manpages, and set the conditional HAVE_DOCBOOK2X_MAN to build
dnl the manpages only if docbook2x-man is available.
AC_PATH_PROG(DOCBOOK2X_MAN, docbook2x-man)
AM_CONDITIONAL(HAVE_DOCBOOK2X_MAN, [test -n "${DOCBOOK2X_MAN}"])

dnl If we're running under cygwin, check whether we want to use X11 or WIN32
dnl GUI
BOINC_CHECK_CYGWIN

dnl enable objective-C 
_AM_DEPENDENCIES([OBJC])
OBJC="$CC"
AC_SUBST(OBJC)
OBJCFLAGS="-ObjC"
AC_SUBST(OBJCFLAGS)

AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_SUBST(PICFLAGS,${lt_prog_compiler_pic})


SAH_OPTION_BITNESS

dnl Determine the BOINC platform given the target arch-platform-os.
BOINC_PLATFORM

dnl Find the extensions used on shared and static libraries on this system
SAH_LIBEXT
SAH_DLLEXT

dnl enable debug mode on all components using standard debug flags
if test "${enable_debug}" = yes; then
    CXXFLAGS="$CXXFLAGS -DDEBUG -D_DEBUG"
    CFLAGS="$CFLAGS -DDEBUG -D_DEBUG"

    AC_SUBST(CXXFLAGS)
    AC_SUBST(CFLAGS)
fi

dnl Checks for libraries.

dnl m4/boinc_getsockopt.m4
BOINC_GETSOCKOPT_TYPE

dnl ---------- libcurl (m4/libcurl.m4) ------------------------------

LIBCURL_CHECK_CONFIG([yes], [7.17.1], [haveCurl=yes], [haveCurl=no])

if test "${haveCurl}" != yes; then
    AC_MSG_ERROR([
================================================================================
ERROR: could not find (recent enough) development-libs for libcurl.

  This library is required to build the BOINC client.

  If libcurl-dev is installed on your system, make sure that the script 
  'curl-config' is found in your PATH, and that 
  'curl-config --version' gives something recent enough (see above).

  You can download libcurl from: http://curl.haxx.se/

================================================================================
    ])
else
    CPPFLAGS="${CPPFLAGS} ${LIBCURL_CPPFLAGS}"
    CLIENTLIBS="${CLIENTLIBS} ${LIBCURL}"
fi


dnl ---------- SSL (m4/check_ssl.m4)
CHECK_SSL
if test "${found_ssl}" = "yes"; then
   CLIENTLIBS="${CLIENTLIBS} ${SSL_LIBS}"
fi

AC_CHECK_LIB([cygipc], [shmget])
#AC_CHECK_LIB([aio], [aio_fork])
#AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([nsl], [gethostbyname])
AC_CHECK_LIB([socket], [bind])
AC_CHECK_LIB([m],[sin])
AC_CHECK_LIB([pthread],[pthread_join])
AC_CHECK_LIB([wsock32],[fopen])


dnl check for pthread
ACX_PTHREAD(AC_DEFINE(HAVE_PTHREAD,1, [Have pthread]))
CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
LIBS="$PTHREAD_LIBS $LIBS"

AC_CHECK_HEADERS([zlib.h], [], [AC_MSG_ERROR([
ERROR: could not find development libs for zlib.
This library is required to build the Synecdoche client.
])])
AC_CHECK_LIB([z], [gzopen], [CLIENTLIBS="${CLIENTLIBS} -lz"], [AC_MSG_ERROR([
ERROR: could not find development libs for zlib.
This library is required to build the Synecdoche client.
])])

AC_SUBST(CLIENTLIBS)

# check for unittest++
PKG_CHECK_MODULES([UNITTEST], [unittest++])

dnl --------------------------------------------------------------------------------
dnl put double-inclusion protection into config.h
AH_TOP([
/* double-inclusion protection for config.h */
#ifndef BOINC_CONFIG_H
#define BOINC_CONFIG_H
])
AH_BOTTOM([

/* end double-inclusion protection for config.h */
#endif /* #ifndef BOINC_CONFIG_H */
])
dnl ----------------------------------------------------------------------

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_TYPE_SIGNAL
AC_CHECK_HEADERS(windows.h arpa/inet.h dirent.h fcntl.h inttypes.h stdint.h malloc.h alloca.h memory.h netdb.h netinet/in.h netinet/tcp.h signal.h strings.h sys/auxv.h sys/file.h sys/ipc.h sys/mount.h sys/param.h sys/resource.h sys/select.h sys/shm.h sys/socket.h sys/stat.h sys/statvfs.h sys/statfs.h sys/swap.h sys/sysctl.h sys/systeminfo.h sys/time.h sys/types.h sys/utsname.h sys/vmmeter.h sys/wait.h unistd.h utmp.h errno.h procfs.h ieeefp.h)

dnl Unfortunately on some 32 bit systems there is a problem with wx-widgets
dnl configuring itself for largefile support.  On these systems largefile
dnl support breaks C++ compiles by defining away many of the C standard library
dnl routines that should reside in namespace std::.  In order to get around
dnl problem first we have to check the largefile support macros.  Later we will
dnl use the macro SAH_LARGEFILE_BREAKS_CXX to check for the breakage.  If
dnl if breakage is found LARGEFILE_BREAKS_CXX is defined in config.h.  This
dnl define is checked in std_fixes.h and the appropriate functions are defined
dnl in order to solve the problem.
AC_SYS_LARGEFILE

AC_LANG_PUSH(C++)
SAH_LARGEFILE_BREAKS_CXX
SAH_HEADER_STDCXX
SAH_CHECK_NAMESPACES
AH_TEMPLATE([HAVE_STD_MIN],[Define to 1 if min is in namespace std::])
AH_TEMPLATE([HAVE_STD_MAX],[Define to 1 if max is in namespace std::])
AH_TEMPLATE([HAVE_STD_LOCALE],[Define to 1 if locale is in namespace std::])
AH_TEMPLATE([HAVE_STD_TRANSFORM],[Define to 1 if transform is in namespace std:: ])
SAH_FUNCS_IN_NAMESPACE([['min(0,0)'] ['max(0,0)'] ['transform((char *)0,(char *) 0,(char *)0,(int(*)(int))malloc)'] ['locale("")']],std)
AC_LANG_POP

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(alloca _alloca setpriority strlcpy strlcat strcasestr sigaction getutent setutent getisax strdup strdupa daemon stat64 putenv setenv)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TIMEZONE

dnl utmp file location
for f in /etc/utmp /var/adm/utmp /var/run/utmp; do
    if test -f $f; then
        utmp_file=$f
    fi
done
if test -n "$utmp_file"; then
    AC_DEFINE_UNQUOTED(UTMP_LOCATION, "$utmp_file", utmp file location)
fi

dnl Check for /proc/self/psinfo (Solaris)
if test -e "/proc/self/psinfo"; then
    AC_DEFINE(HAVE__PROC_SELF_PSINFO, 1, [Define to 1 if /proc/self/psinfo exists])
fi

dnl Check for /proc/self/stat (Linux)
if test -e "/proc/self/stat"; then
    AC_DEFINE(HAVE__PROC_SELF_STAT, 1, [Define to 1 if /proc/self/stat exists])
fi

dnl Check for /proc/meminfo (Linux)
if test -e "/proc/meminfo"; then
    AC_DEFINE(HAVE__PROC_MEMINFO, 1, [Define to 1 if /proc/meminfo exists])
fi

dnl ---------- wxWidgets --------------------------------------------------

AM_OPTIONS_WXCONFIG

dnl check for wxWidgets

if test "${enable_debug}" = yes ; then
    if test "${enable_unicode}" = yes ; then
        echo 'checking for debug unicode wx-libs ...'
        AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=yes --debug=yes)
    else
        echo 'checking for debug wx-libs ...'
        AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=no --debug=yes)
    fi
else
    if test "${enable_unicode}" = yes ; then
        echo 'checking for unicode wx-libs ...'
        AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=yes --debug=no)
    else
        echo 'checking for wx-libs ...'
        AM_PATH_WXCONFIG([2.6.0], [wxWin=1], [wxWin=0], --unicode=no --debug=no)
    fi
fi


if ( test "$wxWin" != 1 ); then
   AC_MSG_WARN([
================================================================================
WARNING: A suitable installation of wxWidgets could not be found 
     ==> building client without clientgui.

  If you add wxWidgets to your system, then this configure script will also
  configure your system to build the BOINC graphical client (clientgui).  

  If wxWidgets is installed on your system, please check that wx-config is 
  in the path, that the directory where wxWidgets libraries are installed 
  (returned by 'wx-config --libs' command) is in LD_LIBRARY_PATH (or equivalent),
  and that wxWidgets version is 2.6.0 or above. Currently wxWidgets version
  2.6.0 (gtk based) is known to work with boinc_gui under Linux. You can
  use wx-config --version to find what version you have currently installed.

  You can get wxWidgets by following the DOWNLOAD link at:
  http://www.wxwindows.org/
================================================================================
       ])
fi

AM_CONDITIONAL(BUILD_CLIENTGUI, [ test "$wxWin" = 1 ])

dnl --------precompiled header support----------------------------------------------

GCC_PCH=0
PCH_FLAGS=
pch_default="yes"
AM_CONDITIONAL([PRECOMPILE_HEADERS], [false])
AC_ARG_ENABLE(pch, [AC_HELP_STRING([--enable-pch], [use precompiled headers if available (default YES)])],,
						enable_pch=$pch_default)
if test "x$enable_pch" = "x" -o "x$enable_pch" = "xyes" ; then
	if test "x$GCC" = "xyes"; then
		dnl test if we have gcc-3.4:
		AC_MSG_CHECKING([if the compiler supports precompiled headers])
		AC_TRY_COMPILE([],
			[
				#if !defined(__GNUC__) || !defined(__GNUC_MINOR__)
					#error "no pch support"
				#endif
				#if (__GNUC__ < 3)
					#error "no pch support"
				#endif
				#if (__GNUC__ == 3) && \
					((!defined(__APPLE_CC__) && (__GNUC_MINOR__ < 4)) || \
					( defined(__APPLE_CC__) && (__GNUC_MINOR__ < 3)))
					#error "no pch support"
				#endif
			],
			[
				AC_MSG_RESULT([yes])
				GCC_PCH=1
				PCH_FLAGS="-DCB_PRECOMP -Winvalid-pch"
			],
			[
				AC_MSG_RESULT([no])
			])
		AM_CONDITIONAL([PRECOMPILE_HEADERS], [test $GCC_PCH = 1])
	fi
fi

dnl --------------------------------------------------------------------------------

dnl Heres another way to set up host-specific stuff
AM_CONDITIONAL(OS_DARWIN, [echo $host_os | grep '^darwin' > /dev/null])
if echo $host_os | grep '^darwin' >/dev/null ; then
  if test `uname -r | sed 's/\.//g'` -lt 800 ; then
    AC_DEFINE_UNQUOTED(DARWIN_10_3, [1],[Define to 1 if compiling under OS X 10.3 or earlier])
  fi
fi

AM_CONDITIONAL(OS_LINUX, [echo $host_os | grep '^linux' > /dev/null])
dnl In case anyone wants to try building the windows code using mingw!
AM_CONDITIONAL(OS_WIN32_MINGW, [echo $host_os | grep '^mingw' > /dev/null])


if test "${enable_debug}" = yes; then
    CLIENTGUIFLAGS="${CLIENTGUIFLAGS} -D_DEBUG -DDEBUG"
fi

CLIENTGUILIBS=${WX_LIBS}

AC_SUBST(CLIENTGUIFLAGS)
AC_SUBST(CLIENTGUILIBS)
dnl ======================================================================

AC_CONFIG_FILES([
                 client/Makefile
                 locale/client/Makefile
                 lib/Makefile
                 lib/tests/Makefile
                 clientgui/Makefile
                 clientgui/res/Makefile
                 clientgui/tests/Makefile
                 tests/Makefile
                 Makefile
                 ])

dnl AC_CONFIG_HEADER([config.h])
AC_CONFIG_HEADERS([config.h])

AC_OUTPUT

echo "--- Configuring BOINC AC_PACKAGE_VERSION (${build_state}) ---"
