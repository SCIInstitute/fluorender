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
#include <compatibility.h>
#include <cstring>
#include <Debug.h>

namespace fluo
{

	RawData::RawData(const Size3& size,
		DataFormat format)
		: m_size(size),
		m_format(format)
	{
	}

	RawData::RawData(const Size3& size,
		DataFormat format,
		Byte* external_buffer,
		DeleterFn deleter)
		: m_size(size),
		m_format(format),
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

	PixelFormat RawData::GetPixelFormat() const noexcept
	{
		switch (m_format)
		{
		case DataFormat::UInt8:   return PixelFormat::R8_UNorm;
		case DataFormat::UInt16:  return PixelFormat::R16_UNorm;
		case DataFormat::UInt32:  return PixelFormat::R32_UNorm;

		case DataFormat::Int8:    return PixelFormat::R8_SNorm;
		case DataFormat::Int16:   return PixelFormat::R16_SNorm;
		case DataFormat::Int32:   return PixelFormat::R32_SNorm;

		case DataFormat::Float32: return PixelFormat::R32_Float;
		case DataFormat::Float64: return PixelFormat::R64_Float;

		default:
			return PixelFormat::Undefined;
		}
	}

	size_t RawData::GetVoxelCount() const noexcept
	{
		return m_size[0] * m_size[1] * m_size[2];
	}

	size_t RawData::GetElementCount() const noexcept
	{
		return GetVoxelCount();
	}

	size_t RawData::GetTotalBytes() const noexcept
	{
		size_t voxels = m_size[0] * m_size[1] * m_size[2];
		return voxels * BytesPerElement(m_format);
	}

	double RawData::GetVoxelValue(
		size_t x, size_t y, size_t z) const noexcept
	{
		if (!IsAllocated())
			return 0.0;

		// Bounds check (cheap safety)
		if (x >= m_size[0] || y >= m_size[1] || z >= m_size[2])
			return 0.0;

		const size_t voxels_per_volume =
			m_size[0] * m_size[1] * m_size[2];

		const size_t voxel_index =
			z * m_size[0] * m_size[1] +
			y * m_size[0] +
			x;

		const size_t element_index =
			voxel_index;

		switch (m_format)
		{
		case DataFormat::UInt8:
		{
			const auto* p = DataAs<uint8_t>();
			return p[element_index] / 255.0;
		}
		case DataFormat::UInt16:
		{
			const auto* p = DataAs<uint16_t>();
			return p[element_index] / 65535.0;
		}
		case DataFormat::UInt32:
		{
			const auto* p = DataAs<uint32_t>();
			return p[element_index] / 4294967295.0;
		}
		case DataFormat::Int8:
		{
			const auto* p = DataAs<int8_t>();
			return (double(p[element_index]) + 128.0) / 255.0;
		}
		case DataFormat::Int16:
		{
			const auto* p = DataAs<int16_t>();
			return (double(p[element_index]) + 32768.0) / 65535.0;
		}
		case DataFormat::Int32:
		{
			const auto* p = DataAs<int32_t>();
			return (double(p[element_index]) + 2147483648.0)
				/ 4294967295.0;
		}
		case DataFormat::Float16:
		case DataFormat::Float32:
		{
			const auto* p = DataAs<float>();
			return double(p[element_index]); // assumed already normalized
		}
		case DataFormat::Float64:
		{
			const auto* p = DataAs<double>();
			return p[element_index];
		}
		default:
			return 0.0;
		}
	}

