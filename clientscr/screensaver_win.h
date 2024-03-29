// 
// Contributor(s):
//     DirectX 8.1 Screen Saver Framework from Microsoft.
//

#ifndef _SCREENSAVER_WIN_H
#define _SCREENSAVER_WIN_H


#include <multimon.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MAX_DISPLAYS                              9
#define NO_ADAPTER                                0xffffffff
#define NO_MONITOR                                0xffffffff

#define BSF_ALLOWSFW                              0x00000080

#define BOINC_WINDOW_CLASS_NAME                   _T("BOINC_app")


//***************************************************************************************
// Modes of operation for screensaver
enum SaverMode
{
    sm_config,         // Config dialog box
    sm_preview,        // Mini preview window in Display Properties dialog
    sm_full,           // Full-on screensaver mode
    sm_test,           // Test mode
    sm_passwordchange  // Change password
};


//-----------------------------------------------------------------------------
// Name: struct INTERNALMONITORINFO
// Desc: Structure for holding information about a monitor
//-----------------------------------------------------------------------------
struct INTERNALMONITORINFO
{
    TCHAR          strDeviceName[128];
    TCHAR          strMonitorName[128];
    HMONITOR       hMonitor;
    RECT           rcScreen;
    HWND           hWnd;

    DWORD          dwTimeLastUpdate;

    // Error message state
    FLOAT          xError;
    FLOAT          yError;
    FLOAT          widthError;
    FLOAT          heightError;
    FLOAT          xVelError;
    FLOAT          yVelError;
};


//-----------------------------------------------------------------------------
// Name: struct INTERNALMONITORINFO
// Desc: Use the following structure rather than DISPLAY_DEVICE, since some
//       old versions of DISPLAY_DEVICE are missing the last two fields and
//       this can cause problems with EnumDisplayDevices on Windows 2000.
//-----------------------------------------------------------------------------
struct DISPLAY_DEVICE_FULL
{
    DWORD  cb;
    TCHAR  DeviceName[32];
    TCHAR  DeviceString[128];
    DWORD  StateFlags;
    TCHAR  DeviceID[128];
    TCHAR  DeviceKey[128];
};

//-----------------------------------------------------------------------------
// Name: class CScreensaver
// Desc: Screensaver class
//-----------------------------------------------------------------------------
class CScreensaver
{
public:
    CScreensaver();

    virtual HRESULT Create( HINSTANCE hInstance );
    virtual INT     Run();
    HRESULT         DisplayErrorMsg( HRESULT hr );


    //
    // Infrastructure layer 
    //
protected:
    SaverMode       ParseCommandLine( TCHAR* pstrCommandLine );
    VOID            EnumMonitors( VOID );

    int             UtilGetRegKey(LPCTSTR name, DWORD &keyval);
    int             UtilSetRegKey(LPCTSTR name, DWORD value);
    int             UtilGetRegStartupStr(LPCTSTR name, LPTSTR str);

    BOOL            CreateInfrastructureMutexes();

    BOOL            GetError( BOOL& bErrorMode, HRESULT& hrError, TCHAR* pszError, size_t iErrorSize );
    BOOL            SetError( BOOL bErrorMode, HRESULT hrError );
    VOID            UpdateErrorBoxText();
    virtual BOOL    GetTextForError( HRESULT hr, TCHAR* pszError, DWORD dwNumChars );


    // Variables for non-fatal error management
    HANDLE          m_hErrorManagementMutex;
    BOOL            m_bErrorMode;        // Whether to display an error
    HRESULT         m_hrError;           // Error code to display
    TCHAR           m_szError[400];      // Error message text

    DWORD           m_dwBlankScreen;
    DWORD           m_dwBlankTime;


    //
    // Data management layer
    //
protected:
    BOOL            CreateDataManagementThread();
    BOOL            DestroyDataManagementThread();

    DWORD WINAPI    DataManagementProc();
    static DWORD WINAPI DataManagementProcStub( LPVOID lpParam );
    void            CheckForegroundWindow();
    int             terminate_screensaver(HANDLE& graphics_application, RESULT *worker_app);
    int             launch_screensaver(RESULT* rp, HANDLE& graphics_application);
    void            HandleRPCError(void);

// Determine if two RESULT pointers refer to the same task
    bool            is_same_task(RESULT* taska, RESULT* taskb);

// Count the number of active graphics-capable apps
    int             count_active_graphic_apps(RESULTS& results, RESULT* exclude = NULL);

// Choose a ramdom graphics application from the vector that
//   was passed in.

    RESULT*         get_random_graphics_app(RESULTS& results, RESULT* exclude = NULL);

    RPC_CLIENT*     rpc;
    CC_STATE        state;
    RESULTS         results;
    RESULT          m_running_result;
    bool            m_updating_results;

    HANDLE          m_hDataManagementThread;
    HANDLE          m_hGraphicsApplication;
    BOOL            m_bResetCoreState;
    bool            m_QuitDataManagementProc;
    bool            m_bV5_GFX_app_is_running;
    int             m_iLastResultShown;
    time_t          m_tLastResultChangeTime;
    time_t          m_tThreadCreateTime;

    //
    // Presentation layer
    //
protected:
    HRESULT         CreateSaverWindow();
    VOID            UpdateErrorBox();
    VOID            InterruptSaver();
    VOID            ShutdownSaver();
    VOID            ChangePassword();

    VOID            DoConfig();
    HRESULT         DoSaver();
    VOID            DoPaint( HWND hwnd, HDC hdc, LPPAINTSTRUCT lpps );

    LRESULT         SaverProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR         ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    static LRESULT CALLBACK SaverProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK ConfigureDialogProcStub( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    SaverMode               m_SaverMode;         // sm_config, sm_full, sm_preview, etc.
    BOOL                    m_bAllScreensSame;   // If TRUE, show same image on all screens
    HWND                    m_hWnd;              // Focus window and device window on primary
    HWND                    m_hWndParent;
    HINSTANCE               m_hInstance;
    BOOL                    m_bWaitForInputIdle;  // Used to pause when preview starts
    DWORD                   m_dwSaverMouseMoveCount;
    BOOL                    m_bWindowed;

    INTERNALMONITORINFO     m_Monitors[MAX_DISPLAYS];
    DWORD                   m_dwNumMonitors;
    RECT                    m_rcRenderTotal;     // Rect of entire area to be rendered
    RECT                    m_rcRenderCurDevice; // Rect of render area of current device
    BOOL                    m_bPaintingInitialized;

    TCHAR                   m_strWindowTitle[200]; // Title for the app's window

    DWORD                   m_dwLastInputTimeAtStartup;
};

#endif
