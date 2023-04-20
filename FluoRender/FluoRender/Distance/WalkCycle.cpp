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
#include "WalkCycle.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace flrd;

WalkCycle::WalkCycle() :
	corr_(0),
	in_corr_(0)
{

}

WalkCycle::~WalkCycle()
{

}

void WalkCycle::ReadData(const std::string& name)
{
	std::ifstream f(name);
	if (!f.good())
		return;

	std::string line;
	int ln = 0;
	while (std::getline(f, line))
	{
		std::istringstream s(line);
		std::string item;
		std::vector<std::string> entry;
		while (std::getline(s, item, ','))
			entry.push_back(item);
		size_t en = 0;
		for (auto& it : entry)
			if (it != "")
				en++;
		if (en < 5)
		{
			ln = 0;//reset
			continue;
		}
		if (ln == 0)
		{
			if (time_.empty())
			{
				//time index
				for (auto& it : entry)
					time_.push_back(std::stoi(it));
			}
		}
		else
		{
			//speed data
			Sequence seq;
			for (auto& it : entry)
				seq.push_back(std::stod(it));
			data_.add_seq(seq);
		}
		ln++;
	}

	f.close();
}

void WalkCycle::Extract()
{
	if (win_.w < 5)
		return;

	std::vector<Window> wins;
	Window target = win_;
	target.moveto(0);
	while (target.r < data_.length())
	{
		Window win2 = Match(target);
		wins.push_back(win2);
		//update target
		target.moveto(win2.r + 1);
	}

	//correlation
	corr_ = in_corr_;
	//average cycle
	cycle_.mul(in_corr_);
	for (auto& w : wins)
	{
		corr_ += w.c;
		for (size_t i = 0; i < cycle_.size(); ++i)
		{
			for (size_t j = 0; j < cycle_.length(); ++j)
			{
				double val = data_.get(i, w, double(j) / double(cycle_.length() - 1));
				val *= w.c;
				cycle_.inc(i, j, val);
			}
		}
	}
	cycle_.div(corr_);
	in_corr_ = corr_;
}

void WalkCycle::Reset()
{
	cycle_.clear();
	corr_ = 0;
	in_corr_ = 0;
	win_ = Window();
}

void WalkCycle::LoadCycle()
{
	cycle_.clear();
	for (size_t i = 0; i < data_.size(); ++i)
	{
		Sequence seq;
		for (size_t j = win_.l; j <= win_.r; ++j)
		{
			seq.push_back(data_.get(i, j));
		}
		cycle_.add_seq(seq);
	}
}

void WalkCycle::LoadCycle(const std::string& name)
{
	std::ifstream f(name);
	if (!f.good())
		return;

	cycle_.clear();
	std::string line;
	while (std::getline(f, line))
	{
		std::istringstream s(line);
		std::string item;
		std::vector<std::string> entry;
		while (std::getline(s, item, ','))
			entry.push_back(item);
		size_t en = 0;
		for (auto& it : entry)
			if (it != "")
				en++;
		if (en < 2)
		{
			if (en == 1)
			{
				//correlation
				in_corr_ = std::stod(entry[0]);
			}
			continue;
		}
		Sequence seq;
		for (auto& it : entry)
			seq.push_back(std::stod(it));
		cycle_.add_seq(seq);
	}
	f.close();
	win_ = Window(0, cycle_.length() -1);
}

void WalkCycle::SaveCycle(const std::string& name)
{
	std::ofstream f(name);
	if (!f.good())
		return;

	//correlation
	f << corr_ << std::endl;

	for (size_t i = 0; i < cycle_.size(); ++i)
	{
		for (size_t j = 0; j < cycle_.length(); ++j)
		{
			double val = cycle_.get(i, j);
			f << val;
			if (j < cycle_.length() - 1)
				f << ", ";
		}
		f << std::endl;
	}
	f.close();
}

Window WalkCycle::Match(const Window& target)
{
	//init target
	Window low = target;
	low.half();
	Window hi = target;
	hi.onehalf();

	//find correlations
	std::vector<double> corr;
	for (Window i = low; i <= hi; ++i)
	{
		double val = Correlate(i);
		corr.push_back(val);
	}

	//find max correlation
	double max_corr = 0;
	size_t max_ind = 0;
	for (size_t i = 0; i < corr.size(); ++i)
	{
		if (corr[i] > max_corr)
		{
			max_corr = corr[i];
			max_ind = i;
		}
	}
	Window result = target;
	result.resize(low.w + max_ind);
	result.c = max_corr;
	return result;
}

double WalkCycle::Correlate(const Window& win)
{
	double result = 0;
	for (size_t i = 0; i < cycle_.size(); ++i)
	{
		for (size_t j = 0; j < cycle_.length(); ++j)
		{
			double val1 = cycle_.get(i, j);
			double val2 = data_.get(i, win,
				double(j) / double(cycle_.length() - 1));
			result += val1 * val2;
		}
	}
	return result;
}