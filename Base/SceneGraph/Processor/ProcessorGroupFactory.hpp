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
#ifndef PROCESSORGROUP_FACTORY_HPP
#define PROCESSORGROUP_FACTORY_HPP

#include <ObjectFactory.hpp>
#include "ProcessorGroup.hpp"

namespace fluo
{
	class ProcessorGroupFactory : public ObjectFactory
	{
	public:
		ProcessorGroupFactory();

		virtual bool isSameKindAs(const Object* obj) const
        { return dynamic_cast<const ProcessorGroupFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "ProcessorGroupFactory"; }

		virtual void createDefault() {}

        virtual ProcessorGroup* getDefault() { return nullptr; }

        virtual ProcessorGroup* build(ProcessorGroup* group = nullptr) { return nullptr; }

        virtual ProcessorGroup* clone(ProcessorGroup*) { return nullptr; }

        virtual ProcessorGroup* clone(const unsigned int) { return nullptr; }

		inline virtual ProcessorGroup* get(size_t i)
		{
			return dynamic_cast<ProcessorGroup*>(ObjectFactory::get(i));
		}

		inline virtual const ProcessorGroup* get(size_t i) const
		{
			return dynamic_cast<ProcessorGroup*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual ProcessorGroup* find(const unsigned int id)
		{
			return dynamic_cast<ProcessorGroup*>(ObjectFactory::find(id));
		}

		inline virtual ProcessorGroup* findFirst(const std::string &name)
		{
			return dynamic_cast<ProcessorGroup*>(ObjectFactory::findFirst(name));
		}

		inline virtual ProcessorGroup* findLast(const std::string &name)
		{
			return dynamic_cast<ProcessorGroup*>(ObjectFactory::findLast(name));
		}

	protected:
		virtual ~ProcessorGroupFactory();

	};

}

#endif//PROCESSORGROUP_FACTORY_HPP
