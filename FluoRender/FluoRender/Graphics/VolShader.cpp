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

#include <VolShader.h>
#include <VolShaderCode.h>
#include <array>

using namespace flvr;

std::shared_ptr<ShaderProgram> VolShaderFactory::shader(const ShaderParams& params)
{
	auto it = shader_map_.find(params);
	if (it != shader_map_.end())
		return it->second;

	std::string vs;
	bool valid_v = emit_v(params, vs);
	std::string fs;
	bool valid_f = emit_f(params, fs);

	if (!valid_v || !valid_f) return nullptr;
	std::shared_ptr<ShaderProgram> prog = std::make_shared<ShaderProgram>(vs, fs);
	shader_map_[params] = prog;

	return prog;
}

bool VolShaderFactory::emit_v(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	if (p.fog)
		z << VTX_SHADER_CODE_FOG;
	else
		z << VTX_SHADER_CODE_CORE_PROFILE;

	s = z.str();
	return true;
}

std::string VolShaderFactory::get_colormap_code(int colormap, ColormapProj colormap_proj)
{
	if (colormap == 0 && colormap_proj == ColormapProj::Normal)
		return std::string(VOL_COLORMAP_DIFF_CALC0);

	static const std::string colormap_codes[] = {
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

	return std::string(VOL_COLORMAP_CALC0); // default fallback
}

std::string VolShaderFactory::get_colormap_proj(ColormapProj colormap_proj)
{
	static const std::array<std::string, 9> colormap_proj_codes = {
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU0,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU1,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU2,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU3,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU4,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU5,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU6,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU7,
		VOL_TRANSFER_FUNCTION_COLORMAP_VALU8
	};

	size_t index = static_cast<size_t>(colormap_proj) - 1;
	if (index < colormap_proj_codes.size())
		return colormap_proj_codes[index];

	return std::string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU0); // default fallback
}

