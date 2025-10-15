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

#include <GL/glew.h>
#include <ShaderProgram.h>
#include <Utils.h>
#include <compatibility.h>
#include <Debug.h>
#include <ctime>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <cfloat>

using std::string;
using std::ostringstream;
using namespace flvr;

bool ShaderProgram::init_ = false;
bool ShaderProgram::supported_ = false;
bool ShaderProgram::non_2_textures_ = false;
int ShaderProgram::max_texture_size_ = 1000;
int ShaderProgram::v_major_ = 4;
int ShaderProgram::v_minor_ = 0;
string ShaderProgram::glsl_version_;
string ShaderProgram::glsl_unroll_;
bool ShaderProgram::no_tex_unpack_ = false;

ShaderProgram::ShaderProgram(const string& frag_shader) :
	id_(0),
	vert_shader_(""),
	frag_shader_(frag_shader),
	valid_(false),
	use_geom_shader_(false)
{
	for (int i = 0; i < MAX_SHADER_UNIFORMS; ++i)
	{
		loc_ui[i] = -1;
		loc_vec4[i] = -1;
		loc_mat4[i] = -1;
		loc_int4[i] = -1;
	}
}
ShaderProgram::ShaderProgram(const string& vert_shader,
	const string& frag_shader) :
	id_(0),
	vert_shader_(vert_shader),
	frag_shader_(frag_shader),
	valid_(false),
	use_geom_shader_(false)
{
	for (int i = 0; i < MAX_SHADER_UNIFORMS; ++i)
	{
		loc_ui[i] = -1;
		loc_vec4[i] = -1;
		loc_mat4[i] = -1;
		loc_int4[i] = -1;
	}
}
ShaderProgram::ShaderProgram(const string& vert_shader,
	const string& frag_shader,
	const string& geom_shader) :
	id_(0),
	vert_shader_(vert_shader),
	frag_shader_(frag_shader),
	geom_shader_(geom_shader),
	valid_(false),
	use_geom_shader_(true)
{
	for (int i = 0; i < MAX_SHADER_UNIFORMS; ++i)
	{
		loc_ui[i] = -1;
		loc_vec4[i] = -1;
		loc_mat4[i] = -1;
		loc_int4[i] = -1;
	}
}

ShaderProgram::~ShaderProgram()
{
	destroy();
}

unsigned int ShaderProgram::id()
{
	return id_;
}

bool ShaderProgram::valid()
{
	return valid_;
}

bool ShaderProgram::init()
{
	return init_;
}

void ShaderProgram::init_shaders_supported()
{
	glewExperimental = GL_TRUE;
	if (!init_ && glewInit() == GLEW_OK)
	{
		//get gl version
		glGetIntegerv(GL_MAJOR_VERSION, &v_major_);
		glGetIntegerv(GL_MINOR_VERSION, &v_minor_);
		ostringstream oss;
		if (v_major_ >= 3)
		{
			if (v_major_ > 3 || v_minor_ >= 3)
				oss << "#version " << v_major_ << v_minor_ << 0 << "\n";
			else
				oss << "#version " << 1 << v_minor_ + 3 << 0 << "\n";
		}
		else if (v_major_ == 2)
			oss << "#version " << 1 << v_minor_ + 1 << 0 << "\n";
		glsl_version_ = oss.str();
		glsl_unroll_ = "#pragma unroll\n";

		supported_ = glTexImage3D;

		//check max texture size
		GLint texSize;
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texSize);
		max_texture_size_ = texSize;

#ifdef _WIN32
		const GLubyte* strRenderer = 0;
		if ((strRenderer = glGetString(GL_RENDERER)))
		{
			string str = (char*)strRenderer;
			if (str.find("FirePro") != string::npos)
				no_tex_unpack_ = true;//for AMD FirePro cards
		}
#else
		no_tex_unpack_ = true;//for Mac OS
#endif

		// Check for non-power-of-two texture support.
		non_2_textures_ = true;//GLEW_ARB_texture_non_power_of_two!=0;

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

void ShaderProgram::set_max_texture_size(int size)
{
	max_texture_size_ = size;
}

void ShaderProgram::reset_max_texture_size()
{
	//check max texture size
	GLint texSize;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texSize);
	max_texture_size_ = texSize;
}

bool ShaderProgram::texture_non_power_of_two()
{
	return non_2_textures_;
}

void ShaderProgram::set_no_tex_upack(bool val)
{
	no_tex_unpack_ = val;
}

