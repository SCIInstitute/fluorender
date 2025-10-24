#include <Debug.h>

#ifdef _WIN32

#ifdef _DEBUG

static unsigned int dbg_counter = 0;

VOID _DBGPRINT(LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ...) \
{
	INT cbFormatString = 0;
	va_list args;
	PWCHAR wszDebugString = NULL;
	size_t st_Offset = 0;

	va_start(args, kwszDebugFormatString);

	cbFormatString = _scwprintf(L"%07d [%s:%d] ", dbg_counter, kwszFunction, iLineNumber) * sizeof(WCHAR);
	cbFormatString += _vscwprintf(kwszDebugFormatString, args) * sizeof(WCHAR) + 2;

	/* Depending on the size of the format string, allocate space on the stack or the heap. */
	wszDebugString = (PWCHAR)_malloca(cbFormatString);

	/* Populate the buffer with the contents of the format string. */
	StringCbPrintfW(wszDebugString, cbFormatString, L"%07d [%s:%d] ", dbg_counter, kwszFunction, iLineNumber);
	StringCbLengthW(wszDebugString, cbFormatString, &st_Offset);
	StringCbVPrintfW(&wszDebugString[st_Offset / sizeof(WCHAR)], cbFormatString - st_Offset, kwszDebugFormatString, args);

	OutputDebugStringW(wszDebugString);

	dbg_counter++;

	_freea(wszDebugString);
	va_end(args);
}

#endif//_DEBUG

#else//_WIN32

void DBGPRINT(const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	vwprintf(format, args);
	va_end(args);
}

#endif//_WIN32