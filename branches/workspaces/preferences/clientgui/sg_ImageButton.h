// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
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

#ifndef _IMAGEBUTTON_H_
#define _IMAGEBUTTON_H_ 

#define TAB_STATUS_RUNNING 1
#define TAB_STATUS_PREEMPTED 2 
#define TAB_STATUS_PAUSED_USER_ACTIVE 3
#define TAB_STATUS_PAUSED_USER_REQ 4
#define TAB_STATUS_PAUSED_POWER 5
#define TAB_STATUS_PAUSED_TIME_OF_DAY 6
#define TAB_STATUS_PAUSED_BENCHMARKS 7
#define TAB_STATUS_PAUSED 8

class CImageButton : public wxPanel 
{ 
public: 
    /// Constructors
    CImageButton(wxWindow* parent,wxBitmap bg,wxPoint coord, wxSize size, bool drawText, int initStatus); 
    ~CImageButton();
    void SetImage(wxBitmap bg);
    void OnPaint(wxPaintEvent& event); 
    void OnLeftUp(wxMouseEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void SetEnableShowGraphics(bool show);
    void SetStatus(int status);
    int GetStatus();
private: 
    //static const int MaxWidth = 320; 
    //static const int MaxHeight = 240; 
    wxBitmap btnBG; 
    bool m_enableShowGraphics;
    int status;
    wxString GetStatusText();
    void SetToolTip();
    DECLARE_EVENT_TABLE() 
}; 

#endif 