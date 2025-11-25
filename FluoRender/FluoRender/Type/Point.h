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

#ifndef _FLPOINT_H_
#define _FLPOINT_H_

#include <string>
#include <iosfwd>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fluo {
	using std::string;

	class Vector;

	class Point {
		double x_, y_, z_;
	public:
		inline explicit Point(const Vector& v);
		inline Point(double x, double y, double z) : x_(x), y_(y), z_(z) {}
		inline Point(double v) : x_(v), y_(v), z_(v) {}
		Point(double, double, double, double);
		Point(const Point&);
		Point(const std::string& s);
		inline Point();
		inline int operator==(const Point&) const;
		inline int operator!=(const Point&) const;
		inline bool operator<(const Point&) const;
		inline bool operator>(const Point&) const;
		inline bool operator<=(const Point&) const;
		inline bool operator>=(const Point&) const;
		inline Point& operator=(const Point&);
		inline Vector operator+(const Point&) const;
		inline Vector operator-(const Point&) const;
		inline Point operator+(const Vector&) const;
		inline Point operator-(const Vector&) const;
		inline Point operator*(double) const;
		inline Point& operator*=(const double);
		inline Point& operator+=(const Vector&);
		inline Point& operator-=(const Vector&);
		inline Point& operator+=(const Point&);
		inline Point& operator-=(const Point&);
		inline Point& operator/=(const double);
		inline Point operator/(const double) const;
		inline Point operator-() const;
		inline double& operator()(int idx);
		inline double operator()(int idx) const;
		inline void addscaled(const Point& p, const double scale);  // this += p * w;
		inline void scale(const double sx, const double sy, const double sz);
		inline void x(const double);
		inline double x() const;
		inline void y(const double);
		inline double y() const;
		inline void z(const double);
		inline double z() const;
		inline int intx() const;
		inline int inty() const;
		inline int intz() const;
		inline const Vector &vector() const;
		inline Vector &asVector() const;

		inline Point unit_sign() const;

		inline string get_string() const;

		friend class Vector;
		friend inline double Dot(const Point&, const Point&);
		friend inline double Dot(const Vector&, const Point&);
		friend inline double Dot(const Point&, const Vector&);

		friend inline Point Min(const Point&, const Point&);
		friend inline Point Max(const Point&, const Point&);

		friend Point AffineCombination(const Point&, double,
			const Point&, double,
			const Point&, double,
			const Point&, double);
		friend Point AffineCombination(const Point&, double,
			const Point&, double,
			const Point&, double);
		friend Point AffineCombination(const Point&, double,
			const Point&, double);

		friend inline Point Interpolate(const Point&, const Point&, double);

		// is one point within a small interval of another?
		int Overlap(double a, double b, double e);
		int InInterval(Point a, double epsilon);

		inline void Set(double x, double y, double z)
		{
			x_ = x;
			y_ = y;
			z_ = z;
		}

		inline std::string to_string() const
		{
			std::ostringstream oss;
			oss << *this;
			return oss.str();
		}

		static Point from_string(const std::string& str) {
			std::istringstream is(str);
			Point p;
			is >> p;
			return p;
		}

		friend std::ostream& operator<<(std::ostream& os, const Point& p)
		{
			os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
			os << '[' << p.x() << ',' << p.y() << ',' << p.z() << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Point& p)
		{
			double x, y, z;
			char ch;
			is >> ch;
			if (ch == '[') {
				is >> x >> ch >> y >> ch >> z >> ch;
			}
			else {
				is.putback(ch);
				is >> x >> y >> z;
			}
			p = Point(x, y, z);
			return is;
		}

	}; // end class Point

	// Actual declarations of these functions (as 'friend' above doesn't
	// (depending on the compiler) actually declare them.
	Point AffineCombination(const Point&, double, const Point&, double,
		const Point&, double, const Point&, double);
	Point AffineCombination(const Point&, double, const Point&, double,
		const Point&, double);
	Point AffineCombination(const Point&, double, const Point&, double);

	inline Point operator*(double d, const Point &p) {
		return p*d;
	}
	inline Point operator+(const Vector &v, const Point &p) {
		return p + v;
	}

} // End namespace fluo

// This cannot be above due to circular dependencies
#include <Vector.h>

namespace fluo
{
	inline Point::Point(const Vector& v)
		: x_(v.x_), y_(v.y_), z_(v.z_)
	{
	}

	inline Point::Point(const Point& p)
	{
		x_ = p.x_;
		y_ = p.y_;
		z_ = p.z_;
	}

	inline Point::Point()
	{
		x_ = y_ = z_ = 0.0;
	}

	inline int Point::operator==(const Point& p) const
	{
		return p.x_ == x_ && p.y_ == y_ && p.z_ == z_;
	}

	inline int Point::operator!=(const Point& p) const
	{
		return p.x_ != x_ || p.y_ != y_ || p.z_ != z_;
	}

	inline bool Point::operator<(const Point& p) const
	{
		if (x_ < p.x_ &&
			y_ < p.y_ &&
			z_ < p.z_)
			return true;
		else
			return false;
	}

	inline bool Point::operator>(const Point& p) const
	{
		if (x_ > p.x_ &&
			y_ > p.y_ &&
			z_ > p.z_)
			return true;
		else
			return false;
	}

