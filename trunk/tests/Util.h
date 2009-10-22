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

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <algorithm>
#include <string>

char rand_char();
char rand_char_null();

template <typename Generator>
std::string rand_string(size_t length, Generator func)
{
    std::string str;
    str.resize(length);
    std::generate(str.begin(), str.end(), func);
    return str;
}

#endif
