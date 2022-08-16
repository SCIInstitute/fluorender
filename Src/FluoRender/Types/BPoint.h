/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#ifndef _FLBPoint_h_
#define _FLBPoint_h_

#include "Point.h"

#include <ostream>

namespace fluo
{
	class Vector;

	class BPoint
	{
	public:

		BPoint() :
			is_valid_(false)
		{}

		~BPoint()
		{}

		BPoint(const BPoint& copy)
			: is_valid_(copy.is_valid_),
			xmin_(copy.xmin_), xmax_(copy.xmax_),
			ymin_(copy.ymin_), ymax_(copy.ymax_),
			zmin_(copy.zmin_), zmax_(copy.zmax_)
		{}

		BPoint& operator=(const BPoint& copy)
		{
			is_valid_ = copy.is_valid_;
			xmin_ = copy.xmin_; xmax_ = copy.xmax_;
			ymin_ = copy.ymin_; ymax_ = copy.ymax_;
			zmin_ = copy.zmin_; zmax_ = copy.zmax_;
			return *this;
		}

		inline bool operator==(const BPoint& bbox) const
		{
			return (xmin_ == bbox.xmin_) && (xmax_ == bbox.xmax_) &&
				(ymin_ == bbox.ymin_) && (ymax_ == bbox.ymax_) &&
				(zmin_ == bbox.zmin_) && (zmax_ == bbox.zmax_);
		}

		inline bool operator!=(const BPoint& bbox) const
		{
			return (xmin_ != bbox.xmin_) || (xmax_ != bbox.xmax_) ||
				(ymin_ != bbox.ymin_) || (ymax_ != bbox.ymax_) ||
				(zmin_ != bbox.zmin_) || (zmax_ != bbox.zmax_);
		}

		BPoint(const Point& xmin, const Point& xmax,
			const Point& ymin, const Point& ymax,
			const Point& zmin, const Point& zmax)
			: is_valid_(true),
			xmin_(xmin), xmax_(xmax),
			ymin_(ymin), ymax_(ymax),
			zmin_(zmin), zmax_(zmax)
		{}

		inline int valid() const { return is_valid_; }

		inline void set_valid(bool v) { is_valid_ = v; }
		inline void reset() { is_valid_ = false; }

		inline Point get(int i)
		{
			switch (i)
			{
			case 0:
				return xmin_;
			case 1:
				return xmax_;
			case 2:
				return ymin_;
			case 3:
				return ymax_;
			case 4:
				return zmin_;
			case 5:
				return zmax_;
			}
			return Point();
		}

		//! Expand the bounding box to include point p
		inline void extend(const Point& p)
		{
			if (is_valid_)
			{
				if (p.x() < xmin_.x()) xmin_ = p;
				if (p.x() > xmax_.x()) xmax_ = p;
				if (p.y() < ymin_.y()) ymin_ = p;
				if (p.y() > ymax_.y()) ymax_ = p;
				if (p.z() < zmin_.z()) zmin_ = p;
				if (p.z() > zmax_.z()) zmax_ = p;
			}
			else
			{
				xmin_ = p;
				xmax_ = p;
				ymin_ = p;
				ymax_ = p;
				zmin_ = p;
				zmax_ = p;
				is_valid_ = true;
			}
		}

		inline void extend(const BPoint& bp)
		{
			if (is_valid_)
			{
				extend(bp.xmin_);
				extend(bp.xmax_);
				extend(bp.ymin_);
				extend(bp.ymax_);
				extend(bp.zmin_);
				extend(bp.zmax_);
			}
			else
			{
				xmin_ = bp.xmin_;
				xmax_ = bp.xmax_;
				ymin_ = bp.ymin_;
				ymax_ = bp.ymax_;
				zmin_ = bp.zmin_;
				zmax_ = bp.zmax_;
				is_valid_ = bp.is_valid_;
			}
		}

		//x, y, z size
		inline Point size() const
		{
			assert(is_valid_);
			return Point(xmax_.x() - xmin_.x(),
				ymax_.y() - ymin_.y(),
				zmax_.z() - zmin_.z());
		}

		inline Point center() const
		{
			assert(is_valid_);
			return Point(xmax_.x() + xmin_.x(),
				ymax_.y() + ymin_.y(),
				zmax_.z() + zmin_.z()) / 2;
		}

		friend std::ostream& operator<<(std::ostream& os, const BPoint& b)
		{
			//os << '[' << b.xmin_ << ',' << b.xmax_ << ',' << b.ymin_ << ',' << b.ymax_ << ',' << b.zmin_ << ',' << b.zmax_ << ']';
			return os;
		}
		friend std::ostream& operator>>(std::ostream& is, const BPoint& b)
		{
			//Point xmin, xmax, ymin, ymax, zmin, zmax;
			//char st;
			//is >> st >> xmin >> st >> xmax >> st >> ymin >> st >> ymax >> st >> zmin >> st >> zmax >> st;
			//b = BPoint(xmin, xmax, ymin, ymax, zmin, zmax);
			return is;
		}

	private:
		Point xmin_;
		Point xmax_;
		Point ymin_;
		Point ymax_;
		Point zmin_;
		Point zmax_;
		bool is_valid_;
	};

} // End namespace fluo

#endif
