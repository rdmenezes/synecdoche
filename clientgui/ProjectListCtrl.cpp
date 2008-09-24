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

#include "ProjectListCtrl.h"

#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/statbmp.h>
#include <wx/log.h>

#include "BOINCGUIApp.h"
#include "hyperlink.h"

#include "res/externalweblink.xpm"

DEFINE_EVENT_TYPE(wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED)

IMPLEMENT_DYNAMIC_CLASS(CProjectListCtrl, wxScrolledWindow)
IMPLEMENT_DYNAMIC_CLASS(ProjectListCtrlEvent, wxNotifyEvent)

BEGIN_EVENT_TABLE(CProjectListCtrl, wxScrolledWindow)
    EVT_PROJECTLISTITEMCTRL_CLICKED(CProjectListCtrl::OnItemClicked)
END_EVENT_TABLE()
 
/*!
 * CProjectListCtrl constructors
 */
 
CProjectListCtrl::CProjectListCtrl() {
}

CProjectListCtrl::CProjectListCtrl(wxWindow* parent) {
    Create(parent);
}
 
/*!
 * CProjectList creator
 */
 
bool CProjectListCtrl::Create(wxWindow* parent) {
    m_pMainSizer = NULL;

    wxScrolledWindow::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER);
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

    SetMinSize(wxSize(100, 180));

    CreateControls();

    SetBackgroundColour(wxT("WHITE"));
    SetScrollRate(0, 25);

    GetSizer()->Fit(this);
    return TRUE;
}
 
/*!
 * Control creation for ProjectListCtrl
 */
 
void CProjectListCtrl::CreateControls() {    
    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    itemFlexGridSizer5->AddGrowableCol(0);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(1, 1, 0, 0);
    itemFlexGridSizer6->AddGrowableRow(0);
    itemFlexGridSizer6->AddGrowableCol(0);
    itemFlexGridSizer5->Add(itemFlexGridSizer6, 0, wxGROW|wxGROW|wxALL, 0);
    m_pMainSizer = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer6->Add(m_pMainSizer, 0, wxGROW|wxGROW|wxALL, 0);

    SetSizer(itemFlexGridSizer5);
}


/*!
 * wxEVT_PROJECTLISTITEMCTRL_CLICKED event handler for window
 */

void CProjectListCtrl::OnItemClicked(ProjectListItemCtrlEvent& event) {
    // Reset the background color back to the default
    wxWindowList::compatibility_iterator current = GetChildren().GetFirst();
    while (current) {
        wxWindow* childWin = current->GetData();
        childWin->SetBackgroundColour(wxNullColour);
        childWin->Refresh();
        current = current->GetNext();
    }

    // Set the background color of the window that threw the event to the
    //   default background color of a selected control. Then fire an event
    //   for the parent window notifing it of the new selection.
    CProjectListItemCtrl* pSelectedItem = wxDynamicCast(event.GetEventObject(), CProjectListItemCtrl);
    if (pSelectedItem) {
        pSelectedItem->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        pSelectedItem->Refresh();

        // Fire Event
        ProjectListCtrlEvent evt(
            wxEVT_PROJECTLISTCTRL_SELECTION_CHANGED,
            pSelectedItem->GetTitle(), 
            pSelectedItem->GetURL()
        );
        evt.SetEventObject(this);

        GetParent()->AddPendingEvent(evt);
    }
}


/*!
 * Append a new entry to the project list.
 */
 
bool CProjectListCtrl::Append(wxString strTitle, wxString strURL) {
    CProjectListItemCtrl* pItem = new CProjectListItemCtrl();
    pItem->Create(this);
    pItem->SetTitle(strTitle);
    pItem->SetURL(strURL);
    m_pMainSizer->Add(pItem, 0, wxEXPAND);

    FitInside();
    Layout();
    return true;
}


DEFINE_EVENT_TYPE(wxEVT_PROJECTLISTITEMCTRL_CLICKED)

IMPLEMENT_DYNAMIC_CLASS(CProjectListItemCtrl, wxPanel)
IMPLEMENT_DYNAMIC_CLASS(ProjectListItemCtrlEvent, wxNotifyEvent)

BEGIN_EVENT_TABLE(CProjectListItemCtrl, wxPanel)
    EVT_ENTER_WINDOW(CProjectListItemCtrl::OnMouseEnterLeave)
    EVT_LEAVE_WINDOW(CProjectListItemCtrl::OnMouseEnterLeave)
    EVT_LEFT_DOWN(CProjectListItemCtrl::OnMouseClick)
    EVT_LEFT_UP(CProjectListItemCtrl::OnMouseClick)
END_EVENT_TABLE()

/*!
 * CProjectListItemCtrl constructors
 */

CProjectListItemCtrl::CProjectListItemCtrl() {
}

CProjectListItemCtrl::CProjectListItemCtrl(wxWindow* parent) {
    Create(parent);
}

/*!
 * CProjectListItemCtrl creator
 */

bool CProjectListItemCtrl::Create(wxWindow* parent) {
    m_pTitleStaticCtrl = NULL;
    m_pWebsiteButtonCtrl = NULL;
    m_strTitle = wxEmptyString;
    m_strURL = wxEmptyString;
    m_bLeftButtonDownDetected = false;

    wxPanel::Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

    CreateControls();
    GetSizer()->Fit(this);
    return TRUE;
}

/*!
 * Control creation for CProjectListItemCtrl
 */

