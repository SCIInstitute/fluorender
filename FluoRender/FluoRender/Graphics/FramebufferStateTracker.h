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

#ifndef FramebufferStateTracker_h
#define FramebufferStateTracker_h

#include <FramebufferState.h>

namespace flvr
{
	class FramebufferStateTracker {
	public:
		// Apply a new desired state by diffing against current tracked state
		void apply(const FramebufferState& desired);
		void apply();//reapply current state to gl

		// Reset tracker to gl state (e.g. on context loss)
		void sync();

		// Access current tracked state
		const FramebufferState& current() const { return currentState_; }

		//fine grained state control
		// Blend
		void set_blend_enabled(bool val);
		void set_blend_func(BlendFactor sfactor, BlendFactor dfactor);
		void set_blend_equation(BlendEquation rgb, BlendEquation alpha);
		// Depth
		void set_depth_test_enabled(bool val);
		void set_depth_func(DepthFunc func);
		void set_clear_depth(float depth);
		// Cull
		void set_cull_face_enabled(bool val);
		void set_face_winding(FaceWinding winding);
		void set_cull_face(CullFace face);
		//scissor
		void set_scissor_test_enabled(bool val);
		void set_scissor_rect(const fluo::Vector4i& rect);
		//viewport
		void set_viewport(const fluo::Vector4i& vp);
		//clear color
		void set_clear_color(const fluo::Vector4f& color);
		//polygon mode
		void set_polygon_mode(PolygonMode mode);

	private:
		FramebufferState currentState_;
	};
}

#endif//FramebufferStateTracker_h