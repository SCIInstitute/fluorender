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

#include <SegShader.h>
#include <VolShaderCode.h>
#include <SegShaderCode.h>

using namespace flvr;

std::shared_ptr<ShaderProgram> SegShaderFactory::shader(const ShaderParams& params)
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

bool SegShaderFactory::emit_v(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	z << SEG_VERTEX_CODE;

	s = z.str();
	return true;
}

bool SegShaderFactory::emit_f(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	z << VOL_INPUTS;

	//uniforms
	z << SEG_OUTPUTS;
	z << VOL_UNIFORMS_COMMON;
	//for paint_mode==9, loc6 = (px, py, view_nx, view_ny)
	z << VOL_UNIFORMS_MASK;
	z << SEG_UNIFORMS_WMAP_2D;
	z << SEG_UNIFORMS_MASK_2D;
	z << SEG_UNIFORMS_MATRICES;
	z << SEG_UNIFORMS_PARAMS;

	//uniforms for clipping
	if (p.paint_mode != 6 && p.clip)
		z << VOL_UNIFORMS_CLIP;

	//for hidden removal
	if (p.type == SEG_SHDR_INITIALIZE &&
		(p.paint_mode == 1 || p.paint_mode == 2 ||
			p.paint_mode == 9) &&
		p.hr_mode > 0)
	{
		z << SEG_UNIFORM_MATRICES_INVERSE;
		z << VOL_GRAD_COMPUTE_FUNC;
		z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC;
	}

	if (p.paint_mode != 6 && p.clip)
		z << VOL_CLIP_FUNC;

	//the common head
	z << VOL_HEAD;

	//head for clipping planes
	if (p.paint_mode != 6 && p.clip)
		z << VOL_HEAD_CLIP_FUNC;

	if (p.paint_mode == 6)
	{
		z << SEG_BODY_INIT_CLEAR;
	}
	else
	{
		//bodies
		switch (p.type)
		{
		case SEG_SHDR_INITIALIZE:
			z << SEG_BODY_INIT_2D_COORD;
			if (p.paint_mode == 1 ||
				p.paint_mode == 2 ||
				p.paint_mode == 4 ||
				p.paint_mode == 8)
				z << SEG_BODY_INIT_CULL;
			else if (p.paint_mode == 3)
				z << SEG_BODY_INIT_CULL_ERASE;
			else if (p.paint_mode == 9)
				z << SEG_BODY_INIT_CULL_POINT;

			if (p.paint_mode != 3)
			{
				z << VOL_HEAD_LIT;
				z << VOL_DATA_VOLUME_LOOKUP;
				z << VOL_GRAD_COMPUTE;
				z << VOL_COMPUTED_GM_LOOKUP;
				z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;

				if (p.use_2d)
				{
					z << SEG_BODY_WEIGHT;
					z << SEG_BODY_BLEND_WEIGHT;
				}
			}

			if (p.paint_mode == 1 ||
				p.paint_mode == 2 ||
				p.paint_mode == 9)
			{
				if (p.hr_mode == 0)
					z << SEG_BODY_INIT_BLEND_APPEND;
				else if (p.hr_mode == 1)//ortho
					z << SEG_BODY_INIT_BLEND_HR_ORTHO;
				else if (p.hr_mode == 2)//persp
					z << SEG_BODY_INIT_BLEND_HR_PERSP;
			}
			else if (p.paint_mode == 3)
				z << SEG_BODY_INIT_BLEND_ERASE;
			else if (p.paint_mode == 4)
				z << SEG_BODY_INIT_BLEND_DIFFUSE;
			else if (p.paint_mode == 5)
				z << SEG_BODY_INIT_BLEND_FLOOD;
			else if (p.paint_mode == 7)
				z << SEG_BODY_INIT_BLEND_ALL;
			else if (p.paint_mode == 8)
				z << SEG_BODY_INIT_BLEND_ALL;
			break;
		case SEG_SHDR_DB_GROW:
			z << SEG_BODY_DB_GROW_2D_COORD;

			if (p.paint_mode != 5 &&
				p.paint_mode != 9)
				z << SEG_BODY_DB_GROW_CULL;

			if (p.paint_mode != 3)
			{
				z << VOL_HEAD_LIT;
				z << VOL_DATA_VOLUME_LOOKUP;
				z << VOL_GRAD_COMPUTE;
				z << VOL_COMPUTED_GM_LOOKUP;
				z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;

				if (p.use_2d)
				{
					z << SEG_BODY_WEIGHT;
					z << SEG_BODY_BLEND_WEIGHT;
				}

				z << SEG_BODY_DB_GROW_STOP_FUNC;
			}

			if (p.paint_mode == 1 ||
				p.paint_mode == 2 ||
				p.paint_mode == 4 ||
				p.paint_mode == 5 ||
				p.paint_mode == 9)
			{
				z << SEG_BODY_DB_GROW_BLEND_APPEND_HEAD;
				if (p.use_dir)
					z << SEG_BODY_DB_GROW_BLEND_APPEND_DIR;
				z << SEG_BODY_DB_GROW_BLEND_APPEND_BODY;
			}
			else if (p.paint_mode == 3)
				z << SEG_BODY_DB_GROW_BLEND_ERASE;
			break;
		}
	}

	//tail
	z << SEG_TAIL;

	s = z.str();
	return true;
}
