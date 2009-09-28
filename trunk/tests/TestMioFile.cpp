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
/// Unit tests for MIOFILE, MFILE, and the temporary adapters for iostreams.

#include <cstring>

#include <sstream>
#include <string>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include "lib/miofile.h"
#include "lib/mfile.h"
#include "lib/miofile_wrap.h"

// These tests are incomplete; but I plan to get rid of MIOFILE and MFILE anyway.

/// Unit tests for MFILE.
/// Currently there is only one test, and it doesn't do much.
class TestMfile: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMfile);
    CPPUNIT_TEST(testPrintf);
    CPPUNIT_TEST_SUITE_END();

  public:
    void testPrintf() {
        MFILE m;
        m.printf("Test");

        char* p;
        int n;
        m.get_buf(p, n);

        CPPUNIT_ASSERT(0 != p);
        CPPUNIT_ASSERT_EQUAL(4, n);
        CPPUNIT_ASSERT(memcmp("Test", p, 4) == 0);

        free(p);
    }
};

/// Unit tests for MIOFILE.
/// Currently there is only one test, and it doesn't do much.
class TestMioFile: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMioFile);
    CPPUNIT_TEST(testFgetsBuf);
    CPPUNIT_TEST_SUITE_END();

  public:
    void testFgetsBuf()
    {
        MIOFILE mf;
        const char* data = "Hello world\n";
        mf.init_buf_read(data);

        char buf[256];
        const char* retval = mf.fgets(buf, 256);

        // check if return value is buf (comparing pointers)
        CPPUNIT_ASSERT_EQUAL((void*)buf, (void*)retval);
        // check if data read into buf is correct
        CPPUNIT_ASSERT(strcmp(data, buf) == 0);

        // test if it returns NULL on EOF
        retval = mf.fgets(buf, 256);
        CPPUNIT_ASSERT_EQUAL((void*)0, (void*)retval);
    }
};

/// Unit tests for MiofileAdapter.
///
/// Currently only tests MiofileAdapter
/// (to wrap an ostream and be passed in place of a MIOFILE).
/// When new adapters are made, this class may be renamed
/// "TestMioFileAdapters" (plural) and test them all.
class TestMioFileAdapter: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMioFileAdapter);
    CPPUNIT_TEST(testWrapper);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST_SUITE_END();

  public:
    /// \name Helpers
    /// @{

    /// Writes \a str to the given MIOFILE.
    /// Helper function for the actual tests.
    void funcUsingMiofile(MIOFILE& out, const char* str) {
        out.printf("%s", str);
    }

    /// Does nothing. Used in testEmpty().
    void noop(MIOFILE& out) {
        ;
    }

    /// Writes \a str to the given std::ostream.
    /// Helper function for the actual tests.
    void funcUsingOstream(std::ostream& out, const char* str) {
        out << str;
    }

    /// @}

    /// \test Tests basic use of MiofileAdapter to adapt an ostream
    /// and pass it to a function that expects a MIOFILE.
    /// First it creates a stringstream.
    /// Then it calls funcUsingMiofile(), giving it a MiofileAdapter created from the stream.
    /// Finally, it compares the contents of the stream's string
    /// with the text written to the MIOFILE.
    void testWrapper() {
        std::ostringstream oss;
        funcUsingMiofile(MiofileAdapter(oss), "foobar");
        CPPUNIT_ASSERT_EQUAL(std::string("foobar"), oss.str());
    }

    /// \test Passes a MiofileAdapter to a function that doesn't actually write
    /// to the MIOFILE, and checks if the string is empty.
    /// It's unlikely that the string wouldn't be empty;
    /// the real test is that nothing \e crashes while running this code.
    void testEmpty() {
        std::ostringstream oss;
        noop(MiofileAdapter(oss));

        CPPUNIT_ASSERT(oss.str().empty());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMfile);
CPPUNIT_TEST_SUITE_REGISTRATION(TestMioFile);
CPPUNIT_TEST_SUITE_REGISTRATION(TestMioFileAdapter);
