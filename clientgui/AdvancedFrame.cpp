// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2010 David Barnard, Peter Kortschack
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

#ifdef __APPLE__
#include "mac/MacGUI.pch"
#endif

#include "AdvancedFrame.h"

#include "stdwx.h"
#include "version.h"
#include "diagnostics.h"
#include "str_util.h"
#include "mfile.h"
#include "miofile.h"
#include "BOINCGUIApp.h"
#include "Events.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "BOINCBaseView.h"
#include "BOINCTaskBar.h"

#ifndef __WXMAC__
#    include "BOINCDialupManager.h"
#endif // __WXMAC__

#include "ViewProjects.h"
#include "ViewWork.h"
#include "ViewTransfers.h"
#include "ViewMessages.h"
#include "ViewStatistics.h"
#include "ViewResources.h"
#include "DlgAbout.h"
#include "DlgOptions.h"
#include "DlgSelectComputer.h"
#include "DlgGenericMessage.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"
#include "WizardAttachProject.h"
#include "WizardAccountManager.h"
#include "DlgAdvPreferences.h"
#include "hyperlink.h"


enum STATUSBARFIELDS {
    STATUS_TEXT,
    STATUS_CONNECTION_STATUS
};


IMPLEMENT_DYNAMIC_CLASS(CStatusBar, wxStatusBar)


CStatusBar::CStatusBar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Default Constructor Function End"));
}


CStatusBar::CStatusBar(wxWindow *parent) :
    wxStatusBar(parent, ID_STATUSBAR, wxST_SIZEGRIP, _T("statusBar"))
{
    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Function Begin"));

    wxPoint conSize = ConvertDialogToPixels(wxPoint(140, 0));
    const int widths[] = {-1, conSize.x};
    SetFieldsCount(WXSIZEOF(widths), widths);

    wxLogTrace(wxT("Function Start/End"), wxT("CStatusBar::CStatusBar - Function End"));
}


IMPLEMENT_DYNAMIC_CLASS(CAdvancedFrame, CBOINCBaseFrame)

BEGIN_EVENT_TABLE (CAdvancedFrame, CBOINCBaseFrame)
    EVT_MENU(ID_FILERUNBENCHMARKS, CAdvancedFrame::OnRunBenchmarks)
    EVT_MENU(ID_FILESELECTCOMPUTER, CAdvancedFrame::OnSelectComputer)
    EVT_MENU(ID_SHUTDOWNCORECLIENT, CAdvancedFrame::OnClientShutdown)
    EVT_MENU(ID_VIEWSIMPLEGUI, CAdvancedFrame::OnSwitchGUI)
    EVT_MENU(ID_READ_PREFS, CAdvancedFrame::Onread_prefs)
    EVT_MENU(ID_READ_CONFIG, CAdvancedFrame::Onread_config)
    EVT_MENU(wxID_EXIT, CAdvancedFrame::OnExit)
    EVT_MENU_RANGE(ID_FILEACTIVITYRUNALWAYS, ID_FILEACTIVITYSUSPEND, CAdvancedFrame::OnActivitySelection)
    EVT_MENU_RANGE(ID_FILENETWORKRUNALWAYS, ID_FILENETWORKSUSPEND, CAdvancedFrame::OnNetworkSelection)
    EVT_MENU(ID_PROJECTSATTACHACCOUNTMANAGER, CAdvancedFrame::OnProjectsAttachToAccountManager)
    EVT_MENU(ID_TOOLSAMUPDATENOW, CAdvancedFrame::OnAccountManagerUpdate)
    EVT_MENU(ID_ADVANCEDAMDEFECT, CAdvancedFrame::OnAccountManagerDetach)
    EVT_MENU(ID_PROJECTSATTACHPROJECT, CAdvancedFrame::OnProjectsAttachToProject)
    EVT_MENU(ID_COMMANDSRETRYCOMMUNICATIONS, CAdvancedFrame::OnCommandsRetryCommunications)
    EVT_MENU(ID_OPTIONSOPTIONS, CAdvancedFrame::OnOptionsOptions)
    EVT_MENU(ID_ADVPREFSDLG, CAdvancedFrame::OnDlgPreferences)
    EVT_HELP(wxID_ANY, CBOINCBaseFrame::OnContextHelp)
    EVT_MENU(ID_HELPBOINC, CBOINCBaseFrame::OnHelp)
    EVT_MENU(ID_HELPBOINCWEBSITE, CBOINCBaseFrame::OnHelp)
    EVT_MENU(wxID_ABOUT, CAdvancedFrame::OnHelpAbout)
    EVT_SIZE(CAdvancedFrame::OnSize)
    EVT_MOVE(CAdvancedFrame::OnMove)
    EVT_FRAME_REFRESH(CAdvancedFrame::OnRefreshView)
    EVT_FRAME_CONNECT(CAdvancedFrame::OnConnect)
    EVT_FRAME_UPDATESTATUS(CAdvancedFrame::OnUpdateStatus)
    EVT_TIMER(ID_REFRESHSTATETIMER, CAdvancedFrame::OnRefreshState)
    EVT_TIMER(ID_FRAMERENDERTIMER, CAdvancedFrame::OnFrameRender)
    EVT_TIMER(ID_FRAMELISTRENDERTIMER, CAdvancedFrame::OnListPanelRender)
    EVT_NOTEBOOK_PAGE_CHANGED(ID_FRAMENOTEBOOK, CAdvancedFrame::OnNotebookSelectionChanged)
END_EVENT_TABLE ()


CAdvancedFrame::CAdvancedFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Default Constructor Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Default Constructor Function End"));
}


CAdvancedFrame::CAdvancedFrame(wxString title, wxIcon* icon, wxIcon* icon32) : 
    CBOINCBaseFrame((wxFrame *)NULL, ID_ADVANCEDFRAME, title, wxDefaultPosition, wxDefaultSize,
                    wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Function Begin"));
    
    m_pMenubar = NULL;
    m_pNotebook = NULL;
    m_pStatusbar = NULL;

    // Working Variables
    m_strBaseTitle = title;
    m_bDisplayShutdownClientWarning = true;
    m_pageToLoad = 0;

    // Initialize Application
    wxIconBundle icons;
    icons.AddIcon(*icon);
    icons.AddIcon(*icon32);
    SetIcons(icons);

    // Create UI elements
    wxCHECK_RET(CreateMenu(), _T("Failed to create menu bar."));
    wxCHECK_RET(CreateNotebook(), _T("Failed to create notebook."));
    wxCHECK_RET(CreateStatusbar(), _T("Failed to create status bar."));

    // Restore view settings (without sizing frame)
    RestoreViewState();

    m_pRefreshStateTimer = new wxTimer(this, ID_REFRESHSTATETIMER);
    wxASSERT(m_pRefreshStateTimer);

    m_pFrameRenderTimer = new wxTimer(this, ID_FRAMERENDERTIMER);
    wxASSERT(m_pFrameRenderTimer);

    m_pFrameListPanelRenderTimer = new wxTimer(this, ID_FRAMELISTRENDERTIMER);
    wxASSERT(m_pFrameListPanelRenderTimer);

    m_pRefreshStateTimer->Start(300000);             // Send event every 5 minutes
    m_pFrameRenderTimer->Start(1000);                // Send event every 1 second
    m_pFrameListPanelRenderTimer->Start(1000);       // Send event every 1 second

    // Limit the number of times the UI can update itself to two times a second
    //   NOTE: Linux and Mac were updating several times a second and eating
    //         CPU time
    wxUpdateUIEvent::SetUpdateInterval(500);

    // We want to disconnect this later, so connect here instead of in the event table.
    Connect(wxEVT_IDLE, wxIdleEventHandler(CAdvancedFrame::OnIdleInit));

    RestoreState();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CAdvancedFrame - Function End"));
}


