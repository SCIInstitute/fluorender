#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest()
{
	flrd::PyDlc test;
	test.Init();
	test.Run(flrd::PyBase::ot_Run_SimpleString,
		"from time import time,ctime\n" \
		"print('Today is', ctime(time()))\n");
	while (test.GetState()) {}
	test.Run(flrd::PyBase::ot_Quit);
	while (test.GetState()) {}
}