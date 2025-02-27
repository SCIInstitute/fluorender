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

#include <Vector.h>
#include <Point.h>
#include <assert.h>
#include <iostream>

using std::istream;
using std::ostream;

using namespace fluo;

inline Vector::Vector(const Point& p)
	: x_(p.x_), y_(p.y_), z_(p.z_)
{
}

inline Vector::Vector()
{
	x_ = y_ = z_ = 0.0;
}

inline Vector::Vector(const Vector& p)
{
	x_ = p.x_;
	y_ = p.y_;
	z_ = p.z_;
}

inline double Vector::length2() const
{
	return x_ * x_ + y_ * y_ + z_ * z_;
}

inline Vector& Vector::operator=(const Vector& v)
{
	x_ = v.x_;
	y_ = v.y_;
	z_ = v.z_;
	return *this;
}

// for initializing in dynamic code
// one often want template<class T> T val = 0.0;

inline Vector& Vector::operator=(const double& d)
{
	x_ = d;
	y_ = d;
	z_ = d;
	return *this;
}

inline Vector& Vector::operator=(const int& d)
{
	x_ = static_cast<int>(d);
	y_ = static_cast<int>(d);
	z_ = static_cast<int>(d);
	return *this;
}

inline bool operator<(Vector v1, Vector v2)
{
	return(v1.length() < v2.length());
}

inline bool operator<=(Vector v1, Vector v2)
{
	return(v1.length() <= v2.length());
}

inline bool operator>(Vector v1, Vector v2)
{
	return(v1.length() > v2.length());
}

inline bool operator>=(Vector v1, Vector v2)
{
	return(v1.length() >= v2.length());
}

inline Vector Vector::operator*(const double s) const
{
	return Vector(x_ * s, y_ * s, z_ * s);
}

inline Vector& Vector::operator*=(const Vector& v)
{
	x_ *= v.x_;
	y_ *= v.y_;
	z_ *= v.z_;
	return *this;
}

inline Vector& Vector::operator/=(const Vector& v)
{
	x_ /= v.x_;
	y_ /= v.y_;
	z_ /= v.z_;
	return *this;
}

// Allows for double * Vector so that everything doesn't have to be
// Vector * double
inline Vector operator*(const double s, const Vector& v)
{
	return v * s;
}

inline Vector Vector::operator/(const double d) const
{
	return Vector(x_ / d, y_ / d, z_ / d);
}

inline Vector Vector::operator/(const Vector& v2) const
{
	return Vector(x_ / v2.x_, y_ / v2.y_, z_ / v2.z_);
}

inline Vector Vector::operator+(const Vector& v2) const
{
	return Vector(x_ + v2.x_, y_ + v2.y_, z_ + v2.z_);
}

inline Vector Vector::operator*(const Vector& v2) const
{
	return Vector(x_ * v2.x_, y_ * v2.y_, z_ * v2.z_);
}

inline Vector Vector::operator-(const Vector& v2) const
{
	return Vector(x_ - v2.x_, y_ - v2.y_, z_ - v2.z_);
}

inline Vector Vector::operator-(const Point& v2) const
{
	return Vector(x_ - v2.x_, y_ - v2.y_, z_ - v2.z_);
}

inline Vector& Vector::operator+=(const Vector& v2)
{
	x_ += v2.x_;
	y_ += v2.y_;
	z_ += v2.z_;
	return *this;
}

inline Vector& Vector::operator-=(const Vector& v2)
{
	x_ -= v2.x_;
	y_ -= v2.y_;
	z_ -= v2.z_;
	return *this;
}

inline Vector Vector::operator-() const
{
	return Vector(-x_, -y_, -z_);
}