CAdvancedFrame::~CAdvancedFrame() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::~CAdvancedFrame - Function Begin"));

    wxASSERT(m_pRefreshStateTimer);
    wxASSERT(m_pFrameRenderTimer);
    wxASSERT(m_pFrameListPanelRenderTimer);
    wxASSERT(m_pMenubar);
    wxASSERT(m_pNotebook);
    wxASSERT(m_pStatusbar);

    SaveState();
    SaveViewState();

    if (m_pRefreshStateTimer) {
        m_pRefreshStateTimer->Stop();
        delete m_pRefreshStateTimer;
        m_pRefreshStateTimer = NULL;
    }

    if (m_pFrameRenderTimer) {
        m_pFrameRenderTimer->Stop();
        delete m_pFrameRenderTimer;
        m_pFrameRenderTimer = NULL;
    }

    if (m_pFrameListPanelRenderTimer) {
        m_pFrameListPanelRenderTimer->Stop();
        delete m_pFrameListPanelRenderTimer;
        m_pFrameListPanelRenderTimer = NULL;
    }

    if (m_pStatusbar) {
        DeleteStatusbar();
    }

    if (m_pNotebook) {
        DeleteNotebook();
    }

    if (m_pMenubar) {
        DeleteMenu();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::~CAdvancedFrame - Function End"));
}


