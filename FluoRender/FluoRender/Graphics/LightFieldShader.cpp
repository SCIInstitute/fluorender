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
#include <ShaderProgram.h>
#include <LightFieldShaderCode.h>
#include <sstream>
#include <iostream>

using std::string;
using std::vector;
using std::ostringstream;

using namespace flvr;

LightFieldShader::LightFieldShader(int type) : 
	type_(type),
	program_(0)
{
}

LightFieldShader::~LightFieldShader()
{
	delete program_;
}

bool LightFieldShader::create()
{
	string vs;
	if (emit_v(vs)) return true;
	string fs;
	if (emit_f(fs)) return true;
	program_ = new ShaderProgram(vs, fs);
	return false;
}

bool LightFieldShader::emit_v(string& s)
{
	ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	//type doesn't do anything for now
#pragma warning(push)
#pragma warning(disable : 4065)
	switch (type_)
	{
	default:
		z << LIGHT_FIELD_SHADER_VERTEX;
		break;
	}
#pragma warning(pop)

	s = z.str();
	return false;
}

bool LightFieldShader::emit_f(string& s)
{
	ostringstream z;

	z << ShaderProgram::glsl_version_;
	z << ShaderProgram::glsl_unroll_;
	//type doesn't do anything for now
#pragma warning(push)
#pragma warning(disable : 4065)
	switch (type_)
	{
	default:
		z << LIGHT_FIELD_SHADER_FRAG;
		break;
	}
#pragma warning(pop)

	s = z.str();
	return false;
}

LightFieldShaderFactory::LightFieldShaderFactory()
	: prev_shader_(-1)
{}

LightFieldShaderFactory::~LightFieldShaderFactory()
{
	for(unsigned int i=0; i<shader_.size(); i++)
		delete shader_[i];
}

void LightFieldShaderFactory::clear()
{
	for (unsigned int i = 0; i<shader_.size(); i++)
		delete shader_[i];
	shader_.clear();
	prev_shader_ = -1;
}

ShaderProgram* LightFieldShaderFactory::shader(int type)
{
	if(prev_shader_ >= 0)
	{
		if(shader_[prev_shader_]->match(type))
			return shader_[prev_shader_]->program();
	}
	for(unsigned int i=0; i<shader_.size(); i++)
	{
		if(shader_[i]->match(type)) 
		{
			prev_shader_ = i;
			return shader_[i]->program();
		}
	}

	LightFieldShader* s = new LightFieldShader(type);
	if(s->create())
	{
		delete s;
		return 0;
	}
	shader_.push_back(s);
	prev_shader_ = int(shader_.size())-1;
	return s->program();
}


