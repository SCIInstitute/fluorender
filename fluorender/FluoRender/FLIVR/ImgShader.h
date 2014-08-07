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
//  

#ifndef ImgShader_h
#define ImgShader_h

#include <string>
#include <vector>

namespace FLIVR
{
#define IMG_SHDR_BRIGHTNESS_CONTRAST		1
#define IMG_SHDR_BRIGHTNESS_CONTRAST_HDR	2
#define IMG_SHDR_GRADIENT_MAP				3
#define IMG_SHDR_FILTER_BLUR				4
#define IMG_SHDR_FILTER_MAX					5
#define IMG_SHDR_FILTER_SHARPEN				6
#define IMG_SHDR_DEPTH_TO_OUTLINES			7
#define IMG_SHDR_DEPTH_TO_GRADIENT			8
#define IMG_SHDR_GRADIENT_TO_SHADOW			9
#define IMG_SHDR_GRADIENT_TO_SHADOW_MESH	10
#define IMG_SHDR_BLEND_BRIGHT_BACKGROUND	11

	class FragmentProgram;

	class ImgShader
	{
	public:
		ImgShader(int type);
		~ImgShader();

		bool create();

		inline int type() {return type_;}

		inline bool match(int type)
		{ 
			return (type_ == type); 
		}

		inline FragmentProgram* program() { return program_; }

	protected:
		bool emit(std::string& s);

		int type_;

		FragmentProgram* program_;
	};

	class ImgShaderFactory
	{
	public:
		ImgShaderFactory();
		~ImgShaderFactory();

		FragmentProgram* shader(int type);

	protected:
		std::vector<ImgShader*> shader_;
		int prev_shader_;
	};

} // end namespace FLIVR

#endif // ImgShader_h
