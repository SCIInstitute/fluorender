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
#ifndef RENDERVIEWFACTORY_HPP
#define RENDERVIEWFACTORY_HPP

#include <ObjectFactory.hpp>
#include <Renderview.hpp>

namespace fluo
{
	class RenderviewFactory : public ObjectFactory
	{
	public:
		RenderviewFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const RenderviewFactory*>(obj) != NULL;
		}

		virtual const char* className() const { return "RenderviewFactory"; }

		virtual void createDefault();

		virtual Renderview* build(Renderview* view = 0);

		virtual Renderview* clone(Renderview*);

		virtual Renderview* clone(const unsigned int);

		inline virtual Renderview* get(size_t i)
		{
			return dynamic_cast<Renderview*>(ObjectFactory::get(i));
		}

		inline virtual const Renderview* get(size_t i) const
		{
			return dynamic_cast<Renderview*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual Renderview* getLast()
		{
			return dynamic_cast<Renderview*>(const_cast<Object*>(ObjectFactory::getLast()));
		}

		inline virtual Renderview* find(const unsigned int id)
		{
			return dynamic_cast<Renderview*>(ObjectFactory::find(id));
		}

		inline virtual Renderview* findFirst(const std::string &name)
		{
			return dynamic_cast<Renderview*>(ObjectFactory::findFirst(name));
		}

		inline virtual Renderview* findLast(const std::string &name)
		{
			return dynamic_cast<Renderview*>(ObjectFactory::findLast(name));
		}

	protected:
		virtual ~RenderviewFactory();
	};
}

#endif//RENDERVIEWFACTORY_HPP
