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
#ifndef _WALKCYCLE_H_
#define _WALKCYCLE_H_

#include <vector>
#include <string>
#include <algorithm>

namespace flrd
{
	class Window
	{
	public:
		size_t l;//left index, data point included
		size_t r;//right index, data point included
		size_t w;//width = r - l + 1;
		double c;//correlation value
		Window() : l(0), r(0), w(1), c(0) {}
		Window(size_t v1, size_t v2) : l(v1), r(v2), c(0)
		{
			w = r - l + 1;
		}
		~Window() {}

		Window& operator=(const Window& win)
		{
			l = win.l;
			r = win.r;
			w = win.w;
			return *this;
		}

		int operator==(const Window& win) const
		{
			return win.l == l && win.r == r && win.w == w;
		}

		bool operator<(const Window& win) const
		{
			return r < win.r;
		}

		bool operator<=(const Window& win) const
		{
			return r <= win.r;
		}

		void operator++()
		{
			r = r + 1;
			w = w + 1;
		}

		void operator++(int)
		{
			r = r + 1;
			w = w + 1;
		}

		void move(size_t d)
		{
			l += d;
			r += d;
		}
		void moveto(size_t o)
		{
			l = o;
			r = l + w - 1;
		}
		void half()
		{
			double dw = w;
			dw /= 2;
			w = size_t(dw + 0.55);
			w = w ? w : 1;
			r = l + w - 1;
		}
		void onehalf()
		{
			double dw = w;
			dw /= 2;
			w = size_t(dw + w);
			w = w ? w : 1;
			r = l + w - 1;
		}
		void resize(size_t i)
		{
			w = i;
			r = l + w - 1;
		}
	};

	class Sequence : public std::vector<double>
	{
	public:
		double get(const Window& win, double j)
		{
			double w = win.w - 1;
			double p = win.l + w * j;
			size_t i0 = size_t(p);
			size_t i1 = i0 + 1;
			p -= i0;
			if (i0 < size() && i1 < size())
				return at(i0) * (1 - p) + at(i1) * p;
			return 0;
		}
		void correct()
		{
			if (empty())
				return;
			double d = sum() / size();
			std::transform(begin(), end(), begin(),
				[d](double& c) { return c - d; });
		}
		double sum()
		{
			double v = 0;
			for (auto i : *this)
				v += i;
			return v;
		}
	};

	class WalkData
	{
	public:
		WalkData() : length_(0), auto_corr_(0) {}
		WalkData(WalkData& d) : length_(d.length_), auto_corr_(d.auto_corr_), data_(d.data_) {}
		~WalkData() {}

		size_t length() { return length_; }
		size_t size() { return data_.size(); }

		void set_size(size_t s)
		{
			data_.resize(s);
		}
		void set_leng(size_t l)
		{
			for (auto& i : data_)
				i.resize(l, 0);
			length_ = l;
			auto_corr_ = 0;
		}
		void add_seq(Sequence& seq)
		{
			length_ = std::max(length_, seq.size());
			if (seq.size() != length_)
				seq.resize(length_);
			data_.push_back(seq);
		}

		void clear()
		{
			data_.clear();
			length_ = 0;
			auto_corr_ = 0;
		}
		void zero()
		{
			for (auto& i : data_)
				std::fill(i.begin(), i.end(), 0);
		}
		WalkData& operator+=(const WalkData& d)
		{
			for (size_t i = 0; i < data_.size(); ++i)
				for (size_t j = 0; j < data_[i].size(); ++j)
				{
					if (i >= d.data_.size())
						continue;
					if (j >= d.data_[i].size())
						continue;
					data_[i][j] += d.data_[i][j];
				}
			return *this;
		}
		void div(double d)
		{
			if (d == 0)
				return;
			for (auto& i : data_)
				std::transform(i.begin(), i.end(), i.begin(),
					[d](double& c) { return c / d; });
		}
		void mul(double d)
		{
			for (auto& i : data_)
				std::transform(i.begin(), i.end(), i.begin(),
					[d](double& c) { return c * d; });
		}
		double get(size_t i, size_t j)
		{
			if (i < data_.size() && j < length_)
				return data_[i][j];
			return 0;
		}
		void set(size_t i, size_t j, double v)
		{
			if (i < data_.size() && j < length_)
				data_[i][j] = v;
		}
		double get(size_t i, const Window& win, double j)
		{
			if (i < data_.size() && j >= 0 && j <= 1)
				return data_[i].get(win, j);
			return 0;
		}
		void inc(size_t i, size_t j, double v)
		{
			if (i < data_.size() && j < length_)
				data_[i][j] += v;
		}
		double get_auto_corr()
		{
			if (auto_corr_ > 0)
				return auto_corr_;
			for (auto& i : data_)
				for (auto& j : i)
					auto_corr_ += j * j;
			return auto_corr_;
		}
		double get_auto_corr(size_t i)//correlation of ith row
		{
			if (i >= size())
				return 0;
			double corr = 0;
			for (auto& j : data_[i])
				corr += j * j;
			return corr;
		}

		void correct(size_t x, size_t n)
		{
			for (size_t i = 0; i < data_.size(); ++i)
			{
				if (x == i % n)
					data_[i].correct();
			}
		}
		void normalize(size_t x, size_t n)
		{
			double maxv = 0;
			for (size_t i = 0; i < data_.size(); ++i)
			{
				if (x == i % n)
					maxv = std::max(maxv, data_[i].sum());
			}
			for (size_t i = 0; i < data_.size(); ++i)
			{
				if (x == i % n)
				{
					double d = data_[i].sum();
					d = maxv / d;
					std::transform(data_[i].begin(), data_[i].end(), data_[i].begin(),
						[d](double& c) { return c * d; });
				}
			}
		}

	private:
		size_t length_;
		double auto_corr_;//auto correlation
		std::vector<Sequence> data_;
	};

	struct WcName
	{
		std::string s;
		size_t n;
		size_t d;
	};

	class WalkCycle
	{
	public:
		WalkCycle();
		~WalkCycle();

		void ReadData(const std::string& name);
		void SaveData(const std::string& name);
		WalkData& GetData()
		{
			return data_;
		}
		void Extract();
		void Reset();//reset for win change not the data
		void Compare();//compare data with cycle

		void SetInitWin(Window& win)
		{
			win_ = win;
		}
		Window GetWin()
		{
			return win_;
		}
		void LoadCycle();//from data
		void LoadCycle(const std::string& name);//from file
		void SaveCycle(const std::string& name);

		WalkData GetCycle()
		{
			return cycle_;
		}
		double GetCorr()
		{
			return corr_;
		}
		double GetNormalizedCorr()
		{
			return corr_ / cycle_.get_auto_corr();
		}

		void GetNames(std::vector<WcName>& names)
		{
			//std::copy(names_.begin(), names_.end(), names);
			if (!names.empty())
				names.clear();
			for (auto& i : names_)
			{
				WcName name;
				name.s = i.s;
				name.d = i.d;
				name.n = i.n;
				names.push_back(name);
			}
		}

		void Correct(int mode);//0: data, 1: cycle

		void SaveDist(const std::string& name);

	private:
		std::vector<size_t> time_;
		std::vector<WcName> names_;//ruler names and point size
		WalkData data_;

		Window win_;
		WalkData cycle_;
		double corr_;//sum correlation
		double in_corr_;//input correlation if cycle is read from file

		WalkData dist_;//distance between data and cycle from compare

	private:
		Window Match(const Window& target);
		double Correlate(const Window& win);
	};
}

#endif//_WALKCYCLE_H_