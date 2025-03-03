﻿/*----------------------------------------------------------------------------

	NAME
		Utils.h

	PURPOSE
		Defines for the general-purpose functions for the WinTab demos.

	COPYRIGHT
		Copyright (c) Wacom Company, Ltd. 2014 All Rights Reserved
		All rights reserved.

		The text and information contained in this file may be freely used,
		copied, or distributed without compensation or licensing restrictions.

---------------------------------------------------------------------------- */
#pragma once

#define WINDOWS_LEAN_AND_MEAN
#include	<windows.h>
#include	<stdio.h>
#include	<assert.h>
#include	<stdarg.h>
#include	"wintab.h"		// NOTE: get from wactab header package

//////////////////////////////////////////////////////////////////////////////
#define WACOM_DEBUG

// Ignore warnings about using unsafe string functions.
#pragma warning( disable : 4996 )

//////////////////////////////////////////////////////////////////////////////
// Function pointers to Wintab functions exported from wintab32.dll. 
typedef UINT ( API * WTINFOA ) ( UINT, UINT, LPVOID );
typedef HCTX ( API * WTOPENA )( HWND, LPLOGCONTEXTA, BOOL );
typedef BOOL ( API * WTGETA ) ( HCTX, LPLOGCONTEXT );
typedef BOOL ( API * WTSETA ) ( HCTX, LPLOGCONTEXT );
typedef BOOL ( API * WTCLOSE ) ( HCTX );
typedef BOOL ( API * WTENABLE ) ( HCTX, BOOL );
typedef BOOL ( API * WTPACKET ) ( HCTX, UINT, LPVOID );
typedef BOOL ( API * WTOVERLAP ) ( HCTX, BOOL );
typedef BOOL ( API * WTSAVE ) ( HCTX, LPVOID );
typedef BOOL ( API * WTCONFIG ) ( HCTX, HWND );
typedef HCTX ( API * WTRESTORE ) ( HWND, LPVOID, BOOL );
typedef BOOL ( API * WTEXTSET ) ( HCTX, UINT, LPVOID );
typedef BOOL ( API * WTEXTGET ) ( HCTX, UINT, LPVOID );
typedef BOOL ( API * WTQUEUESIZESET ) ( HCTX, int );
typedef int  ( API * WTDATAPEEK ) ( HCTX, UINT, UINT, int, LPVOID, LPINT);
typedef int  ( API * WTPACKETSGET ) (HCTX, int, LPVOID);
typedef HMGR ( API * WTMGROPEN ) ( HWND, UINT );
typedef BOOL ( API * WTMGRCLOSE ) ( HMGR );
typedef HCTX ( API * WTMGRDEFCONTEXT ) ( HMGR, BOOL );
typedef HCTX ( API * WTMGRDEFCONTEXTEX ) ( HMGR, UINT, BOOL );

// TODO - add more wintab32 function defs as needed

//////////////////////////////////////////////////////////////////////////////
// Loaded Wintab32 API functions.
extern HINSTANCE ghWintab;

extern WTINFOA gpWTInfoA;
extern WTOPENA gpWTOpenA;
extern WTGETA gpWTGetA;
extern WTSETA gpWTSetA;
extern WTCLOSE gpWTClose;
extern WTPACKET gpWTPacket;
extern WTENABLE gpWTEnable;
extern WTOVERLAP gpWTOverlap;
extern WTSAVE gpWTSave;
extern WTCONFIG gpWTConfig;
extern WTRESTORE gpWTRestore;
extern WTEXTSET gpWTExtSet;
extern WTEXTGET gpWTExtGet;
extern WTQUEUESIZESET gpWTQueueSizeSet;
extern WTDATAPEEK gpWTDataPeek;
extern WTPACKETSGET gpWTPacketsGet;
extern WTMGROPEN gpWTMgrOpen;
extern WTMGRCLOSE gpWTMgrClose;
extern WTMGRDEFCONTEXT gpWTMgrDefContext;
extern WTMGRDEFCONTEXTEX gpWTMgrDefContextEx;

// TODO - add more wintab32 function pointers as needed

//////////////////////////////////////////////////////////////////////////////
BOOL LoadWintab( void );
void UnloadWintab( void );

void ShowError( char *pszErrorMessage );

//////////////////////////////////////////////////////////////////////////////
#ifdef WACOM_DEBUG

void WacomTrace( const char *lpszFormat, ...);

#define WACOM_ASSERT( x ) assert( x )
#define WACOM_TRACE(...)  WacomTrace(__VA_ARGS__)
#else
#define WACOM_TRACE(...)
#define WACOM_ASSERT( x )

#endif // WACOM_DEBUG

