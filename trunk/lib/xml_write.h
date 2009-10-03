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
/// \todo document!

#ifndef XML_WRITE_H
#define XML_WRITE_H

/// Disallows the use of strings in XmlTag(). If set to \c 1, uses of XmlTag()
/// with \c char*, \c char array, or \c std::string will fail to compile.
///
/// This is useful to see all places that are calling it with a string, and
/// check if you shouldn't be using XmlString instead (which escapes XML
/// characters).
///
/// \note This feature requires Boost libraries: utility, type_traits, and mpl.
/// Use only during development. Don't commit with this macro set to \c 1.
#define DISALLOW_STRINGS 0

#include <cstdlib>
#include <ostream>
#include <sstream>

template <typename T>
class XmlTag_t {
private:
    const char* name;
    const T& value;

    friend std::ostream& operator<<(std::ostream& stream, const XmlTag_t<T>& tag) {
        return stream << '<' << tag.name << '>' << tag.value << "</" << tag.name << ">\n";
    }
public:
    XmlTag_t(const char* name, const T& value):
        name(name), value(value)
    {}
};

#if DISALLOW_STRINGS
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/mpl/or.hpp>

// I'd love to add some using declarations here so I can remove the "boost::"
// and make the template mess more readable; but this is a header file, I'd
// leak the 'using's to .cpp files that include this.
template <typename T>
typename boost::disable_if<
    boost::mpl::or_<
        boost::is_same<typename boost::decay<const T>::type, const char*>,
        boost::is_same<T, std::string>
    >,
    XmlTag_t<T>
>::type
XmlTag(const char* name, const T& value) {
    return XmlTag_t<T>(name, value);
}
#else
template <typename T>
XmlTag_t<T> XmlTag(const char* name, const T& value) {
    return XmlTag_t<T>(name, value);
}
#endif

inline void write_escaped_xml(std::ostream& stream, const char* str) {
    for (; *str; ++str) {
        int c = (unsigned char)*str;
        if (c == '<') {
            stream << "&lt;";
        } else if (c == '&') {
            stream << "&amp;";
        } else if (c > 127) {
            stream << "&#" << c << ';';
        } else if (c < 32) {
            switch(c) {
            case 9:
            case 10:
            case 13:
                stream << "&#" << c << ';';
            }
        } else {
            stream.put(c);
        }
    }
}
class XmlString {
private:
    const char* data;
public:
    XmlString(const char* data): data(data){}
    XmlString(const std::string& str): data(str.c_str()){}

    friend std::ostream& operator<<(std::ostream& stream, const XmlString& str) {
        write_escaped_xml(stream, str.data);
        return stream;
    }
};


#endif
