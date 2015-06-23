//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
#include <FLIVR/VolShader.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/VolShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{
#define CORE_PROFILE_VTX_SHADER 1

#define VTX_SHADER_CODE_CORE_PROFILE \
	"//VTX_SHADER_CODE_CORE_PROFILE\n" \
	"uniform mat4 matrix0; //projection matrix\n" \
	"uniform mat4 matrix1; //modelview matrix\n" \
	"layout(location = 0) in vec3 InVertex;  //w will be set to 1.0 automatically\n" \
	"layout(location = 1) in vec3 InTexture;\n" \
	"out vec3 OutVertex;\n" \
	"out vec3 OutTexture;\n" \
	"//-------------------\n" \
	"void main()\n" \
	"{\n" \
	"	gl_Position = matrix0 * matrix1 * vec4(InVertex,1.);\n" \
	"	OutTexture = InTexture;\n" \
	"	OutVertex  = InVertex;\n" \
	"}\n" 

#define VTX_SHADER_CODE_FOG \
	"//VTX_SHADER_CODE_FOG\n" \
	"uniform mat4 matrix0; //projection matrix\n" \
	"uniform mat4 matrix1; //modelview matrix\n" \
	"layout(location = 0) in vec3 InVertex;  //w will be set to 1.0 automatically\n" \
	"layout(location = 1) in vec3 InTexture;\n" \
	"out vec3 OutVertex;\n" \
	"out vec3 OutTexture;\n" \
	"out vec4 OutFogCoord;\n" \
	"//-------------------\n" \
	"void main()\n" \
	"{\n" \
	"	OutFogCoord = matrix1 * vec4(InVertex,1.);\n" \
	"	gl_Position = matrix0 * OutFogCoord;\n" \
	"	OutTexture = InTexture;\n" \
	"	OutVertex  = InVertex;\n" \
	"}\n" 

#define FRG_SHADER_CODE_CORE_PROFILE \
	"//FRG_SHADER_CODE_CORE_PROFILE\n" \
	"in vec3 OutVertex;\n" \
	"in vec3 OutTexCoord;\n" \
	"out vec4 FragColor;\n" \
	"\n" \
	"uniform vec4 loc0;//color\n" \
	"void main()\n" \
	"{\n" \
	"	FragColor = loc0;\n" \
	"}\n"

VolShader::VolShader(
	bool poly, int channels,
	bool shading, bool fog,
	int peel, bool clip,
	bool hiqual, int mask,
	int color_mode, int colormap, int colormap_proj,
	bool solid, int vertex_shader)
	: poly_(poly),
	channels_(channels),
	shading_(shading),
	fog_(fog),
	peel_(peel),
	clip_(clip),
	hiqual_(hiqual),
	mask_(mask),
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
		if (fog_)
			z << VTX_SHADER_CODE_FOG;
		else
			z << VTX_SHADER_CODE_CORE_PROFILE;

		s = z.str();
		return false;
	}

	string VolShader::get_colormap_code()
	{
		switch (colormap_)
		{
		case 0:
			return string(VOL_COLORMAP_CALC0);
		case 1:
			return string(VOL_COLORMAP_CALC1);
		case 2:
			return string(VOL_COLORMAP_CALC2);
		case 3:
			return string(VOL_COLORMAP_CALC3);
		case 4:
			return string(VOL_COLORMAP_CALC4);
		}
		return string(VOL_COLORMAP_CALC0);
	}

	string VolShader::get_colormap_proj()
	{
		switch (colormap_proj_)
		{
		case 0:
			return string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU0);
		case 1:
			return string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU1);
		case 2:
			return string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU2);
		case 3:
			return string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU3);
		}
		return string(VOL_TRANSFER_FUNCTION_COLORMAP_VALU0);
	}

	bool VolShader::emit_f(string& s)
	{
		ostringstream z;

		if (poly_)
		{
			z << ShaderProgram::glsl_version_;
			z << FRG_SHADER_CODE_CORE_PROFILE;
			//output
			s = z.str();
			return false;
		}

		//version info
		z << ShaderProgram::glsl_version_;
		z << VOL_INPUTS;
		if (fog_)
			z << VOL_INPUTS_FOG;
		z << VOL_OUTPUTS;

		//the common uniforms
		z << VOL_UNIFORMS_COMMON;

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

			if (hiqual_)
				z << VOL_GRAD_COMPUTE_HI;
			else
				z << VOL_GRAD_COMPUTE_LO;

			z << VOL_BODY_SHADING;

			if (channels_ == 1)
				z << VOL_COMPUTED_GM_LOOKUP;
			else
				z << VOL_TEXTURE_GM_LOOKUP;

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
				if (hiqual_)
					z << VOL_GRAD_COMPUTE_HI;
				else
					z << VOL_GRAD_COMPUTE_LO;

				z << VOL_COMPUTED_GM_LOOKUP;
			}
			else
			{
				z << VOL_TEXTURE_GM_LOOKUP;
			}

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
			break;
		case 4:
			z << VOL_RASTER_BLEND_LABEL_MASK;
			break;
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

	ShaderProgram* VolShaderFactory::shader(
		bool poly, int channels,
		bool shading, bool fog,
		int peel, bool clip,
		bool hiqual, int mask,
		int color_mode, int colormap, int colormap_proj,
		bool solid, int vertex_shader)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(
				poly, channels,
				shading, fog,
				peel, clip,
				hiqual, mask,
				color_mode, colormap, colormap_proj,
				solid,vertex_shader))
			{
				return shader_[prev_shader_]->program();
			}
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(
				poly, channels,
				shading, fog,
				peel, clip,
				hiqual, mask,
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
			hiqual, mask,
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

} // end namespace FLIVR
