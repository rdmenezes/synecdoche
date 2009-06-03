// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2009 David Barnard
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

#include "boinc_win.h"
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>

#include "client_types.h"
#include "filesys.h"
#include "str_util.h"
#include "client_msgs.h"
#include "hostinfo_network.h"
#include "hostinfo.h"

HINSTANCE g_hClientLibraryDll;


// Newer system metrics values
#ifndef SM_SERVERR2
#define SM_SERVERR2 89
#endif


// Newer processor features than what is currently defined in
//   Visual Studio 2003
#ifndef PF_SSE_DAZ_MODE_AVAILABLE
#define PF_SSE_DAZ_MODE_AVAILABLE               11
#endif
#ifndef PF_NX_ENABLED
#define PF_NX_ENABLED                           12
#endif
#ifndef PF_SSE3_INSTRUCTIONS_AVAILABLE
#define PF_SSE3_INSTRUCTIONS_AVAILABLE          13
#endif
#ifndef PF_COMPARE_EXCHANGE128
#define PF_COMPARE_EXCHANGE128                  14
#endif
#ifndef PF_COMPARE64_EXCHANGE128
#define PF_COMPARE64_EXCHANGE128                15
#endif
#ifndef PF_CHANNELS_ENABLED
#define PF_CHANNELS_ENABLED                     16
#endif

// Newer product types than what is currently defined in
//   Visual Studio 2005
#ifndef PRODUCT_ULTIMATE
#define PRODUCT_ULTIMATE                        0x00000001
#endif
#ifndef PRODUCT_HOME_BASIC
#define PRODUCT_HOME_BASIC                      0x00000002
#endif
#ifndef PRODUCT_HOME_PREMIUM
#define PRODUCT_HOME_PREMIUM                    0x00000003
#endif
#ifndef PRODUCT_ENTERPRISE
#define PRODUCT_ENTERPRISE                      0x00000004
#endif
#ifndef PRODUCT_HOME_BASIC_N
#define PRODUCT_HOME_BASIC_N                    0x00000005
#endif
#ifndef PRODUCT_BUSINESS
#define PRODUCT_BUSINESS                        0x00000006
#endif
#ifndef PRODUCT_STANDARD_SERVER
#define PRODUCT_STANDARD_SERVER                 0x00000007
#endif
#ifndef PRODUCT_DATACENTER_SERVER
#define PRODUCT_DATACENTER_SERVER               0x00000008
#endif
#ifndef PRODUCT_SMALLBUSINESS_SERVER
#define PRODUCT_SMALLBUSINESS_SERVER            0x00000009
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER
#define PRODUCT_ENTERPRISE_SERVER               0x0000000A
#endif
#ifndef PRODUCT_STARTER
#define PRODUCT_STARTER                         0x0000000B
#endif
#ifndef PRODUCT_DATACENTER_SERVER_CORE
#define PRODUCT_DATACENTER_SERVER_CORE          0x0000000C
#endif
#ifndef PRODUCT_STANDARD_SERVER_CORE
#define PRODUCT_STANDARD_SERVER_CORE            0x0000000D
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER_CORE
#define PRODUCT_ENTERPRISE_SERVER_CORE          0x0000000E
#endif
#ifndef PRODUCT_ENTERPRISE_SERVER_IA64
#define PRODUCT_ENTERPRISE_SERVER_IA64          0x0000000F
#endif
#ifndef PRODUCT_BUSINESS_N
#define PRODUCT_BUSINESS_N                      0x00000010
#endif
#ifndef PRODUCT_WEB_SERVER
#define PRODUCT_WEB_SERVER                      0x00000011
#endif
#ifndef PRODUCT_CLUSTER_SERVER
#define PRODUCT_CLUSTER_SERVER                  0x00000012
#endif
#ifndef PRODUCT_HOME_SERVER
#define PRODUCT_HOME_SERVER                     0x00000013
#endif
#ifndef PRODUCT_STORAGE_EXPRESS_SERVER
#define PRODUCT_STORAGE_EXPRESS_SERVER          0x00000014
#endif
#ifndef PRODUCT_STORAGE_STANDARD_SERVER
#define PRODUCT_STORAGE_STANDARD_SERVER         0x00000015
#endif
#ifndef PRODUCT_STORAGE_WORKGROUP_SERVER
#define PRODUCT_STORAGE_WORKGROUP_SERVER        0x00000016
#endif
#ifndef PRODUCT_STORAGE_ENTERPRISE_SERVER
#define PRODUCT_STORAGE_ENTERPRISE_SERVER       0x00000017
#endif
#ifndef PRODUCT_SERVER_FOR_SMALLBUSINESS
#define PRODUCT_SERVER_FOR_SMALLBUSINESS        0x00000018
#endif
#ifndef PRODUCT_SMALLBUSINESS_SERVER_PREMIUM
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM    0x00000019
#endif


