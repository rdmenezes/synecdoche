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
//
#ifndef _WIZ_ACCOUNTKEYPAGE_H_
#define _WIZ_ACCOUNTKEYPAGE_H_

/*!
 * CAccountKeyPage class declaration
 */

class CAccountKeyPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CAccountKeyPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountKeyPage( );

    CAccountKeyPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountKeyPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTKEYPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTKEYPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CAccountKeyPage event handler declarations

////@begin CAccountKeyPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    wxString GetAccountKey() const { return m_strAccountKey ; }
    void SetAccountKey(wxString value) { m_strAccountKey = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountKeyPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountKeyPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
    wxStaticText* m_pAccountKeyExampleDescriptionStaticCtrl;
    wxStaticText* m_pAccountKeyExampleStaticCtrl;
    wxStaticText* m_pAccountKeyStaticCtrl;
    wxTextCtrl* m_pAccountKeyCtrl;
    wxString m_strAccountKey;
////@end CAccountKeyPage member variables
};

#endif // _WIZ_ACCOUNTKEYPAGE_H_