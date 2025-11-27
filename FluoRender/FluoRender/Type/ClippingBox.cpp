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
#include <Transform.h>

using namespace fluo;

// Default constructor: empty bbox, six planes
ClippingBox::ClippingBox() :
	planes_world_(6),
	planes_unit_(6),
	planes_index_(6)
{
}

// Construct from a bbox
ClippingBox::ClippingBox(const BBox& box, const BBox& index_box) :
	planes_world_(6),
	planes_unit_(6),
	planes_index_(6),
	bbox_world_(box)
{
	if (index_box.valid())
		bbox_index_ = index_box;
	else
		bbox_index_ = bbox_world_;
	clips_world_ = bbox_world_;
	clips_index_ = bbox_index_;
	clips_unit_.unit();
	Update();
}

// Copy constructor
ClippingBox::ClippingBox(const ClippingBox& copy) :
	planes_world_(copy.planes_world_),
	planes_unit_(copy.planes_unit_),
	planes_index_(copy.planes_index_),
	bbox_world_(copy.bbox_world_),
	bbox_index_(copy.bbox_index_),
	clips_world_(copy.clips_world_),
	clips_index_(copy.clips_index_),
	clips_unit_(copy.clips_unit_),
	links_(copy.links_),
	linked_ds_world_(copy.linked_ds_world_),
	linked_ds_index_(copy.linked_ds_index_),
	linked_ds_unit_(copy.linked_ds_unit_),
	q_(copy.q_),
	euler_(copy.euler_)
{
}

// Assignment
ClippingBox& ClippingBox::operator=(const ClippingBox& cb)
{
	if (this != &cb)
	{
		planes_world_ = cb.planes_world_;
		planes_unit_ = cb.planes_unit_;
		planes_index_ = cb.planes_index_;
		bbox_world_ = cb.bbox_world_;
		bbox_index_ = cb.bbox_index_;
		clips_world_ = cb.clips_world_;
		clips_index_ = cb.clips_index_;
		clips_unit_ = cb.clips_unit_;
		links_ = cb.links_;
		linked_ds_unit_ = cb.linked_ds_unit_;
		q_ = cb.q_;
		euler_ = cb.euler_;
	}
	return *this;
}