// Traverse the video adapters and flag them as potiential accelerators.
struct INTERNALMONITORINFO
{
    DWORD  cb;
    TCHAR  DeviceName[32];
    TCHAR  DeviceString[128];
    DWORD  StateFlags;
    TCHAR  DeviceID[128];
    TCHAR  DeviceKey[128];
};

// Returns the number of seconds difference from UTC
int get_timezone(int& timezone) {
    TIME_ZONE_INFORMATION tzi;
    memset(&tzi, 0, sizeof(TIME_ZONE_INFORMATION));
    DWORD result = GetTimeZoneInformation(&tzi);
    if (result == TIME_ZONE_ID_DAYLIGHT) {
        timezone = -(tzi.Bias + tzi.DaylightBias) * 60;
    } else {
        timezone = -(tzi.Bias + tzi.StandardBias) * 60;
    }
    return 0;
}

// Returns the memory information
int get_memory_info(double& bytes, double& swap) {

    MEMORYSTATUSEX mStatusEx;
    ZeroMemory(&mStatusEx, sizeof(MEMORYSTATUSEX));
    mStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&mStatusEx);
    bytes = (double)mStatusEx.ullTotalPhys;
    swap = (double)mStatusEx.ullTotalPageFile;

    return 0;
}

// Returns the OS name and version
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

