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

#ifndef _FLBBOX_H_
#define _FLBBOX_H_

#include <Vector.h>
#include <Point.h>
#include <Utils.h>
#include <ostream>
#include <algorithm>
#include <iomanip>

namespace fluo
{
	class BBox
	{
	public:

		BBox() :
			is_valid_(false)
		{
		}

		~BBox()
		{
		}

		BBox(const BBox& copy)
			: cmin_(copy.cmin_), cmax_(copy.cmax_), is_valid_(copy.is_valid_) {
		}

		BBox& operator=(const BBox& copy)
		{
			is_valid_ = copy.is_valid_;
			cmin_ = copy.cmin_;
			cmax_ = copy.cmax_;
			return *this;
		}

		inline bool operator==(const BBox& bbox) const
		{
			return (cmin_ == bbox.cmin_) && (cmax_ == bbox.cmax_);
		}

		inline bool operator!=(const BBox& bbox) const
		{
			return (cmin_ != bbox.cmin_) || (cmax_ != bbox.cmax_);
		}

		BBox(const Point& min, const Point& max)
			: cmin_(min), cmax_(max), is_valid_(true) {
		}

		BBox(const Point& max)
			: cmin_(Point(0.0)), cmax_(max), is_valid_(true) { }

		Vector scale_to(const BBox& bbox) const
		{
			Vector d1 = this->diagonal();
			Vector d2 = bbox.diagonal();
			return Vector(d2.x() / d1.x(), d2.y() / d1.y(), d2.z() / d1.z());
		}

		BBox normalized(const BBox& ref) const
		{
			Point refMin = ref.Min();
			Vector refDiag = ref.diagonal();

			auto safeNormalize = [&](double val, double refMin, double refExtent) -> double {
				if (refExtent == 0.0) return 0.0; // avoid divide by zero
				double u = (val - refMin) / refExtent;
				// clamp to [0,1] to stay inside unit box
				if (u < 0.0) u = 0.0;
				if (u > 1.0) u = 1.0;
				return u;
				};

			Point nmin(
				safeNormalize(cmin_.x(), refMin.x(), refDiag.x()),
				safeNormalize(cmin_.y(), refMin.y(), refDiag.y()),
				safeNormalize(cmin_.z(), refMin.z(), refDiag.z())
			);

			Point nmax(
				safeNormalize(cmax_.x(), refMin.x(), refDiag.x()),
				safeNormalize(cmax_.y(), refMin.y(), refDiag.y()),
				safeNormalize(cmax_.z(), refMin.z(), refDiag.z())
			);

			return BBox(nmin, nmax);
		}

		BBox denormalized(const BBox& ref) const
		{
			Point refMin = ref.Min();
			Vector refDiag = ref.diagonal();

			auto safeDenorm = [&](double u, double refMin, double refExtent) -> double {
				return refMin + u * refExtent;
				};

			Point dmin(
				safeDenorm(cmin_.x(), refMin.x(), refDiag.x()),
				safeDenorm(cmin_.y(), refMin.y(), refDiag.y()),
				safeDenorm(cmin_.z(), refMin.z(), refDiag.z())
			);

			Point dmax(
				safeDenorm(cmax_.x(), refMin.x(), refDiag.x()),
				safeDenorm(cmax_.y(), refMin.y(), refDiag.y()),
				safeDenorm(cmax_.z(), refMin.z(), refDiag.z())
			);

			return BBox(dmin, dmax);
		}

		void unit()
		{
			cmin_ = Point(0.0);
			cmax_ = Point(1.0);
			is_valid_ = true;
		}

		inline int valid() const { return is_valid_; }

		inline void set_valid(bool v) { is_valid_ = v; }
		inline void reset() { is_valid_ = false; }

		//! Expand the bounding box to include point p
		inline void extend(const Point& p)
		{
			if (is_valid_)
			{
				cmin_ = fluo::Min(p, cmin_);
				cmax_ = fluo::Max(p, cmax_);
			}
			else
			{
				cmin_ = p;
				cmax_ = p;
				is_valid_ = true;
			}
		}

