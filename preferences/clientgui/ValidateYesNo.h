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

#ifndef _VALIDATEYESNO_H_
#define _VALIDATEYESNO_H_

#include "prefs.h"


class ValidateYesNo : public wxValidator {

    DECLARE_CLASS(ValidateYesNo)

public:
    ValidateYesNo() : wxValidator() {}
    ValidateYesNo(bool* val, bool invert = false) : wxValidator(), m_bool(val), m_invert(invert) {

    }
    ValidateYesNo(const ValidateYesNo& val): wxValidator() { 
        m_bool = val.m_bool;
        m_invert = val.m_invert;
    }
    ~ValidateYesNo() {};

    virtual wxObject*   Clone() const { return new ValidateYesNo(*this); }

    virtual bool        TransferToWindow();
    virtual bool        TransferFromWindow();
    virtual bool        Validate(wxWindow* parent);
    bool                GetBool() const { return *m_bool; }

protected:
    bool*               m_bool;
    bool                m_invert;
};

#endif // _VALIDATEYESNO_H_
