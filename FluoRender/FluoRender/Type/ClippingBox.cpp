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

#include <ClippingBox.h>

using namespace fluo;

// Default constructor: empty bbox, six planes
ClippingBox::ClippingBox()
	: PlaneSet(6),
	bbox_(BBox(Point(0.0), Point(1.0)))
{
	Update();
}

// Construct from a bbox
ClippingBox::ClippingBox(const BBox& box)
	: PlaneSet(6),
	bbox_(box)
{
	Update();
}

// Copy constructor
ClippingBox::ClippingBox(const ClippingBox& copy)
	: PlaneSet(copy),
	bbox_(copy.bbox_),
	m_q(copy.m_q),
	m_q_zero(copy.m_q_zero),
	m_rot(copy.m_rot)
{
}

ClippingBox::~ClippingBox() {}

// Assignment
ClippingBox& ClippingBox::operator=(const ClippingBox& cb)
{
	if (this != &cb)
	{
		PlaneSet::operator=(cb);
		bbox_ = cb.bbox_;
		m_q = cb.m_q;
		m_q_zero = cb.m_q_zero;
		m_rot = cb.m_rot;
	}
	return *this;
}

bool ClippingBox::operator==(const ClippingBox& rhs) const
{
	// Compare bbox
	if (!(bbox_ == rhs.bbox_))
		return false;

	// Compare planes (PlaneSet already has operator==)
	if (!PlaneSet::operator==(rhs))
		return false;

	// Compare rotation state
	if (!(m_q == rhs.m_q))
		return false;
	if (!(m_q_zero == rhs.m_q_zero))
		return false;

	return true;
}

bool ClippingBox::operator!=(const ClippingBox& rhs) const
{
	return !(*this == rhs);
}

// Update planes from bbox
void ClippingBox::Update()
{
	planes_.resize(6);

	const Point& minp = bbox_.Min();
	const Point& maxp = bbox_.Max();

	planes_[0].ChangePlane(minp, Vector(1, 0, 0));   // XNeg
	planes_[1].ChangePlane(maxp, Vector(-1, 0, 0));  // XPos
	planes_[2].ChangePlane(minp, Vector(0, 1, 0));   // YNeg
	planes_[3].ChangePlane(maxp, Vector(0, -1, 0));  // YPos
	planes_[4].ChangePlane(minp, Vector(0, 0, 1));   // ZNeg
	planes_[5].ChangePlane(maxp, Vector(0, 0, -1));  // ZPos
}

void ClippingBox::ResetClip()
{
	bbox_ = BBox(Point(0.0), Point(1.0));
	Update();
}

void ClippingBox::ResetRotation()
{
	m_q = m_q_zero; // restore neutral rotation
	m_rot = Vector(0.0);
	Update();       // reapply rotation if needed
}

void ClippingBox::ResetAll()
{
	ResetClip();
	ResetRotation();
}

void ClippingBox::SetClip(ClipPlane plane, int value)
{
	Vector size = GetSize();
	double clip;
	Point p;
	Vector v;

	switch (plane)
	{
	case ClipPlane::XNeg:
		clip = (double)value / size.x();
		p = Point(clip, 0, 0);
		v = Vector(1, 0, 0);
		planes_[0].ChangePlane(p, v);
		break;
	case ClipPlane::XPos:
		clip = (double)value / size.x();
		p = Point(clip, 0, 0);
		v = Vector(-1, 0, 0);
		planes_[1].ChangePlane(p, v);
		break;
	case ClipPlane::YNeg:
		clip = (double)value / size.y();
		p = Point(0, clip, 0);
		v = Vector(0, 1, 0);
		planes_[2].ChangePlane(p, v);
		break;
	case ClipPlane::YPos:
		clip = (double)value / size.y();
		p = Point(0, clip, 0);
		v = Vector(0, -1, 0);
		planes_[3].ChangePlane(p, v);
		break;
	case ClipPlane::ZNeg:
		clip = (double)value / size.z();
		p = Point(0, 0, clip);
		v = Vector(0, 0, 1);
		planes_[4].ChangePlane(p, v);
		break;
	}
}

int ClippingBox::GetClip(ClipPlane plane) const
{
	Vector size = GetSize();
	switch (plane)
	{
	case ClipPlane::XNeg: return std::round(-size.x() * planes_[0].d());
	case ClipPlane::XPos: return std::round(size.x() * planes_[1].d());
	case ClipPlane::YNeg: return std::round(-size.y() * planes_[2].d());
	case ClipPlane::YPos: return std::round(size.y() * planes_[3].d());
	case ClipPlane::ZNeg: return std::round(-size.z() * planes_[4].d());
	case ClipPlane::ZPos: return std::round(size.z() * planes_[5].d());
	}
	return 0;
}

void ClippingBox::SetClipPair(ClipPlane axis, int val1, int val2)
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		SetClip(ClipPlane::XNeg, val1);
		SetClip(ClipPlane::XPos, val2);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		SetClip(ClipPlane::YNeg, val1);
		SetClip(ClipPlane::YPos, val2);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		SetClip(ClipPlane::ZNeg, val1);
		SetClip(ClipPlane::ZPos, val2);
		break;
	}
}

void ClippingBox::GetClipPair(ClipPlane axis, int& val1, int& val2) const
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		val1 = GetClip(ClipPlane::XNeg);
		val2 = GetClip(ClipPlane::XPos);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		val1 = GetClip(ClipPlane::YNeg);
		val2 = GetClip(ClipPlane::YPos);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		val1 = GetClip(ClipPlane::ZNeg);
		val2 = GetClip(ClipPlane::ZPos);
		break;
	}
}

void ClippingBox::SetAllClip(const int val[6])
{
	for (int i = 0; i < 6; ++i)
		SetClip(static_cast<ClipPlane>(i), val[i]);
}

void ClippingBox::GetAllClip(int val[6]) const
{
	for (int i = 0; i < 6; ++i)
		val[i] = GetClip(static_cast<ClipPlane>(i));
}

Vector ClippingBox::GetZeroEuler()
{
	double x, y, z;
	m_q_zero.ToEuler(x, y, z);
	return Vector(x, y, z);
}

Vector ClippingBox::GetEuler()
{
	double x, y, z;
	m_q.ToEuler(x, y, z);
	return Vector(x, y, z);
}

void ClippingBox::Rotate(const Quaternion& q)
{
	m_q = q;
	// apply rotation to planes
	for (auto& plane : planes_)
		plane.Rotate(const_cast<Quaternion&>(m_q));
}

// Point containment test
bool ClippingBox::Contains(const Point& p) const
{
	for (const auto& plane : planes_)
	{
		if (plane.eval_point(p) > 0.0) // point is outside
			return false;
	}
	return true;
}

