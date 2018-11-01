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
//modified from:
//https://stackoverflow.com/questions/2136998/using-a-stl-map-of-function-pointers

#ifndef _EVENTHANDLER_H_
#define _EVENTHANDLER_H_

#include <Flobject/Referenced.h>
#include <Flobject/Event.h>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <functional>

namespace FL
{
	typedef std::function<void(Event&)> eventFunctionType;

	class EventHandler :public Referenced
	{
	public:
		EventHandler() : Referenced() {}

		virtual const char* className() const { return "EventHandler"; }

		void setBeforeFunction(const std::string &name, eventFunctionType func)
		{
			before_functions_.insert(std::make_pair(name, func));
		}

		void setAfterFunction(const std::string &name, eventFunctionType func)
		{
			after_functions_.insert(std::make_pair(name, func));
		}

		void onBefore(const std::string &name, Event& event) const
		{
			auto map_it = before_functions_.find(name);
			if (map_it != before_functions_.end() && !_hold &&
				event.pass(const_cast<EventHandler*>(this)))
			{
				auto map_val = map_it->second;
				//return map_val(std::forward<Args>(args)...);
				event.push(const_cast<EventHandler*>(this));
				map_val(event);
				event.pop();
			}
		}

		void onAfter(const std::string &name, Event& event) const
		{
			auto map_it = after_functions_.find(name);
			if (map_it != after_functions_.end() && !_hold &&
				event.pass(const_cast<EventHandler*>(this)))
			{
				auto map_val = map_it->second;
				//return map_val(std::forward<Args>(args)...);
				event.push(const_cast<EventHandler*>(this));
				map_val(event);
				event.pop();
			}
		}

	protected:
		std::unordered_map<std::string, eventFunctionType> before_functions_;
		std::unordered_map<std::string, eventFunctionType> after_functions_;
	};
}
#endif