	double RawData::GetVoxelValue(
		size_t index) const noexcept
	{
		if (!IsAllocated())
			return 0.0;
		if (index >= GetElementCount())
			return 0.0;
		switch (m_format)
		{
		case DataFormat::UInt8:
		{
			const auto* p = DataAs<uint8_t>();
			return p[index] / 255.0;
		}
		case DataFormat::UInt16:
		{
			const auto* p = DataAs<uint16_t>();
			return p[index] / 65535.0;
		}
		case DataFormat::UInt32:
		{
			const auto* p = DataAs<uint32_t>();
			return p[index] / 4294967295.0;
		}
		case DataFormat::Int8:
		{
			const auto* p = DataAs<int8_t>();
			return (double(p[index]) + 128.0) / 255.0;
		}
		case DataFormat::Int16:
		{
			const auto* p = DataAs<int16_t>();
			return (double(p[index]) + 32768.0) / 65535.0;
		}
		case DataFormat::Int32:
		{
			const auto* p = DataAs<int32_t>();
			return (double(p[index]) + 2147483648.0)
				/ 4294967295.0;
		}
		case DataFormat::Float16:
		case DataFormat::Float32:
		{
			const auto* p = DataAs<float>();
			return double(p[index]); // assumed already normalized
		}
		case DataFormat::Float64:
		{
			const auto* p = DataAs<double>();
			return p[index];
		}
		default:
			return 0.0;
		}
	}

	double RawData::GetNormalizedValue(
		size_t x, size_t y, size_t z,
		double scalar_scale) const noexcept
	{
		double v = GetVoxelValue(x, y, z);

		switch (m_format)
		{
		case DataFormat::UInt8:
			return v / 255.0;

		case DataFormat::UInt16:
			return v * scalar_scale / 65535.0;

		case DataFormat::Float32:
		case DataFormat::Float64:
			return v; // already normalized by definition

		default:
			return 0.0;
		}
	}

	double RawData::GetNormalizedValue(size_t index, double scale) const noexcept
	{
		switch (m_format)
		{
		case DataFormat::UInt8:
			return DataAs<uint8_t>()[index] / 255.0;
		case DataFormat::UInt16:
			return DataAs<uint16_t>()[index] * scale / 65535.0;
		default:
			return GetVoxelValue(index); // fallback
		}
	}

	double RawData::GetGradientMagnitude(
		size_t x, size_t y, size_t z,
		double scalar_scale) const noexcept
	{
		// Must be interior voxel: matches old behavior
		if (x == 0 || y == 0 || z == 0 ||
			x + 1 >= m_size[0] ||
			y + 1 >= m_size[1] ||
			z + 1 >= m_size[2])
			return 0.0;

		if (!IsAllocated())
			return 0.0;

		switch (m_format)
		{
		case DataFormat::UInt8:
			return GradientMagnitudeT<uint8_t>(
				x, y, z, /*scalar_scale=*/1.0);

		case DataFormat::UInt16:
			return GradientMagnitudeT<uint16_t>(
				x, y, z, scalar_scale);

		case DataFormat::Float32:
			return GradientMagnitudeT<float>(
				x, y, z, 1.0);

		case DataFormat::Float64:
			return GradientMagnitudeT<double>(
				x, y, z, 1.0);

		default:
			return 0.0;
		}
	}

	// --- Raw memory access -------------------------------------------------

	fluo::Byte* RawData::GetData() noexcept
	{
		if (m_external)
			return m_external.get();
		return m_data.empty() ? nullptr : m_data.data();
	}

	const fluo::Byte* RawData::GetData() const noexcept
	{
		if (m_external)
			return m_external.get();
		return m_data.empty() ? nullptr : m_data.data();
	}

	void* RawData::GetDataVoid() noexcept
	{
		return static_cast<void*>(GetData());
	}

	const void* RawData::GetDataVoid() const noexcept
	{
		return static_cast<const void*>(GetData());
	}

	void* RawData::ElementPtr(
		size_t x, size_t y, size_t z) noexcept
	{
		if (!GetData())
			return nullptr;

		// Optional safety (cheap; remove if hot path)
		if (x >= m_size[0] || y >= m_size[1] || z >= m_size[2])
			return nullptr;

		const size_t voxels_per_volume =
			m_size[0] * m_size[1] * m_size[2];

		const size_t voxel_index =
			z * m_size[0] * m_size[1] +
			y * m_size[0] +
			x;

		const size_t element_index =
			voxel_index;

		const size_t byte_offset =
			element_index * GetBytesPerElement();

		return static_cast<void*>(GetData() + byte_offset);
	}

	const void* RawData::ElementPtr(
		size_t x, size_t y, size_t z) const noexcept
	{
		return const_cast<RawData*>(this)->
			ElementPtr(x, y, z);
	}

