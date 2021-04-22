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

#include "DrawView.hpp"

using namespace fluo;

DrawView::DrawView():
	Renderer2D()
{
	setConditionFunction(std::bind(&DrawView::drawType,
		this));
}

DrawView::DrawView(const DrawView& renderer, const CopyOp& copyop, bool copy_values):
	Renderer2D(renderer, copyop, false)
{
	if (copy_values)
		copyValues(renderer, copyop);
	setConditionFunction(std::bind(&DrawView::drawType,
		this));
}

DrawView::~DrawView()
{
}

ProcessorBranchType DrawView::drawType()
{
	unsigned int result = PBT_STOP;
	bool bval;
	if (getValue("draw background", bval) && bval)
		result |= (unsigned int)PBT_01;
	if (getValue("draw grid", bval) && bval)
		result |= (unsigned int)PBT_02;
	if (getValue("draw bound", bval) && bval)
		result |= (unsigned int)PBT_03;
	if (getValue("draw clipplane", bval) && bval)
		result |= (unsigned int)PBT_04;
	if (getValue("draw ruler", bval) && bval)
		result |= (unsigned int)PBT_05;
	if (getValue("draw track", bval) && bval)
		result |= (unsigned int)PBT_06;
	if (getValue("draw annotation", bval) && bval)
		result |= (unsigned int)PBT_07;
	if (getValue("draw cell", bval) && bval)
		result |= (unsigned int)PBT_08;
	return ProcessorBranchType(result);
}