bool CAdvancedFrame::CreateMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateMenu - Function Begin"));

    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    ACCT_MGR_INFO      ami;
    bool               is_acct_mgr_detected = false;
    wxString           strMenuName;
    wxString           strMenuDescription;
    
    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    // Account managers have a different menu arrangement
    pDoc->rpc.acct_mgr_info(ami);
    is_acct_mgr_detected = !ami.acct_mgr_url.empty();

    // File menu
    wxMenu *menuFile = new wxMenu;

    strMenuDescription.Printf(
        _("Close %s window."), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuFile->Append(
        ID_FILECLOSEWINDOW,
        _("&Close Window\tCTRL+W"),
        strMenuDescription
    );

    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuDescription.Printf(
        _("Exit %s."), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuFile->Append(
        wxID_EXIT,
        _("E&xit"),
        strMenuDescription
    );

    // View menu
    wxMenu *menuView = new wxMenu;

    menuView->AppendRadioItem(
        ID_VIEWADVANCEDGUI,
        _("&Advanced View"),
        _("Display the advanced interface.")
    );

    menuView->AppendRadioItem(
        ID_VIEWSIMPLEGUI,
        _("&Simple View"),
        _("Display a simpler view for less technical users.")
    );

    // Screen too small?
    if (wxGetDisplaySize().GetHeight() < 600) {
        menuView->Enable(ID_VIEWSIMPLEGUI, false);
    }
    
    menuView->Check(ID_VIEWADVANCEDGUI, true);

    // Tools menu
    wxMenu *menuTools = new wxMenu;

    if (!is_acct_mgr_detected) {
        menuTools->Append(
            ID_PROJECTSATTACHPROJECT, 
            _("Attach to &project..."),
            _("Attach to a project")
        );
        menuTools->Append(
            ID_PROJECTSATTACHACCOUNTMANAGER, 
            _("Attach to &account manager..."),
            _("Attach to an account manager")
        );
    } else {
        strMenuName.Printf(
            _("&Synchronize with %s"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        strMenuDescription.Printf(
            _("Get current settings from %s"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        menuTools->Append(
            ID_TOOLSAMUPDATENOW, 
            strMenuName,
            strMenuDescription
        );
    }

    // Activity menu
    wxMenu *menuActivity = new wxMenu;

    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYRUNALWAYS,
        _("&Run always"),
        _("Allow work regardless of preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYRUNBASEDONPREPERENCES,
        _("Run based on &preferences"),
        _("Allow work according to your preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILEACTIVITYSUSPEND,
        _("&Suspend"),
        _("Stop work regardless of preferences")
    );

#if defined(__WXMSW__) || defined(__WXMAC__)
    menuActivity->AppendSeparator();
#else
    // for some reason, the above radio items do not display the active
    // selection on linux (wxGtk library) with the separator here,
    // so we add a blank disabled menu item instead
    //
    wxMenuItem* pItem = menuActivity->Append(
        ID_ACTIVITYMENUSEPARATOR,
        (const wxChar *) wxT(" "),
            // wxEmptyString here causes a wxWidgets assertion when debugging
        wxEmptyString,
        wxITEM_NORMAL
            // wxITEM_SEPARATOR here causes a wxWidgets assertion when debugging
    );
    pItem->Enable(false); // disable this menu item
#endif

    menuActivity->AppendRadioItem(
        ID_FILENETWORKRUNALWAYS,
        _("&Network activity always available"),
        _("Allow network activity regardless of preferences")
    );
    menuActivity->AppendRadioItem(
        ID_FILENETWORKRUNBASEDONPREPERENCES,
        _("Network activity based on &preferences"),
        _("Allow network activity according to your preferences")
    );
    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuDescription.Printf(
        _("Stop %s network activity"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuActivity->AppendRadioItem(
        ID_FILENETWORKSUSPEND,
        _("&Network activity suspended"),
        strMenuDescription
    );

    // Advanced menu
    wxMenu *menuAdvanced = new wxMenu;
    menuAdvanced->Append(
        ID_OPTIONSOPTIONS, 
        _("&Options..."),
        _("Configure GUI options and proxy settings")
    );
    menuAdvanced->Append(
        ID_ADVPREFSDLG, 
        _("&Preferences..."),
        _("Configure local preferences")
    );

    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuDescription.Printf(
        _("Connect to another computer running %s"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuAdvanced->Append(
        ID_FILESELECTCOMPUTER, 
        _("Select computer..."),
        strMenuDescription
    );
    menuAdvanced->Append(
        ID_SHUTDOWNCORECLIENT, 
        _("Shut down connected client..."),
        _("Shut down the currently connected core client")
    );
    strMenuDescription.Printf(
        _("Runs %s CPU benchmarks"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuAdvanced->Append(
        ID_FILERUNBENCHMARKS, 
        _("Run CPU &benchmarks"),
        strMenuDescription
    );
    menuAdvanced->Append(
        ID_COMMANDSRETRYCOMMUNICATIONS, 
        _("Do network &communication"),
        _("Do all pending network communication.")
    );
    menuAdvanced->Append(
        ID_READ_CONFIG, 
        _("Read config file"),
        _("Read configuration info from cc_config.xml.")
    );
    menuAdvanced->Append(
        ID_READ_PREFS, 
        _("Read local prefs file"),
        _("Read preferences from global_prefs_override.xml.")
    );
    if (is_acct_mgr_detected) {
        strMenuName.Printf(
            _("&Stop using %s..."), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        menuAdvanced->Append(
            ID_ADVANCEDAMDEFECT, 
            strMenuName,
            _("Remove client from account manager control.")
        );
        menuAdvanced->Append(
            ID_PROJECTSATTACHPROJECT, 
            _("Attach to &project"),
            _("Attach to a project to begin processing work")
        );
    }


    // Help menu
    wxMenu *menuHelp = new wxMenu;

    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuName.Printf(
        _("%s &help"),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuDescription.Printf(
        _("Get help and support for %s."), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINC,
        strMenuName,
        strMenuDescription
    );

    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuName.Printf(
        _("%s &website"), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuDescription.Printf(
        _("Show %s website."),
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuHelp->Append(
        ID_HELPBOINCWEBSITE,
        strMenuName,
        strMenuDescription
    );

#ifndef __WXMAC__
    menuHelp->AppendSeparator();
#endif

    // %s is the project name
    //    i.e. 'Synecdoche', 'GridRepublic'
    strMenuName.Printf(
        _("&About %s..."), 
        pSkinAdvanced->GetApplicationShortName().c_str()
    );
    menuHelp->Append(
        wxID_ABOUT,
        strMenuName, 
        _("Licensing and copyright information.")
    );

    // construct menu
    m_pMenubar = new wxMenuBar;
    m_pMenubar->Append(
        menuFile,
        _("&File")
    );
    m_pMenubar->Append(
        menuView,
        _("&View")
    );
    m_pMenubar->Append(
        menuTools,
        _("&Tools")
    );
    m_pMenubar->Append(
        menuActivity,
        _("&Activity")
    );
    m_pMenubar->Append(
        menuAdvanced,
        _("A&dvanced")
    );
    m_pMenubar->Append(
        menuHelp,
        _("&Help")
    );

    wxMenuBar* m_pOldMenubar = GetMenuBar();
    SetMenuBar(m_pMenubar);
#ifdef __WXMAC__
    m_pMenubar->MacInstallMenuBar();
#endif
    if (m_pOldMenubar) {
        delete m_pOldMenubar;
    }
    
#ifdef __WXMAC__
    MenuRef prefsMenuRef;
    MenuItemIndex prefsMenuItemIndex;

    // Hide Mac OS X's standard Preferences menu item, since we don't use it
    if (GetIndMenuItemWithCommandID(NULL, kHICommandPreferences, 1, &prefsMenuRef, &prefsMenuItemIndex) == noErr)
        ChangeMenuItemAttributes(prefsMenuRef, prefsMenuItemIndex, kMenuItemAttrHidden, 0);
    
    // Set HELP key as keyboard shortcut
    m_Shortcuts[0].Set(wxACCEL_NORMAL, WXK_HELP, ID_HELPBOINCMANAGER);
    m_pAccelTable = new wxAcceleratorTable(1, m_Shortcuts);
    SetAcceleratorTable(*m_pAccelTable);
 #endif

    UpdateMenuBarState(pDoc->IsConnected());

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateMenu - Function End"));
    return true;
}


bool CAdvancedFrame::CreateNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebook - Function Begin"));

    // create frame panel
    wxPanel *pPanel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize,
                                 wxTAB_TRAVERSAL|wxCLIP_CHILDREN|wxNO_BORDER);
    pPanel->SetAutoLayout(TRUE);

    // initialize notebook
    m_pNotebook = new wxNotebook(pPanel, ID_FRAMENOTEBOOK, wxDefaultPosition, wxDefaultSize,
                                wxNB_FIXEDWIDTH|wxCLIP_CHILDREN);

    // layout frame panel
    wxBoxSizer *pPanelSizer = new wxBoxSizer(wxVERTICAL);

    pPanelSizer->Add(new wxStaticLine(pPanel, -1), 0, wxEXPAND);
    pPanelSizer->Add(0, 5);
    pPanelSizer->Add(m_pNotebook, 1, wxEXPAND);

    // Display default views
    RepopulateNotebook();

    pPanel->SetSizer(pPanelSizer);
    pPanel->Layout();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebook - Function End"));
    return true;
}


bool CAdvancedFrame::RepopulateNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RepopulateNotebook - Function Begin"));

    Freeze();
    DeleteNotebook();

    // Create the various notebook pages
    CreateNotebookPage(new CViewProjects(m_pNotebook));
    CreateNotebookPage(new CViewWork(m_pNotebook));
    CreateNotebookPage(new CViewTransfers(m_pNotebook));
    CreateNotebookPage(new CViewMessages(m_pNotebook));
    CreateNotebookPage(new CViewStatistics(m_pNotebook));
    CreateNotebookPage(new CViewResources(m_pNotebook));

    m_pNotebook->SetSelection(0);
    Thaw();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RepopulateNotebook - Function End"));
    return true;
}


bool CAdvancedFrame::CreateNotebookPage(CBOINCBaseView* pwndNewNotebookPage) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebookPage - Function Begin"));

    wxImageList*    pImageList;
    int         iImageIndex = 0;

    wxASSERT(pwndNewNotebookPage);
    wxASSERT(m_pNotebook);


    pImageList = m_pNotebook->GetImageList();
    if (!pImageList) {
        pImageList = new wxImageList(16, 16, true, 0);
        wxASSERT(pImageList != NULL);
        m_pNotebook->AssignImageList(pImageList);
    }
    
    iImageIndex = pImageList->Add(wxBitmap(pwndNewNotebookPage->GetViewIcon()));
    m_pNotebook->InsertPage(
        m_pNotebook->GetPageCount(),
        pwndNewNotebookPage,
        pwndNewNotebookPage->GetViewDisplayName(),
        false,
        iImageIndex);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateNotebookPage - Function End"));
    return true;
}


bool CAdvancedFrame::CreateStatusbar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateStatusbar - Function Begin"));

    if (m_pStatusbar)
        return true;

    m_pStatusbar = new CStatusBar(this);
    wxASSERT(m_pStatusbar);

    SetStatusBar(m_pStatusbar);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::CreateStatusbar - Function End"));
    return true;
}


void CAdvancedFrame::DeleteMenu() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteMenu - Function Begin"));
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteMenu - Function End"));
}


void CAdvancedFrame::DeleteNotebook() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteNotebook - Function Begin"));

    wxASSERT(m_pNotebook);

    // Delete all existing pages
    m_pNotebook->DeleteAllPages();

    // Delete all existing images
    wxImageList* pImageList = m_pNotebook->GetImageList();
    if (pImageList) {
        pImageList->RemoveAll();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteNotebook - Function End"));
}


void CAdvancedFrame::DeleteStatusbar() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteStatusbar - Function Begin"));

    if (!m_pStatusbar)
        return;

    SetStatusBar(NULL);

    delete m_pStatusbar;
    m_pStatusbar = NULL;

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::DeleteStatusbar - Function End"));
}


bool CAdvancedFrame::SaveState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::SaveState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;


    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);


    CBOINCBaseFrame::SaveState();


    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Save Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("DisplayShutdownClientWarning"), m_bDisplayShutdownClientWarning);

    // Retrieve and store the latest window dimensions.
    SaveWindowDimensions();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::SaveState - Function End"));
    return true;
}


bool CAdvancedFrame::SaveViewState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::SaveViewState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxWindow*       pwndNotebookPage = NULL;
    CBOINCBaseView* pView = NULL;
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;

    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);

    // An odd case happens every once and awhile where wxWidgets looses
    // the pointer to the config object, or it is cleaned up before
    // the window has finished it's cleanup duty.  If we detect a NULL
    // pointer, return false.
    if (!pConfig) {
        return false;
    }

    // Save Frame State
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("CurrentPage"), m_pNotebook->GetSelection());

    // Save Page(s) State
    // Convert to a zero based index
    size_t iItemCount = m_pNotebook->GetPageCount() - 1;

    for (size_t iIndex = 0; iIndex <= iItemCount; ++iIndex) {
        pwndNotebookPage = m_pNotebook->GetPage(iIndex);
        wxASSERT(wxDynamicCast(pwndNotebookPage, CBOINCBaseView));

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        strPreviousLocation = pConfig->GetPath();
        strConfigLocation = strPreviousLocation + pView->GetViewName();

        pConfig->SetPath(strConfigLocation);
        pView->FireOnSaveState(pConfig);
        pConfig->SetPath(strPreviousLocation);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::SaveViewState - Function End"));
    return true;
}


bool CAdvancedFrame::RestoreState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RestoreState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    wxString        strValue;


    wxASSERT(pConfig);


    CBOINCBaseFrame::RestoreState();


    // An odd case happens every once and awhile where wxWidgets looses
    //   the pointer to the config object, or it is cleaned up before
    //   the window has finished it's cleanup duty.  If we detect a NULL
    //   pointer, return false.
    if (!pConfig) return false;

    //
    // Restore Frame State
    //
    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("DisplayShutdownClientWarning"), &m_bDisplayShutdownClientWarning, true);

    RestoreWindowDimensions();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RestoreState - Function End"));
    return true;
}


bool CAdvancedFrame::RestoreViewState() {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RestoreViewState - Function Begin"));

    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    wxString        strConfigLocation;
    wxString        strPreviousLocation;
    wxString        strBuffer;
    wxString        strValue;
    long            iCurrentPage;

    wxASSERT(pConfig);
    wxASSERT(m_pNotebook);

    CBOINCBaseFrame::RestoreState();

    // An odd case happens every once and awhile where wxWidgets looses
    // the pointer to the config object, or it is cleaned up before
    // the window has finished it's cleanup duty.  If we detect a NULL
    // pointer, return false.
    if (!pConfig) {
        return false;
    }

    // Restore Frame State
    pConfig->SetPath(strBaseConfigLocation);

    if (wxGetApp().GetSkinManager()->GetAdvanced()->GetDefaultTab()) {
        m_pNotebook->SetSelection(wxGetApp().GetSkinManager()->GetAdvanced()->GetDefaultTab());
    } else {
        pConfig->Read(wxT("CurrentPage"), &iCurrentPage, (ID_LIST_WORKVIEW - ID_LIST_BASE));
        m_pNotebook->SetSelection(iCurrentPage);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::RestoreViewState - Function End"));
    return true;
}


void CAdvancedFrame::SaveWindowDimensions() {
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Write(wxT("Width"), m_windowRect.GetWidth());
    pConfig->Write(wxT("Height"), m_windowRect.GetHeight());

#ifdef __WXMAC__
    pConfig->Write(wxT("XPos"), m_windowRect.GetX());
    pConfig->Write(wxT("YPos"), m_windowRect.GetY());
#endif
}


void CAdvancedFrame::RestoreWindowDimensions() {
    wxString        strBaseConfigLocation = wxString(wxT("/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int             iHeight = 0;
    int             iWidth = 0;
    int             iTop = 0;
    int             iLeft = 0;

    wxASSERT(pConfig);

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("YPos"), &iTop, 30);
    pConfig->Read(wxT("XPos"), &iLeft, 30);
    pConfig->Read(wxT("Width"), &iWidth, 800);
    pConfig->Read(wxT("Height"), &iHeight, 600);

#ifndef __WXMAC__

    // Window is always restored to the last normal size.
    // Position is ignored.
    SetSize(-1, -1, iWidth, iHeight);

#else   // __WXMAC__

    // If the user has changed the arrangement of multiple 
    // displays, make sure the window title bar is still on-screen.
    Rect titleRect = {iTop, iLeft, iTop+22, iLeft+iWidth };
    InsetRect(&titleRect, 5, 5);    // Make sure at least a 5X5 piece visible
    RgnHandle displayRgn = NewRgn();
    CopyRgn(GetGrayRgn(), displayRgn);  // Region encompassing all displays
    Rect menuRect = ((**GetMainDevice())).gdRect;
    menuRect.bottom = GetMBarHeight() + menuRect.top;
    RgnHandle menuRgn = NewRgn();
    RectRgn(menuRgn, &menuRect);                // Region hidden by menu bar
    DiffRgn(displayRgn, menuRgn, displayRgn);   // Subtract menu bar region
    if (!RectInRgn(&titleRect, displayRgn))
        iTop = iLeft = 30;
    DisposeRgn(menuRgn);
    DisposeRgn(displayRgn);

    SetSize(iLeft, iTop, iWidth, iHeight);

#endif
}


void CAdvancedFrame::OnActivitySelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnActivitySelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    switch(event.GetId()) {
    case ID_FILEACTIVITYRUNALWAYS:
        pDoc->SetActivityRunMode(RUN_MODE_ALWAYS, 0);
        break;
    case ID_FILEACTIVITYSUSPEND:
        pDoc->SetActivityRunMode(RUN_MODE_NEVER, 0);
        break;
    case ID_FILEACTIVITYRUNBASEDONPREPERENCES:
        pDoc->SetActivityRunMode(RUN_MODE_AUTO, 0);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnActivitySelection - Function End"));
}


void CAdvancedFrame::OnNetworkSelection(wxCommandEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNetworkSelection - Function Begin"));

    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    switch(event.GetId()) {
    case ID_FILENETWORKRUNALWAYS:
        pDoc->SetNetworkRunMode(RUN_MODE_ALWAYS, 0);
        break;
    case ID_FILENETWORKSUSPEND:
        pDoc->SetNetworkRunMode(RUN_MODE_NEVER, 0);
        break;
    case ID_FILENETWORKRUNBASEDONPREPERENCES:
        pDoc->SetNetworkRunMode(RUN_MODE_AUTO, 0);
        break;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNetworkSelection - Function End"));
}

   
void CAdvancedFrame::OnRunBenchmarks(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnRunBenchmarks - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
    pDoc->RunBenchmarks();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnRunBenchmarks - Function End"));
}


void CAdvancedFrame::OnSelectComputer(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSelectComputer - Function Begin"));

    CMainDocument*      pDoc = wxGetApp().GetDocument();
    CDlgSelectComputer  dlg(this);
    int                 retVal = -1;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));


    // Lets copy the template store in the system state
    wxArrayString aComputerNames = m_aSelectedComputerMRU;

    // Lets populate the combo control with the MRU list
    dlg.SetMRUList(aComputerNames);

    if (wxID_OK == dlg.ShowModal()) {

        UpdateMenuBarState(false);

        // Make a null hostname be the same thing as localhost
        if (wxEmptyString == dlg.GetComputerName()) {
            retVal = pDoc->Connect(
                wxT("localhost"),
                GUI_RPC_PORT,
                "",
                true,
                true
            );
        } else {
            // Connect to the remote machine
            wxString sHost = dlg.GetComputerName(); 
            long lPort = GUI_RPC_PORT; 
            size_t pos = sHost.find(wxT(":")); 
            if (pos != wxString::npos) { 
                wxString sPort = sHost.substr(pos + 1); 
                if (!sPort.ToLong(&lPort)) lPort = GUI_RPC_PORT; 
                sHost.erase(pos); 
            } 
            retVal = pDoc->Connect(
                sHost,
                (int)lPort,
                (const char*)dlg.GetComputerPassword().mb_str(),
                true,
                false
            );
        }
        if (retVal) {
            ShowConnectionFailedAlert();
        }

        // Insert a copy of the current combo box value to the head of the
        //   computer names string array
        if (wxEmptyString != dlg.GetComputerName()) {
            aComputerNames.Insert(dlg.GetComputerName(), 0);
        }

        // Loops through the computer names and remove any duplicates that
        //   might exist with the new head value
        if (!aComputerNames.empty()) {
            for (size_t index = aComputerNames.Count() - 1; index > 0; --index) {
                if (aComputerNames.Item(index) == aComputerNames.Item(0))
                    aComputerNames.RemoveAt(index);
            }
        }

        // Store the modified computer name MRU list back to the system state
        m_aSelectedComputerMRU = aComputerNames;
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSelectComputer - Function End"));
}


void CAdvancedFrame::OnClientShutdown(wxCommandEvent& WXUNUSED(event)) {
    wxCommandEvent     evtSelectNewComputer(wxEVT_COMMAND_MENU_SELECTED, ID_FILESELECTCOMPUTER);
    CMainDocument*     pDoc = wxGetApp().GetDocument();
    CSkinAdvanced*     pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CDlgGenericMessage dlg(this);
    wxString           strDialogTitle = wxEmptyString;
    wxString           strDialogMessage = wxEmptyString;
    bool               shutdown = true;

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnClientShutdown - Function Begin"));

    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    // Stop all timers
    StopTimers();

    if (m_bDisplayShutdownClientWarning) {

        // %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogTitle.Printf(
            _("%s - Shutdown the current client..."),
            pSkinAdvanced->GetApplicationName().c_str()
        );

        // 1st %s is the application name
        //    i.e. 'BOINC Manager', 'GridRepublic Manager'
        strDialogMessage.Printf(
            _("%s will shut down the currently connected client,\n"
              "and prompt you for another host to connect to.\n"),
            pSkinAdvanced->GetApplicationName().c_str()
        );

        dlg.SetTitle(strDialogTitle);
        dlg.m_DialogMessage->SetLabel(strDialogMessage);
        dlg.Fit();
        dlg.Centre();

        if (wxID_OK == dlg.ShowModal()) {
            if (dlg.m_DialogDisableMessage->GetValue()) {
                m_bDisplayShutdownClientWarning = false;
            }
        } else {
            shutdown = false;
        }
    }

    StartTimers();

    if (shutdown) {
        pDoc->CoreClientQuit();
        pDoc->ForceDisconnect();

        // Since the core client we were connected to just shutdown, prompt for a new one.
        AddPendingEvent(evtSelectNewComputer);
    }

    // Restart timers


    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnClientShutdown - Function End"));
}


void CAdvancedFrame::Onread_prefs(wxCommandEvent& WXUNUSED(event)) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    pDoc->rpc.read_global_prefs_override();
}


void CAdvancedFrame::Onread_config(wxCommandEvent& WXUNUSED(event)) {
    CMainDocument* pDoc = wxGetApp().GetDocument();
    pDoc->rpc.read_cc_config();
}


void CAdvancedFrame::OnSize(wxSizeEvent& event) {

    // Minimising doesn't generate a size event on Windows, but other platforms may vary.
    // Only save the size if we are saving a normal window.
    if (!IsIconized() && !IsMaximized()) {
        m_windowRect.SetSize(event.GetSize());
    }

    event.Skip();
}


void CAdvancedFrame::OnMove(wxMoveEvent& event) {

    // Only save the position if we are saving a normal window.
    if (!IsIconized() && !IsMaximized()) {
        m_windowRect.SetTopLeft(event.GetPosition());
    }

    event.Skip();
}


void CAdvancedFrame::OnSwitchGUI(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSwitchGUI - Function Begin"));

    // Screen too small?
    if (wxGetDisplaySize().GetHeight() >= 600) {
        wxGetApp().SetActiveGUI(BOINC_SIMPLEGUI, true);
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnSwitchGUI - Function End"));
}


void CAdvancedFrame::OnProjectsAttachToAccountManager(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToAccountManager - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        // Stop all timers so that the wizard is the only thing doing anything
        StopTimers();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_ATTACH);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

        // Restart timers to continue normal operations.
        StartTimers();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToAccountManager - Function End"));
}


void CAdvancedFrame::OnAccountManagerUpdate(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerUpdate - Function Begin"));

    CMainDocument*            pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        // Stop all timers so that the wizard is the only thing doing anything
        StopTimers();

        CWizardAccountManager* pWizard = new CWizardAccountManager(this);

        pWizard->Run(ACCOUNTMANAGER_UPDATE);

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();
        FireRefreshView();
        ResetReminderTimers();

        // Restart timers to continue normal operations.
        StartTimers();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerUpdate - Function End"));
}


void CAdvancedFrame::OnAccountManagerDetach(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerDetach - Function Begin"));

    CMainDocument* pDoc           = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    wxInt32        iAnswer        = 0; 
    wxString       strTitle       = wxEmptyString;
    wxString       strMessage     = wxEmptyString;
    ACCT_MGR_INFO  ami;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {

        pDoc->rpc.acct_mgr_info(ami);

        strTitle.Printf(
            _("%s - Detach from %s"),
            pSkinAdvanced->GetApplicationName().c_str(),
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );
        strMessage.Printf(
            _("If you stop using %s,\n"
              "you'll keep all your current projects,\n"
              "but you'll have to manage projects manually.\n"
              "\n"
              "Do you want to stop using %s?"), 
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str(),
            wxString(ami.acct_mgr_name.c_str(), wxConvUTF8).c_str()
        );

        iAnswer = ::wxMessageBox(
            strMessage,
            strTitle,
            wxYES_NO | wxICON_QUESTION,
            this
        );

        if (wxYES == iAnswer) {
            std::string url, name, passwd;
            pDoc->rpc.acct_mgr_rpc(
                url.c_str(),
                name.c_str(),
                passwd.c_str(),
                false
            );
        }

        DeleteMenu();
        CreateMenu();
        FireRefreshView();

    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnAccountManagerDetach - Function End"));
}


void CAdvancedFrame::OnProjectsAttachToProject( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if (!pDoc->IsUserAuthorized())
        return;

    if (pDoc->IsConnected()) {
        UpdateStatusText(_("Attaching to project..."));

        // Stop all timers so that the wizard is the only thing doing anything
        StopTimers();

        CWizardAttachProject* pWizard = new CWizardAttachProject(this);

        wxString strName = wxEmptyString;
        wxString strURL = wxEmptyString;
        pWizard->Run( strName, strURL, false );

        if (pWizard)
            pWizard->Destroy();

        DeleteMenu();
        CreateMenu();

        // Restart timers to continue normal operations.
        StartTimers();

        UpdateStatusText(wxT(""));

        FireRefreshView();
    } else {
        ShowNotCurrentlyConnectedAlert();
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnProjectsAttachToProject - Function End"));
}


void CAdvancedFrame::OnCommandsRetryCommunications( wxCommandEvent& WXUNUSED(event) ) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnCommandsRetryCommunications - Function Begin"));

    CMainDocument* pDoc     = wxGetApp().GetDocument();
    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    UpdateStatusText(_("Retrying communications for project(s)..."));
    pDoc->rpc.network_available();
    UpdateStatusText(wxT(""));

    FireRefreshView();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnCommandsRetryCommunications - Function End"));
}


void CAdvancedFrame::OnDlgPreferences(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnDlgPreferences - Function Begin"));
    CDlgAdvPreferences dlg(this);
    dlg.ShowModal();
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnDlgPreferences - Function End"));
}

void CAdvancedFrame::OnOptionsOptions(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnOptionsOptions - Function Begin"));

    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CDlgOptions    dlg(this);
    int            iBuffer = 0;
    wxString       strBuffer = wxEmptyString;
    wxArrayString  astrDialupConnections;
    bool           bRetrievedProxyConfiguration = false;

    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));


    // General Tab
    dlg.m_LanguageSelectionCtrl->Append(wxGetApp().GetSupportedLanguages());

    dlg.m_LanguageSelectionCtrl->SetSelection(m_iSelectedLanguage);
    dlg.m_ReminderFrequencyCtrl->SetValue(m_iReminderFrequency);

#ifdef __WXMSW__
    // Connection Tab
    if (m_pDialupManager) {
        m_pDialupManager->GetISPNames(astrDialupConnections);

        dlg.m_DialupConnectionsCtrl->Append(astrDialupConnections);
        dlg.SetDefaultDialupConnection(m_strNetworkDialupConnectionName);
    } else {
        dlg.m_DialupSetDefaultCtrl->Disable();
        dlg.m_DialupClearDefaultCtrl->Disable();
    }
#endif

    // Proxy Tabs
    bRetrievedProxyConfiguration = (0 == pDoc->GetProxyConfiguration());
    if(!bRetrievedProxyConfiguration) {
        // We were unable to get the proxy configuration, so disable
        //   the controls
        dlg.m_EnableHTTPProxyCtrl->Enable(false);
        dlg.m_EnableSOCKSProxyCtrl->Enable(false);
    } else {
        dlg.m_EnableHTTPProxyCtrl->Enable(true);
        dlg.m_EnableSOCKSProxyCtrl->Enable(true);
    }

    dlg.m_EnableHTTPProxyCtrl->SetValue(pDoc->proxy_info.use_http_proxy);
    dlg.m_HTTPAddressCtrl->SetValue(wxString(pDoc->proxy_info.http_server_name.c_str(), wxConvUTF8));
    dlg.m_HTTPUsernameCtrl->SetValue(wxString(pDoc->proxy_info.http_user_name.c_str(), wxConvUTF8));
    dlg.m_HTTPPasswordCtrl->SetValue(wxString(pDoc->proxy_info.http_user_passwd.c_str(), wxConvUTF8));

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.http_server_port);
    dlg.m_HTTPPortCtrl->SetValue(strBuffer);

    dlg.m_EnableSOCKSProxyCtrl->SetValue(pDoc->proxy_info.use_socks_proxy);
    dlg.m_SOCKSAddressCtrl->SetValue(wxString(pDoc->proxy_info.socks_server_name.c_str(), wxConvUTF8));
    dlg.m_SOCKSUsernameCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_name.c_str(), wxConvUTF8));
    dlg.m_SOCKSPasswordCtrl->SetValue(wxString(pDoc->proxy_info.socks5_user_passwd.c_str(), wxConvUTF8));

    strBuffer.Printf(wxT("%d"), pDoc->proxy_info.socks_server_port);
    dlg.m_SOCKSPortCtrl->SetValue(strBuffer);

    if (wxID_OK == dlg.ShowModal()) {
        // General Tab
        if (m_iSelectedLanguage != dlg.m_LanguageSelectionCtrl->GetSelection()) {
            wxString strDialogTitle;
            wxString strDialogMessage;

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogTitle.Printf(
                _("%s - Language Selection"),
                pSkinAdvanced->GetApplicationName().c_str()
            );

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogMessage.Printf(
                _("The %s's default language has been changed, in order for this "
                  "change to take affect you must restart the %s."),
                pSkinAdvanced->GetApplicationName().c_str(),
                pSkinAdvanced->GetApplicationName().c_str()
            );

            ShowAlert(
                strDialogTitle,
                strDialogMessage,
                wxOK | wxICON_INFORMATION
           );
        }

        m_iSelectedLanguage = dlg.m_LanguageSelectionCtrl->GetSelection();
        m_iReminderFrequency = dlg.m_ReminderFrequencyCtrl->GetValue();

        if (dlg.isResetWarningDialogs()) {
            m_bDisplayShutdownClientWarning = true;
            wxGetApp().SetDisplayExitWarning(1);
        }

#ifdef __WXMSW__
        // Connection Tab
        m_strNetworkDialupConnectionName = dlg.GetDefaultDialupConnection();
#endif

        // Proxy Tabs
        if (bRetrievedProxyConfiguration) {
            pDoc->proxy_info.use_http_proxy = dlg.m_EnableHTTPProxyCtrl->GetValue();
            pDoc->proxy_info.http_server_name = (const char*)dlg.m_HTTPAddressCtrl->GetValue().mb_str();
            pDoc->proxy_info.http_user_name = (const char*)dlg.m_HTTPUsernameCtrl->GetValue().mb_str();
            pDoc->proxy_info.http_user_passwd = (const char*)dlg.m_HTTPPasswordCtrl->GetValue().mb_str();

            strBuffer = dlg.m_HTTPPortCtrl->GetValue();
            strBuffer.ToLong((long*)&iBuffer);
            pDoc->proxy_info.http_server_port = iBuffer;

            pDoc->proxy_info.use_socks_proxy = dlg.m_EnableSOCKSProxyCtrl->GetValue();
            pDoc->proxy_info.socks_server_name = (const char*)dlg.m_SOCKSAddressCtrl->GetValue().mb_str();
            pDoc->proxy_info.socks5_user_name = (const char*)dlg.m_SOCKSUsernameCtrl->GetValue().mb_str();
            pDoc->proxy_info.socks5_user_passwd = (const char*)dlg.m_SOCKSPasswordCtrl->GetValue().mb_str();

            strBuffer = dlg.m_SOCKSPortCtrl->GetValue();
            strBuffer.ToLong((long*)&iBuffer);
            pDoc->proxy_info.socks_server_port = iBuffer;

            pDoc->SetProxyConfiguration();
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnOptionsOptions - Function End"));
}


void CAdvancedFrame::OnHelpAbout(wxCommandEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpAbout - Function Begin"));

    CDlgAbout dlg(this);
    dlg.ShowModal();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnHelpAbout - Function End"));
}


void CAdvancedFrame::OnRefreshView(CFrameEvent& WXUNUSED(event)) {
    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        if (IsShown()) {
            wxWindow*       pwndNotebookPage = NULL;
            CBOINCBaseView* pView = NULL;
            wxTimerEvent    timerEvent;

            wxASSERT(m_pNotebook);

            pwndNotebookPage = m_pNotebook->GetPage(m_pNotebook->GetSelection());
            wxASSERT(pwndNotebookPage);

            pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
            wxASSERT(pView);

            pView->FireOnListRender(timerEvent);
        }

        bAlreadyRunningLoop = false;
    }
}


