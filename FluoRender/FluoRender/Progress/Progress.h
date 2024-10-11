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

#ifndef _PROGRESS_H_
#define _PROGRESS_H_

#include <wx/string.h>

class Progress
{
public:
	Progress();
	~Progress() {}

	//set progress
	void SetProgressFunc(const std::function<void(int, const wxString&)>& f)
	{
		m_progress_func = f;
	}
	std::function<void(int, const wxString&)> GetProgressFunc()
	{
		return m_progress_func;
	}
	void SetProgress(int val, const wxString& str)
	{
		int prg = std::round(m_min + (val / 100.0) * m_range);
		if (m_progress_func)
			m_progress_func(prg, str);
	}
	void SetRange(int v1, int v2)
	{
		m_min = std::min(v1, v2);
		m_max = std::max(v1, v2);
		m_range = m_max - m_min;
	}
	int GetMin()
	{
		return m_min;
	}
	int GetMax()
	{
		return m_max;
	}
	int GetRange()
	{
		return m_range;
	}

	bool IsBusy() { return m_busy; }

protected:
	bool m_busy;//it's currently working

private:
	std::function<void(int, const wxString&)> m_progress_func;
	int m_min;
	int m_max;
	int m_range;

};

#endif//_PROGRESS_H_