int get_os_information(char* os_name, int os_name_size, char* os_version, int os_version_size)
{
    // This code snip-it was copied straight out of the MSDN Platform SDK
    //   Getting the System Version example and modified to dump the output
    //   into os_name.
    char szVersion[128];
    std::string szSKU;
    char szServicePack[128];
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    PGNSI pGNSI;
    BOOL bOsVersionInfoEx;
    DWORD dwType = 0;

    ZeroMemory(szVersion, sizeof(szVersion));
    ZeroMemory(szServicePack, sizeof(szServicePack));
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);


    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.
    bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO*)&osvi);
    if (!bOsVersionInfoEx) {
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx ((OSVERSIONINFO *) &osvi);
    }


    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");
    if(NULL != pGNSI) {
        pGNSI(&si);
    } else {
        GetSystemInfo(&si);
    }

    PGPI pGPI = (PGPI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetProductInfo");

    if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
            strlcpy(os_name, "Windows 7", os_name_size);
        } else {
            strlcpy(os_name, "Windows Server 2008 \"R2\"", os_name_size);
        }
        pGPI(6, 1, 0, 0, &dwType);
    } else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
            strlcpy(os_name, "Microsoft Windows Vista", os_name_size);
        } else {
            strlcpy(os_name, "Microsoft Windows Server 2008", os_name_size);
        }
        pGPI(6, 0, 0, 0, &dwType);
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
        if (osvi.wProductType == VER_NT_WORKSTATION) {
            strlcpy(os_name, "Microsoft Windows XP", os_name_size);
        } else {
            if(GetSystemMetrics(SM_SERVERR2)) {
                strlcpy(os_name, "Microsoft Windows Server 2003 \"R2\"", os_name_size);
            } else {
                strlcat(os_name, "Microsoft Windows Server 2003", os_name_size);
            }
        }
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
        strlcpy(os_name, "Microsoft Windows XP", os_name_size);
    } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
        strlcpy(os_name, "Microsoft Windows 2000", os_name_size);
    } else if (osvi.dwMajorVersion <= 4) {
        strlcpy(os_name, "Microsoft Windows NT", os_name_size);
    }

    snprintf(szVersion, sizeof(szVersion), ", (%.2u.%.2u.%.4u.%.2u)",
        osvi.dwMajorVersion, osvi.dwMinorVersion, (osvi.dwBuildNumber & 0xFFFF), 0);

    // Test for specific product on Windows NT 4.0 SP6 and later.
    if( bOsVersionInfoEx ) {
        // Test for the workstation type.
        if (osvi.wProductType == VER_NT_WORKSTATION) {
            if ((osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion == 0)) {
                switch(dwType) {
                    case PRODUCT_ULTIMATE:
                        szSKU = "Ultimate ";
                        break;
                    case PRODUCT_HOME_PREMIUM:
                        szSKU = "Home Premium ";
                        break;
                    case PRODUCT_HOME_BASIC:
                        szSKU = "Home Basic ";
                        break;
                    case PRODUCT_ENTERPRISE:
                        szSKU = "Enterprise ";
                        break;
                    case PRODUCT_BUSINESS:
                        szSKU = "Business ";
                        break;
                    case PRODUCT_STARTER:
                        szSKU = "Starter ";
                        break;
                }
            } else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 2)) {
                if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
                    szSKU = "Home ";
                } else {
                    szSKU = "Professional ";
                }
            } else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 1)) {
                if (osvi.wSuiteMask & VER_SUITE_PERSONAL) {
                    szSKU = "Home ";
                } else {
                    szSKU = "Professional ";
                }
            } else if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0)) {
                szSKU = "Professional ";
            } else if ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion == 0)) {
                szSKU = "Workstation ";
            }
        }

        // Test for the server type.
        else if ((osvi.wProductType == VER_NT_SERVER) || (osvi.wProductType == VER_NT_DOMAIN_CONTROLLER)) {
            if ((osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion == 0)) {
                switch(dwType) {
                    case PRODUCT_CLUSTER_SERVER:
                        szSKU = "Cluster Server ";
                        break;
                    case PRODUCT_DATACENTER_SERVER:
                        szSKU = "Datacenter ";
                        break;
                    case PRODUCT_DATACENTER_SERVER_CORE:
                        szSKU = "Datacenter (core installation) ";
                        break;
                    case PRODUCT_ENTERPRISE_SERVER:
                        szSKU = "Enterprise ";
                        break;
                    case PRODUCT_ENTERPRISE_SERVER_CORE:
                        szSKU = "Enterprise (core installation) ";
                        break;
                    case PRODUCT_ENTERPRISE_SERVER_IA64:
                        szSKU = "Enterprise ";
                        break;
                    case PRODUCT_SMALLBUSINESS_SERVER:
                        szSKU = "Small Business Server";
                        break;
                    case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                        szSKU = "Small Business Server Premium ";
                        break;
                    case PRODUCT_STANDARD_SERVER:
                        szSKU = "Standard ";
                        break;
                    case PRODUCT_STANDARD_SERVER_CORE:
                        szSKU = "Standard (core installation) ";
                        break;
                    case PRODUCT_WEB_SERVER:
                        szSKU = "Web Server ";
                        break;
                }
            } else if((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 2)) {
                if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
                    szSKU = "Datacenter Server ";
                } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
                    szSKU = "Enterprise Server ";
                } else if (osvi.wSuiteMask == VER_SUITE_BLADE) {
                    szSKU = "Web Server ";
                } else {
                    szSKU = "Standard Server ";
                }
            } else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
                if (osvi.wSuiteMask & VER_SUITE_DATACENTER) {
                    szSKU, "Datacenter Server ";
                } else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
                    szSKU, "Advanced Server ";
                } else {
                    szSKU, "Standard Server ";
                }
            } else { // Windows NT 4.0
                if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE) {
                    szSKU = "Enterprise Server ";
                } else {
                    szSKU = "Server ";
                }
            }
        }

        switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_INTEL:
                szSKU += "x86";
                break;
            case PROCESSOR_ARCHITECTURE_MIPS:
                szSKU += "MIPS";
                break;
            case PROCESSOR_ARCHITECTURE_ALPHA:
                szSKU += "Alpha";
                break;
            case PROCESSOR_ARCHITECTURE_PPC:
                szSKU += "PowerPC";
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
                szSKU += "Itanium";
                break;
            case PROCESSOR_ARCHITECTURE_ALPHA64:
                szSKU += "Alpha 64-bit";
                break;
            case PROCESSOR_ARCHITECTURE_AMD64:
                szSKU += "x64";
                break;
            case PROCESSOR_ARCHITECTURE_UNKNOWN:
                szSKU += "Unknown";
                break;
        }
        szSKU += " Edition";
    } else { // Test for specific product on Windows NT 4.0 SP5 and earlier
        HKEY hKey;
        char szProductType[80];
        DWORD dwBufLen=sizeof(szProductType);
        LONG lRet;

        lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
            0, KEY_QUERY_VALUE, &hKey );
        if( lRet != ERROR_SUCCESS )
            return FALSE;

        lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
            (LPBYTE) szProductType, &dwBufLen);
        if( (lRet != ERROR_SUCCESS) || (dwBufLen > 80) )
            return FALSE;

        RegCloseKey( hKey );

        if (lstrcmpi("WINNT", szProductType) == 0) {
            szSKU = "Workstation Edition";
        } if (lstrcmpi("LANMANNT", szProductType) == 0) {
            szSKU = "Server Edition";
        } if (lstrcmpi("SERVERNT", szProductType) == 0) {
            szSKU = "Advanced Server Edition";
        }

    }

    // Display service pack (if any) and build number.
    if( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
    {
        HKEY hKey;
        LONG lRet;

        // Test for SP6 versus SP6a.
        lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
            0, KEY_QUERY_VALUE, &hKey );

        if( lRet == ERROR_SUCCESS ) {
            strcpy( szServicePack, ", " );
            strcat( szServicePack, "Service Pack 6a" );
        } else {// Windows NT 4.0 prior to SP6a
            if ( strlen(osvi.szCSDVersion) > 0 ) {
                strcpy( szServicePack, ", " );
                strcat( szServicePack, osvi.szCSDVersion );
            }
        }

        RegCloseKey( hKey );

    } else { // Windows NT 3.51 and earlier or Windows 2000 and later
        if ( strlen(osvi.szCSDVersion) > 0 ) {
            strcpy( szServicePack, ", " );
            strcat( szServicePack, osvi.szCSDVersion );
        }
    }

    snprintf(os_version, os_version_size, "%s%s%s", szSKU.c_str(), szServicePack, szVersion);

    return 0;
}


