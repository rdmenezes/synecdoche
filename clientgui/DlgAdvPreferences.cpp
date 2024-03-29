// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Nicolas Alvarez, Peter Kortschack
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

#include "DlgAdvPreferences.h"
#include "stdwx.h"
#include "res/usage.xpm"
#include "res/xfer.xpm"
#include "res/proj.xpm"
#include "res/warning.xpm"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "SkinManager.h"
#include "hyperlink.h"
#include "Events.h"
#include "error_numbers.h"

namespace {
    /// Small helper function to trim a value to the interval [0; 100]
    ///
    /// \param[in] x The value that should be trimmed.
    /// \return \a x if it lies in the interval [0; 100], otherwise 0 or 100
    ///        is returned, whichever is closer to \a x.
    double clamp_pct(double x) {
        if (x < 0.0) {
            return 0.0;
        } else if (x > 100.0) {
            return 100.0;
        }
        return x;
    }
}

IMPLEMENT_DYNAMIC_CLASS(CDlgAdvPreferences, wxDialog)

BEGIN_EVENT_TABLE(CDlgAdvPreferences, wxDialog)
    EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_CHECKBOX_CLICKED,CDlgAdvPreferences::OnHandleCommandEvent)
    EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_RADIOBUTTON_SELECTED,CDlgAdvPreferences::OnHandleCommandEvent)
    EVT_COMMAND_RANGE(20000,21000,wxEVT_COMMAND_TEXT_UPDATED,CDlgAdvPreferences::OnHandleCommandEvent)
    //buttons
    EVT_BUTTON(wxID_OK,CDlgAdvPreferences::OnOK)
    EVT_BUTTON(wxID_HELP,CDlgAdvPreferences::OnHelp)
    EVT_BUTTON(ID_BTN_CLEAR,CDlgAdvPreferences::OnClear)
END_EVENT_TABLE()

CDlgAdvPreferences::CDlgAdvPreferences(wxWindow* parent) : CDlgAdvPreferencesBase(parent,ID_ANYDIALOG) {
    m_bInInit=false;
    m_bDataChanged=false;
    m_arrTabPageIds.Add(ID_TABPAGE_PROC);
    m_arrTabPageIds.Add(ID_TABPAGE_NET);
    m_arrTabPageIds.Add(ID_TABPAGE_DISK);

    //setting tab page images (not handled by generated code)
    int iImageIndex = 0;
    wxImageList* pImageList = m_Notebook->GetImageList();
    if (!pImageList) {
        pImageList = new wxImageList(16, 16, true, 0);
        wxASSERT(pImageList != NULL);
        m_Notebook->SetImageList(pImageList);
    }
    iImageIndex = pImageList->Add(wxBitmap(proj_xpm));
    m_Notebook->SetPageImage(0, iImageIndex);

    iImageIndex = pImageList->Add(wxBitmap(xfer_xpm));
    m_Notebook->SetPageImage(1, iImageIndex);

    iImageIndex = pImageList->Add(wxBitmap(usage_xpm));
    m_Notebook->SetPageImage(2, iImageIndex);

    // setting warning bitmap
    m_bmpWarning->SetBitmap(wxBitmap(warning_xpm));

    // init special tooltips
    SetSpecialTooltips();

    // setting the validators for correct input handling
    SetValidators();

    // read in settings and initialize controls
    ReadPreferenceSettings();

    RestoreState();
}

CDlgAdvPreferences::~CDlgAdvPreferences() {
    SaveState();
}

/// Set validators for input filtering purposes only.
void CDlgAdvPreferences::SetValidators() {
    //proc page
    m_txtProcIdleFor->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcSwitchEvery->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcUseProcessors->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtProcUseCPUTime->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    //net page
    m_txtNetConnectInterval->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetDownloadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetUploadRate->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtNetAdditionalDays->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    //disk and memory page
    m_txtDiskMaxSpace->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskLeastFree->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskMaxOfTotal->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskWriteToDisk->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtDiskMaxSwap->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtMemoryMaxInUse->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
    m_txtMemoryMaxOnIdle->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

}

