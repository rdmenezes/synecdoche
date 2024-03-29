// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Nicolas Alvarez
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

#include "sg_ImageButton.h"

#include "stdwx.h"
#include "common/wxFlatNotebook.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_ImageLoader.h"
#include "sg_ViewTabPage.h"

BEGIN_EVENT_TABLE(CImageButton, wxPanel)
    EVT_PAINT(CImageButton::OnPaint)
    EVT_LEFT_UP(CImageButton::OnLeftUp)
    EVT_ERASE_BACKGROUND(CImageButton::OnEraseBackground)
END_EVENT_TABLE()

CImageButton::CImageButton(wxWindow* parent, wxBitmap bg, wxPoint coord, wxSize size, bool enableShowGraphics, int initStatus) : wxPanel(parent, wxID_ANY, coord, size, wxNO_BORDER)
{
    btnBG = bg;
    m_enableShowGraphics = enableShowGraphics;
    status = initStatus;
    SetToolTip();
}

CImageButton::~CImageButton() {
}

void CImageButton::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    dc.SetTextForeground(wxColour(*wxWHITE));
#ifdef __WXMAC__
    dc.SetFont(wxFont(10,74,90,90,0, wxT("Arial")));
#else
    dc.SetFont(wxFont(7,74,90,90,0, wxT("Arial")));
#endif
    int x, y;
    GetSize(&x, &y);
    wxCoord width, height;
    dc.GetTextExtent(GetStatusText(), &width, &height);
    dc.DrawText(GetStatusText(), wxPoint(7, y-height-4));
    // display show graphics text (if available)
    if(m_enableShowGraphics) {
        wxString text = wxString(_T("Graphics Available"));
        dc.GetTextExtent(text, &width, &height);
        dc.DrawText(text, wxPoint(x-width-7,y-height-4));
    }
}
void CImageButton::SetImage(wxBitmap bg)
{
    btnBG = bg;
}
void CImageButton::SetStatus(int newStatus) {
    status = newStatus;
}
int CImageButton::GetStatus() {
    return status;
}

wxString CImageButton::GetStatusText() {
    switch (status) {
    case TAB_STATUS_RUNNING:
        return wxString(_("Running"));
    case TAB_STATUS_PREEMPTED:
        return wxString(_("Paused: Other work running"));
    case TAB_STATUS_PAUSED_USER_REQ:
        return wxString(_("Paused: User initiatied.  Click 'Resume' to continue"));
    case TAB_STATUS_PAUSED_USER_ACTIVE:
        return wxString(_("Paused: User active"));
    case TAB_STATUS_PAUSED_POWER:
        return wxString(_("Paused: Computer on battery"));
    case TAB_STATUS_PAUSED_TIME_OF_DAY:
        return wxString(_("Paused: Time of Day"));
    case TAB_STATUS_PAUSED_BENCHMARKS:
        return wxString(_("Paused: Benchmarks running"));
    case TAB_STATUS_PAUSED:
        return wxString(_("Paused"));
    default:
        return wxEmptyString;
    }
}
void CImageButton::OnEraseBackground(wxEraseEvent& event) {

    event.Skip(false);
    wxDC *dc = event.GetDC();
    dc->SetBackground(wxBrush(this->GetBackgroundColour(), wxSOLID));
    dc->Clear();
    if (btnBG.Ok()) {
        dc->DrawBitmap(btnBG, 0, 0);
    }
}
void CImageButton::OnLeftUp(wxMouseEvent& WXUNUSED(event))
{
    CViewTabPage* pView = wxDynamicCast(GetParent(), CViewTabPage);
    wxASSERT(pView);
    pView->OnImageButton();
}

void CImageButton::SetToolTip() {
    if ( m_enableShowGraphics ) {
        wxPanel::SetToolTip(new wxToolTip(_("Click to show project graphics")));
    } else {
        wxPanel::SetToolTip(NULL);
    }
}

void CImageButton::SetEnableShowGraphics(bool show) {
    if ( show != m_enableShowGraphics ) {
        m_enableShowGraphics = show;
        SetToolTip();
        Refresh();
    }
}

