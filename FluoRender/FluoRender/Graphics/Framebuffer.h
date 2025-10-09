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

	class Framebuffer
	{
	public:
		Framebuffer(const FBTexConfig& config, int nx, int ny, const std::string &name);
		~Framebuffer();

		void create();
		void destroy();
		void bind();
		void unbind();
		void protect();
		void unprotect();
		bool valid();
		unsigned int id();

		bool attach_texture(int ap, const std::shared_ptr<FramebufferTexture>& tex);
		bool attach_texture(int ap, unsigned int tex_id, int layer=0);
		void detach_texture(const std::shared_ptr<FramebufferTexture>& tex);
		void detach_texture(int ap);
		void bind_texture(int ap);
		unsigned int tex_id(int ap);

		bool match_size(int nx, int ny);
		void resize(int nx, int ny);

		//generate mipmap
		void generate_mipmap(int ap);

		//match without size
		bool match(const FBTexConfig& config);
		//match with size
		bool match(const FBTexConfig& config, int, int);
		//match by name
		bool match(const std::string &name);

		//name represents its use
		void set_name(const std::string &name);
		void clear_name();
		std::string get_name() { return name_; }

		//read pick value
		unsigned int read_value(int, int);

	private:
		unsigned int id_ = 0;
		FBTexConfig config_;
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

		Framebuffer* framebuffer(const FBTexConfig& config, int nx, int ny,
			const std::string &name="");
		Framebuffer* framebuffer(const std::string &name);

	private:
		std::vector<std::shared_ptr<Framebuffer>> fb_list_;
		std::vector<std::shared_ptr<FramebufferTexture>> tex_list_;
	};

}
#endif//Framebuffer_h
