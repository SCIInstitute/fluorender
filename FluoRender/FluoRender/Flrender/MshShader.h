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

#ifndef MshShader_h
#define MshShader_h

#include <string>
#include <vector>

namespace flvr
{

	class ShaderProgram;

	class MshShader
	{
	public:
		MshShader(int type,
			int peel, bool tex,
			bool fog, bool light,
			bool normal);
		~MshShader();

		bool create();

		inline int type() { return type_; }
		inline int peel() { return peel_; }
		inline bool tex() { return tex_; }
		inline bool fog() { return fog_; }
		inline bool light() { return light_; }
		inline bool normal() { return normal_; }

		inline bool match(int type,
			int peel, bool tex,
			bool fog, bool light,
			bool normal)
		{ 
			return (type_ == type &&
					fog_ == fog && 
					peel_ == peel &&
					tex_ == tex &&
					light_ == light &&
					normal_ == normal); 
		}

		inline ShaderProgram* program() { return program_; }

	protected:
		bool emit_v(std::string& s);
		bool emit_f(std::string& s);
		bool emit_g(std::string& s);

		int type_;	//0:normal; 1:integer
		int peel_;	//0:no peeling; 1:peel positive; 2:peel both; -1:peel negative
		bool tex_;
		bool fog_;
		bool light_;
		bool normal_;//0:use normal from mesh; 1:generate normal in geom shader

		ShaderProgram* program_;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class MshShaderFactory
	{
	public:
		MshShaderFactory();
		~MshShaderFactory();

		ShaderProgram* shader(int type, int peel, bool tex,
			bool fog, bool light, bool normal);

	protected:
		std::vector<MshShader*> shader_;
		int prev_shader_;
	};

} // end namespace flvr

#endif // MshShader_h
