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
#ifndef FL_PROCESSOR
#define FL_PROCESSOR 1

#include <Flobject/Object.h>
namespace FL
{
class ProcessorFactory;

class Processor : public Object
{
public:

	Processor();

	Processor(const Processor& processor, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

	virtual Processor* clone(const CopyOp& copyop) const { return new Processor(*this, copyop); };

	virtual bool isSameKindAs(const Processor*) const {return true;}

	virtual const char* className() const { return "Processor"; }

	virtual bool run(Event& event = Event()) { return true; }

	inline void copyInputValues(Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY)
	{
		ValueCollection names = obj.getValueNames();
		for (auto it = names.begin();
			it != names.end(); ++it)
		{
			auto find_it = inputs_.find(*it);
			if (find_it == inputs_.end())
				continue;
			Value* value = obj.getValue(*it);
			Value* new_value = 0;
			if (copyop.getCopyFlags() & CopyOp::DEEP_COPY_VALUES)
				new_value = value->clone();
			else
				new_value = value;
			replaceValue(new_value);
		}
	}

	inline void copyOutputValues(Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY)
	{
	}

protected:
	~Processor();

protected:
	ValueCollection inputs_;
	ValueCollection outputs_;

	virtual void setupInputs() {}
	virtual void setupOutputs() {}
};
}
#endif//FL_PROCESSOR
