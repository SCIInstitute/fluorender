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
#ifndef PROCESSORNODE_FACTORY_HPP
#define PROCESSORNODE_FACTORY_HPP

#include <ObjectFactory.hpp>
#include "ProcessorNode.hpp"

namespace fluo
{
	class ProcessorNodeFactory : public ObjectFactory
	{
	public:
		ProcessorNodeFactory();

		virtual bool isSameKindAs(const Object* obj) const
        { return dynamic_cast<const ProcessorNodeFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "ProcessorNodeFactory"; }

		virtual void createDefault() {}

        virtual ProcessorNode* getDefault() { return nullptr; }

        virtual ProcessorNode* build(ProcessorNode* processor = nullptr) { return nullptr; }

        virtual ProcessorNode* clone(ProcessorNode*) { return nullptr; }

        virtual ProcessorNode* clone(const unsigned int) { return nullptr; }

		inline virtual ProcessorNode* get(size_t i)
		{
			return dynamic_cast<ProcessorNode*>(ObjectFactory::get(i));
		}

		inline virtual const ProcessorNode* get(size_t i) const
		{
			return dynamic_cast<ProcessorNode*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual ProcessorNode* find(const unsigned int id)
		{
			return dynamic_cast<ProcessorNode*>(ObjectFactory::find(id));
		}

		inline virtual ProcessorNode* findFirst(const std::string &name)
		{
			return dynamic_cast<ProcessorNode*>(ObjectFactory::findFirst(name));
		}

		inline virtual ProcessorNode* findLast(const std::string &name)
		{
			return dynamic_cast<ProcessorNode*>(ObjectFactory::findLast(name));
		}

		//FLR::ClipPlaneRenderer* getOrAddClipPlaneRenderer(const std::string &name);

	protected:
		virtual ~ProcessorNodeFactory();

	};

}

#endif//PROCESSORNODE_FACTORY_HPP
