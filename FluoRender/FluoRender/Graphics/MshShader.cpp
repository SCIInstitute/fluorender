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

#include <MshShader.h>
#include <MeshShaderCode.h>

using namespace flvr;

std::shared_ptr<ShaderProgram> MeshShaderFactory::shader(const ShaderParams& params)
{
	auto it = shader_map_.find(params);
	if (it != shader_map_.end())
		return it->second;

	bool use_geom_shader = params.normal;
	std::string vs;
	bool valid_v = emit_v(params, vs);
	std::string fs;
	bool valid_f = emit_f(params, fs);
	std::string gs;
	bool valid_g = false;
	if (use_geom_shader)
		valid_g = emit_g(params, gs);

	std::shared_ptr<ShaderProgram> prog;
	if (use_geom_shader)
	{
		if (!valid_v || !valid_f || !valid_g) return nullptr;
		prog = std::make_shared<ShaderProgram>(vs, fs, gs);
	}
	else
	{
		if (!valid_v || !valid_f) return nullptr;
		prog = std::make_shared<ShaderProgram>(vs, fs);
	}

	shader_map_[params] = prog;

	return prog;
}

bool MeshShaderFactory::emit_v(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;

	//inputs
	z << MSH_VERTEX_INPUTS_V;
	if (p.type == 0)
	{
		if (p.shading)
			z << MSH_VERTEX_INPUTS_N;
		if (p.tex)
			z << MSH_VERTEX_INPUTS_T;
		if (p.color)
			z << MSH_VERTEX_INPUTS_C;
		//outputs
		if (p.normal)
			z << MSH_VERTEX_OUTPUTS_VPOS;
		if (p.shading)
			z << MSH_VERTEX_OUTPUTS_N;
		if (p.tex)
			z << MSH_VERTEX_OUTPUTS_T;
		if (p.color)
			z << MSH_VERTEX_OUTPUTS_C;
		z << MSH_VERTEX_OUTPUTS_FOG;
		if (p.shading)
			z << MSH_VERTEX_UNIFORM_MATRIX_NORMAL;
	}
	//uniforms
	z << MSH_VERTEX_UNIFORM_MATRIX;

	z << MSH_HEAD;

	//body
	if (p.normal)
		z << MSH_VERTEX_BODY_VPOS;
	z << MSH_VERTEX_BODY_POS;
	if (p.type == 0)
	{
		if (p.shading)
			z << MSH_VERTEX_BODY_NORMAL;
		if (p.tex)
			z << MSH_VERTEX_BODY_TEX;
		if (p.color)
			z << MSH_VERTEX_BODY_COLOR;
		z << MSH_VERTEX_BODY_FOG;
	}

	z << MSH_TAIL;

	s = z.str();
	return true;
}

bool MeshShaderFactory::emit_f(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;

	if (p.type == 0)
	{
		z << MSH_FRAG_OUTPUTS;
		z << MSH_FRAG_OUTPUTS_DEPTH;
		//inputs
		if (p.shading)
			z << MSH_FRAG_INPUTS_N;
		if (p.tex)
			z << MSH_FRAG_INPUTS_T;
		if (p.color)
			z << MSH_FRAG_INPUTS_C;
		z << MSH_FRAG_INPUTS_FOG;
		//uniforms
		z << MSH_FRAG_UNIFORMS_SHADING;
		if (p.tex)
			z << MSH_FRAG_UNIFORMS_TEX;
		z << MSH_FRAG_UNIFORMS_FOG;
		if (p.peel)
			z << MSH_FRAG_UNIFORMS_DP;

		z << MSH_HEAD;

		//body
		switch (p.peel)
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

		z << MSH_FRAG_HEAD_FOG;

		if (p.shading || p.tex)
			z << MSH_FRAG_BODY_SHADING_COLOR;
		else
			z << MSH_FRAG_BODY_PLAIN_COLOR;
		if (p.color)
			z << MSH_FRAG_BODY_VERTEX_COLOR;
		if (p.shading)
			z << MSH_FRAG_BODY_SHADING;
		if (p.tex)
			z << MSH_FRAG_BODY_TEXTURE;
		z << MSH_FRAG_BODY_FOG;
		if (p.fog)
			z << MSH_FRAG_BODY_FOG_MIX;
		z << MSH_FRAG_BODY_COLOR_OUT;
		z << MSH_FRAG_BODY_DEPTH_OUT;
	}
	else if (p.type == 1)
	{
		z << MSH_FRAG_OUTPUTS_INT;
		z << MSH_FRAG_UNIFORMS_INT;
		z << MSH_HEAD;
		z << MSH_FRAG_BODY_INT;
	}

	z << MSH_TAIL;

	s = z.str();

	return true;
}

bool MeshShaderFactory::emit_g(const ShaderParams& p, std::string& s)
{
	if (!p.normal)
		return false;

	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;

	z << MSH_GEOM_NORMALS_INPUTS;
	if (p.normal)
		z << MSH_GEOM_NORMALS_INPUTS_VPOS;
	if (p.tex)
		z << MSH_GEOM_NORMALS_INPUTS_T;
	if (p.color)
		z << MSH_GEOM_NORMALS_INPUTS_C;
	z << MSH_GEOM_NORMALS_INPUTS_FOG;
	z << MSH_GEOM_NORMALS_OUTPUTS_N;
	if (p.tex)
		z << MSH_GEOM_NORMALS_OUTPUTS_T;
	if (p.color)
		z << MSH_GEOM_NORMALS_OUTPUTS_C;
	z << MSH_GEOM_NORMALS_OUTPUTS_FOG;
	z << MSH_VERTEX_UNIFORM_MATRIX_NORMAL;
	z << MSH_GEOM_NORMALS_HEAD;
	if (p.tex)
		z << MSH_GEOM_NORMALS_BODY_T;
	if (p.color)
		z << MSH_GEOM_NORMALS_BODY_C;
	z << MSH_GEOM_NORMALS_BODY_FOG;
	z << MSH_GEOM_NORMALS_TAIL;

	s = z.str();

	return true;
}
