// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2010 Nicolas Alvarez
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
/// Unit tests for utility code in the manager.

#include <UnitTest++.h>

#include <wx/string.h>

#include "clientgui/UiFormatString.h"

//this sucks; we *have* to move away from UnitTest++
#define CHECK_STREQUAL(expected, actual) \
    assertStrEqual(*UnitTest::CurrentTest::Results(), expected, actual, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__))

void assertStrEqual(UnitTest::TestResults& results, const wxString& expected, const wxString& actual, UnitTest::TestDetails const& details)
{
    if (!(expected == actual))
    {
        UnitTest::MemoryOutStream stream;
        stream << "Expected \"" << expected.ToAscii() << "\" but was \"" << actual.ToAscii() << '\"';

        results.OnTestFailure(details, stream.GetText());
    }
}

SUITE(TestFormatString)
{
    TEST(SingleFormatSpecifier)
    {
        UiFormatString str(wxT("Connect every %1 days"));
        CHECK_EQUAL(2u, str.labels().size());
        CHECK_EQUAL(1u, str.placeholders().size());
        CHECK_STREQUAL(wxT("Connect every"), str.label(0));
        CHECK_STREQUAL(wxT("days"), str.label(1));
        CHECK_EQUAL(1, str.placeholder(0));
    }
    TEST(TwoFormats)
    {
        UiFormatString str(wxT("Maximum %1 MB every %2 days"));
        CHECK_EQUAL(3u, str.labels().size());
        CHECK_EQUAL(2u, str.placeholders().size());
        CHECK_STREQUAL(wxT("Maximum"), str.label(0));
        CHECK_STREQUAL(wxT("MB every"), str.label(1));
        CHECK_STREQUAL(wxT("days"), str.label(2));
        CHECK_EQUAL(1, str.placeholder(0));
        CHECK_EQUAL(2, str.placeholder(1));
    }
    TEST(TwoFormatsReversed)
    {
        UiFormatString str(wxT("Every %2 days, download up to %1 MB"));
        CHECK_EQUAL(3u, str.labels().size());
        CHECK_EQUAL(2u, str.placeholders().size());
        CHECK_STREQUAL(wxT("Every"), str.label(0));
        CHECK_STREQUAL(wxT("days, download up to"), str.label(1));
        CHECK_STREQUAL(wxT("MB"), str.label(2));
        CHECK_EQUAL(2, str.placeholder(0));
        CHECK_EQUAL(1, str.placeholder(1));
    }
}
