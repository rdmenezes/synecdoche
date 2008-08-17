// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
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

#ifndef h_BASE64
#define h_BASE64

#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <string>
#endif

class InvalidBase64Exception
{
};

std::string r_base64_encode (const char* from, size_t length);
  std::string r_base64_decode (const char* from, size_t length);
inline std::string r_base64_decode (std::string const& from)
{
    return r_base64_decode(from.c_str(), from.length());
}


#endif
