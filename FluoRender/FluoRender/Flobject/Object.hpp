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
#ifndef FL_OBJECT_HPP
#define FL_OBJECT_HPP

#include <ref_ptr.hpp>
#include <Observer.hpp>
#include <CopyOp.hpp>
#include <Value.hpp>

#include <string>
#include <vector>
#include <functional>

namespace FluoUI
{
	class InterfaceAgent;
}

namespace fluo
{
	class Object;
	typedef std::vector<Object*> ObjectList;
	class ObjectFactory;

	class Object : public EventHandler, public Observer
	{
	public:

		Object();

		Object(const Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

		virtual Object* clone(const CopyOp& copyop) const { return new Object(*this, copyop); }

		virtual bool isSameKindAs(const Object*) const { return true; }

		virtual const char* className() const { return "Object"; }

		inline void setId(const unsigned int id) { _id = id; }

		inline unsigned int getId() const { return _id; }

		inline void setName(const std::string& name) { m_name = name; }

		inline const char* getName() const { return m_name.c_str(); }

		virtual unsigned int getPriority() const { return 999; }

		//observer
		virtual void objectDeleted(Event& event);
		virtual void processNotification(Event& event);

		inline void copyValues(const Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY)
		{
			if (!obj._value_set)
				return;
			for (auto it = obj._value_set->getValues().begin();
				it != obj._value_set->getValues().end(); ++it)
			{
				if (getValuePointer(it->second->getName()))
					continue;
				Value* value = nullptr;
				if (copyop.getCopyFlags() & CopyOp::DEEP_COPY_VALUES)
					value = it->second->clone();
				else
					value = it->second.get();
				ref_ptr<Value> pvalue(value);
				addValue(value);
			}
		}

		inline void clearValues()
		{
			if (!_value_set)
				return;
			//remove observer
			for (auto it = _value_set->getValues().begin();
				it != _value_set->getValues().end(); ++it)
			{
				Value * value = it->second.get();
				if (value)
					value->removeObserver(this);
			}
			_value_set->clear();
		}

		//compare by values
		//vc: a subset of values to compare
		inline bool cmpValues(const Object& obj, const ValueCollection& vc = {})
		{
			if (std::string(className()) != obj.className())
				return false;
			if (!_value_set || obj._value_set)
				return false;
			for (auto it = _value_set->getValues().begin();
				it != _value_set->getValues().end(); ++it)
			{
				Value* value1 = it->second.get();
				if (!value1)
					continue;
				if (!vc.empty() &&
					vc.find(value1->getName()) == vc.end())
					continue;
				Value* value2 = const_cast<Object&>(obj).getValuePointer(value1->getName());
				if (!value2)
					return false;
				if (*value1 != *value2)
					return false;
			}
			return true;
		}

		bool addValue(ValueTuple &vt)
		{
			if (_value_set)
			{
				bool result = _value_set->addValue(vt);
				if (result)
				{
					std::string name = std::get<0>(vt);
					Value* vs_value = _value_set->findValue(name);
					if (vs_value)
					{
						vs_value->addObserver(this);
						Event event;
						event.init(Event::EVENT_VALUE_ADDED, this, vs_value, true);
						vs_value->notify(event);
					}
				}
				return result;
			}
			return false;
		}
		template<typename V>
		bool addValue(const string &name, const V value)
		{
			if (_value_set)
			{
				bool result = _value_set->addValue(name, value);
				if (result)
				{
					Value* vs_value = _value_set->findValue(name);
					if (vs_value)
					{
						vs_value->addObserver(this);
						Event event;
						event.init(Event::EVENT_VALUE_ADDED,
							this, vs_value, true);
						vs_value->notify(event);
					}
				}
				return result;
			}
			else
				return false;
		}
		bool addRvalu(const string &name, Referenced* value)
		{
			if (_value_set)
			{
				bool result = _value_set->addRvalu(name, value);
				if (result)
				{
					Value* vs_value = _value_set->findValue(name);
					if (vs_value)
					{
						vs_value->addObserver(this);
						Event event;
						event.init(Event::EVENT_VALUE_ADDED,
							this, vs_value, true);
						vs_value->notify(event);
					}
				}
				return result;
			}
			else
				return false;
		}

		template<typename V>
		bool setValue(const string &name, const V &value)
		{
			Event event;
			return setValue(name, value, event);
		}
		template<typename V>
		bool setValue(const string &name, const V &value, Event &event)
		{
			V old_value;
			if (getValue(name, old_value) && value != old_value)
			{
				bool result = false;
				if (_value_set)
				{
					if (!event.sender)
						event.init(Event::EVENT_VALUE_CHANGING,
							this, getValuePointer(name), true);
					else
						event.push(this);
					result = _value_set->setValue(name, value, event);
					event.pop();
				}
				return result;
			}
			return false;
		}
		bool setValueTuple(ValueTuple& vt)
		{
			Event event;
			return setValueTuple(vt, event);
		}
		bool setValueTuple(ValueTuple& vt, Event& event)
		{
			ValueTuple old_vt;
			std::string name = std::get<0>(vt);
			std::get<0>(old_vt) = name;
			Value* value = getValuePointer(name);
			if (getValue(old_vt) && vt != old_vt)
			{
				bool result = false;
				if (_value_set)
				{
					if (!event.sender)
						event.init(Event::EVENT_VALUE_CHANGING, this, value, true);
					result = _value_set->setValue(vt, event);
				}
				return result;
			}
			return false;
		}
		bool setRvalu(const std::string& name, Referenced* value)
		{
			Event event;
			return setRvalu(name, value, event);
		}
		bool setRvalu(const std::string& name, Referenced* value, Event& event)
		{
			Referenced* old_value;
			if (getRvalu(name, &old_value) && value != old_value)
			{
				bool result = false;
				if (_value_set)
				{
					if (!event.sender)
						event.init(Event::EVENT_VALUE_CHANGING,
							this, getValuePointer(name), true);
					else
						event.push(this);
					result = _value_set->setRvalu(name, value, event);
					event.pop();
				}
				return result;
			}
			return false;
		}
		template<typename V>
		bool addSetValue(const std::string& name, const V &value)
		{
			if (addValue(name, value))
				return true;
			else
				return setValue(name, value);
		}

		bool getValue(ValueTuple &vt)
		{
			if (_value_set)
				return _value_set->getValue(vt);
			else
				return false;
		}
		bool getRvalu(const string &name, Referenced** value)
		{
			if (_value_set)
				return _value_set->getRvalu(name, value);
			else
				return false;
		}
		template<typename V>
		bool getValue(const string &name, V &value)
		{
			if (_value_set)
				return _value_set->getValue(name, value);
			else
				return false;
		}

		//get value pointer
		Value* getValuePointer(const std::string &name)
		{
			if (_value_set)
				return _value_set->findValue(name);
			else
				return nullptr;
		}

		//toggle value for bool
		bool toggleValue(const std::string &name, bool &value)
		{
			Event event;
			return toggleValue(name, value, event);
		}
		bool toggleValue(const std::string &name, bool &value, Event& event);

		//sync value only sets a state but doesn't change values when called
		//observer's value updates when the value of this changes (data flow is one-way: this -> obj)
		bool syncValue(const std::string &name, Object* obj);
		bool unsyncValue(const std::string &name, Object* obj);
		bool syncValues(const ValueCollection &names, Object* obj);
		bool unsyncValues(const ValueCollection &names, Object* obj);
		bool syncAllValues(Object* obj);
		bool unsyncAllValues(Object* obj);
		//propagate value (this -> object)
		bool propValue(const std::string &name, Object* obj);
		bool propValues(const ValueCollection &names, Object* obj);
		bool propAllValues(Object* obj);
		//sync values belonging to the same object (mutual!)
		bool syncValues(const std::string &name1, const std::string &name2);
		bool unsyncValues(const std::string &name1, const std::string &name2);
		bool syncValues(const ValueCollection &names);
		bool unsyncValues(const ValueCollection &names);
		//propagate values belonging to the same object (1 -> 2)
		bool propValues(const std::string &name1, const std::string &name2);
		bool propValues(const std::string &name1, const ValueCollection &names);

		//directly add
		bool addValue(Value* value)
		{
			if (!value)
				return false;
			if (_value_set && _value_set->addValue(value))
			{
				//also observe the values
				value->addObserver(this);
				return true;
			}
			else return false;
		}

		bool removeValue(Value* value)
		{
			if (!value)
				return false;
			return removeValue(value->getName());
		}

		bool removeValue(const std::string &name)
		{
			if (!_value_set)
				return false;
			Value* value = _value_set->findValue(name);
			if (value)
			{
				//remove observer
				value->removeObserver(this);
				return _value_set->removeValue(value);
			}
			return false;
		}

		bool replaceValue(Value* value)
		{
			if (!value || !_value_set)
				return false;
			Value* old_value = _value_set->findValue(value->getName());
			if (old_value && old_value != value)
				removeValue(old_value);
			return addValue(value);
		}

		ValueCollection getValueCollection()
		{
			ValueCollection result;
			if (_value_set)
			{
				Values values = _value_set->getValues();
				for (auto it = values.begin();
					it != values.end(); ++it)
				{
					result.insert((*it).first);
				}
			}
			return result;
		}
		//order:0-unordered; 1-ascending; 2-descending
		//3-numeric ascending; 4-numeric descending
		ValueVector getValueNames(int order=0)
		{
			ValueVector result;
			if (_value_set)
			{
				Values values = _value_set->getValues();
				for (auto it = values.begin();
					it != values.end(); ++it)
				{
					result.push_back((*it).first);
				}
			}
			switch (order)
			{
			case 0:
			default:
				break;
			case 1:
				std::sort(result.begin(), result.end(),
					[](const std::string& s1, const std::string& s2)
					{
						return s1 < s2;
					});
				break;
			case 2:
				std::sort(result.begin(), result.end(),
					[](const std::string& s1, const std::string& s2)
					{
						return s1 > s2;
					});
				break;
			case 3:
				std::sort(result.begin(), result.end(),
					[](const std::string& s1, const std::string& s2)
					{
					int i1, i2;
					try
					{
						i1 = std::stoi(s1);
						i2 = std::stoi(s2);
					}
					catch (...)
					{
						return s1 < s2;
					}
					return i1 < i2;
				});
				break;
			case 4:
				std::sort(result.begin(), result.end(),
					[](const std::string& s1, const std::string& s2)
					{
					int i1, i2;
					try
					{
						i1 = std::stoi(s1);
						i2 = std::stoi(s2);
					}
					catch (...)
					{
						return s1 > s2;
					}
					return i1 > i2;
				});
				break;
			}
			return result;
		}
		std::string getValueType(const std::string &name)
		{
			if (_value_set)
			{
				Value* value = _value_set->findValue(name);
				if (value)
					return value->getType();
			}
			return "";
		}

		bool containsValue(Value* value)
		{
			if (_value_set)
				return _value_set->containsValue(value);
			return false;
		}

	protected:
		virtual ~Object();

		virtual void handleEvent(Event& event);

		virtual void handleValueChanging(Event& event) {}
		virtual void handleValueChanged(Event& event) {}

		/** ID of an object is non-zero. */
		unsigned int _id;
		// object name
		std::string m_name;

		ref_ptr<ValueSet> _value_set;

		//inputs and outputs are just value names for ui display
		//value names in the collections are to be shown in ui
		ValueCollection inputs_;
		ValueCollection outputs_;

		virtual void setupInputs() {}
		virtual void setupOutputs() {}

		friend class ObjectFactory;

	private:

		Object& operator = (const Object&) { return *this; }
	};

	template<typename T>
	T* clone(const T* t, const CopyOp& copyop = CopyOp::SHALLOW_COPY)
	{
		if (t)
		{
			ref_ptr<Object> obj = t->clone(copyop);

			T* ptr = dynamic_cast<T*>(obj.get());
			if (ptr)
			{
				obj.release();
				return ptr;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}

	template<typename T>
	T* clone(const T* t, const std::string& name, const CopyOp& copyop = CopyOp::SHALLOW_COPY)
	{
		T* newObject = clone(t, copyop);
		if (newObject)
		{
			newObject->setName(name);
			return newObject;
		}
		else
		{
			return 0;
		}
	}

}
#endif
