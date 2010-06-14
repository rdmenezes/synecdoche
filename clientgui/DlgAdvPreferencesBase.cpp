// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2010 Nicolas Alvarez
// Copyright (C) 2010 University of California
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


// This code was initially generated with wxFormBuilder (version Oct 13 2006)
// http://www.wxformbuilder.org/

#include "DlgAdvPreferencesBase.h"

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "BuildLayout.h"

///////////////////////////////////////////////////////////////////////////

CDlgAdvPreferencesBase::CDlgAdvPreferencesBase( wxWindow* parent, int id, wxString title, wxPoint pos, wxSize size, int style ) :
    wxDialog( parent, id, title, pos, size, style )
{
    wxString strCaption = title;
    if (strCaption.IsEmpty()) {
        CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();
        wxASSERT(pSkinAdvanced);
        wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

        strCaption.Printf(_("%s - Preferences"), pSkinAdvanced->GetApplicationName().c_str());
    }

    this->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    this->Centre( wxBOTH );
    this->SetTitle(strCaption);

    wxBoxSizer* dialogSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* topControlsSizer = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("") ), wxHORIZONTAL );

    m_bmpWarning = new wxStaticBitmap( this, ID_DEFAULT, wxNullBitmap );
    m_bmpWarning->SetMinSize( wxSize( 48,48 ) );

    topControlsSizer->Add( m_bmpWarning, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    m_staticText321 = new wxStaticText( this, ID_DEFAULT, _("This dialog controls preferences for this computer only.\nClick OK to set preferences.\nClick Clear to restore web-based settings.") );
    topControlsSizer->Add( m_staticText321, 1, wxALL, 1 );

    m_btnClear = new wxButton( this, ID_BTN_CLEAR, _("Clear") );
    m_btnClear->SetToolTip( _("clear all local preferences and close the dialog") );

    topControlsSizer->Add( m_btnClear, 0, wxALIGN_BOTTOM|wxALL, 1 );

    dialogSizer->Add( topControlsSizer, 0, wxALL|wxEXPAND, 1 );

    m_panelControls = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelControls->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* notebookSizer = new wxBoxSizer( wxVERTICAL );

    m_Notebook = new wxNotebook( m_panelControls, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxNB_FLAT|wxNB_TOP );
    m_Notebook->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    m_panelProcessor = createProcessorTab(m_Notebook);
    m_Notebook->AddPage( m_panelProcessor, _("processor usage"), false );

    m_panelNetwork = createNetworkTab(m_Notebook);
    m_Notebook->AddPage( m_panelNetwork, _("network usage"), true );

    m_panelDiskAndMemory = createDiskAndMemoryTab(m_Notebook);
    m_Notebook->AddPage( m_panelDiskAndMemory, _("disk and memory usage"), false );

    notebookSizer->Add( m_Notebook, 1, wxEXPAND | wxALL, 1 );

    m_panelControls->SetSizer( notebookSizer );
    m_panelControls->Layout();
    notebookSizer->Fit( m_panelControls );
    dialogSizer->Add( m_panelControls, 1, wxALL|wxEXPAND, 1 );

    m_panelButtons = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("OK") );
    m_btnOK->SetToolTip( _("save all values and close the dialog") );

    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel") );
    m_btnCancel->SetToolTip( _("close the dialog without saving") );

    buttonSizer->Add( m_btnCancel, 0, wxALL, 5 );

    m_btnHelp = new wxButton( m_panelButtons, wxID_HELP, _("Help") );
    m_btnHelp->SetToolTip( _("shows the preferences web page") );

    buttonSizer->Add( m_btnHelp, 0, wxALL, 5 );

    m_panelButtons->SetSizer( buttonSizer );
    m_panelButtons->Layout();
    buttonSizer->Fit( m_panelButtons );
    dialogSizer->Add( m_panelButtons, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 1 );

    this->SetSizer( dialogSizer );
    this->Layout();
}

CDlgAdvPreferencesBase::~CDlgAdvPreferencesBase() {
}



