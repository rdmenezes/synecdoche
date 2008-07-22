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

AC_CHECK_INCLUDE_FILE(malloc.h)

FOREACH(inc "types" "socket" "resource" "param" "mount" "statvfs" "statfs")
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
		MESSAGE("Testing ${type}")
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
			SET(BOINC_SOCKLEN_T "${type}" CACHE STRING "type pointed to by the 5th parameter of getsockopt")
		ENDIF(getsockopt_type_test)
	ENDIF (NOT DEFINED BOINC_SOCKLEN_T)
ENDFOREACH(type)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmakein ${CMAKE_BINARY_DIR}/config.h)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
SUBDIRS(lib api)
