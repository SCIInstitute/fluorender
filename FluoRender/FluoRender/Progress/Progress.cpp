/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#include <Progress.h>
#include <Global.h>
#include <MovieMaker.h>

Progress::Progress() :
	m_progress_func(0),
	m_min(0),
	m_max(100),
	m_range(100),
	m_busy(false)
{
	//SetProgressFunc(
	//	std::bind(&MainFrame::SetProgress, glbin_current.mainframe,
	//		std::placeholders::_1, std::placeholders::_2));
}

void Progress::SetProgress(int val, const std::string& str)
{
	if (glbin_moviemaker.IsRunning())
		return;

	int prg = std::round(m_min + (val / 100.0) * m_range);
	if (m_progress_func)
		m_progress_func(prg, str);
}
