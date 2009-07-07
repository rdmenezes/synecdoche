// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
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

#ifndef STATIMAGELOADER_H
#define STATIMAGELOADER_H

#include <string>
#include <wx/bitmap.h>
#include <wx/window.h>

class wxMenu;

class StatImageLoader : public wxWindow 
{
public:
    //members
    wxMenu *statPopUpMenu;
    //Skin Class
    std::string m_prjUrl;
    /// Constructors
    StatImageLoader(wxWindow* parent, std::string url); 
    ~StatImageLoader(); 
    void LoadImage();
    void OnMenuLinkClicked(wxCommandEvent& event);
    void OnProjectDetach();
    void PopUpMenu(wxMouseEvent& event); 
    void OnPaint(wxPaintEvent& event);
    void RebuildMenu();
    void UpdateInterface();

private:
    wxBitmap Bitmap;
    std::string projectIcon;
    int numReloadTries;
    size_t urlCount;
    double project_files_downloaded_time;
    double project_last_rpc_time;
    void LoadStatIcon(wxBitmap& image);
    void ReloadProjectSpecificIcon();
    void BuildUserStatToolTip();
    void AddMenuItems();

    /// Retrieve the location of the project icon.
    std::string GetProjectIconLoc();

    DECLARE_EVENT_TABLE() 
};

#endif // STATIMAGELOADER_H
