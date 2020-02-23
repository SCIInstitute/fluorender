#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <cstdlib>
#include <stdio.h>

#ifdef _WIN32

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <strsafe.h>
#include <Windows.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#define DBGPRINT(kwszDebugFormatString, ...) _DBGPRINT(__FUNCTIONW__, __LINE__, kwszDebugFormatString, __VA_ARGS__)

VOID _DBGPRINT(LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ...);

#else//_DEBUG
#define DBG_NEW new
#define DBGPRINT( kwszDebugFormatString, ... )
#endif//_DEBUG

#else//_WIN32

#include <cstdarg>
void DBGPRINT(...);
//{}

#endif//_WIN32

#endif//_DEBUG_H_
