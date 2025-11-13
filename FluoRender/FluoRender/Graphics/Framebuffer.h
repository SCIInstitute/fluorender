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

#ifndef Framebuffer_h
#define Framebuffer_h

#include <FramebufferState.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#ifndef __glew_h__
typedef unsigned int GLenum;
typedef float GLfloat;
#endif

namespace flvr
{
	enum class FBTexType : int
	{
		Render_RGBA,
		UChar_RGBA,
		Render_Int32,
		Render_Float,
		Depth_Float,
		Ext_3D
	};
	enum class TexFilter : int {
		Nearest,
		Linear,
		LinearMipmapLinear,
		// Add more as needed
	};
	enum class TexWrap : int {
		ClampToEdge,
		Repeat,
		MirroredRepeat,
		// Add more as needed
	};
	struct FBTexConfig {
		FBTexType type;
		bool useMipmap = false;
		TexFilter minFilter = TexFilter::Nearest;
		TexFilter magFilter = TexFilter::Nearest;
		TexWrap wrapS = TexWrap::ClampToEdge;
		TexWrap wrapT = TexWrap::ClampToEdge;
	};
	inline bool operator==(const FBTexConfig& a, const FBTexConfig& b)
	{
		return a.type == b.type &&
				a.useMipmap == b.useMipmap &&
				a.minFilter == b.minFilter &&
				a.magFilter == b.magFilter &&
				a.wrapS == b.wrapS &&
				a.wrapT == b.wrapT;
	}
	inline bool operator!=(const FBTexConfig& a, const FBTexConfig& b)
	{
		return !(a == b);
	}
	class Framebuffer;
	class FramebufferFactory;
	class FramebufferStateGuard;
	class FramebufferTexture
	{
	public:
		FramebufferTexture(const FBTexConfig& config, int nx, int ny);
		~FramebufferTexture();

		void create();
		void destroy();
		bool bind(int tex_unit);
		void unbind();
		bool valid() { return valid_; }
		unsigned int id() { return id_; }
		void resize(int nx, int ny);

	private:
		unsigned int id_ = 0;
		FBTexConfig config_;
		int nx_ = 0;
		int ny_ = 0;
		bool valid_ = false;
		int tex_unit_ = -1;

		friend class Framebuffer;
	};

	enum class FBRole : int
	{
		Canvas,// Represents the default framebuffer (id == 0)
		RenderColor,
		RenderColorFxDepth,
		RenderColorFx,
		RenderColorFilter,
		RenderColorMipmap,
		RenderUChar,
		Pick,
		Depth,
		Volume
	};
	struct AttachmentPoint {
		enum class Type : int {
			Color,
			Depth,
			Stencil,
			DepthStencil
		};

		Type type;
		int index = 0; // Only used for Color

		static AttachmentPoint Color(int i) { return {Type::Color, i}; }
		static AttachmentPoint Depth() { return {Type::Depth}; }
		static AttachmentPoint Stencil() { return {Type::Stencil}; }
		static AttachmentPoint DepthStencil() { return {Type::DepthStencil}; }
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FBRole& role, int nx, int ny, const std::string &name);
		~Framebuffer();

		void create();
		void destroy();
		bool valid() { return valid_; }
		unsigned int id() { return id_; }

		//states
		void apply_state();

		//fine grained state control
		// Blend
		void set_blend_enabled(bool val);
		void set_blend_func(BlendFactor sfactor, BlendFactor dfactor);
		void set_blend_equation(BlendEquation rgb, BlendEquation alpha);
		// Depth
		void set_depth_test_enabled(bool val);
		void set_depth_func(DepthFunc func);
		void set_clear_depth(float depth);
		// Cull
		void set_cull_face_enabled(bool val);
		void set_face_winding(FaceWinding winding);
		void set_cull_face(CullFace face);
		//scissor
		void set_scissor_test_enabled(bool val);
		void set_scissor_rect(const fluo::Vector4i& rect);
		//viewport
		void set_viewport(const fluo::Vector4i& vp);
		//clear color
		void set_clear_color(const fluo::Vector4f& color);
		//polygon mode
		void set_polygon_mode(PolygonMode mode);
		//clear
		void clear(bool color, bool depth);
		void clear_attachment(const AttachmentPoint& ap, const float* value);