/// Set tooltips that are shared with multiple controls.
void CDlgAdvPreferences::SetSpecialTooltips() {
    m_txtProcMonday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    m_txtProcTuesday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    m_txtProcWednesday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    m_txtProcThursday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    m_txtProcFriday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    m_txtProcSaturday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    m_txtProcSunday->SetToolTip(TXT_PROC_TIME_TOOLTIP);
    //
    m_txtNetMonday->SetToolTip(TXT_NET_TIME_TOOLTIP);
    m_txtNetTuesday->SetToolTip(TXT_NET_TIME_TOOLTIP);
    m_txtNetWednesday->SetToolTip(TXT_NET_TIME_TOOLTIP);
    m_txtNetThursday->SetToolTip(TXT_NET_TIME_TOOLTIP);
    m_txtNetFriday->SetToolTip(TXT_NET_TIME_TOOLTIP);
    m_txtNetSaturday->SetToolTip(TXT_NET_TIME_TOOLTIP);
    m_txtNetSunday->SetToolTip(TXT_NET_TIME_TOOLTIP);
}

/// Save selected tab page and dialog size.
bool CDlgAdvPreferences::SaveState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);

    wxASSERT(pConfig);
    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("CurrentPage"),m_Notebook->GetSelection());
    pConfig->Write(wxT("Width"),this->GetSize().GetWidth());
    pConfig->Write(wxT("Height"),this->GetSize().GetHeight());
    return true;
}

/// Restore former selected tab page and dialog size.
bool CDlgAdvPreferences::RestoreState() {
    wxString        strBaseConfigLocation = wxString(wxT("/DlgAdvPreferences/"));
    wxConfigBase*   pConfig = wxConfigBase::Get(FALSE);
    int             p,w,h;

    wxASSERT(pConfig);

    if (!pConfig) return false;

    pConfig->SetPath(strBaseConfigLocation);

    pConfig->Read(wxT("CurrentPage"), &p,0);
    m_Notebook->SetSelection(p);
    pConfig->Read(wxT("Width"), &w,-1);
    pConfig->Read(wxT("Height"), &h,-1);
    this->SetSize(w,h);

    return true;
}

/// Convert a Timestring HH:MM into a time_t.
time_t CDlgAdvPreferences::TimeStringToTimeT(wxString timeStr) {
    unsigned long hour;
    unsigned long minutes;
    timeStr.SubString(0, timeStr.First(':')).ToULong(&hour);
    timeStr.SubString(timeStr.First(':') + 1, timeStr.Length()).ToULong(&minutes);
    return hour * 3600 + minutes * 60;
}

/// Convert a time_t into a timestring HH:MM.
wxString CDlgAdvPreferences::TimeTToTimeString(time_t point_in_time) {
    int hour = point_in_time / 3600;
    int minutes = (point_in_time / 60) % 60;
    return wxString::Format(wxT("%02d:%02d"), hour, minutes);
}

