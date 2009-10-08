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
/// Common utility functions for unit tests.
/// 
/// Not to be confused with TestUtil.cpp, which tests lib/util.C.

#include <cstdlib>

#include <string>
#include <algorithm>

char rand_char() {
    int n = std::rand() % (0x7e - 0x20 + 1) - 1;
    switch (n) {
        case -1: return '\n';
        default: return n + 0x20;
    }
}
char rand_char_null() {
    int n = std::rand() % (0x7e - 0x20 + 2) - 2;
    switch (n) {
        case -2: return '0';
        case -1: return '\n';
        default: return n + 0x20;
    }
}

