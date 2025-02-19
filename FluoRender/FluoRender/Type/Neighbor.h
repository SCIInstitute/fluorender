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

#ifndef _FLNEIGHBOR_H_
#define _FLNEIGHBOR_H_

#include <Point.h>
#include <Vector.h>
#include <BBox.h>

namespace fluo
{
	//defines a uniform sqaure sample space around a center point
	class Neighbor
	{
		Point c_;//center of neighborhood
		Vector n_;//number of neighbors in each dimension and direction (total = 2n+1)
		Vector h_;//uniform step size
		int x_, y_, z_;
	public:
		Neighbor() : n_(1), h_(1) {}
		Neighbor(const Point& c) : c_(c), n_(1), h_(1) {}
		Neighbor(const Point& c, const Vector& n) : c_(c), n_(n), h_(1) {}
		Neighbor(const Point& c, const Vector& n, const Vector& h) :
			c_(c), n_(n), h_(h) {}
		Neighbor(const BBox& b) :
			c_(b.center()), h_(1)
		{
			n_ = (b.diagonal() - h_) / 2;
		}
		Neighbor(const BBox& b, const Vector& h) :
			c_(b.center()), h_(h)
		{
			n_ = (b.diagonal() - h_) / 2;
		}
		Neighbor(const Neighbor& neighbor)
		{
			c_ = neighbor.c_;
			n_ = neighbor.n_;
			h_ = neighbor.h_;
		}

		void c(const Point& c) { c_ = c; }
		void n(const Vector& n) { n_ = n; }
		void h(const Vector& h) { h_ = h; }
		Point c() const { return c_; }
		Vector n() const { return n_; }
		Vector h() const { return h_; }

		void offset(const Vector& v) { c_ = c_ + v; }
		void halfn()
		{
			n_.x(n_.x() == 0.0 ? 0 : std::max(1, int(n_.x() / 2)));
			n_.y(n_.y() == 0.0 ? 0 : std::max(1, int(n_.y() / 2)));
			n_.z(n_.z() == 0.0 ? 0 : std::max(1, int(n_.z() / 2)));
			//n_ = Max(n_ / 2, Vector(1));
		}
		void halfh() { h_ = Max(h_ / 2, Vector(Epsilon())); }

		const Point begin();
		const Point end();
		const Point operator++();
		const Point operator++(int);
	};

	class Range
	{
		Point c_;//corner of neighborhood
		Vector n_;//number of samples in the positive direction
		Vector h_;//uniform step size (set to negative to go backward)
		int x_, y_, z_;//counter
	public:
		Range() : n_(1), h_(1) {}
		Range(const Point& c) : c_(c), n_(1), h_(1) {}
		Range(const Point& c, const Vector& n) : c_(c), n_(n), h_(1) {}
		Range(const Point& c, const Vector& n, const Vector& h) :
			c_(c), n_(n), h_(h) {}
		Range(const BBox& b) :
			c_(b.Min()), h_(1)
		{
			n_ = b.diagonal() / h_;
		}
		Range(const BBox& b, const Vector& h) :
			c_(b.Min()), h_(h)
		{
			n_ = b.diagonal() / h_;
		}

		void c(const Point& c) { c_ = c; }
		void n(const Vector& n) { n_ = n; }
		void h(const Vector& h) { h_ = h; }
		Point c() const { return c_; }
		Vector n() const { return n_; }
		Vector h() const { return h_; }

		const Point begin();
		const Point end();
		const Point operator++();
		const Point operator++(int);
	};
}
#endif//_FLNEIGHBOR_H_