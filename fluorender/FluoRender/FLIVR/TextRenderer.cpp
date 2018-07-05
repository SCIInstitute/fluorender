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
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/Color.h>

namespace FLIVR
{
	TextTexture::TextTexture(wchar_t p) :
		id_(0), p_(p), valid_(false)
	{
	}

	TextTexture::~TextTexture()
	{
		destroy();
	}

	void TextTexture::create(FT_Face face)
	{
		if (!face)
			return;
		glGenTextures(1, &id_);
		glBindTexture(GL_TEXTURE_2D, id_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (FT_Load_Char(face, p_, FT_LOAD_RENDER))
			return;
		FT_GlyphSlot g = face->glyph;
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
		left_ = g->bitmap_left;
		top_ = g->bitmap_top;
		width_ = g->bitmap.width;
		rows_ = g->bitmap.rows;
		ax_ = g->advance.x;
		ay_ = g->advance.y;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		valid_ = true;
	}

	void TextTexture::destroy()
	{
		glDeleteTextures(1, &id_);
		id_ = 0;
		valid_ = false;
	}

	TextTextureManager::TextTextureManager():
		m_init(false),
		m_valid(false),
		m_size(14)
	{
	}

	TextTextureManager::~TextTextureManager()
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
			delete it->second;
	}

	void TextTextureManager::load_face(const std::string &lib_name)
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
		clear();
	}

	void TextTextureManager::SetSize(unsigned int size)
	{
		if (!m_valid)
			return;
		FT_Set_Pixel_Sizes(m_face, 0, size);
		m_size = size;
		clear();
	}

	void TextTextureManager::clear()
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
			delete it->second;
		tex_list_.clear();
	}

	TextTexture* TextTextureManager::text_texture(wchar_t p)
	{
		TextTexture* result = 0;
		auto it = tex_list_.find(p);
		if (it != tex_list_.end())
			result = it->second;
		else
		{
			result = new TextTexture(p);
			result->create(m_face);
			tex_list_.insert(std::pair<wchar_t, TextTexture*>(
				p, result));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		return result;
	}

	TextTextureManager TextRenderer::text_texture_manager_;
	TextRenderer::TextRenderer()
	{
	}

	TextRenderer::~TextRenderer()
	{
	}

	void TextRenderer::RenderText(const std::wstring& text, Color &color,
		float x, float y, float sx, float sy)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ShaderProgram* shader =
			TextureRenderer::m_img_shader_factory.shader(IMG_SHDR_DRAW_TEXT);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
			shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0f);
		}

		VertexArray* va_text =
			TextureRenderer::vertex_array_manager_.vertex_array(VA_Text);
		if (va_text && shader)
		{
			va_text->draw_begin();
			const wchar_t *p;
			for (p = text.c_str(); *p; p++)
			{
				TextTexture* tex_p =
					text_texture_manager_.text_texture(*p);
				if (tex_p)
				{
					tex_p->bind();
					float x2 = x + tex_p->left_ * sx;
					float y2 = -y - tex_p->top_ * sy;
					float w = tex_p->width_ * sx;
					float h = tex_p->rows_ * sy;

					float mat[16] = {
						w, 0.0f, 0.0f, 0.0f,
						0.0f, -h, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						x2, -y2, 0.0f, 1.0f
					};
					shader->setLocalParamMatrix(0, mat);
					va_text->draw_norm_square();

					x += (tex_p->ax_ >> 6) * sx;
					y += (tex_p->ay_ >> 6) * sy;
				}

			}
			va_text->draw_end();
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		if (shader && shader->valid())
			shader->release();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	float TextRenderer::RenderTextLen(std::wstring& text)
	{
		float len = 0.0f;

		const wchar_t *p;
		for (p = text.c_str(); *p; p++)
		{
			TextTexture* tex_p =
				text_texture_manager_.text_texture(*p);
			if (tex_p)
				len += tex_p->width_;
		}
		return len;
	}
}