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

#include <PixelFormat.h>
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
			DataFormat format);

		//adoption constructor
		RawData(const Size3& size,
			DataFormat format,
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

		// --- Cloning -----------------------------------------------------------
		std::shared_ptr<RawData> Clone() const
		{
			auto copy = std::make_shared<RawData>(m_size, m_format);
			copy->Allocate();

			std::memcpy(
				copy->GetData(),
				GetData(),
				GetTotalBytes());

			return copy;
		}

		// --- Properties --------------------------------------------------------

		const Size3& GetSize() const noexcept { return m_size; }
		size_t GetWidth()  const noexcept { return m_size[0]; }
		size_t GetHeight() const noexcept { return m_size[1]; }
		size_t GetDepth()  const noexcept { return m_size[2]; }

		DataFormat GetFormat() const noexcept { return m_format; }
		size_t GetBitsPerElement() const noexcept;
		size_t GetBytesPerElement() const noexcept;

		PixelFormat GetPixelFormat() const noexcept;

		//size_t GetChannelCount() const noexcept { return m_channels; }
		//size_t GetTimeSteps() const noexcept { return m_time_steps; }

		//size_t GetResolutionLevel() const noexcept { return m_resolution_level; }
		//size_t GetBrickIndex() const noexcept { return m_brick_index; }

		size_t GetVoxelCount() const noexcept;
		size_t GetElementCount() const noexcept;
		size_t GetTotalBytes() const noexcept;

		double GetVoxelValue(
			size_t x, size_t y, size_t z) const noexcept;
		double GetNormalizedValue(
			size_t x, size_t y, size_t z,
			double scalar_scale = 1.0) const noexcept;
		double GetGradientMagnitude(
			size_t x, size_t y, size_t z,
			double scalar_scale = 1.0) const noexcept;

		// --- Raw memory access -------------------------------------------------

		/// Canonical raw byte access
		fluo::Byte* GetData() noexcept;
		const fluo::Byte* GetData() const noexcept;

		/// Untyped access (graphics / IO / memcpy)
		void* GetDataVoid() noexcept;
		const void* GetDataVoid() const noexcept;

		/// Typed access to entire buffer (caller must match format)
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

		/// Pointer to element at voxel coordinate (untyped)
		void* ElementPtr(size_t x, size_t y, size_t z) noexcept;

		const void* ElementPtr(size_t x, size_t y, size_t z) const noexcept;

		/// Pointer to element at voxel coordinate (typed)
		template <typename T>
		T* ElementAs(size_t x, size_t y, size_t z) noexcept
		{
			return reinterpret_cast<T*>(
				ElementPtr(x, y, z));
		}

		template <typename T>
		const T* ElementAs(size_t x, size_t y, size_t z) const noexcept
		{
			return reinterpret_cast<const T*>(
				ElementPtr(x, y, z));
		}

		std::pair<double, double> GetMinMax() const;

		// --- Data population -------------------------------------------------
		/// Set all elements to zero (type-correct zero)
		void FillZero();
		/// Fill with a constant value (double used as universal input)
		void FillConstant(double value);
		/// Fill with ordered sequence: start, step
		void FillSequence();

		// ID layout generators
		void FillOrderedID();
		void FillReverseID();
		void FillShuffledID();

		template <typename Fn>
		void DispatchByFormat(Fn&& fn);

		template <typename Fn>
		static void DispatchBinary(RawData& dst,
			const RawData& rhs,
			Fn&& fn);

		template <typename Fn>
		static void DispatchBinaryConvert(
			RawData& dst,
			const RawData& src,
			Fn&& fn);

		//search functions
		template <typename Pred>
		bool AnyOf(Pred&& pred) const;
		bool ContainsUInt32(uint32_t value) const;
		bool ContainsNonZero() const;
		bool ContainsValue(double value) const;

	private:
		Size3 m_size = { 0, 0, 0 };

		DataFormat m_format = DataFormat::Unknown;
		//size_t m_channels = 1;
		//size_t m_time_steps = 1;

		//size_t m_resolution_level = 0;
		//size_t m_brick_index = 0;

		std::vector<fluo::Byte> m_data;

		std::unique_ptr<Byte, DeleterFn> m_external{ nullptr, nullptr };

		mutable bool m_minmax_valid = false;
		mutable std::pair<double, double> m_minmax_cache;

	private:
		template <typename T>
		std::pair<double, double> ComputeMinMaxT() const;

		std::pair<double, double> ComputeMinMaxInternal() const;

		template <typename T, typename Fn>
		void ForEachElementT(Fn&& fn)
		{
			T* data = DataAs<T>();
			const size_t n = GetElementCount();

			for (size_t i = 0; i < n; ++i)
				fn(data[i], i);
		}

		template <typename T, typename Fn>
		static void ForEachBinaryT(RawData& dst,
			const RawData& rhs,
			Fn&& fn);

		template <typename DstT, typename SrcT, typename Fn>
		static void ForEachBinaryConvertT(
			RawData& dst,
			const RawData& src,
			Fn&& fn);

		template <typename T, typename Pred>
		bool AnyOfT(Pred&& pred) const;

		template <typename T>
		double GradientMagnitudeT(
			size_t x, size_t y, size_t z,
			double scalar_scale) const noexcept;
	};

	inline RawData::DeleterFn MakeNewArrayDeleter(DataFormat /*format*/)
	{
		// All external allocations use new[]
		return [](Byte* p)
			{
				delete[] p;
			};
	}
} // namespace fluo

#endif//_RAW_DATA_H_