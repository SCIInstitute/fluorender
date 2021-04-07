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
#ifndef PROCESSORGROUP_HPP
#define PROCESSORGROUP_HPP

#include <Group.hpp>

namespace fluo
{
class ProcessorGroup : public Group
{
public:

	ProcessorGroup();

	ProcessorGroup(const ProcessorGroup& group, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

	virtual ProcessorGroup* clone(const CopyOp& copyop) const { return new ProcessorGroup(*this, copyop); };

	virtual bool isSameKindAs(const ProcessorGroup*) const {return true;}

	virtual const char* className() const { return "ProcessorGroup"; }


protected:
	~ProcessorGroup();

protected:

};
}
#endif//PROCESSORGROUP_HPP
