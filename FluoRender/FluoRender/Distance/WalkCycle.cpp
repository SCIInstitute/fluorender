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
#include <WalkCycle.h>
#include <compatibility.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace flrd;

WalkCycle::WalkCycle() :
	corr_(0),
	in_corr_(0),
	cycle_size_(0)
{

}

WalkCycle::~WalkCycle()
{

}

void WalkCycle::ReadData(const std::wstring& name)
{
	time_.clear();
	names_.clear();
	data_.clear();

#ifdef _WIN32
	std::ifstream f(name);
#else
    std::ifstream f(ws2s(name));
#endif
	if (!f.good())
		return;

	std::string line;
	int ln = 0;
	while (std::getline(f, line))
	{
		std::istringstream s(line);
		std::string item;
		std::vector<std::string> entry;
		while (std::getline(s >> std::ws, item, ','))
			entry.push_back(item);
		size_t en = 0;
		for (auto& it : entry)
			if (it != "")
				en++;
		if (en < 3)
		{
			ln = 0;//reset

			//get names
			if (en == 2)
			{
				if (entry[0] == "correlation")
				{
					//correlation for cycle averaging
				}
				else //point index
				{
					WcName wcn;
					wcn.s = s2ws(entry[1]);
					wcn.n = 0;
					wcn.d = 0;
					names_.push_back(wcn);
				}
			}
			else if (en == 1 && !names_.empty())
			{
				names_.back().n++;
				names_.back().d = 0;
			}
			continue;
		}
		if (ln == 0)
		{
			if (time_.empty())
			{
				//time index
				for (auto& it : entry)
				{
					size_t t;
					try
					{
						t = std::stoi(it);
					}
					catch (...)
					{
						continue;
					}
					time_.push_back(t);
				}
			}
		}
		else
		{
			//speed data
			Sequence seq;
			for (auto& it : entry)
			{
				double v;
				try
				{
					v = std::stod(it);
				}
				catch (...)
				{
					continue;
				}
				seq.push_back(v);
			}
			data_.add_seq(seq);
			if (!names_.empty())
				names_.back().d++;
		}
		ln++;
	}

	f.close();
}

void WalkCycle::SaveData(const std::wstring& name)
{
#ifdef _WIN32
	std::ofstream f(name);
#else
    std::ofstream f(ws2s(name));
#endif
	if (!f.good())
		return;

	size_t cnt = 0;
	size_t length = data_.length();
	for (size_t i = 0; i < names_.size(); ++i)
	{
		//name
		f << i + 1 << ", " << ws2s(names_[i].s) << std::endl;
		for (size_t j = 0; j < names_[i].n; ++j)
		{
			//sn
			f << j << std::endl;
			//time
			for (size_t k = 0; k < time_.size(); ++k)
			{
				f << time_[k];
				if (k < time_.size() - 1)
					f << ", ";
				else
					f << std::endl;
			}
			//values
			for (size_t k = 0; k < names_[i].d; ++k)
			{
				for (size_t l = 0; l < length; ++l)
				{
					double val = data_.get(cnt, l);
					f << val;
					if (l < length - 1)
						f << ", ";
					else
						f << std::endl;
				}
				cnt++;
			}
		}
	}
	f.close();
}

