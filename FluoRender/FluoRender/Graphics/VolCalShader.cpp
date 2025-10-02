/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <string>
#include <sstream>
#include <iostream>
#include <VolCalShader.h>
#include <ShaderProgram.h>
#include <VolShaderCode.h>
#include <VolCalShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace flvr
{
	VolCalShader::VolCalShader(int type) :
	type_(type),
	program_(0)
	{}

	VolCalShader::~VolCalShader()
	{
		delete program_;
	}

	bool VolCalShader::create()
	{
		string vs = ShaderProgram::glsl_version_ +
			ShaderProgram::glsl_unroll_ +
			CAL_VERTEX_CODE;
		string fs;
		if (emit(fs)) return true;
		program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool VolCalShader::emit(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		z << VOL_INPUTS;
		z << CAL_OUTPUTS;

		switch (type_)
		{
		case CAL_SUBSTRACTION:
		case CAL_ADDITION:
		case CAL_DIVISION:
		case CAL_INTERSECTION:
		case CAL_APPLYMASK:
			z << CAL_UNIFORMS_COMMON;
			break;
		case CAL_APPLYMASKINV:
		case CAL_APPLYMASKINV2:
			z << VOL_UNIFORMS_COMMON;
			z << VOL_UNIFORMS_MASK;
			break;
		case CAL_INTERSECTION_WITH_MASK:
			z << CAL_UNIFORMS_WITH_MASK;
			break;
		}

		z << CAL_HEAD;

		if (type_ == CAL_INTERSECTION_WITH_MASK)
			z << CAL_TEX_LOOKUP_WITH_MASK;
		else
			z << CAL_TEX_LOOKUP;

		switch (type_)
		{
		case CAL_SUBSTRACTION:
			z << CAL_BODY_SUBSTRACTION;
			break;
		case CAL_ADDITION:
			z << CAL_BODY_ADDITION;
			break;
		case CAL_DIVISION:
			z << CAL_BODY_DIVISION;
			break;
		case CAL_INTERSECTION:
			z << CAL_BODY_INTERSECTION;
			break;
		case CAL_APPLYMASK:
			z << CAL_BODY_APPLYMASK;
			break;
		case CAL_APPLYMASKINV:
		case CAL_APPLYMASKINV2:
			z << CAL_BODY_APPLYMASKINV;
			break;
		case CAL_INTERSECTION_WITH_MASK:
			z << CAL_BODY_INTERSECTION_WITH_MASK;
			break;
		}

		z << CAL_RESULT;
		z << CAL_TAIL;

		s = z.str();
		return false;
	}


	VolCalShaderFactory::VolCalShaderFactory()
		: prev_shader_(-1)
	{}

	VolCalShaderFactory::~VolCalShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
			delete shader_[i];
	}

	void VolCalShaderFactory::clear()
	{
		for (unsigned int i = 0; i<shader_.size(); i++)
			delete shader_[i];
		shader_.clear();
		prev_shader_ = -1;
	}

	ShaderProgram* VolCalShaderFactory::shader(int type)
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

		VolCalShader* s = new VolCalShader(type);
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
