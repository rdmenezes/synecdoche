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

#include "browser.h"

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <string>
#include <vector>
#include <time.h>
#endif

#include <cstring>

#include "error_numbers.h"
#include "mfile.h"
#include "miofile.h"
#include "str_util.h"


//
// Utility Functions
//

#ifdef _WIN32

// Prototype for SHGetFolderPath() in shell32.dll.
typedef HRESULT (WINAPI *MYSHGETFOLDERPATH)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);

#endif

// retrieve the user's application data directory.
// Win  : C:\Documents and Settings\<username>\Application Data
// Unix: ~/
//
void get_home_dir_path( std::string& path ) {
#ifdef _WIN32
    TCHAR               szBuffer[MAX_PATH];
    HMODULE             hShell32;
    MYSHGETFOLDERPATH   pfnMySHGetFolderPath = NULL;

    // Attempt to link to dynamic function if it exists
    hShell32 = LoadLibrary(_T("SHELL32.DLL"));
    if (NULL != hShell32) {
        pfnMySHGetFolderPath = (MYSHGETFOLDERPATH) GetProcAddress(hShell32, _T("SHGetFolderPathA"));
    }

    if (NULL != pfnMySHGetFolderPath) {
        if (SUCCEEDED((pfnMySHGetFolderPath)(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szBuffer))) {
            path  = std::string(szBuffer);
            path += std::string("\\");
        }
    }

    // Free the dynamically linked to library
    if (NULL != hShell32) {
        FreeLibrary(hShell32);
    }
#elif defined(__APPLE__)
    path = std::string(getenv("HOME") )+ std::string("/");
#else
    path = std::string("~/");
#endif
}

// parse name value pairs based on INI file rules.
bool parse_name_value_pair(char* buf, std::string& name, std::string& value) {
    std::basic_string<char>::size_type i;
    std::string s;

    s = std::string(buf);
    i = s.find("=", 0);
    if ( i < s.npos ) {
        name = s.substr(0, i);
        value = s.substr(i + 1);
        strip_whitespace(name);
        strip_whitespace(value);
        return true;
    }
    return false;
}


// parse host name from url.
bool parse_hostname(std::string& project_url, std::string& hostname) {
    std::basic_string<char>::size_type start;
    std::basic_string<char>::size_type end;

    start = project_url.find("//", 0) + 2;
    end   = project_url.find("/", start);

    hostname = project_url.substr(start, end - start);
    if (starts_with(hostname.c_str(), "www"))
        hostname.erase(0, 3);
    if (!hostname.empty())
        return true;
    return false;
}


// is the authenticator valid?
bool is_authenticator_valid(const std::string authenticator) {
    std::string tmp = authenticator;

    // Perform some basic validation of the suspect authenticator
    //

    // If the string is null then it is invalid.
    if (0 == tmp.length()) {
        return false;
    }

    // If the string contains non alpha numeric characters it is invalid.
    std::string::iterator it = tmp.begin();
    while (it != tmp.end()) {
        if (!isalpha(*it) && !isdigit(*it)) {
            return false;
        }
        it++;
    }

    return true;
}

//
// Mozilla-Based Browser Support
//
class MOZILLA_PROFILE {
public:
    std::string name;
    std::string path;
    bool        is_relative;
    bool        is_default;

    MOZILLA_PROFILE();

    void clear();
    int parse(MIOFILE& in);
};


class MOZILLA_PROFILES {
public:
    std::vector<MOZILLA_PROFILE*> profiles;

    MOZILLA_PROFILES();

    void clear();
    int parse(MIOFILE& in);
};


MOZILLA_PROFILE::MOZILLA_PROFILE() {
    clear();
}


void MOZILLA_PROFILE::clear() {
    name.clear();
    path.clear();
    is_relative = false;
    is_default = false;
}


int MOZILLA_PROFILE::parse(MIOFILE& in) {
    char buf[512];
    std::string sn;
    std::string sv;

    while (in.fgets(buf, 512)) {
        if (starts_with(buf, "\n")) return 0;
        if (starts_with(buf, "\r\n")) return 0;
        if (!parse_name_value_pair(buf, sn, sv)) continue;

        if ("Name" == sn) name = sv;
        if ("Path" == sn) path = sv;
        if (("IsRelative" == sn) && ("1" == sv)) is_relative = true;
        if (("Default" == sn) && ("1" == sv)) is_default = true;
    }
    return ERR_FREAD;
}


