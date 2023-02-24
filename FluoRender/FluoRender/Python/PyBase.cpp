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
#include <sys/stat.h>
#include <sys/types.h>

using namespace flrd;

bool PyBase::m_valid = false;
#ifdef _WIN32
HMODULE PyBase::python_dll = nullptr;
decltype(&Py_Initialize) PyBase::Initialize = nullptr;
decltype(&PyRun_SimpleString) PyBase::Run_SimpleString = nullptr;
decltype(&Py_FinalizeEx) PyBase::FinalizeEx = nullptr;
#else
void* PyBase::python_dll = nullptr;
PyBase::Initialize_tp* PyBase::Initialize = nullptr;
PyBase::PyRun_SimpleString_tp* PyBase::Run_SimpleString = nullptr;
PyBase::Py_FinalizeEx_tp* PyBase::FinalizeEx = nullptr;
#endif

PyBase::PyBase() :
	m_state(1),
	m_interval(std::chrono::milliseconds(100))
{
	m_valid = true;
#ifdef _WIN32
	python_dll = LoadLibrary(L"python310.dll");
	if (!SetValid(python_dll)) return;

	Initialize = (decltype(&Py_Initialize))GetProcAddress(python_dll, "Py_Initialize");
	if (!SetValid(Initialize)) return;

	Run_SimpleString = (decltype(&PyRun_SimpleString))GetProcAddress(python_dll, "PyRun_SimpleString");
	if (!SetValid(Run_SimpleString)) return;

	FinalizeEx = (decltype(&Py_FinalizeEx))GetProcAddress(python_dll, "Py_FinalizeEx");
	if (!SetValid(FinalizeEx)) return;
#else
	//python_dll = dlopen("Python", RTLD_NOW);
	//if (!SetValid(python_dll)) return;

	//Initialize = (Initialize_tp*)dlsym(python_dll, "Py_Initialize");
	//if (!SetValid((void*)Initialize)) return;
	Initialize = Py_Initialize;

	//Run_SimpleString = (PyRun_SimpleString_tp*)dlsym(python_dll, "PyRun_SimpleString");
	//if (!SetValid((void*)Run_SimpleString)) return;
	Run_SimpleString = PyRun_SimpleString;

	//FinalizeEx = (Py_FinalizeEx_tp*)dlsym(python_dll, "Py_FinalizeEx");
	//if (!SetValid((void*)FinalizeEx)) return;
	FinalizeEx = Py_FinalizeEx;
#endif
}

PyBase::~PyBase()
{
	m_thread.get();
}

bool PyBase::Init()
{
	if (!m_valid)
		return false;

	m_state = 1;
	//configure the thread
	m_thread = std::async(std::launch::async, &PyBase::ThreadFunc, this);

	return true;
}

void PyBase::Run(OpType func, const std::string& par)
{
	m_queue.push(std::pair<OpType, std::string>(func, par));
}

void PyBase::ThreadFunc()
{
	Initialize();

	bool run = true;
	while (run)
	{
		//set an interval
		std::this_thread::sleep_for(m_interval);

		auto m = m_queue.pop();

		m_state = 2;
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

void PyBase::Exit()
{
	Run(ot_Quit);
}