void WalkCycle::Extract(size_t ol)
{
	if (win_.w < 3 + ol)
		return;

	std::vector<Window> wins;
	Window target = win_;
	target.moveto(0);
	while (target.r < data_.length())
	{
		Window win2 = Match(target);
		wins.push_back(win2);
		//update target
		target.moveto(win2.r + 1 - ol);
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

//void WalkCycle::Compare()
//{
//	dist_.clear();
//	if (data_.length() < 1 ||
//		cycle_.length() < 1 ||
//		data_.length() < cycle_.length() ||
//		data_.size() != cycle_.size())
//		return;
//
//	size_t leng = data_.length() - cycle_.length();
//	size_t size = data_.size();
//	size_t width = cycle_.size();
//	dist_.set_size(size);
//	dist_.set_leng(leng);
//	for (size_t i = 0; i < size; ++i)
//	{
//		for (size_t j = 0; j < leng; ++j)
//		{
//			double d = 0;
//			for (size_t k = 0; k < width; ++k)
//			{
//				double v1 = data_.get(i, j + k);
//				double v2 = cycle_.get(i, k);
//				d += (v1 - v2) * (v1 - v2);
//			}
//			dist_.set(i, j, d);
//		}
//	}
//}

void WalkCycle::Compare(size_t ol)
{
	dist_.clear();

	if (win_.w < 3 + ol)
		return;

	std::vector<Window> wins;
	Window target = win_;
	target.moveto(0);
	while (target.r < data_.length())
	{
		Window win2 = Match(target);
		wins.push_back(win2);
		//update target
		target.moveto(win2.r + 1 - ol);
	}

	if (wins.empty())
		return;

	size_t size = data_.size();
	size_t leng = std::min(wins.back().r + 1, data_.length());
	dist_.set_size(size);
	dist_.set_leng(leng);
	Window w2(0, cycle_.length() - 1);
	for (auto& w : wins)
	{
		w2.moveto(0);
		for (size_t i = 0; i < size; ++i)
		{
			double v = 0;
			double v1, v2;
			for (size_t k = 0; k < w.w; ++k)
			{
				v1 = data_.get(i, k + w.l);
				v2 = cycle_.get(i, w2, double(k) / double(w.w - 1));
				v += v1 * v2;
			}
			double c = cycle_.get_auto_corr(i);
			if (c != 0)
				v /= c;
			//log
			v = v > 0.0 ? std::log2(v + 1.0) : -std::log2(1.0 - v);
			for (size_t j = w.l; j <= w.r; ++j)
			{
				dist_.set(i, j, v);
			}
		}
	}
}

void WalkCycle::Align(size_t ol)
{
	if (win_.w < 3 + ol)
		return;

	std::vector<Window> wins;
	Window target = win_;
	target.moveto(0);
	while (target.r < data_.length())
	{
		Window win2 = Match(target);
		wins.push_back(win2);
		//update target
		target.moveto(win2.r + 1 - ol);
	}

	if (wins.empty())
		return;

	size_t size = data_.size();
	size_t leng = wins.size() * (cycle_.length() - ol);
	aligned_.set_size(size);
	aligned_.set_leng(leng);
	Window w2(0, cycle_.length() - 1);
	for (size_t c = 0; c < wins.size(); ++c)
	{
		Window w = wins[c];
		w2.moveto(c * (cycle_.length() - ol));
		for (size_t j = 0; j < w2.w; ++j)
		{
			for (size_t i = 0; i < size; ++i)
			{
				double v = data_.get(i, w, double(j) / double(w2.w - 1));
				aligned_.set(i, w2.l + j, v);
			}
		}
	}
}

void WalkCycle::ComputeVar(size_t ol)
{
	if (win_.w < 3 + ol)
		return;

	std::vector<Window> wins;
	Window target = win_;
	target.moveto(0);
	while (target.r < data_.length())
	{
		Window win2 = Match(target);
		wins.push_back(win2);
		//update target
		target.moveto(win2.r + 1 - ol);
	}

	if (wins.empty())
		return;

	cycle_size_ = wins.size();
	diff2_ = 0;
	size_t size = data_.size();
	size_t leng = std::min(wins.back().r + 1, data_.length());
	Window w2(0, cycle_.length() - 1);
	for (auto& w : wins)
	{
		double diff = 0;
		w2.moveto(0);
		for (size_t i = 0; i < size; ++i)
		{
			double v1, v2;
			for (size_t k = 0; k < w.w; ++k)
			{
				v1 = data_.get(i, k + w.l);
				v2 = cycle_.get(i, w2, double(k) / double(w.w - 1));
				diff += v1 - v2;
			}
		}
		diff2_ += diff * diff;
	}
}

void WalkCycle::ComputeFrames(size_t ol, double d)
{
	if (win_.w < 3 + ol)
		return;

	std::vector<Window> wins;
	Window target = win_;
	target.moveto(0);
	while (target.r < data_.length())
	{
		Window win2 = Match(target);
		wins.push_back(win2);
		//update target
		target.moveto(win2.r + 1 - ol);
	}

	if (wins.empty())
		return;

	cycle_size_ = 0;
	frames_ = 0;
	cycle_speed_ = 0;
	size_t size = cycle_.size();
	size_t n = size / 18;
	size_t leng = std::min(wins.back().r + 1, data_.length());
	Window w2(0, cycle_.length() - 1);
	for (auto& w : wins)
	{
		double diff = 0;
		w2.moveto(0);
		for (size_t i = 0; i < n*7; i=i+n)//first 7 points including head and body
		{
			double v1;
			for (size_t k = 0; k < w.w; ++k)
			{
				v1 = data_.get(i, k + w.l);
				diff += v1;
			}
		}
		double av = std::fabs(diff / (7 * w.w));
		if (av > d)
		{
			cycle_size_++;
			frames_ += w.w - ol;
			cycle_speed_ += av;
		}
	}
}

void WalkCycle::Reset()
{
	cycle_.clear();
	corr_ = 0;
	in_corr_ = 0;
	cycle_size_ = 0;
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

void WalkCycle::LoadCycle(const std::wstring& name)
{
	cycle_.clear();

#ifdef _WIN32
	std::ifstream f(name);
#else
    std::ifstream f(ws2s(name));
#endif
	if (!f.good())
		return;

	std::string line;
	int ln = 0;
	while (std::getline(f, line))
	{
		std::istringstream s(line);
		std::string item;
		std::vector<std::string> entry;
		while (std::getline(s >> std::ws, item, ','))
			entry.push_back(item);
		size_t en = 0;
		for (auto& it : entry)
			if (it != "")
				en++;
		if (en < 3)
		{
			ln = 0;//reset

			if (en == 2)
			{
				if (entry[0] == "correlation")
				{
					//correlation
					in_corr_ = STOD(entry[1]);
				}
			}
			continue;
		}
		if (ln == 0)
		{
			//time
		}
		else
		{
			//speed data
			Sequence seq;
			for (auto& it : entry)
			{
				double v;
				try
				{
					v = std::stod(it);
				}
				catch (...)
				{
					continue;
				}
				seq.push_back(v);
			}
			cycle_.add_seq(seq);
		}
		ln++;
	}

	f.close();
	win_ = Window(0, cycle_.length() -1);
}

void WalkCycle::SaveCycle(const std::wstring& name)
{
#ifdef _WIN32
	std::ofstream f(name);
#else
    std::ofstream f(ws2s(name));
#endif
	if (!f.good())
		return;

	//correlation
	f << "correlation, " << corr_ << std::endl;

	size_t cnt = 0;
	size_t length = cycle_.length();
	for (size_t i = 0; i < names_.size(); ++i)
	{
		//name
		f << i + 1 << ", " << ws2s(names_[i].s) << std::endl;
		for (size_t j = 0; j < names_[i].n; ++j)
		{
			//sn
			f << j << std::endl;
			//time
			for (size_t k = 0; k < length; ++k)
			{
				f << k + 1;
				if (k < length - 1)
					f << ", ";
				else
					f << std::endl;
			}
			//values
			for (size_t k = 0; k < names_[i].d; ++k)
			{
				for (size_t l = 0; l < length; ++l)
				{
					double val = cycle_.get(cnt, l);
					f << val;
					if (l < length - 1)
						f << ", ";
					else
						f << std::endl;
				}
				cnt++;
			}
		}
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

void WalkCycle::Correct(int mode)
{
	if (names_.empty())
		return;
	int dim = static_cast<int>(names_[0].d);
	switch (mode)
	{
	case 0:
		data_.normalize(0, dim);
		if (dim > 1)
			data_.correct(1, dim);
		if (dim > 2)
			data_.correct(2, dim);
	break;
	case 1:
		cycle_.normalize(0, dim);
		if (dim > 1)
			cycle_.correct(1, dim);
		if (dim > 2)
			cycle_.correct(2, dim);
		break;
	}
}

void WalkCycle::SaveDist(const std::wstring& name)
{
#ifdef _WIN32
	std::ofstream f(name);
#else
    std::ofstream f(ws2s(name));
#endif
	if (!f.good())
		return;

	size_t cnt = 0;
	size_t length = dist_.length();
	for (size_t i = 0; i < names_.size(); ++i)
	{
		//name
		f << i + 1 << ", " << ws2s(names_[i].s) << std::endl;
		for (size_t j = 0; j < names_[i].n; ++j)
		{
			//sn
			f << j << std::endl;
			//time
			for (size_t k = 0; k < length; ++k)
			{
				f << time_[k];
				if (k < length - 1)
					f << ", ";
				else
					f << std::endl;
			}
			//values
			for (size_t k = 0; k < names_[i].d; ++k)
			{
				for (size_t l = 0; l < length; ++l)
				{
					double val = dist_.get(cnt, l);
					f << val;
					if (l < length - 1)
						f << ", ";
					else
						f << std::endl;
				}
				cnt++;
			}
		}
	}
	f.close();
}

void WalkCycle::SaveAligned(const std::wstring& name)
{
#ifdef _WIN32
	std::ofstream f(name);
#else
    std::ofstream f(ws2s(name));
#endif
	if (!f.good())
		return;

	size_t cnt = 0;
	size_t length = aligned_.length();
	for (size_t i = 0; i < names_.size(); ++i)
	{
		//name
		f << i + 1 << ", " << ws2s(names_[i].s) << std::endl;
		for (size_t j = 0; j < names_[i].n; ++j)
		{
			//sn
			f << j << std::endl;
			//time
			//for (size_t k = 0; k < length; ++k)
			//{
			//	f << time_[k];
			//	if (k < length - 1)
			//		f << ", ";
			//	else
			//		f << std::endl;
			//}
			//values
			for (size_t k = 0; k < names_[i].d; ++k)
			{
				for (size_t l = 0; l < length; ++l)
				{
					double val = aligned_.get(cnt, l);
					f << val;
					if (l < length - 1)
						f << ", ";
					else
						f << std::endl;
				}
				cnt++;
			}
		}
	}
	f.close();
}