bool ClippingBox::operator==(const ClippingBox& rhs) const
{
	// Compare planes
	if (!(planes_world_ == rhs.planes_world_))
		return false;
	if (!(planes_unit_ == rhs.planes_unit_))
		return false;
	if (!(planes_index_ == rhs.planes_index_))
		return false;

	// Compare bbox
	if (!(bbox_world_ == rhs.bbox_world_))
		return false;
	if (!(bbox_index_ == rhs.bbox_index_))
		return false;

	// Compare clips
	if (clips_unit_ != rhs.clips_unit_)
		return false;

	//links
	if (links_ != rhs.links_)
		return false;
	if (linked_ds_unit_ != rhs.linked_ds_unit_)
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

// Sync this clipping box to match another box's WORLD clipping and rotation,
// but keep this object's own bounding boxes.
void ClippingBox::SyncWorld(const ClippingBox& src)
{
	// --- Copy world-space clipping values ---
	clips_world_ = src.clips_world_;

	// --- Copy rotation (quaternion is canonical) ---
	q_ = src.q_;

	// --- Recompute unit-space clipping from world-space ---
	{
		const Point wmin = bbox_world_.Min();
		const Vector wsize = bbox_world_.diagonal();

		const Point cmin = clips_world_.Min();
		const Point cmax = clips_world_.Max();

		// Normalize into [0,1] unit space
		Point umin(
			(cmin.x() - wmin.x()) / wsize.x(),
			(cmin.y() - wmin.y()) / wsize.y(),
			(cmin.z() - wmin.z()) / wsize.z()
		);

		Point umax(
			(cmax.x() - wmin.x()) / wsize.x(),
			(cmax.y() - wmin.y()) / wsize.y(),
			(cmax.z() - wmin.z()) / wsize.z()
		);

		clips_unit_ = BBox(umin, umax);
	}

	// --- Recompute index-space clipping from unit-space ---
	{
		const Point imin = bbox_index_.Min();
		const Vector isize = bbox_index_.diagonal();

		const Point umin = clips_unit_.Min();
		const Point umax = clips_unit_.Max();

		Point cimin(
			imin.x() + umin.x() * isize.x(),
			imin.y() + umin.y() * isize.y(),
			imin.z() + umin.z() * isize.z()
		);

		Point cimax(
			imin.x() + umax.x() * isize.x(),
			imin.y() + umax.y() * isize.y(),
			imin.z() + umax.z() * isize.z()
		);

		clips_index_ = BBox(cimin, cimax);
	}

	// --- Rebuild planes in all spaces ---
	Update();
}

void ClippingBox::Update()
{
	planes_world_.resize(6);
	planes_index_.resize(6);
	planes_unit_.resize(6);

	// Canonical clips in UNIT space
	const Point uMin = clips_unit_.Min();
	const Point uMax = clips_unit_.Max();
	const Point uCtr((uMin.x() + uMax.x()) * 0.5,
		(uMin.y() + uMax.y()) * 0.5,
		(uMin.z() + uMax.z()) * 0.5);

	// Denormalize clips for bookkeeping
	clips_world_ = clips_unit_.denormalized(bbox_world_);
	clips_index_ = clips_unit_.denormalized(bbox_index_);

	// Build WORLD planes with dummy points (only normals matter initially)
	planes_world_[0].ChangePlane(Point(0, 0, 0), Vector(1, 0, 0));
	planes_world_[1].ChangePlane(Point(0, 0, 0), Vector(-1, 0, 0));
	planes_world_[2].ChangePlane(Point(0, 0, 0), Vector(0, 1, 0));
	planes_world_[3].ChangePlane(Point(0, 0, 0), Vector(0, -1, 0));
	planes_world_[4].ChangePlane(Point(0, 0, 0), Vector(0, 0, 1));
	planes_world_[5].ChangePlane(Point(0, 0, 0), Vector(0, 0, -1));

	// Rotate WORLD planes around world center
	const Point cW = bbox_world_.center();
	for (auto& pw : planes_world_)
		pw.RotateAroundPoint(q_, cW);

	// World -> Unit affine parameters
	BBox unitBox; unitBox.unit();
	const Vector S_WU = bbox_world_.scale_to(unitBox);
	const Vector t_WU = Vector(unitBox.Min());

	// Convert rotated WORLD planes to UNIT planes
	for (int k = 0; k < 6; ++k)
		planes_unit_[k] = TransformPlaneAffine(planes_world_[k], S_WU, t_WU);

	// Apply unit-space clipping by setting d to pass through unit clip faces
	std::array<Point, 6> unitFacePoints = {
		Point(uMin.x(), uCtr.y(), uCtr.z()),
		Point(uMax.x(), uCtr.y(), uCtr.z()),
		Point(uCtr.x(), uMin.y(), uCtr.z()),
		Point(uCtr.x(), uMax.y(), uCtr.z()),
		Point(uCtr.x(), uCtr.y(), uMin.z()),
		Point(uCtr.x(), uCtr.y(), uMax.z())
	};
	for (int k = 0; k < 6; ++k)
		planes_unit_[k].ChangeD(Dot(Vector(unitFacePoints[k].x(),
			unitFacePoints[k].y(),
			unitFacePoints[k].z()), planes_unit_[k].n()));

	// Unit -> Index / World affine parameters
	const Vector S_UI = unitBox.scale_to(bbox_index_);
	const Vector t_UI = Vector(bbox_index_.Min());
	const Vector S_UW = unitBox.scale_to(bbox_world_);
	const Vector t_UW = Vector(bbox_world_.Min());

	// Generate INDEX and WORLD clipped planes from UNIT planes
	for (int k = 0; k < 6; ++k) {
		planes_index_[k] = TransformPlaneAffine(planes_unit_[k], S_UI, t_UI);
		planes_world_[k] = TransformPlaneAffine(planes_unit_[k], S_UW, t_UW);
	}

	// Sync Euler angles
	euler_ = q_.ToEuler();
}

void ClippingBox::SetLink(ClipPlane plane, bool link)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
	case ClipPlane::XPos:
		links_[0] = link;
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		links_[1] = link;
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		links_[2] = link;
		break;
	}
}

