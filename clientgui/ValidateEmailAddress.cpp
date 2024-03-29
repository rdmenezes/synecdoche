// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
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

#include "ValidateEmailAddress.h"

#include "stdwx.h"

IMPLEMENT_DYNAMIC_CLASS(CValidateEmailAddress, wxValidator)


CValidateEmailAddress::CValidateEmailAddress(wxString *val) {
    m_stringValue = val ;
}


CValidateEmailAddress::CValidateEmailAddress(const CValidateEmailAddress& val)
    : wxValidator()
{
    Copy(val);
}


CValidateEmailAddress::~CValidateEmailAddress() {}


bool CValidateEmailAddress::Copy(const CValidateEmailAddress& val) {
    wxValidator::Copy(val);

    m_stringValue = val.m_stringValue ;

    return TRUE;
}


bool CValidateEmailAddress::Validate(wxWindow *parent) {
    if(!CheckValidator())
        return FALSE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;

    if (!control->IsEnabled())
        return TRUE;

    bool ok = TRUE;
    wxString val(control->GetValue().Trim().Trim(false));  // trim spaces before and after

    // Regular Expression from Frank S. Thomas
    wxRegEx reEmail(
        wxT("^([^@]+)@([^@\\.]+)\\.([^@]{2,})$"));

    if (val.Length() == 0) {
        ok = FALSE;
        m_errormsg = _("Please specify an email address");
    } else if (!reEmail.Matches(val)) {
        ok = FALSE;
        m_errormsg = _("Invalid email address; please enter a valid email address");
    }

    if (!ok) {
        wxASSERT_MSG(!m_errormsg.empty(), _T("you forgot to set errormsg"));

        m_validatorWindow->SetFocus();

        wxString buf;
        buf.Printf(m_errormsg, control->GetValue().c_str());

        wxMessageBox(buf, _("Validation conflict"),
            wxOK | wxICON_EXCLAMATION, parent
        );
    }

    return ok;
}


bool CValidateEmailAddress::TransferToWindow(void) {
    if(!CheckValidator())
        return FALSE;
    
    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    control->SetValue(* m_stringValue) ;

    return TRUE;
}


bool CValidateEmailAddress::TransferFromWindow(void) {
    if(!CheckValidator())
        return FALSE;

    if (!m_stringValue)
        return TRUE;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow ;
    * m_stringValue = control->GetValue() ;

    return TRUE;
}


bool CValidateEmailAddress::wxIsAlphaNumeric(const wxString& val) {
    size_t i;
    for (i = 0; i < val.Length(); i++) {
        if (!wxIsalnum(val[i]))
            return FALSE;
    }
    return TRUE;
}


bool CValidateEmailAddress::CheckValidator() const {
    wxCHECK_MSG(m_validatorWindow, FALSE,
                    _T("No window associated with validator"));
    wxCHECK_MSG(m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), FALSE,
                    _T("wxTextValidator is only for wxTextCtrl's"));
    wxCHECK_MSG(m_stringValue, FALSE,
                    _T("No variable storage for validator"));

    return TRUE;
}
