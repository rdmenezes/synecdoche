// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#include "stdwx.h"
#include "PrefFrame.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"
#include "hyperlink.h"
#include "Events.h"
#include "PrefLocationManager.h"

IMPLEMENT_DYNAMIC_CLASS(PrefFrame, wxDialog)

BEGIN_EVENT_TABLE(PrefFrame, wxDialog)
    EVT_BUTTON(wxID_OK, PrefFrame::OnOK)
    EVT_BUTTON(wxID_HELP, PrefFrame::OnHelp)
    EVT_BUTTON(ID_LOCATIONMANAGER, PrefFrame::OnLocationManager)
END_EVENT_TABLE()

PrefFrame::PrefFrame(wxWindow* parent) : wxDialog(parent, ID_ANYDIALOG, _("Preferences"),
    wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    SetMinSize(wxSize(500, 400));

    RestoreState();

    wxBoxSizer* vShape = new wxBoxSizer(wxVERTICAL);

    // Venue controls at the top.
    wxBoxSizer* headerRow = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* locationText = new wxStaticText(this, wxID_ANY, _("Location:"));


    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    //pDoc->rpc.get_venue_list(m_venues);

    wxChoice* locationChoice = new wxChoice(this, wxID_ANY);
    //std::vector<VENUE>::iterator i = m_venues.begin();
    //while (i != m_venues.end()) {
    //    locationChoice->AppendString((*i).get_venue_description());
    //    i++;
    //}

    locationChoice->SetSelection(0);

    wxButton* locationManager = new wxButton(this, ID_LOCATIONMANAGER, _("Manage locations..."));

    headerRow->Add(locationText, 0, wxALL | wxCENTER, PREF_DLG_MARGIN);
    headerRow->Add(locationChoice, 1, wxALL | wxCENTER, PREF_DLG_MARGIN);
    headerRow->Add(locationManager, 0, wxALL | wxCENTER, PREF_DLG_MARGIN);

    // Treeview preferences
    m_treeBook = new PrefTreeBook(this);

    // Dialog buttons - right aligned.
    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);

    m_buttonOkay = new wxButton(this, wxID_OK);
    m_buttonCancel = new wxButton(this, wxID_CANCEL);
    m_buttonHelp = new wxButton(this, wxID_HELP);

    buttonRow->AddStretchSpacer();
    buttonRow->Add(m_buttonOkay, 0, wxALL, PREF_DLG_MARGIN);
    buttonRow->Add(m_buttonCancel, 0, wxALL, PREF_DLG_MARGIN);
    buttonRow->Add(m_buttonHelp, 0, wxALL, PREF_DLG_MARGIN);
    
    vShape->Add(headerRow, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, PREF_DLG_MARGIN);
    vShape->Add(m_treeBook, 1, wxLEFT | wxRIGHT | wxEXPAND, PREF_DLG_MARGIN);
    vShape->Add(buttonRow, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, PREF_DLG_MARGIN);

    SetSizer(vShape);
}


PrefFrame::~PrefFrame() {
    SaveState();
}


bool PrefFrame::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/PrefFrame/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);

    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("Width"), GetSize().GetWidth());
    pConfig->Write(wxT("Height"), GetSize().GetHeight());

    return true;
}


bool PrefFrame::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/PrefFrame/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(false);
    int             w, h;

    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("Width"), &w, 600);
    pConfig->Read(wxT("Height"), &h, 500);
    this->SetSize(w, h);

    return true;
}

// OK button handler.
void PrefFrame::OnOK(wxCommandEvent& ev) {

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


// Help button handler.
void PrefFrame::OnHelp(wxCommandEvent& ev) {
    //wxString url = wxGetApp().GetSkinManager()->GetAdvanced()->GetCompanyWebsite();
    //url += wxT("/prefs.php");//this seems not the right url, but which instead ?
    //wxHyperLink::ExecuteLink(url);
    ev.Skip();
}


void PrefFrame::OnLocationManager(wxCommandEvent& WXUNUSED(event)) {
    PrefLocationManager dlg(this);
    dlg.ShowModal();
}
