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

#ifndef _FLVECTOR_H_
#define _FLVECTOR_H_

#include <Utils.h>
#include <string>
#include <iosfwd>
#include <math.h>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <sstream>

namespace fluo
{
	using std::string;

	class Point;

	class Vector
	{
		double x_, y_, z_;
	public:
		inline explicit Vector(const Point&);
		inline Vector(double x, double y, double z) : x_(x), y_(y), z_(z)
		{ }
		inline Vector(const Vector&);
		inline Vector();
		inline explicit Vector(double init) : x_(init), y_(init), z_(init) {}
		inline Vector(const std::string& s)
		{
			*this = from_string(s);
		}
		inline double length() const;
		inline double length2() const;
		friend inline double Dot(const Vector&, const Vector&);
		friend inline double Dot(const Point&, const Vector&);
		friend inline double Dot(const Vector&, const Point&);
		inline Vector& operator=(const Vector&);
		inline Vector& operator=(const double&);
		inline Vector& operator=(const int&);

		//Note vector[0]=vector.x();vector[1]=vector.y();vector[2]=vector.z()
		inline double& operator[](int idx)
		{
			// Ugly, but works
			return (&x_)[idx];
		}

		//Note vector[0]=vector.x();vector[1]=vector.y();vector[2]=vector.z()
		inline double operator[](int idx) const
		{
			// Ugly, but works
			return (&x_)[idx];
		}

		// checks if one vector is exactly the same as another
		int operator==(const Vector&) const;
		int operator!=(const Vector&) const;

		inline Vector operator*(const double) const;
		inline Vector operator*(const Vector&) const;
		inline Vector& operator*=(const double);
		inline Vector& operator*=(const Vector&);
		inline Vector operator/(const double) const;
		inline Vector operator/(const Vector&) const;
		inline Vector& operator/=(const double);
		inline Vector& operator/=(const Vector&);
		inline Vector operator+(const Vector&) const;
		inline Vector& operator+=(const Vector&);
		inline Vector operator-() const;
		inline Vector operator-(const Vector&) const;
		inline Vector operator-(const Point&) const;
		inline Vector& operator-=(const Vector&);
		inline double normalize();
		inline double safe_normalize();
		Vector normal() const;
		Vector safe_normal() const;
		friend inline Vector Cross(const Vector&, const Vector&);
		friend inline Vector Abs(const Vector&);
		inline void x(double);
		inline double x() const;
		inline void y(double);
		inline double y() const;
		inline void z(double);
		inline double z() const;
		inline int intx() const;
		inline int inty() const;
		inline int intz() const;

		inline Vector unit_sign() const;

		inline double volume() const;//product of all
		inline int min() const;//index to min, least index if equal
		inline int max() const;//index to max, last index if equal
		inline int mid() const;//index to mid, mid index if equal

		inline void u(double);
		inline double u() const;
		inline void v(double);
		inline double v() const;
		inline void w(double);
		inline double w() const;

		void rotz90(const int);

		string get_string() const;


		friend class Point;
		friend class Transform;

		friend inline Vector Interpolate(const Vector&, const Vector&, double);

		void find_orthogonal(Vector&, Vector&) const;
		bool check_find_orthogonal(Vector&, Vector&) const;

		inline const Point &point() const;
		inline Point &asPoint() const;
		inline double minComponent() const
		{
			if (x_ < y_)
			{
				if (x_ < z_)
					return x_;
				else
					return z_;
			}
			else
			{
				if (y_ < z_)
					return y_;
				else
					return z_;
			}
		}
		inline double maxComponent() const
		{
			if (x_ > y_)
			{
				if (x_ > z_)
					return x_;
				else
					return z_;
			}
			else
			{
				if (y_ > z_)
					return y_;
				else
					return z_;
			}
		}

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
		}

		static Vector from_string(const std::string& str) {
			std::istringstream is(str);
			Vector v;
			is >> v;
			return v;
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector& v)
		{
			//avoid using spaces so that it can be read correctly using >>
			os << '[' << v.x() << ',' << v.y() << ',' << v.z() << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Vector& v)
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
			v=Vector(x,y,z);
			return is;
		}

	}; // end class Vector

} // End namespace fluo


namespace fluo
{
inline Vector Abs(const Vector& v)
{
	double x = v.x_ < 0 ? -v.x_ : v.x_;
	double y = v.y_ < 0 ? -v.y_ : v.y_;
	double z = v.z_ < 0 ? -v.z_ : v.z_;
	return Vector(x, y, z);
}

inline Vector Cross(const Vector& v1, const Vector& v2)
{
	return Vector(v1.y_ * v2.z_ - v1.z_ * v2.y_,
		v1.z_ * v2.x_ - v1.x_ * v2.z_,
		v1.x_ * v2.y_ - v1.y_ * v2.x_);
}

inline Vector Interpolate(const Vector& v1, const Vector& v2,
	double weight)
{
	double weight1 = 1.0 - weight;
	return Vector(v2.x_ * weight + v1.x_ * weight1,
		v2.y_ * weight + v1.y_ * weight1,
		v2.z_ * weight + v1.z_ * weight1);
}

inline double Dot(const Vector& v1, const Vector& v2)
{
	return v1.x_ * v2.x_ + v1.y_ * v2.y_ + v1.z_ * v2.z_;
}

inline double Dot(const Vector& v, const Point& p)
{
	return v.x_ * p.x_ + v.y_ * p.y_ + v.z_ * p.z_;
}


} // End namespace fluo


#endif//_FLVECTOR_H_
