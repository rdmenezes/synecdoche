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

#ifndef _BOINCBASEVIEW_H_
#define _BOINCBASEVIEW_H_

#include <wx/panel.h>

#define DEFAULT_TASK_FLAGS             wxTAB_TRAVERSAL | wxADJUST_MINSIZE
#define DEFAULT_LIST_SINGLE_SEL_FLAGS  wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL

class wxNotebook;
class wxConfigBase;
class wxListEvent;
class wxTimerEvent;
class wxListItemAttr;
class wxGridEvent;
class wxGridRangeSelectEvent;

class CBOINCListCtrl;


typedef int     (*ListSortCompareFunc)(int*, int*);


class CBOINCBaseView : public wxPanel {
    DECLARE_DYNAMIC_CLASS( CBOINCBaseView )

public:

    CBOINCBaseView();
    CBOINCBaseView(
        wxNotebook* pNotebook
    );

    ~CBOINCBaseView();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();
    virtual const int       GetViewRefreshRate();

    bool                    FireOnSaveState( wxConfigBase* pConfig );
    bool                    FireOnRestoreState( wxConfigBase* pConfig );

    virtual int             GetListRowCount();
    void                    FireOnListRender( wxTimerEvent& event );
    void                    FireOnListSelected( wxListEvent& event );
    wxString                FireOnListGetItemText( long item, long column ) const;
    int                     FireOnListGetItemImage( long item ) const;
    wxListItemAttr*         FireOnListGetItemAttr( long item ) const;

    void                    FireOnShowView();
    
    int                     GetProgressColumn() { return m_iProgressColumn; }
    virtual double          GetProgressValue(long item);

    void                    InitSort();

    int                     m_iSortColumn;
    bool                    m_bReverseSort;

protected:

    virtual bool            OnSaveState( wxConfigBase* pConfig );
    virtual bool            OnRestoreState( wxConfigBase* pConfig );

    virtual void            OnListRender( wxTimerEvent& event );
    virtual void            OnListSelected( wxListEvent& event );
    virtual void            OnCacheHint(wxListEvent& event);
    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual int             OnListGetItemImage( long item ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    void                    OnColClick(wxListEvent& event);
    
    virtual void            OnGridSelectCell( wxGridEvent& event );
    virtual void            OnGridSelectRange( wxGridRangeSelectEvent& event );

    virtual int             GetDocCount();
    virtual wxString        OnDocGetItemImage( long item ) const;
    virtual wxString        OnDocGetItemAttr( long item ) const;

    virtual int             AddCacheElement();
    virtual int             EmptyCache();
    virtual int             GetCacheCount();
    virtual int             RemoveCacheElement();
    virtual int             SynchronizeCache();
    virtual bool            SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex);
    void                    sortData();

    virtual void            PreUpdateSelection();
    virtual void            UpdateSelection();
    virtual void            PostUpdateSelection();

    // Can't be pure virtual because of wxWidgets' RTTI.
    virtual void            DemandLoadView() {}
    virtual void            DemandLoadView(
                                wxWindowID,
                                int,
                                wxWindowID,
                                int
                                ) {}

    bool                    _EnsureLastItemVisible();
    virtual bool            EnsureLastItemVisible();

    static  void            AppendToStatus(wxString& existing, const wxChar* additional);
    static  wxString        HtmlEntityEncode(wxString strRaw);
    static  wxString        HtmlEntityDecode(wxString strRaw);

    bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;

    bool                    m_bForceUpdateSelection;
    bool                    m_bIgnoreUIEvents;
    
    int                     m_iProgressColumn;

    ListSortCompareFunc     m_funcSortCompare;
    wxArrayInt              m_iSortedIndexes;

    CBOINCListCtrl*         m_pListPane;

    bool                    m_bViewLoaded;
};


#endif
