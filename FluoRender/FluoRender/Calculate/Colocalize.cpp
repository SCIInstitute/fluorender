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
#include <Colocalize.h>
#include <Global.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <ColocalDefault.h>
#include <AutomateDefault.h>
#include <Compare.h>
#include <string>

using namespace flrd;

Colocalize::Colocalize() :
	m_test_speed(false)
{
}

Colocalize::~Colocalize()
{
}

bool Colocalize::GetAutoColocalize()
{
	int get_colocalize = glbin_automate_def.m_colocalize;
	if (get_colocalize == 0)
		return false;
	else if (get_colocalize == 1)
		return true;
	else if (get_colocalize == 2)
	{
		auto group = glbin_current.vol_group.lock();
		if (!group)
			return false;
		auto vd = group->GetVolumeData(0);
		if (!vd)
			return false;
		if (vd->GetAllBrickNum() == 1)
			return true;
	}
	return false;
}

void Colocalize::Compute()
{
	auto view = glbin_current.render_view.lock();
	auto group = glbin_current.vol_group.lock();
	if (!group)
		return;

	int num = group->GetVolumeNum();
	if (num < 2)
		return;

	//spacings, assuming they are all same for channels
	double spc = 1.0;
	std::wstring unit;
	auto vd = group->GetVolumeData(0);
	if (vd)
	{
		auto spacing = vd->GetSpacing();
		spc = spacing.x() * spacing.y() * spacing.z();
	}
	if (view)
	{
		switch (view->m_sb_unit)
		{
		case 0:
			unit = L"nm\u00B3";
			break;
		case 1:
		default:
			unit = L"\u03BCm\u00B3";
			break;
		case 2:
			unit = L"mm\u00B3";
			break;
		}
	}

	//result
	std::vector<std::vector<double>> rm;//result matrix
	rm.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	m_titles.clear();
	m_values.clear();
	m_tps.clear();

	//fill the matrix
	if (glbin_colocal_def.m_method == 0 || glbin_colocal_def.m_method == 1 ||
		(glbin_colocal_def.m_method == 2 && !glbin_colocal_def.m_int_weighted))
	{
		//dot product and min value
		//symmetric matrix
		for (int it1 = 0; it1 < num; ++it1)
		{
			for (int it2 = it1; it2 < num; ++it2)
			{
				auto vd1 = group->GetVolumeData(it1);
				auto vd2 = group->GetVolumeData(it2);
				if (!vd1 || !vd2 ||
					!vd1->GetDisp() ||
					!vd2->GetDisp())
					continue;

				flrd::ChannelCompare compare(vd1.get(), vd2.get());
				compare.SetUseMask(glbin_colocal_def.m_use_mask);
				compare.SetIntWeighted(glbin_colocal_def.m_int_weighted);
				compare.prework = std::bind(
					&Colocalize::StartTimer, this, std::placeholders::_1);
				compare.postwork = std::bind(
					&Colocalize::StopTimer, this, std::placeholders::_1);
				switch (glbin_colocal_def.m_method)
				{
				case 0://dot product
					compare.Product();
					break;
				case 1://min value
					compare.MinValue();
					break;
				case 2://threshold
				{
					//get threshold values
					float th1, th2, th3, th4;
					th1 = (float)(vd1->GetLeftThresh());
					th2 = (float)(vd1->GetRightThresh());
					th3 = (float)(vd2->GetLeftThresh());
					th4 = (float)(vd2->GetRightThresh());
					compare.Threshold(th1, th2, th3, th4);
				}
				break;
				}
				rm[it1][it2] = compare.Result();
				if (it1 != it2)
					rm[it2][it1] = compare.Result();
			}
		}
	}
	else if (glbin_colocal_def.m_method == 2 && glbin_colocal_def.m_int_weighted)
	{
		//threshold, asymmetrical
		for (int it1 = 0; it1 < num; ++it1)
			for (int it2 = 0; it2 < num; ++it2)
			{
				auto vd1 = group->GetVolumeData(it1);
				auto vd2 = group->GetVolumeData(it2);
				if (!vd1 || !vd2 ||
					!vd1->GetDisp() ||
					!vd2->GetDisp())
					continue;

				flrd::ChannelCompare compare(vd1.get(), vd2.get());
				compare.SetUseMask(glbin_colocal_def.m_use_mask);
				compare.SetIntWeighted(glbin_colocal_def.m_int_weighted);
				compare.prework = std::bind(
					&Colocalize::StartTimer, this, std::placeholders::_1);
				compare.postwork = std::bind(
					&Colocalize::StopTimer, this, std::placeholders::_1);
				//get threshold values
				float th1, th2, th3, th4;
				th1 = (float)(vd1->GetLeftThresh());
				th2 = (float)(vd1->GetRightThresh());
				th3 = (float)(vd2->GetLeftThresh());
				th4 = (float)(vd2->GetRightThresh());
				compare.Threshold(th1, th2, th3, th4);
				rm[it1][it2] = compare.Result();
			}
	}

	if (m_test_speed)
	{
		m_titles += L"Function\t";
		m_titles += L"Time\n";
	}
	else
	{
		std::wstring name;
		double v;
		glbin_colocal_def.ResetMinMax();
		for (size_t i = 0; i < num; ++i)
		{
			if (glbin_colocal_def.m_get_ratio)
				m_titles += std::to_wstring(i+1) + L" (%%)";
			else
				m_titles += std::to_wstring(i+1);
			auto vd = group->GetVolumeData(static_cast<int>(i));
			if (vd)
				name = vd->GetName();
			else
				name = L"";
			m_titles += L": " + name;
			if (i < num - 1)
				m_titles += L"\t";
			else
				m_titles += L"\n";
		}
		for (int it1 = 0; it1 < num; ++it1)
			for (int it2 = 0; it2 < num; ++it2)
			{
				if (glbin_colocal_def.m_get_ratio)
				{
					if (rm[it2][it2])
					{
						v = rm[it1][it2] * 100.0 / rm[it1][it1];
						glbin_colocal_def.SetMinMax(v);
						m_values += std::to_wstring(v);
					}
					else
					{
						glbin_colocal_def.SetMinMax(0.0);
						m_values += L"0";
					}
				}
				else
				{
					if (glbin_colocal_def.m_physical_size)
					{
						v = rm[it1][it2] * spc;
						glbin_colocal_def.SetMinMax(v);
						m_values += std::to_wstring(v);
						m_values += unit;
					}
					else
					{
						v = rm[it1][it2];
						glbin_colocal_def.SetMinMax(v);
						if (glbin_colocal_def.m_int_weighted)
							m_values += std::to_wstring(v);
						else
							m_values += std::to_wstring(static_cast<int>(v));
					}
				}
				if (it2 < num - 1)
					m_values += L"\t";
				else
					m_values += L"\n";
			}
	}
}

void Colocalize::StartTimer(const std::string& str)
{
	if (m_test_speed)
	{
		m_tps.push_back(std::chrono::high_resolution_clock::now());
	}
}

void Colocalize::StopTimer(const std::string& str)
{
	if (m_test_speed)
	{
		auto t0 = m_tps.back();
		m_tps.push_back(std::chrono::high_resolution_clock::now());
		std::chrono::duration<double> time_span =
			std::chrono::duration_cast<std::chrono::duration<double>>(
				m_tps.back() - t0);

		std::wostringstream oss;
		oss << s2ws(str) << L"\t";
		oss << std::fixed << std::setprecision(4) << time_span.count();
		oss << L" sec.\n";
		m_values += oss.str();
	}
}

