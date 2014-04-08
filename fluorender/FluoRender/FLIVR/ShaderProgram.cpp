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

#include "GL/glew.h"
#include "ShaderProgram.h"
#include "Utils.h"
#include "../compatibility.h"
#include <time.h>
#include <cstdio>
#include <iostream>
#include <cfloat>

using std::string;

namespace FLIVR
{
	bool ShaderProgram::init_ = false;
	bool ShaderProgram::supported_ = false;
	bool ShaderProgram::non_2_textures_ = false;
	int ShaderProgram::max_texture_size_ = 64;

	ShaderProgram::ShaderProgram(const string& program) :
	type_(0), id_(0), program_(program)
	{
	}

	ShaderProgram::~ShaderProgram ()
	{
		destroy();
	}

	unsigned int ShaderProgram::id()
	{
		return id_;
	}

	bool ShaderProgram::valid()
	{
		return shaders_supported() ? glIsProgram(id_)!=0 : false;
	}

	bool ShaderProgram::init()
	{
		return init_;
	}

	void ShaderProgram::init_shaders_supported()
	{
		if (!init_ && glewInit()==GLEW_OK)
		{
			//experimental
			glewExperimental = GL_TRUE;

			supported_ = GLEW_ARB_shading_language_100 && GLEW_EXT_framebuffer_object && glTexImage3D;

			//check max texture size
			GLint texSize;
			glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texSize);
			max_texture_size_ = texSize;
#ifdef _WIN32
			const GLubyte* strRenderer=0;
			if ((strRenderer=glGetString(GL_RENDERER)))
			{
				string str = (char*)strRenderer;
				if (str.find("FirePro") != string::npos)
#endif
					glPixelTransferf(GL_RED_BIAS, FLT_MIN);//for AMD FirePro cards
#ifdef _WIN32
			}
#endif

			// Check for non-power-of-two texture support.
			non_2_textures_ = GLEW_ARB_texture_non_power_of_two!=0;

			//random numbers
			srand((unsigned int)TIME(NULL));

			init_ = true;
		}
	}

	bool ShaderProgram::shaders_supported()
	{
		return supported_;
	}

	int ShaderProgram::max_texture_size()
	{
		return max_texture_size_;
	}

	bool ShaderProgram::texture_non_power_of_two()
	{
		return non_2_textures_;
	}

	bool
		ShaderProgram::create()
	{
		if (shaders_supported())
		{
			GLuint shader;
			shader = glCreateShader(type_);

			if (shader == 0)
				return true;

			// set the source code and compile the shader
			const char *source[1];
			source[0] = program_.c_str();
			glShaderSource(shader, 1, source, NULL);
			glCompileShader(shader);

			// check the compilation of the shader
			GLint shader_status[1];
			glGetShaderiv(shader, GL_COMPILE_STATUS, shader_status);
			if (shader_status[0] == GL_FALSE)
			{
				char shader_log[1000];
				GLint shader_length[1];
				glGetShaderInfoLog(shader, 1000, shader_length, shader_log);
				return true;
			}

			// create the GLSL program and attach the shader
			id_ = glCreateProgram();
			if (id_ == 0)
			{
				return true;
			}
			glAttachShader(id_, shader);
			glLinkProgram(id_);

			glUseProgram(id_);

			/* get the variable and texture locations */
			const char *tex_strings[] = {"tex0", "tex1", "tex2", "tex3",
				"tex4", "tex5", "tex6", "tex7",
				"tex8", "tex9", "tex10", "tex11",
				"tex12", "tex13", "tex14", "tex15"};
			for (int i = 0; i < MAX_SHADER_UNIFORMS; i++)
			{
				int location = glGetUniformLocation(id_, tex_strings[i]);
				if (location != -1)
				{ // able to get that link
					glUniform1i(location, i);
				}
			}

			return false;
		}

		return true;
	}

	void ShaderProgram::destroy ()
	{
		if (shaders_supported())
		{
			glDeleteProgram(id_);
			id_ = 0;
		}
	}

	void ShaderProgram::bind ()
	{
		if (shaders_supported())
		{
			// check to linking of the program
			GLint program_status[1];
			glGetProgramiv(id_, GL_LINK_STATUS, program_status);
			if (program_status[0] == GL_FALSE)
			{
				char program_log[1000];
				glGetInfoLogARB(id_, 1000, NULL, program_log);
			}

			glUseProgram(id_);
		}
	}

	void ShaderProgram::release ()
	{
		if (shaders_supported())
		{
			glUseProgram(0);
		}
	}

	void ShaderProgram::setLocalParam(int i, double x, double y, double z, double w)
	{
		if (shaders_supported())
		{
			const char *loc_strings[] = {"loc0", "loc1", "loc2", "loc3",
				"loc4", "loc5", "loc6", "loc7",
				"loc8", "loc9", "loc10", "loc11",
				"loc12", "loc13", "loc14", "loc15"};

			int location = glGetUniformLocation(id_, loc_strings[i]);
			if (location != -1)
				glUniform4f(location, float(x), float(y), float(z), float(w));
		}
	}

	void ShaderProgram::setLocalParamMatrix(int i, float* matrix4)
	{
		if (shaders_supported())
		{
			const char *loc_strings[] = {"matrix0", "matrix1", "matrix2", "matrix3",
				"matrix4", "matrix5", "matrix6", "matrix7",
				"matrix8", "matrix9", "matrix10", "matrix11",
				"matrix12", "matrix13", "matrix14", "matrix15"};

			int location = glGetUniformLocation(id_, loc_strings[i]);
			if (location != -1)
			{
				glUniformMatrix4fv(location, 1, false, matrix4);
			}
		}
	}

	VertexProgram::VertexProgram(const string& program)
		: ShaderProgram(program)
	{
		type_ = GL_VERTEX_SHADER;
	}

	VertexProgram::~VertexProgram()
	{
	}

	FragmentProgram::FragmentProgram(const string& program)
		: ShaderProgram(program)
	{
		type_ = GL_FRAGMENT_SHADER;
	}

	FragmentProgram::~FragmentProgram()
	{
	}

} // end namespace FLIVR
