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
	Referenced(),
	_id(0)
{
	ValueSet* value_set = new ValueSet();
	_vs_stack.push(value_set);
}

Object::Object(const Object& obj, const CopyOp& copyop):
	Referenced(),
	_id(0),
	m_name(obj.m_name)
{
	if (copyop.getCopyFlags() & CopyOp::SHALLOW_COPY)
		_vs_stack.push(obj._vs_stack.top());
	else
	{
		_vs_stack.push(obj._vs_stack.top()->clone(copyop));
		//also observe the values
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			it->second->addObserver(this);
		}
	}
}

Object::~Object()
{
}

//observer functions
void Object::objectDeleted(void* ptr)
{
	Referenced* refd = static_cast<Referenced*>(ptr);
	if (refd)
		_vs_stack.top()->resetRefPtr(refd);

	//remove observee
	removeObservee(refd);
}

void Object::objectChanging(void* ptr, const std::string &exp)
{
	//before change
}

void Object::objectChanged(void* ptr, const std::string &exp)
{
	Referenced* refd = static_cast<Referenced*>(ptr);
	if (refd->className() == std::string("Value"))
	{
		Value* value = dynamic_cast<Value*>(refd);
		_vs_stack.top()->syncValue(value);
		//check self sync list and see if there is any to sync
		if (value)
		{
			std::string name1 = value->getName();
			for (auto group = _sync_values.begin();
				group != _sync_values.end(); ++group)
			{
				auto it = (*group).find(name1);
				if (it != (*group).end())
				{
					//found in group
					for (auto it2 = (*group).begin();
						it2 != (*group).end(); ++it2)
					{
						std::string name2 = *it2;
						if (name1 != name2)
							propValues(name1, name2);
					}
				}
			}
		}
	}
	//else if (refd->className() == std::string("Object"))
	//{ //do something in response
	//}
}

//add functions
bool Object::addValue(ValueTuple &vt)
{
	if (_vs_stack.top())
	{
		bool result = _vs_stack.top()->addValue(vt);
		if (result)
		{
			std::string name = std::get<0>(vt);
			Value* vs_value = _vs_stack.top()->findValue(name);
			if (vs_value)
				vs_value->addObserver(this);
		}
		return result;
	}
	return false;
}

//define function bodies first
#define OBJECT_ADD_VALUE_BODY \
	if (_vs_stack.top()) \
	{ \
		bool result = _vs_stack.top()->addValue(name, value); \
		if (result) \
		{ \
			Value* vs_value = _vs_stack.top()->findValue(name); \
			if (vs_value) \
				vs_value->addObserver(this); \
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

//define function bodies first
bool Object::setValue(ValueTuple &vt, bool notify)
{
	ValueTuple old_vt;
	std::string name = std::get<0>(vt);
	std::get<0>(old_vt) = name;
	if (getValue(old_vt) && vt != old_vt)
	{
		bool result = false;
		if (_vs_stack.top())
		{
			notifyObserversBeforeChange(name);
			result = _vs_stack.top()->setValue(vt, notify);
			if (result)
				notifyObserversOfChange(name);
		}
		return result;
	}
	return false;
}

//set functions
#define OBJECT_SET_VALUE_BODY \
	if (getValue(name, old_value) && value != old_value) \
	{ \
		bool result = false; \
		if (_vs_stack.top()) \
		{ \
			notifyObserversBeforeChange(name); \
			result = _vs_stack.top()->setValue(name, value, notify); \
			if (result) \
				notifyObserversOfChange(name); \
		} \
		return result; \
	} \
	return false

//set functions
bool Object::setValue(const std::string &name, Referenced* value, bool notify)
{
	Referenced* old_value;
	if (getValue(name, &old_value) && value != old_value)
	{
		if (old_value)
			old_value->removeObserver(this);
		if (value)
			value->addObserver(this);
		bool result = false;
		if (_vs_stack.top())
		{
			notifyObserversBeforeChange();
			result = _vs_stack.top()->setValue(name, value, notify);
			if (result)
				notifyObserversOfChange();
		}
		return result;
	}
	return false;
}

bool Object::setValue(const std::string &name, bool value, bool notify)
{
	bool old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, char value, bool notify)
{
	char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned char value, bool notify)
{
	unsigned char old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, short value, bool notify)
{
	short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned short value, bool notify)
{
	unsigned short old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long value, bool notify)
{
	long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long value, bool notify)
{
	unsigned long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, long long value, bool notify)
{
	long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, unsigned long long value, bool notify)
{
	unsigned long long old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, float value, bool notify)
{
	float old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, double value, bool notify)
{
	double old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::string &value, bool notify)
{
	std::string old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const std::wstring &value, bool notify)
{
	std::wstring old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Point &value, bool notify)
{
	FLTYPE::Point old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Vector &value, bool notify)
{
	FLTYPE::Vector old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::BBox &value, bool notify)
{
	FLTYPE::BBox old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::HSVColor &value, bool notify)
{
	FLTYPE::HSVColor old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Color &value, bool notify)
{
	FLTYPE::Color old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Plane &value, bool notify)
{
	FLTYPE::Plane old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::PlaneSet &value, bool notify)
{
	FLTYPE::PlaneSet old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Quaternion &value, bool notify)
{
	FLTYPE::Quaternion old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Ray &value, bool notify)
{
	FLTYPE::Ray old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::Transform &value, bool notify)
{
	FLTYPE::Transform old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::GLfloat4 &value, bool notify)
{
	FLTYPE::GLfloat4 old_value;
	OBJECT_SET_VALUE_BODY;
}

bool Object::setValue(const std::string &name, const FLTYPE::GLint4 &value, bool notify)
{
	FLTYPE::GLint4 old_value;
	OBJECT_SET_VALUE_BODY;
}

//toggle value for bool
bool Object::toggleValue(const std::string &name, bool &value, bool notify)
{
	bool result = false;
	if (_vs_stack.top())
	{
		notifyObserversBeforeChange(name);
		result = _vs_stack.top()->toggleValue(name, value, notify);
	}
	if (result)
		notifyObserversOfChange(name);
	return result;
}

//get functions
bool Object::getValue(ValueTuple &vt)
{
	if (_vs_stack.top())
		return _vs_stack.top()->getValue(vt);
	else
		return false;
}

//define function bodies first
#define OBJECT_GET_VALUE_BODY \
	if (_vs_stack.top()) \
		return _vs_stack.top()->getValue(name, value); \
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
bool Object::syncValue(const std::string &name, Observer* obsrvr)
{
	Value* value = getValue(name);
	if (value)
	{
		value->addObserver(obsrvr);
		return true;
	}
	return false;
}

//unsync a value
bool Object::unsyncValue(const std::string &name, Observer* obsrvr)
{
	Value* value = getValue(name);
	if (value)
	{
		value->removeObserver(obsrvr);
		return true;
	}
	return false;
}

//sync a list of values
bool Object::syncValues(const std::vector<std::string> &names, Observer* obsrvr)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= syncValue(*it, obsrvr);
	}
	return result;
}

