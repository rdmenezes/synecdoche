// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California, 2009 Michael Tughan
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

/*
 *  SetVersion.c
 *  boinc
 *
 *  Created by Charlie Fenton on 3/29/05.
 *
 */

// Set STAND_ALONE TRUE if testing as a separate applicaiton
#define STAND_ALONE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include "version.h"

int IsFileCurrent(const char* filePath);
int FixInfoPlistFile(const char* myPath);
int FixInfoPlist_Strings(const char* myPath, const char* brand);
int MakeInstallerInfoPlistFile(const char* myPath, const char* brand);

int main(int argc, char** argv) {
    int retval = 0, err;

#if STAND_ALONE
    char myPath[1024];
    getcwd(myPath, sizeof(myPath));
    printf("%s\n", myPath);       // For debugging
    err = chdir("../");
    getcwd(myPath, sizeof(myPath));
    printf("%s\n", myPath);       // For debugging
#endif

    err = FixInfoPlist_Strings("./English.lproj/InfoPlist.strings", "Synecdoche");
    if (err) retval = err;
    err = FixInfoPlistFile("./Info.plist");
    if (err) retval = err;
    //err = FixInfoPlistFile("./Installer-Info.plist");
    //if (err) retval = err;
    //err = FixInfoPlistFile("./PostInstall-Info.plist");
    //if (err) retval = err;
    err = FixInfoPlistFile("./ScreenSaver-Info.plist");
    if (err) retval = err;
    err = FixInfoPlistFile("./SystemMenu-Info.plist");
    if (err) retval = err;
    //err = FixInfoPlistFile("./Uninstaller-Info.plist");
    //if (err) retval = err;
    err = MakeInstallerInfoPlistFile("./Pkg-Info.plist", "Synecdoche Manager");
    return retval;
}

std::string version_string() {
	std::string version = SYNEC_VERSION_STRING;
#if SYNEC_PRERELEASE
	if(SYNEC_SVN_VERSION) {
		version += " r";
		version += SYNEC_SVN_VERSION;
	}
#endif
	return version;
}

int IsFileCurrent(const char* filePath) {
    FILE *f;
    char *c, buf[1024];
    
    f = fopen(filePath, "r");
    if (f == 0)
        return false;
    for (;;) {
        c = fgets(buf, sizeof(buf), f);
        if (c == NULL)
            break;   // EOF reached without finding correct version string
        c = strstr(buf, (version_string() + "<").c_str());
        if (c) {
            fclose(f);
            return true;  // File contains current version string
        }
        else {
            c = strstr(buf, (version_string() + "\"").c_str());
            if (c) {
                fclose(f);
                return true;  // File contains current version string
            }
        }
    }
    fclose(f);
    return false;  // File does not contain current version string
}


int FixInfoPlist_Strings(const char* myPath, const char* brand) {
    int retval = 0;
    std::string versionString = version_string();
    FILE *f;
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "/* Localized versions of Info.plist keys */\n\n");
        fprintf(f, "CFBundleName = \"%s\";\n", brand);
        fprintf(f, "CFBundleShortVersionString = \"%s version %s\";\n", brand, versionString.c_str());
        fprintf(f, "CFBundleGetInfoString = \"%s version %s, Copyright 2009 Synecdoche.\";\n", brand, versionString.c_str());
        fflush(f);
        retval = fclose(f);
    }
    else {
        puts("Error updating version number in file InfoPlist.strings\n");
        retval = -1;
    }
        
    return retval;
}


