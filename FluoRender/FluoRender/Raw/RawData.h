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

		size_t GetVoxelCount() const noexcept;
		size_t GetElementCount() const noexcept;
		size_t GetTotalBytes() const noexcept;

		double GetVoxelValue(
			size_t x, size_t y, size_t z) const noexcept;
		double GetVoxelValue(
			size_t index) const noexcept;
		double GetNormalizedValue(
			size_t x, size_t y, size_t z,
			double scalar_scale = 1.0) const noexcept;
		double GetNormalizedValue(size_t index, double scale = 1.0) const noexcept;
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

		void* ElementPtr(size_t index) noexcept;

		const void* ElementPtr(size_t index) const noexcept;

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

		template <typename T>
		T* ElementAs(size_t index) noexcept
		{
			return reinterpret_cast<T*>(
				ElementPtr(index));
		}

		template <typename T>
		const T* ElementAs(size_t index) const noexcept
		{
			return reinterpret_cast<const T*>(
				ElementPtr(index));
		}

		template <typename T>
		T GetValue(size_t x, size_t y, size_t z) const noexcept
		{
			if (!IsAllocated() ||
				x >= m_size[0] ||
				y >= m_size[1] ||
				z >= m_size[2])
				return T{};

			// assert(m_format matches T);

			return *ElementAs<T>(x, y, z);
		}

		template <typename T>
		T GetValue(size_t index) const noexcept
		{
			if (!IsAllocated() ||
				index >= GetElementCount())
				return T{};

			// assert(m_format matches T);

			return *ElementAs<T>(index);
		}

		template <typename T>
		void SetValue(
			size_t x, size_t y, size_t z, T value) noexcept
		{
			if (!IsAllocated() ||
				x >= m_size[0] ||
				y >= m_size[1] ||
				z >= m_size[2])
				return;

			// assert(FormatMatches<T>());

			* ElementAs<T>(x, y, z) = value;

			// Invalidate cached min/max if you have one
			m_minmax_valid = false;
		}

		template <typename T>
		void SetValue(
			size_t index, T value) noexcept
		{
			if (!IsAllocated() ||
				index >= GetElementCount())
				return;

			// assert(FormatMatches<T>());

			*ElementAs<T>(index) = value;

			// Invalidate cached min/max if you have one
			m_minmax_valid = false;
		}

		template <typename Fn>
		void ForEachElement(Fn&& fn)
		{
			switch (m_format)
			{
			case DataFormat::UInt8:
				ForEachElementT<uint8_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::UInt16:
				ForEachElementT<uint16_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::UInt32:
				ForEachElementT<uint32_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Int8:
				ForEachElementT<int8_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Int16:
				ForEachElementT<int16_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Int32:
				ForEachElementT<int32_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Float32:
				ForEachElementT<float>(std::forward<Fn>(fn));
				break;
			case DataFormat::Float64:
				ForEachElementT<double>(std::forward<Fn>(fn));
				break;
			default:
				// Unknown / unsupported → no-op or assert
				break;
			}
		}

		template <typename Fn>
		void ForEachElementIndexed(Fn&& fn)
		{
			switch (m_format)
			{
			case DataFormat::UInt8:
				ForEachElementT<uint8_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::UInt16:
				ForEachElementT<uint16_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::UInt32:
				ForEachElementT<uint32_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Int8:
				ForEachElementT<int8_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Int16:
				ForEachElementT<int16_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Int32:
				ForEachElementT<int32_t>(std::forward<Fn>(fn));
				break;
			case DataFormat::Float32:
				ForEachElementT<float>(std::forward<Fn>(fn));
				break;
			case DataFormat::Float64:
				ForEachElementT<double>(std::forward<Fn>(fn));
				break;
			default:
				break;
			}
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
		void DispatchByFormat(Fn&& fn)
		{
			switch (m_format)
			{
			case DataFormat::UInt8:   ForEachElementT<uint8_t>(fn); break;
			case DataFormat::UInt16:  ForEachElementT<uint16_t>(fn); break;
			case DataFormat::UInt32:  ForEachElementT<uint32_t>(fn); break;

			case DataFormat::Int8:    ForEachElementT<int8_t>(fn); break;
			case DataFormat::Int16:   ForEachElementT<int16_t>(fn); break;
			case DataFormat::Int32:   ForEachElementT<int32_t>(fn); break;

				//case DataFormat::Float16: ForEachElementT<Half>(fn); break; // assuming Half type
			case DataFormat::Float32: ForEachElementT<float>(fn); break;
			case DataFormat::Float64: ForEachElementT<double>(fn); break;

			default:
				break;
			}
		}

		template <typename Fn>
		static void DispatchBinary(RawData& dst,
			const RawData& rhs,
			Fn&& fn)
		{
			// Strong sanity checks (you may relax or remove in release builds)
			if (dst.GetFormat() != rhs.GetFormat())
				throw std::runtime_error("DispatchBinary: format mismatch");

			if (dst.GetElementCount() != rhs.GetElementCount())
				throw std::runtime_error("DispatchBinary: size mismatch");

			switch (dst.GetFormat())
			{
			case DataFormat::UInt8:
				ForEachBinaryT<uint8_t>(dst, rhs, std::forward<Fn>(fn));
				break;

			case DataFormat::UInt16:
				ForEachBinaryT<uint16_t>(dst, rhs, std::forward<Fn>(fn));
				break;

			case DataFormat::UInt32:
				ForEachBinaryT<uint32_t>(dst, rhs, std::forward<Fn>(fn));
				break;

			case DataFormat::Int8:
				ForEachBinaryT<int8_t>(dst, rhs, std::forward<Fn>(fn));
				break;

			case DataFormat::Int16:
				ForEachBinaryT<int16_t>(dst, rhs, std::forward<Fn>(fn));
				break;

			case DataFormat::Int32:
				ForEachBinaryT<int32_t>(dst, rhs, std::forward<Fn>(fn));
				break;

				//case DataFormat::Float16:
				//	ForEachBinaryT<Half>(dst, rhs, std::forward<Fn>(fn));
				//	break;

			case DataFormat::Float32:
				ForEachBinaryT<float>(dst, rhs, std::forward<Fn>(fn));
				break;

			case DataFormat::Float64:
				ForEachBinaryT<double>(dst, rhs, std::forward<Fn>(fn));
				break;

			default:
				throw std::runtime_error("DispatchBinary: unsupported DataFormat");
			}
		}

		template <typename Fn>
		static void DispatchBinaryConvert(
			RawData& dst,
			const RawData& src,
			Fn&& fn)
		{
			if (dst.GetElementCount() != src.GetElementCount())
				throw std::runtime_error("DispatchBinaryConvert: size mismatch");

			switch (dst.GetFormat())
			{
				// ============================================================
				// DESTINATION: UInt8 (most common for masks)
				// ============================================================
			case DataFormat::UInt8:
				switch (src.GetFormat())
				{
				case DataFormat::UInt8:
					ForEachBinaryConvertT<uint8_t, uint8_t>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::UInt16:
					ForEachBinaryConvertT<uint8_t, uint16_t>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::UInt32:
					ForEachBinaryConvertT<uint8_t, uint32_t>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::Float32:
					ForEachBinaryConvertT<uint8_t, float>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::Float64:
					ForEachBinaryConvertT<uint8_t, double>(
						dst, src, std::forward<Fn>(fn));
					break;

				default:
					throw std::runtime_error(
						"DispatchBinaryConvert: unsupported src -> UInt8");
				}
				break;

				// ============================================================
				// DESTINATION: Float32 (normalized volumes, weighting, etc.)
				// ============================================================
			case DataFormat::Float32:
				switch (src.GetFormat())
				{
				case DataFormat::UInt8:
					ForEachBinaryConvertT<float, uint8_t>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::UInt16:
					ForEachBinaryConvertT<float, uint16_t>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::Float32:
					ForEachBinaryConvertT<float, float>(
						dst, src, std::forward<Fn>(fn));
					break;

				case DataFormat::Float64:
					ForEachBinaryConvertT<float, double>(
						dst, src, std::forward<Fn>(fn));
					break;

				default:
					throw std::runtime_error(
						"DispatchBinaryConvert: unsupported src -> Float32");
				}
				break;

			default:
				throw std::runtime_error(
					"DispatchBinaryConvert: unsupported destination format");
			}
		}

		//search functions
		template <typename Pred>
		bool AnyOf(Pred&& pred) const
		{
			if (!IsAllocated())
				return false;

			switch (m_format)
			{
			case DataFormat::UInt8:
				return AnyOfT<uint8_t>(pred);
			case DataFormat::UInt16:
				return AnyOfT<uint16_t>(pred);
			case DataFormat::UInt32:
				return AnyOfT<uint32_t>(pred);
			case DataFormat::Int8:
				return AnyOfT<int8_t>(pred);
			case DataFormat::Int16:
				return AnyOfT<int16_t>(pred);
			case DataFormat::Int32:
				return AnyOfT<int32_t>(pred);
			case DataFormat::Float32:
				return AnyOfT<float>(pred);
			case DataFormat::Float64:
				return AnyOfT<double>(pred);
			default:
				return false;
			}
		}
		bool ContainsUInt32(uint32_t value) const;
		bool ContainsNonZero() const;
		bool ContainsValue(double value) const;

	private:
		Size3 m_size = { 0, 0, 0 };

		DataFormat m_format = DataFormat::Unknown;

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
			Fn&& fn)
		{
			T* d = dst.DataAs<T>();
			const T* r = rhs.DataAs<T>();

			const size_t n = dst.GetElementCount();
			for (size_t i = 0; i < n; ++i)
				d[i] = fn(d[i], r[i]);
		}

		template <typename DstT, typename SrcT, typename Fn>
		static void ForEachBinaryConvertT(
			RawData& dst,
			const RawData& src,
			Fn&& fn)
		{
			DstT* d = dst.DataAs<DstT>();
			const SrcT* s = src.DataAs<SrcT>();

			const size_t n = dst.GetElementCount();
			for (size_t i = 0; i < n; ++i)
				d[i] = fn(d[i], s[i]);
		}

		template <typename T, typename Pred>
		bool AnyOfT(Pred&& pred) const
		{
			const T* data = DataAs<T>();
			const size_t n = GetElementCount();

			for (size_t i = 0; i < n; ++i)
			{
				if (pred(data[i]))
					return true;
			}
			return false;
		}

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

	template <typename Fn>
	inline void ForEach6Neighbor(
		size_t x, size_t y, size_t z,
		size_t nx, size_t ny, size_t nz,
		Fn&& fn)
	{
		if (x > 0)         fn(x - 1, y, z);
		if (x + 1 < nx)    fn(x + 1, y, z);
		if (y > 0)         fn(x, y - 1, z);
		if (y + 1 < ny)    fn(x, y + 1, z);
		if (z > 0)         fn(x, y, z - 1);
		if (z + 1 < nz)    fn(x, y, z + 1);
	}

} // namespace fluo

#endif//_RAW_DATA_H_