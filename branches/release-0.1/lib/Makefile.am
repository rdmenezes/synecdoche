## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

include $(top_srcdir)/Makefile.incl

bin_PROGRAMS = crypt_prog syneccmd

all-local: syneccmd$(EXEEXT)

syneccmd_SOURCES = \
    boinc_cmd.C \
    gui_rpc_client.h

syneccmd_CPPFLAGS = -O3 $(AM_CPPFLAGS)
syneccmd_LDADD = libboinc.a $(PTHREAD_LIBS)

noinst_LIBRARIES = libboinc.a 

libboinc_a_SOURCES = \
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
    app_ipc.h \
    attributes.h \
    base64.h \
    boinc_win.h \
    common_defs.h \
    crypt.h \
    diagnostics.h \
    error_numbers.h \
    filesys.h \
    gui_rpc_client.h \
    hostinfo.h \
    md5.h \
    md5_file.h \
    mem_usage.h \
    mfile.h \
    miofile.h \
    msg_log.h \
    network.h \
    parse.h \
    prefs.h \
    procinfo.h \
    proxy_info.h \
    shmem.h \
    std_fixes.h \
    str_util.h \
    unix_util.h \
    util.h

crypt_prog_SOURCES = crypt_prog.C crypt.C md5.c md5_file.C
crypt_prog_DEPENDENCIES =
crypt_prog_CPPFLAGS = -O3 $(AM_CPPFLAGS)
crypt_prog_LDADD = $(RSA_LIBS) $(PTHREAD_LIBS)

