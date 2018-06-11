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
//  

#include "GL/glew.h"
#include "Framebuffer.h"

namespace FLIVR
{
	FramebufferTexture::FramebufferTexture(FBTexType type, int nx, int ny):
		id_(0), type_(type), nx_(nx), ny_(ny), valid_(false)
	{
	}

	FramebufferTexture::~FramebufferTexture()
	{
		destroy();
	}

	bool FramebufferTexture::create()
	{
		glGenTextures(1, &id_);
		if (!glIsTexture(id_))
			return false;
		glBindTexture(GL_TEXTURE_2D, id_);
		switch (type_)
		{
		case FBTex_Render_RGBA:
		default:
			glBindTexture(GL_TEXTURE_2D, id_);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
				GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
			break;
		}
		valid_ = true;
		return true;
	}

	void FramebufferTexture::destroy()
	{
		glDeleteTextures(1, &id_);
		id_ = 0;
		valid_ = false;
	}

	bool FramebufferTexture::bind()
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

	void FramebufferTexture::unbind()
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

	bool FramebufferTexture::valid()
	{
		return valid_;
	}

	void FramebufferTexture::resize(int nx, int ny)
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

	Framebuffer::Framebuffer(FBType type, int nx, int ny):
		id_(0), type_(type), nx_(nx), ny_(ny), valid_(false), protected_(false)
	{
	}

	Framebuffer::~Framebuffer()
	{
		destroy();
	}

	bool Framebuffer::create()
	{
		glGenFramebuffers(1, &id_);
		if (!glIsFramebuffer(id_))
			return false;
		valid_ = true;
		return true;
	}

	void Framebuffer::destroy()
	{
		glDeleteFramebuffers(1, &id_);
		id_ = 0;
		valid_ = false;
	}

	void Framebuffer::bind()
	{
		if (valid_)
			glBindFramebuffer(GL_FRAMEBUFFER, id_);
	}

	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::protect()
	{
		protected_ = true;
	}

	void Framebuffer::unprotect()
	{
		protected_ = false;
	}

	bool Framebuffer::valid()
	{
		return valid_;
	}

	bool Framebuffer::attach_texture(int ap, FramebufferTexture* tex)
	{
		if (!valid_)
			return false;
		switch (type_)
		{
		case FB_Render_RGBA:
			{
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

	bool Framebuffer::attach_texture(int ap, unsigned int tex_id, int layer)
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

	void Framebuffer::detach_texture(int ap)
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

	void Framebuffer::detach_texture(FramebufferTexture* tex)
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

	void Framebuffer::resize(int nx, int ny)
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
		{
			if ((*it).second->valid())
				(*it).second->resize(nx, ny);
		}
		nx_ = nx; ny_ = ny;
	}
}