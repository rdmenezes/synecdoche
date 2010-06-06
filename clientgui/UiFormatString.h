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

#ifndef SYNEC_GUI_FORMATSTRING_H
#define SYNEC_GUI_FORMATSTRING_H

#include <cstddef>

#include <wx/string.h>
#include <vector>

class UiFormatString {
public:
    UiFormatString(const wxString& str);
    const std::vector<wxString>& labels() const;
    const std::vector<int>& placeholders() const;
    
    wxString label(size_t index) const { return labels().at(index); }
    int placeholder(size_t index) const { return placeholders().at(index); }

private:
    wxString str;
    std::vector<wxString> labels_;
    std::vector<int> placeholders_;

    void parse();
    int get_next_pholder(size_t& pos);
};

#endif
