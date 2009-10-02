check_PROGRAMS = UnitTest

UnitTest_SOURCES = TestRunner.cpp \
                   TestXmlParser.cpp \
                   TestStrUtil.cpp \
                   TestMioFile.cpp \
                   TestXmlWrite.cpp
UnitTest_CPPFLAGS = -I$(top_srcdir)
UnitTest_CXXFLAGS = $(CPPUNIT_CFLAGS)
UnitTest_LDADD = $(CPPUNIT_LIBS) ../lib/libboinc.a

#TESTS = XMLParser
#TESTS_ENVIRONMENT = ./UnitTest

check-local:
	./UnitTest