// Returns the processor make, model, and additional cpu flags supported by
//   the processor, use the Linux CPU processor feature descriptions.
//
int get_processor_info(
    char* p_vendor, int p_vendor_size, char* p_model, int p_model_size,
    char* p_features, int p_features_size
)
{
    char vendorName[256], processorName[256], identifierName[256], capabilities[256], temp_model[256];
    HKEY hKey = NULL;
    LONG retval = 0;
    DWORD nameSize = 0, procSpeed = 0;
    bool gotIdent = false, gotProcName = false, gotMHz = false, gotVendIdent = false;

    strcpy(vendorName, "");
    strcpy(processorName, "");
    strcpy(capabilities, "");
    strcpy(temp_model, "");

    // determine what the cpu's capabilities are
    if (!IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED)) {
        strncat(capabilities, "fpu ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE)) {
        strncat(capabilities, "tsc ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_PAE_ENABLED)) {
        strncat(capabilities, "pae ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_NX_ENABLED)) {
        strncat(capabilities, "nx ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "sse ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "sse2 ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "pni ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "3dnow ", sizeof(capabilities) - strlen(capabilities));
    }
    if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE)) {
        strncat(capabilities, "mmx ", sizeof(capabilities) - strlen(capabilities));
    }
    strip_whitespace(capabilities);


    retval = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
    if(retval == ERROR_SUCCESS) {
        // NT Examples:
        // ProcessorNameString: Intel(R) Xeon(TM) CPU 3.06GHz
        // Identifier: x86 Family 15 Model 2 Stepping 7
        // VendorIdentifier: GenuineIntel
        // ~MHz: 3056

        // Look in various places for processor information, add'l
        // entries suggested by mark mcclure
        nameSize = sizeof(vendorName);
        retval = RegQueryValueEx(hKey, "VendorIdentifier", NULL, NULL, (LPBYTE)vendorName, &nameSize);
        if (retval == ERROR_SUCCESS) gotVendIdent = true;

        nameSize = sizeof(identifierName);
        retval = RegQueryValueEx(hKey, "Identifier", NULL, NULL, (LPBYTE)identifierName, &nameSize);
        if (retval == ERROR_SUCCESS) gotIdent = true;

        nameSize = sizeof(processorName);
        retval = RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)processorName, &nameSize);
        if (retval == ERROR_SUCCESS) gotProcName = true;

        nameSize = sizeof(DWORD);
        retval = RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&procSpeed, &nameSize);
        if (retval == ERROR_SUCCESS) gotMHz = true;
    }

    // populate vendor field.
    if (gotVendIdent) {
        strlcpy( p_vendor, vendorName, p_vendor_size );
    } else {
        strlcpy( p_vendor, "Unknown", p_vendor_size );
    }

    // construct the human readable model name
    if (gotProcName) {
        strlcpy( temp_model, processorName, sizeof(temp_model) );
    } else if (gotIdent && gotMHz) {
        sprintf( temp_model, "%s %dMHz", identifierName, procSpeed );
    } else if (gotVendIdent && gotMHz) {
        sprintf( temp_model, "%s %dMHz", vendorName, procSpeed );
    } else if (gotIdent) {
        strlcpy( temp_model, identifierName, sizeof(temp_model) );
    } else if (gotVendIdent) {
        strlcpy( temp_model, vendorName, sizeof(temp_model) );
    } else {
        strlcpy( temp_model, "Unknown", sizeof(temp_model) );
    }

    // Merge all the seperate pieces of information into one.
    snprintf(p_model, p_model_size, "%s [%s]", temp_model, identifierName);
    p_model[p_model_size-1] = 0;
    strlcpy(p_features, capabilities, p_features_size);

    RegCloseKey(hKey);

    return 0;
}


