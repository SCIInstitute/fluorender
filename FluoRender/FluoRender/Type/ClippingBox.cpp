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
ClippingBox::ClippingBox() :
	planes_(6),
	planes_normalized_(6)
{
}

// Construct from a bbox
ClippingBox::ClippingBox(const BBox& box) :
	planes_(6),
	planes_normalized_(6),
	bbox_(box)
{
	Update();
}

// Copy constructor
ClippingBox::ClippingBox(const ClippingBox& copy) :
	planes_(copy.planes_),
	planes_normalized_(copy.planes_normalized_),
	bbox_(copy.bbox_),
	clips_(copy.clips_),
	q_(copy.q_),
	euler_(copy.euler_)
{
}

// Assignment
ClippingBox& ClippingBox::operator=(const ClippingBox& cb)
{
	if (this != &cb)
	{
		planes_ = cb.planes_;
		planes_normalized_ = cb.planes_normalized_;
		bbox_ = cb.bbox_;
		clips_ = cb.clips_;
		q_ = cb.q_;
		euler_ = cb.euler_;
	}
	return *this;
}

bool ClippingBox::operator==(const ClippingBox& rhs) const
{
	// Compare planes
	if (!(planes_ == rhs.planes_))
		return false;

	// Compare normalized planes
	if (!(planes_normalized_ == rhs.planes_normalized_))
		return false;

	// Compare bbox
	if (!(bbox_ == rhs.bbox_))
		return false;

	// Compare clips
	if (clips_ != rhs.clips_)
		return false;

	// Compare quaternion
	if (!(q_ == rhs.q_))
		return false;

	return true;
}

bool ClippingBox::operator!=(const ClippingBox& rhs) const
{
	return !(*this == rhs);
}

// Protected: synchronize planes_, planes_normalized_, and euler_
// Canonical inputs: bbox_, clips_, q_
void ClippingBox::Update()
{
	// --- Ensure exactly 6 planes in both sets ---
	planes_.resize(6);
	planes_normalized_.resize(6);

	// --- World coordinate planes from clips_ ---
	const Point& minp = clips_.Min();
	const Point& maxp = clips_.Max();

	planes_[static_cast<int>(ClipPlane::XNeg)].ChangePlane(minp, Vector(1, 0, 0));
	planes_[static_cast<int>(ClipPlane::XPos)].ChangePlane(maxp, Vector(-1, 0, 0));
	planes_[static_cast<int>(ClipPlane::YNeg)].ChangePlane(minp, Vector(0, 1, 0));
	planes_[static_cast<int>(ClipPlane::YPos)].ChangePlane(maxp, Vector(0, -1, 0));
	planes_[static_cast<int>(ClipPlane::ZNeg)].ChangePlane(minp, Vector(0, 0, 1));
	planes_[static_cast<int>(ClipPlane::ZPos)].ChangePlane(maxp, Vector(0, 0, -1));

	// --- Rotate planes around bbox center ---
	Point center = bbox_.center();
	for (auto& plane : planes_)
		plane.RotatePoint(q_, center);

	// --- Normalized coordinate planes ---
	Vector diag = bbox_.diagonal();
	Point origin = bbox_.Min();

	auto normalizePoint = [&](const Point& p) -> Point {
		return Point(
			(p.x() - origin.x()) / diag.x(),
			(p.y() - origin.y()) / diag.y(),
			(p.z() - origin.z()) / diag.z()
		);
		};

	Point minNorm = normalizePoint(clips_.Min());
	Point maxNorm = normalizePoint(clips_.Max());

	planes_normalized_[static_cast<int>(ClipPlane::XNeg)].ChangePlane(minNorm, Vector(1, 0, 0));
	planes_normalized_[static_cast<int>(ClipPlane::XPos)].ChangePlane(maxNorm, Vector(-1, 0, 0));
	planes_normalized_[static_cast<int>(ClipPlane::YNeg)].ChangePlane(minNorm, Vector(0, 1, 0));
	planes_normalized_[static_cast<int>(ClipPlane::YPos)].ChangePlane(maxNorm, Vector(0, -1, 0));
	planes_normalized_[static_cast<int>(ClipPlane::ZNeg)].ChangePlane(minNorm, Vector(0, 0, 1));
	planes_normalized_[static_cast<int>(ClipPlane::ZPos)].ChangePlane(maxNorm, Vector(0, 0, -1));

	// --- Rotate normalized planes around (0.5,0.5,0.5) ---
	Point normCenter(0.5, 0.5, 0.5);
	for (auto& plane : planes_normalized_)
		plane.RotatePoint(q_, normCenter);

	// --- Sync Euler angles from quaternion ---
	euler_ = q_.ToEuler();
}

void ClippingBox::ResetClip()
{
	clips_ = bbox_;
	Update();
}

void ClippingBox::ResetClip(ClipPlane plane)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
	case ClipPlane::XPos:
		clips_.minx(bbox_.minx());
		clips_.maxx(bbox_.maxx());
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		clips_.miny(bbox_.miny());
		clips_.maxy(bbox_.maxy());
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		clips_.minz(bbox_.minz());
		clips_.maxz(bbox_.maxz());
	}
	Update();
}

void ClippingBox::ResetRotation()
{
	q_ = Quaternion();
	Update();
}

void ClippingBox::ResetAll()
{
	clips_ = bbox_;
	q_ = Quaternion();
	Update();
}

void ClippingBox::SetClip(ClipPlane plane, double value)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
		clips_.minx(value);
		break;
	case ClipPlane::XPos:
		clips_.maxx(value);
		break;
	case ClipPlane::YNeg:
		clips_.miny(value);
		break;
	case ClipPlane::YPos:
		clips_.maxy(value);
		break;
	case ClipPlane::ZNeg:
		clips_.minz(value);
		break;
	case ClipPlane::ZPos:
		clips_.maxz(value);
	}
	Update();
}

double ClippingBox::GetClip(ClipPlane plane) const
{
	switch (plane)
	{
	case ClipPlane::XNeg: return clips_.minx();
	case ClipPlane::XPos: return clips_.maxx();
	case ClipPlane::YNeg: return clips_.miny();
	case ClipPlane::YPos: return clips_.maxy();
	case ClipPlane::ZNeg: return clips_.minz();
	case ClipPlane::ZPos: return clips_.maxz();
	}
	return 0.0;
}

void ClippingBox::SetClipPair(ClipPlane axis, double val1, double val2)
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		clips_.minx(val1);
		clips_.maxx(val2);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		clips_.miny(val1);
		clips_.maxy(val2);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		clips_.minz(val1);
		clips_.maxz(val2);
		break;
	}
	Update();
}

void ClippingBox::GetClipPair(ClipPlane axis, double& val1, double& val2) const
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

void ClippingBox::SetAllClip(const double val[6])
{
	clips_.minx(val[0]);
	clips_.maxx(val[1]);
	clips_.miny(val[2]);
	clips_.maxy(val[3]);
	clips_.minz(val[4]);
	clips_.maxz(val[5]);
	Update();
}

void ClippingBox::GetAllClip(double val[6]) const
{
	for (int i = 0; i < 6; ++i)
		val[i] = GetClip(static_cast<ClipPlane>(i));
}

void ClippingBox::Rotate(const Quaternion& q)
{
	q_ = q;
	Update();
}

void ClippingBox::Rotate(const Vector& euler)
{
	euler_ = euler;
	q_.FromEuler(euler);
	Update();
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

