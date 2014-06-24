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

#ifndef SLIVR_MeshRenderer_h
#define SLIVR_MeshRenderer_h

#include "ShaderProgram.h"
#include "MshShader.h"
#include "glm.h"
#include "Plane.h"
#include <vector>

using namespace std;

namespace FLIVR
{
	class MshShaderFactory;

	class MeshRenderer
	{
	public:
		MeshRenderer(GLMmodel* data);
		MeshRenderer(MeshRenderer&);
		~MeshRenderer();

		//draw
		void draw(bool tex=true, bool list=true);
		void update();

		//depth peeling
		void set_depth_peel(int peel) {depth_peel_ = peel;}
		int get_depth_peel() {return depth_peel_;}

		//clipping planes
		void set_planes(vector<Plane*> *p);
		vector<Plane*> *get_planes();

		//size limiter
		void set_limit(int val) {limit_ = val;}
		int get_limit() {return limit_;}

	protected:
		//data and display list
		GLMmodel* data_;
		GLuint data_list_;
		//depth peeling
		int depth_peel_;	//0:no peeling; 1:peel positive; 2:peel both; -1:peel negative
		//planes
		vector<Plane *> planes_;
		//draw with clipping
		bool draw_clip_;
		int limit_;

		static MshShaderFactory msh_shader_factory_;
	};

} // End namespace FLIVR

#endif 
