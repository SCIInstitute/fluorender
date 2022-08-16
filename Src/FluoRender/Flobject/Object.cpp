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
#include <Object.hpp>
#include <ObjectFactory.hpp>
#include <Names.hpp>

using namespace fluo;

Object::Object() :
	EventHandler(),
	_id(0)
{
	_value_set = new ValueSet();
	_value_bank = new ValueSet();
	setDefaultValueChangingFunction(
		std::bind(&Object::handleValueChanging,
			this, std::placeholders::_1));
	setDefaultValueChangedFunction(
		std::bind(&Object::handleValueChanged,
			this, std::placeholders::_1));
}

Object::Object(const Object& obj, const CopyOp& copyop, bool copy_values) :
	EventHandler(),
	_id(0),
	m_name(obj.m_name)
{
	_value_set = new ValueSet();
	_value_bank = new ValueSet();
	if (copy_values)
		copyValues(obj, copyop);
	setDefaultValueChangingFunction(
		std::bind(&Object::handleValueChanging,
			this, std::placeholders::_1));
	setDefaultValueChangedFunction(
		std::bind(&Object::handleValueChanged,
			this, std::placeholders::_1));
}

Object::~Object()
{
}

//observer functions
void Object::objectDeleted(Event& event)
{
	Referenced* refd = event.sender;
	//remove observee
	removeObservee(refd);
}

void Object::handleEvent(Event& event)
{
	Referenced* refd = event.sender;
	if (!refd)
		return;

	Value* value = dynamic_cast<Value*>(refd);
	std::string value_name;
	if (value)
		value_name = value->getName();

	switch (event.type)
	{
	case Event::EVENT_VALUE_CHANGING:
		onValueChanging(value_name, event);
		break;
	case Event::EVENT_VALUE_CHANGED:
		onValueChanged(value_name, event);
		break;
	case Event::EVENT_NODE_ADDED:
		onNodeAdded(event);
		break;
	case Event::EVENT_NODE_REMOVED:
		onNodeRemoved(event);
		break;
	}
}

void Object::processNotification(Event& event)
{
	//determine if self or others
	bool flag = false;
	Event::NotifyFlags notify = event.getNotifyFlags();
	Object* obj = dynamic_cast<Object*>(event.origin);
	if (notify & Event::NOTIFY_SELF)
	{
		if (obj == this)
			flag = true;
	}
	if (notify & Event::NOTIFY_OTHERS)
	{
		if (obj != this)
			flag = true;
	}
	if (flag)
	{
		//handle event
		handleEvent(event);
	}

	//notify observers
	if (event.type == Event::EVENT_VALUE_CHANGING)
		return;
	notifyObservers(event);
}

//toggle value for bool
bool Object::flipUpdateValue(const std::string &name, bool &value, Event& event)
{
	bool result = false;
	if (_value_set)
	{
		if (!event.sender)
			event.init(Event::EVENT_VALUE_CHANGING,
				this, getValuePointer(name), true);
		else
			event.push(this);
		result = _value_set->flipValue(name, value, event);
		event.pop();
	}
	return result;
}

//sync a value
//observer's value updates when this updates
bool Object::syncValue(const std::string &name, Object* obj)
{
	if (obj)
	{
		Value* value = getValuePointer(name);
		Value* value2 = obj->getValuePointer(name);
		if (value && value2)
		{
			value->addObserver(value2);
			return true;
		}
	}
	return false;
}

//unsync a value
bool Object::unsyncValue(const std::string &name, Object* obj)
{
	if (obj)
	{
		Value* value = getValuePointer(name);
		Value* value2 = obj->getValuePointer(name);
		if (value && value2)
		{
			value->removeObserver(value2);
			return true;
		}
	}
	return false;
}

//sync a list of values
bool Object::syncValues(const ValueCollection &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= syncValue(*it, obj);
	}
	return result;
}

//unsync a list of values
bool Object::unsyncValues(const ValueCollection &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= unsyncValue(*it, obj);
	}
	return result;
}

//sync all values
bool Object::syncAllValues(Object* obj)
{
	bool result = false;
	std::string name;
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
		{
			if (it->second)
			{
				name = it->second->getName();
				result |= syncValue(name, obj);
			}
		}
	}
	return result;
}

//unsync all values
bool Object::unsyncAllValues(Object* obj)
{
	bool result = false;
	std::string name;
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
		{
			if (it->second)
			{
				name = it->second->getName();
				result |= unsyncValue(name, obj);
			}
		}
	}
	return result;
}

//propagate a value
bool Object::propagateValue(const std::string &name, Object* obj)
{
	Value* value = getValuePointer(name);
	if (value)
	{
		Value* obj_value = obj->getValuePointer(name);
		if (obj_value)
		{
			Event event;
			event.init(Event::EVENT_SYNC_VALUE,
				value, value, true);
			obj_value->sync(event);
		}
		return true;
	}
	return false;
}

