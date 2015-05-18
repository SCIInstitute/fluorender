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
#include <FLIVR/MshShader.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/VolShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{
#define MSH_VERTEX_INPUTS_V \
	"//MSH_VERTEX_INPUTS_V\n" \
	"layout(location = 0) in vec3 InVertex;\n"

#define MSH_VERTEX_INPUTS_N \
	"//MSH_VERTEX_INPUTS_N\n" \
	"layout(location = 1) in vec3 InNormal;\n"

#define MSH_VERTEX_INPUTS_T1 \
	"//MSH_VERTEX_INPUTS_T1\n" \
	"layout(location = 1) in vec2 InTexcoord;\n"

#define MSH_VERTEX_INPUTS_T2 \
	"//MSH_VERTEX_INPUTS_T1\n" \
	"layout(location = 2) in vec2 InTexcoord;\n"

#define MSH_VERTEX_OUTPUTS_N \
	"//MSH_VERTEX_OUTPUTS_N\n" \
	"out vec3 OutNormal;\n"

#define MSH_VERTEX_OUTPUTS_T \
	"//MSH_VERTEX_OUTPUTS_T\n" \
	"out vec2 OutTexcoord;\n"

#define MSH_VERTEX_OUTPUTS_FOG \
	"//MSH_VERTEX_OUTPUTS_FOG\n" \
	"out vec4 OutFogCoord;\n"

#define MSH_VERTEX_UNIFORM_MATRIX \
	"//MSH_VERTEX_UNIFORM_MATRIX\n" \
	"uniform mat4 matrix0;//projection\n" \
	"uniform mat4 matrix1;//model view\n"

#define MSH_VERTEX_UNIFORM_MATRIX_NORMAL \
	"//MSH_VERTEX_UNIFORM_MATRIX_NORMAL\n" \
	"uniform mat4 matrix2;//normal\n"

#define MSH_HEAD \
	"// MSH_HEAD\n" \
	"void main()\n" \
	"{\n"

#define MSH_VERTEX_BODY_POS \
	"//MSH_VERTEX_BODY_POS\n" \
	"	gl_Position = matrix0 * matrix1 * vec4(InVertex, 1.0);\n"

#define MSH_VERTEX_BODY_NORMAL \
	"//MSH_VERTEX_BODY_NORMAL\n" \
	"	OutNormal = normalize((matrix2 * vec4(InNormal, 0.0)).xyz);\n"

#define MSH_VERTEX_BODY_TEX \
	"//MSH_VERTEX_BODY_TEX\n" \
	"	OutTexcoord = InTexcoord;\n"

#define MSH_VERTEX_BODY_FOG \
	"//MSH_VERTEX_BODY_FOG\n" \
	"	OutFogCoord = matrix1 * vec4(InVertex,1.);\n"

#define MSH_FRAG_OUTPUTS \
	"//MSH_FRAG_OUTPUTS\n" \
	"out vec4 FragColor;\n" \
	"\n"

#define MSH_FRAG_OUTPUTS_INT \
	"//MSH_FRAG_OUTPUTS_INT\n" \
	"out uint FragUint;\n"\
	"\n"

#define MSH_FRAG_INPUTS_N \
	"//MSH_FRAG_INPUTS_N\n" \
	"in vec3 OutNormal;\n"

#define MSH_FRAG_INPUTS_T \
	"//MSH_FRAG_INPUTS_T\n" \
	"in vec2 OutTexcoord;\n"

#define MSH_FRAG_INPUTS_FOG \
	"//MSH_FRAG_INPUTS_FOG\n" \
	"in vec4 OutFogCoord;\n"

#define MSH_FRAG_UNIFORMS_COLOR \
	"//MSH_FRAG_UNIFORMS_COLOR\n" \
	"uniform vec4 loc0;//color\n"

#define MSH_FRAG_UNIFORMS_NOMAT \
	"//MSH_FRAG_UNIFORMS_NOMAT\n" \
	"uniform vec4 loc0;//color\n" \
	"uniform vec4 loc3;//(0, alpha, 0, 0)\n"

#define MSH_FRAG_UNIFORMS_TEX \
	"// MSH_FRAG_UNIFORMS_TEX\n" \
	"uniform sampler2D tex0;\n"

#define MSH_FRAG_UNIFORMS_MATERIAL \
	"//MSH_FRAG_UNIFORMS_MATERIAL\n" \
	"uniform vec4 loc0;//ambient color\n" \
	"uniform vec4 loc1;//diffuse color\n" \
	"uniform vec4 loc2;//specular color\n" \
	"uniform vec4 loc3;//(shine, alpha, 0, 0)\n"

#define MSH_FRAG_UNIFORMS_DP \
	"// MSH_FRAG_UNIFORMS_DP\n" \
	"uniform vec4 loc7;//(1/vx, 1/vy, 0, 0)\n" \
	"uniform sampler2D tex13;\n" \
	"uniform sampler2D tex14;\n" \
	"uniform sampler2D tex15;\n"

#define MSH_FRAG_UNIFORMS_INT \
	"//MSH_FRAG_UNIFORMS_INT\n" \
	"uniform uint loci0;//name\n"

