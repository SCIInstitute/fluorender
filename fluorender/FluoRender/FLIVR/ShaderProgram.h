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

#ifndef ShaderProgram_h 
#define ShaderProgram_h

#include <string>


namespace FLIVR
{
	class ShaderProgram
	{
	public:
		ShaderProgram(const std::string& vert_shader,const std::string& frag_shader);
		ShaderProgram(const std::string& frag_shader);
		~ShaderProgram();

		unsigned int id();
		bool create();
		bool valid();
		void destroy();

		void bind();
		void bind_frag_data_location(int color_num, const char* name);
		void release();

		//set vector uniform (4x1)
		void setLocalParam(int, double, double, double, double);
		//set matrix uniform (4x4)
		void setLocalParamMatrix(int, float*);
		//set integer
		void setLocalParamUInt(int, unsigned int);

		// Call init_shaders_supported before shaders_supported queries!
		static bool init();
		static void init_shaders_supported();
		static bool shaders_supported();
		static int max_texture_size();
		static bool texture_non_power_of_two();
		static const int MAX_SHADER_UNIFORMS = 16;
		static std::string glsl_version_;

	protected:
		unsigned int id_;
		std::string  vert_shader_;
		std::string  frag_shader_;

		//validation
		bool valid_;

		//locations
		int loc_ui[MAX_SHADER_UNIFORMS];
		int loc_vec4[MAX_SHADER_UNIFORMS];
		int loc_mat4[MAX_SHADER_UNIFORMS];

		static bool init_;
		static bool supported_;
		static bool non_2_textures_;
		static int  max_texture_size_;
		static int v_major_;
		static int v_minor_;
	};
} // end namespace FLIVR

#endif // ShaderProgram_h