bool ClippingBox::GetLink(ClipPlane plane)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
	case ClipPlane::XPos:
		return links_[0];
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		return links_[1];
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		return links_[2];
	}
	return false;
}

void ClippingBox::ResetLink()
{
	links_.fill(false);
}

void ClippingBox::ResetLinkedDist()
{
	double dist = 1.0;
	linked_ds_index_.fill(dist);
	for (int i = 0; i < 3; ++i)
	{
		linked_ds_world_[i] = IndexToWorldDist(i, dist);
		linked_ds_unit_[i] = IndexToUnitDist(i, dist);
	}
}

void ClippingBox::SetLinkedDistWorld(ClipPlane plane, double dist)
{
	int axis = (plane == ClipPlane::XNeg || plane == ClipPlane::XPos) ? 0 :
		(plane == ClipPlane::YNeg || plane == ClipPlane::YPos) ? 1 : 2;

	linked_ds_world_[axis] = dist;
	linked_ds_index_[axis] = WorldToIndexDist(axis, dist);
	linked_ds_unit_[axis] = IndexToUnitDist(axis, linked_ds_index_[axis]);
}

double ClippingBox::GetLinkedDistWorld(ClipPlane plane)
{
	int axis = (plane == ClipPlane::XNeg || plane == ClipPlane::XPos) ? 0 :
		(plane == ClipPlane::YNeg || plane == ClipPlane::YPos) ? 1 : 2;
	return linked_ds_world_[axis];
}

void ClippingBox::SetLinkedDistIndex(ClipPlane plane, double dist)
{
	int axis = (plane == ClipPlane::XNeg || plane == ClipPlane::XPos) ? 0 :
		(plane == ClipPlane::YNeg || plane == ClipPlane::YPos) ? 1 : 2;

	linked_ds_index_[axis] = dist;
	linked_ds_world_[axis] = IndexToWorldDist(axis, dist);
	linked_ds_unit_[axis] = IndexToUnitDist(axis, dist);
}

double ClippingBox::GetLinkedDistIndex(ClipPlane plane)
{
	int axis = (plane == ClipPlane::XNeg || plane == ClipPlane::XPos) ? 0 :
		(plane == ClipPlane::YNeg || plane == ClipPlane::YPos) ? 1 : 2;
	return linked_ds_index_[axis];
}

void ClippingBox::ResetClips()
{
	clips_world_ = bbox_world_;
	clips_index_ = bbox_index_;
	clips_unit_.unit();
	Update();
}

void ClippingBox::ResetClips(ClipPlane plane)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
	case ClipPlane::XPos:
		clips_world_.minx(bbox_world_.minx());
		clips_world_.maxx(bbox_world_.maxx());
		clips_index_.minx(bbox_index_.minx());
		clips_index_.maxx(bbox_index_.maxx());
		clips_unit_.minx(0.0);
		clips_unit_.maxx(1.0);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		clips_world_.miny(bbox_world_.miny());
		clips_world_.maxy(bbox_world_.maxy());
		clips_index_.miny(bbox_index_.miny());
		clips_index_.maxy(bbox_index_.maxy());
		clips_unit_.miny(0.0);
		clips_unit_.maxy(1.0);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		clips_world_.minz(bbox_world_.minz());
		clips_world_.maxz(bbox_world_.maxz());
		clips_index_.minz(bbox_index_.minz());
		clips_index_.maxz(bbox_index_.maxz());
		clips_unit_.minz(0.0);
		clips_unit_.maxz(1.0);
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
	clips_world_ = bbox_world_;
	clips_index_ = bbox_index_;
	clips_unit_.unit();
	q_ = Quaternion();
	links_.fill(false);
	Update();
}

