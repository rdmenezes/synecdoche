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
#include "ValidateYesNo.h"


IMPLEMENT_CLASS(ValidateYesNo, wxTextValidator)


bool ValidateYesNo::TransferToWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxComboCtrl))) {

        wxComboCtrl* pControl = (wxComboCtrl*) m_validatorWindow;
        if (m_bool) {
            if (*m_bool ^ m_invert) {
                pControl->SetValue(_("Yes"));
            } else {
                pControl->SetValue(_("No"));
            }
            return true;
        }
    }
    return false;
}


bool ValidateYesNo::TransferFromWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxComboCtrl))) {

        wxComboCtrl* pControl = (wxComboCtrl*) m_validatorWindow;
        if (m_bool) {
            *m_bool = m_invert ^ pControl->GetValue().IsSameAs(_("Yes"));
            return true;
        }
    }
    return false;
}


bool ValidateYesNo::Validate(wxWindow* parent) {

    return true;
}
