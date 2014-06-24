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

#include "MeshRenderer.h"
#include <iostream>

namespace FLIVR
{
	MshShaderFactory MeshRenderer::msh_shader_factory_;

	MeshRenderer::MeshRenderer(GLMmodel* data)
		: data_(data),
		depth_peel_(0),
		draw_clip_(false),
		limit_(-1)
	{
		data_list_ = glmList(data_, GLM_TEXTURE, limit_);
	}

	MeshRenderer::MeshRenderer(MeshRenderer &copy)
		: data_(copy.data_),
		depth_peel_(copy.depth_peel_),
		draw_clip_(copy.draw_clip_)
	{
		if (glIsList(data_list_))
			glDeleteLists(data_list_, 1);
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::update()
	{
		if (glIsList(data_list_))
			glDeleteLists(data_list_, 1);
		data_list_ = glmList(data_, GLM_TEXTURE, limit_);
	}

	//clipping planes
	void MeshRenderer::set_planes(vector<Plane*> *p)
	{
		int i;
		if (!planes_.empty())
		{
			for (i=0; i<(int)planes_.size(); i++)
			{
				if (planes_[i])
					delete planes_[i];
			}
			planes_.clear();
		}

		for (i=0; i<(int)p->size(); i++)
		{
			Plane *plane = new Plane(*(*p)[i]);
			planes_.push_back(plane);
		}
	}

	vector<Plane*>* MeshRenderer::get_planes()
	{
		return &planes_;
	}

	void MeshRenderer::draw(bool tex, bool list)
	{
		GLboolean use_fog = glIsEnabled(GL_FOG);
		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);

		FragmentProgram* shader = 0;
		shader = msh_shader_factory_.shader(use_fog!=0,
			depth_peel_, tex&&data_->hastexture==GL_TRUE);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}

		shader->setLocalParam(7, 1.0/double(vp[2]), 1.0/double(vp[3]), 0.0, 0.0);
		if (list)
		{
			if (glIsList(data_list_))
				glCallList(data_list_);
			else
			{
				data_list_ = glmList(data_, GLM_TEXTURE, limit_);
				glCallList(data_list_);
			}
		}
		else
			glmDraw(data_, GLM_TEXTURE, true, limit_);

		// Release shader.
		if (shader && shader->valid())
			shader->release();
	}

} // namespace FLIVR
