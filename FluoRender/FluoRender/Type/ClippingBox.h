//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
#ifndef _CLIPPINGBOX_H_
#define _CLIPPINGBOX_H_

#include <Plane.h>
#include <BBox.h>
#include <Point.h>
#include <Vector.h>
#include <Quaternion.h>
#include <array>

namespace fluo
{
	enum class ClipPlane : int
	{
		XNeg = 0, // left
		XPos = 1, // right
		YNeg = 2, // bottom
		YPos = 3, // top
		ZNeg = 4, // near
		ZPos = 5  // far
	};

	class ClippingBox
	{
	public:
		// Constructors
		ClippingBox();
		ClippingBox(const BBox& world_box, const BBox& index_box = BBox());
		ClippingBox(const ClippingBox& copy);
		~ClippingBox() {}

		ClippingBox& operator=(const ClippingBox& cb);
		bool operator==(const ClippingBox& rhs) const;
		bool operator!=(const ClippingBox& rhs) const;

		void SyncWorld(const ClippingBox& src);

		// --- Plane access ---
		const PlaneSet& GetPlanesWorld() const { return planes_world_; }
		const PlaneSet& GetPlanesIndex() const { return planes_index_; }
		const PlaneSet& GetPlanesUnit()  const { return planes_unit_; }

		// --- Bounding boxes (dataset initialization) ---
		const BBox& GetBBoxWorld() const { return bbox_world_; }
		void SetBBoxes(const BBox& box_world, const BBox& box_index) { bbox_world_ = box_world; bbox_index_ = box_index; ResetClips(); }

		const BBox& GetBBoxIndex() const { return bbox_index_; }

		Vector GetWorldSize() const { return bbox_world_.diagonal(); }
		Vector GetIndexSize() const { return bbox_index_.diagonal(); }

		// --- Clip boxes (UI input) ---
		const BBox& GetClipsWorld() const { return clips_world_; }
		const BBox& GetClipsIndex() const { return clips_index_; }

		Vector GetClipWorldSize() const { return clips_world_.diagonal(); }
		Vector GetClipIndexSize() const { return clips_index_.diagonal(); }

		// --- Linking controls ---
		void SetLink(ClipPlane plane, bool link);
		bool GetLink(ClipPlane plane);
		void ResetLink();
		void ResetLinkedDist();

		void SetLinkedDistWorld(ClipPlane plane, double dist);
		double GetLinkedDistWorld(ClipPlane plane);

		void SetLinkedDistIndex(ClipPlane plane, double dist);
		double GetLinkedDistIndex(ClipPlane plane);

		// --- Reset helpers ---
		void ResetClips();
		void ResetClips(ClipPlane plane);
		void ResetRotation();
		void ResetAll();

		// --- Clip setters/getters (UI convenience) ---
		void SetClipWorld(ClipPlane plane, double value);
		double GetClipWorld(ClipPlane plane) const;
		void SetClipPairWorld(ClipPlane axis, double val1, double val2);
		void GetClipPairWorld(ClipPlane axis, double& val1, double& val2) const;
		void SetAllClipsWorld(const double val[6]);
		void GetAllClipsWorld(double val[6]) const;

		void SetClipIndex(ClipPlane plane, double value);
		double GetClipIndex(ClipPlane plane) const;
		void SetClipPairIndex(ClipPlane axis, double val1, double val2);
		void GetClipPairIndex(ClipPlane axis, double& val1, double& val2) const;
		void SetAllClipsIndex(const double val[6]);
		void GetAllClipsIndex(double val[6]) const;

		// --- Rotation ---
		void Rotate(const Quaternion& q);   // rotate by quaternion
		void Rotate(const Vector& euler);   // rotate by euler angles
		Quaternion GetRotation() const { return q_; }
		Vector GetEuler() const { return euler_; }

		// --- Point containment (world space) ---
		bool ContainsWorld(const Point& p) const;
		bool ContainsIndex(const Point& p) const;
		bool ContainsUnit(const Point& p) const;

		friend std::ostream& operator<<(std::ostream& os, const ClippingBox& cb)
		{
			os << "ClippingBox: world bbox " << cb.bbox_world_ << " planes: " << cb.planes_world_;
			return os;
		}
		friend std::istream& operator>>(std::istream& is, ClippingBox& cb)
		{
			BBox box_world, box_index;
			is >> box_world >> box_index;
			cb.SetBBoxes(box_world, box_index);
			return is;
		}

	protected:
		// --- Plane sets ---
		PlaneSet planes_world_;   // planes in world coordinates
		PlaneSet planes_index_;   // planes in voxel index space
		PlaneSet planes_unit_;    // planes in normalized coordinates

		// --- Bounding boxes (dataset initialization) ---
		BBox bbox_world_;         // finite bounding box in world coordinates
		BBox bbox_index_;         // bounding box in voxel index space

		// --- Clip boxes (UI input) ---
		BBox clips_world_;        // clip values in world coordinates
		BBox clips_index_;        // clip values in voxel index space
		BBox clips_unit_;         // internal canonical values

		// --- Linking state ---
		std::array<bool, 3> links_ = { false, false, false };
		std::array<double, 3> linked_ds_world_ = { 1.0, 1.0, 1.0 };
		std::array<double, 3> linked_ds_index_ = { 1.0, 1.0, 1.0 };
		std::array<double, 3> linked_ds_unit_ = { 1.0, 1.0, 1.0 };

		// --- Rotation ---
		Quaternion q_;
		Vector euler_;

	protected:
		void Update();

		// Convert a distance in world space to index space
		double WorldToIndexDist(int axis, double dist) const {
			Vector scale = bbox_index_.scale_to(bbox_world_);
			if (scale[axis] == 0.0) return 0.0;
			return dist / scale[axis];
		}

		// Convert a distance in index space to world space
		double IndexToWorldDist(int axis, double dist) const {
			Vector scale = bbox_index_.scale_to(bbox_world_);
			return dist * scale[axis];
		}

		// Convert a distance in index space to unit space
		double IndexToUnitDist(int axis, double dist) const {
			Vector extent = bbox_index_.diagonal();
			if (extent[axis] == 0.0) return 0.0;
			return dist / extent[axis];
		}

		// Convert a distance in unit space to index space
		double UnitToIndexDist(int axis, double dist) const {
			Vector extent = bbox_index_.diagonal();
			return dist * extent[axis];
		}
	};

} // namespace fluo

#endif
