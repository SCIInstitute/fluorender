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
#ifndef _STREAMPROCESSOR_H_
#define _STREAMPROCESSOR_H_

#include <Processor/Processor.h>

namespace FL
{
	class StreamProcessor : public FL::Processor
	{
	public:

		StreamProcessor();

		StreamProcessor(const StreamProcessor& renderer, const FL::CopyOp& copyop = FL::CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual StreamProcessor* clone(const FL::CopyOp& copyop) const { return new StreamProcessor(*this, copyop); };

		virtual bool isSameKindAs(const StreamProcessor*) const { return true; }

		virtual const char* className() const { return "StreamProcessor"; }

		virtual bool run(FL::Event& event = FL::Event())
		{
			bool result = true;
			for (auto it : processors_)
				if (it)
					result &= it->run(event);
			return result;
		}

		//sub processors
		virtual bool addProcessor(Processor* processor);
		//virtual bool insertProcessor
		inline bool removeProcessor(Processor* processor)
		{
			size_t pos = getProcessorIndex(processor);
			if (pos < processors_.size())
				return removeProcessors(pos, 1);
		}
		inline bool removeProcessor(size_t pos, size_t num = 1)
		{
			if (pos < processors_.size())
				return removeProcessors(pos, num);
		}
		inline bool removeAllProcessors()
		{
			return removeProcessors(0, getNumProcessors());
		}

	protected:
		ProcessorList processors_;

		virtual void setupInputs();
		virtual void setupOutputs();

	private:
		~StreamProcessor();
	};
}
#endif//_STREAMPROCESSOR_H_
