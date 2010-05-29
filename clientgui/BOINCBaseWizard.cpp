// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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

/// \file
/// Base class for manager's wizards

#include "BOINCBaseWizard.h"

#include "stdwx.h"

IMPLEMENT_DYNAMIC_CLASS(CBOINCBaseWizard, wxWizard)

BEGIN_EVENT_TABLE(CBOINCBaseWizard, wxWizard)
    EVT_BUTTON(wxID_BACKWARD, CBOINCBaseWizard::OnBackOrNext)
    EVT_BUTTON(wxID_FORWARD, CBOINCBaseWizard::OnBackOrNext)
END_EVENT_TABLE()
 
CBOINCBaseWizard::CBOINCBaseWizard() : wxWizard() {
    close_when_completed = false;
}

CBOINCBaseWizard::CBOINCBaseWizard(wxWindow *parent, int id, const wxString& title, const wxBitmap& bitmap, const wxPoint& pos, long style) : wxWizard(parent, id, title, bitmap, pos, style) {
    close_when_completed = false;
}

bool CBOINCBaseWizard::Create(wxWindow *parent, wxWindowID id, const wxString& title, const wxBitmap& bitmap, const wxPoint& pos) {
    project_config.clear();
    account_in.clear();
    account_out.clear();
    account_created_successfully = false;
    attached_to_project_successfully = false;
    project_url = wxEmptyString;
    project_authenticator = wxEmptyString;
    m_bCancelInProgress = false;
    return wxWizard::Create(parent, id, title, bitmap, pos);
}

/// Determine if the wizard page has a previous page.
wxWizardPage* CBOINCBaseWizard::PopPageTransition() {
    return _PopPageTransition();
}

wxWizardPage* CBOINCBaseWizard::_PopPageTransition() {
    return NULL;
}

/// Remove the page transition to the stack
wxWizardPage* CBOINCBaseWizard::PushPageTransition(wxWizardPage* pCurrentPage, unsigned long ulPageID) {
    return _PushPageTransition(pCurrentPage, ulPageID);
}

wxWizardPage* CBOINCBaseWizard::_PushPageTransition(wxWizardPage* WXUNUSED(pCurrentPage), unsigned long WXUNUSED(ulPageID)) {
    return NULL;
}

/// Process Cancel Event
bool CBOINCBaseWizard::IsCancelInProgress() const {
    return m_bCancelInProgress;
}

void CBOINCBaseWizard::SetCancelInProgress(const bool value) {
    m_bCancelInProgress = value;
}

void CBOINCBaseWizard::ProcessCancelEvent(wxWizardEvent& event) {
    _ProcessCancelEvent(event);
}

void CBOINCBaseWizard::_ProcessCancelEvent(wxWizardEvent& WXUNUSED(event)) {
}

/*!
 * Button Controls
 */

wxButton* CBOINCBaseWizard::GetNextButton() const {
    wxButton* btnNext = static_cast<wxButton*>(wxWindow::FindWindowById(wxID_FORWARD, this));
    wxASSERT(btnNext);
    return btnNext;
}

void CBOINCBaseWizard::SimulateNextButton() {
    wxButton* btnNext = GetNextButton();
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, btnNext->GetId());
    event.SetEventObject(btnNext);
    AddPendingEvent(event);
}

void CBOINCBaseWizard::EnableNextButton() {
    GetNextButton()->Enable();
}

void CBOINCBaseWizard::DisableNextButton() {
    GetNextButton()->Disable();
}

wxButton* CBOINCBaseWizard::GetBackButton() const {
    wxButton* btnBack = static_cast<wxButton*>(wxWindow::FindWindowById(wxID_BACKWARD, this));
    wxASSERT(btnBack);
    return btnBack;
}

void CBOINCBaseWizard::SimulateBackButton() {
    wxButton* btnBack = GetNextButton();
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, btnBack->GetId());
    event.SetEventObject(btnBack);
    AddPendingEvent(event);
}
 
void CBOINCBaseWizard::EnableBackButton() {
    GetBackButton()->Enable();
}

void CBOINCBaseWizard::DisableBackButton() {
    GetBackButton()->Disable();
}

/// Check if account creation was successful or not.
///
/// \return True if account creation was successful, false otherwise.
bool CBOINCBaseWizard::GetAccountCreatedSuccessfully() const {
    return account_created_successfully;
}

