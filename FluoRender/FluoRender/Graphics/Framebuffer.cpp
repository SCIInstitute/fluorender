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

	case FBTexType::Render_Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, nx_, ny_, 0,
			GL_RED, GL_FLOAT, nullptr);
		break;

	case FBTexType::Render_FloatRG:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, nx_, ny_, 0,
			GL_RG, GL_FLOAT, nullptr);
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

	case FBTexType::Render_Float:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, nx_, ny_, 0,
			GL_RED, GL_FLOAT, nullptr);
		break;

	case FBTexType::Render_FloatRG:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, nx_, ny_, 0,
			GL_RG, GL_FLOAT, nullptr);
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
	name_ = "";
}

void Framebuffer::apply_state()
{
	glbin_fb_state_tracker.apply(state_);
}

//blend
void Framebuffer::set_blend_enabled_all(bool val)
{
	for (auto& [index, bs] : state_.blendStates)
		bs.enabled = val;
}

void Framebuffer::set_color_mask_all(bool r, bool g, bool b, bool a)
{
	for (auto& [index, bs] : state_.blendStates)
	{
		bs.maskRed = r;
		bs.maskGreen = g;
		bs.maskBlue = b;
		bs.maskAlpha = a;
	}
}

void Framebuffer::set_blend_func_all(BlendFactor src_rgb, BlendFactor dst_rgb, BlendFactor src_alpha, BlendFactor dst_alpha)
{
	for (auto& [index, bs] : state_.blendStates)
	{
		bs.srcRGB = src_rgb;
		bs.dstRGB = dst_rgb;
		bs.srcAlpha = src_alpha;
		bs.dstAlpha = dst_alpha;
	}
}

void Framebuffer::set_blend_equation_all(BlendEquation rgb, BlendEquation alpha)
{
	for (auto& [index, bs] : state_.blendStates)
	{
		bs.eqRGB = rgb;
		bs.eqAlpha = alpha;
	}
}

void Framebuffer::set_blend_enabled(int index, bool val)
{
	auto it = state_.blendStates.find(index);
	if (it != state_.blendStates.end() && it->second.enabled != val)
		it->second.enabled = val;
}

void Framebuffer::set_color_mask(int index, bool r, bool g, bool b, bool a)
{
	auto it = state_.blendStates.find(index);
	if (it != state_.blendStates.end() &&
		(it->second.maskRed != r ||
		 it->second.maskGreen != g ||
		 it->second.maskBlue != b ||
		 it->second.maskAlpha != a))
	{
		it->second.maskRed = r;
		it->second.maskGreen = g;
		it->second.maskBlue = b;
		it->second.maskAlpha = a;
	}
}

void Framebuffer::set_blend_func(int index, BlendFactor src_rgb, BlendFactor dst_rgb, BlendFactor src_alpha, BlendFactor dst_alpha)
{
	auto it = state_.blendStates.find(index);
	if (it != state_.blendStates.end() &&
		(it->second.srcRGB != src_rgb ||
		 it->second.dstRGB != dst_rgb ||
		 it->second.srcAlpha != src_alpha ||
		 it->second.dstAlpha != dst_alpha))
	{
		it->second.srcRGB = src_rgb;
		it->second.dstRGB = dst_rgb;
		it->second.srcAlpha = src_alpha;
		it->second.dstAlpha = dst_alpha;
	}
}

