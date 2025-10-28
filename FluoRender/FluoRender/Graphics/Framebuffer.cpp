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
#include <Global.h>
#include <CurrentObjects.h>
#include <FramebufferStateTracker.h>
#include <algorithm>
#include <compatibility.h>
#include <Debug.h>

using namespace flvr;

FramebufferTexture::FramebufferTexture(const FBTexConfig& config, int nx, int ny) :
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
	const std::string& name) :
	role_(role), nx_(nx), ny_(ny),
	name_(name)
{
	state_ = default_state();
}

Framebuffer::~Framebuffer()
{
	destroy();
}

void Framebuffer::create()
{
	if (valid_)
		return;
	if (role_ == FBRole::Canvas)
	{
		id_ = 0;
	}
	else
	{
		glGenFramebuffers(1, &id_);
	}
	valid_ = true;
}

void Framebuffer::destroy()
{
	if (!valid_)
		return;
	if (role_ != FBRole::Canvas)
		glDeleteFramebuffers(1, &id_);
	id_ = 0;
	valid_ = false;
	protected_ = false;
	name_ = "";
}

void Framebuffer::apply_state()
{
	glbin_fb_state_tracker.apply(state_);
}

//blend
void Framebuffer::set_blend_enabled(bool val)
{
	if (val == state_.enableBlend)
		return;
	state_.enableBlend = val;
}

void Framebuffer::set_blend_func(BlendFactor sfactor, BlendFactor dfactor)
{
	if (sfactor == state_.blendSrc && dfactor == state_.blendDst)
		return;
	state_.blendSrc = sfactor;
	state_.blendDst = dfactor;
}

void Framebuffer::set_blend_equation(BlendEquation rgb, BlendEquation alpha)
{
	if (rgb == state_.blendEquationRGB &&
		alpha == state_.blendEquationAlpha)
		return;
	state_.blendEquationRGB = rgb;
	state_.blendEquationAlpha = alpha;
}

//clear color
void Framebuffer::set_clear_color(const fluo::Vector4f& color)
{
	if (color == state_.clearColor)
		return;
	state_.clearColor = color;
}

//depth
void Framebuffer::set_depth_test_enabled(bool val)
{
	if (val == state_.enableDepthTest)
		return;
	state_.enableDepthTest = val;
}

void Framebuffer::set_depth_func(DepthFunc func)
{
	if (func == state_.depthFunc)
		return;
	state_.depthFunc = func;
}

void Framebuffer::set_clear_depth(float depth)
{
	if (depth == state_.clearDepth)
		return;
	state_.clearDepth = depth;
}

//cull
void Framebuffer::set_cull_face_enabled(bool val)
{
	if (val == state_.enableCullFace)
		return;
	state_.enableCullFace = val;
}

void Framebuffer::set_face_winding(FaceWinding winding)
{
	if (winding == state_.faceWinding)
		return;
	state_.faceWinding = winding;
}

void Framebuffer::set_cull_face(CullFace face)
{
	if (face == state_.cullFace)
		return;
	state_.cullFace = face;
}

//viewport
void Framebuffer::set_viewport(const fluo::Vector4i& vp)
{
	if (vp == state_.viewport)
		return;
	state_.viewport = vp;
	for (auto& it : state_stack_)
	{
		it.viewport = vp;
	}
}

//scissor
void Framebuffer::set_scissor_test_enabled(bool val)
{
	if (val == state_.enableScissorTest)
		return;
	state_.enableScissorTest = val;
}

void Framebuffer::set_scissor_rect(const fluo::Vector4i& rect)
{
	if (rect == state_.scissorRect)
		return;
	state_.scissorRect = rect;
}

void Framebuffer::set_polygon_mode(PolygonMode mode)
{
	if (mode == state_.polygonMode)
		return;
	state_.polygonMode = mode;
}

void Framebuffer::clear(bool color, bool depth)
{
	GLbitfield mask = 0;
	if (color)   mask |= GL_COLOR_BUFFER_BIT;
	if (depth)   mask |= GL_DEPTH_BUFFER_BIT;
	if (mask)    glClear(mask);
}

void Framebuffer::bind()
{
	if (valid_)
	{
		DBGPRINT(L"Binding framebuffer: %d, %s\n", id_, s2ws(name_).c_str());

		glBindFramebuffer(GL_FRAMEBUFFER, id_);
		apply_state();
	}
}

void Framebuffer::unbind(unsigned int prev_id)
{
	DBGPRINT(L"Unbinding framebuffer: %d, %s\n", id_, s2ws(name_).c_str());

	glBindFramebuffer(GL_FRAMEBUFFER, prev_id);
	restore_state();
}

