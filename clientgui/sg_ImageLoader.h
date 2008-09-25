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

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <wx/bitmap.h>
#include <wx/window.h>

class ImageLoader : public wxWindow
{
public:
    /// Constructors
    ImageLoader(wxWindow* parent, bool center = false);
    void LoadImage(wxBitmap image);
    void OnPaint(wxPaintEvent& event);
private:
    //static const int MaxWidth = 320;
    //static const int MaxHeight = 240;
    wxBitmap Bitmap;
    bool centerOnParent;
    DECLARE_EVENT_TABLE()
};

#endif