//1: draw depth after 15 (15)
#define MSH_FRAG_BODY_DP_1 \
	"	// MSH_FRAG_BODY_DP_1\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture(tex15, t).r >= gl_FragCoord.z-1e-6) discard;\n"

//2: draw mesh after 14 (14, 15)
#define MSH_FRAG_BODY_DP_2 \
	"	// MSH_FRAG_BODY_DP_2\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture(tex14, t).r >= gl_FragCoord.z-1e-6) discard;\n" \

//3: draw mesh after 13 and before 15 (13, 14, 15)
#define MSH_FRAG_BODY_DP_3 \
	"	// MSH_FRAG_BODY_DP_3\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture(tex15, t).r <= gl_FragCoord.z+1e-6) discard;\n" \
	"	else if (texture(tex13, t).r >= gl_FragCoord.z-1e-6) discard;\n"

//4: draw mesh before 15 (at 14) (14, 15)
#define MSH_FRAG_BODY_DP_4 \
	"	// MSH_FRAG_BODY_DP_4\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture(tex15, t).r <= gl_FragCoord.z+1e-6) discard;\n" \

//5: draw mesh at 15 (15)
#define MSH_FRAG_BODY_DP_5 \
	"	// MSH_FRAG_BODY_DP_5\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture(tex15, t).r <= gl_FragCoord.z-1e-6) discard;\n" \

#define MSH_FRAG_BODY_COLOR \
	"	//MSH_FRAG_BODY_COLOR\n" \
	"	vec4 c = vec4(1.0);\n"

#define MSH_FRAG_BODY_COLOR_OUT \
	"	// MSH_FRAG_BODY_COLOR_OUT\n" \
	"	FragColor = vec4(c.xyz, c.w*loc3.y);\n"

#define MSH_FRAG_BODY_SIMPLE \
	"	//MSH_FRAG_BODY_SIMPLE\n" \
	"	c = loc0;\n"

#define MSH_FRAG_BODY_COLOR_LIGHT \
	"	//MSH_FRAG_BODY_COLOR_LIGHT\n" \
	"	vec4 spec = vec4(0.0);\n" \
	"	vec3 eye = vec3(0.0, 0.0, 1.0);\n" \
	"	vec3 l_dir = vec3(0.0, 0.0, 1.0);\n" \
	"	vec3 n = normalize(OutNormal);\n" \
	"	float intensity = abs(dot(n, l_dir));\n" \
	"	if (intensity > 0.0)\n" \
	"	{\n" \
	"		vec3 h = normalize(l_dir+eye);\n" \
	"		float intSpec = max(dot(h, n), 0.0);\n" \
	"		spec = loc2 * pow(intSpec, loc3.x);\n" \
	"	}\n" \
	"	c.xyz = max(intensity * loc1 + spec, loc0).xyz;\n"

#define MSH_FRAG_BODY_TEXTURE \
	"	//MSH_FRAG_BODY_TEXTURE\n" \
	"	c = c * texture(tex0, OutTexcoord);\n"

#define MSH_FRAG_BODY_FOG_V \
	"	// MSH_FRAG_BODY_FOG\n" \
	"	vec4 v;\n"

#define MSH_FRAG_BODY_INT \
	"	//MSH_FRAG_BODY_INT\n" \
	"	FragUint = loci0;\n"

#define MSH_TAIL \
	"}\n"

	MshShader::MshShader(int type,
		int peel, bool tex,
		bool fog, bool light)
		: type_(type),
		peel_(peel),
		tex_(tex),
		fog_(fog),
		light_(light),
		program_(0)
	{}

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
		program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool MshShader::emit_v(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;

		//inputs
		z << MSH_VERTEX_INPUTS_V;
		if (type_ == 0)
		{
			if (light_)
				z << MSH_VERTEX_INPUTS_N;
			if (tex_)
				z << MSH_VERTEX_INPUTS_T2;
			//outputs
			if (light_)
				z << MSH_VERTEX_OUTPUTS_N;
			if (tex_)
				z << MSH_VERTEX_OUTPUTS_T;
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
		z << MSH_VERTEX_BODY_POS;
		if (type_ == 0)
		{
			if (light_)
				z << MSH_VERTEX_BODY_NORMAL;
			if (tex_)
				z << MSH_VERTEX_BODY_TEX;
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

		if (type_ == 0)
		{
			z << MSH_FRAG_OUTPUTS;
			//inputs
			if (light_)
				z << MSH_FRAG_INPUTS_N;
			if (tex_)
				z << MSH_FRAG_INPUTS_T;
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
			if (light_)
				z << MSH_FRAG_BODY_COLOR_LIGHT;
			if (tex_)
				z << MSH_FRAG_BODY_TEXTURE;
			if (!light_ && !tex_)
				z << MSH_FRAG_BODY_SIMPLE;
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
		bool fog, bool light)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(type, peel, tex, fog, light))
			{
				return shader_[prev_shader_]->program();
			}
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(type, peel, tex, fog, light)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		MshShader* s = new MshShader(type, peel, tex, fog, light);
		if(s->create())
		{
			delete s;
			return 0;
		}
		shader_.push_back(s);
		prev_shader_ = (int)shader_.size()-1;
		return s->program();
	}

} // end namespace FLIVR

