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
/// Adapter to ease migration from MIOFILE to standard iostreams.

#ifndef MIOFILE_WRAP_H
#define MIOFILE_WRAP_H

#include <cstdlib>
#include <ostream>
#include <sstream>

#include "miofile.h"
#include "mfile.h"

/// Adapts a std::ostream to a MIOFILE so that functions that weren't yet
/// modified to use std::ostream can keep working.
///
/// This adapter should be used as a temporary object in an argument list.
/// Pass the ostream to the constructor.
/// The constructor will create a new internal MIOFILE backed by an MFILE.
/// This adapter object is convertible to MIOFILE&
/// via an explicit conversion operator,
/// which returns the internal MIOFILE.
/// When the temporary MiofileFromOstream is destructed,
/// the data that was written to the MIOFILE will be written to the ostream.
///
/// It's your responsibility to ensure that the std::ostream
/// passed to the constructor remains alive
/// until the adapter is destroyed.
///
/// \par Example
/// \code
/// void write_foo(MIOFILE& fout) {
///     fout.printf("<foo/>");
/// }
///
/// void write_bar(std::ostream& out) {
///     out << "<bar>";
///     write_foo(MiofileFromOstream(out));
///     out << "</bar>";
/// }
/// \endcode

class MiofileFromOstream {
private:
    MIOFILE mf;
    MFILE m;
    std::ostream& stream;

    //disallow assignment
    MiofileFromOstream& operator=(const MiofileFromOstream&);

public:
    MiofileFromOstream(std::ostream& stream):
        stream(stream)
    {
        mf.init_mfile(&m);
    }
    operator MIOFILE&() {
        return mf;
    }
    ~MiofileFromOstream() {
        char* p;
        int n;
        m.get_buf(p, n);
        if (p) {
            stream.write(p, n);
            free(p);
        }
    }
};

/// Adapts a MIOFILE to a std::ostream so that functions that were already
/// modified to use standard streams can still be called by functions not yet
/// converted.
///
/// This adapter should be used as a temporary object in an argument list.
/// Pass the MIOFILE to the constructor.
/// The constructor will create an internal ostringstream.
/// This class is convertible to std::ostream via an explicit conversion operator,
/// which returns the internal stringstream.
/// When this adapter is destructed,
/// the data that was written to the stream
/// will be written to the MIOFILE by calling printf.
///
/// It's your responsibility to ensure that the MIOFILE
/// passed to the constructor, and its associated MFILE if any,
/// remain alive until the adapter is destroyed.
///
/// \par Example
/// \code
/// void write_foo(std::ostream& out) {
///     out << "<foo/>";
/// }
///
/// void write_bar(MIOFILE& out) {
///     out.printf("<bar>");
///     write_foo(OstreamFromMiofile(out));
///     out.printf("</bar>");
/// }
/// \endcode

class OstreamFromMiofile {
private:
    MIOFILE& mf;
    std::ostringstream oss;

    //disallow assignment
    OstreamFromMiofile& operator=(const OstreamFromMiofile&);

public:
    OstreamFromMiofile(MIOFILE& miofile):
        mf(miofile)
    {
    }
    operator std::ostream&() {
        return oss;
    }
    ~OstreamFromMiofile() {
        mf.printf("%s", oss.str().c_str());
    }
};

#endif
