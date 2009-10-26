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

#include <string>
#include <list>

#include <UnitTest++.h>

#include "lib/str_util.h"

using std::string;
using std::list;

/// Unit tests for several functions in lib/str_util.h.
SUITE(TestStrUtil)
{
    /// \test Basic usage of strip_whitespace().
    /// \note Only the std::string overload is being tested here. However, the
    /// char* overload is implemented by calling the string overload...
    TEST(StripWsStdString)
    {
        string input;

        input = "test";
        strip_whitespace(input);
        CHECK_EQUAL("test", input);

        input = "test   ";
        strip_whitespace(input);
        CHECK_EQUAL("test", input);

        input = "   test";
        strip_whitespace(input);
        CHECK_EQUAL("test", input);

        input = "    test   ";
        strip_whitespace(input);
        CHECK_EQUAL("test", input);

        input = "\r\n  test\r\n";
        strip_whitespace(input);
        CHECK_EQUAL("test", input);
    }
    /// \test Tests the starts_with() function.
    /// It checks normal situations, and corner cases like empty strings.
    TEST(StartsWith)
    {
        CHECK_EQUAL(true, starts_with("preteststring", "pre"));
        CHECK_EQUAL(true, starts_with("pre", "pre"));
        CHECK_EQUAL(false, starts_with("teststring", "pre"));
        CHECK_EQUAL(false, starts_with("long", "longer"));

        // empty strings don't start with anything
        CHECK_EQUAL(false, starts_with("", "foo"));

        // but everything starts with the empty string
        CHECK_EQUAL(true, starts_with("foo", ""));
        CHECK_EQUAL(true, starts_with("", ""));
    }
    /// \test Tests the ends_with() function.
    /// It checks normal situations, and corner cases like empty strings.
    TEST(EndsWith)
    {
        CHECK_EQUAL(true, ends_with("teststringpost", "post"));
        CHECK_EQUAL(true, ends_with("post", "post"));
        CHECK_EQUAL(false, ends_with("teststring", "post"));

        // empty strings don't end with anything
        CHECK_EQUAL(false, ends_with("", "foo"));

        // but everything ends with the empty string
        CHECK_EQUAL(true, ends_with("foo", ""));
        CHECK_EQUAL(true, ends_with("", ""));
    }
    /// \test Calls parse_command_line() in the simplest situation, passing two
    /// arguments with no quotes.
    TEST(CmdLineParse)
    {
        list<string> args = parse_command_line("simple test");
        CHECK_EQUAL(2u, args.size());

        list<string>::iterator it = args.begin();
        CHECK_EQUAL("simple", *it++);
        CHECK_EQUAL("test", *it++);
    }
    /// \test Calls parse_command_line() with a single character as the last
    /// argument. This functionality was broken before, crashing workunits for
    /// some BOINC projects.
    /// \sa Synecdoche \issue{59}.
    TEST(CmdLineParseSingleCharLast)
    {
        list<string> args = parse_command_line("foo --nthreads 2");
        CHECK_EQUAL(3u, args.size());

        list<string>::iterator it = args.begin();
        CHECK_EQUAL("foo", *it++);
        CHECK_EQUAL("--nthreads", *it++);
        CHECK_EQUAL("2", *it++);
    }
}
