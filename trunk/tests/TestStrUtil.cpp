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

#include "TestStrUtil.h"

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include "lib/str_util.h"

void TestStrUtil::testStripWsStdString()
{
    std::string input;
    input = "test";
    strip_whitespace(input);
    CPPUNIT_ASSERT_EQUAL(std::string("test"), input);

    input = "test   ";
    strip_whitespace(input);
    CPPUNIT_ASSERT_EQUAL(std::string("test"), input);

    input = "   test";
    strip_whitespace(input);
    CPPUNIT_ASSERT_EQUAL(std::string("test"), input);

    input = "    test   ";
    strip_whitespace(input);
    CPPUNIT_ASSERT_EQUAL(std::string("test"), input);

    input = "\r\n  test\r\n";
    strip_whitespace(input);
    CPPUNIT_ASSERT_EQUAL(std::string("test"), input);
}
void TestStrUtil::testStartsWith()
{
    CPPUNIT_ASSERT_EQUAL(true, starts_with("preteststring", "pre"));
    CPPUNIT_ASSERT_EQUAL(true, starts_with("pre", "pre"));
    CPPUNIT_ASSERT_EQUAL(false, starts_with("teststring", "pre"));
    CPPUNIT_ASSERT_EQUAL(false, starts_with("long", "longer"));

    // empty strings don't start with anything
    CPPUNIT_ASSERT_EQUAL(false, starts_with("", "foo"));

    // but everything starts with the empty string
    CPPUNIT_ASSERT_EQUAL(true, starts_with("foo", ""));
    CPPUNIT_ASSERT_EQUAL(true, starts_with("", ""));
}
void TestStrUtil::testEndsWith()
{
    CPPUNIT_ASSERT_EQUAL(true, ends_with("teststringpost", "post"));
    CPPUNIT_ASSERT_EQUAL(true, ends_with("post", "post"));
    CPPUNIT_ASSERT_EQUAL(false, ends_with("teststring", "post"));

    // empty strings don't end with anything
    CPPUNIT_ASSERT_EQUAL(false, ends_with("", "foo"));

    // but everything ends with the empty string
    CPPUNIT_ASSERT_EQUAL(true, ends_with("foo", ""));
    CPPUNIT_ASSERT_EQUAL(true, ends_with("", ""));
}
CPPUNIT_TEST_SUITE_REGISTRATION(TestStrUtil);

