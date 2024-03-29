// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 David Barnard, Peter Kortschack
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

#include "BOINCListCtrl.h"
#include "stdwx.h"
#include "BOINCBaseView.h"
#include "Events.h"

#include "res/sortascending.xpm"
#include "res/sortdescending.xpm"


#if USE_NATIVE_LISTCONTROL
DEFINE_EVENT_TYPE(wxEVT_DRAW_BARGRAPH)

BEGIN_EVENT_TABLE(CBOINCListCtrl, LISTCTRL_BASE)
    EVT_DRAW_BARGRAPH(CBOINCListCtrl::OnDrawBarGraph)
END_EVENT_TABLE()
#endif

BEGIN_EVENT_TABLE(MyEvtHandler, wxEvtHandler)
    EVT_PAINT(MyEvtHandler::OnPaint)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CBOINCListCtrl, LISTCTRL_BASE)


CBOINCListCtrl::CBOINCListCtrl() {}


CBOINCListCtrl::CBOINCListCtrl(
    CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags
) : LISTCTRL_BASE(
    pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), iListWindowFlags | wxLC_HRULES
) {
    m_pParentView = pView;

    m_bIsSingleSelection = (iListWindowFlags & wxLC_SINGLE_SEL) ? true : false ;
    
#if USE_NATIVE_LISTCONTROL
    m_bBarGraphEventPending = false;
#endif

    Connect(
        iListWindowID, 
        wxEVT_COMMAND_LEFT_CLICK, 
        (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) &CBOINCListCtrl::OnClick
    );

#if USE_NATIVE_LISTCONTROL
    PushEventHandler(new MyEvtHandler(this));
#else
    (GetMainWin())->PushEventHandler(new MyEvtHandler(this));
#endif

    m_SortArrows = new wxImageList(16, 16, true);
    m_SortArrows->Add( wxIcon( sortascending_xpm ) );
    m_SortArrows->Add( wxIcon( sortdescending_xpm ) );
    SetImageList(m_SortArrows, wxIMAGE_LIST_SMALL);
}


CBOINCListCtrl::~CBOINCListCtrl()
{
#if USE_NATIVE_LISTCONTROL
    PopEventHandler(true);
#else
    (GetMainWin())->PopEventHandler(true);
#endif

    if (m_SortArrows) {
        delete m_SortArrows;
    }
}

bool CBOINCListCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxASSERT(pConfig);

    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    wxString strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    wxInt32 iColumnCount = GetColumnCount();

    // Which fields are we interested in?
    wxListItem  liColumnInfo;
    liColumnInfo.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH | wxLIST_MASK_FORMAT);

    const ColumnListMap& keys = m_pParentView->GetColumnKeys();

    // Cycle through the columns recording anything interesting
    for (wxInt32 iIndex = 0; iIndex < iColumnCount; ++iIndex) {
        GetColumn(iIndex, liColumnInfo);

        // We can't use the column label as key because it might be translated
        // which renders all settings invalid. Instead we use an extra
        // vector that does the mapping between the column index and 
        pConfig->SetPath(strBaseConfigLocation + (*keys.find(iIndex)).second);

        pConfig->Write(wxT("Width"), liColumnInfo.GetWidth());
        
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
        pConfig->Write(wxT("Width"), GetColumnWidth(iIndex)); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
    }

    // Save sorting column and direction
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("SortColumn"), m_pParentView->m_iSortColumn);
    pConfig->Write(wxT("ReverseSortOrder"), m_pParentView->m_bReverseSort);

    return true;
}

bool CBOINCListCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxInt32     iTempValue = 0;

    wxASSERT(pConfig);

    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    wxString strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    wxInt32 iColumnCount = GetColumnCount();

    // Which fields are we interested in?
    wxListItem  liColumnInfo;
    liColumnInfo.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH | wxLIST_MASK_FORMAT);

    const ColumnListMap& keys = m_pParentView->GetColumnKeys();

    // Cycle through the columns recording anything interesting
    for (wxInt32 iIndex = 0; iIndex < iColumnCount; ++iIndex) {
        GetColumn(iIndex, liColumnInfo);

        // We can't use the column label as key because it might be translated
        // which renders all settings invalid. Instead we use an extra
        // vector that does the mapping between the column index and 
        pConfig->SetPath(strBaseConfigLocation + (*keys.find(iIndex)).second);

        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetWidth(iTempValue);
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
            SetColumnWidth(iIndex,iTempValue); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
        }

        pConfig->Read(wxT("Format"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetAlign((wxListColumnFormat)iTempValue);
        }

        SetColumn(iIndex, liColumnInfo);
    }

    // Restore sorting column and direction
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("ReverseSortOrder"), &iTempValue,-1);
    if (-1 != iTempValue) {
            m_pParentView->m_bReverseSort = iTempValue != 0 ? true : false;
    }
    pConfig->Read(wxT("SortColumn"), &iTempValue,-1);
    if (-1 != iTempValue) {
            m_pParentView->m_iSortColumn = iTempValue;
            m_pParentView->InitSort();
    }

    return true;
}


