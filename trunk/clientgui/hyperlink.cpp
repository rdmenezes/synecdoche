// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
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

#include "hyperlink.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"

void HyperLink::ExecuteLink (const wxString &strLink) {
    if (!wxLaunchDefaultBrowser(strLink)) {
        wxString strDialogTitle = wxEmptyString;
        wxString strDialogMessage = wxEmptyString;

        // %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogTitle.Printf(
            _("%s - Can't find web browser"),
            wxGetApp().GetSkinManager()->GetAdvanced()->GetApplicationName().c_str()
        );

        // 1st %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        // 2nd %s is the URL that the browser is supposed to
        //    open.
        // 3rd %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogMessage.Printf(
            _("%s tried to display the web page\n"
            "\t%s\n"
            "but couldn't find a web browser.\n"
            "To fix this, set the environment variable\n"
            "BROWSER to the path of your web browser,\n"
            "then restart %s."),
            wxGetApp().GetSkinManager()->GetAdvanced()->GetApplicationName().c_str(),
            strLink.c_str(),
            wxGetApp().GetSkinManager()->GetAdvanced()->GetApplicationName().c_str()
        );

        ::wxMessageBox(
            strDialogMessage,
            strDialogTitle,
            wxOK | wxICON_INFORMATION
        );
    }
}
