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
#ifndef FL_EVENT
#define FL_EVENT

#include <Flobject/Referenced.h>
#include <limits>
#include <string>

namespace FL
{
	class Event
	{
	public:
		enum NotifyOptions
		{
			NOTIFY_NONE = 0,
			NOTIFY_SELF = 1 << 0,
			NOTIFY_OTHERS = 1 << 1,
			NOTIFY_VALUE = 1 << 2,
			NOTIFY_PARENT = 1 << 3,
			NOTIFY_AGENT = 1 << 4,
			NOTIFY_FACTORY = 1 << 5,
			NOTIFY_ALL = 0x7FFFFFFF
		};

		enum EventTypes
		{
			EVENT_DELETED = 1,
			EVENT_CHANGING,
			EVENT_CHANGED,
			EVENT_NODE_REMOVED,
			EVENT_NODE_ADDED,
		};

		typedef unsigned int NotifyFlags;
		typedef unsigned int EventType;

		inline Event(Referenced* ptr = 0, NotifyFlags flags = NOTIFY_ALL) :
			m_flags(flags), m_level(0),
			m_limit(std::numeric_limits<unsigned int>::max()),
			type(0),
			sender(ptr), origin(0), value(0),
			parent(0), child(0) {}
		virtual ~Event() {}

		void setNotifyFlags(NotifyFlags flags) { m_flags = flags; }
		NotifyFlags getNotifyFlags() const { return m_flags; }

		inline Event& operator++()
		{
			m_level++;
			return *this;
		}
		bool pass(unsigned int limit = 0)
		{
			if (limit)
				return m_level < limit;
			else
				return m_level < m_limit;
		}

		EventType type;
		Referenced* sender;
		Referenced* origin;
		Referenced* value;
		Referenced* parent;
		Referenced* child;
		std::string value_name;

	protected:
		NotifyFlags m_flags;
		unsigned int m_level;
		unsigned int m_limit;
	};
}

#endif//FL_EVENT