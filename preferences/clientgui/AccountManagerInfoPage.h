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
#ifndef _WIZ_ACCOUNTMANAGERINFOPAGE_H_
#define _WIZ_ACCOUNTMANAGERINFOPAGE_H_

class wxHyperlinkCtrl;

/*!
 * CAccountManagerInfoPage class declaration
 */

class CAccountManagerInfoPage: public wxWizardPageEx
{    
    DECLARE_DYNAMIC_CLASS( CAccountManagerInfoPage )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAccountManagerInfoPage( );

    CAccountManagerInfoPage( CBOINCBaseWizard* parent );

    /// Creation
    bool Create( CBOINCBaseWizard* parent );

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAccountManagerInfoPage event handler declarations

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanged( wxWizardExEvent& event );

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnPageChanging( wxWizardExEvent& event );

    /// wxEVT_WIZARD_CANCEL event handler for ID_ACCOUNTMANAGERINFOPAGE
    void OnCancel( wxWizardExEvent& event );

////@end CAccountManagerInfoPage event handler declarations

////@begin CAccountManagerInfoPage member function declarations

    /// Gets the previous page.
    virtual wxWizardPageEx* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPageEx* GetNext() const;

    wxString GetProjectURL() const { return m_strProjectURL ; }
    void SetProjectURL(wxString value) { m_strProjectURL = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAccountManagerInfoPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAccountManagerInfoPage member variables
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    wxStaticText* m_pDescription2StaticCtrl;
    wxStaticText* m_pProjectUrlStaticCtrl;
    wxTextCtrl* m_pProjectUrlCtrl;
    wxStaticText* m_pBOINCPromoStaticCtrl;
    wxHyperlinkCtrl* m_pBOINCPromoUrlCtrl;
    wxString m_strProjectURL;
////@end CAccountManagerInfoPage member variables
};

#endif // _WIZ_ACCOUNTMANAGERINFOPAGE_H_
