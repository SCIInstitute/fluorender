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
#ifndef _COLOCALIZE_H_
#define _COLOCALIZE_H_

#include <Compare.h>
#include <wx/string.h>
#include <chrono>

namespace flrd
{
	class Colocalize
	{
	public:
		Colocalize();
		~Colocalize();

		void Compute();
		wxString GetTitles() { return m_titles; }
		wxString GetValues() { return m_values; }

	private:
		wxString m_titles, m_values;//results
		bool m_test_speed;
		std::vector<std::chrono::high_resolution_clock::time_point> m_tps;

	private:
		//speed test
		void StartTimer(const std::string& str);
		void StopTimer(const std::string& str);
	};

}
#endif//_COLOCALIZE_H_
