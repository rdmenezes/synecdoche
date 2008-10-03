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

#ifndef PREFNODEPRESETS_H
#define PREFNODEPRESETS_H

class GLOBAL_PREFS;

/// Allows setting the preferences to built-in preset values.
class PrefNodePresets : public PrefNodeBase {

    DECLARE_DYNAMIC_CLASS(PrefNodePresets)

public:
    PrefNodePresets(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodePresets() {}

};

#endif // PREFNODEPRESETS_H
