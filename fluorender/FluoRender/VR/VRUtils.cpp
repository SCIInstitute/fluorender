/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include "VRUtils.h"

#ifdef _WIN32
HMODULE openvr_api = 0;
#endif

funcVR_Init mVR_Init = 0;
funcVR_Shutdown mVR_Shutdown = 0;
funcVRCompositor mVRCompositor = 0;

bool LoadVR()
{
#ifdef _WIN32
	openvr_api = LoadLibrary(L"openvr_api");
	if (!openvr_api)
		return false;
	mVR_Init = reinterpret_cast<funcVR_Init>(
		GetProcAddress(openvr_api, "VR_Init"));
	if (!mVR_Init)
		return false;
	mVR_Shutdown = reinterpret_cast<funcVR_Shutdown>(
		GetProcAddress(openvr_api, "VR_Shutdown"));
	if (!mVR_Shutdown)
		return false;
	mVRCompositor = reinterpret_cast<funcVRCompositor>(
		GetProcAddress(openvr_api, "VRCompositor"));
	if (!mVRCompositor)
		return false;
	return true;
#else
	return false;
#endif
}

void UnloadVR()
{
#ifdef _WIN32
	if (openvr_api)
	{
		FreeLibrary(openvr_api);
		openvr_api = 0;
	}
#endif
	mVR_Init = 0;
	mVR_Shutdown = 0;
	mVRCompositor = 0;
}