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
#ifndef STOPWATCHFACTORY_HPP
#define STOPWATCHFACTORY_HPP

#include <ObjectFactory.hpp>
#include <StopWatch.hpp>

namespace fluo
{
	class StopWatchFactory : public ObjectFactory
	{
	public:
		StopWatchFactory();

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const StopWatchFactory*>(obj) != NULL;
		}

		virtual const char* className() const { return "StopWatchFactory"; }

		virtual void createDefault();

		virtual StopWatch* build(StopWatch* ann = 0);

		virtual StopWatch* clone(StopWatch*);

		virtual StopWatch* clone(const unsigned int);

		inline virtual StopWatch* get(size_t i)
		{
			return dynamic_cast<StopWatch*>(ObjectFactory::get(i));
		}

		inline virtual const StopWatch* get(size_t i) const
		{
			return dynamic_cast<StopWatch*>(const_cast<Object*>(ObjectFactory::get(i)));
		}

		inline virtual StopWatch* getLast()
		{
			return dynamic_cast<StopWatch*>(const_cast<Object*>(ObjectFactory::getLast()));
		}

		inline virtual StopWatch* find(const unsigned int id)
		{
			return dynamic_cast<StopWatch*>(ObjectFactory::find(id));
		}

		inline virtual StopWatch* findFirst(const std::string &name)
		{
			return dynamic_cast<StopWatch*>(ObjectFactory::findFirst(name));
		}

		inline virtual StopWatch* findLast(const std::string &name)
		{
			return dynamic_cast<StopWatch*>(ObjectFactory::findLast(name));
		}

	protected:
		virtual ~StopWatchFactory();
	};
}

#endif//STOPWATCHFACTORY_HPP
