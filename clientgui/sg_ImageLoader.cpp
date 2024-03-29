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

#include "sg_ImageLoader.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"

BEGIN_EVENT_TABLE(ImageLoader, wxWindow)
        EVT_PAINT(ImageLoader::OnPaint)
END_EVENT_TABLE()

ImageLoader::ImageLoader(wxWindow* parent, bool center) :
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
    centerOnParent = center;
}


void ImageLoader::LoadImage(wxBitmap image)
{
        int width = image.GetWidth();
        int height = image.GetHeight();
        Bitmap = image;
        SetSize(width, height);
        if ( centerOnParent ) {
            CentreOnParent();
        }
}


void ImageLoader::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{
        wxPaintDC dc(this);
        if(Bitmap.Ok())
        {
                dc.DrawBitmap(Bitmap, 0, 0);
        }
}
