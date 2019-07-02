/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef FLCOLOR_HPP
#define FLCOLOR_HPP

#include <stdlib.h>
#include <iostream>

namespace FLTYPE
{
	class Color;
	class HSVColor
	{
		double hue_;
		double sat_;
		double val_;
	public:
		HSVColor();
		HSVColor(double hue, double sat, double val);
		~HSVColor();
		HSVColor(const HSVColor&);
		HSVColor(const Color&);
		HSVColor& operator=(const HSVColor&);

		inline int operator==(const HSVColor& c) const
		{
			return ((hue_ == c.hue_) && (sat_ == c.sat_) && (val_ == c.val_));
		}

		inline int operator!=(const HSVColor& c) const
		{
			return ((hue_ != c.hue_) || (sat_ != c.sat_) || (val_ != c.val_));
		}

		// These only affect hue.
		HSVColor operator*(double);
		HSVColor operator+(const HSVColor&);

		inline double& operator[](const int i)
		{
			switch (i)
			{
			case 0:
				return hue_;
			case 1:
				return sat_;
			default:
				return val_;
			}
		}

		inline double hue() const {return hue_;}
		inline double sat() const {return sat_;}
		inline double val() const {return val_;}

		inline void hue(const double v) { hue_ = v; }
		inline void sat(const double v) { sat_ = v; }
		inline void val(const double v) { val_ = v; }

		friend std::ostream& operator<<(std::ostream& os, const HSVColor& c)
		{
			os << '[' << c.hue_ << ',' << c.sat_ << ',' << c.val_ << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, HSVColor& c)
		{
			double hue, sat, val;
			char st;
			is >> st >> hue >> st >> sat >> st >> val >> st;
			c = HSVColor(hue, sat, val);
			return is;
		}

		friend class Color;
	};

	class Color
	{
	protected:
		double r_, g_, b_;
	public:
		Color();
		Color(double, double, double);
		Color(double);
		Color(double[3]);
		Color(const Color&);
		Color& operator=(const Color&);
		Color(const HSVColor&);
		~Color();

		Color operator*(const Color&) const;
		Color operator*(double) const;
		Color operator/(double) const;
		Color operator+(const Color&) const;
		Color operator-(const Color&) const;
		Color& operator+=(const Color&);

		inline int operator==(const Color& c) const
		{
			return ((r_==c.r_)&&(g_==c.g_)&&(b_==c.b_));
		}

		inline int operator!=(const Color& c) const
		{
			return ((r_ != c.r_)||(g_!=c.g_)||(b_!=c.b_));
		}

		void get_color(double color[4]);
		inline double r() const {return r_;}
		inline double g() const {return g_;}
		inline double b() const {return b_;}

		inline void r( const double v ) { r_ = v; }
		inline void g( const double v ) { g_ = v; }
		inline void b( const double v ) { b_ = v; }

		inline double& operator[](int i)
		{
			switch (i)
			{
			case 0:
				return r_;
			case 1:
				return g_;
			default:
				return b_;
			}
		}
		inline const double& operator[](int i) const
		{
			switch (i)
			{
			case 0:
				return r_;
			case 1:
				return g_;
			default:
				return b_;
			}
		}

		inline void rand()
		{
			double hue = (double)::rand()/(RAND_MAX) * 360.0;
			Color color(HSVColor(hue, 1.0, 1.0));
			r_ = color.r();
			g_ = color.g();
			b_ = color.b();
		}

		friend std::ostream& operator<<(std::ostream& os, const Color& c)
		{
			os << '[' << c.r_ << ',' << c.g_ << ',' << c.b_ << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Color& c)
		{
			double r, g, b;
			char st;
			is >> st >> r >> st >> g >> st >> b >> st;
			c = Color(r, g, b);
			return is;
		}

		friend class HSVColor;
	};

} // End namespace FLTYPE

#endif//_FLCOLOR_H_
