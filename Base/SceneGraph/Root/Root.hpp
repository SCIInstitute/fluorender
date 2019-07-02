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

#ifndef ROOT_HPP
#define ROOT_HPP

#include <Groups/Group.hpp>

namespace fluo
{
	enum SortMethod
	{
		SortNone = 0,//no sorting
		SortAscend = 1,//ascending
		SortDescend = 2//descending
	};

	class Root : public Group
	{
	public:
		Root();
		Root(const Root& root, const CopyOp& copyop = CopyOp::SHALLOW_COPY) {}

		virtual Object* clone(const CopyOp& copyop) const
		{
			return 0;
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const Root*>(obj) != NULL;
		}

		virtual const char* className() const { return "Root"; }

		virtual Root* asRoot() { return this; }
		virtual const Root* asRoot() const { return this; }

		virtual bool addChild(Node* child);
		virtual bool insertChild(size_t index, Node* child);
		virtual bool removeChildren(size_t pos, size_t num);
		virtual bool setChild(size_t i, Node* node);

	protected:
		virtual ~Root()
		{
		}
	};
}

#endif//_ROOT_H_
