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
#include <RawData.h>
#include <cstring>
#include <Debug.h>

namespace fluo
{

	RawData::RawData(const Size3& size,
		DataFormat format,
		size_t channels,
		size_t time_steps,
		size_t resolution_level,
		size_t brick_index)
		: m_size(size),
		m_format(format),
		m_channels(channels),
		m_time_steps(time_steps),
		m_resolution_level(resolution_level),
		m_brick_index(brick_index)
	{
	}

	RawData::RawData(const Size3& size,
		DataFormat format,
		size_t channels,
		size_t time_steps,
		size_t resolution_level,
		size_t brick_index,
		Byte* external_buffer,
		DeleterFn deleter)
		: m_size(size),
		m_format(format),
		m_channels(channels),
		m_time_steps(time_steps),
		m_resolution_level(resolution_level),
		m_brick_index(brick_index),
		m_external(external_buffer, deleter)
	{
		assert(external_buffer);
		assert(deleter);
	}

	bool RawData::Allocate()
	{
		if (IsAllocated())
			return false;

		if (m_external)
			return false; // already using external memory

		Clear();

		const size_t total_bytes = GetTotalBytes();
		if (total_bytes == 0)
			return false;

		try
		{
			m_data.resize(total_bytes);
			return true;
		}
		catch (...)
		{
			m_data.clear();
			return false;
		}
	}

	void RawData::Clear()
	{
		m_data.clear();
		m_data.shrink_to_fit();

		m_external.reset(); // releases external memory if owned
	}
	
	bool RawData::IsAllocated() const noexcept
	{
		return m_external || !m_data.empty();
	}

	size_t RawData::GetBitsPerElement() const noexcept
	{
		return BitsPerElement(m_format);
	}

	size_t RawData::GetBytesPerElement() const noexcept
	{
		return GetBitsPerElement() / 8;
	}

	size_t RawData::GetVoxelCount() const noexcept
	{
		return m_size[0] * m_size[1] * m_size[2];
	}

	size_t RawData::GetElementCount() const noexcept
	{
		return GetVoxelCount() * m_channels * m_time_steps;
	}

	size_t RawData::GetTotalBytes() const noexcept
	{
		size_t voxels = m_size[0] * m_size[1] * m_size[2];
		return voxels * m_channels * m_time_steps * BytesPerElement(m_format);
	}

	const fluo::Byte* RawData::GetData() const noexcept
	{
		return m_external ? m_external.get() : m_data.data();
	}

	fluo::Byte* RawData::GetData() noexcept
	{
		return m_external ? m_external.get() : m_data.data();
	}

} // namespace fluo