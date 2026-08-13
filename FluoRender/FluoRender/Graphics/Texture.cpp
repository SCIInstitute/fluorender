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

#include <glad/gl.h>
#include <Texture.h>
#include <GLTextureUtils.h>
#include <utility>

using namespace flvr;

Texture::Texture(const TextureDesc& desc)
	: desc_(desc)
{
}

Texture::~Texture()
{
	destroy();
}

Texture::Texture(Texture&& other) noexcept
	: desc_(std::move(other.desc_)),
	id_(other.id_),
	bound_unit_(other.bound_unit_),
	valid_(other.valid_)
{
	// Leave other in a safe, destructible state
	other.id_ = 0;
	other.bound_unit_ = -1;
	other.valid_ = false;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
	{
		// Release any currently owned GPU resource
		destroy();

		// Transfer ownership
		desc_ = std::move(other.desc_);
		id_ = other.id_;
		bound_unit_ = other.bound_unit_;
		valid_ = other.valid_;

		// Leave other safe to destroy
		other.id_ = 0;
		other.bound_unit_ = -1;
		other.valid_ = false;
	}
	return *this;
}

bool Texture::create()
{
	if (valid_)
		return true;

	glGenTextures(1, &id_);
	glBindTexture(glTarget(), id_);

	apply_parameters();
	allocate_storage();

	if (desc_.spec.useMipmap)
		glGenerateMipmap(glTarget());

	valid_ = true;
	return true;
}

void Texture::destroy()
{
	if (!valid_ || id_ == 0)
		return;

	glDeleteTextures(1, &id_);
	id_ = 0;
	valid_ = false;
}

bool Texture::bind(int unit) const
{
	if (!valid_)
		return false;

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(glTarget(), id_);
	bound_unit_ = unit;
	return true;
}

void Texture::unbind() const
{
	if (bound_unit_ < 0)
		return;

	glActiveTexture(GL_TEXTURE0 + bound_unit_);
	glBindTexture(glTarget(), 0);
	glActiveTexture(GL_TEXTURE0);
	bound_unit_ = -1;
}

void Texture::resize(int w, int h, int d)
{
	desc_.size.width = w;
	desc_.size.height = h;
	desc_.size.depth = d;

	glBindTexture(glTarget(), id_);
	apply_parameters();
	allocate_storage();

	if (desc_.spec.useMipmap)
		glGenerateMipmap(glTarget());
}

void Texture::upload_2d(const void* data)
{
	upload_2d(data, desc_.size.width, desc_.size.height, 0);
}

void Texture::upload_2d(const void* data, int width, int height, int level)
{
	if (!valid_ || desc_.spec.type != TextureType::Tex2D || !data)
		return;

	GLenum format = fluo::gl::GetExternalFormat(desc_.spec.format);
	GLenum type = fluo::gl::GetDataType(desc_.spec.format);

	glBindTexture(glTarget(), id_);

	// Ensure correct unpack for tightly packed glyphs, images, etc.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage2D(
		GL_TEXTURE_2D,
		level,
		0, 0,
		width, height,
		format,
		type,
		data
	);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	if (desc_.spec.useMipmap && level == 0)
		glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::upload_2d_subimage(const void* data,
	int x, int y, int width, int height, int level)
{
	if (!valid_ || desc_.spec.type != TextureType::Tex2D || !data)
		return;

	GLenum format = fluo::gl::GetExternalFormat(desc_.spec.format);
	GLenum type = fluo::gl::GetDataType(desc_.spec.format);

	glBindTexture(glTarget(), id_);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexSubImage2D(
		GL_TEXTURE_2D,
		level,
		x, y,
		width, height,
		format,
		type,
		data
	);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	if (desc_.spec.useMipmap && level == 0)
		glGenerateMipmap(GL_TEXTURE_2D);
}

GLenum Texture::glTarget() const
{
	return desc_.spec.type == TextureType::Tex3D ?
		GL_TEXTURE_3D : GL_TEXTURE_2D;
}

void Texture::apply_parameters() const
{
	glTexParameteri(glTarget(), GL_TEXTURE_MIN_FILTER, fluo::gl::ToGLFilter(desc_.spec.minFilter));
	glTexParameteri(glTarget(), GL_TEXTURE_MAG_FILTER, fluo::gl::ToGLFilter(desc_.spec.magFilter));
	glTexParameteri(glTarget(), GL_TEXTURE_WRAP_S, fluo::gl::ToGLWrap(desc_.spec.wrapS));
	glTexParameteri(glTarget(), GL_TEXTURE_WRAP_T, fluo::gl::ToGLWrap(desc_.spec.wrapT));

	if (desc_.spec.type == TextureType::Tex3D)
		glTexParameteri(glTarget(), GL_TEXTURE_WRAP_R, fluo::gl::ToGLWrap(desc_.spec.wrapR));
}

void Texture::allocate_storage() const
{
	GLenum internal = fluo::gl::GetInternalFormat(desc_.spec.format);
	GLenum format = fluo::gl::GetExternalFormat(desc_.spec.format);
	GLenum type = fluo::gl::GetDataType(desc_.spec.format);

	if (desc_.spec.type == TextureType::Tex3D)
	{
		glTexImage3D(glTarget(), 0, static_cast<GLint>(internal),
			desc_.size.width, desc_.size.height, desc_.size.depth,
			0, format, type, nullptr);
	}
	else
	{
		glTexImage2D(glTarget(), 0, static_cast<GLint>(internal),
			desc_.size.width, desc_.size.height,
			0, format, type, nullptr);
	}
}