/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include "TextRenderer.h"

bool TextRenderer::m_init = false;
FT_Library TextRenderer::m_ft;

#define TXT_RENDER_VTX_CODE \
	"//TXT_RENDER_VTX_CODE\n" \
	"#version 400\n" \
	"in vec4 coord;\n" \
	"out vec2 texcoord;\n" \
	"\n" \
	"void main(void)\n" \
	"{\n" \
	"	gl_Position = vec4(coord.xy, 0, 1);\n" \
	"	texcoord = coord.zw;\n" \
	"}\n"

#define TXT_RENDER_FRG_CODE \
	"//TXT_RENDER_FRG_CODE\n" \
	"#version 400\n" \
	"in vec2 texcoord;\n" \
	"out vec4 FragColor;\n" \
	"uniform sampler2D tex;\n" \
	"uniform vec4 color;\n" \
	"\n" \
	"void main(void)\n" \
	"{\n" \
	"	FragColor = vec4(1, 1, 1, texture2D(tex, texcoord).r) * color;\n" \
	"}\n"

TextRenderer::TextRenderer(const string &lib_name)
	: m_valid(false),
	m_size(0),
	m_init_gl(false),
	m_tex(0),
	m_vbo(0),
	m_vao(0),
	m_prog(0)
{
	if (!m_init && !FT_Init_FreeType(&m_ft))
		m_init = true;

	if (!m_init) return;

	if (!FT_New_Face(m_ft, lib_name.c_str(), 0, &m_face))
		m_valid = true;
}

TextRenderer::~TextRenderer()
{
	if (glIsTexture(m_tex))
		glDeleteTextures(1, &m_tex);
	if (glIsBuffer(m_vbo))
		glDeleteBuffers(1, &m_vbo);
	if (glIsVertexArray(m_vao))
		glDeleteVertexArrays(1, &m_vao);
	if (glIsProgram(m_prog))
		glDeleteProgram(m_prog);
}

void TextRenderer::SetSize(unsigned int size)
{
	if (!m_valid)
		return;

	FT_Set_Pixel_Sizes(m_face, 0, size);
	m_size = size;
}

unsigned int TextRenderer::GetSize()
{
	if (!m_valid)
		return 0;
	else
		return m_size;
}

void TextRenderer::RenderText(string& text, Color &color,
	float x, float y, float sx, float sy)
{
	GLint loc;
	if (!m_init_gl)
	{
		m_prog = glCreateProgram();
		GLuint v_shader, f_shader;
		v_shader = glCreateShader(GL_VERTEX_SHADER);
		f_shader = glCreateShader(GL_FRAGMENT_SHADER);

		string v_source_str, f_source_str;
		const char *v_source[1], *f_source[1];
		v_source_str = TXT_RENDER_VTX_CODE;
		f_source_str = TXT_RENDER_FRG_CODE;
		GLint lengths[1];

		v_source[0] = v_source_str.c_str();
		lengths[0] = (GLint)v_source_str.length();
		glShaderSource(v_shader, 1, v_source, lengths);
		glCompileShader(v_shader);

		f_source[0] = f_source_str.c_str();
		lengths[0] = (GLint)f_source_str.length();
		glShaderSource(f_shader, 1, f_source, lengths);
		glCompileShader(f_shader);

		glAttachShader(m_prog, v_shader);
		glAttachShader(m_prog, f_shader);
		glLinkProgram(m_prog);
		glDetachShader(m_prog, v_shader);
		glDetachShader(m_prog, f_shader);
		glDeleteShader(v_shader);
		glDeleteShader(f_shader);

		glUseProgram(m_prog);
		loc = glGetUniformLocation(m_prog, "tex");
		if (loc != -1)
			glUniform1i(loc, 0);
	}
}

