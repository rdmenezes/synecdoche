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

#include "lib/miofile.h"
#include "lib/parse.h"

class TestXmlParser: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestXmlParser);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST_SUITE_END();

  private:
    MIOFILE mf;
    XML_PARSER xp;

  public:
    TestXmlParser():
        mf(),
        xp(&mf)
    {
        ;
    }

    void testSimple()
    {
        mf.init_buf_read("<root></root>");

        CPPUNIT_ASSERT(xp.parse_start("root"));

        char tag[256];
        bool is_tag;

        CPPUNIT_ASSERT_EQUAL(false, xp.get(tag, sizeof(tag), is_tag));
        CPPUNIT_ASSERT_EQUAL(true, is_tag);
        CPPUNIT_ASSERT_EQUAL(std::string("/root"), std::string(tag));

        CPPUNIT_ASSERT_EQUAL(true, xp.get(tag, sizeof(tag), is_tag));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestXmlParser);

