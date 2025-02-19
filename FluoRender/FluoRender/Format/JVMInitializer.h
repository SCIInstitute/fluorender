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

#ifndef _JVMINITIALIZER_H_
#define _JVMINITIALIZER_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <jni.h>
#include <vector>
#include <iostream>
#include <compatibility.h>

#ifdef __linux__
#include <dlfcn.h>
#endif

class JVMInitializer
{
public:
	JVMInitializer(const std::vector<std::string>& args) { m_valid = create_JVM(args); };
	~JVMInitializer() {destroyJVM();};

	bool IsValid() { return m_valid; }

#ifdef _WIN32
	HMODULE m_jvm_dll = nullptr;
#else
	void* m_jvm_dll = nullptr;
#endif
	JavaVM* m_pJvm = nullptr;                      // Pointer to the JVM (Java Virtual Machine)
	JNIEnv* m_pEnv = nullptr;                      // Pointer to native interface
	JavaVMInitArgs m_VMargs;

#ifdef _WIN32
	decltype(&JNI_CreateJavaVM) m_createJVM_Ptr = nullptr;
#else
	typedef jint(JNICALL CreateJavaVM_t)(JavaVM** pvm, void** env, void* args);
	CreateJavaVM_t* m_createJVM_Ptr = nullptr;
#endif

private:
	bool m_valid = false;
	bool m_with_fiji = false;

	bool create_JVM(const std::vector<std::string>& args);
	void destroyJVM();
	inline char getPathSeparator()
	{
#ifdef _WIN32
		return ';';
#else
		return ':';
#endif
	}
};

#endif //_JVMINITIALIZER_H_
