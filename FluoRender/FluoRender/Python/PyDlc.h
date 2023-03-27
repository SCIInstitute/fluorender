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
#include <compatibility.h>
#include <Types/Point.h>
#include <regex>
#include <boost/lexical_cast.hpp>

namespace flrd
{
	class RulerHandler;
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
		void SetFrameNumber(int n)
		{
			m_frame_num = n;
			m_start_fn = 0;
			m_end_fn = n - 1;
			m_fn_length = 0;
			int temp = n;
			while (temp)
			{
				temp /= 10;
				m_fn_length++;
			}
		}
		void SetFrameRange(int start, int end)
		{
			m_start_fn = start;
			m_end_fn = end;
		}
		void SetFrameSize(int nx, int ny)
		{
			m_nx = nx;
			m_ny = ny;
		}
		std::string GetLabelPath()
		{
			return m_label_path;
		}
		std::string GetConfigFile()
		{
			return m_config_file_py;
		}
		//train
		std::string GetTrainCmd(int maxiters);
		void Train(int maxiters);
		void CreateConfigFile(const std::string& prj_name,
			const std::string& usr_name,
			RulerHandler* rhdl);
		void WriteHDF(RulerHandler* rhdl);
		//analysis
		void AnalyzeVideo();
		bool GetResultFile();
		int GetDecodeErrorCount();
		bool AddRulers(RulerHandler* rhdl, size_t toff);//time offset: dlc may have decoding error causing the offset

	protected:
		std::string m_config_file;
		std::string m_config_file_py;
		std::string m_video_file;
		std::string m_video_file_py;
		std::string m_result_file;
		std::string m_label_path;
		std::string m_prj_name;
		std::string m_usr_name;
		int m_frame_num;
		int m_start_fn;
		int m_end_fn;
		int m_fn_length;
		int m_nx;
		int m_ny;

		bool isFloat(const std::string& someString)
		{
			using boost::lexical_cast;
			using boost::bad_lexical_cast;

			try
			{
				boost::lexical_cast<float>(someString);
			}
			catch (bad_lexical_cast&)
			{
				return false;
			}

			return true;
		}

		bool getPoints(std::vector<std::string>& entry,
			std::vector<std::string>& props,
			std::vector<fluo::Point>& points)
		{
			fluo::Point p;
			for (size_t i = 0; i < entry.size(); ++i)
			{
				if (props[i] == "x")
					p.x(std::stof(entry[i]));
				if (props[i] == "y")
				{
					p.y(std::stof(entry[i]));
					points.push_back(p);
				}
			}
			return true;
		}

		bool getNames(std::vector<std::string>& entry,
			std::vector<std::string>& names,
			std::vector<int>& sn)
		{
			for (size_t i = 1; i < entry.size(); ++i)
			{
				std::string str = entry[i];
				std::string base = getBase(str);//separate digits
				if (names.empty())
				{
					//first
					names.push_back(base);
					sn.push_back(0);
					continue;
				}
				std::string last_base = names.back();
				std::string last_str = entry[i - 1];
				bool inc = false;
				if (base == last_base)
				{
					if (str == last_str)
						continue;//same point
					else
						inc = true;//same ruler
				}
				names.push_back(base);
				if (inc)
					sn.push_back(sn.back() + 1);//same ruler
				else
					sn.push_back(0);//new ruler
			}
			//also use sn for ruler type
			for (size_t i = 0; i < sn.size(); ++i)
			{
				if (sn[i] == 0)
				{
					if (i == sn.size() - 1)
						sn[i] = -2;//locator
					else if (names[i] != names[i + 1])
						sn[i] = -2;//locator
				}
			}
			return true;
		}

		std::string getBase(const std::string& str)
		{
			std::string base = str;
			while (std::isdigit(base.back()))
				base.pop_back();
			while (base.back() == '_')
				base.pop_back();
			while (base.back() == ' ')
				base.pop_back();
			return base;
		}

#if defined(_WIN32) || defined(_DARWIN)
		typedef int64_t hid_t;
		bool hdf_write_attr_b8(hid_t item, const std::string& name, char cval);
		bool hdf_write_attr_int(hid_t item, const std::string& name, int ival);
		bool hdf_write_attr_utf(hid_t item, const std::string& name, const std::u8string& str);
		bool hdf_write_array(hid_t group, const std::string& name, const std::vector<int>& vals);
		bool hdf_write_array_char(hid_t group, const std::string& name, const std::vector<char>& vals);
		bool hdf_write_array_short(hid_t group, const std::string& name, const std::vector<short>& vals);
		bool hdf_write_array_int(hid_t group, const std::string& name, const std::vector<int>& vals);
		bool hdf_write_array_str(hid_t group,
			const std::string& name,
			const std::string& str1,
			const std::u8string& str2,
			const std::vector<std::string>& vals);
		bool hdf_write_array2_double(hid_t group,
			const std::string& name,
			int nx, int ny,
			const std::vector<double>& vals);
#endif
	};
}

#endif//PYDLC_H