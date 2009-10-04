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

#include <cstdlib>
#include <ostream>
#include <sstream>

class SaveIosFlags {
    std::ios_base& stream_;
    std::ios_base::fmtflags flags_;
public:
    SaveIosFlags(std::ios_base& stream): stream_(stream) {
        flags_ = stream.flags();
    }
    ~SaveIosFlags() {
        stream_.flags(flags_);
    }
};

// this is in a separate function so it can be specialized more easily
template <typename T>
inline std::ostream& write_tag(std::ostream& stream, const char* name, const T& value) {
    SaveIosFlags saver(stream);
    stream << std::fixed;
    return stream << '<' << name << '>' << value << "</" << name << ">\n";
}

template <typename T>
class XmlTag {
private:
    const char* name;
    const T& value;

    friend std::ostream& operator<<(std::ostream& stream, const XmlTag<T>& tag) {
        return write_tag(stream, tag.name, tag.value);
    }
public:
    XmlTag(const char* name, const T& value):
        name(name), value(value)
    {}
};

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