constexpr size_t kMaxStateStackDepth = 10;

void Framebuffer::push_state()
{
	state_stack_.push_back(glbin_fb_state_tracker.current());
	if (state_stack_.size() > kMaxStateStackDepth)
	{
		DBGPRINT(L"[WARN] Framebuffer %d (%s) state stack depth exceeded: %zu\n",
			id_, s2ws(name_).c_str(), state_stack_.size());
		assert(state_stack_.size() <= kMaxStateStackDepth);
	}
}

void Framebuffer::pop_state()
{
	if (state_stack_.empty())
	{
		DBGPRINT(L"[ERROR] Framebuffer %d (%s) attempted to pop empty state stack\n",
			id_, s2ws(name_).c_str());
		assert(state_stack_.size() > 0);
		return;
	}
	state_stack_.pop_back();
}

void Framebuffer::restore_state()
{
	if (state_stack_.empty())
		return;
	const auto& s = state_stack_.back();
	glbin_fb_state_tracker.apply(s);
}

FramebufferState Framebuffer::default_state()
{
	FramebufferState s;

	switch (role_)
	{
	case FBRole::RenderFloat:
	case FBRole::RenderFloatMipmap:
	case FBRole::RenderUChar:
		s.enableBlend = true;
		break;

	case FBRole::RenderFloatDepth:
		s.enableBlend = true;
		s.enableDepthTest = true;
		break;

	case FBRole::Pick:
		s.enableBlend = false;
		s.enableDepthTest = true;
		s.enableScissorTest = true;
		break;

	case FBRole::Depth:
		s.enableBlend = false;
		s.enableDepthTest = true;
		break;

	case FBRole::Canvas:
	case FBRole::Volume:
		s.enableBlend = false;
		break;
	}

	s.viewport = { 0, 0, nx_, ny_ };

	return s;
}

