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

#include <DataBrick.h>

#include <cstring>
#include <cassert>

using namespace fluo;

DataBrick::DataBrick(const Offset3& offset,
	const Size3& size,
	const RawData& source,
	const Size3& stride)
	: Brick(offset, size),
	m_source(source),
	m_stride(stride)
{
}

std::shared_ptr<RawData>
DataBrick::Extract() const
{
	DataFormat format = m_source.GetFormat();

	auto rd = std::make_shared<RawData>(m_size, format);
	rd->Allocate();

	const size_t bpe = rd->GetBytesPerElement();

	const size_t src_x = m_offset[0];
	const size_t src_y = m_offset[1];
	const size_t src_z = m_offset[2];

	const fluo::Byte* src =
		static_cast<const fluo::Byte*>(
			m_source.ElementPtr(src_x, src_y, src_z));

	CopyPacked(
		rd->GetData(),
		src,
		bpe,
		m_size,
		m_stride
	);

	return rd;
}

void DataBrick::CopyPacked(
	fluo::Byte* dst,
	const fluo::Byte* src,
	size_t            bytes_per_voxel,
	const Size3& size,
	const Size3& stride)
{
	const size_t row_bytes = size[0] * bytes_per_voxel;
	const size_t padded_row_bytes = stride[0] * bytes_per_voxel;

	for (size_t z = 0; z < size[2]; ++z)
	{
		const fluo::Byte* src_z =
			src + z * stride[0] * stride[1] * bytes_per_voxel;

		for (size_t y = 0; y < size[1]; ++y)
		{
			std::memcpy(dst, src_z, row_bytes);
			dst += row_bytes;
			src_z += padded_row_bytes;
		}
	}
}