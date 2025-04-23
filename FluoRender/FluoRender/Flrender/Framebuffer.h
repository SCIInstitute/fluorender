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

namespace flvr
{
	enum FBTexType
	{
		FBTex_Render_RGBA = 0,
		FBTex_UChar_RGBA,
		FBTex_3D_Int,
		FBTex_Render_Int32,
		FBTex_Depth_Float,
	};
	class Framebuffer;
	class FramebufferManager;
	class FramebufferTexture
	{
	public:
		FramebufferTexture(FBTexType type, int nx, int ny);
		~FramebufferTexture();

		void create();
		void destroy();
		bool bind();
		void unbind();
		bool valid();
		void resize(int nx, int ny);

	private:
		unsigned int id_;
		FBTexType type_;
		int nx_;
		int ny_;
		bool valid_;

		friend class Framebuffer;
	};

	enum FBType
	{
		FB_Render_RGBA = 0,
		FB_UChar_RGBA,
		FB_3D_Int,
		FB_Pick_Int32_Float,
		FB_Depth_Float,
	};
	class Framebuffer
	{
	public:
		Framebuffer(FBType type, int nx, int ny, const std::string &name);
		~Framebuffer();

		void create();
		void destroy();
		void bind();
		void unbind();
		void protect();
		void unprotect();
		bool valid();
		unsigned int id();

		bool attach_texture(int ap, FramebufferTexture* tex);
		bool attach_texture(int ap, unsigned int tex_id, int layer=0);
		void detach_texture(int ap);
		void detach_texture(FramebufferTexture* tex);
		void bind_texture(int ap);
		unsigned int tex_id(int ap);

		bool match_size(int nx, int ny);
		void resize(int nx, int ny);

		//match without size
		bool match(FBType);
		//match with size
		bool match(FBType, int, int);
		//match by name
		bool match(const std::string &name);

		//name represents its use
		void set_name(const std::string &name);
		void clear_name();
		std::string &get_name() { return name_; }

		//read pick value
		unsigned int read_value(int, int);

	private:
		unsigned int id_;
		FBType type_;
		int nx_;
		int ny_;
		std::string name_;//specify its use
		bool valid_;
		bool protected_;
		std::vector<std::pair<int, FramebufferTexture*>> tex_list_;

		friend class FramebufferManager;
	};

	class FramebufferManager
	{
	public:
		FramebufferManager();
		~FramebufferManager();
		void clear();

		Framebuffer* framebuffer(FBType type, int nx, int ny,
			const std::string &name="");
		Framebuffer* framebuffer(const std::string &name);

	private:
		std::vector<Framebuffer*> fb_list_;
		std::vector<FramebufferTexture*> tex_list_;
	};

}
#endif//Framebuffer_h