void CBOINCListCtrl::OnClick(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCListCtrl::OnClick - Function Begin"));

    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    wxListEvent leDeselectedEvent(wxEVT_COMMAND_LIST_ITEM_DESELECTED, m_windowId);
    leDeselectedEvent.SetEventObject(this);

    if (m_bIsSingleSelection) {
        if (GetFocusedItem() != GetFirstSelected()) {
            wxLogTrace(wxT("Function Status"), wxT("CBOINCListCtrl::OnClick - GetFocusedItem() '%ld' != GetFirstSelected() '%ld'"), GetFocusedItem(), GetFirstSelected());

            if (-1 == GetFirstSelected()) {
                wxLogTrace(wxT("Function Status"), wxT("CBOINCListCtrl::OnClick - Force Selected State"));

                long desiredstate = wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED;
                SetItemState(GetFocusedItem(), desiredstate, desiredstate);
            } else {
                m_pParentView->FireOnListSelected(leDeselectedEvent);
            }
        }
    } else {
        if (-1 == GetFirstSelected()) {
            m_pParentView->FireOnListSelected(leDeselectedEvent);
        }
    }

    event.Skip();
    wxLogTrace(wxT("Function Start/End"), wxT("CBOINCListCtrl::OnClick - Function End"));
}


wxString CBOINCListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemText(item, column);
}


int CBOINCListCtrl::OnGetItemImage(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemImage(item);
}


wxListItemAttr* CBOINCListCtrl::OnGetItemAttr(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemAttr(item);
}


void CBOINCListCtrl::DrawBarGraphs()
{
    long topItem, numItems, numVisibleItems, i, item;
    wxRect r;
    int progressColumn = m_pParentView->GetProgressColumn();
#if USE_NATIVE_LISTCONTROL
    wxClientDC dc(this);
    m_bBarGraphEventPending = false;
#else
    wxClientDC dc(GetMainWin());   // Available only in wxGenericListCtrl
#endif

    wxColour progressColorLight = GetBlendedColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), 0.6);

    wxColour progressColorDark = GetBlendedColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), 0.4);

    // This is a compromise.
    dc.SetLogicalFunction(wxAND);

    numItems = GetItemCount();
    if (numItems) {
        topItem = GetTopItem();     // Doesn't work properly for Mac Native control

        numVisibleItems = GetCountPerPage();
        ++numVisibleItems;

        if (numItems <= (topItem + numVisibleItems)) numVisibleItems = numItems - topItem;

        // Ugly hack required because GetSubItemRect isn't supported by the generic control.
#if ! USE_NATIVE_LISTCONTROL
        int w = 0, x = 0;
        if (progressColumn >= 0) {
            for (i=0; i< progressColumn; i++) {
                x += GetColumnWidth(i);
            }
            w = GetColumnWidth(progressColumn);
        }

        // Unchecked cast is only safe for wxWidgets 2.8
        wxASSERT((wxMAJOR_VERSION == 2) && (wxMINOR_VERSION == 8));
        int dummy;
        ((wxScrolledWindow*)m_mainWin)->CalcScrolledPosition(x, 0, &x, &dummy);
#endif

        for (i=0; i<numVisibleItems; i++) {
            item = topItem + i;
            GetItemRect(item, r);
#if ! USE_NATIVE_LISTCONTROL
            r.y = r.y - GetHeaderHeight() - 1;
#endif

            if (progressColumn < 0) continue;
#if USE_NATIVE_LISTCONTROL
            GetSubItemRect(item, progressColumn, r);
#else
            r.x = x;
            r.width = w;
#endif
            r.Inflate(-1, -1);
            dc.SetPen(progressColorDark);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle( r );
            r.Inflate(-1, 0);
            dc.DrawRectangle( r );

            r.width = static_cast<int>(r.width * m_pParentView->GetProgressValue(item));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(progressColorLight);
            dc.DrawRectangle( r );
        }
    }
}


// Returns a solid colour that would result from alpha blending the accent colour over the base.
wxColour CBOINCListCtrl::GetBlendedColour(const wxColour base, const wxColour accent, float blend) {

    float r = base.Red() * blend + accent.Red() * (1.0 - blend);
    float g = base.Green() * blend + accent.Green() * (1.0 - blend);
    float b = base.Blue() * blend + accent.Blue() * (1.0 - blend);
    return wxColour(static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b));
}

#if USE_NATIVE_LISTCONTROL

void MyEvtHandler::OnPaint(wxPaintEvent & event)
{
    event.Skip();
    if (m_listCtrl) {
        m_listCtrl->PostDrawBarGraphEvent();
    }
}

void CBOINCListCtrl::PostDrawBarGraphEvent() {
    if (m_bBarGraphEventPending) return;
    
    CDrawBarGraphEvent newEvent(wxEVT_DRAW_BARGRAPH, this);
    AddPendingEvent(newEvent);
    m_bBarGraphEventPending = true;
}

void CBOINCListCtrl::OnDrawBarGraph(CDrawBarGraphEvent& event) {
    DrawBarGraphs();
    event.Skip();
}

#else

void MyEvtHandler::OnPaint(wxPaintEvent & event)
{
    if (m_listCtrl) {
        (m_listCtrl->GetMainWin())->ProcessEvent(event);
        m_listCtrl->DrawBarGraphs();
    } else {
        event.Skip();
    }
}

#endif