int FixInfoPlistFile(const char* myPath) {
    int retval = 0;
    FILE *fin = NULL, *fout = NULL;
    char *c, a, buf[1024];
	std::string filePath = myPath;
    
    if (IsFileCurrent((const char*)filePath.c_str()))
        return 0;

    rename((const char*)filePath.c_str(), (const char*)(("./" + filePath + ".back").c_str()));
//    sprintf(buf, "mv -f %s temp", myPath);
//    retval = system(buf);

    fin = fopen((const char*)(filePath + ".template").c_str(), "r");
    if (fin == NULL)
        goto bail;

    fout = fopen((const char*)filePath.c_str(), "w");
    if (fout == NULL) {
        goto bail;
    }

    // Copy everything up to version number
    for (;;) {
        c = fgets(buf, sizeof(buf), fin);
        if (c == NULL)
            goto bail;   // EOF
        c = strstr(buf, "CFBundleVersion</key>");
        if (c)
            break;  // Found "CFBundleVersion</key>"
        fputs(buf, fout);
    }
        
    c = strstr(buf, "<string>");
    if (c == NULL) {
        fputs(buf, fout);
        c = fgets(buf, sizeof(buf), fin);
        if (c == NULL)
            goto bail;   // EOF
        c = strstr(buf, "<string>");
        if (c == NULL)
            goto bail;   // "CFBundleVersion</key>" not followed by "<string>"
    }
    
    a = *(c+8);
    *(c+8) = '\0';                      // Put terminator after "<string>"
    fputs(buf, fout);                   // Copy up to end of "<string>"
    fputs(version_string().c_str(), fout);  // Write the current version number
    *(c+8) = a;                         // Undo terminator we inserted
    c = strstr(buf, "</string>");       // Skip over old version number in input
    fputs(c, fout);                     // Copy rest of input line

    // Copy rest of file
    for (;;) {
        c = fgets(buf, sizeof(buf), fin);
        if (c == NULL)
            break;   // EOF
        fputs(buf, fout);
    }

    fclose(fin);
    fflush(fout);
    fclose(fout);
    
    unlink((const char*)(filePath + ".back").c_str());
    
    return retval;

bail:
    if (fin)
        fclose(fin);
    if (fout)
        fclose(fout);

    rename((const char*)("./" + filePath + ".back").c_str(), (const char*)filePath.c_str());
//    sprintf(buf, "mv -f temp %s", myPath);
//    retval = system(buf);
    
    printf("Error updating version number in file %s\n", myPath);
    return -1;
}


int MakeInstallerInfoPlistFile(const char* myPath, const char* brand) {
    int retval = 0;
    std::string versionString = version_string();
    FILE *f;
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
        fprintf(f, "<plist version=\"1.0\">\n<dict>\n");
        fprintf(f, "\t<key>CFBundleGetInfoString</key>\n");
        fprintf(f, "\t<string>%s %s</string>\n", brand, versionString.c_str());
        fprintf(f, "\t<key>CFBundleIdentifier</key>\n\t<string>%s</string>\n", "${BUNDLE_IDENTIFIER}");
        fprintf(f, "\t<key>CFBundleShortVersionString</key>\n");
        fprintf(f, "\t<string>%s</string>\n", versionString.c_str());
        fprintf(f, "\t<key>IFPkgFlagAllowBackRev</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagAuthorizationAction</key>\n\t<string>AdminAuthorization</string>\n");
        fprintf(f, "\t<key>IFPkgFlagDefaultLocation</key>\n\t<string>/</string>\n");
        fprintf(f, "\t<key>IFPkgFlagFollowLinks</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagInstallFat</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagInstalledSize</key>\n\t<integer>6680</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagIsRequired</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagOverwritePermissions</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagRelocatable</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagRestartAction</key>\n\t<string>RequiredLogout</string>\n");
        fprintf(f, "\t<key>IFPkgFlagRootVolumeOnly</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagUpdateInstalledLanguages</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFormatVersion</key>\n\t<real>0.10000000149011612</real>\n");
        fprintf(f, "</dict>\n</plist>\n");

        fflush(f);
        retval = fclose(f);
    }
    else {
        puts("Error creating file Pkg-Info.plist\n");
        retval = -1;
    }
        
    return retval;
}


const char *BOINC_RCSID_9263a2dc22="$Id: SetVersion.C 13804 2007-10-09 11:35:47Z fthomas $";