/// Set or reset the success flag for account creation.
///
/// \param[in] value The new value for the success flag for account creation.
void CBOINCBaseWizard::SetAccountCreatedSuccessfully(const bool value) {
    account_created_successfully = value;
}

/// Check if attaching to the project was successful.
///
/// \return True if the client successfully attached to the project, false otherwise.
bool CBOINCBaseWizard::GetAttachedToProjectSuccessfully() const {
    return attached_to_project_successfully;
}

/// Set or reset the success flag for project attachment.
///
/// \param[in] value The new value for the success flag for project attachment.
void CBOINCBaseWizard::SetAttachedToProjectSuccessfully(const bool value) {
    attached_to_project_successfully = value;
}

/// Get the URL for the selected project.
///
/// \return The URL for the selected project.
wxString CBOINCBaseWizard::GetProjectURL() const {
    return project_url;
}

/// Set the project URL.
///
/// \param[in] value The new value for the project URL.
void CBOINCBaseWizard::SetProjectURL(const wxString& value) {
    project_url = value;
}

/// Get the name of the selected project.
///
/// \return The name of the selected project.
wxString CBOINCBaseWizard::GetProjectName() const {
    return project_name;
}

/// Get the authenticator used for the current project.
///
/// \return The authenticator used for the current project.
wxString CBOINCBaseWizard::GetProjectAuthenticator() const {
    return project_authenticator;
}

/// Set the authenticator used for the current project to a new value.
///
/// \param[in] value The new value for the authenticator used for the current project.
void CBOINCBaseWizard::SetProjectAuthenticator(const wxString &value) {
    project_authenticator = value;
}

/// Get the 'close when completed' flag.
///
/// \return True when the 'close when completed' flag is set, false otherwise.
bool CBOINCBaseWizard::GetCloseWhenCompleted() const {
    return close_when_completed;
}

/// Set or reset the 'close when completed' flag.
///
/// \param[in] value The new value for the 'close when completed' flag.
void CBOINCBaseWizard::SetCloseWhenCompleted(const bool value) {
    close_when_completed = value;
}

/// Return a pointer to the project preferences.
///
/// \return a pointer to the project preferences.
PROJECT_CONFIG* CBOINCBaseWizard::GetProjectConfig() {
    return &project_config;
}

/// Return a pointer to the 'account in' object.
///
/// \return a pointer to the 'account in' object.
ACCOUNT_IN* CBOINCBaseWizard::GetAccountIn() {
    return &account_in;
}

/// Return a pointer to the 'account out' object.
///
/// \return a pointer to the 'account out' object.
ACCOUNT_OUT* CBOINCBaseWizard::GetAccountOut() {
    return &account_out;
}

/// Handle clicks on the 'Back' or 'Next' button.
/// Normally this is done by wxWizard but the standard behaviour is to call
/// the validators of the current page no matter which button is clicked.
/// This means that the user has to enter valid data before he is able to
/// click any of the two buttons. This is undesired if the user wants to
/// go back to the previous page so this version of the event handler just
/// skips the validation of the page's data in case the back button was clicked.
///
/// \param[in] event The event generated by clicking the 'Back' or 'Next' button.
void CBOINCBaseWizard::OnBackOrNext(wxCommandEvent& event) {
    int button_id = event.GetId();
    wxASSERT_MSG((button_id == wxID_FORWARD) || (button_id == wxID_BACKWARD),
                  wxT("unknown button"));

    wxWizardPage* cur_page = GetCurrentPage();
    wxCHECK_RET(cur_page, _T("should have a valid current page"));

    // Ask the current page first: notice that we do it before calling
    // GetNext/Prev() because the data transfered from the controls of the page
    // may change the value returned by these methods.
    // Only do this in case of the next button. This differs from standard
    // wxWizard behaviour!
    if (button_id == wxID_FORWARD) {
        if ((!cur_page->Validate()) || (!cur_page->TransferDataFromWindow())) {
            // the page data is incorrect, don't do anything
            return;
        }
    }

    wxWizardPage* next_page;
    bool forward = (button_id == wxID_FORWARD);
    if (forward) {
        next_page = cur_page->GetNext();
    } else { // back
        next_page = cur_page->GetPrev();

        wxASSERT_MSG(next_page, wxT("\"<Back\" button should have been disabled"));
    }

    // just pass to the new page (or maybe not - but we don't care here)
    (void)ShowPage(next_page, forward);
}
