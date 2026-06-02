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
		R8,
		R8_UNORM,
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

	struct TextureSpec
	{
		TextureType type;
		TextureFormat format;

		bool useMipmap = false;

		TexFilter minFilter = TexFilter::Nearest;
		TexFilter magFilter = TexFilter::Nearest;

		TexWrap wrapS = TexWrap::ClampToEdge;
		TexWrap wrapT = TexWrap::ClampToEdge;
		TexWrap wrapR = TexWrap::ClampToEdge;

		bool operator==(const TextureSpec& o) const
		{
			return type == o.type &&
				format == o.format &&
				useMipmap == o.useMipmap &&
				minFilter == o.minFilter &&
				magFilter == o.magFilter &&
				wrapS == o.wrapS &&
				wrapT == o.wrapT &&
				wrapR == o.wrapR;
		}

		bool operator!=(const TextureSpec& o) const { return !(*this == o); }

		// For framebuffer compatibility (ignore sampling state)
		bool same_storage(const TextureSpec& o) const
		{
			return type == o.type &&
				format == o.format &&
				useMipmap == o.useMipmap;
		}
	};

	struct TextureSize
	{
		int width = 0;
		int height = 0;
		int depth = 1;

		bool operator==(const TextureSize& o) const
		{
			return width == o.width &&
				height == o.height &&
				depth == o.depth;
		}

		bool operator!=(const TextureSize& o) const { return !(*this == o); }

		bool valid() const
		{
			return width > 0 && height > 0 && depth > 0;
		}
	};

	struct TextureDesc
	{
		TextureSpec spec;
		TextureSize size;

		bool operator==(const TextureDesc& o) const
		{
			return spec == o.spec && size == o.size;
		}

		bool operator!=(const TextureDesc& o) const
		{
			return !(*this == o);
		}

		bool same_format(const TextureSpec& s) const
		{
			return spec.same_storage(s);
		}

		bool same_size(int w, int h, int d) const
		{
			return size.width == w &&
				size.height == h &&
				size.depth == d;
		}

		bool framebuffer_compatible(const TextureSpec& s, const TextureSize& sz) const
		{
			return spec.same_storage(s) && size == sz;
		}
	};

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

		const TextureDesc& desc() const { return desc_; }
		const TextureSpec& spec() const { return desc_.spec; }

		int width() const { return desc_.size.width; }
		int height() const { return desc_.size.height; }
		int depth() const { return desc_.size.depth; }

		void upload_2d(const void* data);
		void upload_2d(const void* data, int width, int height, int level = 0);
		void upload_2d_subimage(const void* data,
			int x, int y, int width, int height, int level = 0);

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
