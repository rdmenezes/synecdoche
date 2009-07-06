// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#include "DlgMsgFilter.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"

// Helper functions:
namespace {
    void CheckAll(wxCheckListBox* checkListBox, bool check) {
        checkListBox->Freeze();
        for (unsigned int i = 0; i < checkListBox->GetCount(); ++i) {
            checkListBox->Check(i, check);
        }
        checkListBox->Thaw();
    }
    
    void AddAndCheck(wxCheckListBox* checkListBox, const string_set& data) {
        for (string_set::const_iterator p = data.begin(); p != data.end(); ++p) {
            int index = checkListBox->Append(wxString((*p).c_str(), wxConvUTF8));
            checkListBox->Check(index);
        }
    }
    
    void CheckSelected(wxCheckListBox* checkListBox, const string_set& selected) {
        for (unsigned int i = 0; i < checkListBox->GetCount(); ++i) {
            std::string value = std::string(static_cast<const char*>(checkListBox->GetString(i).mb_str()));
            checkListBox->Check(i, (selected.find(value) != selected.end()));
        }
    }
}

DlgMsgFilter::DlgMsgFilter(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style) {
    SetMinSize(wxSize(377, 337));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxNotebook* notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

    // Projects page:
    wxPanel* projectsPage = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* projectsPageSizer = new wxBoxSizer(wxVERTICAL);

    m_projectsList = new wxCheckListBox(projectsPage, wxID_ANY);
    projectsPageSizer->Add(m_projectsList, 6, wxALL | wxEXPAND, 5);

    wxBoxSizer* projectsBtnSizerHelper = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* projectsBtnSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnProjectsSelAll = new wxButton(projectsPage, wxID_ANY, _("Select &all"), wxDefaultPosition, wxDefaultSize, 0);
    projectsBtnSizer->Add(btnProjectsSelAll, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* btnProjectsSelNone = new wxButton(projectsPage, wxID_ANY, _("Select &none"), wxDefaultPosition, wxDefaultSize, 0);
    projectsBtnSizer->Add(btnProjectsSelNone, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    projectsBtnSizerHelper->Add(projectsBtnSizer, 1, wxALIGN_RIGHT, 5);

    projectsPageSizer->Add(projectsBtnSizerHelper, 1, wxEXPAND, 5);

    projectsPage->SetSizer(projectsPageSizer);
    projectsPage->Layout();
    projectsPageSizer->Fit(projectsPage);
    notebook->AddPage(projectsPage, _("Projects"), true);

    // Debug flags page:
    wxPanel* debugFlagsPage = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* debugFlagsPageSizer = new wxBoxSizer(wxVERTICAL);

    m_debugFlagsList = new wxCheckListBox(debugFlagsPage, wxID_ANY);
    debugFlagsPageSizer->Add(m_debugFlagsList, 6, wxALL | wxEXPAND, 5);

    wxBoxSizer* debugFlagsBtnSizerHelper = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* debugFlagsBtnSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnDebugFlagsSelAll = new wxButton(debugFlagsPage, wxID_ANY, _("Select &all"), wxDefaultPosition, wxDefaultSize, 0);
    debugFlagsBtnSizer->Add(btnDebugFlagsSelAll, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);

    wxButton* btnDebugFlagsSelNone = new wxButton(debugFlagsPage, wxID_ANY, _("Select &none"), wxDefaultPosition, wxDefaultSize, 0);
    debugFlagsBtnSizer->Add(btnDebugFlagsSelNone, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    debugFlagsBtnSizerHelper->Add(debugFlagsBtnSizer, 1, wxALIGN_RIGHT, 5);

    debugFlagsPageSizer->Add(debugFlagsBtnSizerHelper, 1, wxEXPAND, 5);

    debugFlagsPage->SetSizer(debugFlagsPageSizer);
    debugFlagsPage->Layout();
    debugFlagsPageSizer->Fit(debugFlagsPage);
    notebook->AddPage(debugFlagsPage, _("Debug flags"), false);

    // Other settings page:
    wxPanel* otherSettingsPage = new wxPanel(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* otherSettingsPageSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* visibleMsgSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* visibleMsgsLabel = new wxStaticText(otherSettingsPage, wxID_ANY, _("Number of visible messages:"), wxDefaultPosition, wxDefaultSize, 0);
    visibleMsgSizer->Add(visibleMsgsLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_visibleMessages = new wxSpinCtrl(otherSettingsPage, wxID_ANY, wxT("1000"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000, 1000);
    visibleMsgSizer->Add(m_visibleMessages, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    otherSettingsPageSizer->Add(visibleMsgSizer, 0, wxEXPAND, 5);

    otherSettingsPage->SetSizer(otherSettingsPageSizer);
    otherSettingsPage->Layout();
    otherSettingsPageSizer->Fit(otherSettingsPage);
    notebook->AddPage(otherSettingsPage, _("Other settings"), false);

    mainSizer->Add(notebook, 7, wxALL | wxEXPAND, 5);

    // Main Buttons:
    wxBoxSizer* mainBtnSizerHelper = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainBtnSizer = new wxBoxSizer(wxHORIZONTAL);

    wxButton* btnOK = new wxButton(this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
    btnOK->SetDefault(); 
    mainBtnSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* btnCancel = new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    mainBtnSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    mainBtnSizerHelper->Add(mainBtnSizer, 1, wxALIGN_RIGHT, 5);

    mainSizer->Add(mainBtnSizerHelper, 1, wxEXPAND | wxRIGHT, 5);

    SetSizer(mainSizer);
    Layout();

    // Connect events:
    btnProjectsSelAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                               wxCommandEventHandler(DlgMsgFilter::OnProjectsSelectAll),
                               0, this);
    btnProjectsSelNone->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                                wxCommandEventHandler(DlgMsgFilter::OnProjectsSelectNone),
                                0, this);
    btnDebugFlagsSelAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                                 wxCommandEventHandler(DlgMsgFilter::OnDebugFlagsSelectAll),
                                 0, this);
    btnDebugFlagsSelNone->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                                  wxCommandEventHandler(DlgMsgFilter::OnDebugFlagsSelectNone),
                                  0, this);
    btnOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                   wxCommandEventHandler(DlgMsgFilter::OnOK), 0, this);

    // Fill the projects list and debug flags list and initialize all settings with their
    // default values. m_filterData will contain the default values at this place as it
    // could not have been changed yet.
    Freeze();
    AddAndCheck(m_projectsList, m_filterData.GetSelectedProjects());
    AddAndCheck(m_debugFlagsList, m_filterData.GetSelectedDebugFlags());
    m_visibleMessages->SetValue(m_filterData.GetNumVisibleMsg());
    Thaw();
}

DlgMsgFilter::~DlgMsgFilter(){
}

const MsgFilterData& DlgMsgFilter::GetFilterData() const {
    return m_filterData;
}

void DlgMsgFilter::SetFilterData(const MsgFilterData& filterData) {
    m_filterData = filterData;
    
    // Update UI:
    Freeze();
    CheckSelected(m_projectsList, m_filterData.GetSelectedProjects());
    CheckSelected(m_debugFlagsList, m_filterData.GetSelectedDebugFlags());
    m_visibleMessages->SetValue(m_filterData.GetNumVisibleMsg());
    Thaw();
}

void DlgMsgFilter::OnProjectsSelectAll(wxCommandEvent& /* event */) {
    CheckAll(m_projectsList, true);
}

void DlgMsgFilter::OnProjectsSelectNone(wxCommandEvent& /* event*/ ) {
    CheckAll(m_projectsList, false);
}

void DlgMsgFilter::OnDebugFlagsSelectAll(wxCommandEvent& /* event */) {
    CheckAll(m_debugFlagsList, true);
}

void DlgMsgFilter::OnDebugFlagsSelectNone(wxCommandEvent& /* event */) {
    CheckAll(m_debugFlagsList, false);
}

void DlgMsgFilter::OnOK(wxCommandEvent& event) {
    m_filterData.Clear();
    
    // Read selected projects:
    for (unsigned int i = 0; i < m_projectsList->GetCount(); ++i) {
        if (m_projectsList->IsChecked(i)) {
            m_filterData.SelectProject(
                    static_cast<const char*>(m_projectsList->GetString(i).mb_str()));
        }
    }
    
    // Read selected debug flags:
    for (unsigned int i = 0; i < m_debugFlagsList->GetCount(); ++i) {
        if (m_debugFlagsList->IsChecked(i)) {
            m_filterData.SelectDebugFlag(
                    static_cast<const char*>(m_debugFlagsList->GetString(i).mb_str()));
        }
    }

    // Read other settings:
    m_filterData.SetNumVisibleMsg(static_cast<unsigned int>(m_visibleMessages->GetValue()));

    event.Skip(); // Allow the dialog to handle the OK event.
}