bool ShaderProgram::create()
{
	// create the GLSL program and attach the shader
	id_ = glCreateProgram();
	if (id_ == 0) return true;
	valid_ = true;

	GLuint v_shader = 0;
	GLuint f_shader = 0;
	GLuint g_shader = 0;

	v_shader = glCreateShader(GL_VERTEX_SHADER);
	f_shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (v_shader == 0 || f_shader == 0) return true;
	if (use_geom_shader_)
	{
		g_shader = glCreateShader(GL_GEOMETRY_SHADER);
		if (g_shader == 0)
			return true;
	}

	GLint shader_status[1];
	char shader_log[1000];
	GLint shader_length[1];
	GLint lengths[1];

	// set the source code and compile the shader // vertex
	const char* v_source[1];
	v_source[0] = vert_shader_.c_str();
	lengths[0] = (int)std::strlen(v_source[0]);
	glShaderSource(v_shader, 1, v_source, lengths);
	glCompileShader(v_shader);

	// check the compilation of the shader
	bool attach_vert = strcmp(*v_source, "") != 0;
	glGetShaderiv(v_shader, GL_COMPILE_STATUS, shader_status);
	if (shader_status[0] == GL_FALSE)
	{
		glGetShaderInfoLog(v_shader, sizeof(shader_log), shader_length, shader_log);
#ifdef _DEBUG
		std::string str = shader_log;
		std::wstring wstr = L"Error compiling vertex shader: " + s2ws(str) + L"\n";
		DBGPRINT(wstr.c_str());
		//std::cerr << "Error compiling vertex shader: " << shader_log << std::endl;
#endif
		attach_vert = false;
	}

	// set the source code and compile the shader // fragment
	const char* f_source[1];
	f_source[0] = frag_shader_.c_str();
	lengths[0] = (int)std::strlen(f_source[0]);
	glShaderSource(f_shader, 1, f_source, lengths);
	glCompileShader(f_shader);

	// check the compilation of the shader
	bool attach_frag = true;
	glGetShaderiv(f_shader, GL_COMPILE_STATUS, shader_status);
	if (shader_status[0] == GL_FALSE)
	{
		glGetShaderInfoLog(f_shader, sizeof(shader_log), shader_length, shader_log);
#ifdef _DEBUG
		std::string str = shader_log;
		std::wstring wstr = L"Error compiling fragment shader: " + s2ws(str) + L"\n";
		DBGPRINT(wstr.c_str());
		//std::cerr << "Error compiling fragment shader: " << shader_log << std::endl;
#endif
		attach_frag = false;
	}

	bool attach_geom = false;
	if (use_geom_shader_)
	{
		// set the source code and compile the shader // geometry
		const char* g_source[1];
		g_source[0] = geom_shader_.c_str();
		lengths[0] = (int)std::strlen(g_source[0]);
		glShaderSource(g_shader, 1, g_source, lengths);
		glCompileShader(g_shader);

		// check the compilation of the shader
		attach_geom = true;
		glGetShaderiv(g_shader, GL_COMPILE_STATUS, shader_status);
		if (shader_status[0] == GL_FALSE)
		{
			glGetShaderInfoLog(f_shader, sizeof(shader_log), shader_length, shader_log);
#ifdef _DEBUG
			std::string str = shader_log;
			std::wstring wstr = L"Error compiling geometry shader: " + s2ws(str) + L"\n";
			DBGPRINT(wstr.c_str());
			//std::cerr << "Error compiling geometry shader: " << shader_log << std::endl;
#endif
			attach_frag = false;
		}
	}

	if (attach_vert)
		glAttachShader(id_, v_shader);
	if (attach_frag)
		glAttachShader(id_, f_shader);
	if (attach_geom)
		glAttachShader(id_, g_shader);

	//link time
	glLinkProgram(id_);
	glGetProgramiv(id_, GL_LINK_STATUS, shader_status);
	if (shader_status[0] == GL_FALSE)
	{
		glGetProgramInfoLog(id_, sizeof(shader_log), shader_length, shader_log);
#ifdef _DEBUG
		std::string str = shader_log;
		std::wstring wstr = L"Error linking shaders: " + s2ws(str) + L"\n";
		DBGPRINT(wstr.c_str());
		//std::cerr << "Error linking shaders: " << shader_log << std::endl;
#endif
		return true;
	}

	glUseProgram(id_);
	if (attach_vert)
		glDetachShader(id_, v_shader);
	if (attach_frag)
		glDetachShader(id_, f_shader);
	if (attach_geom)
		glDetachShader(id_, g_shader);
	glDeleteShader(v_shader);
	glDeleteShader(f_shader);
	glDeleteShader(g_shader);

	//general use of the units
	//tex0: data volume, render buffer
	//tex1: gm (unused), operand A, shadow intermediate
	//tex2: mask, operand B, shadow intermediate
	//tex3: label, operand A mask
	//tex4: depth map, operand B mask, segmentation weight
	//tex5: segmentation weight
	//tex6: sementation mask
	//tex7: unused
	//tex8: unused
	//tex9: unused
	//tex10: unused
	//tex11: unused
	//tex12: unused
	//tex13: depth peeling layer
	//tex14: depth peeling layer
	//tex15: depth peeling layer
	const char* loc_strings[] = {
		"tex0", "tex1", "tex2", "tex3",
		"tex4", "tex5", "tex6", "tex7",
		"tex8", "tex9", "tex10", "tex11",
		"tex12", "tex13", "tex14", "tex15",
		"tex16", "tex17", "tex18", "tex19",
		"tex20", "tex21", "tex22", "tex23",
		"tex24", "tex25", "tex26", "tex27",
		"tex28", "tex29", "tex30", "tex31"
	};

	int location;
	for (size_t i = 0; i < MAX_SHADER_UNIFORMS; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + GLenum(i));
		location = glGetUniformLocation(id_, loc_strings[i]);
		if (location != -1)
			glUniform1i(location, GLint(i));
	}
	glActiveTexture(GL_TEXTURE0);

	//glValidateProgram(id_);
	//glGetProgramiv(id_, GL_VALIDATE_STATUS, shader_status);
	//if (shader_status[0] == GL_FALSE)
	//{
	//	glGetProgramInfoLog(id_, sizeof(shader_log), shader_length, shader_log);
	//	std::cerr << "Invalid shader program: " << shader_log << std::endl;
	//	return true;
	//}
	return true;
}

