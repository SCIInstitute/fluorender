#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest()
{
	flrd::PyBase* test = new flrd::PyBase();
	test->Init();
	test->Run(flrd::PyBase::ot_Run_SimpleString,
		"import deeplabcut");
	while (test->GetState()) {}
	test->Run(flrd::PyBase::ot_Quit);
	while (test->GetState()) {}
	delete test;
}