MOZILLA_PROFILES::MOZILLA_PROFILES() {
    clear();
}


void MOZILLA_PROFILES::clear() {
    unsigned int i;
    for (i=0; i<profiles.size(); i++) {
        delete profiles[i];
    }
    profiles.clear();
}


int MOZILLA_PROFILES::parse(MIOFILE& in) {
    char buf[512];
    MOZILLA_PROFILE* p = NULL;

    while (in.fgets(buf, 512)) {
        if (starts_with(buf, "[Profile")) {
            p = new MOZILLA_PROFILE;
            if (!p->parse( in )) {
                profiles.push_back( p );
            }
        }
    }

    return 0;
}

// search for the project specific 'Setup' cookie for mozilla based
// browsers.
//
// file format is:
// 
// host \t isDomain \t path \t secure \t expires \t name \t cookie
// 
// if this format isn't respected we move onto the next line in the file.
// isDomain is "TRUE" or "FALSE" (default to "FALSE")
// isSecure is "TRUE" or "FALSE" (default to "TRUE")
// expires is a PRInt64 integer
// note 1: cookie can contain tabs.
// note 2: cookies are written in order of lastAccessed time:
//         most-recently used come first; least-recently-used come last.
// 
bool find_project_cookie_mozilla_generic(
    MIOFILE& in, std::string& project_url, std::string& authenticator
) {
    bool retval = false;
    char buf[2048];
    char host[256], domain[16], path[256], secure[16], name[256], cookie[256];
    long long expires;
    std::string hostname;


    strcpy(host, "");
    strcpy(domain, "");
    strcpy(path, "");
    strcpy(secure, "");
    strcpy(name, "");
    strcpy(cookie, "");
    expires = 0;


    // determine the project hostname using the project url
    parse_hostname(project_url, hostname);

    // traverse cookie file
    while (in.fgets(buf, 2048)) {
        sscanf(
            buf,
#ifdef _WIN32
            "%255s\t%15s\t%255s\t%15s\t%I64d\t%255s\t%255s",
#else
            "%255s\t%15s\t%255s\t%15s\t%Ld\t%255s\t%255s",
#endif
            host, domain, path, secure, &expires, name, cookie
        );

        // is this a real cookie?
        // temporary cookie? these cookies do not trickle back up
        // to the jscript interface, so ignore them.
        if (starts_with(host, "#HttpOnly")) continue;

        // is this the right host?
        if (!strstr(host, hostname.c_str())) continue;

        // has the cookie expired?
        if (time(0) > expires) continue;

        // is this the right cookie?
        if (starts_with(name, "Setup")) {
            // If validation failed, null out the authenticator just in case
            //   somebody tries to use it, otherwise copy in the real deal.
            if (!is_authenticator_valid(cookie)) {
                authenticator = "";
            } else {
                authenticator = cookie;
                retval = true;
            }
        }
    }

    return retval;
}

// traverse the profiles and determine which cookie fle to use.
// this should be compatible with firefox, seamonkey, and netscape.
//
bool detect_setup_authenticator_mozilla_generic(
    std::string profiles_root, std::string& project_url, std::string& authenticator
) {
    bool retval = false;
    FILE* pf = NULL;
    FILE* cf = NULL;
    MIOFILE pmf;
    MIOFILE cmf;
    MOZILLA_PROFILES mps;
    MOZILLA_PROFILE* mp = NULL;
    std::string cookies_root;
    std::string tmp;
    unsigned int i = 0;
    unsigned int default_index = 0;

    // lets see if we can open the profiles configuration file
    tmp = profiles_root + "profiles.ini";
    pf = fopen(tmp.c_str(), "r");

    // if profiles configuration file exists, parse it.
    if (pf) {
        pmf.init_file(pf);
        mps.parse(pmf);
    }

    // we need to know which cookie file to look at, so if only
    // one profile exists, use it.
    //
    // if more than one profile exists, look through all the
    // profiles until we find the default profile. even when the
    // user selects a different profile at startup the default
    // profile flag is changed at startup to the new profile.
    if (mps.profiles.size() == 0) {
        fclose(pf);
        return retval;          // something is very wrong, don't
                                // risk a crash
    }

    if (mps.profiles.size() == 1) {
        default_index = 0;
    } else {
        for (i=0; i<mps.profiles.size(); i++) {
            if (mps.profiles[i]->is_default) {
                default_index = i;
                break;
            }
        }
    }

    // should the path be treated as an absolute path or a relative
    // path?
    mp = mps.profiles[default_index];
    if (mp->is_relative) {
        cookies_root = profiles_root + mp->path + "/";
    } else {
        cookies_root = mp->path + "/";
    }

    // now we should open up the cookie file.
    tmp = cookies_root + "cookies.txt";
    cf = fopen(tmp.c_str(), "r");

    // if the cookie file exists, lookup the projects 'Setup' cookie.
    if (cf) {
        cmf.init_file(cf);
        retval = find_project_cookie_mozilla_generic(
            cmf,
            project_url,
            authenticator
        );
    }

    // cleanup
    if (cf) fclose(cf);
    if (pf) fclose(pf);

    return retval;
}

    
//
// Firefox Browser Support
//

