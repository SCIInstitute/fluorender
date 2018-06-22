/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <jni.h>
#include "iostream"

#ifndef _JVMINITIALIZER_H_
#define _JVMINITIALIZER_H_


class JVMInitializer {
	public:
		static JVMInitializer* getInstance();
		static void destroyJVM();
		static JavaVM *m_pJvm;                      // Pointer to the JVM (Java Virtual Machine)
		static JNIEnv *m_pEnv;                      // Pointer to native interface
		static JavaVMInitArgs m_VMargs;

	private:
		static JVMInitializer* m_pJVMInstance;		

		JVMInitializer() {};
		~JVMInitializer() {};
		JVMInitializer(JVMInitializer const&);
		JVMInitializer& operator=(JVMInitializer const&);

		bool static create_JVM();
};

#endif //_JVMINITIALIZER_H_