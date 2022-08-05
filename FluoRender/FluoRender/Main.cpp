/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <Main.h>
#pragma message ("replace main frame")
//#include <RenderFrame.h>

#ifdef _WIN32
#include <Windows.h>
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	return 0;
}
#else
int main()
{
  return 1;
}
#endif
#pragma message ("replace command line handling")
/*static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
	{ wxCMD_LINE_SWITCH, "u", "usage", "list command line usage",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "b", "benchmark", "start FluoRender in the benchmark mode",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
	{ wxCMD_LINE_SWITCH, "f", "fullscreen", "start FluoRender in the full screen mode",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "win", "windowed", "start FluoRender in a non maximized window",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "hp", "hidepanels", "start FluoRender without showing its ui panels",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "lzw", "lzw", "compress images of the exported movie using LZW",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "a", "alpha", "save alpha channel of the exported movie",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "fp", "float", "save float channel of the exported movie",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "j", "imagej", "use imagej to load volume files",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION, "w", "width", "width of FluoRender's start window",
		wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION, "h", "height", "height of FluoRender's start window",
		wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION, "br", "bitrate", "bit rate of the exported movie",
		wxCMD_LINE_VAL_DOUBLE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION, "m", "movie", "export movie",
		wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_PARAM, NULL, NULL, "a file or project to load",
		wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
	{ wxCMD_LINE_NONE }
};*/

bool FluoRenderApp::OnInit()
{
	//_CrtSetBreakAlloc(331430);
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// call default behaviour (mandatory)
#pragma message ("replace app initialization")
	//if (!wxApp::OnInit())
	//	return false;

	m_manager.Init();
	//RenderFrame* frame = m_manager.GetFrame();
	//if (!frame)
	//	return false;

	//SetTopWindow(frame);
	//frame->Show();

	return true;
}

int FluoRenderApp::OnExit()
{
	return 0;
}

/*void FluoRenderApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars("-");
}

bool FluoRenderApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	//control string
	if (parser.Found("u"))
		parser.Usage();
	if (parser.Found("b"))
		m_manager.SetBenchmark();
	if (parser.Found("f"))
		m_manager.SetFullscreen();
	if (parser.Found("win"))
		m_manager.SetWindowed();
	if (parser.Found("hp"))
		m_manager.SetHidePanels();
	if (parser.Found("lzw"))
		m_manager.SetLzw();
	if (parser.Found("a"))
		m_manager.SetSaveAlpha();
	if (parser.Found("fp"))
		m_manager.SetSaveFloat();
	if (parser.Found("j"))
		m_manager.SetImagej();
	long lVal;
	if (parser.Found("w", &lVal))
		m_manager.SetWinWidth(lVal);
	if (parser.Found("h", &lVal))
		m_manager.SetWinHeight(lVal);
	double dVal;
	if (parser.Found("br", &dVal))
		m_manager.SetBitRate(dVal);
	wxString str;
	if (parser.Found("m", &str))
		m_manager.SetMovFile(str);
	//volumes to load
	for (size_t i = 0; i < parser.GetParamCount(); ++i)
		m_manager.AddFile(parser.GetParam(i));

	return true;
}*/
