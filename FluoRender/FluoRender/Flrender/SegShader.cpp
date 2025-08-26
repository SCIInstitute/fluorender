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
#include <SegShader.h>
#include <ShaderProgram.h>
#include <VolShaderCode.h>
#include <SegShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
	SegShader::SegShader(int type, int paint_mode, int hr_mode,
		bool use_2d, bool shading, int peel,
		bool clip, bool use_dir) :
	type_(type),
	paint_mode_(paint_mode),
	hr_mode_(hr_mode),
	use_2d_(use_2d),
	shading_(shading),
	peel_(peel),
	clip_(clip),
	use_dir_(use_dir),
	program_(0)
	{}

	SegShader::~SegShader()
	{
		delete program_;
	}

	bool SegShader::create()
	{
		string vs = ShaderProgram::glsl_version_ +
			ShaderProgram::glsl_unroll_ +
			SEG_VERTEX_CODE;
		string fs;
		if (emit(fs)) return true;
		program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool SegShader::emit(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		z << VOL_INPUTS;

		//uniforms
		z << SEG_OUTPUTS;
		z << VOL_UNIFORMS_COMMON;
		z << VOL_UNIFORMS_SIN_COLOR;
		//for paint_mode==9, loc6 = (px, py, view_nx, view_ny)
		z << VOL_UNIFORMS_MASK;
		z << SEG_UNIFORMS_WMAP_2D;
		z << SEG_UNIFORMS_MASK_2D;
		z << SEG_UNIFORMS_MATRICES;
		z << VOL_UNIFORMS_MATRICES;
		z << SEG_UNIFORMS_PARAMS;

		//uniforms for clipping
		if (paint_mode_!=6 && clip_)
			z << VOL_UNIFORMS_CLIP;

		//for hidden removal
		if (type_==SEG_SHDR_INITIALIZE &&
			(paint_mode_==1 || paint_mode_==2 ||
			paint_mode_==9) &&
			hr_mode_>0)
		{
			z << SEG_UNIFORM_MATRICES_INVERSE;
			z << VOL_GRAD_COMPUTE_FUNC;
			z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC;
		}

		if (paint_mode_!=6 && clip_)
			z << VOL_CLIP_FUNC;

		//the common head
		z << VOL_HEAD;

		//head for clipping planes
		if (paint_mode_!=6 && clip_)
			z << VOL_HEAD_CLIP_FUNC;

		if (paint_mode_ == 6)
		{
			z << SEG_BODY_INIT_CLEAR;
		}
		else
		{
			//bodies
			switch (type_)
			{
			case SEG_SHDR_INITIALIZE:
				z << SEG_BODY_INIT_2D_COORD;
				if (paint_mode_ == 1 ||
					paint_mode_ == 2 ||
					paint_mode_ == 4 ||
					paint_mode_ == 8)
					z << SEG_BODY_INIT_CULL;
				else if (paint_mode_ == 3)
					z << SEG_BODY_INIT_CULL_ERASE;
				else if (paint_mode_ == 9)
					z << SEG_BODY_INIT_CULL_POINT;

				if (paint_mode_ != 3)
				{
					z << VOL_HEAD_LIT;
					z << VOL_DATA_VOLUME_LOOKUP;
					z << VOL_GRAD_COMPUTE;
					z << VOL_COMPUTED_GM_LOOKUP;
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;

					if (use_2d_)
					{
						z << SEG_BODY_WEIGHT;
						z << SEG_BODY_BLEND_WEIGHT;
					}
				}

				if (paint_mode_==1 ||
					paint_mode_==2 ||
					paint_mode_==9)
				{
					if (hr_mode_ == 0)
						z << SEG_BODY_INIT_BLEND_APPEND;
					else if (hr_mode_ == 1)//ortho
						z << SEG_BODY_INIT_BLEND_HR_ORTHO;
					else if (hr_mode_ == 2)//persp
						z << SEG_BODY_INIT_BLEND_HR_PERSP;
				}
				else if (paint_mode_==3)
					z << SEG_BODY_INIT_BLEND_ERASE;
				else if (paint_mode_==4)
					z << SEG_BODY_INIT_BLEND_DIFFUSE;
				else if (paint_mode_==5)
					z << SEG_BODY_INIT_BLEND_FLOOD;
				else if (paint_mode_==7)
					z << SEG_BODY_INIT_BLEND_ALL;
				else if (paint_mode_==8)
					z << SEG_BODY_INIT_BLEND_ALL;
				break;
			case SEG_SHDR_DB_GROW:
				z << SEG_BODY_DB_GROW_2D_COORD;

				if (paint_mode_!=5 &&
					paint_mode_!=9)
					z << SEG_BODY_DB_GROW_CULL;

				if (paint_mode_ != 3)
				{
					z << VOL_HEAD_LIT;
					z << VOL_DATA_VOLUME_LOOKUP;
					z << VOL_GRAD_COMPUTE;
					z << VOL_COMPUTED_GM_LOOKUP;
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;

					if (use_2d_)
					{
						z << SEG_BODY_WEIGHT;
						z << SEG_BODY_BLEND_WEIGHT;
					}

					z << SEG_BODY_DB_GROW_STOP_FUNC;
				}

				if (paint_mode_ == 1 ||
					paint_mode_ == 2 ||
					paint_mode_ == 4 ||
					paint_mode_ == 5 ||
					paint_mode_ == 9)
				{
					z << SEG_BODY_DB_GROW_BLEND_APPEND_HEAD;
					if (use_dir_)
						z << SEG_BODY_DB_GROW_BLEND_APPEND_DIR;
					z << SEG_BODY_DB_GROW_BLEND_APPEND_BODY;
				}
				else if (paint_mode_==3)
					z << SEG_BODY_DB_GROW_BLEND_ERASE;
				break;
			}
		}

		//tail
		z << SEG_TAIL;

		s = z.str();
		return false;
	}

	SegShaderFactory::SegShaderFactory()
		: prev_shader_(-1)
	{}

	SegShaderFactory::~SegShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
			delete shader_[i];
	}

	void SegShaderFactory::clear()
	{
		for (unsigned int i = 0; i<shader_.size(); i++)
			delete shader_[i];
		shader_.clear();
		prev_shader_ = -1;
	}

	ShaderProgram* SegShaderFactory::shader(
		int type, int paint_mode, int hr_mode,
		bool use_2d, bool shading, int peel,
		bool clip, bool use_dir)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(
				type, paint_mode, hr_mode,
				use_2d, shading, peel,
				clip, use_dir)) 
				return shader_[prev_shader_]->program();
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(
				type, paint_mode, hr_mode,
				use_2d, shading, peel,
				clip, use_dir)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		SegShader* s = new SegShader(
			type, paint_mode, hr_mode,
			use_2d, shading, peel,
			clip, use_dir);
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

