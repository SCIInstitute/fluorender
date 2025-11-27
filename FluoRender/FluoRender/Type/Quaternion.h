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

#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include <Vector.h>
#include <Utils.h>
#include <math.h>

#pragma warning (disable : 4521 4522)

namespace fluo
{

class Quaternion
{
public:
	Quaternion() : x(0), y(0), z(0), w(1)
	{
	}

	Quaternion(double x, double y, double z, double w) :
	x(x), y(y), z(z), w(w)
	{
	}

	// Set Quat from axis-angle
	Quaternion(double d, Vector& axis)
	{
		double h_angle = d * Pi() / 360.0;
		double sin_a = sin(h_angle);
		w = cos(h_angle);
		x = sin_a * axis.x();
		y = sin_a * axis.y();
		z = sin_a * axis.z();
	}

	// Set Quat from vector
	Quaternion(Vector & v)
	{
		w = 0.0;
		x = v.x();
		y = v.y();
		z = v.z();
	}

	Quaternion(Quaternion& copy) :
	x(copy.x), y(copy.y), z(copy.z), w(copy.w)
	{
	}

	Quaternion(const Quaternion& copy) :
	x(copy.x), y(copy.y), z(copy.z), w(copy.w)
	{
	}

	Quaternion(const std::string& s)
	{
		*this = from_string(s);
	}

	Quaternion& operator=(Quaternion& copy)
	{
		x = copy.x;
		y = copy.y;
		z = copy.z;
		w = copy.w;
		return *this;
	}

	Quaternion& operator=(const Quaternion& copy)
	{
		x = copy.x;
		y = copy.y;
		z = copy.z;
		w = copy.w;
		return *this;
	}

	~Quaternion()
	{
	}

	void Reset()
	{
		x = y = z = 0;
		w = 1;
	}

