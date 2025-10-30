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

#include <GL/glew.h>
#include <FramebufferStateManager.h>
#include <Debug.h>

using namespace flvr;

void FramebufferStateManager::applyFull(const FramebufferState& state)
{
	state.enableBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	glBlendFunc(toGLBlendFactor(state.blendSrc), toGLBlendFactor(state.blendDst));
	glBlendEquationSeparate(toGLBlendEquation(state.blendEquationRGB), toGLBlendEquation(state.blendEquationAlpha));

	glClearColor(state.clearColor[0], state.clearColor[1], state.clearColor[2], state.clearColor[3]);

	state.enableDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	glDepthFunc(toGLDepthFunc(state.depthFunc));
	glClearDepth(state.clearDepth);

	state.enableCullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	glFrontFace(toGLFaceWinding(state.faceWinding));
	glCullFace(toGLCullFace(state.cullFace));

	glViewport(state.viewport[0], state.viewport[1], state.viewport[2], state.viewport[3]);

	state.enableScissorTest ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
	glScissor(state.scissorRect[0], state.scissorRect[1], state.scissorRect[2], state.scissorRect[3]);

	glPolygonMode(GL_FRONT_AND_BACK, toGLPolygonMode(state.polygonMode));
}

void FramebufferStateManager::applyDiff(const FramebufferState& current, const FramebufferState& desired) {
	if (current.enableBlend != desired.enableBlend)
	{
		desired.enableBlend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
		DBGPRINT(L"glEnableBlend(%d)\n", desired.enableBlend);
	}

	if (current.blendSrc != desired.blendSrc || current.blendDst != desired.blendDst)
	{
		glBlendFunc(toGLBlendFactor(desired.blendSrc), toGLBlendFactor(desired.blendDst));
		DBGPRINT(L"glBlendFunc(%d, %d)\n", desired.blendSrc, desired.blendDst);
	}

	if (current.blendEquationRGB != desired.blendEquationRGB || current.blendEquationAlpha != desired.blendEquationAlpha)
	{
		glBlendEquationSeparate(toGLBlendEquation(desired.blendEquationRGB), toGLBlendEquation(desired.blendEquationAlpha));
		DBGPRINT(L"glBlendEquationSeparate(%d, %d)\n", desired.blendEquationRGB, desired.blendEquationAlpha);
	}

	if (current.clearColor != desired.clearColor)
	{
		glClearColor(desired.clearColor[0], desired.clearColor[1], desired.clearColor[2], desired.clearColor[3]);
		//DBGPRINT(L"glClearColor(%f, %f, %f, %f)\n", desired.clearColor[0], desired.clearColor[1], desired.clearColor[2], desired.clearColor[3]);
	}

	if (current.enableDepthTest != desired.enableDepthTest)
	{
		desired.enableDepthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		//DBGPRINT(L"glEnableDepthTest(%d)\n", desired.enableDepthTest);
	}

	if (current.depthFunc != desired.depthFunc)
	{
		glDepthFunc(toGLDepthFunc(desired.depthFunc));
		//DBGPRINT(L"glDepthFunc(%d)\n", desired.depthFunc);
	}

	if (current.clearDepth != desired.clearDepth)
	{
		glClearDepth(desired.clearDepth);
		//DBGPRINT(L"glClearDepth(%f)\n", desired.clearDepth);
	}

	if (current.enableCullFace != desired.enableCullFace)
	{
		desired.enableCullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		//DBGPRINT(L"glEnableCullFace(%d)\n", desired.enableCullFace);
	}

	if (current.faceWinding != desired.faceWinding)
	{
		glFrontFace(toGLFaceWinding(desired.faceWinding));
		//DBGPRINT(L"glFrontFace(%d)\n", desired.faceWinding);
	}

	if (current.cullFace != desired.cullFace)
	{
		glCullFace(toGLCullFace(desired.cullFace));
		//DBGPRINT(L"glCullFace(%d)\n", desired.cullFace);
	}

	if (current.viewport != desired.viewport)
	{
		glViewport(desired.viewport[0], desired.viewport[1], desired.viewport[2], desired.viewport[3]);
		//DBGPRINT(L"glViewport(%d, %d, %d, %d)\n", desired.viewport[0], desired.viewport[1], desired.viewport[2], desired.viewport[3]);
	}

	if (current.enableScissorTest != desired.enableScissorTest)
	{
		desired.enableScissorTest ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
		//DBGPRINT(L"glEnableScissorTest(%d)\n", desired.enableScissorTest);
	}

	if (current.scissorRect != desired.scissorRect)
	{
		glScissor(desired.scissorRect[0], desired.scissorRect[1], desired.scissorRect[2], desired.scissorRect[3]);
		//DBGPRINT(L"glScissor(%d, %d, %d, %d)\n", desired.scissorRect[0], desired.scissorRect[1], desired.scissorRect[2], desired.scissorRect[3]);
	}

	if (current.polygonMode != desired.polygonMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, toGLPolygonMode(desired.polygonMode));
		//DBGPRINT(L"glPolygonMode(%d)\n", desired.polygonMode);
	}
}