		bool attach_texture(const AttachmentPoint& ap, const std::shared_ptr<FramebufferTexture>& tex);
		bool attach_texture(const AttachmentPoint& ap, unsigned int tex_id, int layer=0);
		void detach_texture(const std::shared_ptr<FramebufferTexture>& tex);
		void detach_texture(const AttachmentPoint& ap);
		void bind_texture(const AttachmentPoint& ap, int tex_unit);
		void unbind_texture(const AttachmentPoint& ap);
		unsigned int tex_id(const AttachmentPoint& ap);

		void resize(int nx, int ny);

		//generate mipmap
		void generate_mipmap(const AttachmentPoint& ap);

		bool match_size(int nx, int ny) { return (nx == nx_) && (ny == ny_); }
		//match without size
		bool match(const FBRole& role);
		//match with size
		bool match(const FBRole& role, int, int);
		//match by name
		bool match(const std::string &name);

		//name represents its use
		void set_name(const std::string& name) { name_ = name; }
		void clear_name() { name_ = ""; }
		std::string get_name() { return name_; }

		//read pick value
		unsigned int read_pick(int, int);
		bool read(int x, int y, int width, int height,
			const AttachmentPoint& ap, GLenum format, GLenum type, void* data);
		double estimate_pick_threshold(int width, int height,
			const AttachmentPoint& ap, GLenum format, GLenum type, double scale = 0.8);

	private:
		unsigned int id_ = 0;
		FBRole role_;
		int nx_ = 0;
		int ny_ = 0;
		std::string name_;//specify its use
		bool valid_ = false;
		std::map<int, std::shared_ptr<FramebufferTexture>> attachments_;

		//states
		FramebufferState state_;
		std::vector<FramebufferState> state_stack_;

	private:
		void bind();
		void unbind(unsigned int prev_id = 0);

		FramebufferState default_state();
		void push_state();
		void pop_state();

		friend class FramebufferFactory;
		friend class FramebufferStateGuard;
	};

	class FramebufferStateGuard {
	public:
		explicit FramebufferStateGuard(Framebuffer& fb);
		~FramebufferStateGuard();
	private:
		Framebuffer& fb_;
	};

	class FramebufferFactory
	{
	public:
		FramebufferFactory();
		~FramebufferFactory();

		void clear();

		// Get or create framebuffer by role and size
		std::shared_ptr<Framebuffer> framebuffer(const FBRole& role, int nx, int ny,
			const std::string &name);

		// Get framebuffer by name (auto-binds if found)
		std::shared_ptr<Framebuffer> framebuffer(const std::string &name);

		// Explicitly bind a framebuffer (nullptr = default)
		void bind(const std::shared_ptr<Framebuffer>& fb);

		class Guard
		{
		public:
			Guard(FramebufferFactory& factory, const std::shared_ptr<Framebuffer>& fb) :
				factory_(factory),
				prev_fb_(factory.current())
			{
				factory_.bind(fb);
			}

			~Guard()
			{
				if (auto fb = prev_fb_.lock()) {
					factory_.bind(fb);
				}
			}

		private:
			FramebufferFactory& factory_;
			std::weak_ptr<Framebuffer> prev_fb_;
		};

		//scoped bind that automatically unbind and restore previous
		[[nodiscard]] Guard bind_scoped(const std::shared_ptr<Framebuffer>& fb);

		// Get currently bound framebuffer (nullptr = default)
		std::shared_ptr<Framebuffer> current() const;

	private:
		std::vector<std::shared_ptr<Framebuffer>> fb_list_;
		std::vector<std::shared_ptr<FramebufferTexture>> tex_list_;

		std::weak_ptr<Framebuffer> current_;
	};

	class FramebufferManager
	{
	public:
		// Get or create framebuffer by role and size
		std::shared_ptr<Framebuffer> framebuffer(const FBRole& role, int nx, int ny,
			const std::string &name);

		// Get framebuffer by name (auto-binds if found)
		std::shared_ptr<Framebuffer> framebuffer(const std::string &name);

		// Explicitly bind a framebuffer (nullptr = default)
		void bind(const std::shared_ptr<Framebuffer>& fb);

		//scoped bind that automatically unbind and restore previous
		[[nodiscard]] FramebufferFactory::Guard bind_scoped(const std::shared_ptr<Framebuffer>& fb);

		// Get currently bound framebuffer (nullptr = default)
		std::shared_ptr<Framebuffer> current() const;

	private:
		std::unordered_map<int, std::unique_ptr<FramebufferFactory>> factory_map_;
	};
}
#endif//Framebuffer_h
