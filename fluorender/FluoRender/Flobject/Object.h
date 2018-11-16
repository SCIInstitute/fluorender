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
#ifndef FL_OBJECT
#define FL_OBJECT 1

#include <Flobject/ref_ptr.h>
#include <Flobject/Observer.h>
#include <Flobject/CopyOp.h>
#include <Flobject/Value.h>
#include <Flobject/EventHandler.h>
#include <string>
#include <vector>
#include <functional>

namespace FUI
{
	class InterfaceAgent;
}
namespace FL
{
class Object;
typedef std::vector<Object*> ObjectList;
class ObjectFactory;

class Object : public EventHandler, public Observer
{
public:

	Object();

	Object(const Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

	virtual Object* clone(const CopyOp& copyop) const { return new Object(*this, copyop); };

	virtual bool isSameKindAs(const Object*) const {return true;}

	virtual const char* className() const { return "Object"; }

	inline void setId(const unsigned int id) { _id = id; }

	inline const unsigned int getId() const { return _id; }

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
			Value* value = 0;
			if (copyop.getCopyFlags() & CopyOp::DEEP_COPY_VALUES)
				value = it->second->clone();
			else
				value = it->second.get();
			addValue(value);
			//value->notify();//not sure why it needs notification, removed
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

	//add a value
	bool addValue(ValueTuple& vt);
	//generic types
	bool addValue(const std::string &name, Referenced* value);
	bool addValue(const std::string &name, bool value);
	bool addValue(const std::string &name, char value);
	bool addValue(const std::string &name, unsigned char value);
	bool addValue(const std::string &name, short value);
	bool addValue(const std::string &name, unsigned short value);
	bool addValue(const std::string &name, long value);
	bool addValue(const std::string &name, unsigned long value);
	bool addValue(const std::string &name, long long value);
	bool addValue(const std::string &name, unsigned long long value);
	bool addValue(const std::string &name, float);
	bool addValue(const std::string &name, double);
	bool addValue(const std::string &name, const std::string &value);
	bool addValue(const std::string &name, const std::wstring &value);
	//FluoRender's special types
	bool addValue(const std::string &name, const FLTYPE::Point &value);
	bool addValue(const std::string &name, const FLTYPE::Vector &value);
	bool addValue(const std::string &name, const FLTYPE::BBox &value);
	bool addValue(const std::string &name, const FLTYPE::HSVColor &value);
	bool addValue(const std::string &name, const FLTYPE::Color &value);
	bool addValue(const std::string &name, const FLTYPE::Plane &value);
	bool addValue(const std::string &name, const FLTYPE::PlaneSet &value);
	bool addValue(const std::string &name, const FLTYPE::Quaternion &value);
	bool addValue(const std::string &name, const FLTYPE::Ray &value);
	bool addValue(const std::string &name, const FLTYPE::Transform &value);
	bool addValue(const std::string &name, const FLTYPE::GLfloat4 &value);
	bool addValue(const std::string &name, const FLTYPE::GLint4 &value);

	/** All the set value functions */
	bool setValue(ValueTuple& vt, Event& event = Event());
	//generic types
	bool setValue(const std::string &name, Referenced* value, Event& event = Event());
	bool setValue(const std::string &name, bool value, Event& event = Event());
	bool setValue(const std::string &name, char value, Event& event = Event());
	bool setValue(const std::string &name, unsigned char value, Event& event = Event());
	bool setValue(const std::string &name, short value, Event& event = Event());
	bool setValue(const std::string &name, unsigned short value, Event& event = Event());
	bool setValue(const std::string &name, long value, Event& event = Event());
	bool setValue(const std::string &name, unsigned long value, Event& event = Event());
	bool setValue(const std::string &name, long long value, Event& event = Event());
	bool setValue(const std::string &name, unsigned long long value, Event& event = Event());
	bool setValue(const std::string &name, float value, Event& event = Event());
	bool setValue(const std::string &name, double value, Event& event = Event());
	bool setValue(const std::string &name, const std::string &value, Event& event = Event());
	bool setValue(const std::string &name, const std::wstring &value, Event& event = Event());
	//FluoRender's special types
	bool setValue(const std::string &name, const FLTYPE::Point &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::Vector &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::BBox &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::HSVColor &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::Color &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::Plane &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::PlaneSet &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::Quaternion &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::Ray &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::Transform &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::GLfloat4 &value, Event& event = Event());
	bool setValue(const std::string &name, const FLTYPE::GLint4 &value, Event& event = Event());

	//toggle value for bool
	bool toggleValue(const std::string &name, bool &value, Event& event = Event());

	/** All the get value functions */
	bool getValue(ValueTuple& vt);
	//generic types
	bool getValue(const std::string &name, Referenced** value);
	bool getValue(const std::string &name, bool &value);
	bool getValue(const std::string &name, char &value);
	bool getValue(const std::string &name, unsigned char &value);
	bool getValue(const std::string &name, short &value);
	bool getValue(const std::string &name, unsigned short &value);
	bool getValue(const std::string &name, long &value);
	bool getValue(const std::string &name, unsigned long &value);
	bool getValue(const std::string &name, long long &value);
	bool getValue(const std::string &name, unsigned long long &value);
	bool getValue(const std::string &name, float &value);
	bool getValue(const std::string &name, double &value);
	bool getValue(const std::string &name, std::string &value);
	bool getValue(const std::string &name, std::wstring &value);
	//FluoRender's special types
	bool getValue(const std::string &name, FLTYPE::Point &value);
	bool getValue(const std::string &name, FLTYPE::Vector &value);
	bool getValue(const std::string &name, FLTYPE::BBox &value);
	bool getValue(const std::string &name, FLTYPE::HSVColor &value);
	bool getValue(const std::string &name, FLTYPE::Color &value);
	bool getValue(const std::string &name, FLTYPE::Plane &value);
	bool getValue(const std::string &name, FLTYPE::PlaneSet &value);
	bool getValue(const std::string &name, FLTYPE::Quaternion &value);
	bool getValue(const std::string &name, FLTYPE::Ray &value);
	bool getValue(const std::string &name, FLTYPE::Transform &value);
	bool getValue(const std::string &name, FLTYPE::GLfloat4 &value);
	bool getValue(const std::string &name, FLTYPE::GLint4 &value);

	//sync value
	//observer's value updates when this updates (this -> obj)
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

	//get value the class
	Value* getValue(const std::string &name)
	{
		if (_value_set)
			return _value_set->findValue(name);
		else
			return 0;
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

	ValueCollection getValueNames()
	{
		ValueCollection result;
		if (_value_set)
		{
			ValueSet::Values values = _value_set->getValues();
			for (auto it = values.begin();
				it != values.end(); ++it)
			{
				result.insert((*it).first);
			}
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

	virtual void handleValueChanging(Event& event) {};
	virtual void handleValueChanged(Event& event) {};

	/** ID of an object is non-zero. */
	unsigned int _id;
	// object name
	std::string m_name;

	ref_ptr<ValueSet> _value_set;

	friend class ObjectFactory;

private:

	Object& operator = (const Object&) {return *this; }
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
