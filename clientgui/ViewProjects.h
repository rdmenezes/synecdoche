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

#ifndef VIEWPROJECTS_H
#define VIEWPROJECTS_H

#include "TaskViewBase.h"

class CProject : public wxObject
{
public:
    CProject();

    wxString m_strProjectName;
    wxString m_strAccountName;
    wxString m_strTeamName;
    float m_fTotalCredit;
    float m_fAVGCredit;
    float m_fResourceShare;
    float m_fResourcePercent;
    wxString m_strStatus;
    wxString m_strTotalCredit;
 	wxString m_strAVGCredit;
 	wxString m_strResourceShare;
};

class CViewProjects : public CTaskViewBase
{
    DECLARE_DYNAMIC_CLASS( CViewProjects )

public:
    CViewProjects();
    CViewProjects(wxNotebook* pNotebook);

    ~CViewProjects();

    virtual const wxString& GetViewName();
    virtual const wxString& GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnProjectUpdate( wxCommandEvent& event );
    void                    OnProjectSuspend( wxCommandEvent& event );
    void                    OnProjectResume(wxCommandEvent& event);
    void                    OnProjectNoNewWork( wxCommandEvent& event );
    void                    OnProjectAllowNewWork(wxCommandEvent& event);
    void                    OnProjectReset( wxCommandEvent& event );
    void                    OnProjectDetach( wxCommandEvent& event );

    void                    OnProjectWebsiteClicked( wxEvent& event );

    std::vector<CProject*>  m_ProjectCache;

protected:
    virtual wxInt32         GetDocCount();

    virtual wxString        OnListGetItemText( long item, long column ) const;
    virtual wxInt32         AddCacheElement();
    virtual wxInt32         EmptyCache();
    virtual wxInt32         GetCacheCount();
    virtual wxInt32         RemoveCacheElement();
    virtual bool            SynchronizeCacheItem(wxInt32 iRowIndex, wxInt32 iColumnIndex);

    virtual void            UpdateSelection();

    virtual void            DemandLoadView();

    void                    GetDocProjectName(size_t item, wxString& strBuffer) const;
    void                    GetDocAccountName(size_t item, wxString& strBuffer) const;
    void                    GetDocTeamName(size_t item, wxString& strBuffer) const;
    void                    GetDocTotalCredit(size_t item, float& fBuffer) const;
    wxInt32                 FormatTotalCredit(float fBuffer, wxString& strBuffer) const;
    void                    GetDocAVGCredit(size_t item, float& fBuffer) const;
    wxInt32                 FormatAVGCredit(float fBuffer, wxString& strBuffer) const;
    void                    GetDocResourceShare(size_t item, float& fBuffer) const;
    void                    GetDocResourcePercent(size_t item, float& fBuffer) const;
    wxInt32                 FormatResourceShare(float fBuffer, float fBufferPercent, wxString& strBuffer) const;
    void                    GetDocStatus(size_t item, wxString& strBuffer) const;

    virtual double          GetProgressValue(long item);

    bool                    IsWebsiteLink( const wxString& strLink );
    wxInt32                 ConvertWebsiteIndexToLink( wxInt32 iProjectIndex, wxInt32 iWebsiteIndex, wxString& strLink );
    wxInt32                 ConvertLinkToWebsiteIndex( const wxString& strLink, wxInt32& iProjectIndex, wxInt32& iWebsiteIndex );

    DECLARE_EVENT_TABLE()
};

#endif // VIEWPROJECTS_H