inline double Vector::length() const
{
	return sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

inline Vector& Vector::operator*=(const double d)
{
	x_ *= d;
	y_ *= d;
	z_ *= d;
	return *this;
}

inline Vector& Vector::operator/=(const double d)
{
	x_ /= d;
	y_ /= d;
	z_ /= d;
	return *this;
}

inline void Vector::x(double d)
{
	x_ = d;
}

inline double Vector::x() const
{
	return x_;
}

inline void Vector::y(double d)
{
	y_ = d;
}

inline double Vector::y() const
{
	return y_;
}

inline void Vector::z(double d)
{
	z_ = d;
}

inline double Vector::z() const
{
	return z_;
}

inline int Vector::intx() const
{
	return int(std::round(x_));
}

inline int Vector::inty() const
{
	return int(std::round(y_));
}

inline int Vector::intz() const
{
	return int(std::round(z_));
}

inline Vector Vector::unit_sign() const
{
	return Vector(
		(x_ > 0) - (x_ < 0),
		(y_ > 0) - (y_ < 0),
		(z_ > 0) - (z_ < 0));
}

inline double Vector::volume() const
{
	return std::abs(x_ * y_ * z_);
}

inline int Vector::min() const
{
	if (x_ <= y_ && x_ <= z_)
		return 0;
	if (y_ <= x_ && y_ <= z_)
		return 1;
	if (z_ <= x_ && z_ <= y_)
		return 2;
	return 0;
}

inline int Vector::max() const
{
	if (z_ >= x_ && z_ >= y_)
		return 2;
	if (y_ >= x_ && y_ >= z_)
		return 1;
	if (x_ >= y_ && x_ >= z_)
		return 0;
	return 2;
}

inline int Vector::mid() const
{
	int imin = min();
	int imax = max();
	for (int i = 0; i < 3; ++i)
	{
		if (i != imin && i != imax)
			return i;
	}
	return 1;
}

inline void Vector::u(double d)
{
	x_ = d;
}

inline double Vector::u() const
{
	return x_;
}

inline void Vector::v(double d)
{
	y_ = d;
}

inline double Vector::v() const
{
	return y_;
}

inline void Vector::w(double d)
{
	z_ = d;
}

inline double Vector::w() const
{
	return z_;
}

inline
double Vector::normalize()
{
	double l = sqrt(x_ * x_ + y_ * y_ + z_ * z_);
	if (l > 0.0)
	{
		x_ /= l;
		y_ /= l;
		z_ /= l;
	}
	return l;
}

inline
double Vector::safe_normalize()
{
	double l = sqrt(x_ * x_ + y_ * y_ + z_ * z_);
	if (l > 0.0)
	{
		x_ /= l;
		y_ /= l;
		z_ /= l;
	}
	return l;
}

inline const Point& Vector::point() const
{
	return (const Point&)(*this);
}

inline Point& Vector::asPoint() const
{
	return (Point&)(*this);
}

inline Vector Min(const Vector& v1, const Vector& v2)
{
	return Vector(std::min(v1.x(), v2.x()),
		std::min(v1.y(), v2.y()),
		std::min(v1.z(), v2.z()));
}

inline Vector Max(const Vector& v1, const Vector& v2)
{
	return Vector(std::max(v1.x(), v2.x()),
		std::max(v1.y(), v2.y()),
		std::max(v1.z(), v2.z()));
}

inline double Max(const Vector& v)
{
	return Max(v.x(), v.y(), v.z());
}

inline double Min(const Vector& v)
{
	return Min(v.x(), v.y(), v.z());
}

string Vector::get_string() const
{
	char buf[100];
	sprintf(buf, "[%10.4g, %10.4g, %10.4g]", x_, y_, z_);
	return buf;
}

void Vector::find_orthogonal(Vector& v1, Vector& v2) const
{
	Vector v0(Cross(*this, Vector(1, 0, 0)));
	if (v0.length2() == 0)
	{
		v0 = Cross(*this, Vector(0, 1, 0));
	}
	v1 = Cross(*this, v0);
	v1.safe_normalize();
	v2 = Cross(*this, v1);
	v2.safe_normalize();
}

bool Vector::check_find_orthogonal(Vector& v1, Vector& v2) const
{
	Vector v0(Cross(*this, Vector(1, 0, 0)));
	if (v0.length2() == 0) {
		v0 = Cross(*this, Vector(0, 1, 0));
	}
	v1 = Cross(*this, v0);
	double length1 = v1.length();
	if (length1 == 0)
		return false;
	v1 *= 1. / length1;
	v2 = Cross(*this, v1);
	double length2 = v2.length();
	if (length2 == 0)
		return false;
	v2 *= 1. / length2;
	return true;
}

Vector Vector::normal() const
{
	Vector v(*this);
	v.normalize();
	return v;			// 
}

Vector Vector::safe_normal() const
{
	Vector v(*this);
	v.safe_normalize();
	return v;			// 
}

int Vector::operator== (const Vector& v) const
{
	return v.x_ == x_ && v.y_ == y_ && v.z_ == z_;
}

int Vector::operator!=(const Vector& v) const
{
	return v.x_ != x_ || v.y_ != y_ || v.z_ != z_;
}

void Vector::rotz90(const int c)
{
	// Rotate by c*90 degrees counter clockwise
	switch (c % 4)
	{
	case 0:
		// 0 degrees, do nothing
		break;
	case 1:
		// 90 degrees
	{
		double newx = -y_;
		y_ = x_;
		x_ = newx;
	}
	break;
	case 2:
		// 180 degrees
		x_ = -x_;
		y_ = -y_;
		break;
	case 3:
		// 270 degrees
	{
		double newy = -x_;
		x_ = y_;
		y_ = newy;
	}
	break;
	}
}