	inline bool Point::operator<=(const Point& p) const
	{
		if (x_ <= p.x_ &&
			y_ <= p.y_ &&
			z_ <= p.z_)
			return true;
		else
			return false;
	}

	inline bool Point::operator>=(const Point& p) const
	{
		if (x_ >= p.x_ &&
			y_ >= p.y_ &&
			z_ >= p.z_)
			return true;
		else
			return false;
	}

	inline Point& Point::operator=(const Point& p)
	{
		x_ = p.x_;
		y_ = p.y_;
		z_ = p.z_;
		return *this;
	}

	inline Vector Point::operator+(const Point& p) const
	{
		return Vector(x_ + p.x_, y_ + p.y_, z_ + p.z_);
	}

	inline Vector Point::operator-(const Point& p) const
	{
		return Vector(x_ - p.x_, y_ - p.y_, z_ - p.z_);
	}

	inline Point Point::operator+(const Vector& v) const
	{
		return Point(x_ + v.x_, y_ + v.y_, z_ + v.z_);
	}

	inline Point Point::operator-(const Vector& v) const
	{
		return Point(x_ - v.x_, y_ - v.y_, z_ - v.z_);
	}

	inline Point& Point::operator+=(const Vector& v)
	{
		x_ += v.x_;
		y_ += v.y_;
		z_ += v.z_;
		return *this;
	}

	inline Point& Point::operator-=(const Vector& v)
	{
		x_ -= v.x_;
		y_ -= v.y_;
		z_ -= v.z_;
		return *this;
	}

	inline Point& Point::operator+=(const Point& v)
	{
		x_ += v.x_;
		y_ += v.y_;
		z_ += v.z_;
		return *this;
	}

	inline Point& Point::operator-=(const Point& v)
	{
		x_ -= v.x_;
		y_ -= v.y_;
		z_ -= v.z_;
		return *this;
	}


	inline Point& Point::operator*=(const double d)
	{
		x_ *= d;
		y_ *= d;
		z_ *= d;
		return *this;
	}

	inline Point& Point::operator/=(const double d)
	{
		x_ /= d;
		y_ /= d;
		z_ /= d;
		return *this;
	}

	inline Point Point::operator-() const
	{
		return Point(-x_, -y_, -z_);
	}

	inline Point Point::operator*(double d) const
	{
		return Point(x_*d, y_*d, z_*d);
	}

	inline Point Point::operator/(const double d) const
	{
		return Point(x_ / d, y_ / d, z_ / d);
	}

	inline double& Point::operator()(int idx)
	{
		return (&x_)[idx];
	}

	inline double Point::operator()(int idx) const
	{
		return (&x_)[idx];
	}

	inline Point Interpolate(const Point& v1, const Point& v2,
		double weight)
	{
		double weight1 = 1.0 - weight;
		return Point(v2.x_*weight + v1.x_*weight1,
			v2.y_*weight + v1.y_*weight1,
			v2.z_*weight + v1.z_*weight1);
	}

	inline void Point::addscaled(const Point& p, const double scale)
	{
		// this += p * w;
		x_ += p.x_ * scale;
		y_ += p.y_ * scale;
		z_ += p.z_ * scale;
	}

	inline void Point::scale(const double sx, const double sy, const double sz)
	{
		x_ *= sx;
		y_ *= sy;
		z_ *= sz;
	}

	inline void Point::x(const double d)
	{
		x_ = d;
	}

	inline double Point::x() const
	{
		return x_;
	}

	inline void Point::y(const double d)
	{
		y_ = d;
	}

	inline double Point::y() const
	{
		return y_;
	}

	inline void Point::z(const double d)
	{
		z_ = d;
	}

	inline double Point::z() const
	{
		return z_;
	}

	inline int Point::intx() const
	{
		return static_cast<int>(std::round(x_));
	}

	inline int Point::inty() const
	{
		return static_cast<int>(std::round(y_));
	}

	inline int Point::intz() const
	{
		return static_cast<int>(std::round(z_));
	}

	inline Point Point::unit_sign() const
	{
		return Point(
			(x_ > 0) - (x_ < 0),
			(y_ > 0) - (y_ < 0),
			(z_ > 0) - (z_ < 0));
	}

	inline const Vector &Point::vector() const
	{
		return (const Vector &)(*this);
	}

	inline Vector &Point::asVector() const
	{
		return (Vector &)(*this);
	}

	inline Point Min(const Point& p1, const Point& p2)
	{
		double x = std::min(p1.x_, p2.x_);
		double y = std::min(p1.y_, p2.y_);
		double z = std::min(p1.z_, p2.z_);
		return Point(x, y, z);
	}

	inline Point Max(const Point& p1, const Point& p2)
	{
		double x = std::max(p1.x_, p2.x_);
		double y = std::max(p1.y_, p2.y_);
		double z = std::max(p1.z_, p2.z_);
		return Point(x, y, z);
	}

	inline double Dot(const Point& p, const Vector& v)
	{
		return p.x_*v.x_ + p.y_*v.y_ + p.z_*v.z_;
	}

	inline double Dot(const Point& p1, const Point& p2)
	{
		return p1.x_*p2.x_ + p1.y_*p2.y_ + p1.z_*p2.z_;
	}

} // End namespace fluo

#endif//_FLPOINT_H_
