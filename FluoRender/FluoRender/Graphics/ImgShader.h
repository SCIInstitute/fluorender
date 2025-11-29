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
//  

#ifndef ImgShader_h
#define ImgShader_h

#include <ShaderProgram.h>

#define IMG_SHDR_TEXTURE_LOOKUP					0
#define IMG_SHDR_BRIGHTNESS_CONTRAST			1
#define IMG_SHDR_BRIGHTNESS_CONTRAST_HDR		2
#define IMG_SHDR_GRADIENT_MAP					3
#define IMG_SHDR_FILTER_BLUR					4
#define IMG_SHDR_FILTER_MAX						5
#define IMG_SHDR_FILTER_SHARPEN					6
#define IMG_SHDR_DEPTH_TO_OUTLINES				7
#define IMG_SHDR_DEPTH_TO_GRADIENT				8
#define IMG_SHDR_DEPTH_ACC_TO_GRADIENT			9
#define IMG_SHDR_DEPTH_TO_SCATTERING			10
#define IMG_SHDR_DEPTH_ACC_TO_DEPTH			11
#define IMG_SHDR_GRADIENT_TO_SHADOW				12
#define IMG_SHDR_BLEND_BRIGHT_BACKGROUND		13
#define IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR	14
#define IMG_SHDR_PAINT							15
#define	IMG_SHDR_DRAW_GEOMETRY					16
#define	IMG_SHDR_DRAW_GEOMETRY_COLOR3			17
#define	IMG_SHDR_DRAW_GEOMETRY_COLOR4			18
#define	IMG_SHDR_GRADIENT_PROJ_MAP				19
#define IMG_SHDR_DRAW_TEXT						20
#define IMG_SHDR_DRAW_THICK_LINES				21
#define IMG_SHDR_DRAW_THICK_LINES_COLOR			22
#define IMG_SHDR_GRADIENT_BACKGROUND			23
#define IMG_SHDR_FILTER_LANCZOS_BICUBIC			24
#define IMG_SHDR_TEXTURE_FLIP					25
#define IMG_SHDR_TEXTURE_EX_ALPHA				26

namespace flvr
{
	class ImgShaderFactory : public ShaderProgramFactory
	{
	public:
		std::shared_ptr<ShaderProgram> shader(const ShaderParams& params) override;

	protected:
		virtual bool emit_v(const ShaderParams& p, std::string& s) override;
		virtual bool emit_g(const ShaderParams& p, std::string& s) override;
		virtual bool emit_f(const ShaderParams& p, std::string& s) override;

		std::string get_colormap_code(int colormap);

	};

} // end namespace flvr

#endif // ImgShader_h
