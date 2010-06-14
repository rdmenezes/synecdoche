// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2010 Nicolas Alvarez
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

#ifndef SYNEC_GUI_BUILDLAYOUT_H
#define SYNEC_GUI_BUILDLAYOUT_H

class wxWindow;
class wxSizer;
class wxString;
class wxControl;
class UiFormatString;

#include <vector>
#include <string>

void buildLayoutf(wxWindow* parent, wxSizer* sizer, const UiFormatString& formatString, const std::vector<wxControl*>& controls);
void buildLayout(wxWindow* parent, wxSizer* sizer, const wxString& string, const std::vector<wxControl*>& controls);
void buildLayoutv(wxWindow* parent, wxSizer* sizer, const wxString& string, ...);

#endif
