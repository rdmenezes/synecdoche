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

#include "UiFormatString.h"

#include <wx/string.h>
#include <iostream>

UiFormatString::UiFormatString(const wxString& str)
{
    this->str = str;
    parse();
    for(std::vector<wxString>::iterator it = labels_.begin(); it != labels_.end(); ++it) {
        (*it).Trim(0).Trim(1);
    }
}
void UiFormatString::parse()
{
    size_t percent_pos = 0;
    size_t label_begin = 0;
    int pholder_idx;
    while ((pholder_idx = get_next_pholder(percent_pos)) != -1) {
        placeholders_.push_back(pholder_idx);

        labels_.push_back(str.SubString(label_begin, percent_pos-1));
        label_begin = percent_pos+2;

        percent_pos++;
    }
    labels_.push_back(str.SubString(label_begin, str.Len()));
}

/// Seraches for a %n placeholder starting at index 'pos'.
/// Returns the digit following the percent sign, or -1 on error.
/// After this call, \a pos will point to the percent sign.
/// Make sure you increment it as appropriate before calling this function again.

int UiFormatString::get_next_pholder(size_t& pos) {
    pos = str.find(wxT('%'), pos);
    if (pos == wxString::npos) return -1;
    
    if (pos + 1 >= str.length()) {
        return -1;
    }
    wxChar pholder_idx_char = str.GetChar(pos+1);
    if ('0' <= pholder_idx_char && pholder_idx_char <= '9') {
        return pholder_idx_char - '0';
    }
    return -1;
}

const std::vector<wxString>& UiFormatString::labels() const {
    return labels_;
}
const std::vector<int>& UiFormatString::placeholders() const {
    return placeholders_;
}
