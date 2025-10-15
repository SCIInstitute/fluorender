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

#include <LightFieldShader.h>
#include <LightFieldShaderCode.h>

using namespace flvr;

std::shared_ptr<ShaderProgram> LightFieldShaderFactory::shader(const ShaderParams& params)
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

bool LightFieldShaderFactory::emit_v(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	//type doesn't do anything for now
#pragma warning(push)
#pragma warning(disable : 4065)
	switch (p.type)
	{
	default:
		z << LIGHT_FIELD_SHADER_VERTEX;
		break;
	}
#pragma warning(pop)

	s = z.str();
	return true;
}

bool LightFieldShaderFactory::emit_f(const ShaderParams& p, std::string& s)
{
	std::ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	//type doesn't do anything for now
#pragma warning(push)
#pragma warning(disable : 4065)
	switch (p.type)
	{
	default:
		z << LIGHT_FIELD_SHADER_FRAG;
		break;
	}
#pragma warning(pop)

	s = z.str();
	return true;
}


