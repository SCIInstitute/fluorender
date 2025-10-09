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

#include <string>
#include <vector>
#include <map>
#include <memory>

#ifndef __glew_h__
typedef unsigned int GLenum;
#endif

namespace flvr
{
	enum class FBTexType
	{
		Render_RGBA,
		UChar_RGBA,
		Render_Int32,
		Depth_Float,
		Ext_3D
	};
	enum class TexFilter {
		Nearest,
		Linear,
		LinearMipmapLinear,
		// Add more as needed
	};
	enum class TexWrap {
		ClampToEdge,
		Repeat,
		MirroredRepeat,
		// Add more as needed
	};
	struct FBTexConfig {
		FBTexType type;
		bool useMipmap = false;
		TexFilter minFilter = TexFilter::Linear;
		TexFilter magFilter = TexFilter::Linear;
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
	class FramebufferManager;
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

	enum class FBRole
	{
		RenderFloat,
		RenderFloatMipmap,
		RenderUChar,
		Pick,
		Depth,
		Volume
	};
	class Framebuffer
	{
	public:
		Framebuffer(const FBRole& role, int nx, int ny, const std::string &name);
		~Framebuffer();

		void create();
		void destroy();
		void bind();
		static void bind(unsigned int id);
		void unbind(unsigned int prev_id = 0);
		void protect() { protected_ = true; }
		void unprotect() { protected_ = false; }
		bool valid() { return valid_; }
		unsigned int id() { return id_; }

		bool attach_texture(int ap, const std::shared_ptr<FramebufferTexture>& tex);
		bool attach_texture(int ap, unsigned int tex_id, int layer=0);
		void detach_texture(const std::shared_ptr<FramebufferTexture>& tex);
		void detach_texture(int ap);
		void bind_texture(int ap, int tex_unit);
		void unbind_texture(int ap);
		unsigned int tex_id(int ap);

		void resize(int nx, int ny);

		//generate mipmap
		void generate_mipmap(int ap);

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
			int ap, GLenum format, GLenum type, void* data);
		static bool read_default(int x, int y, int width, int height,
			GLenum format, GLenum type, void* data);

	private:
		unsigned int id_ = 0;
		FBRole role_;
		int nx_ = 0;
		int ny_ = 0;
		std::string name_;//specify its use
		bool valid_ = false;
		bool protected_ = false;
		std::map<int, std::shared_ptr<FramebufferTexture>> attachments_;

		friend class FramebufferManager;
	};

	class FramebufferManager
	{
	public:
		FramebufferManager();
		~FramebufferManager();
		void clear();

		Framebuffer* framebuffer(const FBRole& role, int nx, int ny,
			const std::string &name="");
		Framebuffer* framebuffer(const std::string &name);

	private:
		std::vector<std::shared_ptr<Framebuffer>> fb_list_;
		std::vector<std::shared_ptr<FramebufferTexture>> tex_list_;
	};

}
#endif//Framebuffer_h
