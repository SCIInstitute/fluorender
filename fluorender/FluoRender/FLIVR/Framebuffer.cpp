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
		//if (!glIsTexture(id_))
		//	return false;
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

	Framebuffer::Framebuffer(FBType type, int nx, int ny,
		const std::string &name):
		id_(0), type_(type), nx_(nx), ny_(ny),
		name_(name),
		valid_(false), protected_(false)
	{
	}

	Framebuffer::~Framebuffer()
	{
		destroy();
	}

	bool Framebuffer::create()
	{
		glGenFramebuffers(1, &id_);
		//if (!glIsFramebuffer(id_))
		//	return false;
		valid_ = true;
		return true;
	}

	void Framebuffer::destroy()
	{
		glDeleteFramebuffers(1, &id_);
		id_ = 0;
		valid_ = false;
	}

	FramebufferManager::FramebufferManager()
	{
	}

	FramebufferManager::~FramebufferManager()
	{
		//release all opengl resources managed by the manager
		for (auto it = fb_list_.begin();
			it != fb_list_.end(); ++it)
			delete *it;
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
			delete *it;
	}

	Framebuffer* FramebufferManager::framebuffer(
		FBType type, int nx, int ny, int ap,
		const std::string &name)
	{
		if (name != "")
		{
			for (auto it = fb_list_.begin();
				it != fb_list_.end(); ++it)
			{
				if ((*it)->match(name))
				{
					//size may not match
					//how to manage size change more efficiently needs further consideration
					if (!(*it)->match_size(nx, ny))
						(*it)->resize(nx, ny);
					return *it;
				}
			}
		}
		for (auto it = fb_list_.begin();
			it != fb_list_.end(); ++it)
		{
			if ((*it)->match(type, ap))
			{
				//size may not match
				//how to manage size change more efficiently needs further consideration
				if (!(*it)->match_size(nx, ny))
					(*it)->resize(nx, ny);
				(*it)->set_name(name);
				return *it;
			}
		}

		//create new framebuffer
		Framebuffer* fb = new Framebuffer(type, nx, ny, name);
		if (!fb->create())
			return 0;
		//add to lists
		fb_list_.push_back(fb);
		if (type == FB_Render_RGBA)
		{
			//create new texture
			FramebufferTexture* tex = new FramebufferTexture(FBTexType(type), nx, ny);
			if (!tex->create())
				return 0;
			//attach texture
			fb->attach_texture(ap, tex);
			//add to lists
			tex_list_.push_back(tex);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		fb->set_name(name);
		return fb;
	}

	Framebuffer* FramebufferManager::framebuffer(const std::string &name)
	{
		if (name == "")
			return 0;
		for (auto it = fb_list_.begin();
			it != fb_list_.end(); ++it)
		{
			if ((*it)->name_ == name)
				return *it;
		}
		return 0;
	}
}