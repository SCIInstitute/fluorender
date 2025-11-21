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

	class ClippingBox : public PlaneSet
	{
	public:
		// Constructors
		ClippingBox();
		ClippingBox(const BBox& box);
		ClippingBox(const ClippingBox& copy);
		~ClippingBox();

		ClippingBox& operator=(const ClippingBox& cb);
		bool operator==(const ClippingBox& rhs) const;
		bool operator!=(const ClippingBox& rhs) const;

		// Access to the bounding box
		inline const BBox& GetBBox() const { return bbox_; }
		inline void SetBBox(const BBox& box) { bbox_ = box; Update(); }
		inline Vector GetSize() const { return bbox_.diagonal(); }

		// Update planes from the bbox
		void Update();

		void ResetClip();
		void ResetRotation();
		void ResetAll();

		//clip
		void SetClip(ClipPlane plane, int value);
		int GetClip(ClipPlane plane) const;
		void SetClipPair(ClipPlane axis, int val1, int val2);
		void GetClipPair(ClipPlane axis, int& val1, int& val2) const;
		void SetAllClip(const int val[6]);
		void GetAllClip(int val[6]) const;

		// Transformations
		void SetZeroRotation(const Quaternion& q) { m_q_zero = q; }
		Quaternion GetZeroRotation() const { return m_q_zero; }
		Vector GetZeroEuler();
		void Rotate(const Quaternion& q);
		Vector GetEuler();

		// Test if a point is inside the clipping box
		bool Contains(const Point& p) const;

		friend std::ostream& operator<<(std::ostream& os, const ClippingBox& cb)
		{
			os << "ClippingBox: " << cb.GetBBox() << " planes: " << static_cast<const PlaneSet&>(cb);
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
		BBox bbox_;   // the finite bounding box that defines the clipping region

		Quaternion m_q;
		Quaternion m_q_zero;
		Vector m_rot;
	};

} // namespace fluo

#endif
