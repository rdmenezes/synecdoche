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
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "LogBOINC.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"

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

    wxBoxSizer* dialogSizer;
    dialogSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* topControlsSizer;
    topControlsSizer = new wxStaticBoxSizer( new wxStaticBox( this, -1, wxT("") ), wxHORIZONTAL );

    m_bmpWarning = new wxStaticBitmap( this, ID_DEFAULT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
    m_bmpWarning->SetMinSize( wxSize( 48,48 ) );

    topControlsSizer->Add( m_bmpWarning, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    m_staticText321 = new wxStaticText( this, ID_DEFAULT, _("This dialog controls preferences for this computer only.\nClick OK to set preferences.\nClick Clear to restore web-based settings."), wxDefaultPosition, wxDefaultSize, 0 );
    topControlsSizer->Add( m_staticText321, 1, wxALL, 1 );

    m_btnClear = new wxButton( this, ID_BTN_CLEAR, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnClear->SetToolTip( _("clear all local preferences and close the dialog") );

    topControlsSizer->Add( m_btnClear, 0, wxALIGN_BOTTOM|wxALL, 1 );

    dialogSizer->Add( topControlsSizer, 0, wxALL|wxEXPAND, 1 );

    m_panelControls = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelControls->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* notebookSizer;
    notebookSizer = new wxBoxSizer( wxVERTICAL );

    m_Notebook = new wxNotebook( m_panelControls, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxNB_FLAT|wxNB_TOP );
    m_Notebook->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    m_panelProcessor = new wxPanel( m_Notebook, ID_TABPAGE_PROC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelProcessor->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* processorTabSizer;
    processorTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* computingAllowedBoxSizer;
    computingAllowedBoxSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelProcessor, -1, _("Computing allowed") ), wxVERTICAL );

    m_chkProcOnBatteries = new wxCheckBox( m_panelProcessor, ID_CHKPROCONBATTERIES, _(" While computer is on batteries"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkProcOnBatteries->SetToolTip( _("check this if you want this computer to do work while it runs on batteries") );

    computingAllowedBoxSizer->Add( m_chkProcOnBatteries, 0, wxALL, 5 );

    m_chkProcInUse = new wxCheckBox( m_panelProcessor, ID_CHKPROCINUSE, _(" While computer is in use"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkProcInUse->SetToolTip( _("check this if you want this computer to do work even when you're using it") );

    computingAllowedBoxSizer->Add( m_chkProcInUse, 0, wxALL, 5 );

    wxFlexGridSizer* procIdleSizer;
    procIdleSizer = new wxFlexGridSizer( 2, 4, 0, 0 );
    procIdleSizer->AddGrowableCol( 3 );
    procIdleSizer->SetFlexibleDirection( wxHORIZONTAL );
    procIdleSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText26 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Only after computer has been idle for"), wxDefaultPosition, wxDefaultSize, 0 );
    procIdleSizer->Add( m_staticText26, 0, wxALL, 5 );

    m_txtProcIdleFor = new wxTextCtrl( m_panelProcessor, ID_TXTPROCIDLEFOR, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtProcIdleFor->SetToolTip( _("do work only after you haven't used the computer for this number of minutes") );

    procIdleSizer->Add( m_txtProcIdleFor, 0, wxALL, 1 );

    m_staticText27 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    procIdleSizer->Add( m_staticText27, 0, wxALL, 5 );

    m_staticText28 = new wxStaticText( m_panelProcessor, ID_DEFAULT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procIdleSizer->Add( m_staticText28, 0, wxALL, 5 );

    computingAllowedBoxSizer->Add( procIdleSizer, 0, wxEXPAND, 5 );

    wxBoxSizer* cpuTimesSizer;
    cpuTimesSizer = new wxBoxSizer( wxHORIZONTAL );

    m_staticText351 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Every day between hours of"), wxDefaultPosition, wxDefaultSize, 0 );
    cpuTimesSizer->Add( m_staticText351, 0, wxALL, 5 );

    m_txtProcEveryDayStart = new wxTextCtrl( m_panelProcessor, ID_TXTPROCEVERYDAYSTART, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtProcEveryDayStart->SetToolTip( _("start work at this time") );

    cpuTimesSizer->Add( m_txtProcEveryDayStart, 0, wxALL, 1 );

    m_staticText25 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    cpuTimesSizer->Add( m_staticText25, 0, wxALL|wxEXPAND, 5 );

    m_txtProcEveryDayStop = new wxTextCtrl( m_panelProcessor, ID_TXTPROCEVERYDAYSTOP, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtProcEveryDayStop->SetToolTip( _("stop work at this time") );

    cpuTimesSizer->Add( m_txtProcEveryDayStop, 0, wxALL, 1 );

    m_staticText55 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("(no restriction if equal)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
    cpuTimesSizer->Add( m_staticText55, 0, wxALL|wxEXPAND, 5 );

    computingAllowedBoxSizer->Add( cpuTimesSizer, 0, wxEXPAND, 1 );

    m_staticText36 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Day-of-week override:"), wxDefaultPosition, wxDefaultSize, 0 );
    computingAllowedBoxSizer->Add( m_staticText36, 0, wxALL, 5 );

    m_panelProcSpecialTimes = new wxPanel( m_panelProcessor, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panelProcSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    m_panelProcSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

    wxBoxSizer* procDaysWrapperSizer;
    procDaysWrapperSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* procDaysSizer;
    procDaysSizer = new wxFlexGridSizer( 4, 4, 0, 0 );
    procDaysSizer->SetFlexibleDirection( wxHORIZONTAL );
    procDaysSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkProcMonday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcMonday, 0, wxALL, 5 );

    m_txtProcMonday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCMONDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcMonday, 0, wxALL, 1 );

    m_chkProcTuesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcTuesday, 0, wxALL, 5 );

    m_txtProcTuesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTUESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcTuesday, 0, wxALL, 1 );

    m_chkProcWednesday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcWednesday, 0, wxALL, 5 );

    m_txtProcWednesday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCWEDNESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcWednesday, 0, wxALL, 1 );

    m_chkProcThursday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcThursday, 0, wxALL, 5 );

    m_txtProcThursday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCTHURSDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcThursday, 0, wxALL, 1 );

    m_chkProcFriday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcFriday, 0, wxALL, 5 );

    m_txtProcFriday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCFRIDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcFriday, 0, wxALL, 1 );

    m_chkProcSaturday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcSaturday, 0, wxALL, 5 );

    m_txtProcSaturday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSATURDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcSaturday, 0, wxALL, 1 );

    m_chkProcSunday = new wxCheckBox( m_panelProcSpecialTimes, ID_CHKPROCSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );

    procDaysSizer->Add( m_chkProcSunday, 0, wxALL, 5 );

    m_txtProcSunday = new wxTextCtrl( m_panelProcSpecialTimes, ID_TXTPROCSUNDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    procDaysSizer->Add( m_txtProcSunday, 0, wxALL, 1 );

    procDaysWrapperSizer->Add( procDaysSizer, 1, wxEXPAND, 1 );

    m_panelProcSpecialTimes->SetSizer( procDaysWrapperSizer );
    m_panelProcSpecialTimes->Layout();
    procDaysWrapperSizer->Fit( m_panelProcSpecialTimes );
    computingAllowedBoxSizer->Add( m_panelProcSpecialTimes, 1, wxEXPAND | wxALL, 1 );

    processorTabSizer->Add( computingAllowedBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* miscProcBoxSizer;
    miscProcBoxSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelProcessor, -1, _("Other options") ), wxVERTICAL );

    wxFlexGridSizer* miscProcGridSizer;
    miscProcGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    miscProcGridSizer->AddGrowableCol( 2 );
    miscProcGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    miscProcGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText18 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Switch between applications between every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    miscProcGridSizer->Add( m_staticText18, 0, wxALL|wxEXPAND, 5 );

    m_txtProcSwitchEvery = new wxTextCtrl( m_panelProcessor, ID_TXTPROCSWITCHEVERY, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    miscProcGridSizer->Add( m_txtProcSwitchEvery, 0, wxALL, 1 );

    m_staticText19 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("minutes"), wxDefaultPosition, wxDefaultSize, 0 );
    miscProcGridSizer->Add( m_staticText19, 0, wxALL, 5 );

    m_staticText20 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("On multiprocessor systems, use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    miscProcGridSizer->Add( m_staticText20, 0, wxALL|wxEXPAND, 5 );

    m_txtProcUseProcessors = new wxTextCtrl( m_panelProcessor, ID_TXTPROCUSEPROCESSORS, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    miscProcGridSizer->Add( m_txtProcUseProcessors, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText21 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("% of the processors"), wxDefaultPosition, wxDefaultSize, 0 );
    miscProcGridSizer->Add( m_staticText21, 0, wxALL, 5 );

    m_staticText22 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    miscProcGridSizer->Add( m_staticText22, 0, wxALL|wxEXPAND, 5 );

    m_txtProcUseCPUTime = new wxTextCtrl( m_panelProcessor, ID_TXTPOCUSECPUTIME, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    miscProcGridSizer->Add( m_txtProcUseCPUTime, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText23 = new wxStaticText( m_panelProcessor, ID_DEFAULT, _("% CPU time"), wxDefaultPosition, wxDefaultSize, 0 );
    miscProcGridSizer->Add( m_staticText23, 0, wxALL, 5 );

    miscProcBoxSizer->Add( miscProcGridSizer, 0, wxEXPAND, 1 );

    processorTabSizer->Add( miscProcBoxSizer, 0, wxEXPAND, 1 );

    m_panelProcessor->SetSizer( processorTabSizer );
    m_panelProcessor->Layout();
    processorTabSizer->Fit( m_panelProcessor );
    m_Notebook->AddPage( m_panelProcessor, _("processor usage"), false );
    m_panelNetwork = new wxPanel( m_Notebook, ID_TABPAGE_NET, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelNetwork->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* networkTabSizer;
    networkTabSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* networkGeneralBoxSizer;
    networkGeneralBoxSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelNetwork, -1, _("General options") ), wxVERTICAL );

    wxFlexGridSizer* networkGeneralGridSizer;
    networkGeneralGridSizer = new wxFlexGridSizer( 3, 6, 0, 0 );
    networkGeneralGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    networkGeneralGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText32 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Maximum download rate"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText32, 0, wxALL, 5 );

    m_txtNetDownloadRate = new wxTextCtrl( m_panelNetwork, ID_TXTNETDOWNLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    networkGeneralGridSizer->Add( m_txtNetDownloadRate, 0, wxALL, 1 );

    m_staticText33 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("KBytes/sec."), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText33, 0, wxALL, 5 );

    m_staticText34 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Maximum upload rate"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText34, 0, wxALIGN_RIGHT|wxALL, 5 );

    m_txtNetUploadRate = new wxTextCtrl( m_panelNetwork, ID_TXTNETUPLOADRATE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    networkGeneralGridSizer->Add( m_txtNetUploadRate, 0, wxALL, 1 );

    m_staticText35 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("KBytes/sec."), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText35, 0, wxALL, 5 );

    m_staticText30 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Connect about every"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText30, 0, wxALL, 5 );

    m_txtNetConnectInterval = new wxTextCtrl( m_panelNetwork, ID_TXTNETCONNECTINTERVAL, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtNetConnectInterval->SetToolTip( _("this computer is connected to the Internet about every X days\n(0 if it's always connected)") );

    networkGeneralGridSizer->Add( m_txtNetConnectInterval, 0, wxALL, 1 );

    m_staticText31 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("days"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText31, 0, wxALL, 5 );

    m_staticText331 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Additional work buffer"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText331, 0, wxALIGN_RIGHT|wxALL, 5 );

    m_txtNetAdditionalDays = new wxTextCtrl( m_panelNetwork, ID_TXTNETADDITIONALDAYS, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    networkGeneralGridSizer->Add( m_txtNetAdditionalDays, 0, wxALL, 1 );

    m_staticText341 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("days (max. 10)"), wxDefaultPosition, wxDefaultSize, 0 );
    networkGeneralGridSizer->Add( m_staticText341, 0, wxALL, 5 );

    m_chkNetSkipImageVerification = new wxCheckBox( m_panelNetwork, ID_CHKNETSKIPIMAGEVERIFICATION, _(" Skip image file verification"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkNetSkipImageVerification->SetToolTip( _("check this if your Internet provider modifies image files") );

    networkGeneralGridSizer->Add( m_chkNetSkipImageVerification, 0, wxALL, 5 );

    networkGeneralBoxSizer->Add( networkGeneralGridSizer, 0, wxEXPAND, 1 );

    networkTabSizer->Add( networkGeneralBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* connectOptionsSizer;
    connectOptionsSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelNetwork, -1, _("Connect options") ), wxVERTICAL );

    m_chkNetConfirmBeforeConnect = new wxCheckBox( m_panelNetwork, ID_CHKNETCONFIRMBEFORECONNECT, _("Confirm before connecting to internet"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkNetConfirmBeforeConnect->SetToolTip( _("if checked, a confirmation dialog will be displayed before trying to connect to the Internet") );

    connectOptionsSizer->Add( m_chkNetConfirmBeforeConnect, 0, wxALL, 5 );

    m_chkNetDisconnectWhenDone = new wxCheckBox( m_panelNetwork, ID_CHKNETDISCONNECTWHENDONE, _("Disconnect when done"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkNetDisconnectWhenDone->SetToolTip( _("if checked, hang up when network usage is done\n(only relevant for dialup-connection)") );

    connectOptionsSizer->Add( m_chkNetDisconnectWhenDone, 0, wxALL, 5 );

    networkTabSizer->Add( connectOptionsSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* networkTimesBoxSizer;
    networkTimesBoxSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelNetwork, -1, _("Network usage allowed") ), wxVERTICAL );

    wxBoxSizer* networkTimesSizer;
    networkTimesSizer = new wxBoxSizer( wxHORIZONTAL );

    m_staticText38 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Every day between hours of"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesSizer->Add( m_staticText38, 0, wxALL, 5 );

    m_txtNetEveryDayStart = new wxTextCtrl( m_panelNetwork, ID_TXTNETEVERYDAYSTART, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), 0 );
    m_txtNetEveryDayStart->SetToolTip( _("network usage start hour") );

    networkTimesSizer->Add( m_txtNetEveryDayStart, 0, wxALL, 1 );

    m_staticText37 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesSizer->Add( m_staticText37, 0, wxALL, 5 );

    m_txtNetEveryDayStop = new wxTextCtrl( m_panelNetwork, ID_TXTNETEVERYDAYSTOP, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), 0 );
    m_txtNetEveryDayStop->SetToolTip( _("network usage stop hour") );

    networkTimesSizer->Add( m_txtNetEveryDayStop, 0, wxALL, 1 );

    m_staticText54 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("(no restriction if equal)"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesSizer->Add( m_staticText54, 0, wxALL, 5 );

    networkTimesBoxSizer->Add( networkTimesSizer, 0, wxEXPAND, 1 );

    m_staticText39 = new wxStaticText( m_panelNetwork, ID_DEFAULT, _("Day-of-week override:"), wxDefaultPosition, wxDefaultSize, 0 );
    networkTimesBoxSizer->Add( m_staticText39, 0, wxALL, 5 );

    m_panelNetSpecialTimes = new wxPanel( m_panelNetwork, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    m_panelNetSpecialTimes->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );
    m_panelNetSpecialTimes->SetToolTip( _("check box to specify hours for this day of week") );

    wxBoxSizer* netDaysWrapperSizer;
    netDaysWrapperSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* netDaysGridSizer;
    netDaysGridSizer = new wxFlexGridSizer( 4, 4, 0, 0 );
    netDaysGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    netDaysGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_chkNetMonday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETMONDAY, _("Monday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetMonday, 0, wxALL, 5 );

    m_txtNetMonday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETMONDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetMonday, 0, wxALL, 1 );

    m_chkNetTuesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTUESDAY, _("Tuesday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetTuesday, 0, wxALL, 5 );

    m_txtNetTuesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTUESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetTuesday, 0, wxALL, 1 );

    m_chkNetWednesday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETWEDNESDAY, _("Wednesday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetWednesday, 0, wxALL, 5 );

    m_txtNetWednesday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETWEDNESDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetWednesday, 0, wxALL, 1 );

    m_chkNetThursday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETTHURSDAY, _("Thursday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetThursday, 0, wxALL, 5 );

    m_txtNetThursday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETTHURSDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetThursday, 0, wxALL, 1 );

    m_chkNetFriday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETFRIDAY, _("Friday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetFriday, 0, wxALL, 5 );

    m_txtNetFriday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETFRIDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetFriday, 0, wxALL, 1 );

    m_chkNetSaturday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSATURDAY, _("Saturday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetSaturday, 0, wxALL, 5 );

    m_txtNetSaturday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSATURDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetSaturday, 0, wxALL, 1 );

    m_chkNetSunday = new wxCheckBox( m_panelNetSpecialTimes, ID_CHKNETSUNDAY, _("Sunday"), wxDefaultPosition, wxDefaultSize, 0 );

    netDaysGridSizer->Add( m_chkNetSunday, 0, wxALL, 5 );

    m_txtNetSunday = new wxTextCtrl( m_panelNetSpecialTimes, ID_TXTNETSUNDAY, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    netDaysGridSizer->Add( m_txtNetSunday, 0, wxALL, 1 );

    netDaysWrapperSizer->Add( netDaysGridSizer, 0, wxEXPAND, 1 );

    m_panelNetSpecialTimes->SetSizer( netDaysWrapperSizer );
    m_panelNetSpecialTimes->Layout();
    netDaysWrapperSizer->Fit( m_panelNetSpecialTimes );
    networkTimesBoxSizer->Add( m_panelNetSpecialTimes, 0, wxEXPAND | wxALL, 1 );

    networkTabSizer->Add( networkTimesBoxSizer, 0, wxEXPAND, 1 );

    m_panelNetwork->SetSizer( networkTabSizer );
    m_panelNetwork->Layout();
    networkTabSizer->Fit( m_panelNetwork );
    m_Notebook->AddPage( m_panelNetwork, _("network usage"), true );
    m_panelDiskAndMemory = new wxPanel( m_Notebook, ID_TABPAGE_DISK, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_panelDiskAndMemory->SetExtraStyle( wxWS_EX_VALIDATE_RECURSIVELY );

    wxBoxSizer* diskAndMemoryBoxSizer;
    diskAndMemoryBoxSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* diskUsageBoxSizer;
    diskUsageBoxSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelDiskAndMemory, -1, _("Disk usage") ), wxVERTICAL );

    wxFlexGridSizer* diskUsageGridSizer;
    diskUsageGridSizer = new wxFlexGridSizer( 5, 3, 0, 0 );
    diskUsageGridSizer->AddGrowableCol( 2 );
    diskUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    diskUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText40 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_staticText40, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxSpace = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKMAXSPACE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtDiskMaxSpace->SetToolTip( _("Maximum allowed disk space (in Gigabytes)") );

    diskUsageGridSizer->Add( m_txtDiskMaxSpace, 0, wxALL, 1 );

    m_staticText41 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Gigabytes disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( m_staticText41, 0, wxALL, 5 );

    m_staticText42 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Leave at least"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_staticText42, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskLeastFree = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKLEASTFREE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtDiskLeastFree->SetToolTip( _("Leave at least this much disk space free (in Gigagytes)") );

    diskUsageGridSizer->Add( m_txtDiskLeastFree, 0, wxALL, 1 );

    m_staticText43 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Gigabytes disk space free"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( m_staticText43, 0, wxALL, 5 );

    m_staticText44 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_staticText44, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxOfTotal = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKMAXOFTOTAL, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    m_txtDiskMaxOfTotal->SetToolTip( _("Use at most this percentage of total disk space") );

    diskUsageGridSizer->Add( m_txtDiskMaxOfTotal, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText45 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% of total disk space"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( m_staticText45, 0, wxALL, 5 );

    m_staticText46 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Write to disk at most every"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_staticText46, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskWriteToDisk = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    diskUsageGridSizer->Add( m_txtDiskWriteToDisk, 0, wxALL, 1 );

    m_staticText47 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( m_staticText47, 0, wxALL, 5 );

    m_staticText48 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    diskUsageGridSizer->Add( m_staticText48, 0, wxALL|wxEXPAND, 5 );

    m_txtDiskMaxSwap = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTDISKWRITETODISK, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    diskUsageGridSizer->Add( m_txtDiskMaxSwap, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText49 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% of page file (swap space)"), wxDefaultPosition, wxDefaultSize, 0 );
    diskUsageGridSizer->Add( m_staticText49, 0, wxALL, 5 );

    diskUsageBoxSizer->Add( diskUsageGridSizer, 0, wxEXPAND, 1 );

    diskAndMemoryBoxSizer->Add( diskUsageBoxSizer, 0, wxEXPAND, 1 );

    wxStaticBoxSizer* memoryUsageBoxSizer;
    memoryUsageBoxSizer = new wxStaticBoxSizer( new wxStaticBox( m_panelDiskAndMemory, -1, _("Memory usage") ), wxVERTICAL );

    wxFlexGridSizer* memoryUsageGridSizer;
    memoryUsageGridSizer = new wxFlexGridSizer( 3, 3, 0, 0 );
    memoryUsageGridSizer->AddGrowableCol( 2 );
    memoryUsageGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    memoryUsageGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_staticText50 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    memoryUsageGridSizer->Add( m_staticText50, 0, wxALL|wxEXPAND, 5 );

    m_txtMemoryMaxInUse = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTMEMORYMAXINUSE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    memoryUsageGridSizer->Add( m_txtMemoryMaxInUse, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText51 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% when computer is in use"), wxDefaultPosition, wxDefaultSize, 0 );
    memoryUsageGridSizer->Add( m_staticText51, 0, wxALL, 5 );

    m_staticText52 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("Use at most"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
    memoryUsageGridSizer->Add( m_staticText52, 0, wxALL|wxEXPAND, 5 );

    m_txtMemoryMaxOnIdle = new wxTextCtrl( m_panelDiskAndMemory, ID_TXTMEMORYMAXONIDLE, wxT(""), wxDefaultPosition, wxSize( 50,-1 ), wxTE_RIGHT );
    memoryUsageGridSizer->Add( m_txtMemoryMaxOnIdle, 0, wxALL, 1 );

    /*xgettext:no-c-format*/
    m_staticText53 = new wxStaticText( m_panelDiskAndMemory, ID_DEFAULT, _("% when computer is idle"), wxDefaultPosition, wxDefaultSize, 0 );
    memoryUsageGridSizer->Add( m_staticText53, 0, wxALL, 5 );

    memoryUsageBoxSizer->Add( memoryUsageGridSizer, 0, wxEXPAND, 1 );

    m_chkMemoryWhileSuspended = new wxCheckBox( m_panelDiskAndMemory, ID_CHKMEMORYWHILESUSPENDED, _(" Leave applications in memory while suspended"), wxDefaultPosition, wxDefaultSize, 0 );

    m_chkMemoryWhileSuspended->SetToolTip( _("if checked, suspended work units are left in memory") );

    memoryUsageBoxSizer->Add( m_chkMemoryWhileSuspended, 0, wxALL, 5 );

    diskAndMemoryBoxSizer->Add( memoryUsageBoxSizer, 0, wxALL|wxEXPAND, 1 );

    m_panelDiskAndMemory->SetSizer( diskAndMemoryBoxSizer );
    m_panelDiskAndMemory->Layout();
    diskAndMemoryBoxSizer->Fit( m_panelDiskAndMemory );
    m_Notebook->AddPage( m_panelDiskAndMemory, _("disk and memory usage"), false );

    notebookSizer->Add( m_Notebook, 1, wxEXPAND | wxALL, 1 );

    m_panelControls->SetSizer( notebookSizer );
    m_panelControls->Layout();
    notebookSizer->Fit( m_panelControls );
    dialogSizer->Add( m_panelControls, 1, wxALL|wxEXPAND, 1 );

    m_panelButtons = new wxPanel( this, ID_DEFAULT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* buttonSizer;
    buttonSizer = new wxBoxSizer( wxHORIZONTAL );

    m_btnOK = new wxButton( m_panelButtons, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnOK->SetToolTip( _("save all values and close the dialog") );

    buttonSizer->Add( m_btnOK, 0, wxALL, 5 );

    m_btnCancel = new wxButton( m_panelButtons, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnCancel->SetToolTip( _("close the dialog without saving") );

    buttonSizer->Add( m_btnCancel, 0, wxALL, 5 );

    m_btnHelp = new wxButton( m_panelButtons, wxID_HELP, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
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