bool VolShaderFactory::emit_f(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	if (p.poly)
	{
		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		z << FRG_SHADER_CODE_CORE_PROFILE;
		//output
		s = z.str();
		return false;
	}

	//version info
	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	z << VOL_INPUTS;
	if (p.fog)
		z << VOL_INPUTS_FOG;
	z << VOL_OUTPUTS;

	//the common uniforms
	z << VOL_UNIFORMS_COMMON;

	//time for 4d colormap
	if (p.colormap_proj == ColormapProj::TValue)
		z << VOL_UNIFROMS_4D_COLORMAP;

	//neighbors for 4d colormap
	if (ShaderParams::IsTimeProj(p.colormap_proj))
		z << VOL_UNIFORMS_4D_CACHE;

	//color modes
	if (p.color_mode == ColorMode::Depth)
		z << VOL_UNIFORMS_DEPTHMAP;

	// add uniform for depth peeling
	if (p.peel != 0)
		z << VOL_UNIFORMS_DP;

	// add uniforms for clipping
	if (p.clip)
	{
		z << VOL_UNIFORMS_CLIP;
		z << VOL_UNIFORMS_MATRICES;
	}

	// add uniforms for masking
	switch (p.mask)
	{
	case 1:
	case 2:
		z << VOL_UNIFORMS_MASK;
		break;
	case 3:
		z << VOL_UNIFORMS_LABEL;
		break;
	case 4:
		z << VOL_UNIFORMS_MASK;
		z << VOL_UNIFORMS_LABEL;
		break;
	}

	//functions
	if (p.clip)
		z << VOL_CLIP_FUNC;
	//the common head
	z << VOL_HEAD;

	if (p.peel != 0 || p.color_mode == ColorMode::Depth)
		z << VOL_HEAD_2DMAP_LOC;

	//head for depth peeling
	if (p.peel == 1)//draw volume before 15
		z << VOL_HEAD_DP_1;
	else if (p.peel == 2 || p.peel == 5)//draw volume after 15
		z << VOL_HEAD_DP_2;
	else if (p.peel == 3 || p.peel == 4)//draw volume after 14 and before 15
		z << VOL_HEAD_DP_3;

	//head for clipping planes
	if (p.clip)
		z << VOL_HEAD_CLIP_FUNC;

	// Set up light variables and input parameters.
	z << VOL_HEAD_LIT;

	// Set up fog variables and input parameters.
	if (p.fog)
		z << VOL_HEAD_FOG;

	//bodies
	if (p.shading)
	{
		//no gradient volume, need to calculate in real-time
		z << VOL_DATA_VOLUME_LOOKUP;
		z << VOL_GRAD_COMPUTE;
		z << VOL_BODY_SHADING;

		if (p.channels == 1)
			z << VOL_COMPUTED_GM_LOOKUP;
		else
			z << VOL_TEXTURE_GM_LOOKUP;

		if (p.colormap_proj == ColormapProj::IntDelta)
			z << VOL_DATA_4D_INTENSITY_DELTA;
		else if (p.colormap_proj == ColormapProj::Speed)
			z << VOL_DATA_4D_SPEED;

		switch (p.color_mode)
		{
		case ColorMode::SingleColor://normal
			if (p.solid)
				z << VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID;
			else
				z << VOL_TRANSFER_FUNCTION_SIN_COLOR;
			break;
		case ColorMode::Colormap://colormap
			if (p.solid)
			{
				z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID;
				z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
				z << get_colormap_proj(p.colormap_proj);
				z << get_colormap_code(p.colormap, p.colormap_proj);
				z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT;
			}
			else
			{
				z << VOL_TRANSFER_FUNCTION_COLORMAP;
				z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
				z << get_colormap_proj(p.colormap_proj);
				z << get_colormap_code(p.colormap, p.colormap_proj);
				z << VOL_TRANSFER_FUNCTION_COLORMAP_RESULT;
			}
			break;
		}

		z << VOL_COLOR_OUTPUT;
	}
	else // No shading
	{
		z << VOL_DATA_VOLUME_LOOKUP;

		if (p.channels == 1)
		{
			// Compute Gradient magnitude and use it.
			if (p.grad ||
				p.colormap_proj == ColormapProj::Speed)
			{
				z << VOL_GRAD_COMPUTE;
				z << VOL_COMPUTED_GM_LOOKUP;
			}
			else
				z << VOL_COMPUTED_GM_NOUSE;
		}
		else
		{
			z << VOL_TEXTURE_GM_LOOKUP;
		}

		if (p.colormap_proj == ColormapProj::IntDelta)
			z << VOL_DATA_4D_INTENSITY_DELTA;
		else if (p.colormap_proj == ColormapProj::Speed)
			z << VOL_DATA_4D_SPEED;

		if (p.render_mode == RenderMode::Mip &&
			ShaderParams::ValidColormapProj(p.colormap_proj))
		{
			//this is the colormap mode for mip
			//mip without colormap is rendered the same as standard
			z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_HEAD;
			if (p.solid)
				z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_ZERO_SOLID;
			else
				z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_ZERO;
			z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_TF;
			z << get_colormap_proj(p.colormap_proj);
			z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_ENCODE;
			if (p.solid)
				z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_SOLID;
			else
				z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_TRANSP;
			z << VOL_RASTER_BLEND_SOLID;
		}
		else
		{
			switch (p.color_mode)
			{
			case ColorMode::SingleColor://normal
				if (p.solid)
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID;
				else
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR;
				break;
			case ColorMode::Colormap://colormap
				if (p.solid)
				{
					z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID;
					z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
					z << get_colormap_proj(p.colormap_proj);
					z << get_colormap_code(p.colormap, p.colormap_proj);
					z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT;
				}
				else
				{
					z << VOL_TRANSFER_FUNCTION_COLORMAP;
					z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
					z << get_colormap_proj(p.colormap_proj);
					z << get_colormap_code(p.colormap, p.colormap_proj);
					z << VOL_TRANSFER_FUNCTION_COLORMAP_RESULT;
				}
				break;
			case ColorMode::Depth://depth map
				z << VOL_TRANSFER_FUNCTION_DEPTHMAP;
				break;
			}
		}
	}

	//if (!(p.mip && p.colormap_proj))
	if (!(p.render_mode == RenderMode::Mip &&
		ShaderParams::ValidColormapProj(p.colormap_proj)))
	{
		// fog
		if (p.fog)
		{
			z << VOL_FOG_BODY;
		}

		//final blend
		switch (p.mask)
		{
		case 0:
			if (p.color_mode == ColorMode::Depth)
				z << VOL_RASTER_BLEND_DMAP;
			else
			{
				if (p.solid)
					z << VOL_RASTER_BLEND_SOLID;
				else
					z << VOL_RASTER_BLEND;
			}
			break;
		case 1:
			if (p.color_mode == ColorMode::Depth)
				z << VOL_RASTER_BLEND_MASK_DMAP;
			else
			{
				if (p.solid)
					z << VOL_RASTER_BLEND_MASK_SOLID;
				else
					z << VOL_RASTER_BLEND_MASK;
			}
			break;
		case 2:
			if (p.color_mode == ColorMode::Depth)
				z << VOL_RASTER_BLEND_NOMASK_DMAP;
			else
			{
				if (p.solid)
					z << VOL_RASTER_BLEND_NOMASK_SOLID;
				else
					z << VOL_RASTER_BLEND_NOMASK;
			}
			break;
		case 3:
			z << VOL_RASTER_BLEND_LABEL;
			z << VOL_COLOR_OUTPUT_LABEL;
			break;
		case 4:
			z << VOL_RASTER_BLEND_LABEL_MASK;
			if (p.solid)
				z << VOL_COLOR_OUTPUT_LABEL_MASK_SOLID;
			else
				z << VOL_COLOR_OUTPUT_LABEL_MASK;
			break;
		}
	}

	//the common tail
	z << VOL_TAIL;

	//output
	s = z.str();
	return true;
}
