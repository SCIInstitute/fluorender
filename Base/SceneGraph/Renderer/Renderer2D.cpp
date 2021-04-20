/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "Renderer2D.hpp"
#include <FLIVR/Framebuffer.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>

using namespace fluo;

Renderer2D::Renderer2D():
	Processor()
{
	setupInputs();
	setupOutputs();
	setBeforeRunFunction(
		std::bind(&Renderer2D::preDraw,
		this, std::placeholders::_1));
	setAfterRunFunction(
		std::bind(&Renderer2D::postDraw,
		this, std::placeholders::_1));
	setProcessFunction(
		std::bind(&Renderer2D::draw,
			this, std::placeholders::_1));
}

Renderer2D::Renderer2D(const Renderer2D& renderer, const CopyOp& copyop, bool copy_values):
	Processor(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

Renderer2D::~Renderer2D()
{
}

void Renderer2D::setupInputs()
{
}

void Renderer2D::setupOutputs()
{

}

void Renderer2D::preDraw(Event &event)
{
	int nx, ny;
	getValue("nx", nx);
	getValue("ny", ny);
	int fb_type;
	getValue("fb type", fb_type);
	std::string fb_name;
	getValue("fb name", fb_name);
	flvr::Framebuffer* fb =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(
			static_cast<flvr::FBType>(fb_type), nx, ny, fb_name);
	if (!fb)
		return;
	fb->bind();
	bool fb_protect;
	getValue("fb protect", fb_protect);
	if (fb_protect)
		fb->protect();
}

void Renderer2D::postDraw(Event &event)
{
	std::string fb_name;
	getValue("fb name", fb_name);
	flvr::Framebuffer* fb =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(fb_name);
	bool fb_protect;
	getValue("fb protect", fb_protect);
	if (fb && fb_protect)
		fb->unprotect();

	std::string fb_old_name;
	getValue("fb old name", fb_old_name);
	fb = flvr::TextureRenderer::framebuffer_manager_.framebuffer(fb_old_name);
	if (!fb)
		return;
	fb->bind();

	bool draw_buffer = false;
	getValue("draw buffer", draw_buffer);
	if (draw_buffer)
		drawBuffer();
}

void Renderer2D::drawBuffer()
{
	//draw the final buffer to the windows buffer
	glActiveTexture(GL_TEXTURE0);
	std::string fb_name;
	getValue("fb name", fb_name);
	flvr::Framebuffer* fb =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(fb_name);
	if (fb)
		fb->bind_texture(GL_COLOR_ATTACHMENT0);
	glEnable(GL_BLEND);
	int blend_param1=GL_ONE, blend_param2= GL_ONE_MINUS_SRC_ALPHA;
	getValue("blend param1", blend_param1);
	getValue("blend param2", blend_param2);
	glBlendFunc(blend_param1, blend_param2);
	glDisable(GL_DEPTH_TEST);

	//2d adjustment
	flvr::ShaderProgram* img_shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_BLEND_BRIGHT_BACKGROUND_HDR);
	if (img_shader)
	{
		if (!img_shader->valid())
			img_shader->create();
		img_shader->bind();
	}
	Color gamma, brightness, hdr;
	getValue("gamma", gamma);
	getValue("brightness", brightness);
	getValue("hdr", hdr);
	img_shader->setLocalParam(0, gamma.r(), gamma.g(), gamma.b(), 1.0);
	img_shader->setLocalParam(1, brightness.r(), brightness.g(), brightness.b(), 1.0);
	img_shader->setLocalParam(2, hdr.r(), hdr.g(), hdr.b(), 0.0);
	//2d adjustment

	//DrawViewQuad();
	flvr::VertexArray* quad_va =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Norm_Square);
	if (quad_va)
		quad_va->draw();

	if (img_shader && img_shader->valid())
		img_shader->release();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}