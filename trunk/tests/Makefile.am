check_PROGRAMS = UnitTest

UnitTest_SOURCES = TestRunner.cpp \
                   TestXmlParser.cpp \
                   TestStrUtil.cpp \
                   TestMioFile.cpp \
                   TestXmlWrite.cpp \
                   TestUtil.cpp \
                   Util.cpp Util.h
UnitTest_CPPFLAGS = -I$(top_srcdir)
UnitTest_CXXFLAGS = $(UNITTEST_CFLAGS)
UnitTest_LDADD = $(UNITTEST_LIBS) ../lib/libboinc.a

check-local:
	./UnitTest
