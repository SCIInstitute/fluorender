/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef _FLRAY_H_
#define _FLRAY_H_

#include <Types/Point.h>
#include <Types/Vector.h>

namespace FLTYPE
{

	class Ray
	{
		Point o_;
		Vector d_;
	public:
		//! Constructors
		Ray() {}
		Ray(const Point&, const Vector&);
		Ray(const Ray&);

		//! Destructor
		~Ray();

		//! Copy Constructor
		Ray& operator=(const Ray&);

		bool operator==(const Ray&) const;
		bool operator!=(const Ray&) const;

		//! Return data
		Point origin() { return o_; }
		Vector direction() { return d_; }

		/*!
		  Returns the Point at parameter t, but does not pre-normalize d
		 */
		Point parameter(double t);

		/*!
		  Computes the ray parameter t at which the ray will
		  intersect the plane specified by the normal N and the
		  point P, such that the plane intersect point Ip:
		  Ip = o + d*t.  Returns true if there is an intersection,
		  false if the vector is parallel to the plane.
		 */
		bool planeIntersectParameter(Vector& N, Point& P,
			double& t);

		//! Modifiers
		void normalize(); //! normalizes the direction vector d

		friend std::ostream& operator<<(std::ostream& os, const Ray& r)
		{
			os << '[' << r.o_ << ' ' << r.d_ << ']';
			return os;
		}
	};


} // End namespace FLTYPE
#endif
