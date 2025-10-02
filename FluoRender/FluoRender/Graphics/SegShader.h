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

#ifndef SegShader_h
#define SegShader_h

#include <string>
#include <vector>

namespace flvr
{
//type definitions
#define SEG_SHDR_INITIALIZE	1	//initialize the segmentation fragment shader
#define SEG_SHDR_DB_GROW	2	//diffusion based grow

	class ShaderProgram;

	class SegShader
	{
	public:
		SegShader(int type, int paint_mode, int hr_mode,
			bool use_2d, bool shading, int peel,
			bool clip, bool use_dir);
		~SegShader();

		bool create();

		inline int type() {return type_;}
		inline int paint_mode() {return paint_mode_;}
		inline int hr_mode() {return hr_mode_;}
		inline bool use_2d() {return use_2d_;}
		inline bool shading() {return shading_;}
		inline int peel() {return peel_;}
		inline bool clip() {return clip_;}
		inline bool use_dir() { return use_dir_; }

		inline bool match(int type, int paint_mode, int hr_mode,
			bool use_2d, bool shading, int peel,
			bool clip, bool use_dir)
		{ 
			return (type_ == type &&
				paint_mode_ == paint_mode &&
				hr_mode_ == hr_mode &&
				use_2d_ == use_2d &&
				shading_ == shading &&
				peel_ == peel &&
				clip_ == clip &&
				use_dir_ == use_dir);
		}

		inline ShaderProgram* program() { return program_; }

	protected:
		bool emit(std::string& s);

		int type_;
		int paint_mode_;
		int hr_mode_;
		bool use_2d_;
		bool shading_;
		int peel_;
		bool clip_;
		bool use_dir_;

		ShaderProgram* program_;
	};

	class SegShaderFactory
	{
	public:
		SegShaderFactory();
		~SegShaderFactory();
		void clear();

		ShaderProgram* shader(
			int type, int paint_mode, int hr_mode,
			bool use_2d, bool shading, int peel,
			bool clip, bool use_dir);

	protected:
		std::vector<SegShader*> shader_;
		int prev_shader_;
	};

} // end namespace flvr

#endif // SegShader_h