/// Read preferences from core client and initialize control values.
void CDlgAdvPreferences::ReadPreferenceSettings() {
    m_bInInit=true; //prevent dialog handlers from doing anything
    CMainDocument* pDoc = wxGetApp().GetDocument();
    wxString buffer;
    int retval;

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    // Get current working preferences (including any overrides) from client
        retval = pDoc->rpc.get_global_prefs_working_struct(prefs, mask);
        if (retval == ERR_NOT_FOUND) {
            // Older clients don't support get_global_prefs_working_struct RPC
            prefs = pDoc->state.global_prefs;
            pDoc->rpc.get_global_prefs_override_struct(prefs, mask);
        }

    // ######### proc usage page
    // do work between
    *m_txtProcEveryDayStart << TimeTToTimeString(prefs.cpu_times.get_start());
    *m_txtProcEveryDayStop << TimeTToTimeString(prefs.cpu_times.get_end());
    //special day times
    wxCheckBox* aChks[] = {m_chkProcSunday,m_chkProcMonday,m_chkProcTuesday,m_chkProcWednesday,m_chkProcThursday,m_chkProcFriday,m_chkProcSaturday};
    wxTextCtrl* aTxts[] = {m_txtProcSunday,m_txtProcMonday,m_txtProcTuesday,m_txtProcWednesday,m_txtProcThursday,m_txtProcFriday,m_txtProcSaturday};
    for (int i=0; i<7; i++) {
        const TIME_SPAN* cpu = prefs.cpu_times.week.get(i);
        if(cpu) {
            aChks[i]->SetValue(true);
            wxString timeStr = TimeTToTimeString(cpu->get_start()) +
                               wxT("-") + TimeTToTimeString(cpu->get_end());
            aTxts[i]->SetValue(timeStr);
        }
    }

    // on batteries
    m_chkProcOnBatteries->SetValue(prefs.run_on_batteries);
    // in use
    m_chkProcInUse->SetValue(prefs.run_if_user_active);
    // idle for X minutes
    buffer.Printf(wxT("%.2f"), prefs.idle_time_to_run);
    *m_txtProcIdleFor << buffer;
    // switch every X minutes
    buffer.Printf(wxT("%.2f"), prefs.cpu_scheduling_period_minutes);
    *m_txtProcSwitchEvery << buffer;
    // max cpus
    buffer.Printf(wxT("%.2f"), prefs.max_ncpus_pct);
    *m_txtProcUseProcessors << buffer;
    //cpu limit
    buffer.Printf(wxT("%.2f"), prefs.cpu_usage_limit);
    *m_txtProcUseCPUTime << buffer;

    // ######### net usage page
    // use network between
    *m_txtNetEveryDayStart << TimeTToTimeString(prefs.net_times.get_start());
    *m_txtNetEveryDayStop << TimeTToTimeString(prefs.net_times.get_end());
    //special day times
    wxCheckBox* aChks2[] = {m_chkNetSunday,m_chkNetMonday,m_chkNetTuesday,m_chkNetWednesday,m_chkNetThursday,m_chkNetFriday,m_chkNetSaturday};
    wxTextCtrl* aTxts2[] = {m_txtNetSunday,m_txtNetMonday,m_txtNetTuesday,m_txtNetWednesday,m_txtNetThursday,m_txtNetFriday,m_txtNetSaturday};
    for(int i=0; i< 7;i++) {
        const TIME_SPAN* net = prefs.net_times.week.get(i);
        if(net) {
            aChks2[i]->SetValue(true);
            wxString timeStr = TimeTToTimeString(net->get_start()) +
                                wxT("-") + TimeTToTimeString(net->get_end());
            aTxts2[i]->SetValue(timeStr);
        }
    }
    // connection interval
    buffer.Printf(wxT("%01.4f"), prefs.work_buf_min_days);
    *m_txtNetConnectInterval << buffer;
    //download rate
    buffer.Printf(wxT("%.2f"), prefs.max_bytes_sec_down / 1024);
    *m_txtNetDownloadRate << buffer;
    // upload rate
    buffer.Printf(wxT("%.2f"), prefs.max_bytes_sec_up / 1024);
    *m_txtNetUploadRate << buffer;
    //
    buffer.Printf(wxT("%.2f"), prefs.work_buf_additional_days);
    *m_txtNetAdditionalDays << buffer;
    // skip image verification
    m_chkNetSkipImageVerification->SetValue(prefs.dont_verify_images);
    // confirm before connect
    m_chkNetConfirmBeforeConnect->SetValue(prefs.confirm_before_connecting);
    // disconnect when done
    m_chkNetDisconnectWhenDone->SetValue(prefs.hangup_if_dialed);

    // ######### disk and memory usage page
    // max space used
    buffer.Printf(wxT("%.2f"), prefs.disk_max_used_gb);
    *m_txtDiskMaxSpace << buffer;
    // min free
    buffer.Printf(wxT("%.2f"), prefs.disk_min_free_gb);
    *m_txtDiskLeastFree << buffer;
    // max used percentage
    buffer.Printf(wxT("%.2f"), prefs.disk_max_used_pct);
    *m_txtDiskMaxOfTotal << buffer;
    // write to disk every X seconds
    buffer.Printf(wxT("%.0f"), prefs.disk_interval);
    *m_txtDiskWriteToDisk << buffer;
    // max swap space (virtual memory)
    buffer.Printf(wxT("%.2f"), prefs.vm_max_used_frac*100.0);
    *m_txtDiskMaxSwap << buffer;
    // max VM used
    buffer.Printf(wxT("%.2f"), prefs.ram_max_used_busy_frac*100.0);
    *m_txtMemoryMaxInUse << buffer;
    // max VM idle
    buffer.Printf(wxT("%.2f"), prefs.ram_max_used_idle_frac*100.0);
    *m_txtMemoryMaxOnIdle << buffer;
    // suspend to memory
    m_chkMemoryWhileSuspended->SetValue(prefs.leave_apps_in_memory);
    m_bInInit = false;
    // update control states
    this->UpdateControlStates();
}

