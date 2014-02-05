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

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{

#define MSH_UNIFORMS \
	"// MSH_UNIFORMS\n" \
	"uniform sampler2D tex0;\n" \
	"\n"

#define MSH_UNIFORMS_DP \
	"// MSH_UNIFORMS_DP\n" \
	"uniform vec4 loc7;//(1/vx, 1/vy, 0, 0)\n" \
	"uniform sampler2D tex0;\n" \
	"uniform sampler2D tex13;\n" \
	"uniform sampler2D tex14;\n" \
	"uniform sampler2D tex15;\n" \
	"\n"

#define MSH_HEAD \
	"// MSH_HEAD\n" \
	"void main()\n" \
	"{\n"

//1: draw depth after 15 (15)
#define MSH_BODY_DP_1 \
	"	// MSH_BODY_DP_1\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture2D(tex15, t).r >= gl_FragCoord.z-1e-6) discard;\n"

//2: draw mesh after 14 (14, 15)
#define MSH_BODY_DP_2 \
	"	// MSH_BODY_DP_2\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture2D(tex14, t).r >= gl_FragCoord.z-1e-6) discard;\n" \

//3: draw mesh after 13 and before 15 (13, 14, 15)
#define MSH_BODY_DP_3 \
	"	// MSH_BODY_DP_3\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture2D(tex15, t).r <= gl_FragCoord.z+1e-6) discard;\n" \
	"	else if (texture2D(tex13, t).r >= gl_FragCoord.z-1e-6) discard;\n"

//4: draw mesh before 15 (at 14) (14, 15)
#define MSH_BODY_DP_4 \
	"	// MSH_BODY_DP_4\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture2D(tex15, t).r <= gl_FragCoord.z+1e-6) discard;\n" \

//5: draw mesh at 15 (15)
#define MSH_BODY_DP_5 \
	"	// MSH_BODY_DP_5\n" \
	"	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"	if (texture2D(tex15, t).r <= gl_FragCoord.z-1e-6) discard;\n" \

#define MSH_BODY_BLEND \
	"	// MSH_BODY_BLEND\n" \
	"	gl_FragColor = gl_Color;\n"

#define MSH_BODY_BLEND_TEXTURE \
	"	// MSH_BODY_BLEND\n" \
	"	gl_FragColor = gl_Color*texture2D(tex0, gl_TexCoord[0].xy);\n"

#define MSH_FOG_BODY \
	"	// MSH_FOG_BODY\n" \
	"	vec4 fc = gl_Fog.color;	//FOG_HEAD\n" \
	"	//vec4 fp;\n" \
	"	//fp.x = gl_Fog.density;\n" \
	"	//fp.y = gl_Fog.start;\n" \
	"	//fp.z = gl_Fog.end;\n" \
	"	//fp.w = gl_Fog.scale;\n" \
	"	//float tf = -(transpose(gl_ProjectionMatrixInverseTranspose)*gl_FragCoord).z;\n" \
	"	//float tf = gl_FragCoord.z;\n" \
	"	vec4 fctmp;\n" \
	"	//vec4 v;\n" \
	"	vec4 c;\n" \
	"	c = gl_Color;\n" \
	"	//v.x = (fp.z - tf)*1e2; // VOL_FOG_BODY\n" \
	"	//v.x = (1.0 - clamp(v.x * fp.w, 0.0, 1.0))*fp.x;\n" \
	"	fctmp = c.w * fc;\n" \
	"	c.xyz = mix(fctmp.xyzz, c.xyzz, 1.0-gl_Fog.density).xyz;\n"

#define MSH_FOG_BODY_BLEND \
	"	// MSH_FOG_BODY_BLEND\n" \
	"	gl_FragColor = c;\n"

#define MSH_FOG_BODY_BLEND_TEXTURE \
	"	// MSH_FOG_BODY_BLEND\n" \
	"	gl_FragColor = c*texture2D(tex0, gl_TexCoord[0].xy);\n"

#define MSH_TAIL \
	"}\n"

	MshShader::MshShader(bool fog, int peel, bool tex)
		: fog_(fog),
		peel_(peel),
		tex_(tex),
		program_(0)
	{}

	MshShader::~MshShader()
	{
		delete program_;
	}

	bool
		MshShader::create()
	{
		string s;
		if (emit(s)) return true;
		program_ = new FragmentProgram(s);
		return false;
	}

	bool
		MshShader::emit(string& s)
	{
		ostringstream z;

		if (peel_ == 1)
		{
			z << MSH_UNIFORMS_DP;
			z << MSH_HEAD;
			z << MSH_BODY_DP_1;
		}
		else if (peel_ == 2)
		{
			z << MSH_UNIFORMS_DP;
			z << MSH_HEAD;
			z << MSH_BODY_DP_2;
		}
		else if (peel_ == 3)
		{
			z << MSH_UNIFORMS_DP;
			z << MSH_HEAD;
			z << MSH_BODY_DP_3;
		}
		else if (peel_ == 4)
		{
			z << MSH_UNIFORMS_DP;
			z << MSH_HEAD;
			z << MSH_BODY_DP_4;
		}
		else if (peel_ == 5)
		{
			z << MSH_UNIFORMS_DP;
			z << MSH_HEAD;
			z << MSH_BODY_DP_5;
		}
		else //peel_ == 0
		{
			z << MSH_UNIFORMS;
			z << MSH_HEAD;
		}

		if (fog_)
			z << MSH_FOG_BODY;

		if (fog_)
		{
			if (tex_)
				z << MSH_FOG_BODY_BLEND_TEXTURE;
			else
				z << MSH_FOG_BODY_BLEND;
		}
		else
		{
			if (tex_)
				z << MSH_BODY_BLEND_TEXTURE;
			else
				z << MSH_BODY_BLEND;
		}
		z<< MSH_TAIL;
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

	FragmentProgram* MshShaderFactory::shader(bool fog, int peel, bool tex)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(fog, peel, tex)) 
			{
				return shader_[prev_shader_]->program();
			}
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(fog, peel, tex)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		MshShader* s = new MshShader(fog, peel, tex);
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

