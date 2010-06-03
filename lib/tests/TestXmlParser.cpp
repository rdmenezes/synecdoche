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

#include <string>

#include <UnitTest++.h>

#include "lib/miofile.h"
#include "lib/parse.h"

SUITE(TestXmlParser)
{
    struct XmlParserFixture
    {
      protected:
        MIOFILE mf;
        XML_PARSER xp;

      public:
        XmlParserFixture():
            mf(),
            xp(&mf)
        {
            ;
        }
    };
    TEST_FIXTURE(XmlParserFixture, Simple)
    {
        mf.init_buf_read("<root></root>");

        CHECK(xp.parse_start("root"));

        char tag[256];
        bool is_tag;

        CHECK_EQUAL(false, xp.get(tag, sizeof(tag), is_tag));
        CHECK_EQUAL(true, is_tag);
        CHECK_EQUAL("/root", std::string(tag));

        CHECK_EQUAL(true, xp.get(tag, sizeof(tag), is_tag));
    }
    TEST(XmlUnescape) {
        CHECK_EQUAL("<foo>", xml_unescape("&lt;foo>"));
        CHECK_EQUAL("a&b", xml_unescape("a&amp;b"));
        CHECK_EQUAL("a&&b", xml_unescape("a&amp;&amp;b"));
        CHECK_EQUAL("&", xml_unescape("&amp;"));
        CHECK_EQUAL("a&amp;b", xml_unescape("a&amp;amp;b"));
        CHECK_EQUAL("<foo>", xml_unescape("&lt;foo&gt;"));
        CHECK_EQUAL("foo", xml_unescape("&#102;&#111;&#111;"));
    }
}
