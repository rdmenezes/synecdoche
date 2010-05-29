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

#ifndef _VALIDATEEMAILADDRESS_H_
#define _VALIDATEEMAILADDRESS_H_

#include <wx/validate.h>

class CValidateEmailAddress : public wxValidator
{
    DECLARE_DYNAMIC_CLASS( CValidateEmailAddress )

public:

    CValidateEmailAddress( wxString *val = 0 );
    CValidateEmailAddress( const CValidateEmailAddress& val );

    ~CValidateEmailAddress();

    virtual wxObject* Clone() const { return new CValidateEmailAddress(*this); }
    virtual bool      Copy( const CValidateEmailAddress& val );

    virtual bool      Validate(wxWindow *parent);
    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();

protected:
    wxString*         m_stringValue;
    wxString          m_errormsg;

    bool              wxIsAlphaNumeric(const wxString& val);
    virtual bool      CheckValidator() const;

};


#endif