		//! Extend the bounding box on all sides by a margin
		//! For example to expand it by a certain epsilon to make
		//! sure that a lookup will be inside the bounding box
		inline void extend(double val)
		{
			if (is_valid_)
			{
				cmin_.x(cmin_.x() - val);
				cmin_.y(cmin_.y() - val);
				cmin_.z(cmin_.z() - val);
				cmax_.x(cmax_.x() + val);
				cmax_.y(cmax_.y() + val);
				cmax_.z(cmax_.z() + val);
			}
		}

		inline void extend_mul(double val)
		{
			if (is_valid_)
			{
				Vector d = (cmax_ - cmin_) * val / 2.0;
				cmin_.x(cmin_.x() - d.x());
				cmin_.y(cmin_.y() - d.y());
				cmin_.z(cmin_.z() - d.z());
				cmax_.x(cmax_.x() + d.x());
				cmax_.y(cmax_.y() + d.y());
				cmax_.z(cmax_.z() + d.z());
			}
		}

		//extend by a ratio for all axes
		inline void extend(double px, double py, double pz)
		{
			if (is_valid_)
			{
				Vector diag = diagonal();
				double valx = diag.x() * px / 2.0;
				double valy = diag.y() * py / 2.0;
				double valz = diag.z() * pz / 2.0;

				cmin_.x(cmin_.x() - valx);
				cmin_.y(cmin_.y() - valy);
				cmin_.z(cmin_.z() - valz);
				cmax_.x(cmax_.x() + valx);
				cmax_.y(cmax_.y() + valy);
				cmax_.z(cmax_.z() + valz);
			}
		}

		//! Expand the bounding box to include a sphere of radius radius
		//! and centered at point p
		inline void extend(const Point& p, double radius)
		{
			Vector r(radius, radius, radius);
			if (is_valid_)
			{
				cmin_ = fluo::Min(p - r, cmin_);
				cmax_ = fluo::Max(p + r, cmax_);
			}
			else
			{
				cmin_ = p - r;
				cmax_ = p + r;
				is_valid_ = true;
			}
		}

		//! Expand the bounding box to include bounding box b
		inline void extend(const BBox& b)
		{
			if (b.valid())
			{
				extend(b.Min());
				extend(b.Max());
			}
		}

		//! Expand the bounding box to include a disk centered at cen,
		//! with normal normal, and radius r.
		void extend_disk(const Point& cen, const Vector& normal, double r);

		//extend anisotropically
		inline void extend_ani(const Vector& v)
		{
			if (is_valid_)
			{
				Vector d = diagonal();
				cmin_ -= d * v;
				cmax_ += d * v;
			}
		}

		//clamp the box to another one
		inline void clamp(const BBox& box)
		{
			if (is_valid_)
			{
				cmin_ = fluo::Max(cmin_, box.Min());
				cmax_ = fluo::Min(cmax_, box.Max());
				if (!(cmin_ <= cmax_))
					is_valid_ = false;
			}
		}

		//x, y, z size
		inline Point size() const
		{
			assert(is_valid_);
			return Point(cmax_ - cmin_);
		}

		inline Point center() const
		{
			assert(is_valid_); Vector d = diagonal(); return cmin_ + (d * 0.5);
		}

		inline double longest_edge() const
		{
			assert(is_valid_);
			Vector diagonal(cmax_ - cmin_);
			return fluo::Max(diagonal.x(), diagonal.y(), diagonal.z());
		}

		inline double shortest_edge() const
		{
			assert(is_valid_);
			Vector diagonal(cmax_ - cmin_);
			return fluo::Min(diagonal.x(), diagonal.y(), diagonal.z());
		}

		//! Move the bounding box 
		inline void translate(const Vector& v)
		{
			cmin_ += v;
			cmax_ += v;
		}

		//! Scale the bounding box by s, centered around o
		void scale(double s, const Vector& o);
		//scale around origin
		void scale(double sx, double sy, double sz);
		//scale around center
		void scale_center(double sx, double sy, double sz);

