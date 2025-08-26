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
#include <VolShader.h>
#include <ShaderProgram.h>
#include <VolShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
VolShader::VolShader(
	bool poly, int channels,
	bool shading, bool fog,
	int peel, bool clip,
	bool grad, int mask, bool mip,
	int color_mode, int colormap, int colormap_proj,
	bool solid, int vertex_shader)
	: poly_(poly),
	channels_(channels),
	shading_(shading),
	fog_(fog),
	peel_(peel),
	clip_(clip),
	grad_(grad),
	mask_(mask),
	mip_(mip),
	color_mode_(color_mode),
	colormap_(colormap),
	colormap_proj_(colormap_proj),
	solid_(solid),
	vertex_type_(vertex_shader),
	program_(0)
	{
	}

	VolShader::~VolShader()
	{
		delete program_;
	}

	bool VolShader::create()
	{
		string fs,vs;
		if (emit_f(fs)) return true;
		if (emit_v(vs)) return true;
		program_ = new ShaderProgram(vs,fs);
		return false;
	}

	bool VolShader::emit_v(string& s)
	{
		ostringstream z;
		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		if (fog_)
			z << VTX_SHADER_CODE_FOG;
		else
			z << VTX_SHADER_CODE_CORE_PROFILE;

		s = z.str();
		return false;
	}

	string VolShader::get_colormap_code()
	{
		if (colormap_ == 0 && colormap_proj_ == 6)
			return string(VOL_COLORMAP_DIFF_CALC0);

		static const string colormap_codes[] = {
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
			return string(colormap_codes[colormap_]);

		return string(VOL_COLORMAP_CALC0); // default fallback
	}

	string VolShader::get_colormap_proj()
	{
		static const string colormap_proj_codes[] = {
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

		if (colormap_proj_ >= 0 && colormap_proj_ < 9)
			return string(colormap_proj_codes[colormap_proj_]);

		return string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU0); // default fallback
	}

	bool VolShader::emit_f(string& s)
	{
		ostringstream z;

		if (poly_)
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
		if (fog_)
			z << VOL_INPUTS_FOG;
		z << VOL_OUTPUTS;

		//the common uniforms
		z << VOL_UNIFORMS_COMMON;

		//time for 4d colormap
		if (colormap_proj_ == 4)
			z << VOL_UNIFROMS_4D_COLORMAP;

		//neighbors for 4d colormap
		if (colormap_proj_ >= 7)
			z << VOL_UNIFORMS_4D_CACHE;

		//2d map location
		if (peel_ != 0 || color_mode_ == 2)
			z << VOL_UNIFORMS_2DMAP_LOC;

		//color modes
		switch (color_mode_)
		{
		case 0://normal
			z << VOL_UNIFORMS_SIN_COLOR;
			break;
		case 1://colormap
			z << VOL_UNIFORMS_COLORMAP;
			break;
		case 2://depth map
			z << VOL_UNIFORMS_SIN_COLOR;
			z << VOL_UNIFORMS_DEPTHMAP;
			break;
		}

		// add uniform for depth peeling
		if (peel_ != 0)
			z << VOL_UNIFORMS_DP;

		// add uniforms for clipping
		if (clip_)
		{
			z << VOL_UNIFORMS_CLIP;
			z << VOL_UNIFORMS_MATRICES;
		}

		// add uniforms for masking
		switch (mask_)
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

		//uniform for fog
		if (fog_)
			z << VOL_UNIFORMS_FOG_LOC;

		//functions
		if (clip_)
			z << VOL_CLIP_FUNC;
		//the common head
		z << VOL_HEAD;

		if (peel_ != 0 || color_mode_ == 2)
			z << VOL_HEAD_2DMAP_LOC;

		//head for depth peeling
		if (peel_ == 1)//draw volume before 15
			z << VOL_HEAD_DP_1;
		else if (peel_ == 2 || peel_ == 5)//draw volume after 15
			z << VOL_HEAD_DP_2;
		else if (peel_ == 3 || peel_ == 4)//draw volume after 14 and before 15
			z << VOL_HEAD_DP_3;

		//head for clipping planes
		if (clip_)
			z << VOL_HEAD_CLIP_FUNC;

		// Set up light variables and input parameters.
		z << VOL_HEAD_LIT;

		// Set up fog variables and input parameters.
		if (fog_)
			z << VOL_HEAD_FOG;

		//bodies
		if (shading_)
		{
			//no gradient volume, need to calculate in real-time
			z << VOL_DATA_VOLUME_LOOKUP;
			z << VOL_GRAD_COMPUTE;
			z << VOL_BODY_SHADING;

			if (channels_ == 1)
				z << VOL_COMPUTED_GM_LOOKUP;
			else
				z << VOL_TEXTURE_GM_LOOKUP;

			if (colormap_proj_ == 7)
				z << VOL_DATA_4D_INTENSITY_DELTA;
			else if (colormap_proj_ == 8)
				z << VOL_DATA_4D_SPEED;

			switch (color_mode_)
			{
			case 0://normal
				if (solid_)
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID;
				else
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR;
				break;
			case 1://colormap
				if (solid_)
				{
					z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID;
					z << get_colormap_proj();
					z << get_colormap_code();
					z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
					z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT;
				}
				else
				{
					z << VOL_TRANSFER_FUNCTION_COLORMAP;
					z << get_colormap_proj();
					z << get_colormap_code();
					z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
					z << VOL_TRANSFER_FUNCTION_COLORMAP_RESULT;
				}
				break;
			}

			z << VOL_COLOR_OUTPUT;
		}
		else // No shading
		{
			z << VOL_DATA_VOLUME_LOOKUP;

			if (channels_ == 1)
			{
				// Compute Gradient magnitude and use it.
				if (grad_ ||
					colormap_proj_  >= 8)
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

			if (colormap_proj_ == 7)
				z << VOL_DATA_4D_INTENSITY_DELTA;
			else if (colormap_proj_ == 8)
				z << VOL_DATA_4D_SPEED;

			if (mip_ && colormap_proj_)
			{
				z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ;
				z << get_colormap_proj();
				z << VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT;
				z << VOL_RASTER_BLEND_SOLID;
			}
			else
			{
				switch (color_mode_)
				{
				case 0://normal
					if (solid_)
						z << VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID;
					else
						z << VOL_TRANSFER_FUNCTION_SIN_COLOR;
					break;
				case 1://colormap
					if (solid_)
					{
						z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID;
						z << get_colormap_proj();
						z << get_colormap_code();
						z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
						z << VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT;
					}
					else
					{
						z << VOL_TRANSFER_FUNCTION_COLORMAP;
						z << get_colormap_proj();
						z << get_colormap_code();
						z << VOL_COMMON_TRANSFER_FUNCTION_CALC;
						z << VOL_TRANSFER_FUNCTION_COLORMAP_RESULT;
					}
					break;
				case 2://depth map
					z << VOL_TRANSFER_FUNCTION_DEPTHMAP;
					break;
				}
			}
		}

		if (!(mip_ && colormap_proj_))
		{
			// fog
			if (fog_)
			{
				z << VOL_FOG_BODY;
			}

			//final blend
			switch (mask_)
			{
			case 0:
				if (color_mode_ == 2)
					z << VOL_RASTER_BLEND_DMAP;
				else
				{
					if (solid_)
						z << VOL_RASTER_BLEND_SOLID;
					else
						z << VOL_RASTER_BLEND;
				}
				break;
			case 1:
				if (color_mode_ == 2)
					z << VOL_RASTER_BLEND_MASK_DMAP;
				else
				{
					if (solid_)
						z << VOL_RASTER_BLEND_MASK_SOLID;
					else
						z << VOL_RASTER_BLEND_MASK;
				}
				break;
			case 2:
				if (color_mode_ == 2)
					z << VOL_RASTER_BLEND_NOMASK_DMAP;
				else
				{
					if (solid_)
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
				if (solid_)
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
		return false;
	}

	VolShaderFactory::VolShaderFactory()
		: prev_shader_(-1)
	{
	}

	VolShaderFactory::~VolShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
			delete shader_[i];
	}

	void VolShaderFactory::clear()
	{
		for (unsigned int i = 0; i<shader_.size(); i++)
			delete shader_[i];
		shader_.clear();
		prev_shader_ = -1;
	}

	ShaderProgram* VolShaderFactory::shader(
		bool poly, int channels,
		bool shading, bool fog,
		int peel, bool clip,
		bool grad, int mask, bool mip,
		int color_mode, int colormap, int colormap_proj,
		bool solid, int vertex_shader)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(
				poly, channels,
				shading, fog,
				peel, clip,
				grad, mask, mip,
				color_mode, colormap, colormap_proj,
				solid,vertex_shader))
				return shader_[prev_shader_]->program();
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(
				poly, channels,
				shading, fog,
				peel, clip,
				grad, mask, mip,
				color_mode, colormap, colormap_proj,
				solid,vertex_shader))
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		VolShader* s = new VolShader(poly, channels,
			shading, fog,
			peel, clip,
			grad, mask, mip,
			color_mode, colormap, colormap_proj,
			solid, vertex_shader);
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