	Quaternion operator+(const Quaternion& q) const
	{
		return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	Quaternion& operator+=(const Quaternion& q)
	{
		x+=q.x; y+=q.y; z+=q.z; w+=q.w;
		return *this;
	}

	Quaternion operator-(const Quaternion& q)const 
	{
		return Quaternion(x - q.x, y - q.y, z - q.z, w + q.w);
	}

	Quaternion& operator-=(const Quaternion& q)
	{
		x-=q.x; y-=q.y; z-=q.z; w+=q.w;
		return *this;
	}

	Quaternion operator-()const 
	{
		return Quaternion(-x, -y, -z, w);
	}

	Quaternion operator*(const Quaternion& q)const 
	{
		return Quaternion(
			w*q.x + x*q.w + y*q.z - z*q.y,
			w*q.y + y*q.w + z*q.x - x*q.z,
			w*q.z + z*q.w + x*q.y - y*q.x,
			w*q.w - x*q.x - y*q.y - z*q.z);
	}

	Quaternion& operator*=(const Quaternion& q)
	{
		double x0, y0, z0, w0;
		x0 = w*q.x + x*q.w + y*q.z - z*q.y;
		y0 = w*q.y + y*q.w + z*q.x - x*q.z;
		z0 = w*q.z + z*q.w + x*q.y - y*q.x;
		w0 = w*q.w - x*q.x - y*q.y - z*q.z;
		x = x0; y = y0; z = z0; w = w0;
		return *this;
	}

	Quaternion operator*(const double s)const 
	{
		return Quaternion(x*s, y*s, z*s, w*s);
	}

	Quaternion& operator*=(const double s)
	{
		x*=s; y*=s; z*=s; w*=s;
		return *this;
	}

	bool operator==(const Quaternion& q)const 
	{
		return (x==q.x && y==q.y && z==q.z && w==q.w);
	}

	bool operator!=(const Quaternion& q)const
	{
		return (x != q.x || y != q.y || z != q.z || w != q.w);
	}

	bool AlmostEqual(const Quaternion& q)const
	{
		return (fabs(x - q.x) < Epsilon() &&
			fabs(y - q.y) < Epsilon() &&
			fabs(z - q.z) < Epsilon() &&
			fabs(w - q.w) < Epsilon()) ||
			(fabs(x + q.x) < Epsilon() &&
			fabs(y + q.y) < Epsilon() &&
			fabs(z + q.z) < Epsilon() &&
			fabs(w + q.w) < Epsilon());
	}

	bool IsIdentity()
	{
		return (x==0.0 && y==0.0 && z==0.0 && w==1.0);
	}

	double Length()
	{
		return sqrt(w*w + Vector(x, y, z).length2());
	}

	Point GetPoint()
	{
		return Point(x, y, z);
	}

	Vector GetVector()
	{
		return Vector(x, y, z);
	}

	Quaternion Invert()
	{
		return -(*this)*(1/this->Length());
	}

	//object to initial
	void FromEuler(const Vector& r)
	{
		double rx = d2r(r.x());
		double ry = d2r(r.y());
		double rz = d2r(r.z());

		// Compute sine and cosine of the half angles
		double	sp, sb, sh;
		double	cp, cb, ch;
		sinCos(&sp, &cp, rx * 0.5);
		sinCos(&sb, &cb, rz * 0.5);
		sinCos(&sh, &ch, ry * 0.5);

		// Compute values
		w =  ch*cp*cb + sh*sp*sb;
		x =  ch*sp*cb + sh*cp*sb;
		y = -ch*sp*sb + sh*cp*cb;
		z = -sh*sp*cb + ch*cp*sb;
	}

	//object to initial
	Vector ToEuler()
	{
		double rx, ry, rz;
		// Extract sin(pitch)
		double sp = -2.0 * (y*z - w*x);

		// Check for Gimbel lock, giving slight tolerance for numerical imprecision
		if (fabs(sp) > 0.9999)
		{
			// Looking straight up or down
			rx = PiHalf() * sp;

			// Compute heading, slam bank to zero
			ry = atan2(-x*z + w*y, 0.5 - y*y - z*z);
			rz = 0.0;

		}
		else
		{
			// Compute angles.  We don't have to use the "safe" asin
			// function because we already checked for range errors when
			// checking for Gimbel lock
			rx	= asin(sp);
			ry	= atan2(x*z + w*y, 0.5 - x*x - y*y);
			rz	= atan2(x*y + w*z, 0.5 - x*x - z*z);
		}

		rx = r2d(rx);
		ry = r2d(ry);
		rz = r2d(rz);

		rx = rx==0.0?0.0:rx;
		ry = ry==0.0?0.0:ry;
		rz = rz==0.0?0.0:rz;

		return Vector(rx, ry, rz);
	}

	int Normalize()
	{
		double len_sq = x * x + y * y + z * z + w * w;
		if (len_sq == 0.0) return -1;
		if (len_sq != 1.0)
		{
			double scale = ( 1.0 / sqrt( len_sq ) );
			x *= scale;
			y *= scale;
			z *= scale;
			w *= scale;
			return 1;
		}
		return 0;
	}

	void Conjugate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

public:
	double x, y, z, w;

public:
	inline std::string to_string() const
	{
		std::ostringstream oss;
		oss << *this;
		return oss.str();
	}

	static Quaternion from_string(const std::string& str) {
		std::istringstream is(str);
		Quaternion q;
		is >> q;
		return q;
	}

	friend std::ostream& operator<<(std::ostream& os, const Quaternion& q)
	{
		os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
		os << '[' << q.x << ',' << q.y << ',' << q.z << ',' << q.w << ']';
		return os;
	}
	friend std::istream& operator >> (std::istream& is, Quaternion& q)
	{
		double x, y, z, w;
		char ch;
		is >> ch;
		if (ch == '[') {
			is >> x >> ch >> y >> ch >> z >> ch >> w >> ch;
		}
		else {
			is.putback(ch);
			is >> x >> y >> z >> w;
		}
		q = Quaternion(x, y, z, w);
		return is;
	}
};

inline Quaternion operator*(double s, Quaternion& q)
{
	return Quaternion(s*q.x, s*q.y, s*q.z, s*q.w);
}

inline double Dot(Quaternion& q1, Quaternion& q2)
{
	return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
}

inline Quaternion Slerp(Quaternion& a, Quaternion& b, double t)
{
	double w1, w2;
	double cos_theta = Dot(a, b);
	double theta;
	double r_sin_theta;
	if (cos_theta >= Epsilon())
	{
		if (1.0-cos_theta > Epsilon())
		{
			theta = acos(cos_theta);
			r_sin_theta = 1.0 / sin(theta);
			w1 = sin((1.0-t)*theta) * r_sin_theta;
			w2 = sin(t*theta) * r_sin_theta;
		}
		else
		{
			w1 = 1.0 - t;
			w2 = t;
		}
	}
	else
	{
		if (1.0+cos_theta > Epsilon())
		{
			theta = acos(-cos_theta);
			r_sin_theta = 1.0 / sin(theta);
			w1 = sin((t-1.0)*theta) * r_sin_theta;
			w2 = sin(t*theta) * r_sin_theta;
		}
		else
		{
			w1 = t-1.0;
			w2 = t;
		}
	}

	return a*w1 + b*w2;
}

inline Quaternion NLerp(Quaternion& a, Quaternion& b, double w2)
{
	double w1 = 1.0 - w2;
	Quaternion q = (a*w1) + (b*w2);
	q.Normalize();
	return q;
}

}
#endif//_QUATERNION_H_