bool get_firefox_profiles_root( std::string& profiles_root ) {
    get_home_dir_path( profiles_root );
#ifdef _WIN32
    profiles_root += std::string("Mozilla\\Firefox\\");
#elif defined(__APPLE__)
    profiles_root += std::string("Library/Application Support/Firefox/");
#else
    profiles_root += std::string(".mozilla/firefox/");
#endif
    return true;
}


bool detect_setup_authenticator_firefox(
    std::string& project_url, std::string& authenticator
) {
    std::string profiles_root;
    get_firefox_profiles_root(profiles_root);
    return detect_setup_authenticator_mozilla_generic(
        profiles_root,
        project_url,
        authenticator
    );
}


#ifdef _WIN32
//
// Internet Explorer Browser Support
//

//
// Detect the 'Setup' cookie in Internet Explorer by using the InternetGetCookie API.
//
bool detect_setup_authenticator_ie(std::string& project_url, std::string& authenticator)
{
    bool        bReturnValue = false;
    char        szCookieBuffer[2048];
    char*       pszCookieFragment = NULL;
    DWORD       dwSize = sizeof(szCookieBuffer)/sizeof(char);
    std::string strCookieFragment;
    std::string strCookieName;
    std::string strCookieValue;
    size_t      uiDelimeterLocation;
    std::string hostname;


    // if we don't find the cookie at the exact project dns name, check one higher (i.e. www.worldcommunitygrid.org becomes
    // worldcommunitygrid.org
    parse_hostname(project_url, hostname);

    bReturnValue = InternetGetCookie(project_url.c_str(), NULL, szCookieBuffer, &dwSize) == TRUE;
    if (!bReturnValue) bReturnValue = InternetGetCookie(hostname.c_str(), NULL, szCookieBuffer, &dwSize) == TRUE;
    if (bReturnValue)
    {
        // reset this becuase at this point we just know that we found some cookies for the website.  We don't
        // know if we actually found the Setup cookie
        //
        bReturnValue = false;
        // Format of cookie buffer:
        // 'cookie1=value1; cookie2=value2; cookie3=value3;
        //
        pszCookieFragment = strtok(szCookieBuffer, "; ");
        while(pszCookieFragment)
        {
            // Convert to a std::string so we can go to town
            strCookieFragment = pszCookieFragment;

            // Extract the name & value
            uiDelimeterLocation = strCookieFragment.find("=", 0);
            strCookieName = strCookieFragment.substr(0, uiDelimeterLocation);
            strCookieValue = strCookieFragment.substr(uiDelimeterLocation + 1);

            if (std::string("Setup") == strCookieName)
            {
                // If validation failed, null out the authenticator just in case
                //   somebody tries to use it, otherwise copy in the real deal.
                if (!is_authenticator_valid(strCookieValue)) {
                    authenticator = "";
                } else {
                    // Now we found it!  Yea - auto attach!
                    authenticator = strCookieValue;
                    bReturnValue = true;
                }
            }

            pszCookieFragment = strtok(NULL, "; ");
        }
    }

    return bReturnValue;
}
#endif


//
// walk through the various browsers looking up the
// project cookies until the projects 'Setup' cookie is found.
//
// give preference to the default platform specific browers first before going
// to the platform independant browsers since most people don't switch from
// the default.
// 
bool detect_setup_authenticator(
    std::string& project_url, std::string& authenticator
) {
#ifdef _WIN32
    if (detect_setup_authenticator_ie(project_url, authenticator)) {
        return true;
    }
#endif
#ifdef __APPLE__
    if (detect_setup_authenticator_safari(project_url, authenticator)) {
        return true;
    }
#endif
    if (detect_setup_authenticator_firefox(project_url, authenticator)) {
        return true;
    }

    return false;
}
