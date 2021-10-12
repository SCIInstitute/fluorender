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

#include "DrawVolume.hpp"
#include <View.hpp>

using namespace fluo;

DrawVolume::DrawVolume():
	Renderer2D()
{
	setConditionFunction(std::bind(&DrawVolume::drawType,
		this));
}

DrawVolume::DrawVolume(const DrawVolume& renderer, const CopyOp& copyop, bool copy_values):
	Renderer2D(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
}

DrawVolume::~DrawVolume()
{
}

ProcessorBranchType DrawVolume::drawType()
{
	View* view;
	Referenced* ref;
	if (!getRvalu("view", &ref))
		return PBT_STOP;
	view = dynamic_cast<View*>(ref);
	if (!view)
		return PBT_STOP;
	long blend_mode;
	view->getValue("blend mode", blend_mode);
	switch (blend_mode)
	{
	case RBM_LAYERED:
	case RBM_COMPOSITE:
		return PBT_01;
	case RBM_DEPTH:
		return PBT_02;
	}
	return PBT_01;
}

