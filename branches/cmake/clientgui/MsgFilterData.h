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

#ifndef MSGFILTERDATA_H
#define MSGFILTERDATA_H

#include <set>
#include <string>

typedef std::set<std::string> string_set;

class MsgFilterData {
public:
    MsgFilterData();
    ~MsgFilterData();
    MsgFilterData(const MsgFilterData& other);

    MsgFilterData& operator=(const MsgFilterData& other);

    /// Get the number of visible messages.
    unsigned int GetNumVisibleMsg() const;
    
    /// Set the number of visible messages.
    void SetNumVisibleMsg(unsigned int val);
    
    /// Get the selected projects.
    const string_set& GetSelectedProjects() const;
    
    /// Checks if the given project is selected.
    bool IsProjectSelected(const std::string& project) const;
    
    /// Adds a project to the list of selected projects.
    void SelectProject(const std::string& project);
    
    /// Get the selected debug flags.
    const string_set& GetSelectedDebugFlags() const;
    
    /// Checks if the given debug flag is selected.
    bool IsDebugFlagSelected(const std::string& debugFlag) const;
    
    /// Adds a debug flag to the list of selected debug flags.
    void SelectDebugFlag(const std::string& debugFlag);

    /// Checks if there is a debug flag with the given name.
    bool IsDebugFlagValid(const std::string& debugFlag) const;
    
    /// Reset the content to default values.
    void ResetToDefaults();
        
    /// Clear all stored data.
    void Clear();
    
private:
    unsigned int m_numVisibleMsg; ///< Number of visible messages.
    string_set m_projects; ///< Selected projects.
    string_set m_debugFlags; ///< Selected debug flags.
    static string_set ms_validDebugFlags; ///< All existing debug flags.
};

#endif // MSGFILTERDATA_H
