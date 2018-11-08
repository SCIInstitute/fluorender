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
#include <Flobject/Object.h>

using namespace FL;

Object::Object():
	EventHandler(),
	_id(0)
{
	_value_set = new ValueSet();
}

Object::Object(const Object& obj, const CopyOp& copyop, bool copy_values):
	EventHandler(),
	_id(0),
	m_name(obj.m_name)
{
	_value_set = new ValueSet();
	if (copy_values)
		copyValues(obj, copyop);
}

Object::~Object()
{
}

//observer functions
void Object::objectDeleted(Event& event)
{
	Referenced* refd = event.sender;
	if (refd)
		_value_set->resetRefPtr(refd);

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
	else
		value_name = event.value_name;

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
	//handle event
	handleEvent(event);
	//notify observers
	if (event.type == Event::EVENT_VALUE_CHANGING)
		return;
	notifyObservers(event);
}

//add functions
bool Object::addValue(ValueTuple &vt)
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

//define function bodies first
#define OBJECT_ADD_VALUE_BODY \
	if (_value_set) \
	{ \
		bool result = _value_set->addValue(name, value); \
		if (result) \
		{ \
			Value* vs_value = _value_set->findValue(name); \
			if (vs_value) \
			{ \
				vs_value->addObserver(this); \
				Event event; \
				event.init(Event::EVENT_VALUE_ADDED, \
					this, vs_value, true); \
				vs_value->notify(event); \
			} \
		} \
		return result; \
	} \
	else \
		return false

//actual add functions
bool Object::addValue(const std::string &name, Referenced* value)
{
	if (value) value->addObserver(this);
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, bool value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, char value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned char value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, short value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned short value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, long long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, unsigned long long value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, float value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, double value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const std::string &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const std::wstring &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Point &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Vector &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::BBox &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::HSVColor &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Color &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Plane &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::PlaneSet &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Quaternion &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Ray &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::Transform &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::GLfloat4 &value)
{
	OBJECT_ADD_VALUE_BODY;
}

bool Object::addValue(const std::string &name, const FLTYPE::GLint4 &value)
{
	OBJECT_ADD_VALUE_BODY;
}

//set functions
bool Object::setValue(ValueTuple &vt, Event& event)
{
	ValueTuple old_vt;
	std::string name = std::get<0>(vt);
	std::get<0>(old_vt) = name;
	Value* value = getValue(name);
	if (getValue(old_vt) && vt != old_vt)
	{
		bool result = false;
		if (_value_set)
		{
			if (!event.sender)
				event.init(Event::EVENT_VALUE_CHANGING,
					this, value, true);
			result = _value_set->setValue(vt, event);
		}
		return result;
	}
	return false;
}

//define function bodies first
#define OBJECT_SET_VALUE_BODY \
	if (getValue(name, old_value) && value != old_value) \
	{ \
		bool result = false; \
		if (_value_set) \
		{ \
			if (!event.sender) \
				event.init(Event::EVENT_VALUE_CHANGING, \
					this, getValue(name), true); \
			result = _value_set->setValue(name, value, event); \
		} \
		return result; \
	} \
	return false

//set functions
bool Object::setValue(const std::string &name, Referenced* value, Event& event)
{
	Referenced* old_value;
	if (getValue(name, &old_value) && value != old_value)
	{
		if (old_value)
			old_value->removeObserver(this);
		if (value)
			value->addObserver(this);
		bool result = false;
		if (_value_set)
		{
			if (!event.sender)
				event.init(Event::EVENT_VALUE_CHANGING,
					this, getValue(name), true);
			result = _value_set->setValue(name, value, event);
		}
		return result;
	}
	return false;
}

