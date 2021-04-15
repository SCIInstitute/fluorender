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

#include "ProcessorVisitor.hpp"
#include "ProcessorNode.hpp"
#include "Processor.hpp"

using namespace fluo;

void ProcessorVisitor::apply(Node& node)
{
	ProcessorNode* pn = dynamic_cast<ProcessorNode*>(&node);
	Event event;
	if (pn)
	{
		if (pn->before_run_function_)
			pn->before_run_function_(event);
		if (pn->process_func_)
			pn->process_func_(event);
	}
	traverse(node);
	if (pn)
	{
		if (pn->after_run_function_)
			pn->after_run_function_(event);
	}
}

void ProcessorVisitor::apply(Group& group)
{
	Processor* prc = dynamic_cast<Processor*>(&group);
	if (prc)
	{
		bool condition = prc->condition();
		if (condition)
			setTraversalMask(PBT_TRUE);
		else
			setTraversalMask(PBT_FALSE);
	}

	Event event;
	if (prc)
	{
		if (prc->before_run_function_)
			prc->before_run_function_(event);
		if (prc->process_func_)
			prc->process_func_(event);
	}
	traverse(group);
	if (prc)
	{
		if (prc->after_run_function_)
			prc->after_run_function_(event);
	}
}

