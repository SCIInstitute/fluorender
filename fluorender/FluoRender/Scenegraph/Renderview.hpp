/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#ifndef RENDERVIEW_HPP
#define RENDERVIEW_HPP

#include <Group.hpp>

namespace fluo
{
	class Renderview : public Group
	{
	public:
		Renderview();
		Renderview(const Renderview& view, const CopyOp& copyop = CopyOp::SHALLOW_COPY);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new Renderview(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const Renderview*>(obj) != NULL;
		}

		virtual const char* className() const { return "Renderview"; }

		virtual Renderview* asRenderview() { return this; }
		virtual const Renderview* asRenderview() const { return this; }

	protected:
		virtual ~Renderview();
	};
}

#endif//RENDERVIEW_HPP