/// Write overridden preferences to disk (global_prefs_override.xml)
///
/// \return Always returns true.
bool CDlgAdvPreferences::SavePreferencesSettings() {
    double td;

    mask.clear();
    //clear special times settings
    prefs.cpu_times.week.clear();
    prefs.net_times.week.clear();
    //proc page
    prefs.run_on_batteries=m_chkProcOnBatteries->GetValue();
    mask.run_on_batteries=true;

    prefs.run_if_user_active=m_chkProcInUse->GetValue();
    mask.run_if_user_active=true;

    if(m_txtProcIdleFor->IsEnabled()) {
        m_txtProcIdleFor->GetValue().ToDouble(&td);
        prefs.idle_time_to_run=td;
        mask.idle_time_to_run=true;
    }

    prefs.cpu_times.set_start(TimeStringToTimeT(m_txtProcEveryDayStart->GetValue()));
    mask.start_hour = true;

    prefs.cpu_times.set_end(TimeStringToTimeT(m_txtProcEveryDayStop->GetValue()));
    mask.end_hour = true;

    wxCheckBox* aChks[] = {m_chkProcSunday,m_chkProcMonday,m_chkProcTuesday,m_chkProcWednesday,m_chkProcThursday,m_chkProcFriday,m_chkProcSaturday};
    wxTextCtrl* aTxts[] = {m_txtProcSunday,m_txtProcMonday,m_txtProcTuesday,m_txtProcWednesday,m_txtProcThursday,m_txtProcFriday,m_txtProcSaturday};
    for (int i = 0; i < 7; ++i) {
        if (aChks[i]->GetValue()) {
            wxString timeStr = aTxts[i]->GetValue();
            wxString startStr = timeStr.SubString(0, timeStr.First('-'));
            wxString endStr = timeStr.SubString(timeStr.First('-') + 1, timeStr.Length());
            prefs.cpu_times.week.set(i, TimeStringToTimeT(startStr), TimeStringToTimeT(endStr));
        }
    }
    m_txtProcSwitchEvery->GetValue().ToDouble(&td);
    prefs.cpu_scheduling_period_minutes=td;
    mask.cpu_scheduling_period_minutes=true;

    m_txtProcUseProcessors->GetValue().ToDouble(&td);
    prefs.max_ncpus_pct = clamp_pct(td);
    mask.max_ncpus_pct=true;

    m_txtProcUseCPUTime->GetValue().ToDouble(&td);
    prefs.cpu_usage_limit=td;
    mask.cpu_usage_limit=true;
    // network page
    m_txtNetConnectInterval->GetValue().ToDouble(&td);
    prefs.work_buf_min_days=td;
    mask.work_buf_min_days=true;

    m_txtNetDownloadRate->GetValue().ToDouble(&td);
    td = td * 1024;
    prefs.max_bytes_sec_down=td;
    mask.max_bytes_sec_down=true;

    m_txtNetUploadRate->GetValue().ToDouble(&td);
    td = td * 1024;
    prefs.max_bytes_sec_up=td;
    mask.max_bytes_sec_up=true;

    prefs.dont_verify_images=m_chkNetSkipImageVerification->GetValue();
    mask.dont_verify_images=true;

    prefs.confirm_before_connecting= m_chkNetConfirmBeforeConnect->GetValue();
    mask.confirm_before_connecting=true;

    prefs.hangup_if_dialed= m_chkNetDisconnectWhenDone->GetValue();
    mask.hangup_if_dialed=true;

    m_txtNetAdditionalDays->GetValue().ToDouble(&td);
    prefs.work_buf_additional_days = td;
    mask.work_buf_additional_days = true;

    prefs.net_times.set_start(TimeStringToTimeT(m_txtNetEveryDayStart->GetValue()));
    mask.net_start_hour = true;

    prefs.net_times.set_end(TimeStringToTimeT(m_txtNetEveryDayStop->GetValue()));
    mask.net_end_hour = true;

    wxCheckBox* aChks2[] = {m_chkNetSunday,m_chkNetMonday,m_chkNetTuesday,m_chkNetWednesday,m_chkNetThursday,m_chkNetFriday,m_chkNetSaturday};
    wxTextCtrl* aTxts2[] = {m_txtNetSunday,m_txtNetMonday,m_txtNetTuesday,m_txtNetWednesday,m_txtNetThursday,m_txtNetFriday,m_txtNetSaturday};
    for (int i=0; i<7; i++) {
        if(aChks2[i]->GetValue()) {
            wxString timeStr = aTxts2[i]->GetValue();
            wxString startStr = timeStr.SubString(0,timeStr.First('-'));
            wxString endStr = timeStr.SubString(timeStr.First('-')+1,timeStr.Length());
            prefs.net_times.week.set(i, TimeStringToTimeT(startStr), TimeStringToTimeT(endStr));
        }
    }
    //disk usage
    m_txtDiskMaxSpace->GetValue().ToDouble(&td);
    prefs.disk_max_used_gb=td;
    mask.disk_max_used_gb=true;

    m_txtDiskLeastFree->GetValue().ToDouble(&td);
    prefs.disk_min_free_gb=td;
    mask.disk_min_free_gb=true;

    m_txtDiskMaxOfTotal->GetValue().ToDouble(&td);
    prefs.disk_max_used_pct = clamp_pct(td);
    mask.disk_max_used_pct=true;

    m_txtDiskWriteToDisk->GetValue().ToDouble(&td);
    prefs.disk_interval=td;
    mask.disk_interval=true;

    m_txtDiskMaxSwap->GetValue().ToDouble(&td);
    td = clamp_pct(td) / 100.0 ;
    prefs.vm_max_used_frac=td;
    mask.vm_max_used_frac=true;
    //Memory
    m_txtMemoryMaxInUse->GetValue().ToDouble(&td);
    td = clamp_pct(td) / 100.0;
    prefs.ram_max_used_busy_frac=td;
    mask.ram_max_used_busy_frac=true;

    m_txtMemoryMaxOnIdle->GetValue().ToDouble(&td);
    td = clamp_pct(td) / 100.0;
    prefs.ram_max_used_idle_frac=td;
    mask.ram_max_used_idle_frac=true;

    prefs.leave_apps_in_memory = m_chkMemoryWhileSuspended->GetValue();
    mask.leave_apps_in_memory=true;
    return true;
}

