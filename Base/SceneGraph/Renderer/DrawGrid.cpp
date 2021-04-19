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
#include "DrawGrid.hpp"
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/ImgShader.h>
#include <FLIVR/VertexArray.h>
#include <glm/gtc/type_ptr.hpp>

using namespace fluo;

DrawGrid::DrawGrid():
	Renderer3D()
{
	//setupInputs();
	//setupOutputs();
}

DrawGrid::DrawGrid(const DrawGrid& renderer, const CopyOp& copyop, bool copy_values):
	Renderer3D(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

DrawGrid::~DrawGrid()
{
}

void DrawGrid::render()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	flvr::ShaderProgram* shader =
		flvr::TextureRenderer::img_shader_factory_.shader(IMG_SHDR_DRAW_GEOMETRY);
	if (shader)
	{
		if (!shader->valid())
			shader->create();
		shader->bind();
	}
	Color text_color;
	getValue("text color", text_color);
	shader->setLocalParam(0, text_color.r(), text_color.g(), text_color.b(), 1.0);
	glm::mat4 proj_mat, mv_mat;
	getValue("proj mat", proj_mat);
	getValue("mv mat", mv_mat);
	glm::mat4 matrix = proj_mat * mv_mat;
	shader->setLocalParamMatrix(0, glm::value_ptr(matrix));

	flvr::VertexArray* va_grid =
		flvr::TextureRenderer::vertex_array_manager_.vertex_array(flvr::VA_Grid);
	if (va_grid)
	{
		//set parameters
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, 5.0));
		double distance;
		getValue("distance", distance);
		params.push_back(std::pair<unsigned int, double>(1, distance));
		va_grid->set_param(params);
		va_grid->draw();
	}

	if (shader && shader->valid())
		shader->release();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}