void ShaderProgram::destroy()
{
	glDeleteProgram(id_);
	id_ = 0;
	valid_ = false;
}

void ShaderProgram::bind()
{
	if (!valid_)
		create();
	if (valid_)
		glUseProgram(id_);
}

void ShaderProgram::bind_frag_data_location(int color_num, const char* name)
{
	glBindFragDataLocation(id_, color_num, name);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

void ShaderProgram::setLocalParam(int i, double x, double y, double z, double w)
{
	const char* loc_strings[] = {
		"loc0", "loc1", "loc2", "loc3",
		"loc4", "loc5", "loc6", "loc7",
		"loc8", "loc9", "loc10", "loc11",
		"loc12", "loc13", "loc14", "loc15",
		"loc16", "loc17", "loc18", "loc19",
		"loc20", "loc21", "loc22", "loc23",
		"loc24", "loc25", "loc26", "loc27",
		"loc28", "loc29", "loc30", "loc31" };

	if (loc_vec4[i] == -1)
	{
		loc_vec4[i] = glGetUniformLocation(id_, loc_strings[i]);
		if (loc_vec4[i] == -1)
			loc_vec4[i]--;
	}
	if (loc_vec4[i] >= 0)
		glUniform4f(loc_vec4[i], float(x), float(y), float(z), float(w));
}

void ShaderProgram::setLocalParamMatrix(int i, float* matrix4)
{
	const char* loc_strings[] = {
		"matrix0", "matrix1", "matrix2", "matrix3",
		"matrix4", "matrix5", "matrix6", "matrix7",
		"matrix8", "matrix9", "matrix10", "matrix11",
		"matrix12", "matrix13", "matrix14", "matrix15",
		"matrix16", "matrix17", "matrix18", "matrix19",
		"matrix20", "matrix21", "matrix22", "matrix23",
		"matrix24", "matrix25", "matrix26", "matrix27",
		"matrix28", "matrix29", "matrix30", "matrix31"
	};

	if (loc_mat4[i] == -1)
	{
		loc_mat4[i] = glGetUniformLocation(id_, loc_strings[i]);
		if (loc_mat4[i] == -1)
			loc_mat4[i]--;
	}
	if (loc_mat4[i] >= 0)
		glUniformMatrix4fv(loc_mat4[i], 1, false, matrix4);
}

void ShaderProgram::setLocalParamUInt(int i, unsigned int value)
{
	const char* loc_strings[] = {
		"loci0", "loci1", "loci2", "loci3",
		"loci4", "loci5", "loci6", "loci7",
		"loci8", "loci9", "loci10", "loci11",
		"loci12", "loci13", "loci14", "loci15",
		"loci16", "loci17", "loci18", "loci19",
		"loci20", "loci21", "loci22", "loci23",
		"loci24", "loci25", "loci26", "loci27",
		"loci28", "loci29", "loci30", "loci31"
	};

	if (loc_ui[i] == -1)
	{
		loc_ui[i] = glGetUniformLocation(id_, loc_strings[i]);
		if (loc_ui[i] == -1)
			loc_ui[i]--;
	}
	if (loc_ui[i] >= 0)
		glUniform1ui(loc_ui[i], value);
}

void ShaderProgram::setLocalParamInt4(int i, int x, int y, int z, int w)
{
	const char* loc_strings[] = {
		"lci0", "lci1", "lci2", "lci3",
		"lci4", "lci5", "lci6", "lci7",
		"lci8", "lci9", "lci10", "lci11",
		"lci12", "lci13", "lci14", "lci15",
		"lci16", "lci17", "lci18", "lci19",
		"lci20", "lci21", "lci22", "lci23",
		"lci24", "lci25", "lci26", "lci27",
		"lci28", "lci29", "lci30", "lci31"
	};

	if (loc_int4[i] == -1)
	{
		loc_int4[i] = glGetUniformLocation(id_, loc_strings[i]);
		if (loc_int4[i] == -1)
			loc_int4[i]--;
	}
	if (loc_int4[i] >= 0)
		glUniform4i(loc_int4[i], x, y, z, w);
}

