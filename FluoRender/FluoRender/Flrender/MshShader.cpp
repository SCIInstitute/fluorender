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
#include <MshShader.h>
#include <ShaderProgram.h>
#include <VolShaderCode.h>
#include <MeshShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
	MshShader::MshShader(int type,
		int peel, bool tex,
		bool fog, bool light,
		bool normal, bool color)
		: type_(type),
		peel_(peel),
		tex_(tex),
		fog_(fog),
		light_(light),
		normal_(normal),
		color_(color),
		program_(0)
	{
	}

	MshShader::~MshShader()
	{
		delete program_;
	}

	bool MshShader::create()
	{
		string vs;
		if (emit_v(vs)) return true;
		string fs;
		if (emit_f(fs)) return true;
		string gs;
		if (normal_)
		{
			if (emit_g(gs)) return true;
			program_ = new ShaderProgram(vs, fs, gs);
		}
		else
			program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool MshShader::emit_v(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;

		//inputs
		z << MSH_VERTEX_INPUTS_V;
		if (type_ == 0)
		{
			if (light_)
				z << MSH_VERTEX_INPUTS_N;
			if (tex_)
				z << MSH_VERTEX_INPUTS_T;
			if (color_)
				z << MSH_VERTEX_INPUTS_C;
			//outputs
			if (normal_)
				z << MSH_VERTEX_OUTPUTS_VPOS;
			if (light_)
				z << MSH_VERTEX_OUTPUTS_N;
			if (tex_)
				z << MSH_VERTEX_OUTPUTS_T;
			if (color_)
				z << MSH_VERTEX_OUTPUTS_C;
			if (fog_)
				z << MSH_VERTEX_OUTPUTS_FOG;
			//uniforms
			z << MSH_VERTEX_UNIFORM_MATRIX;
			if (light_)
				z << MSH_VERTEX_UNIFORM_MATRIX_NORMAL;
		}
		else if (type_ == 1)
			z << MSH_VERTEX_UNIFORM_MATRIX;


		z << MSH_HEAD;

		//body
		if (normal_)
			z << MSH_VERTEX_BODY_VPOS;
		z << MSH_VERTEX_BODY_POS;
		if (type_ == 0)
		{
			if (light_)
				z << MSH_VERTEX_BODY_NORMAL;
			if (tex_)
				z << MSH_VERTEX_BODY_TEX;
			if (color_)
				z << MSH_VERTEX_BODY_COLOR;
			if (fog_)
				z << MSH_VERTEX_BODY_FOG;
		}

		z << MSH_TAIL;

		s = z.str();
		return false;
	}

	bool MshShader::emit_f(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;

		if (type_ == 0)
		{
			z << MSH_FRAG_OUTPUTS;
			//inputs
			if (light_)
				z << MSH_FRAG_INPUTS_N;
			if (tex_)
				z << MSH_FRAG_INPUTS_T;
			if (color_)
				z << MSH_FRAG_INPUTS_C;
			if (fog_)
				z << MSH_FRAG_INPUTS_FOG;
			//uniforms
			if (light_)
				z << MSH_FRAG_UNIFORMS_MATERIAL;
			else
				z << MSH_FRAG_UNIFORMS_NOMAT;
			if (tex_)
				z << MSH_FRAG_UNIFORMS_TEX;
			if (fog_)
				z << VOL_UNIFORMS_FOG_LOC;
			if (peel_)
				z << MSH_FRAG_UNIFORMS_DP;

			z << MSH_HEAD;

			//body
			switch (peel_)
			{
			case 1:
				z << MSH_FRAG_BODY_DP_1;
				break;
			case 2:
				z << MSH_FRAG_BODY_DP_2;
				break;
			case 3:
				z << MSH_FRAG_BODY_DP_3;
				break;
			case 4:
				z << MSH_FRAG_BODY_DP_4;
				break;
			case 5:
				z << MSH_FRAG_BODY_DP_5;
				break;
			}

			if (fog_)
				z << VOL_HEAD_FOG;

			z << MSH_FRAG_BODY_COLOR;
			if (color_)
				z << MSH_FRAG_BODY_VERTEX_COLOR;
			if (light_)
				z << MSH_FRAG_BODY_MATL_LIGHT;
			if (tex_)
				z << MSH_FRAG_BODY_TEXTURE;
			//if (!light_ && !tex_)
			//	z << MSH_FRAG_BODY_SIMPLE;
			if (fog_)
			{
				z << MSH_FRAG_BODY_FOG_V;
				z << VOL_FOG_BODY;
			}
			z << MSH_FRAG_BODY_COLOR_OUT;
		}
		else if (type_ == 1)
		{
			z << MSH_FRAG_OUTPUTS_INT;
			z << MSH_FRAG_UNIFORMS_INT;
			z << MSH_HEAD;
			z << MSH_FRAG_BODY_INT;
		}

		z << MSH_TAIL;

		s = z.str();

		return false;
	}

	bool MshShader::emit_g(string& s)
	{
		if (!normal_)
			return true;

		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;

		z << MSH_GEOM_NORMALS_INPUTS;
		if (normal_)
			z << MSH_GEOM_NORMALS_INPUTS_VPOS;
		if (tex_)
			z << MSH_GEOM_NORMALS_INPUTS_T;
		if (color_)
			z << MSH_GEOM_NORMALS_INPUTS_C;
		if (fog_)
			z << MSH_GEOM_NORMALS_INPUTS_FOG;
		z << MSH_GEOM_NORMALS_OUTPUTS_N;
		if (tex_)
			z << MSH_GEOM_NORMALS_OUTPUTS_T;
		if (color_)
			z << MSH_GEOM_NORMALS_OUTPUTS_C;
		if (fog_)
			z << MSH_GEOM_NORMALS_OUTPUTS_FOG;
		z << MSH_VERTEX_UNIFORM_MATRIX_NORMAL;
		z << MSH_GEOM_NORMALS_HEAD;
		if (tex_)
			z << MSH_GEOM_NORMALS_BODY_T;
		if (color_)
			z << MSH_GEOM_NORMALS_BODY_C;
		if (fog_)
			z << MSH_GEOM_NORMALS_BODY_FOG;
		z << MSH_GEOM_NORMALS_TAIL;

		s = z.str();

		return false;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MshShaderFactory::MshShaderFactory()
		: prev_shader_(-1)
	{}

	MshShaderFactory::~MshShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
			delete shader_[i];
	}

	ShaderProgram* MshShaderFactory::shader(int type,
		int peel, bool tex,
		bool fog, bool light,
		bool normal, bool color)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(type, peel, tex, fog, light, normal, color))
			{
				return shader_[prev_shader_]->program();
			}
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(type, peel, tex, fog, light, normal, color))
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		MshShader* s = new MshShader(type, peel, tex, fog, light, normal, color);
		if(s->create())
		{
			delete s;
			return 0;
		}
		shader_.push_back(s);
		prev_shader_ = (int)shader_.size()-1;
		return s->program();
	}

} // end namespace flvr