void CAdvancedFrame::OnConnect(CFrameEvent& WXUNUSED(event)) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnConnect - Function Begin"));
    
    CMainDocument* pDoc = wxGetApp().GetDocument();
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
    CWizardAccountManager* pAMWizard = NULL;
    CWizardAttachProject* pAPWizard = NULL;
    wxString strName = wxEmptyString;
    wxString strURL = wxEmptyString;
    wxString strDialogTitle = wxEmptyString;
    wxString strDialogDescription = wxEmptyString;
    bool bCachedCredentials = false;
    ACCT_MGR_INFO ami;
    PROJECT_INIT_STATUS pis;
    CC_STATUS     status;

    wxASSERT(m_pNotebook);
    wxASSERT(pDoc);
    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    pDoc->GetCoreClientStatus(status);

    // Do we need to bug out to the simple view?
    if (status.simple_gui_only) {
        wxGetApp().SetActiveGUI(BOINC_SIMPLEGUI, true);
        return;
    }


    // Update the menus
    DeleteMenu();
    CreateMenu();

    // Stop all timers so that the wizard is the only thing doing anything
    StopTimers();


    // If we are connected to the localhost, run a really quick screensaver
    //   test to trigger a firewall popup.
    if (pDoc->IsLocalClient()) {
        wxGetApp().StartBOINCScreensaverTest();
    }


    pDoc->rpc.get_project_init_status(pis);
    pDoc->rpc.acct_mgr_info(ami);
    if (!ami.acct_mgr_url.empty() && !ami.have_credentials) {
        if (!IsShown()) {
            Show();
        }

        pAMWizard = new CWizardAccountManager(this);
        if (pAMWizard->Run()) {
#ifdef SYNEC_USE_TASKBARICON
            // If successful, hide the main window
            Hide();
#endif

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            strDialogTitle.Printf(
                _("%s"),
                pSkinAdvanced->GetApplicationName().c_str()
            );

            // %s is the application name
            //    i.e. 'BOINC Manager', 'GridRepublic Manager'
            // %s is the project name
            //    i.e. 'BOINC', 'GridRepublic'
            strDialogDescription.Printf(
                _("%s has successfully attached to %s"),
                pSkinAdvanced->GetApplicationName().c_str(),
                pSkinAdvanced->GetApplicationShortName().c_str()
            );

            ShowAlert(
                strDialogTitle,
                strDialogDescription,
                wxOK | wxICON_INFORMATION,
                true
            );
        } else {
            // If failure, display the messages tab
            m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
        }
    } else if ((!pis.url.empty() || (pDoc->GetProjectCount() == 0)) && !status.disallow_attach) {
        if (!IsShown()) {
            Show();
        }

        pAPWizard = new CWizardAttachProject(this);
        strName = wxString(pis.name.c_str(), wxConvUTF8);
        strURL = wxString(pis.url.c_str(), wxConvUTF8);
        bCachedCredentials = pis.url.length() && pis.has_account_key;

        if (pAPWizard->Run(strName, strURL, bCachedCredentials)) {
            // If successful, display the work tab
            m_pNotebook->SetSelection(ID_LIST_WORKVIEW - ID_LIST_BASE);
        } else {
            // If failure, display the messages tab
            m_pNotebook->SetSelection(ID_LIST_MESSAGESVIEW - ID_LIST_BASE);
        }
    }


    // Restart timers to continue normal operations.
    StartTimers();

    // Notify all notebook tabs about the successful connection attempt:
    for (size_t index = 0; index < m_pNotebook->GetPageCount(); ++index) {
        wxNotebookPage* page = m_pNotebook->GetPage(index);
        CBOINCBaseView* view = dynamic_cast<CBOINCBaseView*>(page);
        if (view) {
            view->OnConnect();
        }
    }

    // Set the correct refresh interval, then manually fire the refresh
    //   event to do the initial population of the view.
    UpdateRefreshTimerInterval(m_pNotebook->GetSelection());
    FireRefreshView();


    if (pAMWizard)
        pAMWizard->Destroy();
    if (pAPWizard)
        pAPWizard->Destroy();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnConnect - Function End"));
}


