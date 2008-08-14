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
#ifndef _WIZ_BOINCBASEWIZARD_H_
#define _WIZ_BOINCBASEWIZARD_H_

/*!
 * CBOINCBaseWizard class declaration
 */

class CBOINCBaseWizard: public wxWizardEx
{    
    DECLARE_DYNAMIC_CLASS( CBOINCBaseWizard )

public:
    /// Constructors
    CBOINCBaseWizard();
    CBOINCBaseWizard(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxBitmap& bitmap = wxNullBitmap,
             const wxPoint& pos = wxDefaultPosition,
             long style = wxDEFAULT_DIALOG_STYLE);

    /// Diagnostics functions
    virtual void SetDiagFlags( unsigned long ulFlags );
    virtual bool IsDiagFlagsSet( unsigned long ulFlags );
    unsigned long m_ulDiagFlags;

    /// Track page transitions
    wxWizardPageEx* PopPageTransition();
    virtual wxWizardPageEx* _PopPageTransition();
    wxWizardPageEx* PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID );
    virtual wxWizardPageEx* _PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID );
    std::stack<wxWizardPageEx*> m_PageTransition;

    /// Cancel Event Infrastructure
    bool IsCancelInProgress() const;
    void ProcessCancelEvent( wxWizardExEvent& event );
    virtual void _ProcessCancelEvent( wxWizardExEvent& event );
    bool m_bCancelInProgress;

    /// Button State Infrastructure
    wxButton* GetNextButton() const;
    void SimulateNextButton();
    void EnableNextButton();
    void DisableNextButton();
    wxButton* GetBackButton() const;
    void SimulateBackButton();
    void EnableBackButton();
    void DisableBackButton();

    /// Wizard Detection
    bool IsAttachToProjectWizard;
    bool IsAccountManagerWizard;
    bool IsAccountManagerUpdateWizard;
    bool IsAccountManagerRemoveWizard;

    /// Global Wizard Status
    PROJECT_CONFIG      project_config;
    ACCOUNT_IN          account_in;
    ACCOUNT_OUT         account_out;
    bool                account_created_successfully;
    bool                attached_to_project_successfully;
    bool                close_when_completed;
    wxString            project_name;
    wxString            project_url;
    wxString            project_authenticator;
};

#endif // _WIZ_BOINCBASEWIZARD_H_