void CProjectListItemCtrl::CreateControls() {    
    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    itemFlexGridSizer8->AddGrowableRow(0);
    itemFlexGridSizer8->AddGrowableCol(0);
    itemBoxSizer7->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 1);

    m_pTitleStaticCtrl = new CProjectListItemStaticCtrl;
    m_pTitleStaticCtrl->Create(this, wxID_STATIC, wxT(""), wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer8->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_pWebsiteButtonCtrl = new wxStaticBitmap;
    m_pWebsiteButtonCtrl->Create(this, ID_WEBSITEBUTTON, wxBitmap(externalweblink_xpm), wxDefaultPosition, wxSize(12,12), wxNO_BORDER);
    m_pWebsiteButtonCtrl->Connect(ID_WEBSITEBUTTON, wxEVT_LEFT_UP, wxMouseEventHandler(CProjectListItemCtrl::OnWebsiteButtonClick), 0, this);
    itemFlexGridSizer8->Add(m_pWebsiteButtonCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    SetSizer(itemBoxSizer7);
}

/*!
 * wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
 */

void CProjectListItemCtrl::OnMouseEnterLeave(wxMouseEvent& event) {
    m_bLeftButtonDownDetected = false;
    event.Skip();
}


/*!
 * wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
 */

void CProjectListItemCtrl::OnMouseClick(wxMouseEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnMouseClick - Function Begin"));

    if (event.LeftDown()) {
        m_bLeftButtonDownDetected = true;
    } else {
        if (m_bLeftButtonDownDetected) {
            // The control that reported the down event is also
            //   the one reporting the up event, so it is a valid
            //   click event.
            wxLogTrace(wxT("Function Status"), wxT("CProjectListItemCtrl::OnMouseClick - Click Detected!"));

            ProjectListItemCtrlEvent evt(wxEVT_PROJECTLISTITEMCTRL_CLICKED, GetId());
            evt.SetEventObject(this);

            GetParent()->AddPendingEvent(evt);
        }
    }
    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnMouseClick - Function End"));
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for window
 */

void CProjectListItemCtrl::OnWebsiteButtonClick(wxMouseEvent& /*event*/) {
    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnWebsiteButtonClick - Function Begin"));

    if (!m_strURL.IsEmpty()) {
        HyperLink::ExecuteLink(m_strURL);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CProjectListItemCtrl::OnWebsiteButtonClick - Function End"));
}


bool CProjectListItemCtrl::SetTitle(wxString strTitle) {
    if (m_pTitleStaticCtrl) {
        m_pTitleStaticCtrl->SetLabel(strTitle);
    }
    m_strTitle = strTitle;
    return true;
}


bool CProjectListItemCtrl::SetURL(wxString strURL) {
    if (m_pWebsiteButtonCtrl) {
        wxString strBuffer = wxEmptyString;

        strBuffer.Printf(_("Click here to go to %s's website."), m_strTitle.c_str()
        );

        m_pWebsiteButtonCtrl->SetToolTip(strBuffer);
    }
    m_strURL = strURL;
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CProjectListItemStaticCtrl, wxStaticText)

BEGIN_EVENT_TABLE(CProjectListItemStaticCtrl, wxStaticText)
    EVT_ENTER_WINDOW(CProjectListItemStaticCtrl::OnMouseEnterLeave)
    EVT_LEAVE_WINDOW(CProjectListItemStaticCtrl::OnMouseEnterLeave)
    EVT_LEFT_DOWN(CProjectListItemStaticCtrl::OnMouseClick)
    EVT_LEFT_UP(CProjectListItemStaticCtrl::OnMouseClick)
END_EVENT_TABLE()

/*!
 * CProjectListItemStaticCtrl constructors
 */

CProjectListItemStaticCtrl::CProjectListItemStaticCtrl() {
}

CProjectListItemStaticCtrl::CProjectListItemStaticCtrl(wxWindow *parent,
            wxWindowID id, const wxString& label, const wxPoint& pos,
            const wxSize& size, long style, const wxString& name) {
    Create (parent, id, label, pos, size, style, name);
}

/*!
 * CProjectListItemStaticCtrl creator
 */
 
bool CProjectListItemStaticCtrl::Create(wxWindow *parent, wxWindowID id,
            const wxString& label, const wxPoint& pos, const wxSize& size,
            long style, const wxString& name) {
    // create static text
    bool okay = wxStaticText::Create(parent, id, label, pos, size, style, name);
    wxASSERT_MSG(okay, wxT("Failed to create wxStaticText, needed by wxHyperLink!"));

    return okay;
}


/*!
 * wxEVT_ENTER_WINDOW, wxEVT_LEAVE_WINDOW event handler for window
 */

void CProjectListItemStaticCtrl::OnMouseEnterLeave(wxMouseEvent& event) {
    CProjectListItemCtrl* pParent = wxDynamicCast(GetParent(), CProjectListItemCtrl);
    if (pParent) {
        pParent->OnMouseEnterLeave(event);
    }
}


/*!
 * wxEVT_LEFT_DOWN, wxEVT_LEFT_UP event handler for window
 */

void CProjectListItemStaticCtrl::OnMouseClick(wxMouseEvent& event) {
    CProjectListItemCtrl* pParent = wxDynamicCast(GetParent(), CProjectListItemCtrl);
    if (pParent) {
        pParent->OnMouseClick(event);
    }
}
