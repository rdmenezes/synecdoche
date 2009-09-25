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

#include <cstring>

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include "lib/miofile.h"
#include "lib/mfile.h"
#include "lib/miofile_wrap.h"

/* These tests are incomplete; but I plan to get rid of MIOFILE and MFILE anyway. */

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
class TestMioFileAdapter: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestMioFileAdapter);
    CPPUNIT_TEST(testTestFuncStandalone);
    CPPUNIT_TEST(testWrapper);
    CPPUNIT_TEST_SUITE_END();

  public:
    void funcUsingMiofile(MIOFILE& out, int n) {
        out.printf("Test%d", n);
    }
    void testTestFuncStandalone() {
        MIOFILE mf;
        MFILE m;
        mf.init_mfile(&m);

        funcUsingMiofile(mf, 42);
        
        char* p; int n;
        m.get_buf(p, n);
        CPPUNIT_ASSERT(0 != p);
        CPPUNIT_ASSERT_EQUAL(6, n);
        CPPUNIT_ASSERT(strcmp("Test42", p) == 0);
        free(p);
    }
    void testWrapper() {
        std::ostringstream oss;
        funcUsingMiofile(MiofileAdapter(oss), 42);
        CPPUNIT_ASSERT_EQUAL(std::string("Test42"), oss.str());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestMfile);
CPPUNIT_TEST_SUITE_REGISTRATION(TestMioFile);
CPPUNIT_TEST_SUITE_REGISTRATION(TestMioFileAdapter);