/// Set state of control depending on other control's state.
void CDlgAdvPreferences::UpdateControlStates() {
    //proc usage page
    m_txtProcIdleFor->Enable(!m_chkProcInUse->IsChecked());
    m_txtProcMonday->Enable(m_chkProcMonday->IsChecked());
    m_txtProcTuesday->Enable(m_chkProcTuesday->IsChecked());
    m_txtProcWednesday->Enable(m_chkProcWednesday->IsChecked());
    m_txtProcThursday->Enable(m_chkProcThursday->IsChecked());
    m_txtProcFriday->Enable(m_chkProcFriday->IsChecked());
    m_txtProcSaturday->Enable(m_chkProcSaturday->IsChecked());
    m_txtProcSunday->Enable(m_chkProcSunday->IsChecked());

    //net usage page
    m_txtNetMonday->Enable(m_chkNetMonday->IsChecked());
    m_txtNetTuesday->Enable(m_chkNetTuesday->IsChecked());
    m_txtNetWednesday->Enable(m_chkNetWednesday->IsChecked());
    m_txtNetThursday->Enable(m_chkNetThursday->IsChecked());
    m_txtNetFriday->Enable(m_chkNetFriday->IsChecked());
    m_txtNetSaturday->Enable(m_chkNetSaturday->IsChecked());
    m_txtNetSunday->Enable(m_chkNetSunday->IsChecked());
}

