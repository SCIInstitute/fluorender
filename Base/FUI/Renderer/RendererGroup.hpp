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
#ifndef RENDERERGROUP_HPP
#define RENDERERGROUP_HPP

#include <Processor/ProcessorGroup.hpp>

namespace fluo
{
	class RendererGroup : public ProcessorGroup
	{
	public:

		RendererGroup();

		RendererGroup(const RendererGroup& group, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual RendererGroup* clone(const CopyOp& copyop) const { return new RendererGroup(*this, copyop); };

		virtual bool isSameKindAs(const RendererGroup*) const {return true;}

		virtual const char* className() const { return "RendererGroup"; }

	protected:
		~RendererGroup();

	protected:
	};
}
#endif//RENDERERGROUP_HPP
