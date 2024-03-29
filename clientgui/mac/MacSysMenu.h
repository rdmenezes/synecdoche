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

#ifndef _MACSYSMENU_H_
#define _MACSYSMENU_H_

#include <Carbon/Carbon.h>

#include "BOINCTaskBar.h"

class CMacSystemMenu : public CTaskBarIcon
{
public:
    CMacSystemMenu(wxString title, wxIcon* icon, wxIcon* iconDisconnected, wxIcon* iconSnooze);
    ~CMacSystemMenu();

    bool SetIcon(const wxIcon& icon);

    void LoadPrivateFrameworkBundle( CFStringRef framework, CFBundleRef *bundlePtr );
    //	Function pointer prototypes to the Mach-O Cocoa wrappers
    typedef void	(*SetUpSystemMenuProc)(MenuRef menuToCopy, CGImageRef theIcon);
    typedef void	(*SetSystemMenuIconProc)(CGImageRef theIcon);

    SetUpSystemMenuProc         SetUpSystemMenu;
    SetSystemMenuIconProc       SetSystemMenuIcon;
    
    bool                        IsOpeningAboutDlg() { return m_OpeningAboutDlg; }
    void                        SetOpeningAboutDlg(bool b) { m_OpeningAboutDlg = b; }
    void                        BuildMenu(void);
#if wxCHECK_VERSION(2,8,0)
    wxMenu                      *GetCurrentMenu();
#endif

private:
    
    bool                        m_OpeningAboutDlg;

};


#endif

