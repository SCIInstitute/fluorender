﻿//  
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

#include <GL/glew.h>
#include <MeshRenderer.h>
#include <TextureRenderer.h>
#include <VertexArray.h>
#include <MshShader.h>
#include <ShaderProgram.h>
#include <Global.h>
#include <glm.h>
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace flvr
{
	MeshRenderer::MeshRenderer(GLMmodel* data)
		: data_(data),
		depth_peel_(0),
		draw_clip_(false),
		limit_(-1),
		light_(true),
		fog_(false),
		alpha_(1.0),
		update_(true),
		va_model_(0)
	{
	}

	MeshRenderer::MeshRenderer(MeshRenderer &copy)
		: data_(copy.data_),
		depth_peel_(copy.depth_peel_),
		draw_clip_(copy.draw_clip_),
		limit_(copy.limit_),
		light_(copy.light_),
		fog_(copy.fog_),
		alpha_(copy.alpha_),
		update_(true),
		va_model_(0)
	{
	}

	MeshRenderer::~MeshRenderer()
	{
		if (va_model_)
			delete va_model_;
	}

	//clipping planes
	void MeshRenderer::set_planes(std::vector<fluo::Plane*> *p)
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
			fluo::Plane *plane = new fluo::Plane(*(*p)[i]);
			planes_.push_back(plane);
		}
	}

	std::vector<fluo::Plane*>* MeshRenderer::get_planes()
	{
		return &planes_;
	}

	void MeshRenderer::update()
	{
		bool bnormal = data_->normals;
		bool btexcoord = data_->texcoords;
		std::vector<float> verts;

		GLMgroup* group = data_->groups;
		GLMtriangle* triangle = 0;
		while (group)
		{
			for (size_t i=0; i<group->numtriangles; ++i)
			{
				triangle = &(data_->triangles[group->triangles[i]]);
				for (size_t j=0; j<3; ++j)
				{
					verts.push_back(data_->vertices[3*triangle->vindices[j]]);
					verts.push_back(data_->vertices[3*triangle->vindices[j]+1]);
					verts.push_back(data_->vertices[3*triangle->vindices[j]+2]);
					if (bnormal)
					{
						verts.push_back(data_->normals[3*triangle->nindices[j]]);
						verts.push_back(data_->normals[3*triangle->nindices[j]+1]);
						verts.push_back(data_->normals[3*triangle->nindices[j]+2]);
					}
					if (btexcoord)
					{
						verts.push_back(data_->texcoords[2*triangle->tindices[j]]);
						verts.push_back(data_->texcoords[2*triangle->tindices[j]+1]);
					}
				}
			}
			group = group->next;
		}

		if (!va_model_ || !va_model_->valid())
			va_model_ = glbin_vertex_array_manager.vertex_array(true, false);
		if (!va_model_)
			return;
		va_model_->buffer_data(
			VABuf_Coord, sizeof(float)*verts.size(),
			&verts[0], GL_STATIC_DRAW);

		GLsizei stride = sizeof(float)*(3+(bnormal?3:0)+(btexcoord?2:0));
		va_model_->attrib_pointer(
			0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
		if (bnormal)
			va_model_->attrib_pointer(
				1, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)12);
		if (btexcoord)
		{
			if (bnormal)
				va_model_->attrib_pointer(
					2, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)24);
			else
				va_model_->attrib_pointer(
					1, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)12);
		}
	}

	void MeshRenderer::draw()
	{
		if (!data_ || !data_->vertices || !data_->triangles)
			return;

		//set up vertex array object
		if (update_)
		{
			update();
			update_ = false;
		}
		if (!va_model_)
			return;
		ShaderProgram* shader = 0;

		GLMgroup* group = data_->groups;
		GLint pos = 0;
		bool tex = data_->hastexture==GL_TRUE;
		va_model_->draw_begin();
		while (group)
		{
			if (group->numtriangles == 0)
			{
				group = group->next;
				continue;
			}

			//set up shader
			shader = glbin_msh_shader_factory.shader(0,
				depth_peel_, tex, fog_, light_);
			if (shader)
			{
				if (!shader->valid())
					shader->create();
				shader->bind();
			}
			//uniforms
			shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
			shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat));
			if (light_)
			{
				glm::mat4 normal_mat = glm::mat4(glm::inverseTranspose(glm::mat3(m_mv_mat)));
				shader->setLocalParamMatrix(2, glm::value_ptr(normal_mat));
				GLMmaterial* material = &data_->materials[group->material];
				if (material)
				{
					shader->setLocalParam(0, material->ambient[0], material->ambient[1], material->ambient[2], material->ambient[3]);
					shader->setLocalParam(1, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
					shader->setLocalParam(2, material->specular[0], material->specular[1], material->specular[2], material->specular[3]);
					shader->setLocalParam(3, material->shininess, alpha_, 0.0, 0.0);
				}
			}
			else
			{//color
				GLMmaterial* material = &data_->materials[group->material];
				if (material)
					shader->setLocalParam(0, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
				else
					shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);//color
				shader->setLocalParam(3, 0.0, alpha_, 0.0, 0.0);//alpha
			}
			if (tex)
			{
				GLMmaterial* material = &data_->materials[group->material];
				if (material)
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,
						material->textureID);
				}
			}
			if (fog_)
				shader->setLocalParam(8, m_fog_intensity, m_fog_start, m_fog_end, 0.0);

			if (depth_peel_)
				shader->setLocalParam(7, 1.0/double(vp_[2]), 1.0/double(vp_[3]), 0.0, 0.0);

			//draw
			va_model_->draw_arrays(GL_TRIANGLES, pos, (GLsizei)(group->numtriangles * 3));
			pos += group->numtriangles*3;
			group = group->next;
		}
		va_model_->draw_end();

		// Release shader.
		if (shader && shader->valid())
			shader->release();
		//release texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void MeshRenderer::draw_wireframe()
	{
		if (!data_ || !data_->vertices || !data_->triangles)
			return;

		//set up vertex array object
		if (update_)
		{
			update();
			update_ = false;
		}
		if (!va_model_)
			return;

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		GLMgroup* group = data_->groups;
		GLint pos = 0;
		int peel = 0;
		bool tex = false;
		bool light = false;
		
		//set up shader
		ShaderProgram* shader = 0;
		shader = glbin_msh_shader_factory.shader(0,
			peel, tex, fog_, light);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}
		//uniforms
		shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
		shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat));
		GLMmaterial* material = &data_->materials[0];
		if (material)
			shader->setLocalParam(0, material->diffuse[0], material->diffuse[1], material->diffuse[2], material->diffuse[3]);
		else
			shader->setLocalParam(0, 1.0, 0.0, 0.0, 1.0);
		shader->setLocalParam(3, 0.0, 1.0, 0.0, 0.0);//alpha
		if (fog_)
			shader->setLocalParam(8, m_fog_intensity, m_fog_start, m_fog_end, 0.0);

		va_model_->draw_begin();
		while (group)
		{
			if (group->numtriangles == 0)
			{
				group = group->next;
				continue;
			}

			//draw
			va_model_->draw_arrays(GL_TRIANGLES, pos, (GLsizei)(group->numtriangles * 3));
			pos += group->numtriangles*3;
			group = group->next;
		}
		va_model_->draw_end();

		// Release shader.
		if (shader && shader->valid())
			shader->release();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void MeshRenderer::draw_integer(unsigned int name)
	{
		if (!data_ || !data_->vertices || !data_->triangles)
			return;

		//set up vertex array object
		if (update_)
		{
			update();
			update_ = false;
		}
		if (!va_model_)
			return;

		GLMgroup* group = data_->groups;
		GLint pos = 0;

		//set up shader
		ShaderProgram* shader = 0;
		shader = glbin_msh_shader_factory.shader(1,
			0, false, false, false);
		if (shader)
		{
			if (!shader->valid())
				shader->create();
			shader->bind();
		}
		//uniforms
		shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
		shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat));
		shader->setLocalParamUInt(0, name);

		va_model_->draw_begin();
		while (group)
		{
			if (group->numtriangles == 0)
			{
				group = group->next;
				continue;
			}

			//draw
			va_model_->draw_arrays(GL_TRIANGLES, pos, (GLsizei)(group->numtriangles * 3));
			pos += group->numtriangles*3;
			group = group->next;
		}
		va_model_->draw_end();

		// Release shader.
		if (shader && shader->valid())
			shader->release();
	}

} // namespace flvr
