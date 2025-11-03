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

#include <FramebufferStateTracker.h>
#include <FramebufferStateManager.h>

using namespace flvr;

void FramebufferStateTracker::apply(const FramebufferState& desired)
{
	FramebufferStateManager::applyDiff(currentState_, desired);
	push_state();
	currentState_ = desired;
}

void FramebufferStateTracker::restore()
{
	if (state_stack_.empty())
		return;
	auto& s = state_stack_.back();
	FramebufferStateManager::applyDiff(currentState_, s);
	currentState_ = s;
	pop_state();
}

void FramebufferStateTracker::apply()
{
	FramebufferStateManager::applyFull(currentState_);
}

void FramebufferStateTracker::sync()
{
	currentState_ = FramebufferStateManager::capture();
}

//blend
void FramebufferStateTracker::set_blend_enabled(bool val)
{
	if (val == currentState_.enableBlend)
		return;
	currentState_.enableBlend = val;
}

void FramebufferStateTracker::set_blend_func(BlendFactor sfactor, BlendFactor dfactor)
{
	if (sfactor == currentState_.blendSrc && dfactor == currentState_.blendDst)
		return;
	currentState_.blendSrc = sfactor;
	currentState_.blendDst = dfactor;
}

void FramebufferStateTracker::set_blend_equation(BlendEquation rgb, BlendEquation alpha)
{
	if (rgb == currentState_.blendEquationRGB &&
		alpha == currentState_.blendEquationAlpha)
		return;
	currentState_.blendEquationRGB = rgb;
	currentState_.blendEquationAlpha = alpha;
}

//clear color
void FramebufferStateTracker::set_clear_color(const fluo::Vector4f& color)
{
	if (color == currentState_.clearColor)
		return;
	currentState_.clearColor = color;
}

//depth
void FramebufferStateTracker::set_depth_test_enabled(bool val)
{
	if (val == currentState_.enableDepthTest)
		return;
	currentState_.enableDepthTest = val;
}

void FramebufferStateTracker::set_depth_func(DepthFunc func)
{
	if (func == currentState_.depthFunc)
		return;
	currentState_.depthFunc = func;
}

void FramebufferStateTracker::set_clear_depth(float depth)
{
	if (depth == currentState_.clearDepth)
		return;
	currentState_.clearDepth = depth;
}

//cull
void FramebufferStateTracker::set_cull_face_enabled(bool val)
{
	if (val == currentState_.enableCullFace)
		return;
	currentState_.enableCullFace = val;
}

void FramebufferStateTracker::set_face_winding(FaceWinding winding)
{
	if (winding == currentState_.faceWinding)
		return;
	currentState_.faceWinding = winding;
}

void FramebufferStateTracker::set_cull_face(CullFace face)
{
	if (face == currentState_.cullFace)
		return;
	currentState_.cullFace = face;
}

//viewport
void FramebufferStateTracker::set_viewport(const fluo::Vector4i& vp)
{
	if (vp == currentState_.viewport)
		return;
	currentState_.viewport = vp;
}

//scissor
void FramebufferStateTracker::set_scissor_test_enabled(bool val)
{
	if (val == currentState_.enableScissorTest)
		return;
	currentState_.enableScissorTest = val;
}

void FramebufferStateTracker::set_scissor_rect(const fluo::Vector4i& rect)
{
	if (rect == currentState_.scissorRect)
		return;
	currentState_.scissorRect = rect;
}

void FramebufferStateTracker::set_polygon_mode(PolygonMode mode)
{
	if (mode == currentState_.polygonMode)
		return;
	currentState_.polygonMode = mode;
}

void FramebufferStateTracker::push_state()
{
	state_stack_.push_back(currentState_);
}

void FramebufferStateTracker::pop_state()
{
	if (!state_stack_.empty())
		state_stack_.pop_back();
}