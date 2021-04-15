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
#ifndef RENDERER3D_FACTORY_HPP
#define RENDERER3D_FACTORY_HPP

#include <ProcessorNodeFactory.hpp>
#include "Renderer3D.hpp"

namespace fluo
{
	class ClipPlaneRenderer;
	class Renderer3DFactory : public ProcessorNodeFactory
	{
	public:
		Renderer3DFactory();

		virtual bool isSameKindAs(const Object* obj) const
        { return dynamic_cast<const Renderer3DFactory*>(obj) != nullptr; }

		virtual const char* className() const { return "Renderer3DFactory"; }

		virtual void createDefault() {}

        virtual Renderer3D* getDefault() { return nullptr; }

        virtual Renderer3D* build(Renderer3D* renderer = nullptr) { return nullptr; }

        virtual Renderer3D* clone(Renderer3D*) { return nullptr; }

        virtual Renderer3D* clone(const unsigned int) { return nullptr; }

		inline virtual Renderer3D* get(size_t i)
		{
			return dynamic_cast<Renderer3D*>(ObjectFactory::get(i));
		}

		inline virtual const Renderer3D* get(size_t i) const
		{
			return dynamic_cast<Renderer3D*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual Renderer3D* find(const unsigned int id)
		{
			return dynamic_cast<Renderer3D*>(ObjectFactory::find(id));
		}

		inline virtual Renderer3D* findFirst(const std::string &name)
		{
			return dynamic_cast<Renderer3D*>(ObjectFactory::findFirst(name));
		}

		inline virtual Renderer3D* findLast(const std::string &name)
		{
			return dynamic_cast<Renderer3D*>(ObjectFactory::findLast(name));
		}

		ClipPlaneRenderer* getOrAddClipPlaneRenderer(const std::string &name);

	protected:
		virtual ~Renderer3DFactory();

	};

}

#endif//RENDERER3D_FACTORY_HPP
