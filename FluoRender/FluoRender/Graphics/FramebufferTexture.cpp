//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2026 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <GL/glew.h>
#include <FramebufferTexture.h>

using namespace flvr;

FramebufferTexture::FramebufferTexture(const FBTexConfig& config, int nx, int ny) :
	config_(config), nx_(nx), ny_(ny)
{
}

FramebufferTexture::~FramebufferTexture()
{
	destroy();
}

void FramebufferTexture::create()
{
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);

	// Scoped lambdas for enum translation
	auto toGLFilter = [](TexFilter f) -> GLint {
		switch (f) {
		case TexFilter::Nearest: return GL_NEAREST;
		case TexFilter::Linear: return GL_LINEAR;
		case TexFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_LINEAR;
		}
		};

	auto toGLWrap = [](TexWrap w) -> GLint {
		switch (w) {
		case TexWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case TexWrap::Repeat: return GL_REPEAT;
		case TexWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
		default: return GL_CLAMP_TO_EDGE;
		}
		};

	// Apply filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(config_.minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(config_.magFilter));

	// Apply wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(config_.wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(config_.wrapT));

	// Allocate texture storage based on type
	switch (config_.type)
	{
	case FBTexType::Render_RGBA:
	default:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
			GL_RGBA, GL_FLOAT, nullptr);
		break;

	case FBTexType::UChar_RGBA:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nx_, ny_, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		break;

	case FBTexType::Render_Int32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, nx_, ny_, 0,
			GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		break;

	case FBTexType::Render_Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, nx_, ny_, 0,
			GL_RED, GL_FLOAT, nullptr);
		break;

	case FBTexType::Render_FloatRG:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, nx_, ny_, 0,
			GL_RG, GL_FLOAT, nullptr);
		break;

	case FBTexType::Depth_Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, nx_, ny_, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	}

	// Generate mipmaps if requested
	if (config_.useMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	valid_ = true;
}

void FramebufferTexture::destroy()
{
	if (!valid_ || id_ == 0)
		return;
	glDeleteTextures(1, &id_);
	id_ = 0;
	valid_ = false;
}

bool FramebufferTexture::bind(int tex_unit)
{
	if (!valid_ || id_ == 0) return false;

	glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(GL_TEXTURE_2D, id_);
	tex_unit_ = tex_unit;
	return true;
}

void FramebufferTexture::unbind()
{
	if (tex_unit_ < 0) return; // Not bound

	glActiveTexture(GL_TEXTURE0 + tex_unit_);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	tex_unit_ = -1;
}

void FramebufferTexture::resize(int nx, int ny)
{
	if (!valid_ || id_ == 0) return;

	nx_ = nx;
	ny_ = ny;

	glBindTexture(GL_TEXTURE_2D, id_);

	// Scoped lambdas for enum translation
	auto toGLFilter = [](TexFilter f) -> GLint {
		switch (f) {
		case TexFilter::Nearest: return GL_NEAREST;
		case TexFilter::Linear: return GL_LINEAR;
		case TexFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_LINEAR;
		}
		};

	auto toGLWrap = [](TexWrap w) -> GLint {
		switch (w) {
		case TexWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case TexWrap::Repeat: return GL_REPEAT;
		case TexWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
		default: return GL_CLAMP_TO_EDGE;
		}
		};

	// Reapply filtering and wrapping (in case driver resets on reallocation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(config_.minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(config_.magFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(config_.wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(config_.wrapT));

	// Reallocate texture storage
	switch (config_.type)
	{
	case FBTexType::Render_RGBA:
	default:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
			GL_RGBA, GL_FLOAT, nullptr);
		break;

	case FBTexType::UChar_RGBA:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nx_, ny_, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		break;

	case FBTexType::Render_Int32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, nx_, ny_, 0,
			GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		break;

	case FBTexType::Render_Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, nx_, ny_, 0,
			GL_RED, GL_FLOAT, nullptr);
		break;

	case FBTexType::Render_FloatRG:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, nx_, ny_, 0,
			GL_RG, GL_FLOAT, nullptr);
		break;

	case FBTexType::Depth_Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, nx_, ny_, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	}

	// Regenerate mipmaps if needed
	if (config_.useMipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

