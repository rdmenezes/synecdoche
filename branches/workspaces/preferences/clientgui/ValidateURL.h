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

#ifndef _VALIDATEURL_H_
#define _VALIDATEURL_H_


class CValidateURL : public wxValidator
{
    DECLARE_DYNAMIC_CLASS( CValidateURL )

public:

    CValidateURL( wxString *val = 0 );
    CValidateURL( const CValidateURL& val );

    ~CValidateURL();

    virtual wxObject* Clone() const { return new CValidateURL(*this); }
    virtual bool      Copy( const CValidateURL& val );

    virtual bool      Validate(wxWindow *parent);
    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();

protected:
    wxString*         m_stringValue;
    wxString          m_errortitle;
    wxString          m_errormsg;

    virtual bool      CheckValidator() const;

};


#endif

