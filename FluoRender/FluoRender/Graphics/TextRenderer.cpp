/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <GL/glew.h>
#include <TextRenderer.h>
#include <Global.h>
#include <Names.h>
#include <ShaderProgram.h>
#include <Framebuffer.h>
#include <FramebufferStateTracker.h>
#include <Color.h>
#include <ImgShader.h>
#include <VertexArray.h>
#include <compatibility.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace flvr
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

	inline void TextTexture::bind()
	{
		if (valid_)
			glBindTexture(GL_TEXTURE_2D, id_);
	}

	inline void TextTexture::unbind()
	{
		if (valid_)
			glBindTexture(GL_TEXTURE_2D, 0);
	}

	TextTextureManager::TextTextureManager():
		m_init(false),
		m_valid(false),
		m_size(14),
		m_enlarge_scale(1)
	{
	}

	TextTextureManager::~TextTextureManager()
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
			delete it->second;
		if (m_init && m_valid)
			FT_Done_Face(m_face);
		if (m_init)
			FT_Done_FreeType(m_ft);
	}

	void TextTextureManager::load_face(const std::wstring &lib_name)
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

		std::string str = ws2s(lib_name);
		err = FT_New_Face(m_ft, str.c_str(), 0, &m_face);
		if (!err)
			m_valid = true;

		if (m_valid)
		{
			err = FT_Select_Charmap(m_face, FT_ENCODING_UNICODE);
			err = FT_Set_Pixel_Sizes(m_face, 0, FT_UInt(m_size * m_enlarge_scale + 0.5));
		}
		clear();
	}

	void TextTextureManager::SetSize(unsigned int size)
	{
		if (!m_valid)
			return;
		m_size = size;
		FT_Set_Pixel_Sizes(m_face, 0, FT_UInt(m_size * m_enlarge_scale + 0.5));
		clear();
	}

	void TextTextureManager::SetEnlargeScale(double dval)
	{
		m_enlarge_scale = dval;
		SetSize(m_size);
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

	TextRenderer::TextRenderer()
	{
	}

	TextRenderer::~TextRenderer()
	{
	}

	void TextRenderer::RenderText(const std::wstring& text, const fluo::Color &color,
		float x, float y, float sx, float sy)
	{
		auto cur_buffer = glbin_framebuffer_manager.current();
		assert(cur_buffer);
		FramebufferStateGuard fbg(*cur_buffer);
		cur_buffer->set_depth_test_enabled(false);
		cur_buffer->set_cull_face_enabled(false);
		cur_buffer->set_blend_enabled(true);
		cur_buffer->set_blend_func(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
		cur_buffer->apply_state();

		auto shader = glbin_shader_manager.shader(gstImgShader,
			ShaderParams::Img(IMG_SHDR_DRAW_TEXT, 0));
		assert(shader);
		shader->bind();
		shader->setLocalParam(0, color.r(), color.g(), color.b(), 1.0f);

		auto va_text = glbin_vertex_array_manager.vertex_array(VAType::VA_Text);
		assert(va_text);
		va_text->draw_begin();
		const wchar_t *p;
		for (p = text.c_str(); *p; p++)
		{
			TextTexture* tex_p =
				glbin_text_tex_manager.text_texture(*p);
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

		glBindTexture(GL_TEXTURE_2D, 0);
		shader->unbind();
	}

	float TextRenderer::RenderTextLen(std::wstring& text)
	{
		float len = 0.0f;

		const wchar_t *p;
		for (p = text.c_str(); *p; p++)
		{
			TextTexture* tex_p =
				glbin_text_tex_manager.text_texture(*p);
			if (tex_p)
				len += (tex_p->ax_ >> 6);
		}
		return len;
	}
}