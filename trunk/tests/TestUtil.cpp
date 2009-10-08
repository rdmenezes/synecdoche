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
/// Unit tests for lib/util.C

#include <sstream>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include "lib/util.h"
#include "Util.h"

class TestUtil: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestUtil);
    CPPUNIT_TEST(testCopyStream);
    CPPUNIT_TEST(testCopyStreamLarge);
    CPPUNIT_TEST_SUITE_END();

  private:
    //testsuite-wide variables

  public:
    void testCopyStream()
    {
        std::istringstream iss("Hello world");
        std::ostringstream oss;
        copy_stream(iss, oss);
        CPPUNIT_ASSERT_EQUAL(iss.str(), oss.str());
    }

    void testCopyStreamLarge()
    {
        std::string str = rand_string(1024 + std::rand() % 1000, rand_char_null);

        std::istringstream iss(str);
        std::ostringstream oss;
        copy_stream(iss, oss);
        CPPUNIT_ASSERT_EQUAL(str, oss.str());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestUtil);
