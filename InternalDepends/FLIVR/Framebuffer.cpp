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

	void FramebufferTexture::create()
	{
		glGenTextures(1, &id_);
		//if (!glIsTexture(id_))
		//	return false;
		glBindTexture(GL_TEXTURE_2D, id_);
		switch (type_)
		{
		case FBTex_Render_RGBA:
		default:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
				GL_RGBA, GL_FLOAT, NULL);//GL_RGBA16F
			break;
		case FBTex_UChar_RGBA:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nx_, ny_, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			break;
		case FBTex_Render_Int32:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, nx_, ny_, 0,
				GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
			break;
		case FBTex_Depth_Float:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, nx_, ny_, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			break;
		}
		valid_ = true;
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

	void Framebuffer::create()
	{
		glGenFramebuffers(1, &id_);
		//if (!glIsFramebuffer(id_))
		//	return false;
		valid_ = true;
	}

	void Framebuffer::destroy()
	{
		glDeleteFramebuffers(1, &id_);
		id_ = 0;
		valid_ = false;
		protected_ = false;
		name_ = "";
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

	void FramebufferManager::clear()
	{
		for (auto it = fb_list_.begin();
			it != fb_list_.end(); ++it)
			delete *it;
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
			delete *it;
		fb_list_.clear();
		tex_list_.clear();
	}

	Framebuffer* FramebufferManager::framebuffer(
		FBType type, int nx, int ny,
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
			if ((*it)->match(type))
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
		fb->create();
		//add to lists
		fb_list_.push_back(fb);

		switch (type)
		{
		case FB_Render_RGBA:
			{
				//create new texture
				FramebufferTexture* tex =
					new FramebufferTexture(FBTex_Render_RGBA, nx, ny);
				tex->create();
				//attach texture
				fb->attach_texture(GL_COLOR_ATTACHMENT0, tex);
				//add to lists
				tex_list_.push_back(tex);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			break;
		case FB_UChar_RGBA:
			{
				//create new texture
				FramebufferTexture* tex =
					new FramebufferTexture(FBTex_UChar_RGBA, nx, ny);
				tex->create();
				//attach texture
				fb->attach_texture(GL_COLOR_ATTACHMENT0, tex);
				//add to lists
				tex_list_.push_back(tex);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			break;
		case FB_Pick_Int32_Float:
			{
				FramebufferTexture* tex_color =
					new FramebufferTexture(FBTex_Render_Int32, nx, ny);
				tex_color->create();
				FramebufferTexture* tex_depth =
					new FramebufferTexture(FBTex_Depth_Float, nx, ny);
				tex_depth->create();
				//attach textures
				fb->attach_texture(GL_COLOR_ATTACHMENT0, tex_color);
				fb->attach_texture(GL_DEPTH_ATTACHMENT, tex_depth);
				//add to lists
				tex_list_.push_back(tex_color);
				tex_list_.push_back(tex_depth);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			break;
		case FB_Depth_Float:
			{
				//create new texture
				FramebufferTexture* tex =
					new FramebufferTexture(FBTex_Depth_Float, nx, ny);
				tex->create();
				//attach texture
				fb->attach_texture(GL_DEPTH_ATTACHMENT, tex);
				//add to lists
				tex_list_.push_back(tex);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			break;
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

	bool Framebuffer::attach_texture(int ap, FramebufferTexture* tex)
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

	void Framebuffer::bind_texture(int ap)
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
		{
			if ((*it).first == ap)
				(*it).second->bind();
		}
	}

	unsigned int Framebuffer::tex_id(int ap)
	{
		for (auto it = tex_list_.begin();
			it != tex_list_.end(); ++it)
		{
			if ((*it).first == ap)
				return (*it).second->id_;
		}
		return 0;
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
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	bool Framebuffer::match(FBType type)
	{
		if (protected_)
			return false;
		if (type_ == type)
			return true;
		return false;
	}

	bool Framebuffer::match(FBType type,
		int nx, int ny)
	{
		if (protected_)
			return false;
		if (type_ == type &&
			nx_ == nx &&
			ny_ == ny)
			return true;
		return false;
	}

}