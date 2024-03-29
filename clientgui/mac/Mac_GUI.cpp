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

//  Mac_GUI.cpp

#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

#include <unistd.h>
#include "sandbox.h"

/// Determine if the currently logged-in user is auhorized to 
/// perform operations which have potential security risks.  
/// An example is "Attach to Project", where a dishonest user might
/// attach to a rogue project which could then read private files 
/// belonging to the user who owns the BOINC application.  This 
/// would be possible because the BOINC Manager runs with the 
/// effectve user ID of its owner on the Mac.

bool Mac_Authorize()
{
    static bool sIsAuthorized = false;

    AuthorizationRef ourAuthRef = NULL;
    AuthorizationRights ourAuthRights;
    AuthorizationFlags ourAuthFlags;
    AuthorizationItem ourAuthItem[1];

    OSStatus err = noErr;
    
    if (sIsAuthorized)
        return true;
        
    // User is not the owner, so require admin authentication
    ourAuthItem[0].name = kAuthorizationRightExecute;
    ourAuthItem[0].value = NULL;
    ourAuthItem[0].valueLength = 0;
    ourAuthItem[0].flags = 0;

    ourAuthRights.count = 1;
    ourAuthRights.items = ourAuthItem;
    
    ourAuthFlags = kAuthorizationFlagInteractionAllowed | kAuthorizationFlagExtendRights;

    err = AuthorizationCreate (&ourAuthRights, kAuthorizationEmptyEnvironment, ourAuthFlags, &ourAuthRef);

    if (err == noErr) {
        sIsAuthorized = true;
        // We have authenticated user's credentials; we won't actually use the 
        // privileges / rights so destroy / discard them.
        err = AuthorizationFree(ourAuthRef, kAuthorizationFlagDestroyRights);
    }
        
    return sIsAuthorized;
}
