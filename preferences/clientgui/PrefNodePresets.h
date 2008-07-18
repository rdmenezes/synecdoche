// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _PREFNODEPRESETS_H_
#define _PREFNODEPRESETS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "PrefNodePresets.cpp"
#endif

struct GLOBAL_PREFS;

class PrefNodePresets : public PrefNodeBase {

    DECLARE_DYNAMIC_CLASS(PrefNodePresets)

public:
    PrefNodePresets(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodePresets() {}

};

#endif // _PREFNODEPRESETS_H_
