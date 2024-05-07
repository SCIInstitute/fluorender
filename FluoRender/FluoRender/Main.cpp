/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <MainFrame.h>
#include <compatibility.h>
#include <JVMInitializer.h>
#include <Global.h>
#include <cstdio>
#include <iostream>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>

IMPLEMENT_APP(VRenderApp)

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
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
	{ wxCMD_LINE_SWITCH, "dpi", "dpi", "set dpi of the exported movie",
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
};

bool VRenderApp::OnInit()
{
	//_CrtSetBreakAlloc(331430);
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	char cpath[FILENAME_MAX];
	GETCURRENTDIR(cpath, sizeof(cpath));
	::wxSetWorkingDirectory(wxString(s2ws(std::string(cpath))));
	// call default behaviour (mandatory)
	if (!wxApp::OnInit())
		return false;
	//add png handler
	wxImage::AddHandler(new wxPNGHandler);
	//random numbers
	srand((unsigned int)TIME());

	//the frame
	std::string title = std::string(FLUORENDER_TITLE) + std::string(" ") +
		std::string(VERSION_MAJOR_TAG) + std::string(".") +
		std::string(VERSION_MINOR_TAG);
	wxFrame* frame = new MainFrame(
		(wxFrame*)NULL,
		wxString(title),
		-1, -1,
		m_win_width, m_win_height,
		m_benchmark, m_fullscreen,
		m_windowed, m_hidepanels);
	SetTopWindow(frame);
	frame->Show();
	bool run_mov = false;
	if (m_mov_file != "")
	{
		glbin_settings.m_save_compress = m_lzw;
		glbin_settings.m_save_alpha = m_save_alpha;
		glbin_settings.m_save_float = m_save_float;
		glbin_settings.m_dpi = m_dpi;
		glbin_settings.m_mov_bitrate = m_bitrate;
		glbin_settings.m_mov_filename = m_mov_file;
		run_mov = true;
	}
	if (m_file_num > 0)
		((MainFrame*)frame)->StartupLoad(m_files, run_mov, m_imagej);

	// Adding JVm initialization.
	JVMInitializer*	pInstance = JVMInitializer::getInstance(glbin_settings.GetJvmArgs());
	return true;
}

int VRenderApp::OnExit()
{
	JVMInitializer::destroyJVM();
	return 0;
}

void VRenderApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars("-");
}

bool VRenderApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	m_file_num = 0;
	m_benchmark = false;
	m_fullscreen = false;
	m_windowed = false;
	m_hidepanels = false;
	m_win_width = 1600;
	m_win_height = 1000;
	m_lzw = false;
	m_save_alpha = false;
	m_save_float = false;
	m_dpi = 72.0f;
	m_bitrate = 10.0;
	m_mov_file = "";
	m_imagej = false;

	//control string
	if (parser.Found("u"))
		parser.Usage();
	if (parser.Found("b"))
		m_benchmark = true;
	if (parser.Found("f"))
		m_fullscreen = true;
	if (parser.Found("win"))
		m_windowed = true;
	if (parser.Found("hp"))
		m_hidepanels = true;
	if (parser.Found("lzw"))
		m_lzw = true;
	if (parser.Found("a"))
		m_save_alpha = true;
	if (parser.Found("fp"))
		m_save_float = true;
	if (parser.Found("j"))
		m_imagej = true;
	long lVal;
	if (parser.Found("w", &lVal))
		m_win_width = lVal;
	if (parser.Found("h", &lVal))
		m_win_height = lVal;
	double dVal;
	if (parser.Found("br", &dVal))
		m_bitrate = dVal;
	if (parser.Found("dpi", &dVal))
		m_dpi = float(dVal);
	wxString str;
	if (parser.Found("m", &str))
		m_mov_file = str;
	//volumes to load
	for (size_t i = 0; i < parser.GetParamCount(); ++i)
		m_files.Add(parser.GetParam(i));
	m_file_num = m_files.GetCount();

	return true;
}
