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
int PyBase::m_high_ver = 10;
#ifdef _WIN32
HMODULE PyBase::python_dll = nullptr;
decltype(&Py_Initialize) PyBase::Initialize = nullptr;
decltype(&PyRun_SimpleString) PyBase::Run_SimpleString = nullptr;
decltype(&Py_FinalizeEx) PyBase::FinalizeEx = nullptr;
decltype(&PyImport_AddModule) PyBase::Import_AddModule = nullptr;
decltype(&PyObject_GetAttrString) PyBase::Object_GetAttrString = nullptr;
decltype(&PyUnicode_AsEncodedString) PyBase::Unicode_AsEncodedString = nullptr;
decltype(&PyBytes_AsString) PyBase::Bytes_AsString = nullptr;
#else
void* PyBase::python_dll = nullptr;
PyBase::Initialize_tp* PyBase::Initialize = nullptr;
PyBase::PyRun_SimpleString_tp* PyBase::Run_SimpleString = nullptr;
PyBase::Py_FinalizeEx_tp* PyBase::FinalizeEx = nullptr;
PyBase::PyImport_AddModule_tp* PyBase::Import_AddModule = nullptr;
PyBase::PyObject_GetAttrString_tp* PyBase::Object_GetAttrString = nullptr;
PyBase::PyUnicode_AsEncodedString_tp* PyBase::Unicode_AsEncodedString = nullptr;
PyBase::PyBytes_AsString_tp* PyBase::Bytes_AsString = nullptr;
#endif

PyBase::PyBase() :
	m_state(1),
	m_interval(std::chrono::milliseconds(100))
{
	if (m_valid)
		return;
	m_valid = true;
#ifdef _WIN32
	std::wstring libstr;
	for (int i = m_high_ver; i > 0; --i)
	{
		//highest supported python version
		libstr = L"python3" + std::to_wstring(i) + L".dll";
		python_dll = LoadLibrary(libstr.c_str());
		if (python_dll != nullptr)
			break;
	}
	if (!SetValid(python_dll)) return;

	Initialize = (decltype(&Py_Initialize))GetProcAddress(python_dll, "Py_Initialize");
	if (!SetValid(Initialize)) return;

	Run_SimpleString = (decltype(&PyRun_SimpleString))GetProcAddress(python_dll, "PyRun_SimpleString");
	if (!SetValid(Run_SimpleString)) return;

	FinalizeEx = (decltype(&Py_FinalizeEx))GetProcAddress(python_dll, "Py_FinalizeEx");
	if (!SetValid(FinalizeEx)) return;

	Import_AddModule = (decltype(&PyImport_AddModule))GetProcAddress(python_dll, "PyImport_AddModule");
	if (!SetValid(Import_AddModule)) return;

	Object_GetAttrString = (decltype(&PyObject_GetAttrString))GetProcAddress(python_dll, "PyObject_GetAttrString");
	if (!SetValid(Object_GetAttrString)) return;

	Unicode_AsEncodedString = (decltype(&PyUnicode_AsEncodedString))GetProcAddress(python_dll, "PyUnicode_AsEncodedString");
	if (!SetValid(Unicode_AsEncodedString)) return;

	Bytes_AsString = (decltype(&PyBytes_AsString))GetProcAddress(python_dll, "PyBytes_AsString");
	if (!SetValid(Bytes_AsString)) return;
#else
	std::string libstr;
	libstr = "/Library/Frameworks/Python.framework/Versions/3.10/lib/libpython3.10.dylib";
	python_dll = dlopen(libstr.c_str(), RTLD_NOW);
	if (!SetValid(python_dll)) return;

	//Initialize = (Initialize_tp*)dlsym(python_dll, "Py_Initialize");
	//if (!SetValid((void*)Initialize)) return;

	//Run_SimpleString = (PyRun_SimpleString_tp*)dlsym(python_dll, "PyRun_SimpleString");
	//if (!SetValid((void*)Run_SimpleString)) return;

	//FinalizeEx = (Py_FinalizeEx_tp*)dlsym(python_dll, "Py_FinalizeEx");
	//if (!SetValid((void*)FinalizeEx)) return;

	//Import_AddModule = (PyImport_AddModule_tp*)dlsym(python_dll, "PyImport_AddModule");
	//if (!SetValid((void*)Import_AddModule)) return;

	//Object_GetAttrString = (PyObject_GetAttrString_tp*)dlsym(python_dll, "PyObject_GetAttrString");
	//if (!SetValid((void*)Object_GetAttrString)) return;

	//Unicode_AsEncodedString = (PyUnicode_AsEncodedString_tp*)dlsym(python_dll, "PyUnicode_AsEncodedString");
	//if (!SetValid((void*)Unicode_AsEncodedString)) return;

	//Bytes_AsString = (PyBytes_AsString_tp*)dlsym(python_dll, "PyBytes_AsString");
	//if (!SetValid((void*)Bytes_AsString)) return;
#endif
}

PyBase::~PyBase()
{
	if (m_thread.valid())
		m_thread.get();
}

bool PyBase::Init()
{
	if (!m_valid)
		return false;

	if (m_state != 1)
		return false;
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
		case ot_Run_SimpleStringEx:
			Run_SimpleStringEx(m.second);
			break;
		case ot_Quit:
			run = false;
			break;
		}
		m_state = 0;
	}

	FinalizeEx();
	m_state = 1;
}

void PyBase::Exit()
{
	Run(ot_Quit);
}

void PyBase::Run_SimpleStringEx(const std::string& str)
{
	std::string stdOutErr =
		"import sys\n" \
		"class CatchOut :\n" \
		"\tdef __init__(self) :\n" \
		"\t\tself.value = ''\n" \
		"\tdef write(self, txt) :\n" \
		"\t\tself.value += txt\n" \
		"catchOut = CatchOut()\n" \
		"sys.stdout = catchOut\n" \
		"sys.stderr = catchOut\n"; //this is python code to redirect stdouts/stderr

	PyObject* pModule = Import_AddModule("__main__"); //create main module
	Run_SimpleString(stdOutErr.c_str()); //invoke code to redirect

	std::string cmd = str + "\n";
	Run_SimpleString(cmd.c_str());
	PyObject* catcher = Object_GetAttrString(pModule, "catchOut");

	PyObject* output = Object_GetAttrString(catcher, "value");
	PyObject* outstr = Unicode_AsEncodedString(output, "utf-8", "~E~");
	m_output = Bytes_AsString(outstr);
}
