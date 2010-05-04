# This file is part of Synecdoche.
# http://synecdoche.googlecode.com/
# Copyright (C) 2009 Nicolas Alvarez
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

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

cmake_policy(SET CMP0003 NEW)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE(AutoconfCompat)

FOREACH(inc "csignal" "signal.h" "malloc.h" "string.h" "unistd.h" "netdb.h" "arpa/inet.h" "netinet/in.h")
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

IF(WIN32)
    STRING(REPLACE "/" "\\" VERSION_CPP_PATH ${CMAKE_BINARY_DIR}/svnversion.cpp)
    ADD_CUSTOM_TARGET(update_svnversion ALL
        ${CMAKE_SOURCE_DIR}/version.bat ${VERSION_CPP_PATH}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
    )
ELSE(WIN32)
    ADD_CUSTOM_TARGET(update_svnversion ALL
        ${CMAKE_SOURCE_DIR}/version.sh ${CMAKE_BINARY_DIR}/svnversion.cpp
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
    )
ENDIF(WIN32)

FILE(WRITE ${CMAKE_BINARY_DIR}/svnversion.cpp "//dummy")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/client)

FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})

FIND_PACKAGE(CURL REQUIRED)
INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmakein ${CMAKE_BINARY_DIR}/config.h)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(clientgui)
IF(WIN32)
    ADD_SUBDIRECTORY(clientlib)
ENDIF(WIN32)

ADD_SUBDIRECTORY(tests)