bool Object::setValue(const std::string &name, bool value, Event& event)
{
	bool old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, char value, Event& event)
{
	char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned char value, Event& event)
{
	unsigned char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, short value, Event& event)
{
	short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned short value, Event& event)
{
	unsigned short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long value, Event& event)
{
	long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long value, Event& event)
{
	unsigned long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long long value, Event& event)
{
	long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long long value, Event& event)
{
	unsigned long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, float value, Event& event)
{
	float old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, double value, Event& event)
{
	double old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::string &value, Event& event)
{
	std::string old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::wstring &value, Event& event)
{
	std::wstring old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Point &value, Event& event)
{
	FLTYPE::Point old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Vector &value, Event& event)
{
	FLTYPE::Vector old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::BBox &value, Event& event)
{
	FLTYPE::BBox old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::HSVColor &value, Event& event)
{
	FLTYPE::HSVColor old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Color &value, Event& event)
{
	FLTYPE::Color old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Plane &value, Event& event)
{
	FLTYPE::Plane old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::PlaneSet &value, Event& event)
{
	FLTYPE::PlaneSet old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Quaternion &value, Event& event)
{
	FLTYPE::Quaternion old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Ray &value, Event& event)
{
	FLTYPE::Ray old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Transform &value, Event& event)
{
	FLTYPE::Transform old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::GLfloat4 &value, Event& event)
{
	FLTYPE::GLfloat4 old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::GLint4 &value, Event& event)
{
	FLTYPE::GLint4 old_value;
	OBJECT_SET_VALUE_BODY;
}

//toggle value for bool
bool Object::toggleValue(const std::string &name, bool &value, Event& event)
{
	bool result = false;
	if (_value_set)
	{
		if (!event.sender)
			event.init(Event::EVENT_VALUE_CHANGING,
				this, getValue(name), true);
		result = _value_set->toggleValue(name, value, event);
	}
	return result;
}

//get functions
bool Object::getValue(ValueTuple &vt)
{
	if (_value_set)
		return _value_set->getValue(vt);
	else
		return false;
}

//define function bodies first
#define OBJECT_GET_VALUE_BODY \
	if (_value_set) \
		return _value_set->getValue(name, value); \
	else \
		return false

//get functions
bool Object::getValue(const std::string &name, Referenced** value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, bool &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, char &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned char &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, short &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned short &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, long long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, unsigned long long &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, float &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, double &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, std::string &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, std::wstring &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Point &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Vector &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::BBox &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::HSVColor &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Color &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Plane &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::PlaneSet &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Quaternion &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Ray &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::Transform &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::GLfloat4 &value)
{
	OBJECT_GET_VALUE_BODY;
}

bool Object::getValue(const std::string &name, FLTYPE::GLint4 &value)
{
	OBJECT_GET_VALUE_BODY;
}

//sync a value
//observer's value updates when this updates
bool Object::syncValue(const std::string &name, Object* obj)
{
	if (obj)
	{
		Value* value = getValue(name);
		Value* value2 = obj->getValue(name);
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
		Value* value = getValue(name);
		Value* value2 = obj->getValue(name);
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
bool Object::propValue(const std::string &name, Object* obj)
{
	Value* value = getValue(name);
	if (value)
	{
		Value* obj_value = obj->getValue(name);
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
bool Object::propValues(const ValueCollection &names, Object* obj)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= propValue(*it, obj);
	}
	return result;
}

//propagate all values
bool Object::propAllValues(Object* obj)
{
	bool result = false;
	if (_value_set)
	{
		for (auto it = _value_set->getValues().begin();
			it != _value_set->getValues().end(); ++it)
		{
			if (it->second)
			{
				propValue(it->second->getName(), obj);
				result = true;
			}
		}
	}
	return result;
}

//sync values belonging to the same object (mutual! hope this is not confusing)
bool Object::syncValues(const std::string &name1, const std::string &name2)
{
	Value* value1 = getValue(name1);
	Value* value2 = getValue(name2);
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
	Value* value1 = getValue(name1);
	Value* value2 = getValue(name2);
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
bool Object::propValues(const std::string &name1, const std::string &name2)
{
	Value* value1 = getValue(name1);
	Value* value2 = getValue(name2);
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

bool Object::propValues(const std::string &name1, const ValueCollection &names)
{
	bool result = false;
	Value* value1 = getValue(name1);
	if (!value1)
		return result;
	Event event;
	event.init(Event::EVENT_SYNC_VALUE,
		value1, value1, true);
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		Value* value2 = getValue(*it);
		if (value2 && value2 != value1)
			result |= value2->sync(event);
	}
	return result;
}
