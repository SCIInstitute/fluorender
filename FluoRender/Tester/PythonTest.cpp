#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest()
{
	flrd::PyDlc test;
	test.Init();
	test.Run();
}