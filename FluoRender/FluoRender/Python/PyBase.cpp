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
#include "PyBase.h"
#include <Debug.h>

using namespace flrd;

bool PyBase::m_valid = false;
std::atomic<int> PyBase::m_state = 0;
std::chrono::milliseconds PyBase::m_interval = std::chrono::milliseconds(100);
PyQueue <std::pair<PyBase::OpType, std::string>> PyBase::m_queue;

#ifdef _WIN32
HMODULE PyBase::python_dll = nullptr;
decltype(&Py_SetProgramName) PyBase::SetProgramName = nullptr;
decltype(&Py_Initialize) PyBase::Initialize = nullptr;
decltype(&PyDict_New) PyBase::Dict_New = nullptr;
decltype(&PyRun_SimpleString) PyBase::Run_SimpleString = nullptr;
decltype(&PyRun_String) PyBase::Run_String = nullptr;
decltype(&PyObject_Repr) PyBase::Object_Repr = nullptr;
decltype(&Py_FinalizeEx) PyBase::FinalizeEx = nullptr;
#else
void* PyBase::python_dll = nullptr;
#endif

PyBase::PyBase()
{
	m_valid = true;
#ifdef _WIN32
	python_dll = LoadLibrary(L"python310.dll");
	if (!SetValid(python_dll)) return;

	SetProgramName = (decltype(&Py_SetProgramName))GetProcAddress(python_dll, "Py_SetProgramName");
	if (!SetValid(SetProgramName)) return;

	Initialize = (decltype(&Py_Initialize))GetProcAddress(python_dll, "Py_Initialize");
	if (!SetValid(Initialize)) return;

	Dict_New = (decltype(&PyDict_New))GetProcAddress(python_dll, "PyDict_New");
	if (!SetValid(Dict_New)) return;

	Run_SimpleString = (decltype(&PyRun_SimpleString))GetProcAddress(python_dll, "PyRun_SimpleString");
	if (!SetValid(Run_SimpleString)) return;

	Run_String = (decltype(&PyRun_String))GetProcAddress(python_dll, "PyRun_String");
	if (!SetValid(Run_String)) return;

	Object_Repr = (decltype(&PyObject_Repr))GetProcAddress(python_dll, "PyObject_Repr");
	if (!SetValid(Object_Repr)) return;

	FinalizeEx = (decltype(&Py_FinalizeEx))GetProcAddress(python_dll, "Py_FinalizeEx");
	if (!SetValid(FinalizeEx)) return;

#else
	python_dll = dlopen("python3.so", RTLD_NOW);
#endif
}

PyBase::~PyBase()
{
}

bool PyBase::Init()
{
	if (!m_valid)
		return false;

	//configure the thread
	m_thread = std::thread(&PyBase::ThreadFunc, this);
	//let it run
	m_thread.detach();

	return true;
}

void PyBase::Run(OpType func, const std::string& par)
{
	m_queue.push(std::pair<OpType, std::string>(func, par));
}

void PyBase::ThreadFunc()
{
	Initialize();

	m_state = 0;
	bool run = true;
	while (run)
	{
		//set an interval
		std::this_thread::sleep_for(m_interval);

		auto m = m_queue.pop();

		m_state = 1;
		switch (m.first)
		{
		case ot_Initialize:
			//should already be done
			break;
		case ot_Run_SimpleString:
		{
			std::string str = m.second + "\n";
			Run_SimpleString(str.c_str());
		}
		break;
		case ot_Quit:
			run = false;
			break;
		}
		m_state = 0;
	}

	FinalizeEx();
}