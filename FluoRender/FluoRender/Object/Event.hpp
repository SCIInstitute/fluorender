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
#ifndef FL_EVENT_HPP
#define FL_EVENT_HPP

#include <Referenced.hpp>


#include <limits>
#include <string>
#include <vector>
#include <algorithm>


namespace fluo
{
	class Value;
	class Object;
	class Event
	{
	public:
		enum NotifyOptions
		{
			NOTIFY_NONE = 0,//for:
			NOTIFY_SELF = 1 << 0,	//Object-EventHandler, handling value change (onValueChanging, onValueChanged)
									//value is contained in the object it self
			NOTIFY_OTHERS = 1 << 1,	//value is from somewhere else
			NOTIFY_VALUE = 1 << 2,//Value, setting and syncing
			NOTIFY_PARENT = 1 << 3,//Node-Group, pass event to parent
			NOTIFY_AGENT = 1 << 4,//InterfaceAgent, process notification
			NOTIFY_FACTORY = 1 << 5,//ObjectFactory, process notification
			NOTIFY_ALL = 0x7FFFFFFF
		};

		enum EventTypes
		{
			EVENT_DELETED = 1,
			EVENT_VALUE_CHANGING,
			EVENT_VALUE_CHANGED,
			EVENT_VALUE_ADDED,
			EVENT_SYNC_VALUE,
			EVENT_NODE_ADDED,
			EVENT_NODE_REMOVED
		};

		typedef unsigned int NotifyFlags;
		typedef unsigned int EventType;

		inline Event(NotifyFlags flags = NOTIFY_ALL) :
			id(0), type(0), m_flags(flags),
            sender(nullptr), origin(nullptr), value(nullptr),
            parent(nullptr), child(nullptr),
			m_cur_level(0), m_sum_level(0),
			m_limit(100),
			m_terminated(false)
		{}
		Event(const Event& event) :
			id(event.id), type(event.type),
			m_flags(event.m_flags),
			sender(event.sender),
			origin(event.origin),
			value(event.value),
			parent(event.parent),
			child(event.child),
			m_cur_level(0),
			m_sum_level(0),
			m_limit(event.m_limit),
			m_terminated(false)
		{
			if (!event.sender_chain.empty())
				sender_chain = event.sender_chain;
		}
		virtual ~Event() {}

		//for delete, changing, changed (general)
        void init(EventType tp, Referenced* sndr, bool push_sender = false);
		// value added, changing and changed
		void init(EventType tp, Referenced* sndr, Value* va, bool push_sender = false);
		// node added and removed
		void init(EventType tp, Object* prt, Object* chd, bool push_sender = false);

		void setNotifyFlags(NotifyFlags flags) { m_flags |= flags; }
		NotifyFlags getNotifyFlags() const { return m_flags; }

		inline Event& operator++()
		{
			++m_cur_level;
			++m_sum_level;
			return *this;
		}
		inline Event& operator--()
		{
			--m_cur_level;
			return *this;
		}
		inline void push(Referenced* sndr)
		{
			sender = sndr;
			sender_chain.push_back(sender);
			++(*this);
		}
		inline void pop()
		{
			if (!sender_chain.empty())
			{
				sender_chain.pop_back();
				--(*this);
			}
			if (!sender_chain.empty())
				sender = sender_chain.back();
			else
                sender = nullptr;
		}
		inline bool pass(Referenced* sndr, unsigned int limit = 0)
		{
			if (m_terminated)
				return false;
			if (!sender_chain.empty() &&
				sndr == sender_chain.back())
			{
				m_terminated = true;
				return false;
			}
			else
			{
				if (limit)
				{
					if (m_cur_level < limit)
						return true;
					else
					{
						m_terminated = true;
						return false;
					}
				}
				else
				{
					if (m_cur_level < m_limit)
						return true;
					else
					{
						m_terminated = true;
						return false;
					}
				}
			}
		}
        inline size_t count(Referenced* sndr)
		{
			return std::count(sender_chain.begin(), sender_chain.end(), sndr);
		}
		inline Referenced* penultimate()
		{
			if (sender_chain.size() > 1)
				return sender_chain[sender_chain.size() - 2];
			else
                return nullptr;
		}
		inline void reset()
		{
			m_terminated = false;
			m_cur_level = 0;
			m_sum_level = 0;
			sender_chain.clear();
			if (sender)
				push(sender);
		}

		unsigned int id;
		EventType type;
		Referenced* sender;
		Referenced* origin;
		Value* value;
		Object* parent;
		Object* child;
		std::string value_name;
		std::vector<Referenced*> sender_chain;

	protected:
		NotifyFlags m_flags;
		unsigned int m_cur_level;
		unsigned int m_sum_level;
		unsigned int m_limit;
		bool m_terminated;
	};
}

#endif//FL_EVENT_HPP
