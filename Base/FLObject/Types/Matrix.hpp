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

#ifndef FLMATRIX_HPP
#define FLMATRIX_HPP

#include <Utils.hpp>
#include <Vector.hpp>
#include <Point.hpp>

namespace fluo
{
	class Mat3
	{
	public:
		double mat[3][3];
		inline explicit Mat3(const Vector& row1,
			const Vector& row2, const Vector row3);
		inline Mat3(double m00, double m01, double m02,
			double m10, double m11, double m12,
			double m20, double m21, double m22);
		inline Mat3(const Mat3&);
		inline Mat3();
		inline Mat3(const double);
		inline Mat3(const Vector&);
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

		inline double det() const;
		inline Mat3 inv() const;
	}; // end class Mat3

	inline Mat3::Mat3(const Vector& row1,
		const Vector& row2, const Vector row3)
	{
		mat[0][0] = row1[0]; mat[0][1] = row1[1]; mat[0][2] = row1[2];
		mat[1][0] = row2[0]; mat[1][1] = row2[1]; mat[1][2] = row2[2];
		mat[2][0] = row3[0]; mat[2][1] = row3[1]; mat[2][2] = row3[2];
	}

	inline Mat3::Mat3(double m00, double m01, double m02,
		double m10, double m11, double m12,
		double m20, double m21, double m22)
	{
		mat[0][0] = m00; mat[0][1] = m01; mat[0][2] = m02;
		mat[1][0] = m10; mat[1][1] = m11; mat[1][2] = m12;
		mat[2][0] = m20; mat[2][1] = m21; mat[2][2] = m22;
	}

	inline Mat3::Mat3(const Mat3& m)
	{
		mat[0][0] = m[0][0]; mat[0][1] = m[0][1]; mat[0][2] = m[0][2];
		mat[1][0] = m[1][0]; mat[1][1] = m[1][1]; mat[1][2] = m[1][2];
		mat[2][0] = m[2][0]; mat[2][1] = m[2][1]; mat[2][2] = m[2][2];
	}

	inline Mat3::Mat3()
	{
		mat[0][0] = 1; mat[0][1] = 0; mat[0][2] = 0;
		mat[1][0] = 0; mat[1][1] = 1; mat[1][2] = 0;
		mat[2][0] = 0; mat[2][1] = 0; mat[2][2] = 1;
	}

	inline Mat3::Mat3(const double d)
	{
		mat[0][0] = d; mat[0][1] = d; mat[0][2] = d;
		mat[1][0] = d; mat[1][1] = d; mat[1][2] = d;
		mat[2][0] = d; mat[2][1] = d; mat[2][2] = d;
	}

	inline Mat3::Mat3(const Vector& trace)
	{
		mat[0][0] = trace[0]; mat[0][1] = 0; mat[0][2] = 0;
		mat[1][0] = 0; mat[1][1] = trace[1]; mat[1][2] = 0;
		mat[2][0] = 0; mat[2][1] = 0; mat[2][2] = trace[2];
	}

	inline int Mat3::operator==(const Mat3& m) const
	{
		return (mat[0][0] == m[0][0] && mat[0][1] == m[0][1] && mat[0][2] == m[0][2] &&
			mat[1][0] == m[1][0] && mat[1][1] == m[1][1] && mat[1][2] == m[1][2] &&
			mat[2][0] == m[2][0] && mat[2][1] == m[2][1] && mat[2][2] == m[2][2]);
	}

	inline int Mat3::operator!=(const Mat3& m) const
	{
		return (mat[0][0] != m[0][0] || mat[0][1] != m[0][1] || mat[0][2] != m[0][2] ||
			mat[1][0] != m[1][0] || mat[1][1] != m[1][1] || mat[1][2] != m[1][2] ||
			mat[2][0] != m[2][0] || mat[2][1] != m[2][1] || mat[2][2] != m[2][2]);
	}

	inline Mat3& Mat3::operator=(const Mat3& m)
	{
		mat[0][0] = m[0][0]; mat[0][1] = m[0][1]; mat[0][2] = m[0][2];
		mat[1][0] = m[1][0]; mat[1][1] = m[1][1]; mat[1][2] = m[1][2];
		mat[2][0] = m[2][0]; mat[2][1] = m[2][1]; mat[2][2] = m[2][2];
		return *this;
	}

	inline Mat3 Mat3::operator+(const Mat3& m) const
	{
		return Mat3(mat[0][0] + m[0][0], mat[0][1] + m[0][1], mat[0][2] + m[0][2],
			mat[1][0] + m[1][0], mat[1][1] + m[1][1], mat[1][2] + m[1][2],
			mat[2][0] + m[2][0], mat[2][1] + m[2][1], mat[2][2] + m[2][2]);
	}

	inline Mat3& Mat3::operator+=(const Mat3& m)
	{
		mat[0][0] += m[0][0]; mat[0][1] += m[0][1]; mat[0][2] += m[0][2];
		mat[1][0] += m[1][0]; mat[1][1] += m[1][1]; mat[1][2] += m[1][2];
		mat[2][0] += m[2][0]; mat[2][1] += m[2][1]; mat[2][2] += m[2][2];
		return *this;
	}

	inline Mat3 Mat3::operator-(const Mat3& m) const
	{
		return Mat3(mat[0][0] - m[0][0], mat[0][1] - m[0][1], mat[0][2] - m[0][2],
			mat[1][0] - m[1][0], mat[1][1] - m[1][1], mat[1][2] - m[1][2],
			mat[2][0] - m[2][0], mat[2][1] - m[2][1], mat[2][2] - m[2][2]);
	}

