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

#include "Renderview.hpp"
#include <Group.hpp>
#include <Timer.h>
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <VolumeLoader.h>
#include <Global.hpp>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/KernelProgram.h>

using namespace fluo;

Renderview::Renderview()
{
}

Renderview::Renderview(const Renderview& view, const CopyOp& copyop) :
	Group(view, copyop)
{

}

Renderview::~Renderview()
{

}

void Renderview::Init()
{
	bool bval;
	getValue(gstInitialized, bval);
	if (!bval)
	{
		//glViewport(0, 0, (GLint)(GetSize().x), (GLint)(GetSize().y));
		glEnable(GL_MULTISAMPLE);

		setValue(gstInitialized, true);

		glbin_timer->start();
	}
}

void Renderview::Clear()
{
	m_loader->RemoveAllLoadedBrick();
	flvr::TextureRenderer::clear_tex_pool();

	setRvalu(gstCurrentVolume, (Referenced*)(0));
}


