// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#include "MsgFilterData.h"

#include "BOINCGUIApp.h"
#include "MainDocument.h"

string_set MsgFilterData::ms_validDebugFlags;

MsgFilterData::MsgFilterData() {
    ResetToDefaults();
}

MsgFilterData::~MsgFilterData() {
}

MsgFilterData::MsgFilterData(const MsgFilterData& other) {
    *this = other;
}

/// Assignment operator
///
/// \param[in] other Object to assign from
/// \return A reference to *this
MsgFilterData& MsgFilterData::operator=(const MsgFilterData& rhs) {
    if (this == &rhs) {  // handle self assignment
        return *this;
    }
    m_numVisibleMsg = rhs.m_numVisibleMsg;
    m_projects      = rhs.m_projects;
    m_debugFlags    = rhs.m_debugFlags;
    return *this;
}

/// Get the number of visible messages.
///
/// \return The number of visible messages.
unsigned int MsgFilterData::GetNumVisibleMsg() const {
    return m_numVisibleMsg;
}

/// Set the number of visible messages.
///
/// \param[in] val The new number of visible messages.
void MsgFilterData::SetNumVisibleMsg(unsigned int val) { 
    m_numVisibleMsg = val;
}

/// Get the selected projects.
///
/// \return A reference to the set containing all projects which should not be filtered out.
const string_set& MsgFilterData::GetSelectedProjects() const {
    return m_projects;
}

/// Checks if the given project is selected.
///
/// \param[in] project Name of the project that should be checked.
/// \return True if \a project is selected, false otherwise.
bool MsgFilterData::IsProjectSelected(const std::string& project) const {
    return m_projects.find(project) != m_projects.end();
}

/// Adds a project to the list of selected projects.
///
/// \param[in] project Name of the project that should be added to the list of selected projects.
void MsgFilterData::SelectProject(const std::string& project) {
    m_projects.insert(project);
}

/// Get the selected debug flags.
///
/// \return A reference to the set containing all debug flags which should not be filtered out.
const string_set& MsgFilterData::GetSelectedDebugFlags() const {
    return m_debugFlags;
}

/// Checks if the given debug flag is selected.
///
/// \param[in] debugFlag Name of the debug flag that should be checked.
/// \return True if \a debugFlag is selected, false otherwise.
bool MsgFilterData::IsDebugFlagSelected(const std::string& debugFlag) const {
    return m_debugFlags.find(debugFlag) != m_debugFlags.end();
}

/// Adds a debug flag to the list of selected debug flags.
///
/// \param[in] debugFlag Name of the debug flag that should be added to the list of
///                      selected debug flags.
void MsgFilterData::SelectDebugFlag(const std::string& debugFlag) {
    m_debugFlags.insert(debugFlag);
}

/// Checks if there is a debug flag with the given name.
///
/// \param[in] debugFlag The name of a debug flag which should be checked.
/// \return True if \a debugFlag denotes an existing debug flag, fals otherwise.
bool MsgFilterData::IsDebugFlagValid(const std::string& debugFlag) const {
    return ms_validDebugFlags.find(debugFlag) != ms_validDebugFlags.end();
}

/// Reset the content to default values.
/// Default currently means everything is selected and at most 1,000 lines are displayed.
void MsgFilterData::ResetToDefaults() {
    Clear();
    
    m_numVisibleMsg = 1000u;
    
    // Get all projects the client is attached to:
    project_names_vec project_names = wxGetApp().GetDocument()->GetProjectNames();
    m_projects.insert(project_names.begin(), project_names.end());
    
    // Fill the debug flags list:
    m_debugFlags.insert("app_msg_receive");
    m_debugFlags.insert("app_msg_send");
    m_debugFlags.insert("benchmark_debug");
    m_debugFlags.insert("checkpoint_debug");
    m_debugFlags.insert("cpu_sched");
    m_debugFlags.insert("cpu_sched_debug");
    m_debugFlags.insert("debt_debug");
    m_debugFlags.insert("file_xfer_debug");
    m_debugFlags.insert("guirpc_debug");
    m_debugFlags.insert("http_debug");
    m_debugFlags.insert("http_xfer_debug");
    m_debugFlags.insert("mem_usage_debug");
    m_debugFlags.insert("network_status_debug");
    m_debugFlags.insert("poll_debug");
    m_debugFlags.insert("proxy_debug");
    m_debugFlags.insert("rr_sim");
    m_debugFlags.insert("sched_op_debug");
    m_debugFlags.insert("scrsave_debug");
    m_debugFlags.insert("state_debug");
    m_debugFlags.insert("statefile_debug");
    m_debugFlags.insert("task_debug");
    m_debugFlags.insert("time_debug");
    m_debugFlags.insert("unparsed_xml");
    m_debugFlags.insert("work_fetch_debug");
    
    // Initialize static list of all available debug flags:
    if (ms_validDebugFlags.empty()) {
        ms_validDebugFlags.insert(m_debugFlags.begin(), m_debugFlags.end());
    }
}

/// Clear all stored data.
void MsgFilterData::Clear() {
    m_numVisibleMsg = 0u;
    m_projects.clear();
    m_debugFlags.clear();
}
