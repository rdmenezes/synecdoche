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

#ifndef TEST_TESTSTRUTIL_H
#define TEST_TESTSTRUTIL_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "lib/str_util.h"

class TestStrUtil: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestStrUtil);
    CPPUNIT_TEST(testStripWsStdString);
    CPPUNIT_TEST_SUITE_END();

  public:
    void testStripWsStdString();

};

#endif

