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

#ifndef COPYOP_HPP
#define COPYOP_HPP

#include <vector>
#include <set>
#include <EventHandler.hpp> // Not sure why, but this is needed in order
                                         // to get Object.hpp to recognicze EventHandler
                                         // as a class.

namespace fluo
{
	class Referenced;
	class Object;
	class Node;
	class Group;
	class Value;
    class ValueSet;
    //class ref_ptr;

    class CopyOp
	{
	public:
		enum Options
		{
			SHALLOW_COPY = 0,
			DEEP_COPY_OBJECTS = 1<<0,
			DEEP_COPY_NODES = 1<<1,
			DEEP_COPY_VALUES = 1<<2,
			DEEP_COPY_ALL = 0x7FFFFFFF
		};

		typedef unsigned int CopyFlags;

		inline CopyOp(CopyFlags flags = SHALLOW_COPY) :
			m_flags(flags) {}
		virtual ~CopyOp() {}

		void setCopyFlags(CopyFlags flags) { m_flags = flags; }
		CopyFlags getCopyFlags() const { return m_flags; }

		virtual Referenced* operator() (const Referenced* ref) const;
		virtual Object* operator() (const Object* obj) const;
		virtual Node* operator() (const Node* node) const;

	protected:
		CopyFlags m_flags;
	};
}
#endif//COPYOP_HPP
