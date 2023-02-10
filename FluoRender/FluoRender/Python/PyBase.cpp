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

using namespace flrd;

PyBase::PyBase()
{
}

PyBase::~PyBase()
{
}

bool PyBase::Run()
{
	wchar_t program[] = L"teset";
	Py_SetProgramName(program);  /* optional but recommended */
	Py_Initialize();
	PyObject* py_dict = PyDict_New();
	PyRun_SimpleString("from time import time,ctime\n"
		"print('Today is', ctime(time()))\n");
	PyObject* pr = PyRun_String("from time import time,ctime\n"
		"print('Today is', ctime(time()))\n",
		Py_single_input,
		py_dict,
		py_dict);
	PyObject* objectsRepresentation = PyObject_Repr(pr);
	if (Py_FinalizeEx() < 0)
	{
		exit(120);
	}
	return true;
}