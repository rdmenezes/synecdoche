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

#ifndef SIMPLEFRAME_H
#define SIMPLEFRAME_H

#include <wx/bitmap.h>
#include <wx/panel.h>

#include "BOINCBaseFrame.h"

class wxFlexGridSizer;
class wxTimer;
class wxTimerEvent;

class CFrameEvent;
class ClientStateIndicator;
class CProjectsComponent;
class CViewTabPage;
class ImageLoader;
class StatImageLoader;
class WorkunitNotebook;

class CSimplePanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS(CSimplePanel)

public:
    CSimplePanel();
    CSimplePanel(wxWindow* parent);

   ~CSimplePanel();
	//
	// Flat Neotebook
	WorkunitNotebook *wrkUnitNB;
    wxBitmap const workWUico;
	// My projects component
	CProjectsComponent *projComponent;
	// Client State Indicator
	ClientStateIndicator *clientState;
	//Collapse button
	bool midAppCollapsed;
	bool btmAppCollapsed;
	////////////////////////////;
	bool projectViewInitialized;
	bool emptyViewInitialized;
	bool notebookViewInitialized;

    void ReskinInterface();
	void InitEmptyView();
	void UpdateEmptyView();
	void DestroyEmptyView();
	void InitResultView();
	void InitProjectView();
	void UpdateProjectView();
	void InitNotebook();
	void DestroyNotebook();
	void OnProjectsAttachToProject();
	void SetDlgOpen(bool newDlgState) { dlgOpen = newDlgState; }
	bool GetDlgOpen() { return dlgOpen; }
	//////////
	wxFlexGridSizer *mainSizer;
	wxSize wxNotebookSize;
	//////////
	wxBitmap *frameBg;
	wxBitmap *bm13cImg0;
	wxBitmap *btmpIcnWorking;
	wxBitmap *bm39cImg0;

	wxBitmap *btmpIcnSleeping;
	wxTimer* m_pFrameRenderTimer;

    DECLARE_EVENT_TABLE()

protected:
    void OnFrameRender(wxTimerEvent& event );
    void OnEraseBackground(wxEraseEvent& event);

private:
    bool dlgOpen;
};


// Define a new frame
class CSimpleFrame : public CBOINCBaseFrame
{
    DECLARE_DYNAMIC_CLASS(CSimpleFrame)

public:
    CSimpleFrame();
    CSimpleFrame(wxString title, wxIcon* icon, wxIcon* icon32);

   ~CSimpleFrame();

    void OnHelp( wxHelpEvent& event );
    void OnHelpBOINC( wxCommandEvent& event );

	void OnConnect(CFrameEvent& event );
    void OnProjectsAttachToProject();
    void OnReloadSkin( CFrameEvent& event );

private:
    bool SaveState();
    bool RestoreState();

protected:

#ifdef __WXMAC__
	wxMenuBar* m_pMenubar;
#endif

	wxAcceleratorEntry  m_Shortcuts[1];
    wxAcceleratorTable* m_pAccelTable;

	CSimplePanel* m_pBackgroundPanel;

    DECLARE_EVENT_TABLE()
};

#endif


