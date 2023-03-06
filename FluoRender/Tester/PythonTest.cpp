#include "tests.h"
#include "asserts.h"
#include <Python/PyDlc.h>
#include <wx/wx.h>

void PythonTest0()
{
	wxString cmd;
	cmd = "python -c \"exec('";
	cmd += "import deeplabcut\n";
	//cmd += "deeplabcut.analyze_videos(";
	//cmd += "\"E:\\\\DATA\\\\Holly\\\\MouseTracking\\\\test-nb-2023-01-30\\\\config.yaml\",";
	//cmd += "\"E:\\\\DATA\\\\Holly\\\\MouseTracking\\\\videos\\\\10June2019_M_P119_GAD2-Cre-GCaMP5G_01L.mp4\",";
	//cmd += "save_as_csv=True)\n";
	cmd += "')\"";
	wxExecute(cmd);
}

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