/// Validate the entered information.
bool CDlgAdvPreferences::ValidateInput() {
    wxString invMsgFloat = _("invalid float");
    wxString invMsgTime = _("invalid time, format is HH:MM");
    wxString invMsgInterval = _("invalid time interval, format is HH:MM-HH:MM");
    wxString buffer;
    //proc page
    if (m_txtProcIdleFor->IsEnabled()) {
        buffer = m_txtProcIdleFor->GetValue();
        if (!IsValidFloatValue(buffer)) {
            ShowErrorMessage(invMsgFloat, m_txtProcIdleFor);
            return false;
        }
    }

    buffer = m_txtProcEveryDayStart->GetValue();
    if (!IsValidTimeValue(buffer)) {
        ShowErrorMessage(invMsgTime, m_txtProcEveryDayStart);
        return false;
    }
    buffer = m_txtProcEveryDayStop->GetValue();
    if (!IsValidTimeValue(buffer)) {
        ShowErrorMessage(invMsgTime, m_txtProcEveryDayStop);
        return false;
    }
    //all text ctrls in proc special time panel
    wxWindowList children = m_panelProcSpecialTimes->GetChildren();
    wxWindowListNode* node = children.GetFirst();
    while (node) {
        if (node->GetData()->IsKindOf(CLASSINFO(wxTextCtrl))) {
            wxTextCtrl*  txt = wxDynamicCast(node->GetData(), wxTextCtrl);
            if (txt) {
                if (txt->IsEnabled()) {
                    buffer = txt->GetValue();
                    if (!IsValidTimeIntervalValue(buffer)) {
                        ShowErrorMessage(invMsgInterval, txt);
                        return false;
                    }
                }
            }
        }
        node = node->GetNext();
    }
    //net page

    buffer = m_txtNetEveryDayStart->GetValue();
    if (!IsValidTimeValue(buffer)) {
        ShowErrorMessage(invMsgTime, m_txtNetEveryDayStart);
        return false;
    }

    buffer = m_txtNetEveryDayStop->GetValue();
    if (!IsValidTimeValue(buffer)) {
        ShowErrorMessage(invMsgTime, m_txtNetEveryDayStop);
        return false;
    }
    //limit additional days from 0 to 10
    double td;
    m_txtNetAdditionalDays->GetValue().ToDouble(&td);
    if (td > 10.0 || td < 0.0) {
        ShowErrorMessage(invMsgFloat, m_txtNetAdditionalDays);
        return false;
    }
    //all text ctrls in net special time panel

    children = m_panelNetSpecialTimes->GetChildren();
    node = children.GetFirst();
    while (node) {
        if (node->GetData()->IsKindOf(CLASSINFO(wxTextCtrl))) {
            wxTextCtrl* txt = wxDynamicCast(node->GetData(), wxTextCtrl);
            if (txt) {
                if (txt->IsEnabled()) {
                    buffer = txt->GetValue();
                    if (!IsValidTimeIntervalValue(buffer)) {
                        ShowErrorMessage(invMsgInterval, txt);
                        return false;
                    }
                } //if(txt->IsEnabled())
            } //if(txt)
        } //if(node->GetData()
        node = node->GetNext();
    }

    return true;
}

/// Ensure that the page which contains txtCtrl is selected.
bool CDlgAdvPreferences::EnsureTabPageVisible(wxTextCtrl* txtCtrl) {
    wxWindow* parent = txtCtrl->GetParent();
    wxASSERT(parent);
    int parentid = parent->GetId();
    int index = m_arrTabPageIds.Index(parentid);
    if(index == wxNOT_FOUND) {
        //some controls are containe din a additional panel, so look at its parent
        parent = parent->GetParent();
        wxASSERT(parent);
        parentid = parent->GetId();
        index = m_arrTabPageIds.Index(parentid);
        if(index == wxNOT_FOUND) {
            //this should never happen
            return false;
        }
    }
    m_Notebook->SetSelection(index);
    return true;
}

/// Show an error message and set the focus to the control that caused the error.
void CDlgAdvPreferences::ShowErrorMessage(wxString& message,wxTextCtrl* errorCtrl) {
    wxASSERT(this->EnsureTabPageVisible(errorCtrl));
    errorCtrl->SetFocus();
    //
    if(message.IsEmpty()){
        message = _("invalid input value detected");
    }
    wxMessageBox(message,_("Validation Error"),wxOK | wxCENTRE | wxICON_ERROR,this);
}

/// Check if ch is a valid character for float values.
bool CDlgAdvPreferences::IsValidFloatChar(const wxChar& ch) {
    //don't accept the e
    return wxIsdigit(ch) || ch=='.' || ch==',' || ch=='+' || ch=='-';
}

/// Check if ch is a valid character for time values.
bool CDlgAdvPreferences::IsValidTimeChar(const wxChar& ch) {
    return wxIsdigit(ch) || ch==':';
}