wxPanel* CDlgAdvPreferencesBase::createProcessorTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel( parent, ID_TABPAGE_PROC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    panel->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* processorTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* computingAllowedBoxSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("Computing allowed") ), wxVERTICAL );

    m_chkProcOnBatteries = new wxCheckBox( panel, ID_CHKPROCONBATTERIES, _("While computer is on batteries") );

    m_chkProcOnBatteries->SetToolTip( _("check this if you want this computer to do work while it runs on batteries") );

    computingAllowedBoxSizer->Add( m_chkProcOnBatteries, 0, wxALL, 5 );

    m_chkProcInUse = new wxCheckBox( panel, ID_CHKPROCINUSE, _("While computer is in use") );

    m_chkProcInUse->SetToolTip( _("check this if you want this computer to do work even when you're using it") );

    computingAllowedBoxSizer->Add( m_chkProcInUse, 0, wxALL, 5 );

    wxBoxSizer* procIdleSizer = new wxBoxSizer(wxHORIZONTAL);

    m_txtProcIdleFor = new wxTextCtrl( panel, ID_TXTPROCIDLEFOR, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtProcIdleFor->SetToolTip( _("do work only after you haven't used the computer for this number of minutes") );

    buildLayout(panel, procIdleSizer, _("Only after computer has been idle for %1 minutes"), m_txtProcIdleFor);

    computingAllowedBoxSizer->Add( procIdleSizer, 0, wxEXPAND, 5 );

    wxBoxSizer* cpuTimesSizer = new wxBoxSizer( wxHORIZONTAL );

    m_txtProcEveryDayStart = new wxTextCtrl( panel, ID_TXTPROCEVERYDAYSTART, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtProcEveryDayStart->SetToolTip( _("start work at this time") );

    m_txtProcEveryDayStop = new wxTextCtrl( panel, ID_TXTPROCEVERYDAYSTOP, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtProcEveryDayStop->SetToolTip( _("stop work at this time") );

    buildLayout(panel, cpuTimesSizer, _("Every day between hours of %1 and %2 (no restriction if equal)"), m_txtProcEveryDayStart, m_txtProcEveryDayStop);

    computingAllowedBoxSizer->Add( cpuTimesSizer, 0, wxEXPAND, 1 );

    m_staticText36 = new wxStaticText( panel, ID_DEFAULT, _("Day-of-week override:") );
    computingAllowedBoxSizer->Add( m_staticText36, 0, wxALL, 5 );

    m_panelProcSpecialTimes = new wxPanel( panel, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panelProcSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    m_panelProcSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

    wxFlexGridSizer* procDaysSizer = new wxFlexGridSizer( 4, 4, 0, 0 );
    procDaysSizer->SetFlexibleDirection( wxHORIZONTAL );
    procDaysSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkProcMonday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCMONDAY, _("Monday") );

    procDaysSizer->Add( m_chkProcMonday, 0, wxALL, 5 );

    m_txtProcMonday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCMONDAY, wxT("") );
    procDaysSizer->Add( m_txtProcMonday, 0, wxALL, 1 );

    m_chkProcTuesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTUESDAY, _("Tuesday") );

    procDaysSizer->Add( m_chkProcTuesday, 0, wxALL, 5 );

    m_txtProcTuesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTUESDAY, wxT("") );
    procDaysSizer->Add( m_txtProcTuesday, 0, wxALL, 1 );

    m_chkProcWednesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCWEDNESDAY, _("Wednesday") );

    procDaysSizer->Add( m_chkProcWednesday, 0, wxALL, 5 );

    m_txtProcWednesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCWEDNESDAY, wxT("") );
    procDaysSizer->Add( m_txtProcWednesday, 0, wxALL, 1 );

    m_chkProcThursday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTHURSDAY, _("Thursday") );

    procDaysSizer->Add( m_chkProcThursday, 0, wxALL, 5 );

    m_txtProcThursday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTHURSDAY, wxT("") );
    procDaysSizer->Add( m_txtProcThursday, 0, wxALL, 1 );

    m_chkProcFriday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCFRIDAY, _("Friday") );

    procDaysSizer->Add( m_chkProcFriday, 0, wxALL, 5 );

    m_txtProcFriday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCFRIDAY, wxT("") );
    procDaysSizer->Add( m_txtProcFriday, 0, wxALL, 1 );

    m_chkProcSaturday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSATURDAY, _("Saturday") );

    procDaysSizer->Add( m_chkProcSaturday, 0, wxALL, 5 );

    m_txtProcSaturday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSATURDAY, wxT("") );
    procDaysSizer->Add( m_txtProcSaturday, 0, wxALL, 1 );

    m_chkProcSunday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSUNDAY, _("Sunday") );

    procDaysSizer->Add( m_chkProcSunday, 0, wxALL, 5 );

    m_txtProcSunday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSUNDAY, wxT("") );
    procDaysSizer->Add( m_txtProcSunday, 0, wxALL, 1 );

    m_panelProcSpecialTimes->SetSizer( procDaysSizer );
    m_panelProcSpecialTimes->Layout();
    procDaysSizer->Fit( m_panelProcSpecialTimes );
    computingAllowedBoxSizer->Add( m_panelProcSpecialTimes, 1, wxEXPAND | wxALL, 1 );

    processorTabSizer->Add( computingAllowedBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* miscProcBoxSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("Other options") ), wxVERTICAL );

    wxFlexGridSizer* miscProcGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    miscProcGridSizer->AddGrowableCol( 2 );
    miscProcGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    miscProcGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_txtProcSwitchEvery = new wxTextCtrl( panel, ID_TXTPROCSWITCHEVERY, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, miscProcGridSizer, _("Switch between applications between every %1 minutes"), m_txtProcSwitchEvery);

    m_txtProcUseProcessors = new wxTextCtrl( panel, ID_TXTPROCUSEPROCESSORS, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, miscProcGridSizer, _("On multiprocessor systems, use at most %1 % of the processors"), m_txtProcUseProcessors);

    m_txtProcUseCPUTime = new wxTextCtrl( panel, ID_TXTPOCUSECPUTIME, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, miscProcGridSizer, _("Use at most %1 % CPU time"), m_txtProcUseCPUTime);

    miscProcBoxSizer->Add( miscProcGridSizer, 0, wxEXPAND, 1 );

    processorTabSizer->Add( miscProcBoxSizer, 0, wxEXPAND, 1 );

    panel->SetSizer( processorTabSizer );
    panel->Layout();
    processorTabSizer->Fit( panel );

    return panel;
}
wxPanel* CDlgAdvPreferencesBase::createNetworkTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel( parent, ID_TABPAGE_NET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    panel->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* networkTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* networkGeneralBoxSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("General options") ), wxVERTICAL );

    wxFlexGridSizer* networkGeneralGridSizer = new wxFlexGridSizer( 3, 6, 0, 0 );
    networkGeneralGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    networkGeneralGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_txtNetDownloadRate = new wxTextCtrl( panel, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, networkGeneralGridSizer, _("Maximum download rate %1 KBytes/sec."), m_txtNetDownloadRate);

    m_txtNetUploadRate = new wxTextCtrl( panel, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, networkGeneralGridSizer, _("Maximum upload rate %1 KBytes/sec."), m_txtNetUploadRate);

    m_txtNetConnectInterval = new wxTextCtrl( panel, ID_TXTNETCONNECTINTERVAL, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtNetConnectInterval->SetToolTip( _("this computer is connected to the Internet about every X days\n(0 if it's always connected)") );
    buildLayout(panel, networkGeneralGridSizer, _("Connect about every %1 days"), m_txtNetConnectInterval);

    m_txtNetAdditionalDays = new wxTextCtrl( panel, ID_TXTNETADDITIONALDAYS, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, networkGeneralGridSizer, _("Additional work buffer %1 days (max. 10)"), m_txtNetAdditionalDays);

    m_chkNetSkipImageVerification = new wxCheckBox( panel, ID_CHKNETSKIPIMAGEVERIFICATION, _("Skip image file verification") );

    m_chkNetSkipImageVerification->SetToolTip( _("check this if your Internet provider modifies image files") );

    networkGeneralGridSizer->Add( m_chkNetSkipImageVerification, 0, wxALL, 5 );

    networkGeneralBoxSizer->Add( networkGeneralGridSizer, 0, wxEXPAND, 1 );

    networkTabSizer->Add( networkGeneralBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* connectOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("Connect options") ), wxVERTICAL );

    m_chkNetConfirmBeforeConnect = new wxCheckBox( panel, ID_CHKNETCONFIRMBEFORECONNECT, _("Confirm before connecting to internet") );

    m_chkNetConfirmBeforeConnect->SetToolTip( _("if checked, a confirmation dialog will be displayed before trying to connect to the Internet") );

    connectOptionsSizer->Add( m_chkNetConfirmBeforeConnect, 0, wxALL, 5 );

    m_chkNetDisconnectWhenDone = new wxCheckBox( panel, ID_CHKNETDISCONNECTWHENDONE, _("Disconnect when done") );

    m_chkNetDisconnectWhenDone->SetToolTip( _("if checked, hang up when network usage is done\n(only relevant for dialup-connection)") );

    connectOptionsSizer->Add( m_chkNetDisconnectWhenDone, 0, wxALL, 5 );

    networkTabSizer->Add( connectOptionsSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* networkTimesBoxSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("Network usage allowed") ), wxVERTICAL );

    wxBoxSizer* networkTimesSizer = new wxBoxSizer( wxHORIZONTAL );

    m_txtNetEveryDayStart = new wxTextCtrl( panel, ID_TXTNETEVERYDAYSTART, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), 0 );
    m_txtNetEveryDayStart->SetToolTip( _("network usage start hour") );

    m_txtNetEveryDayStop = new wxTextCtrl( panel, ID_TXTNETEVERYDAYSTOP, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), 0 );
    m_txtNetEveryDayStop->SetToolTip( _("network usage stop hour") );

    buildLayout(panel, networkTimesSizer, _("Every day between hours of %1 and %2 (no restriction if equal)"), m_txtNetEveryDayStart, m_txtNetEveryDayStop);

    networkTimesBoxSizer->Add( networkTimesSizer, 0, wxEXPAND, 1 );

    m_staticText39 = new wxStaticText( panel, ID_DEFAULT, _("Day-of-week override:") );
    networkTimesBoxSizer->Add( m_staticText39, 0, wxALL, 5 );

    m_panelNetSpecialTimes = new wxPanel( panel, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panelNetSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    m_panelNetSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

    wxFlexGridSizer* netDaysGridSizer = new wxFlexGridSizer( 4, 4, 0, 0 );
    netDaysGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    netDaysGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkNetMonday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETMONDAY, _("Monday") );

    netDaysGridSizer->Add( m_chkNetMonday, 0, wxALL, 5 );

    m_txtNetMonday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETMONDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetMonday, 0, wxALL, 1 );

    m_chkNetTuesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTUESDAY, _("Tuesday") );

    netDaysGridSizer->Add( m_chkNetTuesday, 0, wxALL, 5 );

    m_txtNetTuesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTUESDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetTuesday, 0, wxALL, 1 );

    m_chkNetWednesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETWEDNESDAY, _("Wednesday") );

    netDaysGridSizer->Add( m_chkNetWednesday, 0, wxALL, 5 );

    m_txtNetWednesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETWEDNESDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetWednesday, 0, wxALL, 1 );

    m_chkNetThursday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTHURSDAY, _("Thursday") );

    netDaysGridSizer->Add( m_chkNetThursday, 0, wxALL, 5 );

    m_txtNetThursday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTHURSDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetThursday, 0, wxALL, 1 );

    m_chkNetFriday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETFRIDAY, _("Friday") );

    netDaysGridSizer->Add( m_chkNetFriday, 0, wxALL, 5 );

    m_txtNetFriday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETFRIDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetFriday, 0, wxALL, 1 );

    m_chkNetSaturday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSATURDAY, _("Saturday") );

    netDaysGridSizer->Add( m_chkNetSaturday, 0, wxALL, 5 );

    m_txtNetSaturday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSATURDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetSaturday, 0, wxALL, 1 );

    m_chkNetSunday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSUNDAY, _("Sunday") );

    netDaysGridSizer->Add( m_chkNetSunday, 0, wxALL, 5 );

    m_txtNetSunday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSUNDAY, wxT("") );
    netDaysGridSizer->Add( m_txtNetSunday, 0, wxALL, 1 );

    m_panelNetSpecialTimes->SetSizer( netDaysGridSizer );
    m_panelNetSpecialTimes->Layout();
    netDaysGridSizer->Fit( m_panelNetSpecialTimes );
    networkTimesBoxSizer->Add( m_panelNetSpecialTimes, 0, wxEXPAND | wxALL, 1 );

    networkTabSizer->Add( networkTimesBoxSizer, 0, wxEXPAND, 1 );

    panel->SetSizer( networkTabSizer );
    panel->Layout();
    networkTabSizer->Fit( panel );

    return panel;
}

