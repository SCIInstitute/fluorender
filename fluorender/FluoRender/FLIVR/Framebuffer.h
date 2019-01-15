//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef Framebuffer_h
#define Framebuffer_h

#include <string>
#include <vector>

namespace FLIVR
{
	enum FBTexType
	{
		FBTex_Render_RGBA = 0,
		FBTex_UChar_RGBA,
		FBTex_3D_Int,
		FBTex_Render_Int32,
		FBTex_Depth_Float,
	};
	class Framebuffer;
	class FramebufferManager;
	class FramebufferTexture
	{
	public:
		FramebufferTexture(FBTexType type, int nx, int ny);
		~FramebufferTexture();

		void create();
		void destroy();
		inline bool bind();
		inline void unbind();
		inline bool valid();
		inline void resize(int nx, int ny);

	private:
		unsigned int id_;
		FBTexType type_;
		int nx_;
		int ny_;
		bool valid_;

		friend class Framebuffer;
	};

	enum FBType
	{
		FB_Render_RGBA = 0,
		FB_UChar_RGBA,
		FB_3D_Int,
		FB_Pick_Int32_Float,
		FB_Depth_Float,
	};
	class Framebuffer
	{
	public:
		Framebuffer(FBType type, int nx, int ny, const std::string &name);
		~Framebuffer();

		void create();
		void destroy();
		inline void bind();
		inline void unbind();
		inline void protect();
		inline void unprotect();
		inline bool valid();
		inline unsigned int id();

		bool attach_texture(int ap, FramebufferTexture* tex);
		inline bool attach_texture(int ap, unsigned int tex_id, int layer=0);
		void detach_texture(int ap);
		void detach_texture(FramebufferTexture* tex);
		void bind_texture(int ap);
		unsigned int tex_id(int ap);

		inline bool match_size(int nx, int ny);
		void resize(int nx, int ny);

		//match without size
		bool match(FBType);
		//match with size
		bool match(FBType, int, int);
		//match by name
		inline bool match(const std::string &name);

		//name represents its use
		inline void set_name(const std::string &name);
		inline void clear_name();
		std::string &get_name() { return name_; }

		//read pick value
		inline unsigned int read_value(int, int);

	private:
		unsigned int id_;
		FBType type_;
		int nx_;
		int ny_;
		std::string name_;//specify its use
		bool valid_;
		bool protected_;
		std::vector<std::pair<int, FramebufferTexture*>> tex_list_;

		friend class FramebufferManager;
	};

	class FramebufferManager
	{
	public:
		FramebufferManager();
		~FramebufferManager();
		void clear();

		Framebuffer* framebuffer(FBType type, int nx, int ny,
			const std::string &name="");
		Framebuffer* framebuffer(const std::string &name);

	private:
		std::vector<Framebuffer*> fb_list_;
		std::vector<FramebufferTexture*> tex_list_;
	};

	inline bool FramebufferTexture::bind()
	{
		if (valid_)
		{
			switch (type_)
			{
			case FBTex_Render_RGBA:
			case FBTex_UChar_RGBA:
			case FBTex_Render_Int32:
			case FBTex_Depth_Float:
			default:
				glBindTexture(GL_TEXTURE_2D, id_);
				break;
			}
			return true;
		}
		else
			return false;
	}

	inline void FramebufferTexture::unbind()
	{
		if (valid_)
		{
			switch (type_)
			{
			case FBTex_Render_RGBA:
			case FBTex_UChar_RGBA:
			case FBTex_Render_Int32:
			case FBTex_Depth_Float:
			default:
				glBindTexture(GL_TEXTURE_2D, 0);
				break;
			}
		}
	}

	inline bool FramebufferTexture::valid()
	{
		return valid_;
	}

	inline void FramebufferTexture::resize(int nx, int ny)
	{
		if (valid_)
		{
			nx_ = nx; ny_ = ny;
			switch (type_)
			{
			case FBTex_Render_RGBA:
			default:
				glBindTexture(GL_TEXTURE_2D, id_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
					GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
				break;
			case FBTex_UChar_RGBA:
				glBindTexture(GL_TEXTURE_2D, id_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nx_, ny_, 0,
					GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				break;
			case FBTex_Render_Int32:
				glBindTexture(GL_TEXTURE_2D, id_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, nx_, ny_, 0,
					GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
				break;
			case FBTex_Depth_Float:
				glBindTexture(GL_TEXTURE_2D, id_);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, nx_, ny_, 0,
					GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				break;
			}
		}
	}

	inline void Framebuffer::bind()
	{
		if (valid_)
			glBindFramebuffer(GL_FRAMEBUFFER, id_);
	}

	inline void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	inline void Framebuffer::protect()
	{
		protected_ = true;
	}

	inline void Framebuffer::unprotect()
	{
		protected_ = false;
	}

	inline bool Framebuffer::valid()
	{
		return valid_;
	}

	inline unsigned int Framebuffer::id()
	{
		return id_;
	}

	inline bool Framebuffer::attach_texture(int ap, unsigned int tex_id, int layer)
	{
		if (!valid_)
			return false;
		switch (type_)
		{
		case FB_Render_RGBA:
		case FB_UChar_RGBA:
		case FB_Pick_Int32_Float:
		case FB_Depth_Float:
		default:
			glFramebufferTexture(GL_FRAMEBUFFER,
				ap, tex_id, 0);
			break;
		case FB_3D_Int:
			glFramebufferTexture3D(GL_FRAMEBUFFER,
				ap, GL_TEXTURE_3D,
				tex_id, 0, layer);
			break;
		}
		return true;
	}

	inline bool Framebuffer::match_size(int nx, int ny)
	{
		return (nx == nx_) && (ny == ny_);
	}

	inline bool Framebuffer::match(const std::string &name)
	{
		if (name == "")
			return false;
		if (name_ == name)
			return true;
		return false;
	}

	inline void Framebuffer::set_name(const std::string &name)
	{
		name_ = name;
	}

	inline void Framebuffer::clear_name()
	{
		name_ = "";
	}

	inline unsigned int Framebuffer::read_value(int px, int py)
	{
		if (type_ != FB_Pick_Int32_Float)
			return 0;
		unsigned int value = 0;
		glReadPixels(px, py, 1, 1, GL_RED_INTEGER,
			GL_UNSIGNED_INT, (GLvoid*)&value);
		return value;
	}
}
#endif//Framebuffer_h