// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2008 Peter Kortschack
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
//
#ifndef WIZ_WELCOMEPAGE_H
#define WIZ_WELCOMEPAGE_H

#include <wx/wizard.h>

class wxStaticText;
class wxStaticBox;
class wxCheckBox;
class CBOINCBaseWizard;

class CWelcomePage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CWelcomePage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWelcomePage();
    CWelcomePage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_WELCOMEPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WELCOMEPAGE
    void OnPageChanging(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_WELCOMEPAGE
    void OnCancel(wxWizardEvent& event);

    /// wxEVT_SET_FOCUS event handler for ID_WELCOMEPAGE
    void OnSetFocus(wxFocusEvent& event);

    /// wxEVT_SHOW event handler for ID_WELCOMEPAGE
    void OnShow(wxShowEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);
////@end CWelcomePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticText* m_pDescriptionStaticCtrl;
    wxStaticText* m_pDirectionsStaticCtrl;
#if defined(__WXDEBUG__)
    wxStaticBox* m_pErrDescriptionCtrl; 
    wxCheckBox* m_pErrProjectPropertiesCtrl;
    wxCheckBox* m_pErrProjectCommCtrl;
    wxCheckBox* m_pErrProjectPropertiesURLCtrl;
    wxCheckBox* m_pErrAccountCreationDisabledCtrl;
    wxCheckBox* m_pErrClientAccountCreationDisabledCtrl;
    wxCheckBox* m_pErrAccountAlreadyExistsCtrl;
    wxCheckBox* m_pErrProjectAlreadyAttachedCtrl;
    wxCheckBox* m_pErrProjectAttachFailureCtrl;
    wxCheckBox* m_pErrGoogleCommCtrl;
    wxCheckBox* m_pErrNetDetectionCtrl;
#endif
};

#endif // WIZ_WELCOMEPAGE_H
