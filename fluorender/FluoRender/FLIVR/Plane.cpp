/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2004 Scientific Computing and Imaging Institute,
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

#include <FLIVR/Plane.h>
#include <FLIVR/Point.h>
#include <FLIVR/Vector.h>
#include <FLIVR/Quaternion.h>
#include <iostream>

namespace FLIVR
{
	Plane::Plane()
		: n(Vector(0,0,1)), d(0)
	{
		n_copy = n;
		d_copy = d;
	}

	Plane::Plane(double a, double b, double c, double d) : n(Vector(a,b,c)), d(d)
	{
		double l=n.length();
		d/=l;
		n.normalize();

		n_copy = n;
		d_copy = d;
	}

	Plane::Plane(const Point &p, const Vector &normal)
		: n(normal), d(-Dot(p, normal))
	{
		n_copy = n;
		d_copy = d;
	}

	Plane::Plane(const Plane &copy)
		: n(copy.n), d(copy.d)
	{
		n_copy = copy.n_copy;
		d_copy = copy.d_copy;
	}

	Plane::Plane(const Point &p1, const Point &p2, const Point &p3)
	{
		Vector v1(p2-p1);
		Vector v2(p2-p3);
		n=Cross(v2,v1);
		n.normalize();
		d=-Dot(p1, n);

		n_copy = n;
		d_copy = d;
	}

	Plane::~Plane()
	{
	}

	Plane& Plane::operator=(const Plane& copy)
	{
		n = copy.n;
		d = copy.d;
		n_copy = copy.n_copy;
		d_copy = copy.d_copy;
		return *this;
	}

	void Plane::flip()
	{
		n.x(-n.x());
		n.y(-n.y());
		n.z(-n.z());
		d=-d;
	}

	double Plane::eval_point(const Point &p) const
	{
		return Dot(p, n)+d;
	}

	Point Plane::get_point() const
	{
		return -d*Point(n);
	}

	Point Plane::project(const Point& p) const
	{
		return p-n*(d+Dot(p,n));
	}

	Vector Plane::project(const Vector& v) const
	{
		return v-n*Dot(v,n);
	}

	Vector Plane::normal() const
	{
		return n;
	}

	void Plane::ChangePlane(const Point &p1, const Point &p2, const Point &p3)
	{
		//Restore();

		Vector v1(p2-p1);
		Vector v2(p2-p3);
		n=Cross(v2,v1);
		n.normalize();
		d=-Dot(p1, n);

		Remember();
	}

	void Plane::ChangePlane(const Point &P, const Vector &N)
	{
		//Restore();

		n = N;
		n.safe_normalize();
		d = -Dot(P,n);

		Remember();
	}

	int Plane::Intersect( Point s, Vector v, Point& hit ) const
	{
		Point origin( 0., 0., 0. );
		Point ptOnPlane = origin - n * d;
		double tmp = Dot( n, v );

		if( tmp > -1.e-6 && tmp < 1.e-6 ) // Vector v is parallel to plane
		{
			// vector from origin of line to point on plane

			Vector temp = s - ptOnPlane;

			if ( temp.length() < 1.e-5 )
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

				tmp = Dot( temp, n );

				if(tmp > -1.e-6 && tmp < 1.e-6)
				{
					hit = s;
					return 1;
				}
				else
					return 0;
			}
		}

		tmp = - ( ( d + Dot( s, n ) ) / Dot( v, n ) );

		hit = s + v * tmp;

		return 1;
	}

	int Plane::Intersect(Point s, Vector v, double &t) const
	{
		double tmp = Dot( n, v );
		if(tmp > -1.e-6 && tmp < 1.e-6) // Vector v is parallel to plane
		{
			// vector from origin of line to point on plane
			Vector temp = (s + n*d).asVector();
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
				tmp = Dot(temp, n);
				if (tmp > -1.e-6 && tmp < 1.e-6)
				{
					t = 0.0;
					return 1;
				}
				else
					return 0;
			}
		}

		t = -((d + Dot(s, n)) / Dot(v, n));
		return 1;
	}

	//plane-plane intersection
	int Plane::Intersect(Plane p, Point &s, Vector &v)
	{
		//cross direction
		Vector cd = Cross(n, p.n);
		if (cd.length() < 1.e-6)
			return 0;

		v = cd;
		double s1, s2, a, b;
		s1 = -d;
		s2 = -p.d;
		double n1n2dot = Dot(n, p.n);
		double n1normsqr = Dot(n, n);
		double n2normsqr = Dot(p.n, p.n);
		a = (s2 *n1n2dot - s1 * n2normsqr) / (n1n2dot*n1n2dot - n1normsqr*n2normsqr);
		b = (s1 *n1n2dot - s2 * n2normsqr) / (n1n2dot*n1n2dot - n1normsqr*n2normsqr);
		s = Point(a*n + b*p.n);
		return 1;
	}

	void Plane::get(double (&abcd)[4]) const
	{
		abcd[0] = n.x();
		abcd[1] = n.y();
		abcd[2] = n.z();
		abcd[3] = d;
	}

	void Plane::get_copy(double (&abcd)[4]) const
	{
		abcd[0] = n_copy.x();
		abcd[1] = n_copy.y();
		abcd[2] = n_copy.z();
		abcd[3] = d_copy;
	}

	bool Plane::operator==(const Plane &rhs) const
	{ 
		double cosine = Dot(this->n, rhs.n);
		double d1 = this->eval_point(Point(0,0,0));
		double d2 = rhs.eval_point(Point(0,0,0));
		return (fabs(double(d1-d2)) < 0.000001) && (fabs(double(cosine-1.0)) < 0.00001); 
	}

	void Plane::Translate(Vector &v)
	{
		double dd = Dot(v, n);
		d = d - dd;
	}

	void Plane::Rotate(Quaternion &q)
	{
		Quaternion p(n.x(), n.y(), n.z(), 0.0);
		Quaternion p2 = (-q) * p * q;
		n = Vector(p2.x, p2.y, p2.z);
	}

	void Plane::Scale(Vector &v)
	{
		n = Vector(n.x()/v.x(), n.y()/v.y(), n.z()/v.z());
		n.safe_normalize();
	}

} // End namespace FLIVR

