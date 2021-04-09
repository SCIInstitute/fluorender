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
#ifndef RENDERGROUP_FACTORY_HPP
#define RENDERGROUP_FACTORY_HPP

#include <Processor/ProcessorGroupFactory.hpp>
#include "RendererGroup.hpp"

namespace fluo
{
	class RendererGroupFactory : public ProcessorGroupFactory
	{
	public:
		RendererGroupFactory();

		virtual bool isSameKindAs(const Object* obj) const
        { return dynamic_cast<const RendererGroupFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "RendererGroupFactory"; }

		virtual void createDefault() {}

        virtual RendererGroup* getDefault() { return nullptr; }

        virtual RendererGroup* build(RendererGroup* group = nullptr) { return nullptr; }

        virtual RendererGroup* clone(RendererGroup*) { return nullptr; }

        virtual RendererGroup* clone(const unsigned int) { return nullptr; }

		inline virtual RendererGroup* get(size_t i)
		{
			return dynamic_cast<RendererGroup*>(ObjectFactory::get(i));
		}

		inline virtual const RendererGroup* get(size_t i) const
		{
			return dynamic_cast<RendererGroup*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual RendererGroup* find(const unsigned int id)
		{
			return dynamic_cast<RendererGroup*>(ObjectFactory::find(id));
		}

		inline virtual RendererGroup* findFirst(const std::string &name)
		{
			return dynamic_cast<RendererGroup*>(ObjectFactory::findFirst(name));
		}

		inline virtual RendererGroup* findLast(const std::string &name)
		{
			return dynamic_cast<RendererGroup*>(ObjectFactory::findLast(name));
		}

	protected:
		virtual ~RendererGroupFactory();

	};

}

#endif//RENDERGROUP_FACTORY_HPP
