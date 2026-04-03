/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#ifndef _BRICK_H_
#define _BRICK_H_

#include <array>
#include <cstddef>

namespace fluo
{
	class Brick
	{
	public:
		using Size3 = std::array<size_t, 3>;
		using Offset3 = std::array<size_t, 3>;

		Brick() = default;

		Brick(const Offset3& offset,
			const Size3& size)
			: m_offset(offset),
			m_size(size)
		{
		}

		virtual ~Brick() = default;

		const Offset3& GetOffset() const noexcept { return m_offset; }
		const Size3& GetSize()   const noexcept { return m_size; }

		size_t GetWidth()  const noexcept { return m_size[0]; }
		size_t GetHeight() const noexcept { return m_size[1]; }
		size_t GetDepth()  const noexcept { return m_size[2]; }

	protected:
		Offset3 m_offset{ 0, 0, 0 }; // voxel offset in volume
		Size3   m_size{ 0, 0, 0 };   // logical size (no padding)
	};
}
#endif//_BRICK_H_