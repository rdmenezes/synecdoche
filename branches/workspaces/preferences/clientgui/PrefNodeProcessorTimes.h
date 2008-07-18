// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#ifndef _PREFNODEPROCESSORTIMES_H_
#define _PREFNODEPROCESSORTIMES_H_

struct GLOBAL_PREFS;

class PrefNodeProcessorTimes : public PrefNodeBase {

    DECLARE_DYNAMIC_CLASS(PrefNodeProcessorTimes)

public:
    PrefNodeProcessorTimes(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
    virtual ~PrefNodeProcessorTimes() {}
};

#endif // _PREFNODEPROCESSORTIMES_H_
