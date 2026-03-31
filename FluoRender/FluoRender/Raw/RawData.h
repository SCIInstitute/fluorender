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
#ifndef _RAW_DATA_H_
#define _RAW_DATA_H_

#include <Utils.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace fluo
{

	enum class DataFormat
	{
		Unknown = 0,

		UInt8,
		UInt16,
		UInt32,

		Int8,
		Int16,
		Int32,

		Float16,
		Float32,
		Float64
	};

	/// Utility functions (can later move elsewhere)
	constexpr size_t BytesPerElement(DataFormat fmt) noexcept
	{
		switch (fmt)
		{
		case DataFormat::UInt8:
		case DataFormat::Int8:
			return 1;
		case DataFormat::UInt16:
		case DataFormat::Int16:
		case DataFormat::Float16:
			return 2;
		case DataFormat::UInt32:
		case DataFormat::Int32:
		case DataFormat::Float32:
			return 4;
		case DataFormat::Float64:
			return 8;
		default:
			return 0;
		}
	}

	constexpr size_t BitsPerElement(DataFormat fmt) noexcept
	{
		switch (fmt)
		{
		case DataFormat::UInt8:
		case DataFormat::Int8:
			return 8;
		case DataFormat::UInt16:
		case DataFormat::Int16:
		case DataFormat::Float16:
			return 16;
		case DataFormat::UInt32:
		case DataFormat::Int32:
		case DataFormat::Float32:
			return 32;
		case DataFormat::Float64:
			return 64;
		default:
			return 0;
		}
	}

	/// Fundamental raw volume data container
	class RawData
	{
	public:
		using Size3 = std::array<size_t, 3>;
		using DeleterFn = void(*)(Byte*);

		RawData() = default;

		RawData(const Size3& size,
			DataFormat format,
			size_t channels = 1,
			size_t time_steps = 1,
			size_t resolution_level = 0,
			size_t brick_index = 0);

		//adoption constructor
		RawData(const Size3& size,
			DataFormat format,
			size_t channels,
			size_t time_steps,
			size_t resolution_level,
			size_t brick_index,
			Byte* external_buffer,
			DeleterFn deleter);

		/// Disable copying for now (can revisit later)
		RawData(const RawData&) = delete;
		RawData& operator=(const RawData&) = delete;

		/// Enable moves
		RawData(RawData&&) noexcept = default;
		RawData& operator=(RawData&&) noexcept = default;

		~RawData() = default;

		// --- Allocation & state ------------------------------------------------

		bool Allocate();
		void Clear();

		bool IsAllocated() const noexcept;

		// --- Properties --------------------------------------------------------

		const Size3& GetSize() const noexcept { return m_size; }
		size_t GetWidth()  const noexcept { return m_size[0]; }
		size_t GetHeight() const noexcept { return m_size[1]; }
		size_t GetDepth()  const noexcept { return m_size[2]; }

		DataFormat GetFormat() const noexcept { return m_format; }
		size_t GetBitsPerElement() const noexcept;
		size_t GetBytesPerElement() const noexcept;

		size_t GetChannelCount() const noexcept { return m_channels; }
		size_t GetTimeSteps() const noexcept { return m_time_steps; }

		size_t GetResolutionLevel() const noexcept { return m_resolution_level; }
		size_t GetBrickIndex() const noexcept { return m_brick_index; }

		size_t GetVoxelCount() const noexcept;
		size_t GetElementCount() const noexcept;
		size_t GetTotalBytes() const noexcept;

		// --- Raw memory access -------------------------------------------------

		const fluo::Byte* GetData() const noexcept;
		fluo::Byte* GetData() noexcept;

		/// Typed access (caller responsible for matching format)
		template <typename T>
		T* DataAs() noexcept
		{
			return reinterpret_cast<T*>(GetData());
		}

		template <typename T>
		const T* DataAs() const noexcept
		{
			return reinterpret_cast<const T*>(GetData());
		}

		template <typename T>
		std::pair<double, double> ComputeMinMax() const;

	private:
		Size3 m_size = { 0, 0, 0 };

		DataFormat m_format = DataFormat::Unknown;
		size_t m_channels = 1;
		size_t m_time_steps = 1;

		size_t m_resolution_level = 0;
		size_t m_brick_index = 0;

		std::vector<fluo::Byte> m_data;

		std::unique_ptr<Byte, DeleterFn> m_external{ nullptr, nullptr };
	};

} // namespace fluo

#endif//_RAW_DATA_H_