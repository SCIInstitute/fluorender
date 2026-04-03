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
#ifndef _DATA_BRICK_H_
#define _DATA_BRICK_H_

#include <Brick.h>
#include <RawData.h>

#include <memory>

namespace fluo
{
	class DataBrick : public Brick
	{
	public:
		DataBrick(const Offset3& offset,
			const Size3& size,
			const RawData& source,
			const Size3& stride);

		/// Extract tightly packed RawData for this brick
		std::shared_ptr<RawData>
			Extract() const;

	private:
		const RawData& m_source;  // non-owning reference to source data
		Size3          m_stride;  // padded stride in source data

	private:
		static void CopyPacked(
			fluo::Byte* dst,
			const fluo::Byte* src,
			size_t            bytes_per_voxel,
			const Size3& size,
			const Size3& stride);
	};
}
#endif//_DATA_BRICK_H_