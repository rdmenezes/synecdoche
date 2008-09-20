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
#ifndef WIZ_PROJECTPROPERTIESPAGE_H
#define WIZ_PROJECTPROPERTIESPAGE_H

#include <wx/event.h>
#include <wx/wizard.h>

class CBOINCBaseWizard;

/// CProjectPropertiesPage custom events
class CProjectPropertiesPageEvent : public wxEvent
{
public:
    CProjectPropertiesPageEvent(wxEventType evtType, wxWizardPage *parent) : wxEvent(-1, evtType) {
        SetEventObject(parent);
    }

    virtual wxEvent *Clone() const { return new CProjectPropertiesPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_PROJECTPROPERTIES_STATECHANGE, 11000)
END_DECLARE_EVENT_TYPES()

#define EVT_PROJECTPROPERTIES_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_PROJECTPROPERTIES_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),

class CProjectPropertiesPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CProjectPropertiesPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectPropertiesPage();

    CProjectPropertiesPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_PROJECTPROPERTIESPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_PROJECTPROPERTIESPAGE
    void OnCancel(wxWizardEvent& event);

    void OnStateChange(CProjectPropertiesPageEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    bool GetProjectPropertiesSucceeded() const { return m_bProjectPropertiesSucceeded; }
    void SetProjectPropertiesSucceeded(bool value) { m_bProjectPropertiesSucceeded = value; }

    bool GetProjectPropertiesURLFailure() const { return m_bProjectPropertiesURLFailure; }
    void SetProjectPropertiesURLFailure(bool value) { m_bProjectPropertiesURLFailure = value; }

    bool GetProjectAccountCreationDisabled() const { return m_bProjectAccountCreationDisabled; }
    void SetProjectAccountCreationDisabled(bool value) { m_bProjectAccountCreationDisabled = value; }

    bool GetProjectClientAccountCreationDisabled() const { return m_bProjectClientAccountCreationDisabled; }
    void SetProjectClientAccountCreationDisabled(bool value) { m_bProjectClientAccountCreationDisabled = value; }

    bool GetProjectAlreadyAttached() const { return m_bProjectAlreadyAttached; }
    void SetProjectAlreadyAttached(bool value) { m_bProjectAlreadyAttached = value; }

    bool GetNetworkConnectionDetected() const { return m_bNetworkConnectionDetected; }
    void SetNetworkConnectionDetected(bool value) { m_bNetworkConnectionDetected = value; }

    wxInt32 GetCurrentState() const { return m_iCurrentState; }
    void SetNextState(wxInt32 value) { m_iCurrentState = value; }

    /// Should we show tooltips?
    static bool ShowToolTips();

    /// Progress Image Support
    void StartProgress(wxStaticBitmap* pBitmap);
    void IncrementProgress(wxStaticBitmap* pBitmap);
    void FinishProgress(wxStaticBitmap* pBitmap);

private:
    wxStaticText* m_pTitleStaticCtrl;
    wxStaticBitmap* m_pProgressIndicator;
    bool m_bProjectPropertiesSucceeded;
    bool m_bProjectPropertiesURLFailure;
    bool m_bProjectAccountCreationDisabled;
    bool m_bProjectClientAccountCreationDisabled;
    bool m_bProjectAlreadyAttached;
    bool m_bNetworkConnectionDetected;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

#endif // WIZ_PROJECTPROPERTIESPAGE_H
