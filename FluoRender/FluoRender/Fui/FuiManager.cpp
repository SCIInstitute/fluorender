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

#include <FuiManager.h>
#include <JVMInitializer.h>
#include <Global.hpp>
#include <StopWatch.hpp>
#include <AgentFactory.hpp>
#include <RenderFrame.h>
#include <wx/stdpaths.h>

FuiManager::FuiManager() :
	m_frame(0),
	m_benchmark(false),
	m_fullscreen(false),
	m_windowed(false),
	m_hidepanels(false),
	m_win_width(1600),
	m_win_height(1000),
	m_lzw(false),
	m_save_alpha(false),
	m_save_float(false),
	m_bitrate(10.0),
	m_mov_file(""),
	m_imagej(false)
{
	char cpath[FILENAME_MAX];
	GETCURRENTDIR(cpath, sizeof(cpath));
	std::wstring wstr_path = s2ws(std::string(cpath));
	::wxSetWorkingDirectory(wstr_path);
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	glbin.setExecutablePath(expath.ToStdWstring());
	//random numbers
	srand((unsigned int)glbin.getStopWatch(gstStopWatch)->sys_time());
}

FuiManager::~FuiManager()
{
	JVMInitializer::destroyJVM();
}

RenderFrame* FuiManager::GetFrame()
{
	return m_frame;
}

void FuiManager::Init()
{
	//add png handler
	wxImage::AddHandler(new wxPNGHandler);
	glbin.initIcons();

	//the frame
	std::string title = std::string(FLUORENDER_TITLE) + std::string(" ") +
		std::string(VERSION_MAJOR_TAG) + std::string(".") +
		std::string(VERSION_MINOR_TAG);
	m_frame = new RenderFrame(
		wxString(title),
		-1, -1,
		m_win_width, m_win_height);
	LinkAgents();
	m_frame->Init(m_benchmark, m_fullscreen, m_windowed, m_hidepanels);

	fluo::RenderFrameAgent* renderframeagent = glbin_agtf->getRenderFrameAgent();

	// Adding JVm initialization.
	if (renderframeagent)
		JVMInitializer*	pInstance = JVMInitializer::getInstance(renderframeagent->GetJvmArgs());

	bool run_mov = false;
	if (m_mov_file != "")
	{
		if (renderframeagent)
		{
			renderframeagent->setValue(gstCaptureCompress, m_lzw);
			renderframeagent->setValue(gstCaptureAlpha, m_save_alpha);
			renderframeagent->setValue(gstCaptureFloat, m_save_float);
		}
		fluo::MovieAgent* movieagent = glbin_agtf->getMovieAgent();
		if (movieagent)
		{
			movieagent->setValue(gstMovBitrate, m_bitrate);
			movieagent->setValue(gstMovFilename, m_mov_file.ToStdWstring());
		}
		run_mov = true;
	}

	if (m_files.GetCount() > 0 && renderframeagent)
		renderframeagent->StartupLoad(m_files, run_mov, m_imagej);

}

void FuiManager::LinkAgents()
{

}