//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <GL/glew.h>
#include <Framebuffer.h>

namespace flvr
{
	FramebufferTexture::FramebufferTexture(const FBTexConfig& config, int nx, int ny):
		config_(config), nx_(nx), ny_(ny)
	{
	}

	FramebufferTexture::~FramebufferTexture()
	{
		destroy();
	}

	void FramebufferTexture::create()
	{
		glGenTextures(1, &id_);
		glBindTexture(GL_TEXTURE_2D, id_);

		// Scoped lambdas for enum translation
		auto toGLFilter = [](TexFilter f) -> GLint {
			switch (f) {
			case TexFilter::Nearest: return GL_NEAREST;
			case TexFilter::Linear: return GL_LINEAR;
			case TexFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
			default: return GL_LINEAR;
			}
			};

		auto toGLWrap = [](TexWrap w) -> GLint {
			switch (w) {
			case TexWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
			case TexWrap::Repeat: return GL_REPEAT;
			case TexWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
			default: return GL_CLAMP_TO_EDGE;
			}
			};

		// Apply filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(config_.minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(config_.magFilter));

		// Apply wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(config_.wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(config_.wrapT));

		// Allocate texture storage based on type
		switch (config_.type)
		{
		case FBTexType::Render_RGBA:
		default:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
				GL_RGBA, GL_FLOAT, nullptr);
			break;

		case FBTexType::UChar_RGBA:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nx_, ny_, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			break;

		case FBTexType::Render_Int32:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, nx_, ny_, 0,
				GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
			break;

		case FBTexType::Depth_Float:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, nx_, ny_, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			break;
		}

		// Generate mipmaps if requested
		if (config_.useMipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		valid_ = true;
	}

	void FramebufferTexture::destroy()
	{
		if (!valid_ || id_ == 0)
			return;
		glDeleteTextures(1, &id_);
		id_ = 0;
		valid_ = false;
	}

	bool FramebufferTexture::bind(int tex_unit)
	{
		if (!valid_ || id_ == 0) return false;

		glActiveTexture(GL_TEXTURE0 + tex_unit);
		glBindTexture(GL_TEXTURE_2D, id_);
		tex_unit_ = tex_unit;
		return true;
	}

	void FramebufferTexture::unbind()
	{
		if (tex_unit_ < 0) return; // Not bound

		glActiveTexture(GL_TEXTURE0 + tex_unit_);
		glBindTexture(GL_TEXTURE_2D, 0);
		tex_unit_ = -1;
	}

	void FramebufferTexture::resize(int nx, int ny)
	{
		if (!valid_ || id_ == 0) return;

		nx_ = nx;
		ny_ = ny;

		glBindTexture(GL_TEXTURE_2D, id_);

		// Scoped lambdas for enum translation
		auto toGLFilter = [](TexFilter f) -> GLint {
			switch (f) {
			case TexFilter::Nearest: return GL_NEAREST;
			case TexFilter::Linear: return GL_LINEAR;
			case TexFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
			default: return GL_LINEAR;
			}
			};

		auto toGLWrap = [](TexWrap w) -> GLint {
			switch (w) {
			case TexWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
			case TexWrap::Repeat: return GL_REPEAT;
			case TexWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
			default: return GL_CLAMP_TO_EDGE;
			}
			};

		// Reapply filtering and wrapping (in case driver resets on reallocation)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(config_.minFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(config_.magFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(config_.wrapS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(config_.wrapT));

		// Reallocate texture storage
		switch (config_.type)
		{
		case FBTexType::Render_RGBA:
		default:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx_, ny_, 0,
				GL_RGBA, GL_FLOAT, nullptr);
			break;

		case FBTexType::UChar_RGBA:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nx_, ny_, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			break;

		case FBTexType::Render_Int32:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, nx_, ny_, 0,
				GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
			break;

		case FBTexType::Depth_Float:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, nx_, ny_, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			break;
		}

		// Regenerate mipmaps if needed
		if (config_.useMipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	Framebuffer::Framebuffer(const FBTexConfig& config, int nx, int ny,
		const std::string &name):
		config_(config), nx_(nx), ny_(ny),
		name_(name)
	{
	}

	Framebuffer::~Framebuffer()
	{
		destroy();
	}

	void Framebuffer::create()
	{
		if (valid_ && id_ != 0)
			return;
		glGenFramebuffers(1, &id_);
		valid_ = true;
	}

	void Framebuffer::destroy()
	{
		if (!valid_ || id_ == 0)
			return;
		glDeleteFramebuffers(1, &id_);
		id_ = 0;
		valid_ = false;
		protected_ = false;
		name_ = "";
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

	bool Framebuffer::attach_texture(int ap, const std::shared_ptr<FramebufferTexture>& tex)
	{
		if (!valid_ || !tex || tex->id() == 0)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, id_);

		switch (config_.type)
		{
		case FBTexType::Render_RGBA:
		case FBTexType::UChar_RGBA:
		case FBTexType::Render_Int32:
		case FBTexType::Depth_Float:
		default:
			glFramebufferTexture2D(GL_FRAMEBUFFER, ap, GL_TEXTURE_2D, tex->id(), 0);
			break;

		case FBTexType::Ext_3D:
			// This type should be handled by the second overload using tex_id + layer
			return false;
		}

		attachments_[ap] = tex;
		return true;
	}

	bool Framebuffer::attach_texture(int ap, unsigned int tex_id, int layer)
	{
		if (!valid_ || tex_id == 0)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, id_);

		switch (config_.type)
		{
		case FBTexType::Render_RGBA:
		case FBTexType::UChar_RGBA:
		case FBTexType::Render_Int32:
		case FBTexType::Depth_Float:
		default:
			glFramebufferTexture(GL_FRAMEBUFFER, ap, tex_id, 0);
			break;

		case FBTexType::Ext_3D:
			glFramebufferTexture3D(GL_FRAMEBUFFER, ap, GL_TEXTURE_3D, tex_id, 0, layer);
			break;
		}

		attachments_.erase(ap); // External texture, not tracked via shared_ptr
		return true;
	}

	void Framebuffer::detach_texture(const std::shared_ptr<FramebufferTexture>& tex)
	{
		if (!valid_ || !tex) return;

		for (auto it = attachments_.begin(); it != attachments_.end();)
		{
			if (it->second == tex)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, id_);
				glFramebufferTexture(GL_FRAMEBUFFER, it->first, 0, 0);
				it = attachments_.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void Framebuffer::detach_texture(int ap)
	{
		if (!valid_) return;

		glBindFramebuffer(GL_FRAMEBUFFER, id_);
		glFramebufferTexture(GL_FRAMEBUFFER, ap, 0, 0);
		attachments_.erase(ap);
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

	unsigned int Framebuffer::id()
	{
		return id_;
	}

	bool Framebuffer::match_size(int nx, int ny)
	{
		return (nx == nx_) && (ny == ny_);
	}

	bool Framebuffer::match(const std::string &name)
	{
		if (name == "")
			return false;
		if (name_ == name)
			return true;
		return false;
	}

	void Framebuffer::set_name(const std::string &name)
	{
		name_ = name;
	}

	void Framebuffer::clear_name()
	{
		name_ = "";
	}

	unsigned int Framebuffer::read_value(int px, int py)
	{
		if (type_ != FB_Pick_Int32_Float)
			return 0;
		unsigned int value = 0;
		glReadPixels(px, py, 1, 1, GL_RED_INTEGER,
			GL_UNSIGNED_INT, (GLvoid*)&value);
		return value;
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

}