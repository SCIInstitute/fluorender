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

#include <GLPixelFormat.h>

namespace fluo::gl
{
	GLenum ToGLType(PixelFormat fmt) noexcept
	{
		switch (fmt)
		{
		case PixelFormat::R8_UNorm:   return GL_UNSIGNED_BYTE;
		case PixelFormat::R16_UNorm:  return GL_UNSIGNED_SHORT;
		case PixelFormat::R32_UNorm:  return GL_UNSIGNED_INT;

		case PixelFormat::R8_SNorm:   return GL_BYTE;
		case PixelFormat::R16_SNorm:  return GL_SHORT;
		case PixelFormat::R32_SNorm:  return GL_INT;

		case PixelFormat::R32_Float:  return GL_FLOAT;
		case PixelFormat::R64_Float:  return GL_DOUBLE;

		default:
			return GL_NONE;
		}
	}
}