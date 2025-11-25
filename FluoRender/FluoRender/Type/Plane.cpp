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

#include <Plane.h>
#include <Point.h>
#include <Vector.h>
#include <Quaternion.h>
#include <Transform.h>
#include <iostream>

namespace fluo
{
	Plane::Plane()
		: n_(Vector(0, 0, 1)), d_(0)
	{
	}

	Plane::Plane(double a, double b, double c, double d) : n_(Vector(a, b, c)), d_(d)
	{
		double l = n_.length();
		d_ /= l;
		n_.normalize();
	}

	Plane::Plane(const Point& p, const Vector& normal)
		: n_(normal), d_(-Dot(p, normal))
	{
	}

	Plane::Plane(const Plane& copy)
		: n_(copy.n_), d_(copy.d_)
	{
	}

	Plane::Plane(const Point& p1, const Point& p2, const Point& p3)
	{
		Vector v1(p2 - p1);
		Vector v2(p2 - p3);
		n_ = Cross(v2, v1);
		n_.normalize();
		d_ = -Dot(p1, n_);
	}

	Plane::~Plane()
	{
	}

	Plane& Plane::operator=(const Plane& copy)
	{
		n_ = copy.n_;
		d_ = copy.d_;
		return *this;
	}

	void Plane::flip()
	{
		n_.x(-n_.x());
		n_.y(-n_.y());
		n_.z(-n_.z());
		d_ = -d_;
	}

	double Plane::eval_point(const Point& p) const
	{
		return Dot(p, n_) + d_;
	}

	double Plane::dot_normal(const Vector& v) const
	{
		return Dot(v, n_);
	}

	Point Plane::get_point() const
	{
		return -d_ * Point(n_);
	}

	Point Plane::project(const Point& p) const
	{
		return p - n_ * (d_ + Dot(p, n_));
	}

	Vector Plane::project(const Vector& v) const
	{
		return v - n_ * Dot(v, n_);
	}

	Vector Plane::normal() const
	{
		return n_;
	}

	void Plane::ChangePlane(const Point& p1, const Point& p2, const Point& p3)
	{
		//Restore();

		Vector v1(p2 - p1);
		Vector v2(p2 - p3);
		n_ = Cross(v2, v1);
		n_.normalize();
		d_ = -Dot(p1, n_);
	}

	void Plane::ChangePlane(const Point& P, const Vector& N)
	{
		//Restore();

		n_ = N;
		n_.safe_normalize();
		d_ = -Dot(P, n_);
	}

	int Plane::Intersect(Point s, Vector v, Point& hit) const
	{
		Point origin(0., 0., 0.);
		Point ptOnPlane = origin - n_ * d_;
		double tmp = Dot(n_, v);

		if (tmp > -1.e-6 && tmp < 1.e-6) // Vector v is parallel to plane
		{
			// vector from origin of line to point on plane

			Vector temp = s - ptOnPlane;

			if (temp.length() < 1.e-5)
			{
				// the origin of plane and the origin of line are almost
				// the same

				hit = ptOnPlane;
				return 1;
			}
			else
			{
				// point s and d are not the same, but maybe s lies
				// in the plane anyways

				tmp = Dot(temp, n_);

				if (tmp > -1.e-6 && tmp < 1.e-6)
				{
					hit = s;
					return 1;
				}
				else
					return 0;
			}
		}

		tmp = -((d_ + Dot(s, n_)) / Dot(v, n_));

		hit = s + v * tmp;

		return 1;
	}

	int Plane::Intersect(Point s, Vector v, double& t) const
	{
		double tmp = Dot(n_, v);
		if (tmp > -1.e-6 && tmp < 1.e-6) // Vector v is parallel to plane
		{
			// vector from origin of line to point on plane
			Vector temp = (s + n_ * d_).asVector();
			if (temp.length() < 1.e-5)
			{
				// origin of plane and origin of line are almost the same
				t = 0.0;
				return 1;
			}
			else
			{
				// point s and d are not the same, but maybe s lies
				// in the plane anyways
				tmp = Dot(temp, n_);
				if (tmp > -1.e-6 && tmp < 1.e-6)
				{
					t = 0.0;
					return 1;
				}
				else
					return 0;
			}
		}

		t = -((d_ + Dot(s, n_)) / Dot(v, n_));
		return 1;
	}

	//plane-plane intersection
	int Plane::Intersect(Plane p, Point& s, Vector& v)
	{
		//cross direction
		Vector cd = Cross(n_, p.n_);
		if (cd.length() < 1.e-6)
			return 0;

		v = cd;
		double s1, s2, a, b;
		s1 = -d_;
		s2 = -p.d_;
		double n1n2dot = Dot(n_, p.n_);
		double n1normsqr = Dot(n_, n_);
		double n2normsqr = Dot(p.n_, p.n_);
		a = (s2 * n1n2dot - s1 * n2normsqr) / (n1n2dot * n1n2dot - n1normsqr * n2normsqr);
		b = (s1 * n1n2dot - s2 * n2normsqr) / (n1n2dot * n1n2dot - n1normsqr * n2normsqr);
		s = Point(a * n_ + b * p.n_);
		return 1;
	}

	void Plane::get(double(&abcd)[4]) const
	{
		abcd[0] = n_.x();
		abcd[1] = n_.y();
		abcd[2] = n_.z();
		abcd[3] = d_;
	}

	bool Plane::operator==(const Plane& rhs) const
	{
		double cosine = Dot(this->n_, rhs.n_);
		double d1 = this->eval_point(Point(0, 0, 0));
		double d2 = rhs.eval_point(Point(0, 0, 0));
		return (fabs(double(d1 - d2)) < Epsilon()) && (fabs(double(cosine - 1.0)) < Epsilon());
	}

	bool Plane::operator!=(const Plane& rhs) const
	{
		double cosine = Dot(this->n_, rhs.n_);
		double d1 = this->eval_point(Point(0, 0, 0));
		double d2 = rhs.eval_point(Point(0, 0, 0));
		return (fabs(double(d1 - d2)) > Epsilon()) || (fabs(double(cosine - 1.0)) > Epsilon());
	}

	void Plane::Translate(const Vector& v)
	{
		double dd = Dot(v, n_);
		d_ = d_ - dd;
	}

	void Plane::Rotate(const Quaternion& q)
	{
		Quaternion p(n_.x(), n_.y(), n_.z(), 0.0);
		Quaternion p2 = (-q) * p * q;
		n_ = Vector(p2.x, p2.y, p2.z);
	}

	void Plane::RotatePoint(const Quaternion& q, const Point& pivot)
	{
		// Rotate normal
		Quaternion p(n_.x(), n_.y(), n_.z(), 0.0);
		Quaternion p2 = (-q) * p * q;
		Vector newNormal(p2.x, p2.y, p2.z);

		// Rotate pivot
		Quaternion pv(pivot.x(), pivot.y(), pivot.z(), 0.0);
		Quaternion pv2 = (-q) * pv * q;
		Point newPivot(pv2.x, pv2.y, pv2.z);

		// Recompute plane: passes through newPivot with newNormal
		n_ = newNormal;
		d_ = Dot(Vector(newPivot.x(), newPivot.y(), newPivot.z()), n_);
	}

	Plane Plane::Transformed(const Transform& t)
	{
		// Transform a representative point on the plane
		Point p0 = Point(n_.x() * -d_, n_.y() * -d_, n_.z() * -d_);
		Point pT = t.transform(p0);

		// Transform the normal using inverse-transpose
		Vector nT = t.project_normal(n_);
		nT.normalize();

		return Plane(pT, nT);
	}

} // End namespace fluo

