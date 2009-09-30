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
/// Skeleton for unit test files.
/// To create a new test class:
/// -# Copy from this file (using 'svn copy')
/// -# Add it to tests/Makefile.am
/// -# Replace this doxygen comment with a short description of the test. Place
/// a longer description in the class itself.
/// -# Replace the class name in all three places: class declaration, TEST_SUITE
/// macro at the beginning of the class, and TEST_SUITE_REGISTRATION at the end
/// of the file.

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

class TestFoo: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestFoo);
    CPPUNIT_TEST(testOne);
    CPPUNIT_TEST_SUITE_END();

  private:
    //testsuite-wide variables

  public:
    void testOne()
    {
        int expected = 42, actual = 42;
        CPPUNIT_ASSERT_EQUAL(expected, actual);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestFoo);
