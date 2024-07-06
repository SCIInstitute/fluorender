#include "tests.h"
#include "asserts.h"
//#include <PyDlc.h>
#include <wx/wx.h>

void PythonTest0()
{
	//int displayiters = 1000;
	//int maxiters = 1;
	//maxiters *= displayiters;
	//int saveiters = maxiters / 2;
	//wxString cmd = "python -c \"";
	//cmd += "import deeplabcut\n";
	//cmd += "deeplabcut.create_training_dataset(\\\"";
	//cmd += "E:\\\\DATA\\\\Holly\\\\MouseTracking\\\\test02\\\\config.yaml";
	//cmd += "\\\", augmenter_type='imgaug')\n";
	//cmd += "deeplabcut.train_network(\\\"";
	//cmd += "E:\\\\DATA\\\\Holly\\\\MouseTracking\\\\test02\\\\config.yaml";
	//cmd += "\\\", ";
	//cmd += "displayiters=" + std::to_string(displayiters) + ", ";
	//cmd += "saveiters=" + std::to_string(saveiters) + ", ";
	//cmd += "maxiters=" + std::to_string(maxiters) + ")\n";
	//cmd += "print('Done. Quit.')\"";
	//wxExecute(cmd);
}

void PythonTest1(const std::string& config, const std::string& video)
{
	//flrd::PyDlc test;
	//test.Init();
	//test.LoadDlc();
	//test.SetConfigFile(config);
	//test.SetVideoFile(video);
	//test.AnalyzeVideo();
	//while (test.GetState())
	//	Sleep(1000);
	//test.GetResultFile();
	//int err = test.GetDecodeErrorCount();
	////test.AddRulers(NULL, 0);
	//test.Exit();
}

void PythonTest2(const std::string& config, const std::string& video)
{
	//flrd::PyDlc test;
	//test.Init();
	//test.LoadDlc();
	//test.SetConfigFile(config);
	//test.SetVideoFile(video);
	//test.CreateConfigFile("test", "tester", 0);
	//while (test.GetState())
	//	Sleep(1000);
	//test.Exit();
}