void CAdvancedFrame::OnUpdateStatus(CFrameEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnUpdateStatus - Function Begin"));

    m_pStatusbar->SetStatusText(event.m_message);
    ::wxSleep(0);

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnUpdateStatus - Function End"));
}


void CAdvancedFrame::OnRefreshState(wxTimerEvent &event) {
    static bool bAlreadyRunningLoop = false;

    if (!bAlreadyRunningLoop) {
        bAlreadyRunningLoop = true;

        // Write a snapshot of the current state to the config
        //   module, on Win9x systems we don't always shutdown
        //   in a nice way, if we are terminated by the user
        //   we still want the UI state to have been stored
        //   for their next use
        SaveState();

        bAlreadyRunningLoop = false;
    }

    event.Skip();
}


void CAdvancedFrame::OnFrameRender(wxTimerEvent &event) {
    static bool       bAlreadyRunningLoop = false;

    CMainDocument*    pDoc     = wxGetApp().GetDocument();
    wxMenuBar*        pMenuBar = GetMenuBar();


    if (!bAlreadyRunningLoop && m_pFrameRenderTimer->IsRunning()) {
        bAlreadyRunningLoop = true;

        if (IsShown()) {
            if (pDoc) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));
                wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));
                wxASSERT(wxDynamicCast(m_pStatusbar, CStatusBar));

                // Update the menu bar
                CC_STATUS  status;
                if ((pDoc->IsConnected()) && (0 == pDoc->GetCoreClientStatus(status))) {
                    UpdateActivityModeControls(status);
                    UpdateNetworkModeControls(status);

                    if (status.disallow_attach) {
                        pMenuBar->Enable(ID_PROJECTSATTACHPROJECT, false);
                    }
                }

                // Update the statusbar
                if (pDoc->IsConnected() || pDoc->IsReconnecting()) {

                    wxString strComputerName;
                    wxString strComputerVersion;
                    wxString strStatusText;
                    wxString strTitle = m_strBaseTitle;
                    wxString strLocale = wxString(setlocale(LC_NUMERIC, NULL), wxConvUTF8);

                    pDoc->GetConnectedComputerName(strComputerName);
                    if (!pDoc->IsReconnecting()) {
                        pDoc->GetConnectedComputerVersion(strComputerVersion);
                    }

                    if (pDoc->IsComputerNameLocal(strComputerName)) {
                        strComputerName = wxT("localhost");
                    }

                    // Only show machine name if connected to remote machine.
                    if (!pDoc->IsComputerNameLocal(strComputerName)) {
                        strTitle = strTitle + wxT(" - (") + strComputerName + wxT(")");
                    }

                    if (pDoc->IsReconnecting()) {
                        strStatusText.Printf(_("Connecting to %s"), strComputerName.c_str());
                    } else {
                        strStatusText.Printf(
                            _("Connected to %s (%s)"),
                            strComputerName.c_str(),
                            strComputerVersion.c_str()
                        );
                    }

                    // The Mac takes a huge performance hit redrawing this window, 
                    //   window, so don't change the text unless we really have too.
                    if (GetTitle() != strTitle) {
                        SetTitle(strTitle);
                    }

                    if (strStatusText != m_cachedStatusText) {
                        m_cachedStatusText = strStatusText;
                        SetStatusText(strStatusText, STATUS_CONNECTION_STATUS);
                    }
                } else {
                    m_cachedStatusText = _("Disconnected");
                    SetStatusText(m_cachedStatusText, STATUS_CONNECTION_STATUS);

                    if (GetTitle() != m_strBaseTitle)
                        SetTitle(m_strBaseTitle);
                }
            }
        }

        bAlreadyRunningLoop = false;
    }

    event.Skip();
}


