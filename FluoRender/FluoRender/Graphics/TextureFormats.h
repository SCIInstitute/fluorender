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

#ifndef TextureFormats_h
#define TextureFormats_h

#include <RawData.h>
#include <Texture.h>

namespace flvr
{
	inline constexpr TextureFormat R8_UNorm
	{
		ChannelLayout::R,
		fluo::DataFormat::UInt8,
		ValueInterpretation::UNorm
	};

	inline constexpr TextureFormat R16_UNorm
	{
		ChannelLayout::R,
		fluo::DataFormat::UInt16,
		ValueInterpretation::UNorm
	};

	inline constexpr TextureFormat R32UI
	{
		ChannelLayout::R,
		fluo::DataFormat::UInt32,
		ValueInterpretation::UInt
	};

	inline constexpr TextureFormat R32F
	{
		ChannelLayout::R,
		fluo::DataFormat::Float32,
		ValueInterpretation::Float
	};

	inline constexpr TextureFormat RG32F
	{
		ChannelLayout::RG,
		fluo::DataFormat::Float32,
		ValueInterpretation::Float
	};

	inline constexpr TextureFormat RGBA8
	{
		ChannelLayout::RGBA,
		fluo::DataFormat::UInt8,
		ValueInterpretation::UNorm
	};

	inline constexpr TextureFormat RGBA32F
	{
		ChannelLayout::RGBA,
		fluo::DataFormat::Float32,
		ValueInterpretation::Float
	};

	inline constexpr TextureFormat Depth32F
	{
		ChannelLayout::Depth,
		fluo::DataFormat::Float32,
		ValueInterpretation::Float
	};
}

#endif//TextureFormats_h