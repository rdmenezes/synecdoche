## -*- mode: makefile; tab-width: 4 -*-
## $Id: Makefile.am 15375 2008-06-08 22:18:52Z davea $

AUTOMAKE_OPTIONS = foreign

## make sure rebuilding uses the right aclocal-switches
ACLOCAL_AMFLAGS = -I m4

if BUILD_CLIENTGUI
   CLIENTGUI_SUBDIRS = clientgui clientgui/res #locale/client 
endif

# ORDER MATTERS below.  One must build dependencies FIRST, then things
# that depend on them later
SUBDIRS = lib client $(CLIENTGUI_SUBDIRS) tests

# Putting a directory name recursively copies the entire contents - the
# dist-hook below gets rid of SVN directories.

EXTRA_DIST =   \
    win_build  \
    INSTALL    \
    version.sh

dist-hook:
	rm -rf `find $(distdir) -name .backup -prune -o -name '*~' -o -name .svn`

svnversion.cpp: version.sh
	$(srcdir)/version.sh svnversion.cpp $(top_srcdir)

BUILT_SOURCES = svnversion.cpp
CLEANFILES = svnversion.cpp

.PHONY: svnversion.cpp docs ctags

docs:
	doxygen Doxyfile 2>&1 | grep -v 'Unsupported xml/html tag'

tags: ctags

ctags:
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q $(SUBDIRS)
