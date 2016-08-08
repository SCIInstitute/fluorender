/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
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

#ifndef SLIVR_Matrix_h
#define SLIVR_Matrix_h

#include "Utils.h"
#include "Vector.h"
#include "Point.h"

namespace FLIVR
{
	class Mat3
	{
		double mat[3][3];
	public:
		inline explicit Mat3(const Vector& row1,
			const Vector& row2, const Vector row3);
		inline Mat3(double m00, double m01, double m02,
			double m10, double m11, double m12,
			double m20, double m21, double m22);
		inline Mat3(const Mat3&);
		inline Mat3();
		inline int operator==(const Mat3&) const;
		inline int operator!=(const Mat3&) const;
		inline Mat3& operator=(const Mat3&);
		inline Mat3 operator+(const Mat3&) const;
		inline Mat3& operator+=(const Mat3&);
		inline Mat3 operator-(const Mat3&) const;
		inline Mat3& operator-=(const Mat3&);
		inline Mat3 operator*(double) const;
		inline Mat3& operator*=(const double);
		inline Mat3 operator*(const Mat3&) const;
		inline Mat3& operator*=(const Mat3&);
		inline Mat3& operator/=(const double);
		inline Mat3 operator/(const double) const;
		inline Mat3 operator-() const;
		inline Mat3 operator+() const;

		inline Vector get_row(int idx) const;
		inline Vector get_col(int idx) const;
		inline Vector operator[](int idx) const;
	}; // end class Mat3

	inline Mat3 transpose(const Mat3 &mat)
	{
		return Mat3(mat[0][0], mat[1][0], mat[2][0],
			mat[0][1], mat[1][1], mat[2][1],
			mat[0][2], mat[1][2], mat[2][2]);
	}

	inline Point operator*(const Mat3 &mat, const Point &p)
	{
		return Point(Dot(mat[0], p),
			Dot(mat[1], p),
			Dot(mat[2], p));
	}
	inline Point operator*(const Point &p, const Mat3 &mat)
	{
		return Point(Dot(p, mat.get_col(0)),
			Dot(p, mat.get_col(1)),
			Dot(p, mat.get_col(2)));
	}

	inline Vector operator*(const Mat3 &mat, const Vector &v)
	{
		return Vector(Dot(mat[0], v),
			Dot(mat[1], v),
			Dot(mat[2], v));
	}
	inline Vector operator*(const Vector &v, const Mat3 &mat)
	{
		return Vector(Dot(v, mat.get_col(0)),
			Dot(v, mat.get_col(1)),
			Dot(v, mat.get_col(2)));
	}

} // End namespace FLIVR

#endif //ifndef SLIVR_Point_h
