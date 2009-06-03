// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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

/// \file
/// Base class for manager's wizards

#ifndef _WIZ_BOINCBASEWIZARD_H_
#define _WIZ_BOINCBASEWIZARD_H_

#include <stack>
#include <wx/wizard.h>
#include "gui_rpc_client.h"

class CBOINCBaseWizard: public wxWizard
{    
    DECLARE_DYNAMIC_CLASS(CBOINCBaseWizard)

public:
    /// Constructors
    CBOINCBaseWizard();
    CBOINCBaseWizard(wxWindow *parent,
             int id = wxID_ANY,
             const wxString& title = wxEmptyString,
             const wxBitmap& bitmap = wxNullBitmap,
             const wxPoint& pos = wxDefaultPosition,
             long style = wxDEFAULT_DIALOG_STYLE);

    /// Creation
    bool Create(wxWindow* parent, wxWindowID id, const wxString& title = wxEmptyString, const wxBitmap& bitmap = wxNullBitmap, const wxPoint& pos = wxDefaultPosition);

    /// Diagnostics functions
    virtual void SetDiagFlags(unsigned long ulFlags);
    virtual bool IsDiagFlagsSet(unsigned long ulFlags);
private:
    unsigned long m_ulDiagFlags;

public:
    /// Track page transitions
    wxWizardPage* PopPageTransition();
    virtual wxWizardPage* _PopPageTransition();
    wxWizardPage* PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID);
    virtual wxWizardPage* _PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID);
protected:
    std::stack<wxWizardPage*> m_PageTransition;

public:
    /// Cancel Event Infrastructure
    bool IsCancelInProgress() const;
    void SetCancelInProgress(const bool value);
    void ProcessCancelEvent(wxWizardEvent& event);
    virtual void _ProcessCancelEvent(wxWizardEvent& event);
private:
    bool m_bCancelInProgress;

public:
    /// Check if account creation was successful or not.
    bool GetAccountCreatedSuccessfully() const;

    /// Set or reset the success flag for account creation.
    void SetAccountCreatedSuccessfully(const bool value);

    /// Check if attaching to the project was successful.
    bool GetAttachedToProjectSuccessfully() const;

    /// Set or reset the success flag for project attachment.
    void SetAttachedToProjectSuccessfully(const bool value);

    /// Get the URL for the selected project.
    wxString GetProjectURL() const;

    /// Set the project URL.
    void SetProjectURL(const wxString& value);

    /// Get the name of the selected project.
    wxString GetProjectName() const;

    /// Get the authenticator used for the current project.
    wxString GetProjectAuthenticator() const;

    /// Set the authenticator used for the current project to a new value.
    void SetProjectAuthenticator(const wxString& value);

    /// Get the 'close when completed' flag.
    bool GetCloseWhenCompleted() const;

    /// Set or reset the 'close when completed' flag.
    void SetCloseWhenCompleted(const bool value);

    /// Return a pointer to the project preferences.
    PROJECT_CONFIG* GetProjectConfig();

    /// Return a pointer to the 'account in' object.
    ACCOUNT_IN* GetAccountIn();

    /// Return a pointer to the 'account out' object.
    ACCOUNT_OUT* GetAccountOut();

    /// Button State Infrastructure
    void SimulateNextButton();
    void EnableNextButton();
    void DisableNextButton();
    void SimulateBackButton();
    void EnableBackButton();
    void DisableBackButton();
private:
    wxButton* GetNextButton() const;
    wxButton* GetBackButton() const;

private:
    /// Handle clicks on the 'Back' or 'Next' button.
    void OnBackOrNext(wxCommandEvent& event);

private:
    /// Wizard Status
    PROJECT_CONFIG      project_config;
    ACCOUNT_IN          account_in;
    ACCOUNT_OUT         account_out;
    bool                account_created_successfully;
    bool                attached_to_project_successfully;
    bool                close_when_completed;
    wxString            project_name;
    wxString            project_url;
    wxString            project_authenticator;

    DECLARE_EVENT_TABLE()
};

#endif // _WIZ_BOINCBASEWIZARD_H_
