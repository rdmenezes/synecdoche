// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
// Copyright (C) 2009 University of California
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

#include "ProjectInfoPage.h"

#include "stdwx.h"
#include "BOINCBaseWizard.h"
#include "BOINCGUIApp.h"
#include "BOINCWizards.h"
#include "MainDocument.h"
#include "hyperlink.h"
#include "ProjectListCtrl.h"
#include "SkinManager.h"
#include "str_util.h"
#include "ValidateURL.h"

IMPLEMENT_DYNAMIC_CLASS(CProjectInfoPage, wxWizardPage)

BEGIN_EVENT_TABLE(CProjectInfoPage, wxWizardPage)
    EVT_WIZARD_PAGE_CHANGED(-1, CProjectInfoPage::OnPageChanged)
    EVT_WIZARD_PAGE_CHANGING(-1, CProjectInfoPage::OnPageChanging)
    EVT_WIZARD_CANCEL(-1, CProjectInfoPage::OnCancel)
    EVT_PROJECTLISTCTRL_SELECTION_CHANGED(CProjectInfoPage::OnProjectSelectionChanged)
END_EVENT_TABLE()

/*!
 * CProjectInfoPage constructors
 */
 
CProjectInfoPage::CProjectInfoPage() {
}

CProjectInfoPage::CProjectInfoPage(CBOINCBaseWizard* parent) {
    Create(parent);
}


/*!
 * WizardPage creator
 */
 
bool CProjectInfoPage::Create(CBOINCBaseWizard* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pDescriptionStaticCtrl = NULL;
    m_pProjectListCtrl = NULL;
    m_pProjectUrlStaticCtrl = NULL;
    m_pProjectUrlCtrl = NULL;
    bProjectListPopulated = false;
 
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPage::Create(parent, wizardBitmap);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}


/*!
 * Control creation for WizardPage
 */
 
void CProjectInfoPage::CreateControls() {
    CProjectInfoPage* itemWizardPage23 = this;

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage23->SetSizer(itemBoxSizer24);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create(itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer24->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDescriptionStaticCtrl = new wxStaticText;
    m_pDescriptionStaticCtrl->Create(itemWizardPage23, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer24->Add(m_pDescriptionStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 1, 0, 0);
    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    m_pProjectListCtrl = new CProjectListCtrl;
    m_pProjectListCtrl->Create(itemWizardPage23);
    itemFlexGridSizer3->Add(m_pProjectListCtrl, 0, wxGROW|wxRIGHT, 10);

    wxFlexGridSizer* itemFlexGridSizer11 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer11->AddGrowableRow(0);
    itemFlexGridSizer11->AddGrowableCol(0);
    itemBoxSizer24->Add(itemFlexGridSizer11, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer11->Add(itemBoxSizer22, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer14 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer14->AddGrowableCol(1);
    itemBoxSizer22->Add(itemFlexGridSizer14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT, 10);

    m_pProjectUrlStaticCtrl = new wxStaticText;
    m_pProjectUrlStaticCtrl->Create(itemWizardPage23, ID_PROJECTURLSTATICCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer14->Add(m_pProjectUrlStaticCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pProjectUrlCtrl = new wxTextCtrl;
    m_pProjectUrlCtrl->Create(itemWizardPage23, ID_PROJECTURLCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    itemFlexGridSizer14->Add(m_pProjectUrlCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    m_pProjectUrlCtrl->SetValidator(CValidateURL(&m_strProjectURL));
}


/*!
 * Gets the previous page.
 */
wxWizardPage* CProjectInfoPage::GetPrev() const {
    return PAGE_TRANSITION_BACK;
}


/*!
 * Gets the next page.
 */ 
wxWizardPage* CProjectInfoPage::GetNext() const {
    if (CHECK_CLOSINGINPROGRESS()) {
        // Cancel Event Detected
        return PAGE_TRANSITION_NEXT(ID_COMPLETIONERRORPAGE);
    } else {
        return PAGE_TRANSITION_NEXT(ID_PROJECTPROPERTIESPAGE);
    }
}


/*!
 * Should we show tooltips?
 */
 
bool CProjectInfoPage::ShowToolTips() {
    return TRUE;
}

/// Disables the validators for all controls on this page.
void CProjectInfoPage::DisableValidators() {
    m_pProjectUrlCtrl->SetValidator(wxDefaultValidator);
}

/*!
 * Get bitmap resources
 */
 
wxBitmap CProjectInfoPage::GetBitmapResource(const wxString& WXUNUSED(name)) {
    return wxNullBitmap;
}


/*!
 * Get icon resources
 */
 
wxIcon CProjectInfoPage::GetIconResource(const wxString& WXUNUSED(name)) {
    return wxNullIcon;
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanged(wxWizardEvent& event) {
    if (event.GetDirection() == false) return;

    CMainDocument* pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDescriptionStaticCtrl);
    wxASSERT(m_pProjectListCtrl);
    wxASSERT(m_pProjectUrlStaticCtrl);
    wxASSERT(m_pProjectUrlCtrl);

    m_pTitleStaticCtrl->SetLabel(_("Choose a project"));
    m_pDescriptionStaticCtrl->SetLabel(_("To choose a project, click its name or type its URL below."));
    m_pProjectUrlStaticCtrl->SetLabel(_("Project &URL:"));


    // Populate the combo box with project information
    if (!bProjectListPopulated) {
        projects_map attached_projects = pDoc->GetProjectsMap();
        pDoc->rpc.get_all_projects_list(pl);
        std::sort(pl.projects.begin(), pl.projects.end(), PROJECT_LIST_ENTRY::compare_name);
        
        for (std::vector<PROJECT_LIST_ENTRY*>::const_iterator pe = pl.projects.begin();
                        pe != pl.projects.end(); ++pe) {
            // Skip projects this client is already attached to:
            std::string project_list_url = (*pe)->url;
            canonicalize_master_url(project_list_url);
            if (attached_projects.find(project_list_url) == attached_projects.end()) {
                m_pProjectListCtrl->Append(wxString((*pe)->name.c_str(), wxConvUTF8),
                                           wxString((*pe)->url.c_str(), wxConvUTF8));
            }
        }
        bProjectListPopulated = true;
    }

    Layout();
    Fit();
    m_pProjectListCtrl->Layout();
    m_pProjectListCtrl->SetFocus();
}


/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnPageChanging(wxWizardEvent& event) {
    // Check if we are already attached to that project:
    std::string new_project_url = static_cast<const char*>(m_strProjectURL.mb_str());
    canonicalize_master_url(new_project_url);
    CMainDocument* pDoc = wxGetApp().GetDocument();
    projects_map projects = pDoc->GetProjectsMap();
    if (projects.find(new_project_url) != projects.end()) {
        wxMessageBox(_("You are already attached to this project. Please choose a different project."),
                     _("Already Attached to Project"));

        // We are already attached to that project.
        event.Veto();
    }
}


/*!
 * wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED event handler for ID_PROJECTSELECTIONCTRL
 */

void CProjectInfoPage::OnProjectSelectionChanged(ProjectListCtrlEvent& event) {
    m_pProjectUrlCtrl->SetValue(event.GetURL());
}


/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_PROJECTINFOPAGE
 */

void CProjectInfoPage::OnCancel(wxWizardEvent& event) {
    PROCESS_CANCELEVENT(event);
}
