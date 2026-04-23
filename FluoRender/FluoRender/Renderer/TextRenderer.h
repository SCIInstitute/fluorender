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

#ifndef _TEXTRENDERER_H_
#define _TEXTRENDERER_H_

#include <string>
#include <map>
#include <memory>

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
	// Forward declarations
	class ImageTexture;

	// ---------------------------------------------
	// FontGlyph
	// ---------------------------------------------
	// Represents a single glyph:
	// - one Unicode character
	// - glyph metrics
	// - one GPU texture (glyph bitmap)
	class FontGlyph
	{
	public:
		explicit FontGlyph(wchar_t ch);
		~FontGlyph() = default;

		void create(FT_Face face);
		void destroy();

		bool valid() const { return valid_; }
		bool match(wchar_t ch) const { return ch_ == ch; }

		ImageTexture& texture() { return *texture_; }
		const ImageTexture& texture() const { return *texture_; }

	public:
		// Glyph metrics (FreeType-based)
		int left_ = 0;
		int top_ = 0;
		int width_ = 0;
		int rows_ = 0;
		unsigned int ax_ = 0; // advance.x
		unsigned int ay_ = 0; // advance.y

	private:
		wchar_t ch_;
		bool valid_ = false;

		std::shared_ptr<ImageTexture> texture_;

		friend class FontGlyphManager;
	};

	// ---------------------------------------------
	// FontGlyphManager
	// ---------------------------------------------
	// Manages:
	// - FreeType library and face
	// - glyph size
	// - glyph -> FontGlyph cache
	class FontGlyphManager
	{
	public:
		FontGlyphManager();
		~FontGlyphManager();

		void load_face(const std::wstring& font_file);
		void set_size(unsigned int size);
		unsigned int get_size() const;
		void set_enlarge_scale(double scale);

		void clear();

		// Fetch or create glyph
		std::shared_ptr<FontGlyph> glyph(wchar_t ch);

	private:
		std::map<wchar_t, std::shared_ptr<FontGlyph>> glyphs_;

		FT_Library ft_lib_ = nullptr;
		FT_Face    face_ = nullptr;

		bool initialized_ = false;
		bool valid_ = false;

		unsigned int size_ = 0;
		double enlarge_scale_ = 1.0;
	};

	// ---------------------------------------------
	// TextRenderer
	// ---------------------------------------------
	// Stateless renderer that draws text using glyphs
	class TextRenderer
	{
	public:
		TextRenderer();
		~TextRenderer();

		void render_text(const std::wstring& text,
			const fluo::Color& color,
			float x, float y,
			float sx, float sy);

		float render_text_length(const std::wstring& text);
	};

} // namespace flvr

#endif // _TEXTRENDERER_H_