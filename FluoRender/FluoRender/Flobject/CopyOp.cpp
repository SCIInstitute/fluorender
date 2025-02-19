/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <CopyOp.hpp>
#include <Referenced.hpp>
#include <CopyOp.hpp>
#include <Node.hpp>

using namespace fluo;

Referenced* CopyOp::operator() (const Referenced* ref) const
{
	return const_cast<Referenced*>(ref);
}

Object* CopyOp::operator() (const Object* obj) const
{
	if (obj && m_flags&DEEP_COPY_OBJECTS)
		return clone(obj, *this);
	else
		return const_cast<Object*>(obj);
}

Node* CopyOp::operator() (const Node* node) const
{
	if (node && m_flags&DEEP_COPY_NODES)
		return clone(node, *this);
	else
		return const_cast<Node*>(node);
}

