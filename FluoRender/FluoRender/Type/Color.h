﻿/*
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

#ifndef _FLCOLOR_H_
#define _FLCOLOR_H_

#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

namespace fluo
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
		HSVColor(const std::string& s)
		{
			*this = from_string(s);
		}
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

		inline double hue() const { return hue_; }
		inline double sat() const { return sat_; }
		inline double val() const { return val_; }

		inline std::string to_string() const
		{
			std::ostringstream oss;
			oss << *this;
			return oss.str();
		}

		static HSVColor from_string(const std::string& str) {
			std::istringstream is(str);
			HSVColor c;
			is >> c;
			return c;
		}

		friend std::ostream& operator<<(std::ostream& os, const HSVColor& c)
		{
			os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
			os << '[' << c.hue_ << ',' << c.sat_ << ',' << c.val_ << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, HSVColor& c)
		{
			double hue, sat, val;
			char ch;
			is >> ch;
			if (ch == '[') {
				is >> hue >> ch >> sat >> ch >> val >> ch;
			}
			else {
				is.putback(ch);
				is >> hue >> sat >> val;
			}
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
		Color(const std::string& s);
		Color& operator=(const Color&);
		Color(const HSVColor&);
		Color(unsigned int id, int shuffle = 0);
		~Color();

		Color operator*(const Color&) const;
		Color operator*(double) const;
		Color operator/(double) const;
		Color operator+(const Color&) const;
		Color operator-(const Color&) const;
		Color& operator+=(const Color&);

		inline int operator==(const Color& c) const
		{
			return ((r_ == c.r_) && (g_ == c.g_) && (b_ == c.b_));
		}

		inline int operator!=(const Color& c) const
		{
			return ((r_ != c.r_) || (g_ != c.g_) || (b_ != c.b_));
		}

		void get_color(double color[4]);
		inline double r() const { return r_; }
		inline double g() const { return g_; }
		inline double b() const { return b_; }

		inline void r(const double v) { r_ = v; }
		inline void g(const double v) { g_ = v; }
		inline void b(const double v) { b_ = v; }

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
			double hue = (double)::rand() / (RAND_MAX) * 360.0;
			Color color(HSVColor(hue, 1.0, 1.0));
			r_ = color.r();
			g_ = color.g();
			b_ = color.b();
		}

		inline std::string to_string() const
		{
			std::ostringstream oss;
			oss << *this;
			return oss.str();
		}

		static Color from_string(const std::string& str) {
			std::istringstream is(str);
			Color c;
			is >> c;
			return c;
		}

		friend std::ostream& operator<<(std::ostream& os, const Color& c)
		{
			os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
			os << '[' << c.r_ << ',' << c.g_ << ',' << c.b_ << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Color& c)
		{
			double r, g, b;
			char ch;
			is >> ch;
			if (ch == '[') {
				is >> r >> ch >> g >> ch >> b >> ch;
			}
			else {
				is.putback(ch);
				is >> r >> g >> b;
			}
			c = Color(r, g, b);
			return is;
		}

		friend class HSVColor;
	};

	class Colorub
	{ // unsigned byte color
		unsigned char data[3]; // data...
	public:
		Colorub() {};
		Colorub(Color& c)
		{
			data[0] = (unsigned char)(c.r() * 255);
			data[1] = (unsigned char)(c.g() * 255);
			data[2] = (unsigned char)(c.b() * 255);
		}; // converts them...

		unsigned char* ptr() { return &data[0]; }; // grab pointer

		inline unsigned char r() const { return data[0]; };
		inline unsigned char g() const { return data[1]; };
		inline unsigned char b() const { return data[2]; };

		// should be enough for now - this is less bandwidth...
	};

	/*********************************************************
	This structure holds a simple RGB color in char format.
	*********************************************************/

	class CharColor
	{
	public:
		char red;
		char green;
		char blue;
		// char alpha;

		CharColor();
		CharColor(char a, char b, char c);
		CharColor(Color& c);

		inline double r() const { return red; }
		inline double g() const { return green; }
		inline double b() const { return blue; }

		CharColor operator= (const Color&);
		CharColor operator= (const CharColor&);

		int operator!= (const CharColor&) const;

	};

} // End namespace fluo

#endif//_FLCOLOR_H_
