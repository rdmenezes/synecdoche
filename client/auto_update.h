// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2006 University of California
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

#ifndef _AUTO_UPDATE_
#define _AUTO_UPDATE_

#include <vector>
#include "common_defs.h"
#include "client_types.h"

class AUTO_UPDATE {
public:
    bool present;
    bool install_failed;
    VERSION_INFO version;
    std::vector<FILE_REF> file_refs;
    PROJECT* project;

    AUTO_UPDATE();
    void init();
    int parse(MIOFILE&);
    void write(MIOFILE&) const;
    int validate_and_link(PROJECT*);
    void install();
    void poll();
};

#endif

