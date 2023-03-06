#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest1(const std::string& config, const std::string& video)
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
	int err = test.GetDecodeErrorCount();
	//test.AddRulers(NULL, 0);
	test.Exit();
}

void PythonTest2(const std::string& config, const std::string& video)
{
	flrd::PyDlc test;
	test.Init();
	test.LoadDlc();
	test.SetConfigFile(config);
	test.SetVideoFile(video);
	test.CreateConfigFile("test", "tester", 0);
	while (test.GetState())
		Sleep(1000);
	test.Exit();
}