// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Nicolas Alvarez
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <sstream>
#include "lib/xml_write.h"

using std::string;

/// Unit tests for the write_escaped_xml function.
class TestXmlEscape: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestXmlEscape);
    CPPUNIT_TEST(testNothing);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testAmp);
    CPPUNIT_TEST(testTag);
    CPPUNIT_TEST(testConsecutive);
    CPPUNIT_TEST(testHighByte);
    CPPUNIT_TEST_SUITE_END();

  private:
    std::ostringstream oss;

  public:
//these macros makes data-driven tests easier
#define ADD_TEST(name, input, output) \
void test ## name() { \
    write_escaped_xml(oss, input); \
    CPPUNIT_ASSERT_EQUAL(string(output), oss.str()); \
}
#define ADD_TEST2(name, input, output1, output2) \
void test ## name() { \
    write_escaped_xml(oss, input); \
    try { \
        CPPUNIT_ASSERT_EQUAL(string(output1), oss.str()); \
    } catch (CppUnit::Exception) { \
        CPPUNIT_ASSERT_EQUAL(string(output2), oss.str()); \
    } \
}

    /// Test an input that doesn't need escaping.
    /// The output should be the same as the input.
    ADD_TEST(Nothing, "test", "test")

    /// Test an empty string as input.
    ADD_TEST(Empty, "", "")

    /// Test escaping a string containing an ampersand.
    ADD_TEST(Amp, "t&st", "t&amp;st")

    /// Test escaping a typical XML tag. This test checks if the open angle
    /// bracket is escaped, but works both if the right angle bracket is
    /// escaped and if it's not.
    ADD_TEST2(Tag, "<test>", "&lt;test&gt;", "&lt;test>")

    /// Test two consecutive ampersands.
    ADD_TEST(Consecutive, "x&&x", "x&amp;&amp;x")

    /// Test a string with a character &gt;128.
    ADD_TEST(HighByte, "qu\xe9?", "qu&#233;?")
#undef ADD_TEST
#undef ADD_TEST2
};

/// Unit tests for the XmlTag_t class, and using it along with XmlString.
class TestXmlWrite: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestXmlWrite);
    CPPUNIT_TEST(testInt);
    CPPUNIT_TEST(testStrLiteral);
    CPPUNIT_TEST(testCharp);
    CPPUNIT_TEST(testStdString);

    CPPUNIT_TEST(testStrLiteralEsc);
    CPPUNIT_TEST(testCharpEsc);
    CPPUNIT_TEST(testStdStringEsc);
    CPPUNIT_TEST_SUITE_END();

  private:
    std::ostringstream oss;
  public:
    void testInt() {
        oss << XmlTag<int>("foo", 42);
        CPPUNIT_ASSERT_EQUAL(string("<foo>42</foo>\n"), oss.str());
    }
    void testStrLiteral() {
        oss << XmlTag<const char*>("foo", "bar");
        CPPUNIT_ASSERT_EQUAL(string("<foo>bar</foo>\n"), oss.str());
    }
    void testCharp() {
        const char* value = "bar";
        oss << XmlTag<const char*>("foo", value);
        CPPUNIT_ASSERT_EQUAL(string("<foo>bar</foo>\n"), oss.str());
    }
    void testStdString() {
        string value = "bar";
        oss << XmlTag<string>("foo", value);
        CPPUNIT_ASSERT_EQUAL(string("<foo>bar</foo>\n"), oss.str());
    }
    void testStrLiteralEsc() {
        oss << XmlTag<XmlString>("foo", "b&r");
        CPPUNIT_ASSERT_EQUAL(string("<foo>b&amp;r</foo>\n"), oss.str());
    }
    void testCharpEsc() {
        const char* value = "b&r";
        oss << XmlTag<XmlString>("foo", value);
        CPPUNIT_ASSERT_EQUAL(string("<foo>b&amp;r</foo>\n"), oss.str());
    }
    void testStdStringEsc() {
        string value = "b&r";
        oss << XmlTag<XmlString>("foo", value);
        CPPUNIT_ASSERT_EQUAL(string("<foo>b&amp;r</foo>\n"), oss.str());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestXmlEscape);
CPPUNIT_TEST_SUITE_REGISTRATION(TestXmlWrite);

