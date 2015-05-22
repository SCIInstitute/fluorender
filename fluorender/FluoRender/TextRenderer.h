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

#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include <glew.h>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <FLIVR/Color.h>

using namespace std;
using namespace FLIVR;

class TextRenderer
{
public:
	TextRenderer(const string &lib_name);
	~TextRenderer();

	void LoadNewFace(const string &lib_name);
	void SetSize(unsigned int size);
	unsigned int GetSize();

	void RenderText(const wstring& text, Color &color,
		float x, float y, float sx, float sy);
	float RenderTextLen(wstring& text);

private:
	static FT_Library m_ft;
	static bool m_init;

	bool m_valid;
	FT_Face m_face;

	unsigned int m_size;

	bool m_init_gl;
	//gl
	GLuint m_tex;
	GLuint m_vbo, m_vao;
	GLuint m_prog;
	GLint m_color_loc;
};

#endif//_TEXTRENDERER_H_