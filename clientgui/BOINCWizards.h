// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
// Copyright (C) 2009 University of California
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
//
#ifndef BOINCWIZARDS_H
#define BOINCWIZARDS_H

// Wizard Identifiers
//
#define ID_ATTACHPROJECTWIZARD 10000
#define ID_ATTACHACCOUNTMANAGERWIZARD 10001
#define SYMBOL_CWIZARDATTACHPROJECT_IDNAME ID_ATTACHPROJECTWIZARD
#define SYMBOL_CWIZARDACCOUNTMANAGER_IDNAME ID_ATTACHACCOUNTMANAGERWIZARD

// The following includes are needed by the wizard detection:
#include "WizardAccountManager.h"
#include "WizardAttachProject.h"

// Page Identifiers
//

// Generic Pages
#define ID_WELCOMEPAGE 10100
#define ID_ACCOUNTINFOPAGE 10102
#define ID_COMPLETIONPAGE 10103
#define ID_COMPLETIONERRORPAGE 10104
#define ID_ERRNOTDETECTEDPAGE 10105
#define ID_ERRUNAVAILABLEPAGE 10106
#define ID_ERRNOINTERNETCONNECTIONPAGE 10108
#define ID_ERRNOTFOUNDPAGE 10109
#define ID_ERRALREADYEXISTSPAGE 10110
#define ID_ERRPROXYINFOPAGE 10111
#define ID_ERRPROXYPAGE 10112

// Attach to Project Wizard Pages
#define ID_PROJECTINFOPAGE 10200
#define ID_PROJECTPROPERTIESPAGE 10201
#define ID_PROJECTPROCESSINGPAGE 10202

// Account Manager Wizard Pages
#define ID_ACCOUNTMANAGERINFOPAGE 10300
#define ID_ACCOUNTMANAGERPROPERTIESPAGE 10301
#define ID_ACCOUNTMANAGERPROCESSINGPAGE 10302


// Control Identifiers
//

// Bitmap Progress Control
#define ID_PROGRESSCTRL 11000

// BOINC Hyperlink Control
#define ID_BOINCHYPERLINK 11001

// Completion Error Page Multiline Text Control
#define ID_TEXTCTRL 11002

// Project Info/Account Manager Info Controls
#define ID_PROJECTSELECTIONCTRL 11200
#define ID_PROJECTURLSTATICCTRL 11201
#define ID_PROJECTURLDESCRIPTIONSTATICCTRL 11202
#define ID_PROJECTURLCTRL 11203

// Account Info Controls
#define ID_ACCOUNTCREATECTRL 11400
#define ID_ACCOUNTUSEEXISTINGCTRL 11401
#define ID_ACCOUNTEMAILADDRESSSTATICCTRL 11402
#define ID_ACCOUNTEMAILADDRESSCTRL 11403
#define ID_ACCOUNTPASSWORDSTATICCTRL 11404
#define ID_ACCOUNTPASSWORDCTRL 11405
#define ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL 11406
#define ID_ACCOUNTCONFIRMPASSWORDCTRL 11407
#define ID_ACCOUNTREQUIREMENTSSTATICCTRL 11408
#define ID_ACCOUNTFORGOTPASSWORDCTRL 11409

// Proxy Page Controls
#define ID_PROXYHTTPSERVERSTATICCTRL 11500
#define ID_PROXYHTTPSERVERCTRL 11501
#define ID_PROXYHTTPPORTSTATICCTRL 11502
#define ID_PROXYHTTPPORTCTRL 11503
#define ID_PROXYHTTPUSERNAMESTATICCTRL 11504
#define ID_PROXYHTTPUSERNAMECTRL 11505
#define ID_PROXYHTTPPASSWORDSTATICCTRL 11506
#define ID_PROXYHTTPPASSWORDCTRL 11507
#define ID_PROXYHTTPAUTODETECTCTRL 11508
#define ID_PROXYSOCKSSERVERSTATICCTRL 11509
#define ID_PROXYSOCKSSERVERCTRL 11510
#define ID_PROXYSOCKSPORTSTATICCTRL 11511
#define ID_PROXYSOCKSPORTCTRL 11512
#define ID_PROXYSOCKSUSERNAMESTATICCTRL 11513
#define ID_PROXYSOCKSUSERNAMECTRL 11514
#define ID_PROXYSOCKSPASSWORDSTATICCTRL 11515
#define ID_PROXYSOCKSPASSWORDCTRL 11516

// Account Manager Status Controls
#define ID_ACCTMANAGERNAMECTRL 11600
#define ID_ACCTMANAGERLINKCTRL 11601
#define ID_ACCTMANAGERUPDATECTRL 11602
#define ID_ACCTMANAGERREMOVECTRL 11603

/// Check to which wizard a page belongs.
/// The header files for all existing wizards need to be included in this file
/// because dynamic_cast needs the class to be defined.
///
/// \tparam wiz The class name of the wizard that should be checked.
/// \param[in] cur_page A pointer to the wizard's page for which the wizard type
///                     should be checked.
/// \return True if the wizard is of the same class as specified by \a wiz,
///         false otherwise.
template<typename wiz> bool CheckWizardTypeByPage(const wxWizardPage* cur_page) {
    return !!(dynamic_cast<wiz*>(cur_page->GetParent()));
}

// Commonly defined macros
//
#define PAGE_TRANSITION_NEXT(id) \
    ((CBOINCBaseWizard*)GetParent())->PushPageTransition((wxWizardPage*)this, id)
 
#define PAGE_TRANSITION_BACK \
    ((CBOINCBaseWizard*)GetParent())->PopPageTransition()
 
#define PROCESS_CANCELEVENT(event) \
    ((CBOINCBaseWizard*)GetParent())->ProcessCancelEvent(event)

#define CHECK_CLOSINGINPROGRESS() \
    ((CBOINCBaseWizard*)GetParent())->IsCancelInProgress()

/// Simple namespace containing constants associated with configuration
/// options for the wizards.
namespace WizardCfg {
    /// Path for wizard settings in the manager's config file:
    static const wxString baseCfgLocation = wxString(wxT("/Wizards"));

    /// Configuration name for last used email address:
    static const wxString defaultEmailAddress = wxString(wxT("DefaultEmailAddress"));

    /// Configuration name for last used user name:
    static const wxString defaultUserName = wxString(wxT("DefaultUserName"));
};

#endif // BOINCWIZARDS_H

