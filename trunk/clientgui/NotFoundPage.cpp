// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
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
//
#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "NotFoundPage.h"


/*!
 * CErrNotFoundPage type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CErrNotFoundPage, wxWizardPageEx )
 
/*!
 * CErrNotFoundPage event table definition
 */
 
BEGIN_EVENT_TABLE( CErrNotFoundPage, wxWizardPageEx )

////@begin CErrNotFoundPage event table entries
    EVT_WIZARDEX_PAGE_CHANGED( -1, CErrNotFoundPage::OnPageChanged )
    EVT_WIZARDEX_CANCEL( -1, CErrNotFoundPage::OnCancel )

////@end CErrNotFoundPage event table entries
 
END_EVENT_TABLE()
 
/*!
 * CErrNotFoundPage constructors
 */
 
CErrNotFoundPage::CErrNotFoundPage( )
{
}
 
CErrNotFoundPage::CErrNotFoundPage( CBOINCBaseWizard* parent )
{
    Create( parent );
}
 
/*!
 * CErrNoInternetConnection creator
 */
 
bool CErrNotFoundPage::Create( CBOINCBaseWizard* parent )
{
////@begin CErrNotFoundPage member initialisation
    m_pTitleStaticCtrl = NULL;
    m_pDirectionsStaticCtrl = NULL;
////@end CErrNotFoundPage member initialisation
 
////@begin CErrNotFoundPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageEx::Create( parent, ID_ERRNOTFOUNDPAGE, wizardBitmap );

    CreateControls();
    GetSizer()->Fit(this);
////@end CErrNotFoundPage creation

    return TRUE;
}
 
/*!
 * Control creation for CErrNoInternetConnection
 */
 
void CErrNotFoundPage::CreateControls()
{    
////@begin CErrNotFoundPage content construction
    CErrNotFoundPage* itemWizardPage96 = this;

    wxBoxSizer* itemBoxSizer97 = new wxBoxSizer(wxVERTICAL);
    itemWizardPage96->SetSizer(itemBoxSizer97);

    m_pTitleStaticCtrl = new wxStaticText;
    m_pTitleStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pTitleStaticCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxBOLD, FALSE, _T("Verdana")));
    itemBoxSizer97->Add(m_pTitleStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer97->Add(5, 5, 0, wxALIGN_LEFT|wxALL, 5);

    m_pDirectionsStaticCtrl = new wxStaticText;
    m_pDirectionsStaticCtrl->Create( itemWizardPage96, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer97->Add(m_pDirectionsStaticCtrl, 0, wxALIGN_LEFT|wxALL, 5);
////@end CErrNotFoundPage content construction
}
 
/*!
 * Gets the previous page.
 */
 
wxWizardPageEx* CErrNotFoundPage::GetPrev() const
{
    return PAGE_TRANSITION_BACK;
}
 
/*!
 * Gets the next page.
 */

wxWizardPageEx* CErrNotFoundPage::GetNext() const
{
    return PAGE_TRANSITION_NEXT(ID_ACCOUNTINFOPAGE);
}
 
/*!
 * Should we show tooltips?
 */
 
bool CErrNotFoundPage::ShowToolTips()
{
    return TRUE;
}
 
/*!
 * Get bitmap resources
 */
 
wxBitmap CErrNotFoundPage::GetBitmapResource( const wxString& WXUNUSED(name) )
{
    // Bitmap retrieval
////@begin CErrNotFoundPage bitmap retrieval
    return wxNullBitmap;
////@end CErrNotFoundPage bitmap retrieval
}
 
/*!
 * Get icon resources
 */

wxIcon CErrNotFoundPage::GetIconResource( const wxString& WXUNUSED(name) )
{
    // Icon retrieval
////@begin CErrNotFoundPage icon retrieval
    return wxNullIcon;
////@end CErrNotFoundPage icon retrieval
}
  
/*!
 * wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CErrNotFoundPage::OnPageChanged( wxWizardExEvent& event ) {
    if (event.GetDirection() == false) return;


    wxASSERT(m_pTitleStaticCtrl);
    wxASSERT(m_pDirectionsStaticCtrl);

    m_pTitleStaticCtrl->SetLabel(
        _("Login Failed.")
    );
    if (((CBOINCBaseWizard*)GetParent())->project_config.uses_username) {
        m_pDirectionsStaticCtrl->SetLabel(
            _("Check the username and password, and try again.")
        );
    } else {
        m_pDirectionsStaticCtrl->SetLabel(
            _("Check the email address and password, and try again.")
        );
    }

    Fit();
}

/*!
 * wxEVT_WIZARD_CANCEL event handler for ID_ERRNACCOUNTDOESNOTEXISTPAGE
 */

void CErrNotFoundPage::OnCancel( wxWizardExEvent& event ) {
    PROCESS_CANCELEVENT(event);
}
