// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 Peter Kortschack
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

#pragma once

BOOL IsTerminalServicesEnabled();
BOOL ValidateProductSuite(LPSTR SuiteName);
BOOL TerminateProcessById(DWORD dwProcessId);
BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);
BOOL AddAceToDesktop(HDESK hdesk, PSID psid);
BOOL GetAccountSid(
    LPCTSTR SystemName,         ///< where to lookup account
    LPCTSTR AccountName,        ///< account of interest
    PSID *Sid                   ///< resultant buffer containing SID
);
int suspend_or_resume_threads(DWORD pid, bool resume);
void chdir_to_data_dir();
