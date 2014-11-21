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
#include "FLIVR/VtxShader.h"
#include "FLIVR/ShaderProgram.h"

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{

#define VTX_SHADER_CODE_GENERIC \
	      "//VERTEX SHADER\n" \
		  "#version 330" \
		  "\n" \
		  "uniform mat4 matrix0; //projection matrix\n" \
		  "uniform mat4 matrix1; //modelview matrix\n" \
		  "attribute vec3 InVertex;  //w will be set to 1.0 automatically\n" \
		  "attribute vec3 InTexCoord0;\n" \
		  "out vec3 OutTexture;\n" \
		  "out vec3 OutVertex;\n" \
		  "//-------------------\n" \
		  "void main()\n" \
		  "{\n" \
		  "   gl_Position = matrix0 * matrix1 * vec4(InVertex,1.);\n" \
		  "   OutTexture = InTexCoord0;\n" \
		  "   OutVertex = InVertex;\n" \
		  "}\n" 

	VtxShader::VtxShader(int type) : 
	type_(type),
	program_(0)
	{}

	VtxShader::~VtxShader()
	{
		delete program_;
	}

	bool
		VtxShader::create()
	{
		string s;
		if (emit(s)) return true;
		program_ = new VertexProgram(s);
		return false;
	}

	bool
		VtxShader::emit(string& s)
	{
		ostringstream z;

		switch (type_)
		{
		case VTX_SHDR_GENERIC:
			z << VTX_SHADER_CODE_GENERIC;
			break;
		}

		s = z.str();
		//std::cerr << s << std::endl;
		return false;
	}


	VtxShaderFactory::VtxShaderFactory()
		: prev_shader_(-1)
	{}

	VtxShaderFactory::~VtxShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			delete shader_[i];
		}
	}

	VertexProgram*
		VtxShaderFactory::shader(int type)
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

		VtxShader* s = new VtxShader(type);
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

