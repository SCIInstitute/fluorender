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

namespace fluo
{
	enum class ClipPlane
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
		ClippingBox(const BBox& box);
		ClippingBox(const ClippingBox& copy);
		~ClippingBox() {}

		ClippingBox& operator=(const ClippingBox& cb);
		bool operator==(const ClippingBox& rhs) const;
		bool operator!=(const ClippingBox& rhs) const;

		//plane access
		const PlaneSet& GetPlanes() const { return planes_; }

		// Access to the bounding box
		inline const BBox& GetBBox() const { return bbox_; }
		inline void SetBBox(const BBox& box) { bbox_ = box; Update(); }
		inline Vector GetSize() const { return bbox_.diagonal(); }
		//clip
		inline const BBox& GetClipBox() const { return clips_; }

		void ResetClip();
		void ResetClip(ClipPlane plane);
		void ResetRotation();
		void ResetAll();

		//clip
		void SetClip(ClipPlane plane, double value);
		double GetClip(ClipPlane plane) const;
		void SetClipPair(ClipPlane axis, double val1, double val2);
		void GetClipPair(ClipPlane axis, double& val1, double& val2) const;
		void SetAllClip(const double val[6]);
		void GetAllClip(double val[6]) const;

		// Transformations
		void Rotate(const Quaternion& q);//rotate by quaternion
		void Rotate(const Vector& euler); //rotate by euler angles
		Quaternion GetRotation() const { return q_; }
		Vector GetEuler() const { return euler_; }

		// Test if a point is inside the clipping box in world coordinates
		bool Contains(const Point& p) const;

		friend std::ostream& operator<<(std::ostream& os, const ClippingBox& cb)
		{
			os << "ClippingBox: " << cb.GetBBox() << " planes: " << cb.planes_;
			return os;
		}
		friend std::istream& operator>>(std::istream& is, ClippingBox& cb)
		{
			BBox box;
			is >> box;
			cb.SetBBox(box);
			return is;
		}

	protected:
		PlaneSet planes_;//planes in world coordinates
		PlaneSet planes_normalized_;//planes in normalized coordinates

		BBox bbox_;   // the finite bounding box that defines the clipping region
		BBox clips_;  // the current clipping

		Quaternion q_;
		Vector euler_;

	protected:
		void Update();
	};

} // namespace fluo

#endif
