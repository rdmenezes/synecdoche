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

#ifndef _VALIDATEYESNO_H_
#define _VALIDATEYESNO_H_

#include "wx/valtext.h"
#include "prefs.h"


class ValidateYesNo : public wxValidator {

    DECLARE_CLASS(ValidateYesNo)

public:
    ValidateYesNo() : wxValidator() {}
    ValidateYesNo(bool* val) : wxValidator(), m_bool(val) {

    }
    ValidateYesNo(const ValidateYesNo& val): wxValidator() { 
        //wxValidator::Copy(val);
        m_bool = val.m_bool; }
    ~ValidateYesNo() {};

    virtual wxObject*   Clone() const { return new ValidateYesNo(*this); }

    virtual bool        TransferToWindow();
    virtual bool        TransferFromWindow();
    virtual bool        Validate(wxWindow* parent);
    bool                GetBool() const { return *m_bool; }

protected:
    bool*             m_bool;
};

#endif // _VALIDATEYESNO_H_
