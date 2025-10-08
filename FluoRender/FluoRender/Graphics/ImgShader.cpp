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

#include <string>
#include <sstream>
#include <iostream>
#include <ImgShader.h>
#include <ShaderProgram.h>
#include <VolShaderCode.h>
#include <ImgShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
	ImgShader::ImgShader(int type, int colormap) : 
		type_(type),
		colormap_(colormap),
		use_geom_shader_(false),
		program_(0)
	{
		if (type_ == IMG_SHDR_DRAW_THICK_LINES ||
			type_ == IMG_SHDR_DRAW_THICK_LINES_COLOR)
			use_geom_shader_ = true;
	}

	ImgShader::~ImgShader()
	{
		delete program_;
	}

	bool ImgShader::create()
	{
		string vs;
		if (emit_v(vs)) return true;
		string fs;
		if (emit_f(fs)) return true;
		string gs;
		if (use_geom_shader_)
		{
			if (emit_g(gs)) return true;
			program_ = new ShaderProgram(vs, fs, gs);
		}
		else
			program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool ImgShader::emit_v(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		switch (type_)
		{
		case IMG_SHDR_DRAW_GEOMETRY:
			z << IMG_VTX_CODE_DRAW_GEOMETRY;
			break;
		case IMG_SHDR_DRAW_THICK_LINES_COLOR:
			z << IMG_VTX_CODE_DRAW_GEOMETRY_COLOR_UNI;
			break;
		case IMG_SHDR_DRAW_GEOMETRY_COLOR3:
		case IMG_SHDR_DRAW_THICK_LINES:
			z << IMG_VTX_CODE_DRAW_GEOMETRY_COLOR3;
			break;
		case IMG_SHDR_DRAW_GEOMETRY_COLOR4:
			z << IMG_VTX_CODE_DRAW_GEOMETRY_COLOR4;
			break;
		case IMG_SHDR_DRAW_TEXT:
			z << IMG_VTX_CODE_DRAW_TEXT;
			break;
		case IMG_SHADER_TEXTURE_LOOKUP:
		case IMG_SHDR_BRIGHTNESS_CONTRAST:
		case IMG_SHDR_BRIGHTNESS_CONTRAST_HDR:
		case IMG_SHDR_GRADIENT_MAP:
		case IMG_SHDR_GRADIENT_PROJ_MAP:
		case IMG_SHDR_FILTER_BLUR:
		case IMG_SHDR_FILTER_MAX:
		case IMG_SHDR_FILTER_SHARPEN:
		case IMG_SHDR_DEPTH_TO_OUTLINES:
		case IMG_SHDR_DEPTH_TO_GRADIENT:
		case IMG_SHDR_GRADIENT_TO_SHADOW_VOL:
		case IMG_SHDR_GRADIENT_TO_SHADOW_MESH:
		case IMG_SHDR_BLEND_BRIGHT_BACKGROUND:
		case IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR:
		case IMG_SHDR_PAINT:
		case IMG_SHDR_GRADIENT_BACKGROUND:
		case IMG_SHDR_FILTER_LANCZOS_BICUBIC:
		case IMG_SHDR_TEXTURE_FLIP:
		default:
			z << IMG_VERTEX_CODE;
			break;
		}

		s = z.str();
		return false;
	}

	string ImgShader::get_colormap_code()
	{
		static const char* colormap_codes[] = {
			VOL_COLORMAP_CALC0,
			VOL_COLORMAP_CALC1,
			VOL_COLORMAP_CALC2,
			VOL_COLORMAP_CALC3,
			VOL_COLORMAP_CALC4,
			VOL_COLORMAP_CALC5,
			VOL_COLORMAP_CALC6,
			VOL_COLORMAP_CALC7,
			VOL_COLORMAP_CALC8
		};

		if (colormap_ >= 0 && colormap_ < 9)
			return std::string(colormap_codes[colormap_]);
		else
			return std::string(VOL_COLORMAP_CALC0); // default fallback
	}

	bool ImgShader::emit_f(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		switch (type_)
		{
		case IMG_SHDR_DRAW_GEOMETRY:
			z << IMG_FRG_CODE_DRAW_GEOMETRY;
			break;
		case IMG_SHDR_DRAW_GEOMETRY_COLOR3:
			z << IMG_FRG_CODE_DRAW_GEOMETRY_COLOR3;
			break;
		case IMG_SHDR_DRAW_THICK_LINES:
		case IMG_SHDR_DRAW_THICK_LINES_COLOR:
			z << IMG_FRG_CODE_DRAW_THICKLINES;
			break;
		case IMG_SHDR_DRAW_GEOMETRY_COLOR4:
			z << IMG_FRG_CODE_DRAW_GEOMETRY_COLOR4;
			break;
		case IMG_SHADER_TEXTURE_LOOKUP:
			z << IMG_SHADER_CODE_TEXTURE_LOOKUP;
			break;
		case IMG_SHDR_BRIGHTNESS_CONTRAST:
			z << IMG_SHADER_CODE_BRIGHTNESS_CONTRAST;
			break;
		case IMG_SHDR_BRIGHTNESS_CONTRAST_HDR:
			z << IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR;
			break;
		case IMG_SHDR_GRADIENT_MAP:
			z << IMG_SHADER_CODE_GRADIENT_MAP;
			z << get_colormap_code();
			z << IMG_SHADER_CODE_GRADIENT_MAP_RESULT;
			break;
		case IMG_SHDR_GRADIENT_PROJ_MAP:
			z << IMG_SHADER_CODE_GRADIENT_PROJ_MAP;
			z << get_colormap_code();
			z << IMG_SHADER_CODE_GRADIENT_PROJ_MAP_RESULT;
			break;
		case IMG_SHDR_FILTER_BLUR:
			z << IMG_SHADER_CODE_FILTER_BLUR;
			break;
		case IMG_SHDR_FILTER_MAX:
			z << IMG_SHADER_CODE_FILTER_MAX;
			break;
		case IMG_SHDR_FILTER_SHARPEN:
			z << IMG_SHADER_CODE_FILTER_SHARPEN;
			break;
		case IMG_SHDR_DEPTH_TO_OUTLINES:
			z << IMG_SHADER_CODE_DEPTH_TO_OUTLINES;
			break;
		case IMG_SHDR_DEPTH_TO_GRADIENT:
			z << IMG_SHADER_CODE_DEPTH_TO_GRADIENT;
			break;
		case IMG_SHDR_GRADIENT_TO_SHADOW_VOL:
			z << IMG_SHADER_CODE_GRAD2SHADOW_INPUT;
			z << IMG_SHADER_CODE_GRAD2SHADOW_HEAD;
			z << IMG_SHADER_CODE_GRAD2SHADOW_HEAD_VOL;
			z << IMG_SHADER_CODE_GRAD2SHADOW_BODY;
			break;
		case IMG_SHDR_GRADIENT_TO_SHADOW_MESH:
			z << IMG_SHADER_CODE_GRAD2SHADOW_INPUT;
			z << IMG_SHADER_CODE_GRAD2SHADOW_INPUT_MESH;
			z << IMG_SHADER_CODE_GRAD2SHADOW_HEAD;
			z << IMG_SHADER_CODE_GRAD2SHADOW_HEAD_MESH;
			z << IMG_SHADER_CODE_GRAD2SHADOW_BODY;
			break;
		case IMG_SHDR_BLEND_BRIGHT_BACKGROUND:
			z << IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND;
			break;
		case IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR:
			z << IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND_HDR;
			break;
		case IMG_SHDR_PAINT:
			z << PAINT_SHADER_CODE;
			break;
		case IMG_SHDR_DRAW_TEXT:
			z << IMG_FRG_CODE_DRAW_TEXT;
			break;
		case IMG_SHDR_GRADIENT_BACKGROUND:
			z << IMG_SHADER_CODE_GRADIENT_BACKGROUND;
			break;
		case IMG_SHDR_FILTER_LANCZOS_BICUBIC:
			z << IMG_SHADER_CODE_FILTER_LANCZOS_BICUBIC;
			break;
		case IMG_SHDR_TEXTURE_FLIP:
			z << IMG_SHADER_CODE_TEXTURE_FLIP;
			break;
		default:
			z << IMG_SHADER_CODE_TEXTURE_LOOKUP;
			break;
		}

		s = z.str();
		//std::cerr << s << std::endl;
		return false;
	}

	bool ImgShader::emit_g(std::string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		switch (type_)
		{
		case IMG_SHDR_DRAW_THICK_LINES:
		case IMG_SHDR_DRAW_THICK_LINES_COLOR:
			z << IMG_SHDR_CODE_DRAW_THICK_LINES;
			break;
		}

		s = z.str();
		return false;
	}

	ImgShaderFactory::ImgShaderFactory()
		: prev_shader_(-1)
	{}

	ImgShaderFactory::~ImgShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
			delete shader_[i];
	}

	void ImgShaderFactory::clear()
	{
		for (unsigned int i = 0; i<shader_.size(); i++)
			delete shader_[i];
		shader_.clear();
		prev_shader_ = -1;
	}

	ShaderProgram*
		ImgShaderFactory::shader(int type, int colormap)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(type, colormap))
				return shader_[prev_shader_]->program();
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(type, colormap)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		ImgShader* s = new ImgShader(type, colormap);
		if(s->create())
		{
			delete s;
			return 0;
		}
		shader_.push_back(s);
		prev_shader_ = int(shader_.size())-1;
		return s->program();
	}

} // end namespace flvr

