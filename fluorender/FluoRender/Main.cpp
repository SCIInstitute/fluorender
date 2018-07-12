/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include "Main.h"
#include <cstdio>
#include <iostream>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include "VRenderFrame.h"
#include "compatibility.h"

IMPLEMENT_APP(VRenderApp)

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
	{ wxCMD_LINE_SWITCH, "b", "benchmark", "start FluoRender in the benchmark mode",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL},
	{ wxCMD_LINE_SWITCH, "f", "fullscreen", "start FluoRender in the full screen mode",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "win", "windowed", "start FluoRender in a non maximized window",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_SWITCH, "hp", "hidepanels", "start FluoRender without showing its ui panels",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION, "w", "width", "width of FluoRender's start window",
		wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_OPTION, "h", "height", "height of FluoRender's start window",
		wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
	{ wxCMD_LINE_PARAM, NULL, NULL, "a volume file to load",
		wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
	{ wxCMD_LINE_NONE }
};

bool VRenderApp::OnInit()
{
	//_CrtSetBreakAlloc(804780);
	char cpath[FILENAME_MAX];
	GETCURRENTDIR(cpath, sizeof(cpath));
	::wxSetWorkingDirectory(wxString(s2ws(std::string(cpath))));
	// call default behaviour (mandatory)
	if (!wxApp::OnInit())
		return false;
	//add png handler
	wxImage::AddHandler(new wxPNGHandler);
	//the frame
	std::string title = std::string(FLUORENDER_TITLE) + std::string(" ") +
		std::string(VERSION_MAJOR_TAG) + std::string(".") +
		std::string(VERSION_MINOR_TAG);
	wxFrame* frame = new VRenderFrame(
		(wxFrame*)NULL,
		wxString(title),
		-1, -1,
		m_win_width, m_win_height,
		m_benchmark, m_fullscreen,
		m_windowed, m_hidepanels);
	SetTopWindow(frame);
	frame->Show();
	if (m_file_num > 0)
		((VRenderFrame*)frame)->StartupLoad(m_files);
	return true;
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

	//control string
	if (parser.Found("b"))
		m_benchmark = true;
	if (parser.Found("f"))
		m_fullscreen = true;
	if (parser.Found("win"))
		m_windowed = true;
	if (parser.Found("hp"))
		m_hidepanels = true;
	long lVal;
	if (parser.Found("w", &lVal))
		m_win_width = lVal;
	if (parser.Found("h", &lVal))
		m_win_height = lVal;

	//volumes to load
	for (size_t i = 0; i < parser.GetParamCount(); ++i)
		m_files.Add(parser.GetParam(i));
	m_file_num = m_files.GetCount();

	return true;
}
