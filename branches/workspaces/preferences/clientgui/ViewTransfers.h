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

#ifndef _VIEWTRANSFERS_H_
#define _VIEWTRANSFERS_H_


#include "TaskViewBase.h"


class CTransfer : public wxObject {
public:
    CTransfer() {}

    wxString m_strProjectName;
    wxString m_strFileName;
    float m_fProgress;
    double m_fBytesXferred;
    double m_fTotalBytes;
    double m_dTime;
    double m_dSpeed;
    wxString m_strStatus;
};


class CViewTransfers : public CTaskViewBase {
    DECLARE_DYNAMIC_CLASS( CViewTransfers )

public:
    CViewTransfers();
    CViewTransfers(wxNotebook* pNotebook);

    ~CViewTransfers();

    virtual wxString&       GetViewName();
    virtual wxString&       GetViewDisplayName();
    virtual const char**    GetViewIcon();

    void                    OnTransfersRetryNow( wxCommandEvent& event );
    void                    OnTransfersAbort( wxCommandEvent& event );

    std::vector<CTransfer*> m_TransferCache;

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

    void                    GetDocProjectName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatProjectName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocFileName(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatFileName( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocProgress(wxInt32 item, float& fBuffer) const;
    wxInt32                 FormatProgress( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocBytesXferred(wxInt32 item, double& fBuffer) const;
    void                    GetDocTotalBytes(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatSize( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocTime(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatTime( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocSpeed(wxInt32 item, double& fBuffer) const;
    wxInt32                 FormatSpeed( wxInt32 item, wxString& strBuffer ) const;
    void                    GetDocStatus(wxInt32 item, wxString& strBuffer) const;
    wxInt32                 FormatStatus( wxInt32 item, wxString& strBuffer ) const;

    virtual double          GetProgressValue(long item);

    DECLARE_EVENT_TABLE()
};


#endif