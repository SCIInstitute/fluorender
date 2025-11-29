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

#include <ImgShader.h>
#include <VolShaderCode.h>
#include <ImgShaderCode.h>

using namespace flvr;

std::shared_ptr<ShaderProgram> ImgShaderFactory::shader(const ShaderParams& params)
{
	auto it = shader_map_.find(params);
	if (it != shader_map_.end())
		return it->second;

	bool use_geom_shader = params.type == IMG_SHDR_DRAW_THICK_LINES ||
		params.type == IMG_SHDR_DRAW_THICK_LINES_COLOR;

	std::string vs, fs, gs;
	bool valid_v = emit_v(params, vs);
	bool valid_f = emit_f(params, fs);
	bool valid_g = false;
	if (use_geom_shader)
		valid_g = emit_g(params, gs);

	std::shared_ptr<ShaderProgram> prog;
	if (use_geom_shader) {
		if (!valid_v || !valid_f || !valid_g) return nullptr;
		prog = std::make_shared<ShaderProgram>(vs, fs, gs);
	}
	else {
		if (!valid_v || !valid_f) return nullptr;
		prog = std::make_shared<ShaderProgram>(vs, fs);
	}

	shader_map_[params] = prog;

	return prog;
}

bool ImgShaderFactory::emit_v(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	switch (p.type)
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
	case IMG_SHDR_TEXTURE_LOOKUP:
	case IMG_SHDR_BRIGHTNESS_CONTRAST:
	case IMG_SHDR_BRIGHTNESS_CONTRAST_HDR:
	case IMG_SHDR_GRADIENT_MAP:
	case IMG_SHDR_GRADIENT_PROJ_MAP:
	case IMG_SHDR_FILTER_BLUR:
	case IMG_SHDR_FILTER_MAX:
	case IMG_SHDR_FILTER_SHARPEN:
	case IMG_SHDR_DEPTH_TO_OUTLINES:
	case IMG_SHDR_DEPTH_TO_GRADIENT:
	case IMG_SHDR_DEPTH_ACC_TO_GRADIENT:
	case IMG_SHDR_DEPTH_TO_SCATTERING:
	case IMG_SHDR_DEPTH_ACC_TO_DEPTH:
	case IMG_SHDR_GRADIENT_TO_SHADOW:
	case IMG_SHDR_BLEND_BRIGHT_BACKGROUND:
	case IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR:
	case IMG_SHDR_PAINT:
	case IMG_SHDR_GRADIENT_BACKGROUND:
	case IMG_SHDR_FILTER_LANCZOS_BICUBIC:
	case IMG_SHDR_TEXTURE_FLIP:
	case IMG_SHDR_TEXTURE_EX_ALPHA:
	default:
		z << IMG_VERTEX_CODE;
		break;
	}

	s = z.str();

	return true;
}

std::string ImgShaderFactory::get_colormap_code(int colormap)
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

	if (colormap >= 0 && colormap < 9)
		return std::string(colormap_codes[colormap]);
	else
		return std::string(VOL_COLORMAP_CALC0); // default fallback
}

bool ImgShaderFactory::emit_f(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	switch (p.type)
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
	case IMG_SHDR_TEXTURE_LOOKUP:
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
		z << get_colormap_code(p.colormap);
		z << IMG_SHADER_CODE_GRADIENT_MAP_RESULT;
		break;
	case IMG_SHDR_GRADIENT_PROJ_MAP:
		z << IMG_SHADER_CODE_GRADIENT_PROJ_MAP;
		z << get_colormap_code(p.colormap);
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
	case IMG_SHDR_DEPTH_ACC_TO_GRADIENT:
		z << IMG_SHADER_CODE_DEPTH_ACC_TO_GRADIENT;
		break;
	case IMG_SHDR_DEPTH_TO_SCATTERING:
		z << IMG_SHADER_CODE_DEPTH_TO_SCATTERING;
		break;
	case IMG_SHDR_DEPTH_ACC_TO_DEPTH:
		z << IMG_SHADER_CODE_DEPTH_ACC_TO_DEPTH;
		break;
	case IMG_SHDR_GRADIENT_TO_SHADOW:
		z << IMG_SHADER_CODE_GRAD2SHADOW_INPUT;
		z << IMG_SHADER_CODE_GRAD2SHADOW_HEAD;
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
	case IMG_SHDR_TEXTURE_EX_ALPHA:
		z << IMG_SHADER_CODE_TEXTURE_EX_ALPHA;
		break;
	default:
		z << IMG_SHADER_CODE_TEXTURE_LOOKUP;
		break;
	}

	s = z.str();

	return true;
}

bool ImgShaderFactory::emit_g(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	switch (p.type)
	{
	case IMG_SHDR_DRAW_THICK_LINES:
	case IMG_SHDR_DRAW_THICK_LINES_COLOR:
		z << IMG_SHDR_CODE_DRAW_THICK_LINES;
		break;
	}

	s = z.str();
	return true;
}


