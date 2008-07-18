// Synecdoche
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 David Barnard
// Copyright (C) 2005 University of California
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

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "hyperlink.h"

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
