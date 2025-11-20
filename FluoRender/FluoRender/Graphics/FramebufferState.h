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
#include <unordered_map>

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

	struct BlendState
	{
		bool enabled = false;

		bool maskRed = true;
		bool maskGreen = true;
		bool maskBlue = true;
		bool maskAlpha = true;

		BlendFactor srcRGB = BlendFactor::One;
		BlendFactor dstRGB = BlendFactor::OneMinusSrcAlpha;
		BlendFactor srcAlpha = BlendFactor::One;
		BlendFactor dstAlpha = BlendFactor::OneMinusSrcAlpha;

		BlendEquation eqRGB = BlendEquation::Add;
		BlendEquation eqAlpha = BlendEquation::Add;

		bool operator==(const BlendState& other) const
		{
			return enabled == other.enabled &&
				maskRed == other.maskRed &&
				maskGreen == other.maskGreen &&
				maskBlue == other.maskBlue &&
				maskAlpha == other.maskAlpha &&
				srcRGB == other.srcRGB &&
				dstRGB == other.dstRGB &&
				srcAlpha == other.srcAlpha &&
				dstAlpha == other.dstAlpha &&
				eqRGB == other.eqRGB &&
				eqAlpha == other.eqAlpha;
		}
	};

	struct FramebufferState
	{
		// Blend settings per draw buffer index
		std::unordered_map<int, BlendState> blendStates;

		// Enabled flags per draw buffer index
		std::unordered_map<int, bool> drawEnabled;

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

		//set up for two attachments
		FramebufferState()
		{
			blendStates[0] = BlendState();
			blendStates[1] = BlendState();
			drawEnabled[0] = true; // default: enabled
			drawEnabled[1] = true;
		}

		// Equality operator for diffing
		bool operator==(const FramebufferState& other) const;
		bool operator!=(const FramebufferState& other) const { return !(*this == other); }
	};
}

#endif//FramebufferState_h