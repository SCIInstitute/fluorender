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

#ifndef VolCalShader_h
#define VolCalShader_h

#include <string>
#include <vector>

namespace flvr
{
	#define CAL_SUBSTRACTION	1	//initialize the segmentation fragment shader
	#define CAL_ADDITION		2	//diffusion based grow
	#define CAL_DIVISION		3	//initialize the labeling fragment shader
	#define CAL_INTERSECTION	4	//minimum of two
	#define CAL_APPLYMASK		5	//apply mask to volume
	#define CAL_APPLYMASKINV	6	//apply the inverted mask
	#define CAL_APPLYMASKINV2	7	//apply the inverted mask
	#define CAL_INTERSECTION_WITH_MASK	8	//minimum of two with mask

	class ShaderProgram;

	class VolCalShader
	{
	public:
		VolCalShader(int type);
		~VolCalShader();

		bool create();

		inline int type() {return type_;}

		inline bool match(int type)
		{ 
			return (type_ == type);
		}

		inline ShaderProgram* program() { return program_; }

	protected:
		bool emit(std::string& s);

		int type_;

		ShaderProgram* program_;
	};

	class VolCalShaderFactory
	{
	public:
		VolCalShaderFactory();
		~VolCalShaderFactory();
		void clear();

		ShaderProgram* shader(int type);

	protected:
		std::vector<VolCalShader*> shader_;
		int prev_shader_;
	};

} // end namespace flvr

#endif // VolCalShader_h