void Framebuffer::set_blend_equation(int index, BlendEquation rgb, BlendEquation alpha)
{
	auto it = state_.blendStates.find(index);
	if (it != state_.blendStates.end() &&
		(it->second.eqRGB != rgb || it->second.eqAlpha != alpha))
	{
		it->second.eqRGB = rgb;
		it->second.eqAlpha = alpha;
	}
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

void Framebuffer::clear_base(bool color, bool depth)
{
	GLbitfield mask = 0;
	if (color)   mask |= GL_COLOR_BUFFER_BIT;
	if (depth)   mask |= GL_DEPTH_BUFFER_BIT;
	if (mask)    glClear(mask);
}

void Framebuffer::clear_attachment(const AttachmentPoint& ap, const float* value)
{
	// For color attachments
	if (ap.type == AttachmentPoint::Type::Color)
	{
		glClearBufferfv(GL_COLOR, ap.index, value);
	}
	// For depth
	else if (ap.type == AttachmentPoint::Type::Depth) {
		glClearBufferfv(GL_DEPTH, 0, value);
	}
}

void Framebuffer::bind()
{
	if (valid_)
	{
		//DBGPRINT(L"Binding framebuffer: %d, %s\n", id_, s2ws(name_).c_str());

		glBindFramebuffer(GL_FRAMEBUFFER, id_);
		apply_state();
	}
}

void Framebuffer::unbind(unsigned int prev_id)
{
	//DBGPRINT(L"Unbinding framebuffer: %d, %s\n", id_, s2ws(name_).c_str());

	glBindFramebuffer(GL_FRAMEBUFFER, prev_id);
	//restore_state();
}

constexpr size_t kMaxStateStackDepth = 10;

FramebufferState Framebuffer::default_state()
{
	FramebufferState s;
	BlendState bs;
	s.blendStates[0] = bs;

	switch (role_)
	{
	case FBRole::RenderColorMipmap:
	case FBRole::RenderUChar:
		s.blendStates[0].enabled = true;
		break;

	case FBRole::RenderColorFxDepth:
		s.blendStates[0].enabled = true;
		s.blendStates[1].enabled = false;
		s.enableDepthTest = true;
		break;

	case FBRole::RenderColorFxFilter:
		s.blendStates[0].enabled = true;
		s.blendStates[1].enabled = true;
		break;

	case FBRole::Pick:
		s.enableDepthTest = true;
		s.enableScissorTest = true;
		break;

	case FBRole::Depth:
		s.enableDepthTest = true;
		break;

	//case FBRole::RenderColor:
	//case FBRole::Canvas:
	//case FBRole::Volume:
	//	s.enableBlend = false;
	//	break;
	}

	s.viewport = { 0, 0, nx_, ny_ };

	return s;
}

void Framebuffer::push_state()
{
	assert(state_stack_.size() < 5);
	state_stack_.push_back(state_);
	//DBGPRINT(L"Framebuffer stack size : %zu\n", state_stack_.size());
}

void Framebuffer::pop_state()
{
	assert(state_stack_.size() > 0);
	state_stack_.pop_back();
	//DBGPRINT(L"Framebuffer stack size : %zu\n", state_stack_.size());
}

void Framebuffer::apply_draw_mask(const std::unordered_map<int, bool>& drawEnabled)
{
	GLint prevFramebuffer = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
	bind();

	std::vector<GLenum> drawBuffers;
	for (const auto& kv : drawEnabled)
	{
		int index = kv.first;
		bool enabled = kv.second;
		if (enabled)
		{
			drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + index);
		}
	}

	if (!drawBuffers.empty())
	{
		glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	}
	else
	{
		// Disable all color writes if nothing is enabled
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	unbind(prevFramebuffer);
}

bool Framebuffer::set_draw_enabled(const AttachmentPoint& ap, bool enabled)
{
	if (role_ == FBRole::Canvas || !valid_)
		return false;

	GLint prevFramebuffer = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
	bind();

	// Update state
	state_.drawEnabled[ap.index] = enabled;

	// Rebuild draw buffer list from state
	std::vector<GLenum> drawBuffers;
	for (const auto& rec : attachments_) {
		if (rec.point.type == AttachmentPoint::Type::Color &&
			state_.drawEnabled[rec.point.index]) {
			GLenum glAttachment = GL_COLOR_ATTACHMENT0 + rec.point.index;
			drawBuffers.push_back(glAttachment);
		}
	}

	if (!drawBuffers.empty()) {
		glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	}
	else {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	unbind(prevFramebuffer);
	return true;
}

bool Framebuffer::attach_texture(const AttachmentPoint& ap,
	const std::shared_ptr<FramebufferTexture>& tex)
{
	if (role_ == FBRole::Canvas)
		return false;
	if (!valid_ || !tex || tex->id() == 0)
		return false;

	GLenum glap = to_gl_attachment(ap);
	GLint prevFramebuffer = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
	bind();

	// Attach texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, glap, GL_TEXTURE_2D, tex->id(), 0);

	// Replace or add record
	auto it = std::find_if(attachments_.begin(), attachments_.end(),
		[&](const AttachmentRecord& rec) { return rec.point == ap; });
	if (it != attachments_.end()) {
		it->texture = tex;
		it->config = tex->config_;
	}
	else {
		attachments_.push_back({ ap, tex, tex->config_ });
	}

	// Sync state: mark enabled
	if (ap.type == AttachmentPoint::Type::Color) {
		state_.drawEnabled[ap.index] = true;
	}

	// Rebuild draw buffer list from state
	std::vector<GLenum> drawBuffers;
	for (const auto& rec : attachments_) {
		if (rec.point.type == AttachmentPoint::Type::Color &&
			state_.drawEnabled[rec.point.index]) {
			GLenum glAttachment = GL_COLOR_ATTACHMENT0 + rec.point.index;
			drawBuffers.push_back(glAttachment);
		}
	}

	if (!drawBuffers.empty()) {
		glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
	}
	else {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

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
	case FBRole::RenderColor:
	case FBRole::RenderColorFxDepth:
	case FBRole::RenderColorFxFilter:
	case FBRole::RenderColorMipmap:
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

	unbind(prevFramebuffer);

	return true;
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

	attachments_.erase(
		std::remove_if(attachments_.begin(), attachments_.end(),
			[&](const AttachmentRecord& rec) { return rec.point == ap; }),
		attachments_.end());

	unbind(prevFramebuffer);
}

void Framebuffer::bind_texture(const AttachmentPoint& ap, int tex_unit)
{
	if (role_ == FBRole::Canvas)
		return;

	for (const auto& rec : attachments_) {
		if (rec.point == ap && rec.texture && rec.texture->valid()) {
			rec.texture->bind(tex_unit);
			return;
		}
	}
}

void Framebuffer::unbind_texture(const AttachmentPoint& ap)
{
	if (role_ == FBRole::Canvas)
		return;

	for (const auto& rec : attachments_) {
		if (rec.point == ap && rec.texture && rec.texture->valid()) {
			rec.texture->unbind();
			return;
		}
	}
}

unsigned int Framebuffer::tex_id(const AttachmentPoint& ap)
{
	if (role_ == FBRole::Canvas)
		return 0;

	for (const auto& rec : attachments_) {
		if (rec.point == ap && rec.texture && rec.texture->valid()) {
			return rec.texture->id();
		}
	}
	return 0;
}

std::shared_ptr<FramebufferTexture> Framebuffer::get_texture(const AttachmentPoint& ap)
{
	if (role_ == FBRole::Canvas)
		return nullptr;

	for (const auto& rec : attachments_) {
		if (rec.point == ap && rec.texture && rec.texture->valid()) {
			return rec.texture;
		}
	}
	return nullptr;
}

void Framebuffer::resize(int nx, int ny)
{
	nx_ = nx;
	ny_ = ny;

	set_viewport({ 0, 0, nx_, ny_ });

	if (role_ == FBRole::Canvas)
		return;

	for (const auto& rec : attachments_) {
		if (rec.texture && rec.texture->valid()) {
			rec.texture->resize(nx, ny);
			rec.texture->unbind(); // Ensures clean state per texture
		}
	}
}

void Framebuffer::generate_mipmap(const AttachmentPoint& ap)
{
	if (role_ != FBRole::RenderColorMipmap)
		return;

	for (const auto& rec : attachments_) {
		if (rec.point == ap && rec.texture && rec.texture->valid()) {
			rec.texture->bind(0);
			glGenerateMipmap(GL_TEXTURE_2D);
			rec.texture->unbind();
			return;
		}
	}
}

bool Framebuffer::match(const FBRole& role)
{
	if (role_ == role)
		return true;
	return false;
}

bool Framebuffer::match(const FBRole& role,
	int nx, int ny)
{
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
	for (const auto& rec : attachments_) {
		if (rec.texture &&
			rec.texture->valid() &&
			rec.texture->config_.type == FBTexType::Render_Int32)
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

	std::shared_ptr<FramebufferTexture> texture;
	for (const auto& rec : attachments_) {
		if (rec.point == ap && rec.texture && rec.texture->valid()) {
			texture = rec.texture;
			break;
		}
	}
	if (!texture)
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
	glReadBuffer(to_gl_attachment(ap)); // Optional: if using multiple color attachments
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

FramebufferStateGuard::FramebufferStateGuard(Framebuffer& fb) :
	fb_(fb)
{
	//save current global state
	glbin_fb_state_tracker.push_state();
	//save framebuffer state
	fb_.push_state();
}

FramebufferStateGuard::~FramebufferStateGuard()
{
	assert(glbin_fb_state_tracker.state_stack_.size() > 0);
	assert(fb_.state_stack_.size() > 0);

	// restore global state
	const auto& globalState = glbin_fb_state_tracker.state_stack_.back();
	glbin_fb_state_tracker.apply(globalState);

	// restore framebuffer state
	const auto& fbState = fb_.state_stack_.back();
	// restore draw buffer mask if needed
	if (fb_.state_.drawEnabled != fbState.drawEnabled) {
		fb_.apply_draw_mask(fbState.drawEnabled);
	}

	fb_.state_ = fbState;

	// pop stacks
	glbin_fb_state_tracker.pop_state();
	fb_.pop_state();
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

using AttachmentLayout = std::vector<AttachmentSpec>;

static const AttachmentLayout& layout_for(FBRole role)
{
	static const FBTexConfig RGBA{ FBTexType::Render_RGBA };
	static const FBTexConfig RGBAFilter{ FBTexType::Render_RGBA, false, TexFilter::Linear, TexFilter::Linear };
	static const FBTexConfig Float{ FBTexType::Render_Float };
	static const FBTexConfig FloatRG{ FBTexType::Render_FloatRG };
	static const FBTexConfig DepthFloat{ FBTexType::Depth_Float };
	static const FBTexConfig UChar{ FBTexType::UChar_RGBA };
	static const FBTexConfig RGBAMipmap{ FBTexType::Render_RGBA, true, TexFilter::LinearMipmapLinear, TexFilter::Linear };
	static const FBTexConfig Int32{ FBTexType::Render_Int32 };

	switch (role) {
	case FBRole::RenderColor:
	{
		static const AttachmentLayout rc{
			{ AttachmentPoint::Color(0), RGBA }
		};
		return rc;
	}

	case FBRole::RenderColorFilter:
	{
		static const AttachmentLayout rcf{
			{ AttachmentPoint::Color(0), RGBAFilter }
		};
		return rcf;
	}

	case FBRole::RenderColorFxDepth:
	{
		static const AttachmentLayout rcfxd{
			{ AttachmentPoint::Color(0), RGBA },
			{ AttachmentPoint::Color(1), Float },
			{ AttachmentPoint::Depth(), DepthFloat }
		};
		return rcfxd;
	}

	case FBRole::RenderColorFxFilter:
	{
		static const AttachmentLayout rcfx{
			{ AttachmentPoint::Color(0), RGBAFilter },
			{ AttachmentPoint::Color(1), FloatRG }
		};
		return rcfx;
	}

	case FBRole::RenderUChar:
	{
		static const AttachmentLayout ruc{
			{ AttachmentPoint::Color(0), UChar }
		};
		return ruc;
	}

	case FBRole::RenderColorMipmap:
	{
		static const AttachmentLayout rcm{
			{ AttachmentPoint::Color(0), RGBAMipmap }
		};
		return rcm;
	}

	case FBRole::Pick:
	{
		static const AttachmentLayout rp{
			{ AttachmentPoint::Color(0), Int32 },
			{ AttachmentPoint::Depth(), DepthFloat }
		};
		return rp;
	}

	case FBRole::Depth:
	{
		static const AttachmentLayout rd{
			{ AttachmentPoint::Depth(), DepthFloat }
		};
		return rd;
	}

	default:
	{
		static const AttachmentLayout empty{};
		return empty;
	}
	}
}

std::shared_ptr<Framebuffer> FramebufferFactory::framebuffer(
	const FBRole& role, int nx, int ny, const std::string& name)
{
	std::shared_ptr<Framebuffer> fb;

	if (!name.empty()) {
		for (const auto& candidate : fb_list_) {
			if (candidate->match(name)) { fb = candidate; break; }
		}
	}
	else {
		for (const auto& candidate : fb_list_) {
			if (candidate->match(role)) { fb = candidate; candidate->set_name(name); break; }
		}
	}

	if (!fb) {
		fb = std::make_shared<Framebuffer>(role, nx, ny, name);
		fb->create();
		fb_list_.push_back(fb);
	}

	assert(fb);

	// Always reconcile to requested role and size
	ensure_layout(fb, role, nx, ny);

	//resize if needed
	if (!fb->match_size(nx, ny))
		fb->resize(nx, ny);

	// Keep name authoritative
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

void FramebufferFactory::bind(const std::shared_ptr<Framebuffer>& fb)
{
	auto cur = current_.lock();

	if (cur)
	{
		// Skip if already bound
		if (cur == fb)
		{
			//DBGPRINT(L"bind(); ERROR: Framebuffer already bound\n");
			fb->apply_state();
			return;
		}
	}

	if (fb)
	{
		fb->bind(); // private, accessed via friendship
	}

	current_ = fb;

	//DBGPRINT(L"bind(); Current Framebuffer: %d, %s\n", fb->id_, s2ws(fb->name_).c_str());
}

FramebufferFactory::Guard FramebufferFactory::bind_scoped(const std::shared_ptr<Framebuffer>& fb)
{
	return Guard(*this, fb);
}

std::shared_ptr<Framebuffer> FramebufferFactory::current() const
{
	return current_.lock(); // nullptr if expired
}

void FramebufferFactory::ensure_layout(std::shared_ptr<Framebuffer>& fb,
	FBRole role, int nx, int ny)
{
	const auto& layout = layout_for(role);

	for (const auto& spec : layout) {
		auto existing = fb->get_texture(spec.point);
		if (!existing || !existing->match_config(spec.config)) {
			// detach old if present
			if (existing) fb->detach_texture(spec.point);

			// create new
			auto tex = std::make_shared<FramebufferTexture>(spec.config, nx, ny);
			tex->create();
			fb->attach_texture(spec.point, tex);
			tex_list_.push_back(tex);
		}
		else if (!existing->match_size(nx, ny)) {
			existing->resize(nx, ny);
		}
	}

	// detach anything not in layout
	for (auto& att : fb->attachments()) {
		bool keep = std::any_of(layout.begin(), layout.end(),
			[&](const AttachmentSpec& s) { return s.point == att.point; });
		if (!keep) fb->detach_texture(att.point);
	}

	fb->set_role(role);
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

void FramebufferManager::bind(const std::shared_ptr<Framebuffer>& fb)
{
	int view_id = glbin_current.GetDrawingViewId();
	auto& factory = factory_map_[view_id];
	if (!factory)
		factory = std::make_unique<FramebufferFactory>();
	factory->bind(fb);
}

FramebufferFactory::Guard FramebufferManager::bind_scoped(const std::shared_ptr<Framebuffer>& fb)
{
	int view_id = glbin_current.GetDrawingViewId();
	auto& factory = factory_map_[view_id];
	if (!factory)
		factory = std::make_unique<FramebufferFactory>();
	return factory->bind_scoped(fb);
}

std::shared_ptr<Framebuffer> FramebufferManager::current() const
{
	int view_id = glbin_current.GetDrawingViewId();
	auto it = factory_map_.find(view_id);
	if (it != factory_map_.end() && it->second)
		return it->second->current();
	return nullptr;
}
