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

#include <glad/gl.h>
#include <GLTextureUtils.h>
#include <Texture.h>

namespace fluo::gl
{
	GLenum ToGLFilter(flvr::TexFilter f)
	{
		switch (f) {
		case flvr::TexFilter::Nearest: return GL_NEAREST;
		case flvr::TexFilter::Linear: return GL_LINEAR;
		case flvr::TexFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		default: return GL_LINEAR;
		}
	}

	GLenum ToGLWrap(flvr::TexWrap w)
	{
		switch (w) {
		case flvr::TexWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case flvr::TexWrap::Repeat: return GL_REPEAT;
		case flvr::TexWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
		default: return GL_CLAMP_TO_EDGE;
		}
	}

	GLenum GetInternalFormat(const flvr::TextureFormat& f)
	{
		using namespace flvr;

		if (f.layout == ChannelLayout::R)
		{
			if (f.component_type == DataFormat::UInt8 &&
				f.interpretation == ValueInterpretation::UNorm)
				return GL_R8;

			if (f.component_type == DataFormat::UInt16 &&
				f.interpretation == ValueInterpretation::UNorm)
				return GL_R16;

			if (f.component_type == DataFormat::Float32 &&
				f.interpretation == ValueInterpretation::Float)
				return GL_R32F;

			if (f.component_type == DataFormat::UInt32 &&
				f.interpretation == ValueInterpretation::UInt)
				return GL_R32UI;
		}

		if (f.layout == ChannelLayout::RG)
		{
			if (f.component_type == DataFormat::Float32 &&
				f.interpretation == ValueInterpretation::Float)
				return GL_RG32F;
		}

		if (f.layout == ChannelLayout::RGBA)
		{
			if (f.component_type == DataFormat::UInt8 &&
				f.interpretation == ValueInterpretation::UNorm)
				return GL_RGBA8;

			if (f.component_type == DataFormat::Float32 &&
				f.interpretation == ValueInterpretation::Float)
				return GL_RGBA32F;
		}

		if (f.layout == ChannelLayout::Depth)
		{
			if (f.component_type == DataFormat::Float32 &&
				f.interpretation == ValueInterpretation::Float)
				return GL_DEPTH_COMPONENT32F;
		}

		return GL_NONE;
	}

	GLenum GetExternalFormat(const flvr::TextureFormat& f)
	{
		using namespace flvr;

		switch (f.layout)
		{
		case ChannelLayout::R:
			return (f.interpretation == ValueInterpretation::UInt) ?
				GL_RED_INTEGER : GL_RED;

		case ChannelLayout::RG:
			return (f.interpretation == ValueInterpretation::UInt) ?
				GL_RG_INTEGER : GL_RG;

		case ChannelLayout::RGB:
			return (f.interpretation == ValueInterpretation::UInt) ?
				GL_RGB_INTEGER : GL_RGB;

		case ChannelLayout::RGBA:
			return (f.interpretation == ValueInterpretation::UInt) ?
				GL_RGBA_INTEGER : GL_RGBA;

		case ChannelLayout::Depth:
			return GL_DEPTH_COMPONENT;

		default:
			return GL_NONE;
		}
	}

	GLenum GetDataType(const flvr::TextureFormat& f)
	{
		using namespace flvr;

		switch (f.component_type)
		{
		case DataFormat::UInt8:
			return GL_UNSIGNED_BYTE;

		case DataFormat::UInt16:
			return GL_UNSIGNED_SHORT;

		case DataFormat::UInt32:
			return GL_UNSIGNED_INT;

		case DataFormat::Int8:
			return GL_BYTE;

		case DataFormat::Int16:
			return GL_SHORT;

		case DataFormat::Int32:
			return GL_INT;

		case DataFormat::Float16:
			return GL_HALF_FLOAT;

		case DataFormat::Float32:
			return GL_FLOAT;

		case DataFormat::Float64:
			return GL_DOUBLE;

		default:
			return GL_NONE;
		}
	}
}