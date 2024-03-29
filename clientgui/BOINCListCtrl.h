// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#ifndef _BOINCLISTCTRL_H_
#define _BOINCLISTCTRL_H_

#ifdef __WXMSW__
#define USE_NATIVE_LISTCONTROL 1
#else
#define USE_NATIVE_LISTCONTROL 0
#endif

#include <wx/listctrl.h>
#if USE_NATIVE_LISTCONTROL
#define LISTCTRL_BASE wxListCtrl
#else
#define LISTCTRL_BASE wxGenericListCtrl
#include <wx/generic/listctrl.h>
#endif

#include <wx/config.h>

class wxScrolledWindow;

class CBOINCBaseView;
class CDrawBarGraphEvent;

class CBOINCListCtrl : public LISTCTRL_BASE {
    DECLARE_DYNAMIC_CLASS(CBOINCListCtrl)

public:
    CBOINCListCtrl();
    CBOINCListCtrl(CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags);

    ~CBOINCListCtrl();

    virtual bool            OnSaveState(wxConfigBase* pConfig);
    virtual bool            OnRestoreState(wxConfigBase* pConfig);

    long                    GetFocusedItem() { return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED); }
    long                    GetFirstSelected() { return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED); }

    bool                    m_bIsSingleSelection;

    /// TODO: Move CBOINCListCtrl::GetBlendedColour to a better place so it can used by different classes.
    static wxColour         GetBlendedColour(const wxColour base, const wxColour accent, float blend);

private:
    virtual void            OnClick(wxCommandEvent& event);

    virtual wxString        OnGetItemText(long item, long column) const;
    virtual int             OnGetItemImage(long item) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;

    CBOINCBaseView*         m_pParentView;
    wxImageList *           m_SortArrows;

#if USE_NATIVE_LISTCONTROL
public:
   void                     PostDrawBarGraphEvent();
private:
    void                    OnDrawBarGraph(CDrawBarGraphEvent& event);
    void                    DrawBarGraphs(void);
    
    bool                    m_bBarGraphEventPending;

    DECLARE_EVENT_TABLE()
#else
 public:
    void                    DrawBarGraphs(void);
    wxScrolledWindow*       GetMainWin(void) { return (wxScrolledWindow*) m_mainWin; }
    wxCoord                 GetHeaderHeight(void) { return m_headerHeight; }
#endif
};

class CDrawBarGraphEvent : public wxEvent
{
public:
    CDrawBarGraphEvent(wxEventType evtType, CBOINCListCtrl* myCtrl)
        : wxEvent(-1, evtType)
        {
            SetEventObject(myCtrl);
        }

    virtual wxEvent *       Clone() const { return new CDrawBarGraphEvent(*this); }
};

DECLARE_EVENT_TYPE( wxEVT_DRAW_BARGRAPH, -1 )

#define EVT_DRAW_BARGRAPH(fn)            DECLARE_EVENT_TABLE_ENTRY(wxEVT_DRAW_BARGRAPH, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


// Define a custom event handler
class MyEvtHandler : public wxEvtHandler
{
public:
    MyEvtHandler(CBOINCListCtrl *theListControl) { m_listCtrl = theListControl; }
    void                    OnPaint(wxPaintEvent & event);

private:
    CBOINCListCtrl *        m_listCtrl;

    DECLARE_EVENT_TABLE()
};

#endif