void ClippingBox::SetClipWorld(ClipPlane plane, double value)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
		clips_world_.minx(value);
		if (links_[0])
			clips_world_.maxx(value + linked_ds_world_[0]);
		break;
	case ClipPlane::XPos:
		clips_world_.maxx(value);
		if (links_[0])
			clips_world_.minx(value - linked_ds_world_[0]);
		break;
	case ClipPlane::YNeg:
		clips_world_.miny(value);
		if (links_[1])
			clips_world_.maxy(value + linked_ds_world_[1]);
		break;
	case ClipPlane::YPos:
		clips_world_.maxy(value);
		if (links_[1])
			clips_world_.miny(value - linked_ds_world_[1]);
		break;
	case ClipPlane::ZNeg:
		clips_world_.minz(value);
		if (links_[2])
			clips_world_.maxz(value + linked_ds_world_[2]);
		break;
	case ClipPlane::ZPos:
		clips_world_.maxz(value);
		if (links_[2])
			clips_world_.minz(value - linked_ds_world_[2]);
		break;
	}

	// --- Sync unit and index clips ---
	clips_unit_ = clips_world_.normalized(bbox_world_);
	clips_index_ = clips_unit_.denormalized(bbox_index_);

	Update();
}

double ClippingBox::GetClipWorld(ClipPlane plane) const
{
	switch (plane)
	{
	case ClipPlane::XNeg: return clips_world_.minx();
	case ClipPlane::XPos: return clips_world_.maxx();
	case ClipPlane::YNeg: return clips_world_.miny();
	case ClipPlane::YPos: return clips_world_.maxy();
	case ClipPlane::ZNeg: return clips_world_.minz();
	case ClipPlane::ZPos: return clips_world_.maxz();
	}
	return 0.0;
}

void ClippingBox::SetClipPairWorld(ClipPlane axis, double val1, double val2)
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		clips_world_.minx(val1);
		clips_world_.maxx(val2);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		clips_world_.miny(val1);
		clips_world_.maxy(val2);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		clips_world_.minz(val1);
		clips_world_.maxz(val2);
		break;
	}

	// --- Sync unit and index clips ---
	clips_unit_ = clips_world_.normalized(bbox_world_);
	clips_index_ = clips_unit_.denormalized(bbox_index_);

	Update();
}

void ClippingBox::GetClipPairWorld(ClipPlane axis, double& val1, double& val2) const
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		val1 = GetClipWorld(ClipPlane::XNeg);
		val2 = GetClipWorld(ClipPlane::XPos);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		val1 = GetClipWorld(ClipPlane::YNeg);
		val2 = GetClipWorld(ClipPlane::YPos);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		val1 = GetClipWorld(ClipPlane::ZNeg);
		val2 = GetClipWorld(ClipPlane::ZPos);
		break;
	}
}

void ClippingBox::SetAllClipsWorld(const double val[6])
{
	clips_world_.minx(val[0]);
	clips_world_.maxx(val[1]);
	clips_world_.miny(val[2]);
	clips_world_.maxy(val[3]);
	clips_world_.minz(val[4]);
	clips_world_.maxz(val[5]);

	// --- Sync unit and index clips ---
	clips_unit_ = clips_world_.normalized(bbox_world_);
	clips_index_ = clips_unit_.denormalized(bbox_index_);

	Update();
}

void ClippingBox::GetAllClipsWorld(double val[6]) const
{
	for (int i = 0; i < 6; ++i)
		val[i] = GetClipWorld(static_cast<ClipPlane>(i));
}

