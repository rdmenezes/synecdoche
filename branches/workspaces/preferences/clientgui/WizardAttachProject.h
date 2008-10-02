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
#ifndef WIZ_ATTACHPROJECT_H
#define WIZ_ATTACHPROJECT_H

#include "BOINCBaseWizard.h"

/// Forward declarations of all used pages:
class CAccountKeyPage;
class CAccountInfoPage;
class CCompletionPage;
class CCompletionErrorPage;
class CErrNotDetectedPage;
class CErrUnavailablePage;
class CErrAlreadyAttachedPage;
class CErrNoInternetConnectionPage;
class CErrNotFoundPage;
class CErrAlreadyExistsPage;
class CErrProxyInfoPage;
class CErrProxyPage;
class CProjectInfoPage;
class CProjectPropertiesPage;
class CProjectProcessingPage;
class CWelcomePage;

class CWizardAttachProject: public CBOINCBaseWizard
{    
    DECLARE_DYNAMIC_CLASS(CWizardAttachProject)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWizardAttachProject();
    CWizardAttachProject(wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition);

    /// Creation
    bool Create(wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDATTACHPROJECT_IDNAME, const wxPoint& pos = wxDefaultPosition);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_FINISHED event handler for ID_ATTACHPROJECTWIZARD
    void OnFinished(wxWizardEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_BACKWARD
    void OnWizardBack(wxCommandEvent& event);

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FORWARD
    void OnWizardNext(wxCommandEvent& event);

    /// Runs the wizard.
    bool Run(wxString& strName, wxString& strURL, bool bCredentialsCached = true);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Overrides
    virtual bool HasNextPage(wxWizardPage* page);
    virtual bool HasPrevPage(wxWizardPage* page);

    /// Track page transitions
    wxWizardPage* _PopPageTransition();
    wxWizardPage* _PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID);

    /// Cancel Event Infrastructure
    void _ProcessCancelEvent(wxWizardEvent& event);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Return a pointer to the current account info page.
    CAccountInfoPage* GetAccountInfoPage() const;

    /// Return a pointer to the current account key page.
    CAccountKeyPage* GetAccountKeyPage() const;

    /// Return a pointer to the current completion error page.
    CCompletionErrorPage* GetCompletionErrorPage() const;

    /// Return a pointer to the current project info page.
    CProjectInfoPage* GetProjectInfoPage() const;

    /// Return credentials cache status.
    bool GetCredentialsCached() const;

    /// Tell whether the credentials were detected or not.
    bool GetCretentialsDetected() const;

private:
    CWelcomePage* m_WelcomePage;
    CProjectInfoPage* m_ProjectInfoPage;
    CProjectPropertiesPage* m_ProjectPropertiesPage;
    CAccountKeyPage* m_AccountKeyPage;
    CAccountInfoPage* m_AccountInfoPage;
    CProjectProcessingPage* m_ProjectProcessingPage;
    CCompletionPage* m_CompletionPage;
    CCompletionErrorPage* m_CompletionErrorPage;
    CErrNotDetectedPage* m_ErrNotDetectedPage;
    CErrUnavailablePage* m_ErrUnavailablePage;
    CErrAlreadyAttachedPage* m_ErrAlreadyAttachedPage;
    CErrNoInternetConnectionPage* m_ErrNoInternetConnectionPage;
    CErrNotFoundPage* m_ErrNotFoundPage;
    CErrAlreadyExistsPage* m_ErrAlreadyExistsPage;
    CErrProxyInfoPage* m_ErrProxyInfoPage;
    CErrProxyPage* m_ErrProxyPage;
    bool m_bCredentialsCached;
    bool m_bCredentialsDetected;
    wxString strProjectName;
};

#endif // WIZ_ATTACHPROJECT_H
