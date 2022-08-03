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

#ifndef FUI_MANAGER_H
#define FUI_MANAGER_H

#include <string>
#include <vector>

//class RenderFrame;
class FuiManager
{
public:
	FuiManager();
	~FuiManager();

	//RenderFrame* GetFrame();

	void Init();

	void SetBenchmark(bool bval = true) { m_benchmark = bval; }
	void SetFullscreen(bool bval = true) { m_fullscreen = bval; }
	void SetWindowed(bool bval = true) { m_windowed = bval; }
	void SetHidePanels(bool bval = true) { m_hidepanels = bval; }
	void SetLzw(bool bval = true) { m_lzw = bval; }
	void SetSaveAlpha(bool bval = true) { m_save_alpha = bval; }
	void SetSaveFloat(bool bval = true) { m_save_float = bval; }
	void SetImagej(bool bval = true) { m_imagej = bval; }
	void SetWinWidth(long lval) { m_win_width = lval; }
	void SetWinHeight(long lval) { m_win_height = lval; }
	void SetBitRate(double dval) { m_bitrate = dval; }
	void SetMovFile(const std::string& sval) { m_mov_file = sval; }
	void AddFile(const std::vector<std::wstring>& saval)
	{
		m_files.insert(m_files.end(), saval.begin(), saval.end());
	}

private:
	//RenderFrame* m_frame;

	bool m_benchmark;
	bool m_fullscreen;
	bool m_windowed;
	bool m_hidepanels;
	bool m_lzw;
	bool m_save_alpha;
	bool m_save_float;
	bool m_imagej;
	int m_win_width;
	int m_win_height;
	double m_bitrate;
	std::string m_mov_file;
	std::vector<std::wstring> m_files;

	void SetupAgents();
};
#endif//FUI_MANAGER_H