FramebufferState FramebufferStateManager::capture()
{
	FramebufferState s;

	// Enable flags
	s.enableDepthTest = glIsEnabled(GL_DEPTH_TEST);
	s.enableBlend = glIsEnabled(GL_BLEND);
	s.enableScissorTest = glIsEnabled(GL_SCISSOR_TEST);
	s.enableCullFace = glIsEnabled(GL_CULL_FACE);

	// Blend settings
	GLint blendSrc, blendDst, blendEqRGB, blendEqAlpha;
	glGetIntegerv(GL_BLEND_SRC, &blendSrc);
	glGetIntegerv(GL_BLEND_DST, &blendDst);
	glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEqRGB);
	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEqAlpha);

	s.blendSrc = static_cast<BlendFactor>(blendSrc);
	s.blendDst = static_cast<BlendFactor>(blendDst);
	s.blendEquationRGB = static_cast<BlendEquation>(blendEqRGB);
	s.blendEquationAlpha = static_cast<BlendEquation>(blendEqAlpha);

	// Depth settings
	GLint depthFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
	s.depthFunc = static_cast<DepthFunc>(depthFunc);

	GLfloat clearDepth;
	glGetFloatv(GL_DEPTH_CLEAR_VALUE, &clearDepth);
	s.clearDepth = clearDepth;

	// Cull settings
	GLint frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);
	s.faceWinding = (frontFace == GL_CCW) ? FaceWinding::Front : FaceWinding::Back;

	GLint cullFace;
	glGetIntegerv(GL_CULL_FACE_MODE, &cullFace);
	s.cullFace = static_cast<CullFace>(cullFace);

	// Clear color
	GLfloat clearColor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
	s.clearColor = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };

	// Scissor rectangle
	GLint scissor[4];
	glGetIntegerv(GL_SCISSOR_BOX, scissor);
	s.scissorRect = { scissor[0], scissor[1], scissor[2], scissor[3] };

	// Polygon mode
	GLint polygonMode[2];
	glGetIntegerv(GL_POLYGON_MODE, polygonMode);
	switch (polygonMode[0]) {
	case GL_POINT: s.polygonMode = PolygonMode::Point; break;
	case GL_LINE:  s.polygonMode = PolygonMode::Line;  break;
	case GL_FILL:  s.polygonMode = PolygonMode::Fill;  break;
	default:       s.polygonMode = PolygonMode::Fill;  break; // Fallback
	}

	return s;
}

GLenum FramebufferStateManager::toGLBlendFactor(BlendFactor factor)
{
	switch (factor)
	{
	case BlendFactor::Zero: return GL_ZERO;
	case BlendFactor::One: return GL_ONE;
	case BlendFactor::SrcColor: return GL_SRC_COLOR;
	case BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
	case BlendFactor::DstColor: return GL_DST_COLOR;
	case BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
	case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
	case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
	case BlendFactor::DstAlpha: return GL_DST_ALPHA;
	case BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
	case BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
	case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
	case BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
	case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
	case BlendFactor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
	default: return GL_ONE;
	}
}

GLenum FramebufferStateManager::toGLBlendEquation(BlendEquation eq)
{
	switch (eq)
	{
	case BlendEquation::Add: return GL_FUNC_ADD;
	case BlendEquation::Subtract: return GL_FUNC_SUBTRACT;
	case BlendEquation::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
	case BlendEquation::Min: return GL_MIN;
	case BlendEquation::Max: return GL_MAX;
	default: return GL_FUNC_ADD;
	}
}

GLenum FramebufferStateManager::toGLDepthFunc(DepthFunc func)
{
	switch (func)
	{
	case DepthFunc::Never: return GL_NEVER;
	case DepthFunc::Less: return GL_LESS;
	case DepthFunc::Equal: return GL_EQUAL;
	case DepthFunc::Lequal: return GL_LEQUAL;
	case DepthFunc::Greater: return GL_GREATER;
	case DepthFunc::Notequal: return GL_NOTEQUAL;
	case DepthFunc::Gequal: return GL_GEQUAL;
	case DepthFunc::Always: return GL_ALWAYS;
	default: return GL_LESS;
	}
}

GLenum FramebufferStateManager::toGLCullFace(CullFace face)
{
	switch (face)
	{
	case CullFace::Front: return GL_FRONT;
	case CullFace::Back: return GL_BACK;
	case CullFace::FrontAndBack: return GL_FRONT_AND_BACK;
	default: return GL_BACK;
	}
}

GLenum FramebufferStateManager::toGLFaceWinding(FaceWinding winding)
{
	switch (winding)
	{
	case FaceWinding::Front: return GL_CCW;  // Counter-clockwise is front-facing
	case FaceWinding::Back:  return GL_CW;   // Clockwise is front-facing
	case FaceWinding::Off:   return GL_CCW;  // Default fallback (won't be used if culling is disabled)
	default:                 return GL_CCW;
	}
}

GLenum FramebufferStateManager::toGLPolygonMode(PolygonMode mode)
{
	switch (mode) {
	case PolygonMode::Point: return GL_POINT;
	case PolygonMode::Line: return GL_LINE;
	case PolygonMode::Fill: return GL_FILL;
	}
	return GL_FILL;
}
