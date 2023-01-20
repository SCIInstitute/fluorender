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

#ifndef MeshRenderer_h
#define MeshRenderer_h

#include "ShaderProgram.h"
#include "MshShader.h"
#include "glm.h"
#include <Types/Plane.h>
#include <vector>
#include <cstring>
#include <glm/glm.hpp>

using namespace std;

namespace flvr
{
	class MshShaderFactory;
	class VertexArray;

	class MeshRenderer
	{
	public:
		MeshRenderer(GLMmodel* data);
		MeshRenderer(MeshRenderer&);
		~MeshRenderer();

		//set viewport
		void set_viewport(GLint vp[4])
		{
			memcpy(vp_, vp, sizeof(GLint) * 4);
		}

		//draw
		void draw();
		void draw_wireframe();
		void draw_integer(unsigned int name);
		void update();

		//depth peeling
		void set_depth_peel(int peel) {depth_peel_ = peel;}
		int get_depth_peel() {return depth_peel_;}

		//clipping planes
		void set_planes(vector<fluo::Plane*> *p);
		vector<fluo::Plane*> *get_planes();

		//size limiter
		void set_limit(int val) {limit_ = val;}
		int get_limit() {return limit_;}

		//matrices
		void SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat)
		{ m_mv_mat = mv_mat; m_proj_mat = proj_mat; }

		//alpha
		void set_alpha(float alpha)
		{ alpha_ = alpha; }
		float get_alpha()
		{ return alpha_; }
		//effects
		void set_lighting(bool val)
		{ light_ = val; }
		bool get_lighting()
		{ return light_; }
		void set_fog(bool use_fog, double fog_intensity, double fog_start, double fog_end)
		{ fog_ = use_fog; m_fog_intensity = fog_intensity; m_fog_start = fog_start; m_fog_end = fog_end; }
		bool get_fog()
		{ return fog_; }

	protected:
		//viewport
		GLint vp_[4];
		//data and display list
		GLMmodel* data_;
		//depth peeling
		int depth_peel_;	//0:no peeling; 1:peel positive; 2:peel both; -1:peel negative
		//planes
		vector<fluo::Plane *> planes_;
		//draw with clipping
		bool draw_clip_;
		int limit_;
		//matrices
		glm::mat4 m_mv_mat, m_proj_mat;
		//lighting
		bool light_;
		//fog
		bool fog_;
		double m_fog_intensity;
		double m_fog_start;
		double m_fog_end;
		float alpha_;
		//vertex buffer
		VertexArray* va_model_;
		//bool update
		bool update_;

		static MshShaderFactory msh_shader_factory_;
	};

} // End namespace flvr

#endif 