	void* RawData::ElementPtr(size_t index) noexcept
	{
		fluo::Byte* data = GetData();
		if (!data)
			return nullptr;

		// Bounds check at element granularity
		if (index >= GetElementCount())
			return nullptr;

		const size_t byte_offset =
			index * GetBytesPerElement();

		return static_cast<void*>(data + byte_offset);
	}

	const void* RawData::ElementPtr(
		size_t index) const noexcept
	{
		return const_cast<RawData*>(this)->
			ElementPtr(index);
	}

	std::pair<double, double> RawData::GetMinMax() const
	{
		if (!m_minmax_valid)
		{
			m_minmax_cache = ComputeMinMaxInternal();
			m_minmax_valid = true;
		}
		return m_minmax_cache;
	}

	std::pair<double, double> RawData::ComputeMinMaxInternal() const
	{
		if (!IsAllocated())
			return { 0.0, 0.0 };

		switch (m_format)
		{
		case DataFormat::UInt8:
			return ComputeMinMaxT<uint8_t>();
		case DataFormat::UInt16:
			return ComputeMinMaxT<uint16_t>();
		case DataFormat::UInt32:
			return ComputeMinMaxT<uint32_t>();

		case DataFormat::Int8:
			return ComputeMinMaxT<int8_t>();
		case DataFormat::Int16:
			return ComputeMinMaxT<int16_t>();
		case DataFormat::Int32:
			return ComputeMinMaxT<int32_t>();

		case DataFormat::Float32:
			return ComputeMinMaxT<float>();
		case DataFormat::Float64:
			return ComputeMinMaxT<double>();

		default:
			return { 0.0, 0.0 };
		}
	}

	template <typename T>
	std::pair<double, double> RawData::ComputeMinMaxT() const
	{
		const T* ptr = DataAs<T>();
		if (!ptr)
			return { 0.0, 0.0 };

		const size_t n = GetElementCount();
		if (n == 0)
			return { 0.0, 0.0 };

		T minv = ptr[0];
		T maxv = ptr[0];

		for (size_t i = 1; i < n; ++i)
		{
			minv = ptr[i] < minv ? ptr[i] : minv;
			maxv = ptr[i] > maxv ? ptr[i] : maxv;
		}

		return { static_cast<double>(minv),
				 static_cast<double>(maxv) };
	}

	void RawData::FillZero()
	{
		if (!IsAllocated())
			return;

		std::memset(GetData(), 0, GetTotalBytes());

		m_minmax_valid = false;
	}

	void RawData::FillConstant(double value)
	{
		if (!IsAllocated())
			return;

		DispatchByFormat([&](auto& elem, size_t)
			{
				using T = std::decay_t<decltype(elem)>;
				elem = static_cast<T>(value);
			});

		m_minmax_valid = false;
	}

	void RawData::FillSequence()
	{
		if (!IsAllocated())
			return;
	}

	void RawData::FillOrderedID()
	{
		if (m_format != DataFormat::UInt32)
			throw std::runtime_error("FillOrderedID requires UInt32 format");

		uint32_t* val = DataAs<uint32_t>();

		const size_t res_x = m_size[0];
		const size_t res_y = m_size[1];
		const size_t res_z = m_size[2];

		for (size_t i = 0; i < res_x; ++i)
		for (size_t j = 0; j < res_y; ++j)
		for (size_t k = 0; k < res_z; ++k)
		{
			size_t index = res_y * res_z * i +
				res_z * j + k;

			val[index] = static_cast<uint32_t>(index + 1);
		}
	}

	void RawData::FillReverseID()
	{
		if (m_format != DataFormat::UInt32)
			throw std::runtime_error("FillReverseID requires UInt32 format");

		uint32_t* val = DataAs<uint32_t>();

		const size_t res_x = m_size[0];
		const size_t res_y = m_size[1];
		const size_t res_z = m_size[2];
		const uint32_t total =
			static_cast<uint32_t>(res_x * res_y * res_z);

		for (size_t i = 0; i < res_x; ++i)
		for (size_t j = 0; j < res_y; ++j)
		for (size_t k = 0; k < res_z; ++k)
		{
			size_t index = res_y * res_z * i +
				res_z * j + k;

			val[index] = total - static_cast<uint32_t>(index);
		}
	}

