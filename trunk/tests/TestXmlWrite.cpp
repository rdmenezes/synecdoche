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

#include <string>
#include <sstream>

#include <UnitTest++.h>

#include "lib/xml_write.h"

using std::string;

struct OssFixture {
    std::ostringstream oss;
};

/// Unit tests for the write_escaped_xml function.
SUITE(TestXmlEscape)
{
//this macro makes data-driven tests easier
#define XML_TEST(name, input, output) \
TEST_FIXTURE(OssFixture, Test ## name) { \
    write_escaped_xml(oss, input); \
    CHECK_EQUAL(output, oss.str()); \
}

    /// Test an input that doesn't need escaping.
    /// The output should be the same as the input.
    XML_TEST(Nothing, "test", "test")

    /// Test an empty string as input.
    XML_TEST(Empty, "", "")

    /// Test escaping a string containing an ampersand.
    XML_TEST(Amp, "t&st", "t&amp;st")

    /// Test escaping a typical XML tag. This test checks if the open angle
    /// bracket is escaped, but works both if the right angle bracket is
    /// escaped and if it's not.
    TEST_FIXTURE(OssFixture, TestTag) {
        write_escaped_xml(oss, "<test>");
        CHECK(oss.str() == "&lt;test&gt;" || oss.str() == "&lt;test>");
    }

    /// Test two consecutive ampersands.
    XML_TEST(Consecutive, "x&&x", "x&amp;&amp;x")

    /// Test a string with a character &gt;128.
    XML_TEST(HighByte, "qu\xe9?", "qu&#233;?")

    /// Test a string with a newline character.
    XML_TEST(Newline, "hello\nworld", "hello&#10;world")
#undef XML_TEST
}

/// Unit tests for the XmlTag_t class, and using it along with XmlString.
SUITE(TestXmlWrite)
{
    TEST_FIXTURE(OssFixture, Int) {
        oss << XmlTag<int>("foo", 42);
        CHECK_EQUAL("<foo>42</foo>\n", oss.str());
    }
    TEST_FIXTURE(OssFixture, StrLiteral) {
        oss << XmlTag<const char*>("foo", "bar");
        CHECK_EQUAL("<foo>bar</foo>\n", oss.str());
    }
    TEST_FIXTURE(OssFixture, Charp) {
        const char* value = "bar";
        oss << XmlTag<const char*>("foo", value);
        CHECK_EQUAL("<foo>bar</foo>\n", oss.str());
    }
    TEST_FIXTURE(OssFixture, StdString) {
        string value = "bar";
        oss << XmlTag<string>("foo", value);
        CHECK_EQUAL("<foo>bar</foo>\n", oss.str());
    }
    TEST_FIXTURE(OssFixture, StrLiteralEsc) {
        oss << XmlTag<XmlString>("foo", "b&r");
        CHECK_EQUAL("<foo>b&amp;r</foo>\n", oss.str());
    }
    TEST_FIXTURE(OssFixture, CharpEsc) {
        const char* value = "b&r";
        oss << XmlTag<XmlString>("foo", value);
        CHECK_EQUAL("<foo>b&amp;r</foo>\n", oss.str());
    }
    TEST_FIXTURE(OssFixture, StdStringEsc) {
        string value = "b&r";
        oss << XmlTag<XmlString>("foo", value);
        CHECK_EQUAL("<foo>b&amp;r</foo>\n", oss.str());
    }
}