//propagate a list of values
bool Object::propagateValues(const ValueCollection &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= propagateValue(*it, obj);
	}
	return result;
}

//propagate all values
bool Object::propagateAllValues(Object* obj)
{
	bool result = false;
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
		{
			if (it->second)
			{
				propagateValue(it->second->getName(), obj);
				result = true;
			}
		}
	}
	return result;
}

//sync values belonging to the same object (mutual! hope this is not confusing)
bool Object::syncValues(const std::string &name1, const std::string &name2)
{
	Value* value1 = getValuePointer(name1);
	Value* value2 = getValuePointer(name2);
	if (value1 && value2 &&
		value1->getType() ==
		value2->getType())
	{
		value1->addObserver(value2);
		value2->addObserver(value1);
		return true;
	}
	return false;
}

bool Object::unsyncValues(const std::string &name1, const std::string &name2)
{
	Value* value1 = getValuePointer(name1);
	Value* value2 = getValuePointer(name2);
	if (value1 && value2)
	{
		value1->removeObserver(value2);
		value2->removeObserver(value1);
		return true;
	}
	return false;
}

bool Object::syncValues(const ValueCollection &names)
{
	bool result = false;
	for (auto it1 = names.begin();
		it1 != names.end(); ++it1)
	{
		for (auto it2 = std::next(it1);
			it2 != names.end(); ++it2)
			result |= syncValues(*it1, *it2);
	}
	return result;
}

bool Object::unsyncValues(const ValueCollection &names)
{
	bool result = false;
	for (auto it1 = names.begin();
		it1 != names.end(); ++it1)
	{
		for (auto it2 = std::next(it1);
			it2 != names.end(); ++it2)
			result |= unsyncValues(*it1, *it2);
	}
	return result;
}

//propagate values belonging to the same object (1 -> 2)
bool Object::propagateValues(const std::string &name1, const std::string &name2)
{
	Value* value1 = getValuePointer(name1);
	Value* value2 = getValuePointer(name2);
	if (value1 && value2 &&
		value1 != value2)
	{
		Event event;
		event.init(Event::EVENT_SYNC_VALUE,
			value1, value1, true);
		value2->sync(event);
		return true;
	}
	return false;
}

bool Object::propagateValues(const std::string &name1, const ValueCollection &names)
{
	bool result = false;
	Value* value1 = getValuePointer(name1);
	if (!value1)
		return result;
	Event event;
	event.init(Event::EVENT_SYNC_VALUE,
		value1, value1, true);
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		Value* value2 = getValuePointer(*it);
		if (value2 && value2 != value1)
			result |= value2->sync(event);
	}
	return result;
}

//save and restore
bool Object::saveValue(const std::string &name)
{
	bool result = false;
	if (_value_set && _value_bank)
	{
		Value* value = getValuePointer(name);
		if (!value) return result;
		_value_bank->removeValue(name);
		value = value->clone();
		result = _value_bank->addValue(value);
	}
	return result;
}

bool Object::drawValue(const std::string &name)
{
	bool result = false;
	if (_value_set && _value_bank)
	{
		Value* save = _value_bank->findValue(name);
		if (!save) return result;
		Value* value = getValuePointer(name);
		if (value)
		{
			Event event;
			event.init(Event::EVENT_SYNC_VALUE,
				save, save, true);
			value->sync(event);
			result = true;
		}
	}
	return result;
}

bool Object::saveValues(const ValueCollection &names)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= saveValue(*it);
	}
	return result;
}

bool Object::drawValues(const ValueCollection &names)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= drawValue(*it);
	}
	return result;
}

//reset values from factory
bool Object::resetValue(const std::string &name)
{
	Referenced* ref;
	getRefValue(gstFactory, &ref);
	ObjectFactory* fac = dynamic_cast<ObjectFactory*>(ref);
	if (!fac) return false;
	Object* dobj = fac->getDefault();
	if (!dobj) return false;
	return dobj->propagateValue(name, this);
}

bool Object::resetValues(const ValueCollection &names)
{
	Referenced* ref;
	getRefValue(gstFactory, &ref);
	ObjectFactory* fac = dynamic_cast<ObjectFactory*>(ref);
	if (!fac) return false;
	Object* dobj = fac->getDefault();
	if (!dobj) return false;
	return dobj->propagateValues(names, this);
}

bool Object::resetAllValues()
{
	Referenced* ref;
	getRefValue(gstFactory, &ref);
	ObjectFactory* fac = dynamic_cast<ObjectFactory*>(ref);
	if (!fac) return false;
	Object* dobj = fac->getDefault();
	if (!dobj) return false;
	return dobj->propagateAllValues(this);
}