bool Framebuffer::attach_texture(const AttachmentPoint& ap, const std::shared_ptr<FramebufferTexture>& tex)
{
	if (role_ == FBRole::Canvas)
		return false;
	if (!valid_ || !tex || tex->id() == 0)
		return false;

	GLenum glap = to_gl_attachment(ap);
	GLint prevFramebuffer = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
	bind();

	switch (role_)
	{
	case FBRole::RenderFloat:
	case FBRole::RenderFloatDepth:
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
	if (role_ == FBRole::Canvas)
		return false;
	if (!valid_ || tex_id == 0)
		return false;

	GLenum glap = to_gl_attachment(ap);
	GLint prevFramebuffer = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
	bind();

	switch (role_)
	{
	case FBRole::RenderFloat:
	case FBRole::RenderFloatDepth:
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
	if (role_ == FBRole::Canvas)
		return;
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
	if (role_ == FBRole::Canvas)
		return;
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
	if (role_ == FBRole::Canvas)
		return;
	GLenum glap = to_gl_attachment(ap);
	auto it = attachments_.find(glap);
	if (it == attachments_.end() || !it->second || !it->second->valid())
		return;
	it->second->bind(tex_unit);
}

void Framebuffer::unbind_texture(const AttachmentPoint& ap)
{
	if (role_ == FBRole::Canvas)
		return;
	GLenum glap = to_gl_attachment(ap);
	auto it = attachments_.find(glap);
	if (it == attachments_.end() || !it->second || !it->second->valid())
		return;
	it->second->unbind();
}

unsigned int Framebuffer::tex_id(const AttachmentPoint& ap)
{
	if (role_ == FBRole::Canvas)
		return 0;
	GLenum glap = to_gl_attachment(ap);
	auto it = attachments_.find(glap);
	if (it == attachments_.end() || !it->second || !it->second->valid())
		return 0;
	return it->second->id();
}

void Framebuffer::resize(int nx, int ny)
{
	nx_ = nx;
	ny_ = ny;

	set_viewport({ 0, 0, nx_, ny_ });

	if (role_ == FBRole::Canvas)
		return;

	for (auto& [ap, tex] : attachments_) {
		if (tex && tex->valid()) {
			tex->resize(nx, ny);
			tex->unbind(); // Ensures clean state per texture
		}
	}
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

bool Framebuffer::match(const std::string& name)
{
	if (name == "")
		return false;
	if (name_ == name)
		return true;
	return false;
}

unsigned int Framebuffer::read_pick(int px, int py)
{
	if (role_ == FBRole::Canvas)
		return 0;
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
	if (role_ == FBRole::Canvas)
		return false;
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

double Framebuffer::estimate_pick_threshold(
	int width, int height,
	const AttachmentPoint& ap,
	GLenum format, GLenum type,
	double scale)
{
	if (role_ == FBRole::Canvas)
		return 0.0;
	if (!valid_ || width <= 0 || height <= 0)
		return 0.0;

	unsigned char pixel[4] = {};
	bool success = read(width / 2, height / 2, 1, 1,
		ap, format, type, pixel);

	if (!success)
		return 0.0;

	return scale * static_cast<double>(pixel[3]) / 255.0;
}

FramebufferFactory::FramebufferFactory()
{
}

FramebufferFactory::~FramebufferFactory()
{
}

void FramebufferFactory::clear()
{
	fb_list_.clear();
	tex_list_.clear();
}

std::shared_ptr<Framebuffer> FramebufferFactory::framebuffer(
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
		case FBRole::Canvas:
		{
			// No attachments for default framebuffer
			break;
		}
		case FBRole::RenderFloat:
		{
			FBTexConfig config{ FBTexType::Render_RGBA };
			auto tex = std::make_shared<FramebufferTexture>(config, nx, ny);
			tex->create();
			fb->attach_texture(AttachmentPoint::Color(0), tex);
			tex_list_.push_back(tex);
			break;
		}
		case FBRole::RenderFloatDepth:
		{
			FBTexConfig color_config{ FBTexType::Render_RGBA };
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

	fb->set_name(name);

	return fb;
}

std::shared_ptr<Framebuffer> FramebufferFactory::framebuffer(const std::string& name)
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

void FramebufferFactory::bind(std::shared_ptr<Framebuffer> fb)
{
	auto cur = current_.lock();

	if (cur)
	{
		// Skip if already bound
		if (cur == fb)
		{
			//DBGPRINT(L"bind(); ERROR: Framebuffer already bound\n");
			fb->restore_state();
			return;
		}
		else
		{
			// Save current as previous
			stack_.push_back(cur);
		}
	}

	if (fb)
	{
		fb->bind(); // private, accessed via friendship
		fb->push_state();
	}

	current_ = fb;

	//DBGPRINT(L"bind(); Current Framebuffer: %d, %s\n", fb->id_, s2ws(fb->name_).c_str());
}

void FramebufferFactory::unbind()
{
	if (stack_.empty())
	{
		//DBGPRINT(L"unbind(); ERROR: Stack underflow\n");
		return;
	}

	auto prev = stack_.back().lock();
	stack_.pop_back();

	if (auto cur = current_.lock())
		cur->pop_state();

	if (prev)
	{
		prev->bind();

		current_ = prev;

		auto id = prev->id_;
		//DBGPRINT(L"unbind(); Current Framebuffer: %d, %s\n", prev->id_, s2ws(prev->name_).c_str());
	}
	else
	{
		//DBGPRINT(L"unbind(); ERROR: Previous framebuffer expired.\n");
	}
}

std::shared_ptr<Framebuffer> FramebufferFactory::current() const
{
	return current_.lock(); // nullptr if expired
}

std::shared_ptr<Framebuffer> FramebufferManager::framebuffer(
	const FBRole& role, int nx, int ny,
	const std::string& name)
{
	int view_id = glbin_current.GetDrawingViewId();
	auto& factory = factory_map_[view_id];
	if (!factory)
		factory = std::make_unique<FramebufferFactory>();
	return factory->framebuffer(role, nx, ny, name);
}

std::shared_ptr<Framebuffer> FramebufferManager::framebuffer(const std::string& name)
{
	int view_id = glbin_current.GetDrawingViewId();
	auto& factory = factory_map_[view_id];
	if (!factory)
		factory = std::make_unique<FramebufferFactory>();
	return factory->framebuffer(name);
}

void FramebufferManager::bind(std::shared_ptr<Framebuffer> fb)
{
	int view_id = glbin_current.GetDrawingViewId();
	auto& factory = factory_map_[view_id];
	if (!factory)
		factory = std::make_unique<FramebufferFactory>();
	factory->bind(fb);
}

void FramebufferManager::unbind()
{
	int view_id = glbin_current.GetDrawingViewId();
	auto& factory = factory_map_[view_id];
	if (factory)
		factory->unbind();
}

std::shared_ptr<Framebuffer> FramebufferManager::current() const
{
	int view_id = glbin_current.GetDrawingViewId();
	auto it = factory_map_.find(view_id);
	if (it != factory_map_.end() && it->second)
		return it->second->current();
	return nullptr;
}
