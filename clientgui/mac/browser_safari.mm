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

#include "str_util.h"
#include "browser.h"

#include <Cocoa/Cocoa.h>

bool detect_setup_authenticator_safari(std::string& project_url, std::string& authenticator)
{    
    NSHTTPCookieStorage *cookieStorage;
    NSArray *theCookies;
    NSHTTPCookie *aCookie;
    NSURL *theURL;
    NSString *theURLString, *theValueString, *theNameString;
    NSDate *expirationDate;
    unsigned int i, n;
    bool retval = false;

    NSAutoreleasePool* pool;
    
    pool = [[NSAutoreleasePool alloc] init];
    
    
    theURLString = [ NSString stringWithCString:project_url.c_str() ];
    
    theURL = [ NSURL URLWithString:theURLString ];

    cookieStorage = [ NSHTTPCookieStorage sharedHTTPCookieStorage ];
    
    if (cookieStorage == NULL)
        goto bail;
    
    theCookies = [ cookieStorage cookiesForURL:theURL ];

    if (theCookies == NULL)
        goto bail;

    n = [ theCookies count ];
    for (i=0; i<n; i++) {
        aCookie = (NSHTTPCookie*)[ theCookies objectAtIndex:i ];

        // has the cookie expired?
        expirationDate = [ aCookie expiresDate ];
        if ([ expirationDate compare:[ NSDate date ]] == NSOrderedAscending)
            continue;
            
        theNameString = [ aCookie name ];
        // is this the right cookie?
#ifdef cStringUsingEncoding     // Available only is OS 10.4 and later
        if (!starts_with([ theNameString cStringUsingEncoding:NSMacOSRomanStringEncoding ], "Setup"))
            continue;
        theValueString = [ aCookie value ];
        authenticator = [ theValueString cStringUsingEncoding:NSMacOSRomanStringEncoding ];
#else
        if (!starts_with([ theNameString cString ], "Setup"))
            continue;
        theValueString = [ aCookie value ];
        authenticator = [ theValueString cString ];
#endif
        // If validation failed, null out the authenticator just in case
        //   somebody tries to use it, otherwise copy in the real deal.
        if (is_authenticator_valid(authenticator)) {
            retval = true;
            break;
        } else {
            authenticator = "";
        }
    }

bail:
    [pool release];

    return retval;
}
/* vim: set ft=objcpp: */
