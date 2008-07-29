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

#include "stdwx.h"
#include <wx/combo.h>
#include "ValidateYesNo.h"


IMPLEMENT_CLASS(ValidateYesNo, wxTextValidator)


bool ValidateYesNo::TransferToWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxComboCtrl))) {

        wxComboCtrl* pControl = (wxComboCtrl*) m_validatorWindow;
        if (m_bool) {
            if (*m_bool) {
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
            *m_bool = pControl->GetValue().IsSameAs(_("Yes"));
            return true;
        }
    }
    return false;
}


bool ValidateYesNo::Validate(wxWindow* parent) {

    return true;
}