wxPanel* CDlgAdvPreferencesBase::createDiskAndMemoryTab(wxWindow* parent)
{
    wxPanel* panel = new wxPanel( parent, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    panel->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* diskAndMemoryBoxSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* diskUsageBoxSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("Disk usage") ), wxVERTICAL );

    wxFlexGridSizer* diskUsageGridSizer = new wxFlexGridSizer( 5, 3, 0, 0 );
    diskUsageGridSizer->AddGrowableCol( 2 );
    diskUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    diskUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_txtDiskMaxSpace = new wxTextCtrl( panel, ID_TXTDISKMAXSPACE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtDiskMaxSpace->SetToolTip( _("Maximum allowed disk space (in Gigabytes)") );
    buildLayout(panel, diskUsageGridSizer, _("Use at most %1 Gigabytes disk space"), m_txtDiskMaxSpace);

    m_txtDiskLeastFree = new wxTextCtrl( panel, ID_TXTDISKLEASTFREE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtDiskLeastFree->SetToolTip( _("Leave at least this much disk space free (in Gigagytes)") );
    buildLayout(panel, diskUsageGridSizer, _("Leave at least %1 Gigabytes disk space free"), m_txtDiskLeastFree);

    m_txtDiskMaxOfTotal = new wxTextCtrl( panel, ID_TXTDISKMAXOFTOTAL, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtDiskMaxOfTotal->SetToolTip( _("Use at most this percentage of total disk space") );
    buildLayout(panel, diskUsageGridSizer, _("Use at most %1 % of total disk space"), m_txtDiskMaxOfTotal);

    m_txtDiskWriteToDisk = new wxTextCtrl( panel, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, diskUsageGridSizer, _("Write to disk at most every %1 seconds"), m_txtDiskWriteToDisk);

    m_txtDiskMaxSwap = new wxTextCtrl( panel, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, diskUsageGridSizer, _("Use at most %1 % of page file (swap space)"), m_txtDiskMaxSwap);

    diskUsageBoxSizer->Add( diskUsageGridSizer, 0, wxEXPAND, 1 );

    diskAndMemoryBoxSizer->Add( diskUsageBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* memoryUsageBoxSizer = new wxStaticBoxSizer( new wxStaticBox( panel, -1, _("Memory usage") ), wxVERTICAL );

    wxFlexGridSizer* memoryUsageGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    memoryUsageGridSizer->AddGrowableCol( 2 );
    memoryUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    memoryUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_txtMemoryMaxInUse = new wxTextCtrl( panel, ID_TXTMEMORYMAXINUSE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, memoryUsageGridSizer, _("Use at most %1 % when computer is in use"), m_txtMemoryMaxInUse);

    m_txtMemoryMaxOnIdle = new wxTextCtrl( panel, ID_TXTMEMORYMAXONIDLE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    buildLayout(panel, memoryUsageGridSizer, _("Use at most %1 % when computer is idle"), m_txtMemoryMaxOnIdle);

    memoryUsageBoxSizer->Add( memoryUsageGridSizer, 0, wxEXPAND, 1 );

    m_chkMemoryWhileSuspended = new wxCheckBox( panel, ID_CHKMEMORYWHILESUSPENDED, _("Leave applications in memory while suspended") );

    m_chkMemoryWhileSuspended->SetToolTip( _("if checked, suspended work units are left in memory") );

    memoryUsageBoxSizer->Add( m_chkMemoryWhileSuspended, 0, wxALL, 5 );

    diskAndMemoryBoxSizer->Add( memoryUsageBoxSizer, 0, wxALL|wxEXPAND, 1 );

    panel->SetSizer( diskAndMemoryBoxSizer );
    panel->Layout();
    diskAndMemoryBoxSizer->Fit( panel );

    return panel;
}
