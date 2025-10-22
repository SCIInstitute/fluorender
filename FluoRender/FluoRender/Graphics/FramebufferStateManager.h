//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#ifndef FramebufferStateManager_h
#define FramebufferStateManager_h

#include <FramebufferState.h>

#ifndef __glew_h__
typedef unsigned int GLenum;
#endif

namespace flvr
{
	class FramebufferStateManager
	{
	public:
		// Apply full state unconditionally
		static void applyFull(const FramebufferState& state);

		// Apply only differences between current and desired state
		static void applyDiff(const FramebufferState& current, const FramebufferState& desired);

		static FramebufferState capture();

	private:
		// Internal helpers
		static GLenum toGLBlendFactor(BlendFactor factor);
		static GLenum toGLBlendEquation(BlendEquation eq);
		static GLenum toGLDepthFunc(DepthFunc func);
		static GLenum toGLCullFace(CullFace face);
		static GLenum toGLFaceWinding(FaceWinding winding);
		static GLenum toGLPolygonMode(PolygonMode mode);
	};
}

#endif//FramebufferStateManager_h