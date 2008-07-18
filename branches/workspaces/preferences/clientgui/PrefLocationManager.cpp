// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "stdwx.h"
#include "PrefLocationManager.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
//#include "SkinManager.h"
//#include "hyperlink.h"
#include "Events.h"

IMPLEMENT_DYNAMIC_CLASS(PrefLocationManager, wxDialog)

BEGIN_EVENT_TABLE(PrefLocationManager, wxDialog)
    EVT_BUTTON(wxID_OK, PrefLocationManager::OnOK)
END_EVENT_TABLE()

PrefLocationManager::PrefLocationManager(wxWindow* parent) : wxDialog(parent, ID_ANYDIALOG, _("Location Manager"),
    wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxBoxSizer* vShape = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* contentRow = new wxBoxSizer(wxHORIZONTAL);

    wxListBox* list = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(150, 200));
    
    //std::vector<VENUE> m_venues;
    //CMainDocument* pDoc = wxGetApp().GetDocument();
    //wxASSERT(pDoc);
    //pDoc->rpc.get_venue_list(m_venues);

    //std::vector<VENUE>::iterator i = m_venues.begin();
    //while (i != m_venues.end()) {
    //    list->Append((*i).get_venue_description());
    //    i++;
    //}

    wxBoxSizer* tasks = new wxBoxSizer(wxVERTICAL);
    wxButton* add = new wxButton(this, wxID_ANY, _("Add"));
    wxButton* remove  = new wxButton(this, wxID_ANY, _("Remove"));
    wxButton* rename  = new wxButton(this, wxID_ANY, _("Rename"));

    tasks->Add(add, 0, wxALL, PREF_DLG_MARGIN);
    tasks->Add(remove, 0, wxALL, PREF_DLG_MARGIN);
    tasks->Add(rename, 0, wxALL, PREF_DLG_MARGIN);

    contentRow->Add(list, 1, wxALL | wxEXPAND, PREF_DLG_MARGIN);
    contentRow->Add(tasks, 0, wxALL | wxEXPAND, 0);

    // Dialog buttons - right aligned.
    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);

    m_buttonOkay = new wxButton(this, wxID_OK, _("Okay"));
    m_buttonCancel = new wxButton(this, wxID_CANCEL, _("Cancel"));

    buttonRow->AddStretchSpacer();
    buttonRow->Add(m_buttonOkay, 0, wxALL, PREF_DLG_MARGIN);
    buttonRow->Add(m_buttonCancel, 0, wxALL, PREF_DLG_MARGIN);
    
    vShape->Add(contentRow, 1, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, PREF_DLG_MARGIN);
    vShape->Add(buttonRow, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, PREF_DLG_MARGIN);

    SetSizerAndFit(vShape);

    RestoreState();
}


PrefLocationManager::~PrefLocationManager() {
    SaveState();
}


bool PrefLocationManager::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/PrefLocationManager/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);

    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());

    return true;
}


bool PrefLocationManager::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/PrefLocationManager/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);
    int				w, h;

    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("Width"), &w, -1);
    pConfig->Read(wxT("Height"), &h, -1);
    this->SetSize(w, h);	

    return true;
}

// OK button handler.
void PrefLocationManager::OnOK(wxCommandEvent& ev) {

    if (Validate() && TransferDataFromWindow())
    {
        m_treeBook->SavePreferences();

        if (IsModal()) {
            EndModal(wxID_OK);
        } else {
            SetReturnCode(wxID_OK);
            this->Show(false);
        }
    }
    ev.Skip();
}
