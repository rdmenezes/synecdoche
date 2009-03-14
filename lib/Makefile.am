## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = crypt_prog syneccmd

all-local: syneccmd$(EXEEXT)

syneccmd_SOURCES = \
    boinc_cmd.C \
    gui_rpc_client.h

syneccmd_CPPFLAGS = -O3 $(AM_CPPFLAGS)
syneccmd_LDADD = $(lib_LIBRARIES) $(PTHREAD_LIBS)

lib_LIBRARIES = libboinc.a 
EXTRA_DIST = *.h *.C

if OS_DARWIN
mac_sources = \
	mac/dyld_gdb.h \
	mac/mac_backtrace.C \
	mac/mac_backtrace.h \
	mac/QBacktrace.c \
	mac/QBacktrace.h \
	mac/QCrashReport.c \
	mac/QCrashReport.h \
	mac/QMachOImage.c \
	mac/QMachOImage.h \
	mac/QMachOImageList.c \
	mac/QMachOImageList.h \
	mac/QSymbols.c \
	mac/QSymbols.h \
	mac/QTaskMemory.c \
	mac/QTaskMemory.h
else
mac_sources = 
endif

libboinc_a_SOURCES = \
    ticpp/ticpp.cpp \
	ticpp/tinyxml.cpp \
	ticpp/tinyxmlerror.cpp \
	ticpp/tinyxmlparser.cpp \
    app_ipc.C \
    base64.C \
    crypt.C \
    diagnostics.C \
    filesys.C \
    gui_rpc_client.C \
    gui_rpc_client_ops.C \
    gui_rpc_client_print.C \
    hostinfo.C \
    md5.c \
    md5_file.C \
    mem_usage.C \
    mfile.C \
    miofile.C \
    msg_log.C \
    network.C \
    parse.C \
    prefs.C \
    procinfo_unix.C \
    proxy_info.C \
    shmem.C \
    str_util.C \
    util.C \
    unix_util.C \
	$(mac_sources)

pkginclude_HEADERS = \
    ticpp/ticpp.h \
	ticpp/ticpprc.h \
	ticpp/tinyxml.h \
    app_ipc.h \
    boinc_win.h \
	common_defs.h \
    crypt.h \
    diagnostics.h \
    error_numbers.h \
    filesys.h \
    gui_rpc_client.h \
    hostinfo.h \
    mfile.h \
    miofile.h \
    msg_log.h \
    parse.h \
    prefs.h \
    proxy_info.h \
    std_fixes.h \
    str_util.h \
    util.h

crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES =
crypt_prog_CPPFLAGS = -O3 $(AM_CPPFLAGS)
crypt_prog_LDADD = $(RSA_LIBS) $(PTHREAD_LIBS)

