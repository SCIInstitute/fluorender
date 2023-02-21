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
#include <PyDlc.h>
#include <filesystem>

using namespace flrd;

PyDlc::PyDlc()
{

}

PyDlc::~PyDlc()
{

}

void PyDlc::LoadDlc()
{
	Run(ot_Run_SimpleString,
		"import deeplabcut");
}

void PyDlc::AnalyzeVideo()
{
	std::string cmd =
		"deeplabcut.analyze_videos(";
	cmd += "\"" + m_config_file_py + "\", ";
	cmd += "\"" + m_video_file_py + "\", ";
	cmd += "save_as_csv=True)";
	Run(ot_Run_SimpleString,
		cmd);
}

bool PyDlc::GetResultFile()
{
	if (m_state)
		return false;//busy

	std::filesystem::path p = m_video_file;
	m_result_file = p.replace_extension().string();
	std::filesystem::path path = p.parent_path();
	m_result_file += "*.csv";
	std::regex rgx = REGEX(m_result_file);
	bool result = false;
	for (auto& it : std::filesystem::directory_iterator(path))
	{
		if (!std::filesystem::is_regular_file(it))
			continue;
		const std::string str = it.path().string();
		if (std::regex_match(str, rgx))
		{
			m_result_file = str;
			result = true;
			break;
		}
	}
	return result;
}