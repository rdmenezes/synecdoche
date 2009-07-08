// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#ifndef DLGMSGFILTER_H
#define DLGMSGFILTER_H

#include <wx/dialog.h>
#include "MsgFilterData.h"

class wxCheckListBox;
class wxSpinCtrl;

class DlgMsgFilter : public wxDialog {
public:
    DlgMsgFilter(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxString& title = _("Message filter"),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(377, 337),
                 long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    ~DlgMsgFilter();
    
    const MsgFilterData& GetFilterData() const;
    void SetFilterData(const MsgFilterData& filterData);
    
private:
    void OnProjectsSelectAll(wxCommandEvent& event);
    void OnProjectsSelectNone(wxCommandEvent& event);
    void OnDebugFlagsSelectAll(wxCommandEvent& event);
    void OnDebugFlagsSelectNone(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    
private:
    wxCheckListBox* m_projectsList;
    wxCheckListBox* m_debugFlagsList;
    wxSpinCtrl*     m_visibleMessages;
    MsgFilterData   m_filterData;
};

#endif // DLGMSGFILTER_H
