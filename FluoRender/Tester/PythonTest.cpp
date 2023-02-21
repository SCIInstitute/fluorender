#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>

void PythonTest()
{
	flrd::PyDlc test;
	test.Init();
	test.LoadDlc();
	test.SetConfigFile(
		"E:\\DATA\\Holly\\MouseTracking\\test-nb-2023-01-30\\config.yaml");
	test.SetVideoFile(
		"E:\\DATA\\Holly\\MouseTracking\\videos\\10June2019_M_P119_GAD2-Cre-GCaMP5G_02L.m4v");
	test.AnalyzeVideo();
	test.Exit();
}