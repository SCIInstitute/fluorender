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
#ifndef FL_PROCESSORFACTORY
#define FL_PROCESSORFACTORY

#include <Flobject/ObjectFactory.h>
#include <Processor/Processor.h>

namespace FLR
{
	class ClipPlaneRenderer;
}
namespace FL
{
	class ProcessorFactory : public ObjectFactory
	{
	public:
		ProcessorFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{ return dynamic_cast<const ProcessorFactory*>(obj) != NULL; }

		virtual const char* className() const { return "ProcessorFactory"; }

		virtual void createDefault() {}

		virtual Processor* getDefault() { return 0; }

		virtual Processor* build(const std::string &exp) { return 0; }

		virtual Processor* clone(Processor*) { return 0; }

		virtual Processor* clone(const unsigned int) { return 0; }

		inline virtual Processor* get(size_t i)
		{
			return dynamic_cast<Processor*>(ObjectFactory::get(i));
		}

		inline virtual const Processor* get(size_t i) const
		{
			return dynamic_cast<Processor*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual Processor* find(const unsigned int id)
		{
			return dynamic_cast<Processor*>(ObjectFactory::find(id));
		}

		inline virtual Processor* findFirst(const std::string &name)
		{
			return dynamic_cast<Processor*>(ObjectFactory::findFirst(name));
		}

		inline virtual Processor* findLast(const std::string &name)
		{
			return dynamic_cast<Processor*>(ObjectFactory::findLast(name));
		}

		FLR::ClipPlaneRenderer* getOrAddClipPlaneRenderer(const std::string &name);

	protected:
		virtual ~ProcessorFactory();

	};

}

#endif//FL_PROCESSORFACTORY