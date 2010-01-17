check_PROGRAMS = TestLib

TestLib_SOURCES = \
	TestRunner.cpp \
	TestXmlParser.cpp \
	TestStrUtil.cpp \
	TestMioFile.cpp \
	TestXmlWrite.cpp \
	TestUtil.cpp \
	Util.cpp Util.h

TestLib_CPPFLAGS = -I$(top_srcdir)
TestLib_CXXFLAGS = $(UNITTEST_CFLAGS)
TestLib_LDADD = $(UNITTEST_LIBS) ../lib/libboinc.a

TESTS = $(check_PROGRAMS)

# this was the old name for TestLib.
MOSTLYCLEANFILES = UnitTest UnitTest-*.o
