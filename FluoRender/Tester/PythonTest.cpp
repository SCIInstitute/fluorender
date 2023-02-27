#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest(const std::string& config, const std::string& video)
{
	flrd::PyDlc test;
	test.Init();
	test.LoadDlc();
	test.SetConfigFile(config);
	test.SetVideoFile(video);
	test.AnalyzeVideo();
	while (test.GetState())
		Sleep(1000);
	test.GetResultFile();
	test.AddRulers(NULL, 0);
	test.Exit();
}