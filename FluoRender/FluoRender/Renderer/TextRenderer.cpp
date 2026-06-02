/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <TextRenderer.h>
#include <Global.h>
#include <Names.h>
#include <ShaderProgram.h>
#include <Framebuffer.h>
#include <FramebufferStateTracker.h>
#include <ImageTexture.h>
#include <Color.h>
#include <ImgShader.h>
#include <VertexArray.h>
#include <compatibility.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace flvr
{
	FontGlyph::FontGlyph(wchar_t p) :
		ch_(p)
	{
	}

	void FontGlyph::create(FT_Face face)
	{
		if (!face)
			return;

		// Load glyph bitmap via FreeType
		if (FT_Load_Char(face, ch_, FT_LOAD_RENDER))
			return;

		FT_GlyphSlot g = face->glyph;

		// Create texture description for glyph bitmap
		TextureDesc desc;
		desc.spec.type = TextureType::Tex2D;
		desc.spec.format = TextureFormat::R8; // or R8_UNORM, depending on your enum
		desc.spec.useMipmap = false;
		desc.spec.minFilter = TexFilter::Linear;
		desc.spec.magFilter = TexFilter::Linear;
		desc.spec.wrapS = TexWrap::ClampToEdge;
		desc.spec.wrapT = TexWrap::ClampToEdge;
		desc.size.width = g->bitmap.width;
		desc.size.height = g->bitmap.rows;
		desc.size.depth = 1;

		// Allocate ImageTexture
		texture_ = std::make_shared<ImageTexture>(desc);
		texture_->create();

		// Upload bitmap data (1 byte per pixel)
		texture_->upload_2d(
			g->bitmap.buffer,
			g->bitmap.width,
			g->bitmap.rows
		);

		// Cache glyph metrics
		left_ = g->bitmap_left;
		top_ = g->bitmap_top;
		width_ = g->bitmap.width;
		rows_ = g->bitmap.rows;
		ax_ = static_cast<unsigned int>(g->advance.x);
		ay_ = static_cast<unsigned int>(g->advance.y);

		valid_ = true;
	}

	void FontGlyph::destroy()
	{
		texture_.reset();
		valid_ = false;
	}

	FontGlyphManager::FontGlyphManager()
	{
	}

	FontGlyphManager::~FontGlyphManager()
	{
		glyphs_.clear();
		if (initialized_ && valid_)
			FT_Done_Face(face_);
		if (initialized_)
			FT_Done_FreeType(ft_lib_);
	}

	void FontGlyphManager::load_face(const std::wstring &lib_name)
	{
		FT_Error err;
		if (!initialized_)
		{
			err = FT_Init_FreeType(&ft_lib_);
			if (!err)
				initialized_ = true;
		}

		if (!initialized_) return;

		if (valid_)
		{
			FT_Done_Face(face_);
			valid_ = false;
		}

		std::string str = ws2s(lib_name);
		err = FT_New_Face(ft_lib_, str.c_str(), 0, &face_);
		if (!err)
			valid_ = true;

		if (valid_)
		{
			err = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
			err = FT_Set_Pixel_Sizes(face_, 0,
				static_cast<FT_UInt>(std::round(size_ * enlarge_scale_)));
		}
		clear();
	}

	void FontGlyphManager::set_size(unsigned int size)
	{
		if (!valid_)
			return;
		size_ = size;
		FT_Set_Pixel_Sizes(face_, 0,
			static_cast<FT_UInt>(std::round(size_ * enlarge_scale_)));
		clear();
	}

	unsigned int FontGlyphManager::get_size() const
	{
		return size_;
	}

	void FontGlyphManager::set_enlarge_scale(double dval)
	{
		enlarge_scale_ = dval;
		set_size(size_);
	}

	void FontGlyphManager::clear()
	{
		glyphs_.clear();
	}

	std::shared_ptr<FontGlyph> FontGlyphManager::glyph(wchar_t ch)
	{
		auto it = glyphs_.find(ch);
		if (it != glyphs_.end())
			return it->second;

		auto result = std::make_shared<FontGlyph>(ch);
		result->create(face_);
		glyphs_.emplace(ch, result);

		return result;
	}

	TextRenderer::TextRenderer()
	{
	}

	TextRenderer::~TextRenderer()
	{
	}

	void TextRenderer::render_text(
		const std::wstring& text,
		const fluo::Color &color,
		float x, float y,
		float sx, float sy)
	{
		auto cur_buffer = glbin_framebuffer_manager.current();
		assert(cur_buffer);
		FramebufferStateGuard fbg(*cur_buffer);
		cur_buffer->set_depth_test_enabled(false);
		cur_buffer->set_cull_face_enabled(false);
		cur_buffer->set_blend_enabled_all(true);
		cur_buffer->set_blend_func_all(BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);
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
			auto tex_p =
				glbin_text_tex_manager.glyph(*p);
			if (tex_p)
			{
				tex_p->texture().bind(0);
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

		shader->unbind();
	}

	float TextRenderer::render_text_length(const std::wstring& text)
	{
		float len = 0.0f;

		const wchar_t *p;
		for (p = text.c_str(); *p; p++)
		{
			auto tex_p =
				glbin_text_tex_manager.glyph(*p);
			if (tex_p)
				len += (tex_p->ax_ >> 6);
		}
		return len;
	}
}