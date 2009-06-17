## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

AUTOMAKE_OPTIONS = foreign

## make sure rebuilding uses the right aclocal-switches
ACLOCAL_AMFLAGS = -I m4

API_SUBDIRS = lib

CLIENT_SUBDIRS = client # client/scripts

if BUILD_CLIENTGUI
   CLIENTGUI_SUBDIRS = clientgui clientgui/res locale/client 
endif

# ORDER MATTERS below.  One must build dependencies FIRST, then things
# that depend on them later
SUBDIRS = m4 $(API_SUBDIRS) $(CLIENT_SUBDIRS) $(CLIENTGUI_SUBDIRS)

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of SVN directories.

EXTRA_DIST =   \
    win_build  \
    INSTALL    \
    version.sh

dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~' -o -name .svn`
#	cd $(distdir) && ./_autosetup

svnversion.cpp: version.sh
	$(srcdir)/version.sh svnversion.cpp

BUILT_SOURCES=svnversion.cpp

.PHONY: svnversion.cpp

