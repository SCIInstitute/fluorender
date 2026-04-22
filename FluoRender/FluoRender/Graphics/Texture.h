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

#ifndef Texture_h
#define Texture_h

#include <cstdint>

namespace flvr
{
	enum class TextureType : int
	{
		Tex2D,
		Tex3D
	};

	enum class TextureFormat
	{
		RGBA8,
		RGBA32F,
		R32F,
		RG32F,
		R32UI,
		Depth32F
	};

	enum class TexFilter : int {
		Nearest,
		Linear,
		LinearMipmapLinear,
		// Add more as needed
	};

	enum class TexWrap : int {
		ClampToEdge,
		Repeat,
		MirroredRepeat,
		// Add more as needed
	};

	struct TextureDesc
	{
		TextureType type;
		TextureFormat format;
		int width = 0;
		int height = 0;
		int depth = 1;            // depth >1 → 3D texture
		bool useMipmap = false;
		TexFilter minFilter = TexFilter::Nearest;
		TexFilter magFilter = TexFilter::Nearest;
		TexWrap wrapS = TexWrap::ClampToEdge;
		TexWrap wrapT = TexWrap::ClampToEdge;
		TexWrap wrapR = TexWrap::ClampToEdge;

		bool same_format(const TextureDesc& other) const
		{
			return type == other.type &&
				format == other.format;
		}

		bool same_size(const TextureDesc& other) const
		{
			return width == other.width &&
				height == other.height &&
				depth == other.depth;
		}

		bool same_storage(const TextureDesc& other) const
		{
			return same_format(other) && same_size(other);
		}

		bool framebuffer_compatible(const TextureDesc& other) const
		{
			// Intentionally ignore filter/wrap
			return same_storage(other) &&
				useMipmap == other.useMipmap;
		}
	};
	inline bool operator==(const TextureDesc& a, const TextureDesc& b)
	{
		return a.type == b.type &&
			a.format == b.format &&
			a.width == b.width &&
			a.height == b.height &&
			a.depth == b.depth &&
			a.useMipmap == b.useMipmap &&
			a.minFilter == b.minFilter &&
			a.magFilter == b.magFilter &&
			a.wrapS == b.wrapS &&
			a.wrapT == b.wrapT &&
			a.wrapR == b.wrapR;
	}
	inline bool operator!=(const TextureDesc& a, const TextureDesc& b)
	{
		return !(a == b);
	}

	class Texture
	{
	public:
		explicit Texture(const TextureDesc& desc);
		virtual ~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		bool create();
		void destroy();
		bool valid() const { return valid_; }

		bool bind(int unit) const;
		void unbind() const;

		void resize(int w, int h, int d = 1);

		uint32_t id() const { return id_; }

		int width() const { return desc_.width; }
		int height() const { return desc_.height; }
		int depth() const { return desc_.depth; }

	protected:
		// API-specific helpers (GL for now)
		unsigned int glTarget() const;
		void apply_parameters() const;
		void allocate_storage() const;

	protected:
		TextureDesc desc_;
		uint32_t id_ = 0;
		mutable int bound_unit_ = -1;
		bool valid_ = false;
	};

} // namespace flvr

#endif // Texture_h
