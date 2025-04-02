﻿/*
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

#ifndef VECTOR4I_HPP
#define VECTOR4I_HPP

#include <string>
#include <iosfwd>
#include <iostream>

#ifndef __glew_h__
typedef float GLfloat;
typedef int GLint;
#endif

namespace fluo
{
	class Vector4i
	{
		GLint v_[4];
	public:
		inline Vector4i(const GLint v[4]);
		inline Vector4i(const GLint v1, const GLint v2, const GLint v3, const GLint v4);
		inline Vector4i(const Vector4i&);
		inline Vector4i();
		inline Vector4i(GLint init);

		inline GLint* get();
		inline Vector4i& operator=(const Vector4i&);
		inline Vector4i& operator=(const GLfloat&);
		inline Vector4i& operator=(const GLint&);
		inline GLint& operator[](int idx);
		inline GLint operator[](int idx) const;
		inline int operator==(const Vector4i&) const;
		inline int operator!=(const Vector4i&) const;

		friend std::ostream& operator<<(std::ostream& os, const Vector4i& v)
		{
			//avoid using spaces so that it can be read correctly using >>
			os << '[' << v[0] << ',' << v[1] << ',' << v[2] << ',' << v[3] << ']';
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Vector4i& v)
		{
			GLfloat v1, v2, v3, v4;
			char st;
			is >> st >> v1 >> st >> v2 >> st >> v3 >> st >> v4 >> st;
			v = Vector4i(static_cast<GLint>(std::round(v1)),
				static_cast<GLint>(std::round(v2)),
				static_cast<GLint>(std::round(v3)),
				static_cast<GLint>(std::round(v4)));
			return is;
		}
	};

	inline Vector4i::Vector4i(const GLint v[4])
	{
		v_[0] = v[0]; v_[1] = v[1]; v_[2] = v[2]; v_[3] = v[3];
	}

	inline Vector4i::Vector4i(const GLint v1, const GLint v2, const GLint v3, const GLint v4)
	{
		v_[0] = v1; v_[1] = v2; v_[2] = v3; v_[3] = v4;
	}

	inline Vector4i::Vector4i(const Vector4i& v)
	{
		v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2]; v_[3] = v.v_[3];
	}

	inline Vector4i::Vector4i()
	{
		v_[0] = 0; v_[1] = 0; v_[2] = 0; v_[3] = 0;
	}

	inline Vector4i::Vector4i(GLint init)
	{
		v_[0] = init; v_[1] = init; v_[2] = init; v_[3] = init;
	}

	inline GLint* Vector4i::get()
	{
		return &(v_[0]);
	}

	inline Vector4i& Vector4i::operator=(const Vector4i& v)
	{
		v_[0] = v.v_[0]; v_[1] = v.v_[1]; v_[2] = v.v_[2]; v_[3] = v.v_[3];
		return *this;
	}

	inline Vector4i& Vector4i::operator=(const GLint& v)
	{
		v_[0] = v; v_[1] = v; v_[2] = v; v_[3] = v;
		return *this;
	}

	inline Vector4i& Vector4i::operator=(const GLfloat& v)
	{
		v_[0] = static_cast<GLint>(v);
		v_[1] = static_cast<GLint>(v);
		v_[2] = static_cast<GLint>(v);
		v_[3] = static_cast<GLint>(v);
		return *this;
	}

	inline GLint& Vector4i::operator[](int idx)
	{
		return v_[idx];
	}

	inline GLint Vector4i::operator[](int idx) const
	{
		return v_[idx];
	}

	inline int Vector4i::operator==(const Vector4i& v) const
	{
		return v_[0] == v.v_[0] &&
			v_[1] == v.v_[1] &&
			v_[2] == v.v_[2] &&
			v_[3] == v.v_[3];
	}

	inline int Vector4i::operator!=(const Vector4i& v) const
	{
		return v_[0] != v.v_[0] ||
			v_[1] != v.v_[1] ||
			v_[2] != v.v_[2] ||
			v_[3] != v.v_[3];
	}
}

#endif//VECTOR4I_HPP
