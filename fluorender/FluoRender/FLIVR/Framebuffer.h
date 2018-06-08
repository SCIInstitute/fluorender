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

#ifndef Framebuffer_h
#define Framebuffer_h

#include <string>
#include <vector>

namespace FLIVR
{
	class FramebufferTexture
	{
	public:
		FramebufferTexture();
		~FramebufferTexture();

		bool create();
		void bind();
		bool valid();

	private:
		unsigned int id_;
	};

	class Framebuffer
	{
	public:
		Framebuffer();
		~Framebuffer();

		bool create();
		void bind();
		bool valid();

		bool attach_texture();
		bool detach_texture();

		bool resize();

	private:
		unsigned int id_;
	};

	class FramebufferManager
	{
	public:
		FramebufferManager();
		~FramebufferManager();

		Framebuffer* framebuffer();

	private:
		std::vector<Framebuffer*> fb_list_;
		std::vector<FramebufferTexture*> tex_list_;
	};
}
#endif//Framebuffer_h