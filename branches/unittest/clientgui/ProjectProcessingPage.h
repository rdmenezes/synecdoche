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
#ifndef WIZ_PROJECTPROCESSINGPAGE_H
#define WIZ_PROJECTPROCESSINGPAGE_H

#include <wx/event.h>
#include <wx/wizard.h>

class wxStaticText;
class wxStaticBitmap;
class CBOINCBaseWizard;

/// CProjectProcessingPage custom event.
class CProjectProcessingPageEvent : public wxEvent
{
public:
    CProjectProcessingPageEvent(wxEventType evtType, wxWizardPage *parent) : wxEvent(-1, evtType) {
        SetEventObject(parent);
    }

    virtual wxEvent *Clone() const { return new CProjectProcessingPageEvent(*this); }
};


BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_PROJECTPROCESSING_STATECHANGE, 11100)
END_DECLARE_EVENT_TYPES()

#define EVT_PROJECTPROCESSING_STATECHANGE(fn) \
    DECLARE_EVENT_TABLE_ENTRY(wxEVT_PROJECTPROCESSING_STATECHANGE, -1, -1, (wxObjectEventFunction) (wxEventFunction) &fn, NULL),


class CProjectProcessingPage: public wxWizardPage
{    
    DECLARE_DYNAMIC_CLASS(CProjectProcessingPage)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProjectProcessingPage();

    CProjectProcessingPage(CBOINCBaseWizard* parent);

    /// Creation
    bool Create(CBOINCBaseWizard* parent);

    /// Creates the controls and sizers
    void CreateControls();

    /// wxEVT_WIZARD_PAGE_CHANGED event handler for ID_ATTACHPROJECTPAGE
    void OnPageChanged(wxWizardEvent& event);

    /// wxEVT_WIZARD_CANCEL event handler for ID_ATTACHPROJECTPAGE
    void OnCancel(wxWizardEvent& event);

    void OnStateChange(CProjectProcessingPageEvent& event);

    /// Gets the previous page.
    virtual wxWizardPage* GetPrev() const;

    /// Gets the next page.
    virtual wxWizardPage* GetNext() const;

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource(const wxString& name);

    /// Retrieves icon resources
    wxIcon GetIconResource(const wxString& name);

    bool GetProjectCommunitcationsSucceeded() const { return m_bProjectCommunitcationsSucceeded; }
    void SetProjectCommunitcationsSucceeded(bool value) { m_bProjectCommunitcationsSucceeded = value; }

    bool GetProjectUnavailable() const { return m_bProjectUnavailable; }
    void SetProjectUnavailable(bool value) { m_bProjectUnavailable = value; }

    bool GetProjectAccountAlreadyExists() const { return m_bProjectAccountAlreadyExists; }
    void SetProjectAccountAlreadyExists(bool value) { m_bProjectAccountAlreadyExists = value; }

    bool GetProjectAccountNotFound() const { return m_bProjectAccountNotFound; }
    void SetProjectAccountNotFound(bool value) { m_bProjectAccountNotFound = value; }

    bool GetProjectAttachSucceeded() const { return m_bProjectAttachSucceeded; }
    void SetProjectAttachSucceeded(bool value) { m_bProjectAttachSucceeded = value; }

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
    bool m_bProjectCommunitcationsSucceeded;
    bool m_bProjectUnavailable;
    bool m_bProjectAccountNotFound;
    bool m_bProjectAccountAlreadyExists;
    bool m_bProjectAttachSucceeded;
    int m_iBitmapIndex;
    int m_iCurrentState;
};

#endif // WIZ_PROJECTPROCESSINGPAGE_H
