/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#ifndef PYDLC_H
#define PYDLC_H

#include <PyBase.h>
#include <regex>
#include <compatibility.h>

namespace flrd
{
	class PyDlc : public PyBase
	{
	public:
		PyDlc();
		~PyDlc();

		void LoadDlc();
		void SetConfigFile(const std::string& str)
		{
			m_config_file = str;
#ifdef _WIN32
			m_config_file_py = std::regex_replace(m_config_file,
				std::regex(R"(\\)"), R"(\\)");
#else
			m_config_file_py = m_config_file;
#endif
		}
		void SetVideoFile(const std::string& str)
		{
			m_video_file = str;
#ifdef _WIN32
			m_video_file_py = std::regex_replace(m_video_file,
				std::regex(R"(\\)"), R"(\\)");
#else
			m_video_file_py = m_video_file;
#endif
		}
		void AnalyzeVideo();
		bool GetResultFile();

	protected:
		std::string m_config_file;
		std::string m_config_file_py;
		std::string m_video_file;
		std::string m_video_file_py;
		std::string m_result_file;
	};
}

#endif//PYDLC_H