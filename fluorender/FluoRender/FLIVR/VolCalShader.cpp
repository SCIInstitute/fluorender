/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
#include <FLIVR/VolCalShader.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/VolShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{
#define CAL_OUTPUTS \
	"//CAL_OUTPUTS\n" \
	"out vec4 FragColor;\n" \
	"\n"

#define CAL_VERTEX_CODE \
	"//CAL_VERTEX_CODE\n" \
	"layout(location = 0) in vec3 InVertex;\n" \
	"layout(location = 1) in vec3 InTexture;\n" \
	"out vec3 OutVertex;\n" \
	"out vec3 OutTexture;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	gl_Position = vec4(InVertex,1.);\n" \
	"	OutTexture = InTexture;\n" \
	"	OutVertex  = InVertex;\n" \
	"}\n" 

#define CAL_UNIFORMS_COMMON \
	"//CAL_UNIFORMS_COMMON\n" \
	"uniform sampler3D tex1;//operand A\n" \
	"uniform sampler3D tex2;//operand B\n" \
	"uniform vec4 loc0;//(scale_A, scale_B, 0.0, 0.0)\n" \
	"\n"

#define CAL_UNIFORMS_WITH_MASK \
	"//CAL_UNIFORMS_WITH_MASK\n" \
	"uniform sampler3D tex1;//operand A\n" \
	"uniform sampler3D tex3;//mask of A\n" \
	"uniform sampler3D tex2;//operand B\n" \
	"uniform sampler3D tex4;//mask of B\n" \
	"uniform vec4 loc0;//(scale_a, scale_b, use_mask_a, use_mask_b)\n" \
	"\n"

#define CAL_HEAD \
	"//CAL_HEAD\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = vec4(OutTexture, 1.0);//position in the result volume\n" \
	"	vec4 t1 = t;//position in the operand A\n" \
	"	vec4 t2 = t;//position in the operand B\n" \
	"\n"

#define CAL_TEX_LOOKUP \
	"	//CAL_TEX_LOOKUP\n" \
	"	vec4 c1 = texture(tex1, t1.stp)*loc0.x;\n" \
	"	vec4 c2 = texture(tex2, t2.stp)*loc0.y;\n" \
	"\n"

#define CAL_TEX_LOOKUP_WITH_MASK \
	"	//CAL_TEX_LOOKUP_WITH_MASK\n" \
	"	vec4 c1 = texture3D(tex1, t1.stp)*loc0.x;\n" \
	"	vec4 m1 = loc0.z>0.0?texture(tex3, t1.stp):vec4(1.0);\n" \
	"	vec4 c2 = texture(tex2, t2.stp)*loc0.y;\n" \
	"	vec4 m2 = loc0.w>0.0?texture(tex4, t2.stp):vec4(1.0);\n" \
	"\n"

#define CAL_BODY_SUBSTRACTION \
	"	//CAL_BODY_SUBSTRACTION\n" \
	"	vec4 c = vec4(clamp(c1.x-c2.x, 0.0, 1.0));\n" \
	"\n"

#define CAL_BODY_ADDITION \
	"	//CAL_BODY_ADDITION\n" \
	"	vec4 c = vec4(clamp(c1.x+c2.x, 0.0, 1.0));\n" \
	"\n"

#define CAL_BODY_DIVISION \
	"	//CAL_BODY_DIVISION\n" \
	"	vec4 c = vec4(0.0);\n" \
	"	if (c1.x>1e-5 && c2.x>1e-5)\n" \
	"		c = vec4(clamp(c1.x/c2.x, 0.0, 1.0));\n" \
	"\n"

#define CAL_BODY_INTERSECTION \
	"	//CAL_BODY_INTERSECTION\n" \
	"	vec4 c = vec4(min(c1.x, c2.x));\n" \
	"\n"

#define CAL_BODY_INTERSECTION_WITH_MASK \
	"	//CAL_BODY_INTERSECTION_WITH_MASK\n" \
	"	vec4 c = vec4(min(c1.x*m1.x, c2.x*m2.x));\n" \
	"\n"

#define CAL_BODY_APPLYMASK \
	"	//CAL_BODY_APPLYMASK\n" \
	"	vec4 c = vec4((loc0.z<0.0?(1.0-c1.x):c1.x)*c2.x);\n" \
	"\n"

#define CAL_BODY_APPLYMASKINV \
	"	//CAL_BODY_APPLYMASKINV\n" \
	"	vec4 c = vec4(c1.x*(1.0-c2.x));\n" \
	"\n"

#define CAL_RESULT \
	"	//CAL_RESULT\n" \
	"	FragColor = c;\n" \
	"\n"

#define CAL_TAIL \
	"//CAL_TAIL\n" \
	"}\n" \
	"\n"

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
		string vs = ShaderProgram::glsl_version_ + CAL_VERTEX_CODE;
		string fs;
		if (emit(fs)) return true;
		program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool VolCalShader::emit(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
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
		{
			delete shader_[i];
		}
	}

	ShaderProgram* VolCalShaderFactory::shader(int type)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(type)) 
			{
				return shader_[prev_shader_]->program();
			}
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

} // end namespace FLIVR
