/**
 * IdleTracker - a DLL that tracks the user's idle input time
 *               system-wide.
 *
 * Usage
 * =====
 * - call IdleTrackerInit() when you want to start monitoring.
 * - call IdleTrackerTerm() when you want to stop monitoring.
 * - to get the time past since last user input, do the following:
 *    GetTickCount() - IdleTrackerGetLastTickCount()
 *
 * Author: Sidney Chong
 * Date: 25/5/2000
 * Version: 1.0
 **/

#include "stdafx.h"
#include "boinc_dll.h"
#include "win_util.h"


/**
 * The following global data is only shared in this instance of the DLL
 **/ 
HMODULE   g_hUser32 = NULL;
HANDLE    g_hMemoryMappedData = NULL;
BOOL      g_bIsTerminalServicesEnabled = FALSE;

/**
 * The following global data is SHARED among all instances of the DLL
 * (processes) within a terminal services session.
 **/ 
#pragma data_seg(".IdleTrac")   // you must define as SHARED in .def
HHOOK   g_hHkKeyboard = NULL;   // handle to the keyboard hook
HHOOK   g_hHkMouse = NULL;      // handle to the mouse hook
LONG    g_mouseLocX = -1;       // x-location of mouse position
LONG    g_mouseLocY = -1;       // y-location of mouse position
DWORD   g_dwLastTick = 0;       // tick time of last input event
#pragma data_seg()
#pragma comment(linker, "/section:.IdleTrac,rws")

/**
 * The following global data is SHARED among all instances of the DLL
 * (processes); i.e., these are system-wide globals.
 **/
struct SystemWideIdleData
{
    DWORD dwLastTick;         // tick time of last input event
};

struct SystemWideIdleData* g_pSystemWideIdleData = NULL;

/**
 * Define stuff that only exists on Windows 2000 compatible machines
 **/
typedef struct tagLASTINPUTINFO {
    UINT cbSize;
    DWORD dwTime;
} LASTINPUTINFO, *PLASTINPUTINFO;

typedef BOOL (WINAPI *GETLASTINPUTINFO)(PLASTINPUTINFO);

GETLASTINPUTINFO g_fnGetLastInputInfo = NULL;

/**
 * Keyboard hook: record tick count
 **/
LRESULT CALLBACK KeyboardTracker(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        g_dwLastTick = GetTickCount();
    }
    return ::CallNextHookEx(g_hHkKeyboard, code, wParam, lParam);
}

/**
 * Mouse hook: record tick count
 **/
LRESULT CALLBACK MouseTracker(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        MOUSEHOOKSTRUCT* pStruct = (MOUSEHOOKSTRUCT*)lParam;
        //we will assume that any mouse msg with the same locations as spurious
        if (pStruct->pt.x != g_mouseLocX || pStruct->pt.y != g_mouseLocY) {
            g_mouseLocX = pStruct->pt.x;
            g_mouseLocY = pStruct->pt.y;
            g_dwLastTick = GetTickCount();
        }
    }
    return ::CallNextHookEx(g_hHkMouse, code, wParam, lParam);
}

/**
 * Get tick count of last keyboard or mouse event
 **/
EXTERN_C __declspec(dllexport) DWORD BOINCGetIdleTickCount() {
    DWORD dwCurrentTickCount = GetTickCount();
    DWORD dwLastTickCount = 0;

    if (g_pSystemWideIdleData) {
        LASTINPUTINFO lii;
        ZeroMemory(&lii, sizeof(lii));
        lii.cbSize = sizeof(lii);
        g_fnGetLastInputInfo(&lii);

        /**
         * If both values are greater than the system tick count then
         *   the system must have looped back to the beginning.
         **/
        if ((dwCurrentTickCount < lii.dwTime) &&
             (dwCurrentTickCount < g_pSystemWideIdleData->dwLastTick)) {
            lii.dwTime = dwCurrentTickCount;
            g_pSystemWideIdleData->dwLastTick = dwCurrentTickCount;
        }

        if (lii.dwTime > g_pSystemWideIdleData->dwLastTick) {
            g_pSystemWideIdleData->dwLastTick = lii.dwTime;
        }

        dwLastTickCount = g_pSystemWideIdleData->dwLastTick;
    }
    return (dwCurrentTickCount - dwLastTickCount);
}

/**
 * Initialize DLL: install kbd/mouse hooks.
 **/
BOOL IdleTrackerStartup() {
    BOOL                bExists = FALSE;
    BOOL                bResult = FALSE;
    SECURITY_ATTRIBUTES sec_attr;
    SECURITY_DESCRIPTOR sd;

    g_bIsTerminalServicesEnabled = IsTerminalServicesEnabled();
        
    g_hUser32 = LoadLibrary("user32.dll");            
    if (g_hUser32) {
        g_fnGetLastInputInfo = (GETLASTINPUTINFO)GetProcAddress(g_hUser32, "GetLastInputInfo");
    }


    /*
    * Create a security descriptor that will allow
    * everyone full access.
    */
    InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
    SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );

    sec_attr.nLength = sizeof(sec_attr);
    sec_attr.bInheritHandle = TRUE;
    sec_attr.lpSecurityDescriptor = &sd;

    /*
    * Create a filemap object that is global for everyone,
    * including users logged in via terminal services.
    */
    g_hMemoryMappedData = 
        CreateFileMapping(
            INVALID_HANDLE_VALUE,
            &sec_attr,
            PAGE_READWRITE,
            0,
            4096,
            "Global\\IdleMonitor"
        );

    if (NULL != g_hMemoryMappedData ) {
        if (ERROR_ALREADY_EXISTS == GetLastError()) {
            bExists = TRUE;
        }

        g_pSystemWideIdleData = (struct SystemWideIdleData*) 
            MapViewOfFile(
                g_hMemoryMappedData, 
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                0
            );

        _ASSERT(g_pSystemWideIdleData);
    }

    if (!bExists && g_pSystemWideIdleData) {
        g_pSystemWideIdleData->dwLastTick = GetTickCount();
    }


    if (!g_hUser32 || !g_fnGetLastInputInfo || !g_hMemoryMappedData || !g_pSystemWideIdleData) {
        bResult = FALSE;
    } else {
        bResult = TRUE;
    }

    return bResult;
}

/**
 * Terminate DLL: remove hooks.
 **/
void IdleTrackerShutdown() {
    if (NULL != g_pSystemWideIdleData) {
        UnmapViewOfFile(g_pSystemWideIdleData);
        CloseHandle(g_hMemoryMappedData);
    }

    if (NULL != g_hUser32) {
        FreeLibrary(g_hUser32);
    }
}