	inline Mat3& Mat3::operator-=(const Mat3& m)
	{
		mat[0][0] -= m[0][0]; mat[0][1] -= m[0][1]; mat[0][2] -= m[0][2];
		mat[1][0] -= m[1][0]; mat[1][1] -= m[1][1]; mat[1][2] -= m[1][2];
		mat[2][0] -= m[2][0]; mat[2][1] -= m[2][1]; mat[2][2] -= m[2][2];
		return *this;
	}

	inline Mat3 Mat3::operator*(double d) const
	{
		return Mat3(mat[0][0] * d, mat[0][1] * d, mat[0][2] * d,
			mat[1][0] * d, mat[1][1] * d, mat[1][2] * d,
			mat[2][0] * d, mat[2][1] * d, mat[2][2] * d);
	}

	inline Mat3& Mat3::operator*=(double d)
	{
		mat[0][0] *= d; mat[0][1] *= d; mat[0][2] *= d;
		mat[1][0] *= d; mat[1][1] *= d; mat[1][2] *= d;
		mat[2][0] *= d; mat[2][1] *= d; mat[2][2] *= d;
		return *this;
	}

	inline Mat3 Mat3::operator*(const Mat3& m) const
	{
		return Mat3(Dot(get_row(0), m.get_col(0)),
			Dot(get_row(0), m.get_col(1)),
			Dot(get_row(0), m.get_col(2)),
			Dot(get_row(1), m.get_col(0)),
			Dot(get_row(1), m.get_col(1)),
			Dot(get_row(1), m.get_col(2)),
			Dot(get_row(2), m.get_col(0)),
			Dot(get_row(2), m.get_col(1)),
			Dot(get_row(2), m.get_col(2)));
	}

	inline Mat3& Mat3::operator*=(const Mat3& m)
	{
		Mat3 temp = *this * m;
		*this = temp;
		return *this;
	}

	inline Mat3 Mat3::operator/(double d) const
	{
		return Mat3(mat[0][0] / d, mat[0][1] / d, mat[0][2] / d,
			mat[1][0] / d, mat[1][1] / d, mat[1][2] / d,
			mat[2][0] / d, mat[2][1] / d, mat[2][2] / d);
	}

	inline Mat3& Mat3::operator/=(double d)
	{
		mat[0][0] /= d; mat[0][1] /= d; mat[0][2] /= d;
		mat[1][0] /= d; mat[1][1] /= d; mat[1][2] /= d;
		mat[2][0] /= d; mat[2][1] /= d; mat[2][2] /= d;
		return *this;
	}

	inline Mat3 Mat3::operator-() const
	{
		return Mat3(-mat[0][0], -mat[0][1], -mat[0][2],
			-mat[1][0], -mat[1][1], -mat[1][2],
			-mat[2][0], -mat[2][1], -mat[2][2]);
	}

	inline Mat3 Mat3::operator+() const
	{
		return *this;
	}

	inline Vector Mat3::get_row(int idx) const
	{
		return Vector(mat[idx][0], mat[idx][1], mat[idx][2]);
	}

	inline Vector Mat3::get_col(int idx) const
	{
		return Vector(mat[0][idx], mat[1][idx], mat[2][idx]);
	}

	inline Vector Mat3::operator[](int idx) const
	{
		return get_row(idx);
	}

	inline double Mat3::det() const
	{
		return mat[0][0] * mat[1][1] * mat[2][2] +
			mat[0][1] * mat[1][2] * mat[2][0] +
			mat[0][2] * mat[1][0] * mat[2][1] -
			mat[0][2] * mat[1][1] * mat[2][0] -
			mat[0][1] * mat[1][0] * mat[2][2] -
			mat[0][0] * mat[1][2] * mat[2][1];
	}

	inline Mat3 Mat3::inv() const
	{
		double a = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];
		double b = mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2];
		double c = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];
		double d = mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2];
		double e = mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0];
		double f = mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1];
		double g = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
		double h = mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2];
		double i = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

		double det = mat[0][0] * a + mat[0][1] * b + mat[0][2] * c;
		if (det != 0.0)
			return Mat3(a / det, d / det, g / det,
				b / det, e / det, h / det,
				c / det, f / det, i / det);
		else
			return Mat3(0);
	}

	inline double det(const Mat3 &mat)
	{
		return mat[0][0] * mat[1][1] * mat[2][2] +
			mat[0][1] * mat[1][2] * mat[2][0] +
			mat[0][2] * mat[1][0] * mat[2][1] -
			mat[0][2] * mat[1][1] * mat[2][0] -
			mat[0][1] * mat[1][0] * mat[2][2] -
			mat[0][0] * mat[1][2] * mat[2][1];
	}

	inline Mat3 inv(const Mat3 &mat)
	{
		double a = mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1];
		double b = mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2];
		double c = mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0];
		double d = mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2];
		double e = mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0];
		double f = mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1];
		double g = mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1];
		double h = mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2];
		double i = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];

		double det = mat[0][0] * a + mat[0][1] * b + mat[0][2] * c;
		if (det != 0.0)
			return Mat3(a / det, d / det, g / det,
				b / det, e / det, h / det,
				c / det, f / det, i / det);
		else
			return Mat3(0);
	}

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

	inline Mat3 form(const Vector &v1, const Vector &v2)
	{
		return Mat3(v1[0] * v2[0], v1[0] * v2[1], v1[0] * v2[2],
					v1[1] * v2[0], v1[1] * v2[1], v1[1] * v2[2],
					v1[2] * v2[0], v1[2] * v2[1], v1[2] * v2[2]);
	}
} // End namespace fluo

#endif //FLMATRIX_HPP
