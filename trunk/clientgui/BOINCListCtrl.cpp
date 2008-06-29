// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
// Copyright (C) 2005 University of California
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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"
#include "Events.h"


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
    pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), iListWindowFlags
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
}


CBOINCListCtrl::~CBOINCListCtrl()
{
}


bool CBOINCListCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT |
        wxLIST_MASK_WIDTH |
        wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

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
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;
    wxInt32     iTempValue = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH | wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

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
            wxLogTrace(wxT("Function Status"), wxT("CBOINCListCtrl::OnClick - GetFocusedItem() '%d' != GetFirstSelected() '%d'"), GetFocusedItem(), GetFirstSelected());

            if (-1 == GetFirstSelected()) {
                wxLogTrace(wxT("Function Status"), wxT("CBOINCListCtrl::OnClick - Force Selected State"));

                long desiredstate = wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED;
                SetItemState(GetFocusedItem(), desiredstate, desiredstate);
            } else {
                m_pParentView->FireOnListDeselected(leDeselectedEvent);
            }
        }
    } else {
        if (-1 == GetFirstSelected()) {
            m_pParentView->FireOnListDeselected(leDeselectedEvent);
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

#ifdef __WXMAC__
    wxColour progressColor = wxColour( 40, 170, 170, 60);
    wxColour rowStripeColor = wxColour( 0, 0, 0, 10);
#else
    wxColour progressColor = wxTheColourDatabase->Find(wxT("LIGHT BLUE"));
    wxColour rowStripeColor = wxColour( 240, 240, 240);
    dc.SetLogicalFunction(wxAND);
#endif

    numItems = GetItemCount();
    if (numItems) {
        topItem = GetTopItem();     // Doesn't work properly for Mac Native control

        numVisibleItems = GetCountPerPage();
        ++numVisibleItems;

        if (numItems <= (topItem + numVisibleItems)) numVisibleItems = numItems - topItem;

#if ! USE_NATIVE_LISTCONTROL
        int w = 0, x = 0;
        if (progressColumn >= 0) {
            for (i=0; i< progressColumn; i++) {
                x += GetColumnWidth(i);
            }
            w = GetColumnWidth(progressColumn);
        }
        int dummy;
        CalcScrolledPosition(x, 0, &x, &dummy);
#endif

        for (i=0; i<numVisibleItems; i++) {
            item = topItem + i;
            GetItemRect(item, r);
#if ! USE_NATIVE_LISTCONTROL
            r.y = r.y - GetHeaderHeight() - 1;
#endif
            if (item % 2) {
                dc.SetPen(rowStripeColor);
                dc.SetBrush(rowStripeColor);
                dc.DrawRectangle( r );
            }

            if (progressColumn < 0) continue;
#if USE_NATIVE_LISTCONTROL
            GetSubItemRect(item, progressColumn, r);
#else
            r.x = x;
            r.width = w;
#endif
            r.Inflate(-1, -1);
            dc.SetPen(progressColor);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle( r );
            r.Inflate(-1, 0);
            dc.DrawRectangle( r );

            r.width = r.width * m_pParentView->GetProgressValue(item);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(progressColor);
            dc.DrawRectangle( r );
        }
    }
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


const char *BOINC_RCSID_5cf411daa0 = "$Id: BOINCListCtrl.cpp 13804 2007-10-09 11:35:47Z fthomas $";
