//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
		FBTex_3D_Int,
	};
	class Framebuffer;
	class FramebufferTexture
	{
	public:
		FramebufferTexture(FBTexType type, int nx, int ny);
		~FramebufferTexture();

		bool create();
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
		FB_3D_Int,
	};
	class Framebuffer
	{
	public:
		Framebuffer(FBType type, int nx, int ny);
		~Framebuffer();

		bool create();
		void destroy();
		inline void bind();
		inline void unbind();
		inline void protect();
		inline void unprotect();
		inline bool valid();

		inline bool attach_texture(int ap, FramebufferTexture* tex);
		inline bool attach_texture(int ap, unsigned int tex_id, int layer=0);
		inline void detach_texture(int ap);
		inline void detach_texture(FramebufferTexture* tex);
		inline void bind_texture(int ap);

		inline bool match_size(int nx, int ny);
		inline void resize(int nx, int ny);

		//match without size
		inline bool match(FBType, int);
		//match with size
		inline bool match(FBType, int, int, int);

	private:
		unsigned int id_;
		FBType type_;
		int nx_;
		int ny_;
		bool valid_;
		bool protected_;
		std::vector<std::pair<int, FramebufferTexture*>> tex_list_;
	};

	class FramebufferManager
	{
	public:
		FramebufferManager();
		~FramebufferManager();

		Framebuffer* framebuffer(FBType type, int nx, int ny, int ap);

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

	inline bool Framebuffer::attach_texture(int ap, FramebufferTexture* tex)
	{
		if (!valid_)
			return false;
		switch (type_)
		{
		case FB_Render_RGBA:
		{
			glBindFramebuffer(GL_FRAMEBUFFER, id_);
			glFramebufferTexture2D(GL_FRAMEBUFFER,
				ap, GL_TEXTURE_2D, tex->id_, 0);
			std::pair<int, FramebufferTexture*> item(ap, tex);
			auto it = std::find(tex_list_.begin(),
				tex_list_.end(), item);
			if (it == tex_list_.end())
				tex_list_.push_back(item);

		}
		break;
		}
		return true;
	}

	inline bool Framebuffer::attach_texture(int ap, unsigned int tex_id, int layer)
	{
		if (!valid_)
			return false;
		switch (type_)
		{
		case FB_Render_RGBA:
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

	inline void Framebuffer::detach_texture(int ap)
	{
		glFramebufferTexture(GL_FRAMEBUFFER,
			ap, 0, 0);
		for (auto it = tex_list_.begin();
			it != tex_list_.end();)
		{
			if ((*it).first == ap)
				it = tex_list_.erase(it);
			else
				++it;
		}
	}

	inline void Framebuffer::detach_texture(FramebufferTexture* tex)
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end();)
		{
			if ((*it).second == tex)
			{
				glFramebufferTexture(GL_FRAMEBUFFER,
					(*it).first, 0, 0);
				it = tex_list_.erase(it);
			}
			else
				++it;
		}
	}

	inline void Framebuffer::bind_texture(int ap)
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
		{
			if ((*it).first == ap)
				(*it).second->bind();
		}
	}

	inline bool Framebuffer::match_size(int nx, int ny)
	{
		return (nx == nx_) && (ny == ny_);
	}

	inline void Framebuffer::resize(int nx, int ny)
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
		{
			if ((*it).second->valid())
				(*it).second->resize(nx, ny);
		}
		nx_ = nx; ny_ = ny;
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	inline bool Framebuffer::match(FBType type, int ap)
	{
		if (protected_)
			return false;
		if (type_ == type)
		{
			if (type == FB_3D_Int)
				return true;
			for (auto it = tex_list_.begin();
				it != tex_list_.end(); ++it)
			{
				if ((*it).first == ap &&
					(*it).second->valid())
					return true;
			}
		}
		return false;
	}

	inline bool Framebuffer::match(FBType type,
		int nx, int ny, int ap)
	{
		if (protected_)
			return false;
		if (type_ == type &&
			nx_ == nx &&
			ny_ == ny)
		{
			if (type == FB_3D_Int)
				return true;
			for (auto it = tex_list_.begin();
				it != tex_list_.end(); ++it)
			{
				if ((*it).first == ap &&
					(*it).second->valid())
					return true;
			}
		}
		return false;
	}

}
#endif//Framebuffer_h