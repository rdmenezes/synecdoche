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

#ifndef VIEWWORK_H
#define VIEWWORK_H

#include <wx/object.h>
#include <wx/dialog.h>

#include "TaskViewBase.h"

class CWork : public wxObject
{
public:
    CWork();

    wxString m_strProjectName;
    wxString m_strApplicationName;
    wxString m_strName;
    float m_fCPUTime;
    float m_fProgress;
    float m_fTimeToCompletion;
    time_t m_tReportDeadline;
    wxString m_strStatus;
    wxString m_strCPUTime;
 	wxString m_strProgress;
 	wxString m_strTimeToCompletion;
 	wxString m_strReportDeadline;
};

class DlgYesToAll: public wxDialog {

    DECLARE_DYNAMIC_CLASS(DlgYesToAll)
    DECLARE_EVENT_TABLE()

public:
    DlgYesToAll() {}
    DlgYesToAll(wxWindow* parent, const wxString& caption, const wxString& message, long buttons);

    void OnButton(wxCommandEvent& event);
};

class CViewWork : public CTaskViewBase
{
    DECLARE_DYNAMIC_CLASS( CViewWork )

public:
    CViewWork();
    CViewWork(wxNotebook* pNotebook);

    ~CViewWork();

    virtual const wxString& GetViewName();
    virtual const wxString& GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnWorkSuspend( wxCommandEvent& event );
    void                    OnWorkResume(wxCommandEvent& event);
    void                    OnWorkShowGraphics( wxCommandEvent& event );
    void                    OnWorkAbort( wxCommandEvent& event );

    void                    OnProjectWebsiteClicked( wxEvent& event );
    
    std::vector<CWork*>     m_WorkCache;

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
    void                    GetDocApplicationName(size_t item, wxString& strBuffer) const;
    void                    GetDocName(size_t item, wxString& strBuffer) const;
    void                    GetDocCPUTime(size_t item, float& fBuffer) const;
    wxInt32                 FormatCPUTime(float fBuffer, wxString& strBuffer) const;
    void                    GetDocProgress(size_t item, float& fBuffer) const;
    wxInt32                 FormatProgress(float fBuffer, wxString& strBuffer) const;
    void                    GetDocTimeToCompletion(size_t item, float& fBuffer) const;
    wxInt32                 FormatTimeToCompletion(float fBuffer, wxString& strBuffer) const;
    void                    GetDocReportDeadline(size_t item, time_t& time) const;
    wxInt32                 FormatReportDeadline(time_t deadline, wxString& strBuffer) const;
    void                    GetDocStatus(size_t item, wxString& strBuffer) const;
    wxInt32                 FormatStatus(wxInt32 item, wxString& strBuffer) const;

    virtual double          GetProgressValue(long item);

    DECLARE_EVENT_TABLE()
};

#endif // VIEWWORK_H
