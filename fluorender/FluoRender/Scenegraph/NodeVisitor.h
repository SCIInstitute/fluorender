/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#ifndef _NODEVISITOR_H_
#define _NODEVISITOR_H_

#include <Scenegraph/Node.h>

namespace FL
{
	class NodeVisitor : public virtual Referenced
	{
	public:

		enum TraversalMode
		{
			TRAVERSE_NONE,
			TRAVERSE_PARENTS,
			TRAVERSE_ALL_CHILDREN,
			TRAVERSE_ACTIVE_CHILDREN,
		};

		enum VisitorType
		{
			NODE_VISITOR = 0,
			UPDATE_VISITOR,
		};

		NodeVisitor(TraversalMode tm = TRAVERSE_NONE);
		NodeVisitor(VisitorType type, TraversalMode tm = TRAVERSE_NONE);

		virtual ~NodeVisitor();

		virtual const char* className() const { return "NodeVisitor"; }

		virtual void reset() {}

	protected:

		VisitorType m_visitor_type;
		unsigned int m_traversal_number;
		TraversalMode m_traversal_mode;
	};
}

#endif//_NODEVISITOR_H_