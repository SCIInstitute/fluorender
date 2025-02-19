/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#ifndef FL_EVENTHANDLER_HPP
#define FL_EVENTHANDLER_HPP

#include <Referenced.hpp>
#include <Event.hpp>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <functional>

namespace fluo
{
    typedef std::function<void(Event&)> eventFunctionType;

    class EventHandler : public Referenced
	{
	public:
		EventHandler() : Referenced(),
            node_added_function_(nullptr),
            node_removed_function_(nullptr),
            default_value_changing_function_(nullptr),
            default_value_changed_function_(nullptr),
			before_run_function_(nullptr),
			after_run_function_(nullptr),
			process_func_(nullptr)
		{}

		virtual const char* className() const { return "EventHandler"; }

		void setValueChangingFunction(const std::string &name, eventFunctionType func)
		{
			value_changing_functions_.insert(std::make_pair(name, func));
		}

		void setValueChangedFunction(const std::string &name, eventFunctionType func)
		{
			value_changed_functions_.insert(std::make_pair(name, func));
		}

		void setDefaultValueChangingFunction(eventFunctionType func)
		{
			default_value_changing_function_ = func;
		}

		void setDefaultValueChangedFunction(eventFunctionType func)
		{
			default_value_changed_function_ = func;
		}

		void setNodeAddedFunction(eventFunctionType func)
		{
			node_added_function_ = func;
		}

		void setNodeRemovedFunction(eventFunctionType func)
		{
			node_removed_function_ = func;
		}

		void setBeforeRunFunction(eventFunctionType func)
		{
			before_run_function_ = func;
		}

		void setAfterRunFunction(eventFunctionType func)
		{
			after_run_function_ = func;
		}

		void setProcessFunction(eventFunctionType func)
		{
			process_func_ = func;
		}

		void onValueChanging(const std::string &name, Event& event) const
		{
			if (_hold ||
				!event.pass(const_cast<EventHandler*>(this)))
				return;

			if (default_value_changing_function_)
			{
				//default value changing function is always called here
				//this is to allow observers to a referenced value to be removed
				//otherwise, there is no value-related function so far
				default_value_changing_function_(event);
			}

			auto map_it = value_changing_functions_.find(name);
			if (map_it != value_changing_functions_.end() && !_hold &&
				event.pass(const_cast<EventHandler*>(this)))
			{
				auto map_val = map_it->second;
				//return map_val(std::forward<Args>(args)...);
				map_val(event);
			}
		}

        void onValueChanged(const std::string &name, Event& event) const
		{
			if (_hold ||
				!event.pass(const_cast<EventHandler*>(this)))
				return;

            auto map_it = value_changed_functions_.find(name);
			if (map_it != value_changed_functions_.end())
			{
				auto map_val = map_it->second;
				//return map_val(std::forward<Args>(args)...);
                map_val(event);
			}
			
			if (default_value_changed_function_)
			{
				//here, the default value changed function is used when
				//any named value is changed, for example, when the render
				//view needs update
				default_value_changed_function_(event);
			}
		}

        void onNodeAdded(Event& event) const
		{
			if (node_added_function_)
			{
				node_added_function_(event);
			}
		}

        void onNodeRemoved(Event& event) const
		{
			if (node_removed_function_)
			{
				node_removed_function_(event);
			}
		}

	public:
		//for processor
		eventFunctionType before_run_function_;
		eventFunctionType after_run_function_;
		eventFunctionType process_func_;
	protected:
		std::unordered_map<std::string, eventFunctionType> value_changing_functions_;
		std::unordered_map<std::string, eventFunctionType> value_changed_functions_;
		eventFunctionType default_value_changing_function_;
		eventFunctionType default_value_changed_function_;
		//for scenegraph change
		eventFunctionType node_added_function_;
		eventFunctionType node_removed_function_;
	};
}
#endif
