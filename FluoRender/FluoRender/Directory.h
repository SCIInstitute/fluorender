/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

#include <iostream>
#include <string>
#include <filesystem>
#include <cwchar>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

std::wstring getExecutablePath() {
	wchar_t buffer[1024];
#if defined(_WIN32)
	GetModuleFileNameW(NULL, buffer, sizeof(buffer) / sizeof(wchar_t));
#elif defined(__linux__)
	char tempBuffer[1024];
	readlink("/proc/self/exe", tempBuffer, sizeof(tempBuffer));
	std::mbstate_t state = std::mbstate_t();
	const char* src = tempBuffer;
	std::size_t len = std::mbsrtowcs(buffer, &src, sizeof(buffer) / sizeof(wchar_t), &state);
#elif defined(__APPLE__)
	char tempBuffer[1024];
	uint32_t size = sizeof(tempBuffer);
	_NSGetExecutablePath(tempBuffer, &size);
	std::mbstate_t state = std::mbstate_t();
	const char* src = tempBuffer;
	std::size_t len = std::mbsrtowcs(buffer, &src, sizeof(buffer) / sizeof(wchar_t), &state);
#endif
	std::wstring wstr(buffer);
	std::filesystem::path p(wstr);
	p = p.parent_path();
	wstr = p.wstring();
	return wstr;
}

#endif//_DIRECTORY_H_