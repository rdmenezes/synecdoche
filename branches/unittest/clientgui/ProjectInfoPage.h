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
#ifndef WIZ_PROJECTINFOPAGE_H
#define WIZ_PROJECTINFOPAGE_H

#include <wx/wizard.h>
#include "gui_rpc_client.h"

class CBOINCBaseWizard;
class CProjectListCtrl;
class ProjectListCtrlEvent;
class wxStaticText;
class wxTextCtrl;

class CProjectInfoPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CProjectInfoPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectInfoPage();
    CProjectInfoPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
    void OnPageChanging(wxWizardEvent& event);

    /// wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED event handler for ID_PROJECTSELECTIONCTRL
    void OnProjectSelectionChanged(ProjectListCtrlEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
    void OnCancel(wxWizardEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    wxString GetProjectURL() const { return m_strProjectURL; }
    void SetProjectURL(wxString value) { m_strProjectURL = value; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Disables the validators for all controls on this page.
    void DisableValidators();

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    CProjectListCtrl* m_pProjectListCtrl;
    wxStaticText* m_pProjectUrlStaticCtrl;
    wxTextCtrl* m_pProjectUrlCtrl;
    wxString m_strProjectURL;
    bool                bProjectListPopulated;
    ALL_PROJECTS_LIST   pl;
};

#endif // WIZ_PROJECTINFOPAGE_H
