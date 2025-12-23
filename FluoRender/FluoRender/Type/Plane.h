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

#ifndef _FLPLANE_H_
#define _FLPLANE_H_

#include <Vector.h>
#include <vector>
#include <iomanip>

namespace fluo
{

	class Point;
	class Quaternion;
	class Transform;
	class Plane
	{
		Vector n_;
		double d_;

	public:
		Plane(const Plane &copy);
		Plane(const Point &p1, const Point &p2, const Point &p3);
		Plane(const Point &p, const Vector &n);
		Plane();
		Plane(double a, double b, double c, double d);
		Plane(const Vector& n, double d);
		~Plane();

		inline double d() const
		{ return d_; }
		inline Vector n() const
		{ return n_; }

		Plane& operator=(const Plane&);
		double eval_point(const Point &p) const;
		double dot_normal(const Vector &v) const;
		Point get_point() const;
		void flip();
		Point project(const Point& p) const;
		Vector project(const Vector& v) const;
		Vector normal() const;
		void get(double (&abcd)[4]) const;

		// Not a great ==, doesnt take into account for floating point error.
		bool operator==(const Plane &rhs) const;
		bool operator!=(const Plane &rhs) const;
		// changes the plane ( n and d )
		void ChangePlane( const Point &p1, const Point &p2, const Point &p3 );
		void ChangePlane( const Point &p1, const Vector &v);
		void ChangeNormal(const Vector& n);
		void ChangeD(double d);

		// plane-line intersection
		// returns true if the line  v*t+s  for -inf < t < inf intersects
		// the plane.  if so, hit contains the point of intersection.
		int Intersect( Point s, Vector v, Point& hit ) const;
		int Intersect( Point s, Vector v, double &t ) const;
		// plane-plane intersection
		int Intersect(Plane p, Point &s, Vector &v);

		//translate the plane by a vector
		void Translate(const Vector &v);
		//rotate the plane around origin by a quaternion
		void Rotate(const Quaternion &q);
		void RotateAroundPoint(const Quaternion& q, const Point& p);
		//full transform
		Plane Transformed(const Transform& t);
		friend std::ostream& operator<<(std::ostream& os, const Plane& p)
		{
			os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
			os << "[[" << p.n_.x() << ',' << p.n_.y() << ',' << p.n_.z() << "]," << p.d_ << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Plane& p)
		{
			double x, y, z, d;
			char st;
			is >> st >> st >> x >> st >> y >> st >> z >> st >> st >> d >> st;
			p = Plane(x, y, z, d);
			return is;
		}
	};

	// Affine transform of a plane under x' = S x + t
	// S is diagonal scale (sx, sy, sz), t is translation (tx, ty, tz)
	inline Plane TransformPlaneAffine(const Plane& p, const Vector& S, const Vector& t)
	{
		// Original plane: n·x + d = 0
		Vector n = p.n();
		double d = p.d();

		// n' = S^{-T} n  (for diagonal S, divide per axis)
		Vector nprime(n.x() / S.x(), n.y() / S.y(), n.z() / S.z());

		// d' = d - n'·t  (translation term with minus sign)
		double dprime = d - Dot(t, nprime);

		// Optional: normalize (maintain equation by scaling d' identically)
		double k = nprime.length();
		if (k > 0.0)
		{
			nprime /= k;
			dprime /= k;
		}

		return Plane(nprime, dprime);
	}

	inline double PlaneDistance(const Plane& a, const Plane& b)
	{
		if (Dot(a.n(), b.n()) < 0.0)
		{
			Vector n = -b.n();
			double d = -b.d();
			return std::abs(d - a.d()) / a.n().length();
		}
		return std::abs(b.d() - a.d()) / a.n().length();
	}

	class PlaneSet
	{
	public:
		PlaneSet() = default;
		explicit PlaneSet(size_t size) : planes_(size) {}
		PlaneSet(const std::vector<Plane>& planes) : planes_(planes) {}
		PlaneSet(const PlaneSet& copy) = default;
		PlaneSet(PlaneSet&& other) noexcept = default;
		~PlaneSet() = default;

		PlaneSet& operator=(const PlaneSet& ps) = default;
		PlaneSet& operator=(PlaneSet&& ps) noexcept = default;

		bool operator==(const PlaneSet& ps) const { return planes_ == ps.planes_; }
		bool operator!=(const PlaneSet& ps) const { return planes_ != ps.planes_; }

		// Container-like interface
		size_t size() const { return planes_.size(); }
		bool empty() const { return planes_.empty(); }
		void clear() { planes_.clear(); }
		void resize(size_t n) { planes_.resize(n); }
		void assign(size_t n, const Plane& p = Plane()) { planes_.assign(n, p); }
		void push_back(const Plane& p) { planes_.push_back(p); }

		Plane& operator[](size_t i) { return planes_[i]; }
		const Plane& operator[](size_t i) const { return planes_[i]; }

		auto begin() { return planes_.begin(); }
		auto end() { return planes_.end(); }
		auto begin() const { return planes_.begin(); }
		auto end() const { return planes_.end(); }

		friend std::ostream& operator<<(std::ostream& os, const PlaneSet& ps)
		{
			for (size_t i=0; i<ps.planes_.size(); ++i)
			{
				os << ps.planes_[i];
				if (i < ps.planes_.size() - 1)
					os << ',';
			}
			return os;
		}
		friend std::istream& operator >> (std::istream& is, PlaneSet& ps)
		{
			ps.planes_.clear();
			while (!is.eof())
			{
				Plane p;
				char st;
				is >> p >> st;
				ps.planes_.push_back(p);
			}
			return is;
		}

	protected:
		std::vector<Plane> planes_;
	};

} // End namespace fluo

#endif
