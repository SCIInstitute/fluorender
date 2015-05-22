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
#include <FLIVR/ShaderProgram.h>

bool TextRenderer::m_init = false;
FT_Library TextRenderer::m_ft;

#define TXT_RENDER_VTX_CODE \
	"//TXT_RENDER_VTX_CODE\n" \
	"layout(location = 0) in vec4 coord;\n" \
	"out vec2 texcoord;\n" \
	"\n" \
	"void main(void)\n" \
	"{\n" \
	"	gl_Position = vec4(coord.xy, 0, 1);\n" \
	"	texcoord = coord.zw;\n" \
	"}\n"

#define TXT_RENDER_FRG_CODE \
	"//TXT_RENDER_FRG_CODE\n" \
	"in vec2 texcoord;\n" \
	"out vec4 FragColor;\n" \
	"uniform sampler2D tex;\n" \
	"uniform vec4 color;\n" \
	"\n" \
	"void main(void)\n" \
	"{\n" \
	"	FragColor = vec4(1, 1, 1, texture(tex, texcoord).r) * color;\n" \
	"//	FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n" \
	"}\n"

TextRenderer::TextRenderer(const string &lib_name)
	: m_valid(false),
	m_size(14),
	m_init_gl(false),
	m_tex(0),
	m_vbo(0),
	m_vao(0),
	m_prog(0)
{
	FT_Error err;
	if (!m_init)
	{
		err = FT_Init_FreeType(&m_ft);
		if (!err)
			m_init = true;
	}

	if (!m_init) return;

	err = FT_New_Face(m_ft, lib_name.c_str(), 0, &m_face);
	if (!err)
		m_valid = true;

	if (m_valid)
	{
		err = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE); 
		err = FT_Set_Pixel_Sizes(m_face, 0, m_size);
	}
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

void TextRenderer::LoadNewFace(const string &lib_name)
{
	FT_Error err;
	if (!m_init)
	{
		err = FT_Init_FreeType(&m_ft);
		if (!err)
			m_init = true;
	}

	if (!m_init) return;

	if (m_valid)
	{
		FT_Done_Face(m_face);
		m_valid = false;
	}

	err = FT_New_Face(m_ft, lib_name.c_str(), 0, &m_face);
	if (!err)
		m_valid = true;

	if (m_valid)
	{
		err = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE); 
		err = FT_Set_Pixel_Sizes(m_face, 0, m_size);
	}
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

void TextRenderer::RenderText(const wstring& text, Color &color,
	float x, float y, float sx, float sy)
{
	if (!m_valid)
		return;

	GLint loc;
	if (!m_init_gl)
	{
		//texture
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &m_tex);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//shader
		m_prog = glCreateProgram();
		GLuint v_shader, f_shader;
		v_shader = glCreateShader(GL_VERTEX_SHADER);
		f_shader = glCreateShader(GL_FRAGMENT_SHADER);

		string v_source_str, f_source_str;
		const char *v_source[1], *f_source[1];
		v_source_str = ShaderProgram::glsl_version_ + TXT_RENDER_VTX_CODE;
		f_source_str = ShaderProgram::glsl_version_ + TXT_RENDER_FRG_CODE;
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
		m_color_loc = glGetUniformLocation(m_prog, "color");

		//vertex
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		glGenBuffers(1, &m_vbo);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		m_init_gl = true;
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_tex);
	//shader
	glUseProgram(m_prog);
	glUniform4f(m_color_loc, color.r(), color.g(), color.b(), 1.0f);
	//vertex
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	const wchar_t *p;
	for(p = text.c_str(); *p; p++)
	{
		if(FT_Load_Char(m_face, *p, FT_LOAD_RENDER))
			continue;

		FT_GlyphSlot g = m_face->glyph;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
			);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] =
		{
			{x2,     -y2    , 0, 0},
			{x2 + w, -y2    , 1, 0},
			{x2,     -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*16, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

float TextRenderer::RenderTextLen(wstring& text)
{
	float len = 0.0f;
	if (!m_valid)
		return len;

	const wchar_t *p;
	for(p = text.c_str(); *p; p++)
	{
		if(FT_Load_Char(m_face, *p, FT_LOAD_RENDER))
			continue;

		FT_GlyphSlot g = m_face->glyph;
		len += g->bitmap.width;
	}
	return len;
}