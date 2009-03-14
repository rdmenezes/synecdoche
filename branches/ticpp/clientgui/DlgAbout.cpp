// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 David Barnard, Peter Kortschack
// Copyright (C) 2008 University of California
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

#include "DlgAbout.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"

#include "version.h"

IMPLEMENT_DYNAMIC_CLASS(CDlgAbout, wxDialog)

CDlgAbout::CDlgAbout(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    Create(parent, id, caption, pos, size, style);
}


bool CDlgAbout::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style) {
    
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    // Need to use two-stage window creation to use extra styles.
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls(pSkinAdvanced);

    wxString title;
    title.Printf(
        _("About %s"),
        pSkinAdvanced->GetApplicationName().c_str()
    );
    SetTitle(title);

    GetSizer()->SetSizeHints(this);
    Centre();

    return true;
}


void CDlgAbout::CreateControls(CSkinAdvanced* pSkinAdvanced) {

    wxBoxSizer* verticalSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(verticalSizer);

    wxStaticText* headingText = new wxStaticText(
        this, wxID_STATIC, pSkinAdvanced->GetApplicationName(), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    headingText->SetFont(wxFont(16, wxDEFAULT, wxNORMAL, wxBOLD));
    verticalSizer->Add(headingText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    verticalSizer->Add(horizontalSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBitmap* logoBitmap = new wxStaticBitmap(this, wxID_STATIC, wxBitmap(*(pSkinAdvanced->GetApplicationLogo())));
    horizontalSizer->Add(logoBitmap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Padding between logo and copyright
    horizontalSizer->Add(20, 20);

    wxFlexGridSizer* copyrightSizer = new wxFlexGridSizer(0, 2, 0, 0);
    horizontalSizer->Add(copyrightSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* versionLabel = new wxStaticText(this, wxID_STATIC, _("Version:"));
    copyrightSizer->Add(versionLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxString version_str = wxT(SYNEC_VERSION_STRING);
    if (SYNEC_SVN_VERSION) {
        version_str << wxT(" r") << wxString(SYNEC_SVN_VERSION, wxConvUTF8);
    }
    wxStaticText* versionText = new wxStaticText(this, wxID_STATIC, version_str);
    copyrightSizer->Add(versionText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* wxVersionLabel = new wxStaticText(this, wxID_STATIC, _("wxWidgets Version:"));
    copyrightSizer->Add(wxVersionLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

    wxString wxVersionStr;
    wxVersionStr.Printf(wxT("%d.%d.%d"), wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);
    wxStaticText* wxVersionText = new wxStaticText(this, wxID_STATIC, wxVersionStr);
    copyrightSizer->Add(wxVersionText, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

    wxStaticText* copyrightLabel = new wxStaticText(this, wxID_STATIC, _("Copyright:"));
    copyrightSizer->Add(copyrightLabel, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxLEFT|wxRIGHT, 5);

    wxStaticText* copyrightText = new wxStaticText(
        this, wxID_STATIC, _("(C) 2008-2009 Synecdoche contributors.\n(C) 2003-2009 University of California, Berkeley.\nAll rights reserved."));
    copyrightSizer->Add(copyrightText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    // Unbranded about info:
    wxStaticText* itemStaticText13 = new wxStaticText(this, wxID_STATIC, wxT("Synecdoche Open Infrastructure for Distributed Computing"));
    verticalSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* SynecHyperLink = new wxHyperlinkCtrl(this, wxID_ANY, wxT("http://synecdoche.googlecode.com/"),
        wxT("http://synecdoche.googlecode.com/"));
    verticalSizer->Add(SynecHyperLink, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* BOINCHyperLink = new wxHyperlinkCtrl(this, wxID_ANY, wxT("http://boinc.berkeley.edu/"),
        wxT("http://boinc.berkeley.edu/"));
    verticalSizer->Add(BOINCHyperLink, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine* staticLine = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    verticalSizer->Add(staticLine, 0, wxGROW|wxALL, 5);

    wxButton* okayButton = new wxButton(this, wxID_OK, _("&OK"));
    okayButton->SetDefault();
    verticalSizer->Add(okayButton, 0, wxALIGN_RIGHT|wxALL, 5);
}