void CAdvancedFrame::OnListPanelRender(wxTimerEvent& WXUNUSED(event)) {
    FireRefreshView();
}


void CAdvancedFrame::OnNotebookSelectionChanged(wxNotebookEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNotebookSelectionChanged - Function Begin"));

    if ((-1 != event.GetSelection())) {

        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        wxASSERT(m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage(event.GetSelection());
        wxASSERT(pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        pView->FireOnShowView();
        UpdateRefreshTimerInterval(event.GetSelection());
        FireRefreshView();
    }

    event.Skip();

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnNotebookSelectionChanged - Function End"));
}


void CAdvancedFrame::ResetReminderTimers() {
#ifdef __WXMSW__
    wxASSERT(m_pDialupManager);
    wxASSERT(wxDynamicCast(m_pDialupManager, CBOINCDialUpManager));

    m_pDialupManager->ResetReminderTimers();
#endif
}


void CAdvancedFrame::UpdateActivityModeControls(const CC_STATUS& status) {
    wxMenuBar* pMenuBar      = GetMenuBar();

    wxASSERT(pMenuBar);
    wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

    // Skip if everything is already setup, Linux and possibly a few other platforms
    //   will emulate a click event for a menu item even when the action of setting
    //   a controls value wasn't initiated via user interaction. This in turn causes
    //   the set_* RPC to be called which will cause the state file to become dirty.
    if ((RUN_MODE_ALWAYS == status.task_mode) && pMenuBar->IsChecked(ID_FILEACTIVITYRUNALWAYS)) return;
    if ((RUN_MODE_NEVER == status.task_mode) && pMenuBar->IsChecked(ID_FILEACTIVITYSUSPEND)) return;
    if ((RUN_MODE_AUTO == status.task_mode) && pMenuBar->IsChecked(ID_FILEACTIVITYRUNBASEDONPREPERENCES)) return;

    // Set things up.
    pMenuBar->Check(ID_FILEACTIVITYRUNALWAYS, false);
    pMenuBar->Check(ID_FILEACTIVITYSUSPEND, false);
    pMenuBar->Check(ID_FILEACTIVITYRUNBASEDONPREPERENCES, false);
    if (RUN_MODE_ALWAYS == status.task_mode)
        pMenuBar->Check(ID_FILEACTIVITYRUNALWAYS, true);
    if (RUN_MODE_NEVER == status.task_mode)
        pMenuBar->Check(ID_FILEACTIVITYSUSPEND, true);
    if (RUN_MODE_AUTO == status.task_mode)
        pMenuBar->Check(ID_FILEACTIVITYRUNBASEDONPREPERENCES, true);
}


void CAdvancedFrame::UpdateNetworkModeControls(const CC_STATUS& status) {
    wxMenuBar* pMenuBar      = GetMenuBar();

    wxASSERT(pMenuBar);
    wxASSERT(wxDynamicCast(pMenuBar, wxMenuBar));

    // Skip if everything is already setup, Linux and possibly a few other platforms
    //   will emulate a click event for a menu item even when the action of setting
    //   a controls value wasn't initiated via user interaction. This in turn causes
    //   the set_* RPC to be called which will cause the state file to become dirty.
    if ((RUN_MODE_ALWAYS == status.network_mode) && pMenuBar->IsChecked(ID_FILENETWORKRUNALWAYS)) return;
    if ((RUN_MODE_NEVER == status.network_mode) && pMenuBar->IsChecked(ID_FILENETWORKSUSPEND)) return;
    if ((RUN_MODE_AUTO == status.network_mode) && pMenuBar->IsChecked(ID_FILENETWORKRUNBASEDONPREPERENCES)) return;

    // Set things up.
    pMenuBar->Check(ID_FILENETWORKRUNALWAYS, false);
    pMenuBar->Check(ID_FILENETWORKSUSPEND, false);
    pMenuBar->Check(ID_FILENETWORKRUNBASEDONPREPERENCES, false);
    if (RUN_MODE_ALWAYS == status.network_mode)
        pMenuBar->Check(ID_FILENETWORKRUNALWAYS, true);
    if (RUN_MODE_NEVER == status.network_mode)
        pMenuBar->Check(ID_FILENETWORKSUSPEND, true);
    if (RUN_MODE_AUTO == status.network_mode)
        pMenuBar->Check(ID_FILENETWORKRUNBASEDONPREPERENCES, true);
}


void CAdvancedFrame::UpdateRefreshTimerInterval(wxInt32 iCurrentNotebookPage) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::UpdateRefreshTimerInterval - Function Begin"));

    if (IsShown()) {
        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;
        CMainDocument*  pDoc = wxGetApp().GetDocument();


        wxASSERT(m_pNotebook);

        pwndNotebookPage = m_pNotebook->GetPage(iCurrentNotebookPage);
        wxASSERT(pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        if (m_pFrameListPanelRenderTimer && m_pFrameListPanelRenderTimer->IsRunning()) {
            m_pFrameListPanelRenderTimer->Stop();

            // View specific refresh rates only apply when a connection to the core
            //   client has been established, otherwise the refresh rate should be 1
            //   second.
            if (pDoc) {
                wxASSERT(wxDynamicCast(pDoc, CMainDocument));
                if (pDoc->IsConnected()) {
                    // Set new view specific refresh rate
                    m_pFrameListPanelRenderTimer->Start(pView->GetViewRefreshRate() * 1000); 
                } else {
                    // Set view refresh rate to 1 second
                    m_pFrameListPanelRenderTimer->Start(1000); 
                }
            }
        }
    }

    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::UpdateRefreshTimerInterval - Function End"));
}


/// Preload notebook pages when the system is idle.
void CAdvancedFrame::OnIdleInit(wxIdleEvent& event) {
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnIdleInit - Function Begin"));
    wxASSERT(m_pNotebook);

    if (m_pageToLoad < m_pNotebook->GetPageCount()) {

        wxWindow*       pwndNotebookPage = NULL;
        CBOINCBaseView* pView = NULL;

        pwndNotebookPage = m_pNotebook->GetPage(m_pageToLoad);
        wxASSERT(pwndNotebookPage);

        pView = wxDynamicCast(pwndNotebookPage, CBOINCBaseView);
        wxASSERT(pView);

        // Demand load view (does nothing for loaded pages)
        pView->FireOnShowView();

        m_pageToLoad++;

        // Handle one page at a time. Make sure idle is called again, even if
        // there is nothing else in the event queue:
        event.RequestMore();

    } else {
        // Loading done, unlink event.
        Disconnect(wxEVT_IDLE, wxIdleEventHandler(CAdvancedFrame::OnIdleInit));
    }
    wxLogTrace(wxT("Function Start/End"), wxT("CAdvancedFrame::OnIdleInit - Function End"));
}


void CAdvancedFrame::StartTimers() {
    wxASSERT(m_pRefreshStateTimer);
    wxASSERT(m_pFrameRenderTimer);
    wxASSERT(m_pFrameListPanelRenderTimer);
    CBOINCBaseFrame::StartTimers();
    m_pRefreshStateTimer->Start();
    m_pFrameRenderTimer->Start();
    m_pFrameListPanelRenderTimer->Start();
}


void CAdvancedFrame::StopTimers() {
    wxASSERT(m_pRefreshStateTimer);
    wxASSERT(m_pFrameRenderTimer);
    wxASSERT(m_pFrameListPanelRenderTimer);
    CBOINCBaseFrame::StopTimers();
    m_pRefreshStateTimer->Stop();
    m_pFrameRenderTimer->Stop();
    m_pFrameListPanelRenderTimer->Stop();
}

/// Enable or disable certain menu items based on the connected state.
///
/// \param[in] connected If true the menu items will be enabled,
///                      otherwise disabled.
void CAdvancedFrame::UpdateMenuBarState(bool connected) {
    CMainDocument*  pDoc = wxGetApp().GetDocument();
    ACCT_MGR_INFO ami;
    pDoc->rpc.acct_mgr_info(ami);
    bool is_acct_mgr_detected = !ami.acct_mgr_url.empty();

    wxMenu* tools = m_pMenubar->GetMenu(2);
    if (!is_acct_mgr_detected) {
        tools->Enable(ID_PROJECTSATTACHPROJECT, connected);
        tools->Enable(ID_PROJECTSATTACHACCOUNTMANAGER, connected);
    } else {
        tools->Enable(ID_TOOLSAMUPDATENOW, connected);
    }

    wxMenu* activity = m_pMenubar->GetMenu(3);
    activity->Enable(ID_FILEACTIVITYRUNALWAYS, connected);
    activity->Enable(ID_FILEACTIVITYRUNBASEDONPREPERENCES, connected);
    activity->Enable(ID_FILEACTIVITYSUSPEND, connected);
    activity->Enable(ID_FILENETWORKRUNALWAYS, connected);
    activity->Enable(ID_FILENETWORKRUNBASEDONPREPERENCES, connected);
    activity->Enable(ID_FILENETWORKSUSPEND, connected);

    wxMenu* advanced = m_pMenubar->GetMenu(4);
    advanced->Enable(ID_ADVPREFSDLG, connected);
    advanced->Enable(ID_SHUTDOWNCORECLIENT, connected);
    advanced->Enable(ID_FILERUNBENCHMARKS, connected);
    advanced->Enable(ID_COMMANDSRETRYCOMMUNICATIONS, connected);
    advanced->Enable(ID_READ_CONFIG, connected);
    advanced->Enable(ID_READ_PREFS, connected);
    if (is_acct_mgr_detected) {
        advanced->Enable(ID_ADVANCEDAMDEFECT, connected);
        advanced->Enable(ID_PROJECTSATTACHPROJECT, connected);
    }
}
