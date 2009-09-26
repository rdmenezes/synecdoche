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

#include "miofile.h"
#include "mfile.h"

/// Adapts a std::ostream to a MIOFILE so that functions that weren't yet
/// modified to use std::ostream can keep working.
///
/// This adapter is designed to be used as a temporary object in an argument
/// list. Pass the ostream to the constructor. The constructor will create a
/// new internal MIOFILE backed by an MFILE. The returned object is convertible
/// to MIOFILE via an explicit conversion operator, which returns the internal
/// object. When the temporary MiofileAdapter is destructed, the data that was
/// written to the MIOFILE will be written to the ostream.
///
/// It's your responsibility to ensure the std::ostream passed to the
/// constructor remains alive until the MiofileAdapter is destroyed.
///
/// \par Example
/// \code
/// void write_foo(MIOFILE& fout) {
///     fout.printf("<foo/>");
/// }
///
/// void write_bar(std::ostream& out) {
///     out << "<bar>";
///     write_foo(MiofileAdapter(out));
///     out << "</bar>";
/// }
/// \endcode

class MiofileAdapter {
    MIOFILE mf;
    MFILE m;
    std::ostream& stream;
public:
    MiofileAdapter(std::ostream& stream):
        stream(stream)
    {
        mf.init_mfile(&m);
    }
    operator MIOFILE&() {
        return mf;
    }
    ~MiofileAdapter() {
        char* p;
        int n;
        m.get_buf(p, n);
        if (p) {
            stream.write(p, n);
            free(p);
        }
    }
};

#endif
