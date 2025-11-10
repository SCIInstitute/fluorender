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

#ifndef _MAIN_H_
#define _MAIN_H_

#include <wx/wx.h>

class FluoRenderApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
	void OnInitCmdLine(wxCmdLineParser& parser);
	bool OnCmdLineParsed(wxCmdLineParser& parser);	

private:
	std::vector<std::wstring> m_files;
	size_t m_file_num;
	int m_reset;// 0: no reset, 1: factory reset, 2: recommended reset
	bool m_benchmark;
	bool m_fullscreen;
	bool m_windowed;
	bool m_hidepanels;
	int m_win_width;
	int m_win_height;
	std::wstring m_mov_file;
	double m_bitrate;
	bool m_lzw;
	bool m_save_alpha;
	bool m_save_float;
	float m_dpi;
	bool m_imagej;
};

DECLARE_APP(FluoRenderApp)

#endif//_MAIN_H_