void ClippingBox::SetClipIndex(ClipPlane plane, double value)
{
	switch (plane)
	{
	case ClipPlane::XNeg:
		clips_index_.minx(value);
		if (links_[0])
			clips_index_.maxx(value + linked_ds_index_[0]);
		break;
	case ClipPlane::XPos:
		clips_index_.maxx(value);
		if (links_[0])
			clips_index_.minx(value - linked_ds_index_[0]);
		break;
	case ClipPlane::YNeg:
		clips_index_.miny(value);
		if (links_[1])
			clips_index_.maxy(value + linked_ds_index_[1]);
		break;
	case ClipPlane::YPos:
		clips_index_.maxy(value);
		if (links_[1])
			clips_index_.miny(value - linked_ds_index_[1]);
		break;
	case ClipPlane::ZNeg:
		clips_index_.minz(value);
		if (links_[2])
			clips_index_.maxz(value + linked_ds_index_[2]);
		break;
	case ClipPlane::ZPos:
		clips_index_.maxz(value);
		if (links_[2])
			clips_index_.minz(value - linked_ds_index_[2]);
		break;
	}

	// --- Sync unit and index clips ---
	clips_unit_ = clips_index_.normalized(bbox_index_);
	clips_world_ = clips_unit_.denormalized(bbox_world_);

	Update();
}

double ClippingBox::GetClipIndex(ClipPlane plane) const
{
	switch (plane)
	{
	case ClipPlane::XNeg: return clips_index_.minx();
	case ClipPlane::XPos: return clips_index_.maxx();
	case ClipPlane::YNeg: return clips_index_.miny();
	case ClipPlane::YPos: return clips_index_.maxy();
	case ClipPlane::ZNeg: return clips_index_.minz();
	case ClipPlane::ZPos: return clips_index_.maxz();
	}
	return 0.0;
}

void ClippingBox::SetClipPairIndex(ClipPlane axis, double val1, double val2)
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		clips_index_.minx(val1);
		clips_index_.maxx(val2);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		clips_index_.miny(val1);
		clips_index_.maxy(val2);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		clips_index_.minz(val1);
		clips_index_.maxz(val2);
		break;
	}

	// --- Sync unit and index clips ---
	clips_unit_ = clips_index_.normalized(bbox_index_);
	clips_world_ = clips_unit_.denormalized(bbox_world_);

	Update();
}

void ClippingBox::GetClipPairIndex(ClipPlane axis, double& val1, double& val2) const
{
	switch (axis)
	{
	case ClipPlane::XNeg: // treat as X axis pair
	case ClipPlane::XPos:
		val1 = GetClipIndex(ClipPlane::XNeg);
		val2 = GetClipIndex(ClipPlane::XPos);
		break;
	case ClipPlane::YNeg:
	case ClipPlane::YPos:
		val1 = GetClipIndex(ClipPlane::YNeg);
		val2 = GetClipIndex(ClipPlane::YPos);
		break;
	case ClipPlane::ZNeg:
	case ClipPlane::ZPos:
		val1 = GetClipIndex(ClipPlane::ZNeg);
		val2 = GetClipIndex(ClipPlane::ZPos);
		break;
	}
}

void ClippingBox::SetAllClipsIndex(const double val[6])
{
	clips_index_.minx(val[0]);
	clips_index_.maxx(val[1]);
	clips_index_.miny(val[2]);
	clips_index_.maxy(val[3]);
	clips_index_.minz(val[4]);
	clips_index_.maxz(val[5]);

	// --- Sync unit and index clips ---
	clips_unit_ = clips_index_.normalized(bbox_index_);
	clips_world_ = clips_unit_.denormalized(bbox_world_);

	Update();
}

void ClippingBox::GetAllClipsIndex(double val[6]) const
{
	for (int i = 0; i < 6; ++i)
		val[i] = GetClipIndex(static_cast<ClipPlane>(i));
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
bool ClippingBox::ContainsWorld(const Point& p) const
{
	for (const auto& plane : planes_world_)
	{
		if (plane.eval_point(p) > 0.0) // point is outside
			return false;
	}
	return true;
}

bool ClippingBox::ContainsIndex(const Point& p) const
{
	for (const auto& plane : planes_index_)
	{
		if (plane.eval_point(p) > 0.0) // point is outside
			return false;
	}
	return true;
}

bool ClippingBox::ContainsUnit(const Point& p) const
{
	for (const auto& plane : planes_unit_)
	{
		if (plane.eval_point(p) > 0.0) // point is outside
			return false;
	}
	return true;
}