/// Returns the CPU count
///
/// \param[out] processor_count Reference to an int variable that will receive
///                             the number of processors that are present in the
///                             system.
/// \return Allways returns zero.
int get_processor_count(int& processor_count) {
    SYSTEM_INFO SystemInfo;
    memset( &SystemInfo, NULL, sizeof( SystemInfo ) );
    ::GetSystemInfo( &SystemInfo );

    processor_count = SystemInfo.dwNumberOfProcessors;
    return 0;
}


// Gets host information; called on startup and before each sched RPC
int HOST_INFO::get_host_info() {
    get_timezone(timezone);
    get_filesystem_info(d_total, d_free);
    get_memory_info(m_nbytes, m_swap);
    get_os_information(
        os_name, sizeof(os_name), os_version, sizeof(os_version)
    );
    get_processor_info(
        p_vendor, sizeof(p_vendor),
        p_model, sizeof(p_model),
        p_features, sizeof(p_features)
    );
    get_processor_count(p_ncpus);
    get_local_network_info();
    if (!strlen(host_cpid)) {
        generate_host_cpid();
    }
    return 0;
}

bool HOST_INFO::host_is_running_on_batteries() {
    SYSTEM_POWER_STATUS pStatus;
    ZeroMemory(&pStatus, sizeof(SYSTEM_POWER_STATUS));
    if (!GetSystemPowerStatus(&pStatus)) {
        return false;
    }

    // Sometimes the system reports the ACLineStatus as an
    //   undocumented value, so lets check to see if the
    //   battery is charging or missing and make that part
    //   of the decision.
    bool bIsOnBatteryPower  = (pStatus.ACLineStatus != 1);
    bool bIsBatteryCharging = ((pStatus.BatteryFlag & 8) == 8);
    bool bIsBatteryMissing = ((pStatus.BatteryFlag & 128) == 128);

    return (bIsOnBatteryPower && !bIsBatteryCharging && !bIsBatteryMissing);
}

bool HOST_INFO::users_idle(bool /*check_all_logins*/, double idle_time_to_run) {
    typedef DWORD (CALLBACK* GetFn)();
    static GetFn fn = (GetFn)GetProcAddress(g_hClientLibraryDll, "BOINCGetIdleTickCount");

    if (g_hClientLibraryDll && fn) {
        double seconds_idle = fn() / 1000;
        double seconds_time_to_run = 60 * idle_time_to_run;
        return seconds_idle > seconds_time_to_run;
    }

    return false;
}
