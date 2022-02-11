/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef MESHFACTORY_HPP
#define MESHFACTORY_HPP

#include <ObjectFactory.hpp>
#include <MeshData.hpp>

namespace fluo
{
	class MeshFactory : public ObjectFactory
	{
	public:
		MeshFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const MeshFactory*>(obj) != NULL;
		}

		virtual const char* className() const { return "MeshFactory"; }

		virtual void createDefault();

		virtual void setEventHandler(MeshData* md);

		virtual MeshData* getDefault()
		{
			return dynamic_cast<MeshData*>(ObjectFactory::getDefault());
		}

		virtual MeshData* build(MeshData* md = 0);

		virtual MeshData* clone(MeshData*);

		virtual MeshData* clone(const unsigned int);

		inline virtual MeshData* get(size_t i)
		{
			return dynamic_cast<MeshData*>(ObjectFactory::get(i));
		}

		inline virtual const MeshData* get(size_t i) const
		{
			return dynamic_cast<MeshData*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual MeshData* getLast()
		{
			return dynamic_cast<MeshData*>(const_cast<Object*>(ObjectFactory::getLast()));
		}

		inline virtual MeshData* find(const unsigned int id)
		{
			return dynamic_cast<MeshData*>(ObjectFactory::find(id));
		}

		inline virtual MeshData* findFirst(const std::string &name)
		{
			return dynamic_cast<MeshData*>(ObjectFactory::findFirst(name));
		}

		inline virtual MeshData* findLast(const std::string &name)
		{
			return dynamic_cast<MeshData*>(ObjectFactory::findLast(name));
		}

		//also builds volume group, whose typical use is to sync properties for volumes
		//the volume group copies properties from the given volume
		MeshGroup* buildGroup(MeshData* md = 0);

	protected:
		virtual ~MeshFactory();
	};
}

#endif//_MESHFACTORY_H_
