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

#include "stdwx.h"
#include "ValidateBool.h"


IMPLEMENT_CLASS(ValidateBoolInverse, ValidateBool)


//bool ValidateBoolInverse::Copy(const ValidateBoolInverse& val) {
//
//    ValidateBool::Copy(val);
//    return true;
//}


bool ValidateBoolInverse::TransferToWindow(void) {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxCheckBox))) {

        wxCheckBox* pControl = (wxCheckBox*) m_validatorWindow;
        if (m_pBool)
        {
            pControl->SetValue(! *m_pBool);
            return true;
        }
    }
    return false;
}


bool ValidateBoolInverse::TransferFromWindow(void) {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxCheckBox))) {

        wxCheckBox* pControl = (wxCheckBox*) m_validatorWindow;
        if (m_pBool)
        {
            *m_pBool = !pControl->GetValue();
            return true;
        }
    }
    return false;
}