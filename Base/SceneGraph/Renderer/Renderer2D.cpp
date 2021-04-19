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
	std::string fb_save_name;
	getValue("fb save name", fb_save_name);
	flvr::Framebuffer* fb =
		flvr::TextureRenderer::framebuffer_manager_.framebuffer(fb_save_name);
	if (!fb)
		return;
	fb->bind();
}
