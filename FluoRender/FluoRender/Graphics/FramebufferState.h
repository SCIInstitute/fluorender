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

#ifndef FramebufferState_h
#define FramebufferState_h

#include <Vector4f.h>
#include <Vector4i.h>

namespace flvr
{
	enum class BlendFactor : int
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SrcAlphaSaturate
	};

	enum class BlendEquation : int
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class DepthFunc : int
	{
		Never,
		Less,
		Equal,
		Lequal,
		Greater,
		Notequal,
		Gequal,
		Always
	};

	enum class CullFace : int
	{
		Front,
		Back,
		FrontAndBack
	};

	enum class FaceWinding : int
	{
		Front,
		Back,
		Off
	};

	enum class PolygonMode : int
	{
		Point,
		Line,
		Fill
	};

	struct FramebufferState
	{
		// Blend settings
		bool enableBlend = false;
		BlendFactor blendSrc = BlendFactor::One;
		BlendFactor blendDst = BlendFactor::OneMinusSrcAlpha;
		BlendEquation blendEquationRGB = BlendEquation::Add;
		BlendEquation blendEquationAlpha = BlendEquation::Add;

		// Clear color
		fluo::Vector4f clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

		// Depth settings
		bool enableDepthTest = false;
		DepthFunc depthFunc = DepthFunc::Lequal;
		float clearDepth = 1.0f;

		// Cull settings
		bool enableCullFace = false;
		FaceWinding faceWinding = FaceWinding::Off;
		CullFace cullFace = CullFace::Back;

		// Viewport rectangle
		fluo::Vector4i viewport = { 0, 0, 0, 0 };

		// Scissor rectangle
		bool enableScissorTest = false;
		fluo::Vector4i scissorRect = { 0, 0, 0, 0 };

		// Polygon mode
		PolygonMode polygonMode = PolygonMode::Fill;

		// Equality operator for diffing
		bool operator==(const FramebufferState& other) const;
		bool operator!=(const FramebufferState& other) const { return !(*this == other); }
	};
}

#endif//FramebufferState_h