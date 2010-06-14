check_PROGRAMS = TestGui

TestGui_SOURCES = \
	TestFormatString.cpp ../UiFormatString.cpp

TestGui_CPPFLAGS = -I$(top_srcdir) $(WX_CPPFLAGS)
TestGui_CXXFLAGS = $(UNITTEST_CFLAGS)
TestGui_LDADD = $(top_builddir)/tests/libsynectest.a $(CLIENTGUILIBS) $(UNITTEST_LIBS)

TESTS = $(check_PROGRAMS)
