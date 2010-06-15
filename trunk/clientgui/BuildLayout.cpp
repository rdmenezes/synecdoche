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

#include "BuildLayout.h"

#include <stdarg.h>

#include <wx/stattext.h>
#include <wx/sizer.h>

#include "UiFormatString.h"

void buildLayoutFmt(wxWindow* parent, wxSizer* sizer, const UiFormatString& formatString, const std::vector<wxControl*>& controls)
{
    std::vector<wxControl*> controlsReordered;

    for (size_t i = 0; i<formatString.placeholders().size(); ++i) {
        controlsReordered.push_back(controls[formatString.placeholder(i) - 1]);
    }

    for (size_t i=0; i<controlsReordered.size(); ++i) {
        wxStaticText* label = new wxStaticText( parent, wxID_ANY, formatString.label(i) );
        sizer->Add(label, 0, wxALL, 5);

        wxControl* control = controlsReordered[i];
        sizer->Add(control, 0, wxALL, 1);
        if (i != 0) {
            control->MoveAfterInTabOrder(controlsReordered[i-1]);
        }
    }
    if (formatString.labels().size() > controls.size()) {
        wxStaticText* label = new wxStaticText( parent, wxID_ANY, formatString.label(controls.size()) );
        sizer->Add(label, 0, wxALL, 5);
    }
}
void buildLayoutv(wxWindow* parent, wxSizer* sizer, const wxString& string, const std::vector<wxControl*>& controls)
{
    UiFormatString formatString(string);
    buildLayoutFmt(parent, sizer, formatString, controls);
}
void buildLayout(wxWindow* parent, wxSizer* sizer, const wxString& string, ...)
{
    va_list ap;
    va_start(ap, string);

    UiFormatString formatString(string);
    std::vector<wxControl*> controls;

    for (size_t i=0; i<formatString.placeholders().size(); ++i) {
        controls.push_back(va_arg(ap, wxControl*));
    }

    buildLayoutFmt(parent, sizer, formatString, controls);
    va_end(ap);
}
