// $Id: stackwalker_win.h 12364 2007-04-13 06:13:46Z rwalton $
//
/*////////////////////////////////////////////////////////////////////////////
 *  Project:
 *    Memory_and_Exception_Trace
 *
 * ///////////////////////////////////////////////////////////////////////////
 *  File:
 *    Stackwalker.h
 *
 *  Remarks:
 *
 *
 *  Note:
 *
 *
 *  Author:
 *    Jochen Kalmbach
 *
 *////////////////////////////////////////////////////////////////////////////

#ifndef __STACKWALKER_H__
#define __STACKWALKER_H__

// Make extern "C", so it will also work with normal C-Programs
#ifdef __cplusplus
extern "C" {
#endif

int DebuggerInitialize( LPCSTR pszBOINCLocation, LPCSTR pszSymbolStore, BOOL bProxyEnabled, LPCSTR pszProxyServer );
int DebuggerDisplayDiagnostics();
DWORD StackwalkFilter( EXCEPTION_POINTERS* ep, DWORD status );
void StackwalkThread( HANDLE hThread, CONTEXT* c );

#ifdef __cplusplus
}
#endif

#endif  // __STACKWALKER_H__
