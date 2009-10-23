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

#ifndef BOINCBASEVIEW_H
#define BOINCBASEVIEW_H

#include <map>
#include <vector>

#include <wx/panel.h>
#include <wx/listbase.h> //for wxListColumnFormat

#define DEFAULT_TASK_FLAGS             wxTAB_TRAVERSAL | wxADJUST_MINSIZE
#define DEFAULT_LIST_MULTI_SEL_FLAGS   wxLC_REPORT | wxLC_VIRTUAL

class wxNotebook;
class wxConfigBase;
class wxListEvent;
class wxTimerEvent;
class wxListItemAttr;
class wxGridEvent;
class wxGridRangeSelectEvent;

class CBOINCListCtrl;


typedef bool (*ListSortCompareFunc)(size_t, size_t);

typedef std::map<long, const wxChar*> ColumnListMap;

class CBOINCBaseView : public wxPanel {
    DECLARE_DYNAMIC_CLASS(CBOINCBaseView)

public:

    CBOINCBaseView();
    CBOINCBaseView(wxNotebook* pNotebook);

    virtual ~CBOINCBaseView();

    /// Return the name of the view.
    virtual const wxString& GetViewName();

    /// Return the user friendly name of the view.
    virtual const wxString& GetViewDisplayName();

    /// Return the user friendly icon of the view.
    virtual const char**    GetViewIcon();

    /// The rate at which the view is refreshed.
    virtual int             GetViewRefreshRate();

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

    /// Get a map containing the keys for all columns.
    const ColumnListMap&    GetColumnKeys() const;
    
    /// Called when the manager has successfully connected to a client.
    virtual void            OnConnect();

    int                     m_iSortColumn;
    bool                    m_bReverseSort;

protected:

    virtual bool            OnSaveState(wxConfigBase* pConfig);
    virtual bool            OnRestoreState(wxConfigBase* pConfig);

    virtual void            OnListRender( wxTimerEvent& event );
    virtual void            OnListSelected( wxListEvent& event );
    virtual void            OnCacheHint(wxListEvent& event);
    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual int             OnListGetItemImage( long item ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    void                    OnColClick(wxListEvent& event);
    
    virtual int             GetDocCount();
    virtual wxString        OnDocGetItemImage( long item ) const;
    virtual wxString        OnDocGetItemAttr( long item ) const;

    /// Add a new column to the tab.
    void                    AddColumn(long column, const wxChar* heading,
                                      wxListColumnFormat align, int width);

    /// Read and apply stored settings like column widths.
    void                    RestoreState();

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

    /// HTML Entity Encoding.
    static  wxString        HtmlEntityEncode(const wxString& strRaw);

    /// HTML Entity Decoding.
    static  wxString        HtmlEntityDecode(const wxString& strRaw);

    bool                    m_bProcessingTaskRenderEvent;
    bool                    m_bProcessingListRenderEvent;

    bool                    m_bForceUpdateSelection;
    bool                    m_bIgnoreUIEvents;
    bool                    m_bNeedSort;
    
    int                     m_iProgressColumn;

    ListSortCompareFunc     m_funcSortCompare;
    std::vector<size_t>     m_iSortedIndexes;

    CBOINCListCtrl*         m_pListPane;

    bool                    m_bViewLoaded;

private:
    ColumnListMap m_column_keys;
};


#endif // BOINCBASEVIEW_H
