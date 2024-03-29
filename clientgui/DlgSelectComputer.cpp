// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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
/// Class implementing the dialog for selecting a different computer in the Manager.

#include "DlgSelectComputer.h"

#include "stdwx.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgSelectComputer, wxDialog)

BEGIN_EVENT_TABLE(CDlgSelectComputer, wxDialog )
    EVT_TEXT(ID_SELECTCOMPUTERNAME, CDlgSelectComputer::OnComputerNameUpdated)
END_EVENT_TABLE()


CDlgSelectComputer::CDlgSelectComputer() : m_ComputerNameCtrl(0), m_ComputerPasswordCtrl(0)
{
}

CDlgSelectComputer::CDlgSelectComputer(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) : m_ComputerNameCtrl(0), m_ComputerPasswordCtrl(0)
{
    Create(parent, id, caption, pos, size, style);
}

bool CDlgSelectComputer::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxString strCaption = caption;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Select Computer"), pSkinAdvanced->GetApplicationName().c_str());
    }

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, strCaption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    return TRUE;
}

/// Creates the controls and sizers.
void CDlgSelectComputer::CreateControls()
{    
    CDlgSelectComputer* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText;
    itemStaticText6->Create( itemDialog1, wxID_STATIC, _("Host name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxString* m_ComputerNameCtrlStrings = NULL;
    m_ComputerNameCtrl = new wxComboBox;
    m_ComputerNameCtrl->Create( itemDialog1, ID_SELECTCOMPUTERNAME, _T(""), wxDefaultPosition, wxSize(250, -1), 0, m_ComputerNameCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer5->Add(m_ComputerNameCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText;
    itemStaticText8->Create( itemDialog1, wxID_STATIC, _("Password:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ComputerPasswordCtrl = new wxTextCtrl;
    m_ComputerPasswordCtrl->Create( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition, wxSize(250, -1), wxTE_PASSWORD );
    itemFlexGridSizer5->Add(m_ComputerPasswordCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton11 = new wxButton;
    itemButton11->Create( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton11->SetDefault();
    itemBoxSizer10->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton;
    itemButton12->Create( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    m_ComputerNameCtrl->SetValidator(wxGenericValidator(&m_strComputerName));
    m_ComputerPasswordCtrl->SetValidator(wxGenericValidator(&m_strComputerPassword));
}

/// Set the list of most recently used computers.
///
/// \param[in] mru_list An array containing the names of all computers
///                     that were used recently.
void CDlgSelectComputer::SetMRUList(const wxArrayString& mru_list)
{
    m_ComputerNameCtrl->Clear();
    for (size_t i = 0; i < mru_list.Count(); ++i) {
        m_ComputerNameCtrl->Append(mru_list.Item(i));
    }
}

/// Should we show tooltips?
bool CDlgSelectComputer::ShowToolTips(){
    return TRUE;
}

/// Get bitmap resources
wxBitmap CDlgSelectComputer::GetBitmapResource(const wxString& WXUNUSED(name))
{
    return wxNullBitmap;
}

/// Get icon resources
wxIcon CDlgSelectComputer::GetIconResource(const wxString& WXUNUSED(name))
{
    return wxNullIcon;
}

/// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SELECTCOMPUTERNAME
void CDlgSelectComputer::OnComputerNameUpdated(wxCommandEvent& WXUNUSED(event))
{
    CMainDocument* pDoc        = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    wxString name = m_ComputerNameCtrl->GetValue();
    if (pDoc->IsComputerNameLocal(name)) {
        std::string password;
        try {
            password = read_gui_rpc_password();
        } catch (...) {
            // Ignore any errors here and set an empty password.
            // This will happen if the manager does not find the
            // GUI-RPC-password file in its working directory.
        }
        wxString wxStrPassword = wxString(password.c_str(), wxConvUTF8);
        m_ComputerPasswordCtrl->SetValue(wxStrPassword);
    }
}
