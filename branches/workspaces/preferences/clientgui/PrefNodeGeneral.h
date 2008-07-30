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

#ifndef _PREFNODEGENERAL_H_
#define _PREFNODEGENERAL_H_

#include "PrefGridBase.h"

struct GLOBAL_PREFS;

class PrefNodeGeneral : public PrefGridBase {

    DECLARE_DYNAMIC_CLASS(PrefNodeGeneral)

public:
    PrefNodeGeneral(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodeGeneral() {}
};

#endif // _PREFNODEGENERAL_H_
