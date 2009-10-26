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

#include <UnitTest++.h>

#include "lib/miofile.h"
#include "lib/mfile.h"
#include "lib/miofile_wrap.h"

#include "Util.h"

// These tests are incomplete; but I plan to get rid of MIOFILE and MFILE anyway.


TEST(MfilePrintf)
{
    MFILE m;
    m.printf("Test");

    char* p;
    int n;
    m.get_buf(p, n);

    CHECK(0 != p);
    CHECK_EQUAL(4, n);
    CHECK(memcmp("Test", p, 4) == 0);

    free(p);
}

TEST(MiofileFgetsBuf)
{
    MIOFILE mf;
    const char* data = "Hello world\n";
    mf.init_buf_read(data);

    char buf[256];
    const char* retval = mf.fgets(buf, 256);

    // check if return value is buf (comparing pointers)
    CHECK_EQUAL((void*)buf, (void*)retval);
    // check if data read into buf is correct
    CHECK(strcmp(data, buf) == 0);

    // test if it returns NULL on EOF
    retval = mf.fgets(buf, 256);
    CHECK_EQUAL((void*)0, (void*)retval);
}

/// Unit tests for the Miofile-iostream adapters.
///
/// Currently tests MiofileFromOstream and OstreamFromMiofile.
SUITE(TestMiofileAdapter)
{
    /// Writes \a str to the given MIOFILE.
    /// Helper function for the actual tests.
    void funcUsingMiofile(MIOFILE& out, const char* str) {
        out.printf("%s", str);
    }

    /// \test Tests basic use of MiofileFromOstream to adapt an ostream
    /// and pass it to a function that expects a MIOFILE.
    /// First it creates a stringstream.
    /// Then it calls funcUsingMiofile(), giving it an adapter object created from the stream.
    /// Finally, it compares the contents of the stream's string
    /// with the text written to the MIOFILE.
    TEST(TestMiofileFromOstream) {
        std::ostringstream oss;
        funcUsingMiofile(MiofileFromOstream(oss), "foobar");
        CHECK_EQUAL("foobar", oss.str());
    }

    /// Does nothing. Used in testEmpty().
    void noop(MIOFILE&) {
        ;
    }

    /// \test Passes a MiofileFromOstream adapter
    /// to a function that doesn't actually write to the MIOFILE,
    /// and checks if the string is empty.
    /// It's unlikely that the string wouldn't be empty;
    /// the real test is that nothing \e crashes while running this code.
    TEST(TestMioFromOstreamEmpty) {
        std::ostringstream oss;
        noop(MiofileFromOstream(oss));

        CHECK(oss.str().empty());
    }

    /// Writes \a str to the given std::ostream.
    /// Helper function for the actual tests.
    void funcUsingOstream(std::ostream& out, const std::string& str) {
        out << str;
    }

    /// \test Tests basic use of OstreamFromMiofile to adapt a MIOFILE
    /// and pass it to a function that expects an ostream.
    ///
    /// First it creates a MIOFILE attached to an MFILE.
    /// Then it calls funcUsingOstream(),
    /// giving it an OstreamFromMiofile adapter created from the MIOFILE.
    /// Finally, it compares the contents of the MFILE
    /// with the text written to the ostream.
    TEST(TestOstreamFromMio) {
        MIOFILE mf;
        MFILE m;
        mf.init_mfile(&m);

        funcUsingOstream(OstreamFromMiofile(mf), "test");

        char* p;
        int n;
        m.get_buf(p, n);

        CHECK(0 != p);
        CHECK_EQUAL(4, n);
        CHECK(memcmp("test", p, 4) == 0);

        free(p);
    }

    /// \test Tests OstreamFromMiofile with more data than fits
    /// in MFILE's vprintf buffer.
    TEST(TestOstreamFromMioLarge) {
        MIOFILE mf;
        MFILE m;
        mf.init_mfile(&m);

        const std::string data = rand_string(312345, rand_char);
        funcUsingOstream(OstreamFromMiofile(mf), data);

        char* p;
        int n;
        m.get_buf(p, n);

        CHECK(0 != p);
        CHECK_EQUAL(data.size(), n);
        CHECK(data.compare(0, data.size(), p, n) == 0);

        free(p);
    }
}
