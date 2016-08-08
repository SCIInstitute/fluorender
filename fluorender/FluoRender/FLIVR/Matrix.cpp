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

#include "Matrix.h"

namespace FLIVR
{
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
} // End namespace FLIVR
