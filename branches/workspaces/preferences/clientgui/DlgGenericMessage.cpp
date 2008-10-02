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

#include "DlgGenericMessage.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgGenericMessage, wxDialog)

BEGIN_EVENT_TABLE(CDlgGenericMessage, wxDialog)
END_EVENT_TABLE()

/*!
 * CDlgGenericMessage constructors
 */

CDlgGenericMessage::CDlgGenericMessage() {
}

CDlgGenericMessage::CDlgGenericMessage(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CDlgFileExit creator
 */

bool CDlgGenericMessage::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    m_DialogMessage = NULL;
    m_DialogDisableMessage = NULL;

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    return true;
}

/*!
 * Control creation for CDlgFileExit
 */

void CDlgGenericMessage::CreateControls() {    
    CDlgGenericMessage* itemDialog1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(1, 2, 0, 0);
    itemDialog1->SetSizer(itemFlexGridSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(2, 1, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_DialogMessage = new wxStaticText;
    m_DialogMessage->Create(itemDialog1, wxID_STATIC, _T(""), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer4->Add(m_DialogMessage, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer4->Add(5, 5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DialogDisableMessage = new wxCheckBox;
    m_DialogDisableMessage->Create(itemDialog1, ID_DISABLEDIALOG, _("Click here to disable displaying this message in the future."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_DialogDisableMessage->SetValue(false);
    itemFlexGridSizer4->Add(m_DialogDisableMessage, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
    itemFlexGridSizer2->Add(itemFlexGridSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_TOP|wxALL, 5);

    wxButton* itemButton9 = new wxButton;
    itemButton9->Create(itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0);
    itemButton9->SetDefault();
    itemFlexGridSizer8->Add(itemButton9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton;
    itemButton10->Create(itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer8->Add(itemButton10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/*!
 * Should we show tooltips?
 */

bool CDlgGenericMessage::ShowToolTips() {
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDlgGenericMessage::GetBitmapResource(const wxString& name) {
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CDlgGenericMessage::GetIconResource(const wxString& name) {
    wxUnusedVar(name);
    return wxNullIcon;
}
