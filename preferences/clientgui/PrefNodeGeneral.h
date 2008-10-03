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

#ifndef PREFNODEGENERAL_H
#define PREFNODEGENERAL_H

#include "PrefGridBase.h"

class GLOBAL_PREFS;

/// Preferences that don't fit into other categories.
class PrefNodeGeneral : public PrefGridBase {

    DECLARE_DYNAMIC_CLASS(PrefNodeGeneral)

public:
    PrefNodeGeneral(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodeGeneral() {}
};

#endif // PREFNODEGENERAL_H
