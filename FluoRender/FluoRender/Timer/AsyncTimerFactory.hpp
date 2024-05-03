/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef ASYNCTIMERFACTORY_HPP
#define ASYNCTIMERFACTORY_HPP

#include <ObjectFactory.hpp>
#include <AsyncTimer.hpp>

namespace fluo
{
	class AsyncTimerFactory : public ObjectFactory
	{
	public:
		AsyncTimerFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const AsyncTimerFactory*>(obj) != NULL;
		}

		virtual const char* className() const { return "AsyncTimerFactory"; }

		virtual void createDefault();

		virtual AsyncTimer* build(AsyncTimer* at = 0);

		virtual AsyncTimer* clone(AsyncTimer*);

		virtual AsyncTimer* clone(const unsigned int);

		inline virtual AsyncTimer* get(size_t i)
		{
			return dynamic_cast<AsyncTimer*>(ObjectFactory::get(i));
		}

		inline virtual const AsyncTimer* get(size_t i) const
		{
			return dynamic_cast<AsyncTimer*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual AsyncTimer* getLast()
		{
			return dynamic_cast<AsyncTimer*>(const_cast<Object*>(ObjectFactory::getLast()));
		}

		inline virtual AsyncTimer* find(const unsigned int id)
		{
			return dynamic_cast<AsyncTimer*>(ObjectFactory::find(id));
		}

		inline virtual AsyncTimer* findFirst(const std::string &name)
		{
			return dynamic_cast<AsyncTimer*>(ObjectFactory::findFirst(name));
		}

		inline virtual AsyncTimer* findLast(const std::string &name)
		{
			return dynamic_cast<AsyncTimer*>(ObjectFactory::findLast(name));
		}

		inline void stopAll()
		{
			for (size_t i = 0; i < getNum(); ++i)
			{
				AsyncTimer* t = get(i);
				t->stop();
			}
		}

	protected:
		virtual ~AsyncTimerFactory();
	};
}

#endif//ASYNCTIMERFACTORY_HPP
