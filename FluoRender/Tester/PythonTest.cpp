#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest()
{
	flrd::PyDlc test;
	test.Init();
	test.LoadDlc();
	test.SetConfigFile(
		"C:\\Users\\basis\\Documents\\DATA\\Holly\\mouse_tracking\\test-nb-2023-01-30\\config.yaml");
	test.SetVideoFile(
		"C:\\Users\\basis\\Documents\\DATA\\Holly\\mouse_tracking\\videos\\10June2019_M_P119_GAD2-Cre-GCaMP5G_02L.m4v");
	test.AnalyzeVideo();
	while (test.GetState())
		Sleep(1000);
	test.GetResultFile();
	test.AddRulers(NULL);
	test.Exit();
}