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

/// \file
/// Unit tests for lib/str_util.h

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include "lib/str_util.h"

/// Unit tests for several functions in lib/str_util.h.
class TestStrUtil: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestStrUtil);
    CPPUNIT_TEST(testStripWsStdString);
    CPPUNIT_TEST(testStartsWith);
    CPPUNIT_TEST(testEndsWith);
    CPPUNIT_TEST(testCmdLineParse);
    CPPUNIT_TEST(testCmdLineParseSingleCharLast);
    CPPUNIT_TEST_SUITE_END();

  public:
    /// \test Basic usage of strip_whitespace().
    /// \note Only the std::string overload is being tested here. However, the
    /// char* overload is implemented by calling the string overload...
    void testStripWsStdString()
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
    /// \test Tests the starts_with() function.
    /// It checks normal situations, and corner cases like empty strings.
    void testStartsWith()
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
    /// \test Tests the ends_with() function.
    /// It checks normal situations, and corner cases like empty strings.
    void testEndsWith()
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
    /// \test Calls parse_command_line() in the simplest situation, passing two
    /// arguments with no quotes.
    void testCmdLineParse()
    {
        std::list<std::string> args = parse_command_line("simple test");
        CPPUNIT_ASSERT_EQUAL(size_t(2), args.size());

        std::list<std::string>::iterator it = args.begin();
        CPPUNIT_ASSERT_EQUAL(std::string("simple"), *it++);
        CPPUNIT_ASSERT_EQUAL(std::string("test"), *it++);
    }
    /// \test Calls parse_command_line() with a single character as the last
    /// argument. This functionality was broken before, crashing workunits for
    /// some BOINC projects.
    /// \sa Synecdoche \issue{59}.
    void testCmdLineParseSingleCharLast()
    {
        std::list<std::string> args = parse_command_line("foo --nthreads 2");
        CPPUNIT_ASSERT_EQUAL(size_t(3), args.size());

        std::list<std::string>::iterator it = args.begin();
        CPPUNIT_ASSERT_EQUAL(std::string("foo"), *it++);
        CPPUNIT_ASSERT_EQUAL(std::string("--nthreads"), *it++);
        CPPUNIT_ASSERT_EQUAL(std::string("2"), *it++);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestStrUtil);

