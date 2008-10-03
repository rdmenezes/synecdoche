// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#ifndef PREFNODEDISK_H
#define PREFNODEDISK_H

#include "prefs.h"

/// Preferences related to disk use.
/// This includes virtual memory settings.
class PrefNodeDisk : public PrefGridBase {

    DECLARE_DYNAMIC_CLASS(PrefNodeDisk)

public:
    PrefNodeDisk(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodeDisk() {}
};

#endif // PREFNODEDISK_H
