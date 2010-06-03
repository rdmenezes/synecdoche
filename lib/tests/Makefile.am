check_PROGRAMS = TestLib

TestLib_SOURCES = \
	TestXmlParser.cpp \
	TestStrUtil.cpp \
	TestMioFile.cpp \
	TestXmlWrite.cpp \
	TestUtil.cpp

TestLib_CPPFLAGS = -I$(top_srcdir)
TestLib_CXXFLAGS = $(UNITTEST_CFLAGS)
TestLib_LDADD = ../libboinc.a $(top_builddir)/tests/libsynectest.a $(UNITTEST_LIBS)

TESTS = $(check_PROGRAMS)
