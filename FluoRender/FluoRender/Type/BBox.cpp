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

#include <BBox.h>
#include <Vector.h>
#include <assert.h>
#include <math.h>

using namespace fluo;

void BBox::extend_disk(const Point& cen, const Vector& normal, double r)
{
	if (normal.length2() < 1.e-12)
	{
		extend(cen); return;
	}
	Vector n(normal);
	n.safe_normalize();
	double x=sqrt(1-n.x())*r;
	double y=sqrt(1-n.y())*r;
	double z=sqrt(1-n.z())*r;
	extend(cen+Vector(x,y,z));
	extend(cen-Vector(x,y,z));
}

void BBox::scale(double s, const Vector&o)
{
	cmin_-=o;
	cmax_-=o;
	cmin_*=s;
	cmax_*=s;
	cmin_+=o;
	cmax_+=o;
}

void BBox::scale(double sx, double sy, double sz)
{
	cmin_.scale(sx, sy, sz);
	cmax_.scale(sx, sy, sz);
}

void BBox::scale(const Vector& scale)
{
	cmin_.scale(scale);
	cmax_.scale(scale);
}

void BBox::scale_center(double sx, double sy, double sz)
{
	Point c = center();
	cmin_ -= c;
	cmax_ -= c;
	cmin_.scale(sx, sy, sz);
	cmax_.scale(sx, sy, sz);
	cmin_ += c;
	cmax_ += c;
}

void BBox::scale_center(const Vector& scale)
{
	Point c = center();
	cmin_ -= c;
	cmax_ -= c;
	cmin_.scale(scale);
	cmax_.scale(scale);
	cmin_ += c;
	cmax_ += c;
}

bool BBox::overlaps(const BBox & bb) const
{
	if( bb.cmin_.x() > cmax_.x() || bb.cmax_.x() < cmin_.x())
		return false;
	else if( bb.cmin_.y() > cmax_.y() || bb.cmax_.y() < cmin_.y())
		return false;
	else if( bb.cmin_.z() > cmax_.z() || bb.cmax_.z() < cmin_.z())
		return false;

	return true;
}

bool BBox::overlaps_inside(const BBox & bb) const
{
	if( bb.cmin_.x() >= cmax_.x() || bb.cmax_.x() <= cmin_.x())
		return false;
	else if( bb.cmin_.y() >= cmax_.y() || bb.cmax_.y() <= cmin_.y())
		return false;
	else if( bb.cmin_.z() >= cmax_.z() || bb.cmax_.z() <= cmin_.z())
		return false;

	return true;
}

bool BBox::intersect(const Point& origin, const Vector& dir,
					 Point& hitPoint)
{
	Vector t1 = (cmin_ - origin) / dir;
	Vector t2 = (cmax_ - origin) / dir;
	Vector tn = fluo::Min(t1, t2);
	Vector tf = fluo::Max(t1, t2);
	double tnear = tn.maxComponent();
	double tfar = tf.minComponent();
	if(tnear <= tfar)
	{
		hitPoint = origin + dir*tnear;
		return true;
	}
	else
	{
		return false;
	}
}

double BBox::distance(const BBox& bb) const
{
	Point p1 = center();
	Point p2 = bb.center();
	return (p1-p2).length();
}