/// Check if ch is a valid character for time interval values.
bool CDlgAdvPreferences::IsValidTimeIntervalChar(const wxChar& ch) {
    return IsValidTimeChar(ch) || ch=='-';
}

/// Checks if the value contains a valid float.
bool CDlgAdvPreferences::IsValidFloatValue(const wxString& value) {
    for(unsigned int i=0; i < value.Length();i++) {
        if(!IsValidFloatChar(value[i])) {
            return false;
        }
    }
    //all chars are valid, now what is with the value as a whole ?
    double td;
    if(!value.ToDouble(&td)) {
        return false;
    }
    return true;
}

/// Checks if the value is a valid time.
bool CDlgAdvPreferences::IsValidTimeValue(const wxString& value) {
    for (unsigned int i = 0; i < value.Length(); ++i) {
        if (!IsValidTimeChar(value[i])) {
            return false;
        }
    }
    // all chars are valid, now what is with the value as a whole ?
    wxDateTime dt;
    const wxChar* stopChar = dt.ParseFormat(value,wxT("%H:%M"));
    if ((stopChar == NULL) && (value != wxT("24:00"))) {
        // conversion failed
        return false;
    }
    return true;
}

/// Checks if the value is a valid time interval, format HH:MM-HH:MM.
bool CDlgAdvPreferences::IsValidTimeIntervalValue(const wxString& value) {
    for(unsigned int i=0; i < value.Length();i++) {
        if(!IsValidTimeIntervalChar(value[i])) {
            return false;
        }
    }
    //all chars are valid, now what is with the value as a whole ?
    //check for -
    if(value.Find('-')<0) {
        return false;
    }
    //split up into start and stop
    wxString start = value.BeforeFirst('-');
    wxString stop = value.AfterFirst('-');
    //validate start and stop parts
    if(!IsValidTimeValue(start) || !IsValidTimeValue(stop)) {
        return false;
    }
    //ensure that start is lower than stop
    wxDateTime dtStart,dtStop;
    dtStart.ParseFormat(start,wxT("%H:%M"));
    dtStop.ParseFormat(stop,wxT("%H:%M"));
    //
    /*if(dtStart>=dtStop) {
        return false;
    }*/
    return true;
}

// ------------ Event handlers start here
// -------- generic command handler
/// Handles all control command events.
void CDlgAdvPreferences::OnHandleCommandEvent(wxCommandEvent& ev) {
    ev.Skip();
    if(!m_bInInit) {
        m_bDataChanged=true;
    }
    UpdateControlStates();
}

// ---- command buttons handlers
/// Handles OK button clicked.
void CDlgAdvPreferences::OnOK(wxCommandEvent& ev) {
    CMainDocument*    pDoc = wxGetApp().GetDocument();

    wxASSERT(pDoc);
    wxASSERT(wxDynamicCast(pDoc, CMainDocument));

    if(!ValidateInput()) {
        return;
    }
    if(SavePreferencesSettings()) {
        pDoc->rpc.set_global_prefs_override_struct(prefs,mask);
        pDoc->rpc.read_global_prefs_override();
    }
    ev.Skip();
}

/// Handles Help button clicked.
void CDlgAdvPreferences::OnHelp(wxCommandEvent& ev) {
    wxString url = wxGetApp().GetSkinManager()->GetAdvanced()->GetOrganizationWebsite();
    url += wxT("/prefs.php"); //this seems not the right url, but which instead ?
    HyperLink::ExecuteLink(url);
    ev.Skip();
}

/// Handles Clear button clicked.
void CDlgAdvPreferences::OnClear(wxCommandEvent& ev) {
    if(this->ConfirmClear()) {
        CMainDocument* pDoc = wxGetApp().GetDocument();

        wxASSERT(pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));

        mask.clear();
        pDoc->rpc.set_global_prefs_override_struct(prefs,mask);
        pDoc->rpc.read_global_prefs_override();
        this->EndModal(wxID_CANCEL);
    }
    ev.Skip();
}

bool CDlgAdvPreferences::ConfirmClear() {
    int res = wxMessageBox(_("Do you really want to clear all local preferences ?"),
        _("Confirmation"), wxCENTER | wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT, this);

    return res == wxYES;
}