//unsync a list of values
bool Object::unsyncValues(const std::vector<std::string> &names, Observer* obsrvr)
{
	bool result = false;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		result |= unsyncValue(*it, obsrvr);
	}
	return result;
}

//sync all values
bool Object::syncAllValues(Observer* obsrvr)
{
	bool result = false;
	if (_vs_stack.top())
	{
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			if (it->second)
			{
				it->second->addObserver(obsrvr);
				result = true;
			}
		}
	}
	return result;
}

//unsync all values
bool Object::unsyncAllValues(Observer* obsrvr)
{
	bool result = false;
	if (_vs_stack.top())
	{
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
		{
			if (it->second)
			{
				it->second->removeObserver(obsrvr);
				result = true;
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
		obj->objectChanged(value, "");
		return true;
	}
	return false;
}

//propagate a list of values
bool Object::propValues(const std::vector<std::string> &names, Object* obj)
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
	if (_vs_stack.top())
	{
		for (auto it = _vs_stack.top()->getValues().begin();
			it != _vs_stack.top()->getValues().end(); ++it)
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
	bool result = false;
	//search the groups first
	std::set<std::string>::iterator it1, it2;
	for (auto group = _sync_values.begin();
		group != _sync_values.end(); ++group)
	{
		it1 = (*group).find(name1);
		it2 = (*group).find(name2);
		if (it1 != (*group).end())
		{
			result |= (*group).insert(name2).second;
			break;
		}
		if (it2 != (*group).end())
		{
			result |= (*group).insert(name1).second;
			break;
		}
	}
	if (!result)
	{
		std::set<std::string> group;
		result |= group.insert(name1).second;
		result |= group.insert(name2).second;
		_sync_values.push_back(group);
	}
	//see if there is anything to merge??
	return result;
}

bool Object::unsyncValues(const std::string *name1, const std::string &name2)
{
	bool result = false;
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
		value2->sync(value1);
		return true;
	}
	return false;
}

bool Object::propValues(const std::string &name1, const std::vector<std::string> &names)
{
	bool result = false;
	Value* value1 = getValue(name1);
	if (!value1)
		return result;
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		Value* value2 = getValue(*it);
		if (value2 && value2 != value1)
			result |= value2->sync(value1);
	}
	return result;
}

