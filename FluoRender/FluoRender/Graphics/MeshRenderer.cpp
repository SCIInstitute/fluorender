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

#include <GL/glew.h>
#include <MeshRenderer.h>
#include <TextureRenderer.h>
#include <VertexArray.h>
#include <MshShader.h>
#include <ShaderProgram.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <Plane.h>
#include <FramebufferStateTracker.h>
#include <Framebuffer.h>
#include <glm.h>
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

using namespace flvr;

MeshRenderer::MeshRenderer()
	: data_(nullptr),
	depth_peel_(0),
	draw_clip_(false),
	limit_(-1),
	shading_(true),
	flat_shading_(false),
	has_vertex_color_(false),
	fog_(false),
	alpha_(1.0)
{
}

MeshRenderer::MeshRenderer(MeshRenderer& copy)
	: data_(copy.data_),
	depth_peel_(copy.depth_peel_),
	draw_clip_(copy.draw_clip_),
	limit_(copy.limit_),
	shading_(copy.shading_),
	flat_shading_(copy.flat_shading_),
	has_vertex_color_(copy.has_vertex_color_),
	fog_(copy.fog_),
	alpha_(copy.alpha_)
{
}

MeshRenderer::~MeshRenderer()
{
}

std::shared_ptr<VertexArray> MeshRenderer::GetOrCreateVertexArray(bool vbuf, bool ibuf)
{
	auto va = va_model_.lock();
	if (!va || !va->valid())
	{
		va = glbin_vertex_array_manager.vertex_array(vbuf, ibuf);
		va_model_ = va;
	}
	if (!ibuf && va->is_indexed())
	{
		//remove index
		va->delete_index_buffer();
	}
	return va;
}

void MeshRenderer::draw()
{
	if (!data_ || !data_->groups)
		return;
	auto va = va_model_.lock();
	if (!va || !va->valid())
		return;

	std::shared_ptr<ShaderProgram> shader;
	GLMgroup* group = data_->groups;
	GLint pos = 0;
	bool tex = data_->hastexture == GL_TRUE;
	//bool normal = data_->numnormals > 0 ? false : true;

	va->draw_begin();
	while (group)
	{
		if (group->numtriangles == 0)
		{
			group = group->next;
			continue;
		}

		//set up shader
		shader = glbin_shader_manager.shader(gstMeshShader,
			ShaderParams::Mesh(0, depth_peel_, true, tex, fog_, shading_, flat_shading_, has_vertex_color_));
		assert(shader);
		shader->bind();

		//uniforms
		shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
		shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat));
		if (shading_)
		{
			glm::mat4 normal_mat = glm::mat4(glm::inverseTranspose(glm::mat3(m_mv_mat)));
			shader->setLocalParamMatrix(2, glm::value_ptr(normal_mat));
		}
		shader->setLocalParam(0,
			color_.r(), color_.g(), color_.b(), alpha_);
		shader->setLocalParam(1,
			shading_strength_, shading_shine_,
			glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x);
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
		shader->setLocalParam(8, m_fog_intensity, m_fog_start, m_fog_end, 0.0);
		shader->setLocalParam(19, m_fog_color.r(), m_fog_color.g(), m_fog_color.b(), 1.0);

		if (depth_peel_)
			shader->setLocalParam(7, 1.0 / double(vp_[2]), 1.0 / double(vp_[3]), 0.0, 0.0);

		//set clipping planes
		auto planes = clipping_box_.GetPlanesWorld();
		double abcd[4];
		planes[0].get(abcd);
		shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
		planes[1].get(abcd);
		shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
		planes[2].get(abcd);
		shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
		planes[3].get(abcd);
		shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
		planes[4].get(abcd);
		shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
		planes[5].get(abcd);
		shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

		//draw
		va->draw_unmanaged(pos, group->numtriangles);
		pos += group->numtriangles * 3;
		group = group->next;
	}
	va->draw_end();

	// Release shader.
	shader->unbind();

	//release texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void MeshRenderer::draw_wireframe()
{
	if (!data_ || !data_->groups)
		return;
	auto va = va_model_.lock();
	if (!va || !va->valid())
		return;

	auto cur_buffer = glbin_framebuffer_manager.current();
	assert(cur_buffer);
	FramebufferStateGuard fbg(*cur_buffer);
	cur_buffer->set_polygon_mode(PolygonMode::Line);
	cur_buffer->apply_state();

	GLMgroup* group = data_->groups;
	GLint pos = 0;
	int peel = 0;
	bool tex = false;
	bool light = false;

	//set up shader
	auto shader = glbin_shader_manager.shader(gstMeshShader,
		ShaderParams::Mesh(0, peel, true, tex, fog_, light, false, false));
	assert(shader);
	shader->bind();

	//uniforms
	shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
	shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat));
	GLMmaterial* material = &data_->materials[0];
		shader->setLocalParam(0,
			color_.r(), color_.g(), color_.b(), alpha_);
	shader->setLocalParam(8, m_fog_intensity, m_fog_start, m_fog_end, 0.0);
	//set clipping planes
	auto planes = clipping_box_.GetPlanesWorld();
	double abcd[4];
	planes[0].get(abcd);
	shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[1].get(abcd);
	shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[2].get(abcd);
	shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[3].get(abcd);
	shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[4].get(abcd);
	shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[5].get(abcd);
	shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

	va->draw_begin();
	while (group)
	{
		if (group->numtriangles == 0)
		{
			group = group->next;
			continue;
		}

		//draw
		va->draw_unmanaged(pos, group->numtriangles);
		pos += group->numtriangles * 3;
		group = group->next;
	}
	va->draw_end();

	// Release shader.
	shader->unbind();
}

void MeshRenderer::draw_integer(unsigned int name)
{
	if (!data_ || !data_->groups)
		return;
	auto va = va_model_.lock();
	if (!va || !va->valid())
		return;

	GLMgroup* group = data_->groups;
	GLint pos = 0;

	//set up shader
	auto shader = glbin_shader_manager.shader(gstMeshShader,
		ShaderParams::Mesh(1, 0, true, false, false, false, false, false));
	assert(shader);
	shader->bind();

	//uniforms
	shader->setLocalParamMatrix(0, glm::value_ptr(m_proj_mat));
	shader->setLocalParamMatrix(1, glm::value_ptr(m_mv_mat));
	shader->setLocalParamUInt(0, name);
	//set clipping planes
	auto planes = clipping_box_.GetPlanesWorld();
	double abcd[4];
	planes[0].get(abcd);
	shader->setLocalParam(10, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[1].get(abcd);
	shader->setLocalParam(11, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[2].get(abcd);
	shader->setLocalParam(12, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[3].get(abcd);
	shader->setLocalParam(13, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[4].get(abcd);
	shader->setLocalParam(14, abcd[0], abcd[1], abcd[2], abcd[3]);
	planes[5].get(abcd);
	shader->setLocalParam(15, abcd[0], abcd[1], abcd[2], abcd[3]);

	va->draw_begin();
	while (group)
	{
		if (group->numtriangles == 0)
		{
			group = group->next;
			continue;
		}

		//draw
		va->draw_unmanaged(pos, group->numtriangles);
		pos += group->numtriangles * 3;
		group = group->next;
	}
	va->draw_end();

	// Release shader.
	shader->unbind();
}
