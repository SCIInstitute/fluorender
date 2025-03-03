﻿/*----------------------------------------------------------------------------

	NAME
		Utils.c

	PURPOSE
		Some general-purpose functions for the WinTab demos.

	COPYRIGHT
		Copyright (c) Wacom Company, Ltd. 2014 All Rights Reserved
		All rights reserved.

		The text and information contained in this file may be freely used,
		copied, or distributed without compensation or licensing restrictions.

---------------------------------------------------------------------------- */
#include "WacomUtils.h"

//////////////////////////////////////////////////////////////////////////////
HINSTANCE ghWintab = NULL;

WTINFOA gpWTInfoA = NULL;
WTOPENA gpWTOpenA = NULL;
WTGETA gpWTGetA = NULL;
WTSETA gpWTSetA = NULL;
WTCLOSE gpWTClose = NULL;
WTPACKET gpWTPacket = NULL;
WTENABLE gpWTEnable = NULL;
WTOVERLAP gpWTOverlap = NULL;
WTSAVE gpWTSave = NULL;
WTCONFIG gpWTConfig = NULL;
WTRESTORE gpWTRestore = NULL;
WTEXTSET gpWTExtSet = NULL;
WTEXTGET gpWTExtGet = NULL;
WTQUEUESIZESET gpWTQueueSizeSet = NULL;
WTDATAPEEK gpWTDataPeek = NULL;
WTPACKETSGET gpWTPacketsGet = NULL;
WTMGROPEN gpWTMgrOpen = NULL;
WTMGRCLOSE gpWTMgrClose = NULL;
WTMGRDEFCONTEXT gpWTMgrDefContext = NULL;
WTMGRDEFCONTEXTEX gpWTMgrDefContextEx = NULL;

// TODO - add more wintab32 function pointers as needed
char gpszProgramName[] = "FluoRender Wacom Support";
char* pszProgramName = NULL;

#define GETPROCADDRESS(type, func) \
	gp##func = (type)GetProcAddress(ghWintab, #func); \
	if (!gp##func){ WACOM_ASSERT(FALSE); UnloadWintab(); return FALSE; }

//////////////////////////////////////////////////////////////////////////////
// Purpose
//		Find wintab32.dll and load it.  
//		Find the exported functions we need from it.
//
//	Returns
//		TRUE on success.
//		FALSE on failure.
//
BOOL LoadWintab( void )
{
//	ghWintab = LoadLibraryA(  "C:\\dev\\mainline\\Wacom\\Win\\Win32\\Debug\\Wacom_Tablet.dll" );
//	ghWintab = LoadLibraryA(  "C:\\dev\\mainline\\Wacom\\Win\\Win32\\Debug\\Wintab32.dll" );	
	ghWintab = LoadLibraryA( "Wintab32.dll" );
	
	if ( !ghWintab )
	{
		DWORD err = GetLastError();
		WACOM_TRACE("LoadLibrary error: %i\n", err);
		//ShowError("Could not load Wintab32.dll");
		return FALSE;
	}

	// Explicitly find the exported Wintab functions in which we are interested.
	// We are using the ASCII, not unicode versions (where applicable).
	GETPROCADDRESS( WTOPENA, WTOpenA );
	GETPROCADDRESS( WTINFOA, WTInfoA );
	GETPROCADDRESS( WTGETA, WTGetA );
	GETPROCADDRESS( WTSETA, WTSetA );
	GETPROCADDRESS( WTPACKET, WTPacket );
	GETPROCADDRESS( WTCLOSE, WTClose );
	GETPROCADDRESS( WTENABLE, WTEnable );
	GETPROCADDRESS( WTOVERLAP, WTOverlap );
	GETPROCADDRESS( WTSAVE, WTSave );
	GETPROCADDRESS( WTCONFIG, WTConfig );
	GETPROCADDRESS( WTRESTORE, WTRestore );
	GETPROCADDRESS( WTEXTSET, WTExtSet );
	GETPROCADDRESS( WTEXTGET, WTExtGet );
	GETPROCADDRESS( WTQUEUESIZESET, WTQueueSizeSet );
	GETPROCADDRESS( WTDATAPEEK, WTDataPeek );
	GETPROCADDRESS( WTPACKETSGET, WTPacketsGet );
	GETPROCADDRESS( WTMGROPEN, WTMgrOpen );
	GETPROCADDRESS( WTMGRCLOSE, WTMgrClose );
	GETPROCADDRESS( WTMGRDEFCONTEXT, WTMgrDefContext );
	GETPROCADDRESS( WTMGRDEFCONTEXTEX, WTMgrDefContextEx );


	// TODO - don't forget to NULL out pointers in UnloadWintab().
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////////
// Purpose
//		Uninitializes use of wintab32.dll
//
// Returns
//		Nothing.
//
void UnloadWintab( void )
{
	WACOM_TRACE( "UnloadWintab()\n" );

	if ( ghWintab )
	{
		FreeLibrary( ghWintab );
		ghWintab = NULL;
	}

	gpWTOpenA			= NULL;
	gpWTClose			= NULL;
	gpWTInfoA			= NULL;
	gpWTPacket			= NULL;
	gpWTEnable			= NULL;
	gpWTOverlap			= NULL;
	gpWTSave				= NULL;
	gpWTConfig			= NULL;
	gpWTGetA				= NULL;
	gpWTSetA				= NULL;
	gpWTRestore			= NULL;
	gpWTExtSet			= NULL;
	gpWTExtGet			= NULL;
	gpWTQueueSizeSet	= NULL;
	gpWTDataPeek		= NULL;
	gpWTPacketsGet		= NULL;
	gpWTMgrOpen			= NULL;
	gpWTMgrClose		= NULL;
	gpWTMgrDefContext = NULL;
	gpWTMgrDefContextEx = NULL;
}



//////////////////////////////////////////////////////////////////////////////
// Purpose
//		Display error to user.
//
void ShowError( char *pszErrorMessage )
{
	WACOM_TRACE( "ShowError()\n" );

	WACOM_ASSERT( pszErrorMessage );

	MessageBoxA( NULL, pszErrorMessage, gpszProgramName, MB_OK | MB_ICONHAND );
}



#ifdef WACOM_DEBUG

//////////////////////////////////////////////////////////////////////////////

void WacomTrace( const char *lpszFormat, ...)
{
	char szTraceMessage[ 128 ];

	int nBytesWritten;

	va_list args;

	WACOM_ASSERT( lpszFormat );

	va_start( args, lpszFormat );

	nBytesWritten = _vsnprintf( szTraceMessage, sizeof( szTraceMessage ) - 1, 
		lpszFormat, args );

	if ( nBytesWritten > 0 )
	{
		OutputDebugStringA( szTraceMessage );
	}

	va_end( args );
}

#endif // WACOM_DEBUG
