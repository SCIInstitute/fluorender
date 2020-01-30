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

  class Object : public Observer, public EventHandler
  {
  public:

	Object();

	Object(const Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY, bool copy_values = true);

    virtual Object* clone(const CopyOp& copyop) const { return new Object(*this, copyop); }

	virtual bool isSameKindAs(const Object*) const {return true;}

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

    template<typename T>
    bool addValue(T &vt)
    {
      if(_value_set)
      {
        bool result = _value_set->addValue(vt);
        if(result)
        {
          std::string name = std::get<0>(vt);
          Value* vs_value = _value_set->findValue(name);
          if(vs_value)
          {
            vs_value->addObserver(this);
            Event event;
            event.init(Event::EVENT_VALUE_ADDED,this,vs_value,true);
            vs_value->notify(event);
          }
        }
        return result;
      }
      return false;
    }

    template<typename T, typename V>
    bool addValue(const T &name, const V &value)
    {
      if(_value_set)
      {
        bool result = _value_set->addValue(name,value);
        if(result)
        {
          Value* vs_value = _value_set->findValue(name);
          if(vs_value)
          {
            vs_value->addObserver(this);
            Event event;
            event.init(Event::EVENT_VALUE_ADDED,this,vs_value,true);
            vs_value->notify(event);
          }
        }
        return result;
      }
      else
        return false;
    }

    template<typename VT>
    bool setValue(VT &vt)
    {
      Event event;
      return setValue(vt,event);
    }

    template<typename T, typename R>
    bool setValue(const T &name, R *value)
    {
        Event event;
        return setValue(name,value,event);
    }

    template<typename T, typename V>
    bool setValue(const T &name, const V value)
    {
        Event event;
        return setValue(name,value,event);
    }

    template<typename VT, typename E>
    bool setValue(VT &vt, E &event)
    {
      VT old_vt;
      std::string name = std::get<0>(vt);
      std::get<0>(old_vt) = name;
      Value* value = getValuePointer(name);
      if(getValue(old_vt) && vt != old_vt)
      {
        bool result = false;
        if(_value_set)
        {
          if(!event.sender)
            event.init(Event::EVENT_VALUE_CHANGING,this,value,true);
          result = _value_set->setValue(vt,event);
        }
        return result;
      }
      return false;
    }

    template<typename T, typename R, typename E>
    bool setValue(const T &name, R *value, E &event)
    {
      R* old_value;
      if (getValue(name, &old_value) && value != old_value)
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

    template<typename T, typename V, typename E>
    bool setValue(const T &name, const V &value, E &event)
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



    template<typename T>
    bool getValue(T &vt)
    {
      if (_value_set)
        return _value_set->getValue(vt);
      else
        return false;
    }

    // this is a bandaid. Not sure what it does
    template<typename T>
    Value* getValuePointer(T &vt)
    {
      Value* newVal = new Value(vt);
      return newVal;
    }

    template<typename T, typename V>
    bool getValue(const T &name, V** value)
    {
      if(_value_set)
        return _value_set->getValue(name,value);
      else
        return false;
    }

    template<typename T, typename V>
    bool getValue(const T &name, V &value)
    {
      if(_value_set)
        return _value_set->getValue(name,value);
      else
        return false;
    }

    //get value the class
    template<typename T>
    Value* getValuePointer(const T &name)
    {
        if (_value_set)
            return _value_set->findValue(name);
        else
            return nullptr;
    }

    /*
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
*/
	/** All the set value functions */
    /*
    bool setValue(ValueTuple& vt, const Event& event = Event()); // only compiles if the event is constant
	//generic types
    bool setValue(const std::string &name, Referenced* value, Event& event);
    bool setValue(const std::string &name, bool value, Event& event);
    bool setValue(const std::string &name, char value, Event& event);
    bool setValue(const std::string &name, unsigned char value, Event& event);
    bool setValue(const std::string &name, short value, Event& event);
    bool setValue(const std::string &name, unsigned short value, Event& event);
    bool setValue(const std::string &name, long value, Event& event);
    bool setValue(const std::string &name, unsigned long value, Event& event);
    bool setValue(const std::string &name, long long value, Event& event);
    bool setValue(const std::string &name, unsigned long long value, Event& event);
    bool setValue(const std::string &name, float value, Event& event);
    bool setValue(const std::string &name, double value, Event& event);
    bool setValue(const std::string &name, const std::string &value, Event& event);
    bool setValue(const std::string &name, const std::wstring &value, Event& event);
	//FluoRender's special types
    bool setValue(const std::string &name, const FLTYPE::Point &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::Vector &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::BBox &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::HSVColor &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::Color &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::Plane &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::PlaneSet &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::Quaternion &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::Ray &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::Transform &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::GLfloat4 &value, Event& event);
    bool setValue(const std::string &name, const FLTYPE::GLint4 &value, Event& event);
*/
	//toggle value for bool
    bool toggleValue(const std::string &name, bool &value)
    {
        Event event;
        return toggleValue(name,value,event);
    }
    bool toggleValue(const std::string &name, bool &value, Event& event);

	/** All the get value functions */
    /*
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
    */

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

    virtual void handleValueChanging(Event& event) {}
    virtual void handleValueChanged(Event& event) {}

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