		//access
		inline void minx(double v) { cmin_.x(v); }
		inline void miny(double v) { cmin_.y(v); }
		inline void minz(double v) { cmin_.z(v); }
		inline void maxx(double v) { cmax_.x(v); }
		inline void maxy(double v) { cmax_.y(v); }
		inline void maxz(double v) { cmax_.z(v); }
		inline double minx() const { return cmin_.x(); }
		inline double miny() const { return cmin_.y(); }
		inline double minz() const { return cmin_.z(); }
		inline double maxx() const { return cmax_.x(); }
		inline double maxy() const { return cmax_.y(); }
		inline double maxz() const { return cmax_.z(); }
		inline int minintx() const { return static_cast<int>(std::round(cmin_.x())); }
		inline int mininty() const { return static_cast<int>(std::round(cmin_.y())); }
		inline int minintz() const { return static_cast<int>(std::round(cmin_.z())); }
		inline int maxintx() const { return static_cast<int>(std::round(cmax_.x())); }
		inline int maxinty() const { return static_cast<int>(std::round(cmax_.y())); }
		inline int maxintz() const { return static_cast<int>(std::round(cmax_.z())); }

		inline Point Min() const
		{
			return cmin_;
		}

		inline Point Max() const
		{
			return cmax_;
		}

		inline Vector diagonal() const
		{
			assert(is_valid_); return cmax_ - cmin_;
		}

		inline bool inside(const Point& p) const
		{
			return (is_valid_ && p.x() >= cmin_.x() &&
				p.y() >= cmin_.y() && p.z() >= cmin_.z() &&
				p.x() <= cmax_.x() && p.y() <= cmax_.y() &&
				p.z() <= cmax_.z());
		}

		//! bbox's that share a face overlap
		bool overlaps(const BBox& bb) const;
		//! bbox's that share a face do not overlap_inside
		bool overlaps_inside(const BBox& bb) const;

		//! returns true if the ray hit the bbox and returns the hit point
		//! in hitNear
		bool intersect(const Point& e, const Vector& v, Point& hitNear);

		//if it intersects another bbox
		bool intersect(const BBox& box) const;

		//distance between two bboxes
		double distance(const BBox& bb) const;

		friend std::ostream& operator<<(std::ostream& os, const BBox& b)
		{
			os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
			os << '[' << b.cmin_ << ',' << b.cmax_ << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, BBox& b)
		{
			Point min, max;
			char st;
			is >> st >> min >> st >> max >> st;
			b = BBox(min, max);
			return is;
		}

	private:
		Point cmin_;
		Point cmax_;
		bool is_valid_;
	};

	//overlap region between two boxes
	inline BBox intersect(const BBox& box1, const BBox& box2)
	{
		BBox ibox;
		if (!box1.valid() || !box2.valid())
			return ibox;
		if (box1.Max().x() < box2.Min().x())
			return ibox;
		if (box1.Min().x() > box2.Max().x())
			return ibox;
		if (box1.Max().y() < box2.Min().y())
			return ibox;
		if (box1.Min().y() > box2.Max().y())
			return ibox;
		if (box1.Max().z() < box2.Min().z())
			return ibox;
		if (box1.Min().z() > box2.Max().z())
			return ibox;
		Point min = Max(box1.Min(), box2.Min());
		Point max = Min(box1.Max(), box2.Max());
		ibox = BBox(min, max);
		return ibox;
	}

	inline	bool BBox::intersect(const BBox& box) const
	{
		if (!valid() || !box.valid())
			return false;
		if (Max().x() < box.Min().x())
			return false;
		if (Min().x() > box.Max().x())
			return false;
		if (Max().y() < box.Min().y())
			return false;
		if (Min().y() > box.Max().y())
			return false;
		if (Max().z() < box.Min().z())
			return false;
		if (Min().z() > box.Max().z())
			return false;
		return true;
	}
} // End namespace fluo

#endif//_FLBBOX_H_