	void RawData::FillShuffledID()
	{
		if (m_format != DataFormat::UInt32)
			throw std::runtime_error("FillShuffledID requires UInt32 format");

		uint32_t* val = DataAs<uint32_t>();

		const size_t res_x = m_size[0];
		const size_t res_y = m_size[1];
		const size_t res_z = m_size[2];
		const uint32_t total =
			static_cast<uint32_t>(res_x * res_y * res_z);

		// Compute bit length
		unsigned int r = static_cast<unsigned int>(
			std::max(res_x, std::max(res_y, res_z)));

		unsigned int len = 0;
		while (r > 0)
		{
			r >>= 1;
			++len;
		}

		for (size_t i = 0; i < res_x; ++i)
		for (size_t j = 0; j < res_y; ++j)
		for (size_t k = 0; k < res_z; ++k)
		{
			unsigned int x = reverse_bit(static_cast<unsigned int>(i), len);
			unsigned int y = reverse_bit(static_cast<unsigned int>(j), len);
			unsigned int z = reverse_bit(static_cast<unsigned int>(k), len);

			unsigned int res = 0;
			for (unsigned int ii = 0; ii < len; ++ii)
			{
				res |= ((x >> ii) & 1u) << (3 * ii);
				res |= ((y >> ii) & 1u) << (3 * ii + 1);
				res |= ((z >> ii) & 1u) << (3 * ii + 2);
			}

			size_t index = res_x * res_y * k +
				res_x * j + i;

			val[index] = total - res;
		}
	}

	template <typename T>
	double RawData::GradientMagnitudeT(
		size_t x, size_t y, size_t z,
		double scalar_scale) const noexcept
	{
		const size_t nx = m_size[0];
		const size_t ny = m_size[1];
		const size_t nz = m_size[2];

		const size_t idx = nx * ny * z +
			nx * y + x;

		const T* data = DataAs<T>();

		auto sample = [&](size_t xi, size_t yi, size_t zi) -> double
			{
				const size_t i = nx * ny * zi +
					nx * yi + xi;

				if constexpr (std::is_same_v<T, uint8_t>)
				{
					return static_cast<double>(data[i]) / 255.0;
				}
				else if constexpr (std::is_same_v<T, uint16_t>)
				{
					return static_cast<double>(data[i]) *
						scalar_scale / 65535.0;
				}
				else
				{
					// float / double assumed already normalized
					return static_cast<double>(data[i]);
				}
			};

		// Central differences (6-neighbor stencil)
		const double v_xm = sample(x - 1, y, z);
		const double v_xp = sample(x + 1, y, z);
		const double v_ym = sample(x, y - 1, z);
		const double v_yp = sample(x, y + 1, z);
		const double v_zm = sample(x, y, z - 1);
		const double v_zp = sample(x, y, z + 1);

		const double gx = v_xp - v_xm;
		const double gy = v_yp - v_ym;
		const double gz = v_zp - v_zm;

		// Match original implementation exactly
		return std::sqrt(gx * gx + gy * gy + gz * gz) * 0.53;
	}
	
	bool RawData::ContainsUInt32(uint32_t value) const
	{
		if (m_format != DataFormat::UInt32)
			throw std::runtime_error("ContainsUInt32 requires UInt32 format");

		return AnyOf([value](uint32_t v)
			{
				return v == value;
			});
	}

	bool RawData::ContainsNonZero() const
	{
		if (!IsAllocated())
			return false;

		return AnyOf([](auto v)
			{
				using T = std::decay_t<decltype(v)>;

				if constexpr (std::is_floating_point_v<T>)
					return v != static_cast<T>(0);
				else
					return v != 0;
			});
	}

	bool RawData::ContainsValue(double value) const
	{
		if (!IsAllocated())
			return false;

		return AnyOf([value](auto v)
			{
				using T = std::decay_t<decltype(v)>;

				if constexpr (std::is_floating_point_v<T>)
				{
					// Exact comparison by design
					return static_cast<double>(v) == value;
				}
				else
				{
					// Integer comparison after conversion
					return static_cast<double>(v) == value;
				}
			});
	}

} // namespace fluo