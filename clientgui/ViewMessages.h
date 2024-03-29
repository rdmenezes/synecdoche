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

#ifndef VIEWMESSAGES_H
#define VIEWMESSAGES_H

#include "MsgFilterData.h"
#include "TaskViewBase.h"

class CViewMessages : public CTaskViewBase
{
    DECLARE_DYNAMIC_CLASS( CViewMessages )
    DECLARE_EVENT_TABLE()

public:
    CViewMessages();
    CViewMessages(wxNotebook* pNotebook);

    ~CViewMessages();

    virtual const wxString& GetViewName();
    virtual const wxString& GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnMessagesCopyAll(wxCommandEvent& event);
    void                    OnMessagesCopySelected(wxCommandEvent& event);
    
    void                    OnMessagesEditFilter(wxCommandEvent& event);
    void                    OnMessagesEnableFilter(wxCommandEvent& event);

    /// Called when the manager has successfully connected to a client.
    void                    OnConnect();

protected:

    wxInt32                 m_iPreviousDocCount;

    wxListItemAttr*         m_pMessageInfoAttr;
    wxListItemAttr*         m_pMessageErrorAttr;

    virtual void            OnListRender( wxTimerEvent& event );

    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual wxListItemAttr* OnListGetItemAttr( long item ) const;

    virtual bool            EnsureLastItemVisible();

    virtual void            UpdateSelection();

    virtual void            DemandLoadView();

    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    wxInt32                 FormatMessage( wxInt32 item, wxString& strBuffer ) const;

#ifdef wxUSE_CLIPBOARD
    bool                    m_bClipboardOpen;
    wxString                m_strClipboardData;
    bool                    OpenClipboard(wxInt32 row_count);
    wxInt32                 CopyToClipboard( wxInt32 item );
    bool                    CloseClipboard();
#endif

private:
    /// Filter the messages based on the current filter settings.
    void                    FilterMessages();
    
    /// Return the message index for the requested row index.
    size_t                  GetFilteredIndex(size_t index) const;
    
    bool                    m_enableMsgFilter;  ///< Toggle message filtering.
    MsgFilterData           m_msgFilterData;    ///< The current message filter settings.
    std::vector<size_t>     m_filteredIndexes;  ///< Internal mapping between row indexes and message indexes based on filter settings.
    size_t                  m_maxFilteredIndex; ///< Index of the last filtered message (plus one).
};

#endif // VIEWMESSAGES_H
