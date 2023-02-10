/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#ifndef PYBASE_H
#define PYBASE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <compatibility.h>
#ifdef __linux__
#include <dlfcn.h>
#endif

namespace flrd
{
	class PyBase
	{
	public:
		PyBase();
		~PyBase();

		bool Run();

	private:
		bool m_valid;
#ifdef _WIN32
		static HMODULE python_dll;//lib
		//functions
		static decltype(&Py_SetProgramName) SetProgramName;
		static decltype(&Py_Initialize) Initialize;
		static decltype(&PyDict_New) Dict_New;
		static decltype(&PyRun_SimpleString) Run_SimpleString;
		static decltype(&PyRun_String) Run_String;
		static decltype(&PyObject_Repr) Object_Repr;
		static decltype(&Py_FinalizeEx) FinalizeEx;
#else
		static void* python_dll;
		//functions
#endif
	private:
		bool SetValid(void* val)
		{
			m_valid = val != nullptr;
			return m_valid;
		}
	};
}

#endif//PYBASE_H
