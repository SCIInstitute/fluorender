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

#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include <string>
#include <map>

// Forward declarations for FreeType2 types
struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;

namespace fluo
{
	class Color;
}
namespace flvr
{
	class TextTextureManager;
	class TextTexture
	{
	public:
		TextTexture(wchar_t p);
		~TextTexture();

		void create(FT_Face face);
		void destroy();

		inline void bind();
		inline void unbind();
		inline bool valid();
		inline bool match(wchar_t);

	public:
		int left_;
		int top_;
		int width_;
		int rows_;
		unsigned int ax_;
		unsigned int ay_;

	private:
		unsigned int id_;
		wchar_t p_;
		bool valid_;

		friend class TextTextureManager;
	};

	inline bool TextTexture::valid()
	{
		return valid_;
	}

	inline bool TextTexture::match(wchar_t p)
	{
		return (p == p_);
	}

	class TextTextureManager
	{
	public:
		TextTextureManager();
		~TextTextureManager();

		void load_face(const std::wstring &lib_name);
		void SetSize(unsigned int size);
		inline unsigned int GetSize();
		void SetEnlargeScale(double dval);

		void clear();
		TextTexture* text_texture(wchar_t p);

	private:
		std::map<wchar_t, TextTexture*> tex_list_;

		FT_Library m_ft;
		bool m_init;
		bool m_valid;
		FT_Face m_face;
		unsigned int m_size;
		double m_enlarge_scale;
	};

	inline unsigned int TextTextureManager::GetSize()
	{
		if (!m_valid)
			return 0;
		else
			return (unsigned int)(m_size * m_enlarge_scale + 0.5);
	}

	class TextRenderer
	{
	public:
		TextRenderer();
		~TextRenderer();

		void RenderText(const std::wstring& text, const fluo::Color &color,
			float x, float y, float sx, float sy);
		float RenderTextLen(std::wstring& text);
	};

}

#endif//_TEXTRENDERER_H_