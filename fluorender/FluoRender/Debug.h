#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef _WIN32

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <stdio.h>
#include <strsafe.h>
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#define DBGPRINT(kwszDebugFormatString, ...) _DBGPRINT(__FUNCTIONW__, __LINE__, kwszDebugFormatString, __VA_ARGS__)

VOID _DBGPRINT(LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ...) \
{
	INT cbFormatString = 0;
	va_list args;
	PWCHAR wszDebugString = NULL;
	size_t st_Offset = 0;

	va_start(args, kwszDebugFormatString);

	cbFormatString = _scwprintf(L"[%s:%d] ", kwszFunction, iLineNumber) * sizeof(WCHAR);
	cbFormatString += _vscwprintf(kwszDebugFormatString, args) * sizeof(WCHAR) + 2;

	/* Depending on the size of the format string, allocate space on the stack or the heap. */
	wszDebugString = (PWCHAR)_malloca(cbFormatString);

	/* Populate the buffer with the contents of the format string. */
	StringCbPrintfW(wszDebugString, cbFormatString, L"[%s:%d] ", kwszFunction, iLineNumber);
	StringCbLengthW(wszDebugString, cbFormatString, &st_Offset);
	StringCbVPrintfW(&wszDebugString[st_Offset / sizeof(WCHAR)], cbFormatString - st_Offset, kwszDebugFormatString, args);

	OutputDebugStringW(wszDebugString);

	_freea(wszDebugString);
	va_end(args);
}

#else
#define DBG_NEW new
#define DBGPRINT( kwszDebugFormatString, ... )
#endif

#endif//_WIN32

#endif//_DEBUG_H_