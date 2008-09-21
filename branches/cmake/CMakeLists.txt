# This file is part of Synecdoche.
# http://synecdoche.googlecode.com/
# Copyright (C) 2008 Nicolas Alvarez
#
# Synecdoche is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Synecdoche is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.4)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFile)
INCLUDE (CheckTypeSize)
INCLUDE (CheckCSourceCompiles)

MACRO(AC_VARNAME ITEM VAR)
	SET(l_varname ${ITEM})
	STRING(TOUPPER ${l_varname} l_varname)
	STRING(REGEX REPLACE "[^A-Z]" "_" l_varname ${l_varname})
	SET(${VAR} HAVE_${l_varname})
ENDMACRO(AC_VARNAME)

MACRO(AC_CHECK_INCLUDE_FILE INCLUDE)
	AC_VARNAME(${INCLUDE} l_include_varname)
	CHECK_INCLUDE_FILE(${INCLUDE} ${l_include_varname})
ENDMACRO(AC_CHECK_INCLUDE_FILE)

MACRO(AC_CHECK_FUNCTION_EXISTS FUNC)
	AC_VARNAME(${FUNC} l_func_varname)
	CHECK_FUNCTION_EXISTS(${FUNC} ${l_func_varname})
ENDMACRO(AC_CHECK_FUNCTION_EXISTS)

MACRO(AC_CHECK_TYPE_SIZE TYPE)
	AC_VARNAME(${TYPE} l_type_varname)
	CHECK_TYPE_SIZE(${TYPE} ${l_type_varname})
ENDMACRO(AC_CHECK_TYPE_SIZE)

# Taken almost straight from autoconf
MACRO(AC_STRUCT_TIMEZONE)
	MESSAGE(STATUS "Checking whether struct tm is in sys/time.h or time.h")
	SET(struct_tm_check "UNKNOWN")
	CHECK_C_SOURCE_COMPILES("
#include <sys/types.h>
#include <time.h>

int main() {
	struct tm tm;
	int *p = &tm.tm_sec;
	return !p;
}
" struct_tm_check)
	IF(struct_tm_check)
		SET(struct_tm_inc "time.h")
	ELSE(struct_tm_check)
		SET(struct_tm_inc "sys/time.h")
	ENDIF(struct_tm_check)

	MESSAGE(STATUS "Checking for struct tm.tm_zone")
	CHECK_C_SOURCE_COMPILES("
#include <${struct_tm_inc}>

int main() {
	static struct tm obj;
	if (obj.tm_zone) {
		return 0;
	}
	return 0;
}
" struct_tm_check)
	IF(struct_tm_check)
		SET(HAVE_STRUCT_TM_TM_ZONE 1 CACHE INTERNAL "1 if 'tm_zone' is member of 'struct tm'")
	ENDIF(struct_tm_check)
ENDMACRO(AC_STRUCT_TIMEZONE)


FOREACH(inc "csignal" "signal.h" "malloc.h" "unistd.h" "netdb.h" "arpa/inet.h" "netinet/in.h")
	AC_CHECK_INCLUDE_FILE(${inc})
ENDFOREACH(inc)
FOREACH(inc "types" "ipc" "socket" "resource" "param" "mount" "statvfs" "statfs" "signal" "wait" "systeminfo" "sysctl" "utsname")
	AC_CHECK_INCLUDE_FILE(sys/${inc}.h)
ENDFOREACH(inc)

AC_CHECK_FUNCTION_EXISTS(strcasestr)

IF(HAVE_SYS_SOCKET_H)
	SET(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
ENDIF(HAVE_SYS_SOCKET_H)
AC_CHECK_TYPE_SIZE("socklen_t")
SET(CMAKE_EXTRA_INCLUDE_FILES)

FOREACH(type "socklen_t" "size_t" "int")
	IF (NOT DEFINED BOINC_SOCKLEN_T)
		SET(getsockopt_type_test "UNKNOWN")
		MESSAGE(STATUS "Testing ${type}")
		CHECK_C_SOURCE_COMPILES("
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
		int main() {
			${type} l;
			return getsockopt(0,0,0,(void *)0,&l);
		}" getsockopt_type_test)
		IF(getsockopt_type_test)
			SET(BOINC_SOCKLEN_T "${type}" CACHE INTERNAL "type pointed to by the 5th parameter of getsockopt")
		ENDIF(getsockopt_type_test)
	ENDIF (NOT DEFINED BOINC_SOCKLEN_T)
ENDFOREACH(type)

AC_STRUCT_TIMEZONE()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/client)

FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})

FIND_PACKAGE(CURL REQUIRED)
INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmakein ${CMAKE_BINARY_DIR}/config.h)

SUBDIRS(lib api client clientgui)
