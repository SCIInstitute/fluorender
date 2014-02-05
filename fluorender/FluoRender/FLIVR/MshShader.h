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

#ifndef MshShader_h
#define MshShader_h

#include <string>
#include <vector>

namespace FLIVR
{

	class FragmentProgram;

	class MshShader
	{
	public:
		MshShader(bool fog, int peel, bool tex);
		~MshShader();

		bool create();

		inline bool fog() { return fog_; }
		inline int peel() { return peel_; }
		inline bool tex() { return tex_; }

		inline bool match(bool fog, int peel, bool tex)
		{ 
			return (fog_ == fog && 
					peel_ == peel &&
					tex_ == tex); 
		}

		inline FragmentProgram* program() { return program_; }

	protected:
		bool emit(std::string& s);

		bool fog_;
		int peel_;	//0:no peeling; 1:peel positive; 2:peel both; -1:peel negative
		bool tex_;

		FragmentProgram* program_;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class MshShaderFactory
	{
	public:
		MshShaderFactory();
		~MshShaderFactory();

		FragmentProgram* shader(bool fog, int peel, bool tex);

	protected:
		std::vector<MshShader*> shader_;
		int prev_shader_;
	};

} // end namespace FLIVR

#endif // MshShader_h
