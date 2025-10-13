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
#include <algorithm>

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
		glActiveTexture(GL_TEXTURE0);
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

	GLenum to_gl_attachment(const AttachmentPoint& ap) {
		switch (ap.type) {
			case AttachmentPoint::Type::Color:
				return GL_COLOR_ATTACHMENT0 + ap.index;
			case AttachmentPoint::Type::Depth:
				return GL_DEPTH_ATTACHMENT;
			case AttachmentPoint::Type::Stencil:
				return GL_STENCIL_ATTACHMENT;
			case AttachmentPoint::Type::DepthStencil:
				return GL_DEPTH_STENCIL_ATTACHMENT;
			default:
				throw std::invalid_argument("Unknown attachment type");
		}
	}

	Framebuffer::Framebuffer(const FBRole& role, int nx, int ny,
		const std::string &name):
		role_(role), nx_(nx), ny_(ny),
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

	void Framebuffer::bind(unsigned int id)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, id);
	}

	void Framebuffer::unbind(unsigned int prev_id)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, prev_id);
	}

	bool Framebuffer::attach_texture(const AttachmentPoint& ap, const std::shared_ptr<FramebufferTexture>& tex)
	{
		if (!valid_ || !tex || tex->id() == 0)
			return false;

		GLenum glap = to_gl_attachment(ap);
		GLint prevFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
		bind();

		switch (role_)
		{
		case FBRole::RenderFloat:
		case FBRole::RenderFloatMipmap:
		case FBRole::RenderUChar:
		case FBRole::Pick:
		case FBRole::Depth:
		default:
			glFramebufferTexture2D(GL_FRAMEBUFFER, glap, GL_TEXTURE_2D, tex->id(), 0);
			break;

		case FBRole::Volume:
			// This type should be handled by the second overload using tex_id + layer
			return false;
		}

		attachments_[glap] = tex;
		unbind(prevFramebuffer);

		return true;
	}

	bool Framebuffer::attach_texture(const AttachmentPoint& ap, unsigned int tex_id, int layer)
	{
		if (!valid_ || tex_id == 0)
			return false;

		GLenum glap = to_gl_attachment(ap);
		GLint prevFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
		bind();

		switch (role_)
		{
		case FBRole::RenderFloat:
		case FBRole::RenderFloatMipmap:
		case FBRole::RenderUChar:
		case FBRole::Pick:
		case FBRole::Depth:
		default:
			glFramebufferTexture(GL_FRAMEBUFFER, glap, tex_id, 0);
			break;

		case FBRole::Volume:
			glFramebufferTexture3D(GL_FRAMEBUFFER, glap, GL_TEXTURE_3D, tex_id, 0, layer);
			break;
		}

		attachments_.erase(glap); // External texture, not tracked via shared_ptr
		unbind(prevFramebuffer);

		return true;
	}

	void Framebuffer::detach_texture(const std::shared_ptr<FramebufferTexture>& tex)
	{
		if (!valid_ || !tex) return;

		GLint prevFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
		bind();

		for (auto it = attachments_.begin(); it != attachments_.end();)
		{
			if (it->second == tex)
			{
				glFramebufferTexture(GL_FRAMEBUFFER, it->first, 0, 0);
				it = attachments_.erase(it);
			}
			else
			{
				++it;
			}
		}

		unbind(prevFramebuffer);
	}

	void Framebuffer::detach_texture(const AttachmentPoint& ap)
	{
		if (!valid_) return;

		GLenum glap = to_gl_attachment(ap);
		GLint prevFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
		bind();

		glFramebufferTexture(GL_FRAMEBUFFER, glap, 0, 0);
		attachments_.erase(glap);

		unbind(prevFramebuffer);
	}

	void Framebuffer::bind_texture(const AttachmentPoint& ap, int tex_unit)
	{
		GLenum glap = to_gl_attachment(ap);
		auto it = attachments_.find(glap);
		if (it == attachments_.end() || !it->second || !it->second->valid())
			return;
		it->second->bind(tex_unit);
	}

	void Framebuffer::unbind_texture(const AttachmentPoint& ap)
	{
		GLenum glap = to_gl_attachment(ap);
		auto it = attachments_.find(glap);
		if (it == attachments_.end() || !it->second || !it->second->valid())
			return;
		it->second->unbind();
	}

	unsigned int Framebuffer::tex_id(const AttachmentPoint& ap)
	{
		GLenum glap = to_gl_attachment(ap);
		auto it = attachments_.find(glap);
		if (it == attachments_.end() || !it->second || !it->second->valid())
			return 0;
		return it->second->id();
	}

	void Framebuffer::resize(int nx, int ny)
	{
		for (auto& [ap, tex] : attachments_) {
			if (tex && tex->valid()) {
				tex->resize(nx, ny);
				tex->unbind(); // Ensures clean state per texture
			}
		}
		nx_ = nx;
		ny_ = ny;
	}

	void Framebuffer::generate_mipmap(const AttachmentPoint& ap)
	{
		if (role_ != FBRole::RenderFloatMipmap)
			return;

		GLenum glap = to_gl_attachment(ap);
		auto it = attachments_.find(glap);
		if (it == attachments_.end() || !it->second || !it->second->valid())
			return;
		it->second->bind(0);
		glGenerateMipmap(GL_TEXTURE_2D);
		it->second->unbind();
	}

	bool Framebuffer::match(const FBRole& role)
	{
		if (protected_)
			return false;
		if (role_ == role)
			return true;
		return false;
	}

	bool Framebuffer::match(const FBRole& role,
		int nx, int ny)
	{
		if (protected_)
			return false;
		if (role_ == role &&
			nx_ == nx &&
			ny_ == ny)
			return true;
		return false;
	}

	bool Framebuffer::match(const std::string &name)
	{
		if (name == "")
			return false;
		if (name_ == name)
			return true;
		return false;
	}

	unsigned int Framebuffer::read_pick(int px, int py)
	{
		if (!valid_ || attachments_.empty())
			return 0;
		if (px < 0 || px >= nx_ || py < 0 || py >= ny_)
			return 0;

		GLint prevFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);

		// Find the first attachment with Render_Int32 type
		for (const auto& [ap, tex] : attachments_)
		{
			if (tex && tex->valid() && tex->config_.type == FBTexType::Render_Int32)
			{
				bind();
				unsigned int value = 0;
				glReadPixels(px, py, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &value);
				unbind(prevFramebuffer);
				return value;
			}
		}

		return 0; // No suitable attachment found
	}

	bool Framebuffer::read(int x, int y, int width, int height,
		const AttachmentPoint& ap, GLenum format, GLenum type, void* data)
	{
		if (!valid_ || !data || width <= 0 || height <= 0)
			return false;

		GLenum glap = to_gl_attachment(ap);
		auto it = attachments_.find(glap);
		if (it == attachments_.end() || !it->second || !it->second->valid())
			return false;

		// Save current framebuffer binding
		GLint prevFramebuffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);

		// Bind this framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, id_);

		// Clamp read region to framebuffer size
		int rx = std::clamp(x, 0, nx_ - 1);
		int ry = std::clamp(y, 0, ny_ - 1);
		int rw = std::min(width, nx_ - rx);
		int rh = std::min(height, ny_ - ry);

		// Read pixels from the specified attachment
		glReadBuffer(glap); // Optional: if using multiple color attachments
		glReadPixels(rx, ry, rw, rh, format, type, data);

		// Restore previous framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, prevFramebuffer);

		return true;
	}

	bool Framebuffer::read_default(int x, int y, int width, int height,
								   GLenum format, GLenum type, void* data)
	{
		if (!data || width <= 0 || height <= 0)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glReadBuffer(GL_BACK);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(x, y, width, height, format, type, data);

		return true;
	}

	double Framebuffer::estimate_pick_threshold(
		int width, int height,
		const AttachmentPoint& ap,
		GLenum format, GLenum type,
		double scale)
	{
		if (!valid_ || width <= 0 || height <= 0)
			return 0.0;

		unsigned char pixel[4] = {};
		bool success = read(width / 2, height / 2, 1, 1,
							ap, format, type, pixel);

		if (!success)
			return 0.0;

		return scale * static_cast<double>(pixel[3]) / 255.0;
	}

	FramebufferManager::FramebufferManager()
	{
	}

	FramebufferManager::~FramebufferManager()
	{
	}

	void FramebufferManager::clear()
	{
		fb_list_.clear();
		tex_list_.clear();
	}

	std::shared_ptr<Framebuffer> FramebufferManager::framebuffer(
		const FBRole& role, int nx, int ny,
		const std::string& name)
	{
		std::shared_ptr<Framebuffer> fb;

		// First, try to match by name
		if (!name.empty())
		{
			for (const auto& candidate : fb_list_)
			{
				if (candidate->match(name))
				{
					fb = candidate;
					break;
				}
			}
		}

		// Then, try to match by role
		if (!fb)
		{
			for (const auto& candidate : fb_list_)
			{
				if (candidate->match(role))
				{
					fb = candidate;
					fb->set_name(name);
					break;
				}
			}
		}

		// If found, resize if needed
		if (fb)
		{
			if (!fb->match_size(nx, ny))
				fb->resize(nx, ny);
		}
		else
		{
			// Create new framebuffer
			fb = std::make_shared<Framebuffer>(role, nx, ny, name);
			fb->create();
			fb_list_.push_back(fb);

			// Create and attach textures based on role
			switch (role)
			{
			case FBRole::RenderFloat:
			{
				FBTexConfig config{ FBTexType::Render_RGBA };
				auto tex = std::make_shared<FramebufferTexture>(config, nx, ny);
				tex->create();
				fb->attach_texture(AttachmentPoint::Color(0), tex);
				tex_list_.push_back(tex);
				break;
			}
			case FBRole::RenderUChar:
			{
				FBTexConfig config{ FBTexType::UChar_RGBA };
				auto tex = std::make_shared<FramebufferTexture>(config, nx, ny);
				tex->create();
				fb->attach_texture(AttachmentPoint::Color(0), tex);
				tex_list_.push_back(tex);
				break;
			}
			case FBRole::RenderFloatMipmap:
			{
				FBTexConfig config{ FBTexType::Render_RGBA };
				config.useMipmap = true;
				config.minFilter = TexFilter::LinearMipmapLinear;
				auto tex = std::make_shared<FramebufferTexture>(config, nx, ny);
				tex->create();
				fb->attach_texture(AttachmentPoint::Color(0), tex);
				tex_list_.push_back(tex);
				break;
			}
			case FBRole::Pick:
			{
				FBTexConfig color_config{ FBTexType::Render_Int32 };
				auto tex_color = std::make_shared<FramebufferTexture>(color_config, nx, ny);
				tex_color->create();

				FBTexConfig depth_config{ FBTexType::Depth_Float };
				auto tex_depth = std::make_shared<FramebufferTexture>(depth_config, nx, ny);
				tex_depth->create();

				fb->attach_texture(AttachmentPoint::Color(0), tex_color);
				fb->attach_texture(AttachmentPoint::Depth(), tex_depth);
				tex_list_.push_back(tex_color);
				tex_list_.push_back(tex_depth);
				break;
			}
			case FBRole::Depth:
			{
				FBTexConfig config{ FBTexType::Depth_Float };
				auto tex = std::make_shared<FramebufferTexture>(config, nx, ny);
				tex->create();
				fb->attach_texture(AttachmentPoint::Depth(), tex);
				tex_list_.push_back(tex);
				break;
			}
			default:
				break;
			}
		}

		// Always bind before returning
		fb->set_name(name);

		return fb;
	}

	std::shared_ptr<Framebuffer> FramebufferManager::framebuffer(const std::string& name)
	{
		if (name.empty())
			return nullptr;

		for (const auto& fb : fb_list_)
		{
			if (fb && fb->name_ == name)
			{
				return fb;
			}
		}

		return nullptr;
	}

	void FramebufferManager::bind(std::shared_ptr<Framebuffer> fb)
	{
		// Skip if already bound
		if (current_bound_.lock() == fb)
			return;

		// Save current as previous
		previous_bound_ = current_bound_;

		if (fb)
			fb->bind(); // private, accessed via friendship
		else
			Framebuffer::bind(0); // bind default

		current_bound_ = fb;
	}

	void FramebufferManager::unbind()
	{
		auto prev = previous_bound_.lock();

		if (prev)
			prev->bind(); // restore previous framebuffer
		else
			Framebuffer::bind(0); // bind default

		current_bound_ = prev;
		previous_bound_.reset();
	}

	std::shared_ptr<Framebuffer> FramebufferManager::current() const
	{
		return current_bound_.lock(); // nullptr if expired
	}

}