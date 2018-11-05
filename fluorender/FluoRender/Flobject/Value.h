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
#ifndef FL_VALUE
#define FL_VALUE

#include <Flobject/Referenced.h>
#include <Flobject/ref_ptr.h>
#include <Flobject/CopyOp.h>
#include <Flobject/Observer.h>
#include <Flobject/Event.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <codecvt>
#include <tuple>
//FluoRender's special types
#include <Types/BBox.h>
#include <Types/Color.h>
#include <Types/Plane.h>
#include <Types/Point.h>
#include <Types/Quaternion.h>
#include <Types/Ray.h>
#include <Types/Transform.h>
#include <Types/Vector.h>
#include <Types/GLfloat4.h>
#include <Types/GLint4.h>

namespace FL
{
//name, type, value
typedef std::tuple<std::string, std::string, std::string> ValueTuple;

class Value : public Referenced, public Observer
{
public:
	enum ValueType
	{
		vt_null = 0,
		vt_pReferenced = 1,
		vt_bool,
		vt_char,
		vt_unsigned_char,
		vt_short,
		vt_unsigned_short,
		vt_long,
		vt_unsigned_long,
		vt_long_long,
		vt_unsigned_long_long,
		vt_float,
		vt_double,
		vt_string,
		vt_wstring,
		vt_Point,
		vt_Vector,
		vt_BBox,
		vt_HSVColor,
		vt_Color,
		vt_Plane,
		vt_PlaneSet,
		vt_Quaternion,
		vt_Ray,
		vt_Transform,
		vt_GLfloat4,
		vt_GLint4,
	};
	Value(std::string name = "", std::string type = "") :
		Referenced(), _name(name), _type(type) {}

	Value* clone();

	virtual const char* className() const { return "Value"; }

	virtual unsigned int getPriority() const { return 10; }

	//observer
	virtual void objectDeleted(Event& event);
	virtual void processNotification(Event& event);

	std::string getName() { return _name; }
	std::string getType() { return _type; }

	inline bool operator == (const Value& v) const;
	inline bool operator != (const Value& v) const;

	inline bool sync(Event& event);

	inline void notify(Event& event);//notify observers when a value is just added

	friend inline std::ostream& operator<<(std::ostream& os, const Value& v);

protected:
	virtual ~Value();

	std::string _name;
	std::string _type;
	ValueType _etype;
	static bool _precise;//if equal is precise
	static std::unordered_map<std::string, ValueType> _value_map;

	friend class ValueSet;
};

template<typename T>
class TemplateValue : public Value
{
public:

	TemplateValue(const std::string& name, const std::string& type, const T& value) :
	Value(name,type), _value(value)
	{
		if (_value_map.empty())
		{
			_etype = vt_null;
			return;
		}
		auto it = _value_map.find(type);
		if (it != _value_map.end())
			_etype = it->second;
	}

	TemplateValue<T>* clone()
	{
		TemplateValue<T>* tv = new TemplateValue<T>(
			_name, _type, _value);
		return tv;
	}

	void setValue(const T& value, Event& event)
	{
		if (!equal(value))
		{
			event.type = Event::EVENT_VALUE_CHANGING;
			notifyObservers(event);
			_value = value;
			event.type = Event::EVENT_VALUE_CHANGED;
			notifyObservers(event);
		}
	}

	const T& getValue() const {return _value;}

	bool equal(const T& value) const
	{
		if (_precise)
			return _value == value;
		else
		{
			ref_ptr<TemplateValue<T>> pvalue(new TemplateValue<T>(_name, _type, value));
			return *this == *pvalue;
		}
	}

	T _value;

protected:
	virtual ~TemplateValue() {};

};

class ValueSet : public Referenced
{
public:
	typedef std::unordered_map<std::string, ref_ptr<Value>> Values;

	ValueSet() : Referenced() {}

	ValueSet(const ValueSet& vs, const CopyOp& copyop=CopyOp::SHALLOW_COPY);

	ValueSet* clone(const CopyOp& copyop) const {return new ValueSet(*this, copyop);}

	virtual const char* className() const { return "ValueSet"; }

	inline bool operator == (const ValueSet& vs) const;
	inline bool operator != (const ValueSet& vs) const;

	void clear();

	Value* findValue(const std::string &name);
	bool containsValue(Value* value);
	bool addValue(Value* value);
	bool removeValue(Value* value);
	bool removeValue(const std::string &name);
	//reset Referenced pointer to NULL
	bool resetRefPtr(Referenced* value);

	//add value functions
	bool addValue(ValueTuple&);
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
	bool addValue(const std::string &name, float value);
	bool addValue(const std::string &name, double value);
	bool addValue(const std::string &name, const std::string &value);
	bool addValue(const std::string &name, const std::wstring &value);
	//add FluoRender's special types here
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
	bool setValue(ValueTuple& vt, Event& event);
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
	//add FluoRender's special types here
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

	//toggle value for bool, result in value
	bool toggleValue(const std::string &name, bool &value, Event& event);

	/** All the get value functions */
	bool getValue(ValueTuple&);
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
	//FluoRender's special value types
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

	Values& getValues() {return _values;}
	const Values& getValues() const {return _values;}

protected:
	virtual ~ValueSet();

	Values _values;

	template<typename T>
	friend class TemplateValue;
};

inline bool Value::operator == (const Value& v) const
{
	if (_name != v._name)
		return false;
	if (_type != v._type)
		return false;

	switch (_etype)
	{
	case vt_pReferenced:
		return dynamic_cast<const TemplateValue<Referenced*>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<Referenced*>*>(&v)->getValue();
	case vt_bool:
		return dynamic_cast<const TemplateValue<bool>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<bool>*>(&v)->getValue();
	case vt_char:
		return dynamic_cast<const TemplateValue<char>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<char>*>(&v)->getValue();
	case vt_unsigned_char:
		return dynamic_cast<const TemplateValue<unsigned char>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned char>*>(&v)->getValue();
	case vt_short:
		return dynamic_cast<const TemplateValue<short>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<short>*>(&v)->getValue();
	case vt_unsigned_short:
		return dynamic_cast<const TemplateValue<unsigned short>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned short>*>(&v)->getValue();
	case vt_long:
		return dynamic_cast<const TemplateValue<long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<long>*>(&v)->getValue();
	case vt_unsigned_long:
		return dynamic_cast<const TemplateValue<unsigned long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned long>*>(&v)->getValue();
	case vt_long_long:
		return dynamic_cast<const TemplateValue<long long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<long long>*>(&v)->getValue();
	case vt_unsigned_long_long:
		return dynamic_cast<const TemplateValue<unsigned long long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned long long>*>(&v)->getValue();
	case vt_float:
		if (_precise)
			return dynamic_cast<const TemplateValue<float>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<float>*>(&v)->getValue();
		else
			return fabs(dynamic_cast<const TemplateValue<float>*>(this)->getValue() -
				dynamic_cast<const TemplateValue<float>*>(&v)->getValue()) < FLTYPE::Epslf();
	case vt_double:
		if (_precise)
			return dynamic_cast<const TemplateValue<double>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<double>*>(&v)->getValue();
		else
			return fabs(dynamic_cast<const TemplateValue<double>*>(this)->getValue() -
				dynamic_cast<const TemplateValue<double>*>(&v)->getValue()) < FLTYPE::Epsld();
	case vt_string:
		return dynamic_cast<const TemplateValue<std::string>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<std::string>*>(&v)->getValue();
	case vt_wstring:
		return dynamic_cast<const TemplateValue<std::wstring>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<std::wstring>*>(&v)->getValue();
	case vt_Point:
		return dynamic_cast<const TemplateValue<FLTYPE::Point>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Point>*>(&v)->getValue();
	case vt_Vector:
		return dynamic_cast<const TemplateValue<FLTYPE::Vector>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Vector>*>(&v)->getValue();
	case vt_BBox:
		return dynamic_cast<const TemplateValue<FLTYPE::BBox>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::BBox>*>(&v)->getValue();
	case vt_HSVColor:
		return dynamic_cast<const TemplateValue<FLTYPE::HSVColor>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::HSVColor>*>(&v)->getValue();
	case vt_Color:
		return dynamic_cast<const TemplateValue<FLTYPE::Color>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Color>*>(&v)->getValue();
	case vt_Plane:
		return dynamic_cast<const TemplateValue<FLTYPE::Plane>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Plane>*>(&v)->getValue();
	case vt_PlaneSet:
		return dynamic_cast<const TemplateValue<FLTYPE::PlaneSet>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::PlaneSet>*>(&v)->getValue();
	case vt_Quaternion:
		return dynamic_cast<const TemplateValue<FLTYPE::Quaternion>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Quaternion>*>(&v)->getValue();
	case vt_Ray:
		return dynamic_cast<const TemplateValue<FLTYPE::Ray>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Ray>*>(&v)->getValue();
	case vt_Transform:
		return dynamic_cast<const TemplateValue<FLTYPE::Transform>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Transform>*>(&v)->getValue();
	case vt_GLfloat4:
		return dynamic_cast<const TemplateValue<FLTYPE::GLfloat4>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::GLfloat4>*>(&v)->getValue();
	case vt_GLint4:
		return dynamic_cast<const TemplateValue<FLTYPE::GLint4>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::GLint4>*>(&v)->getValue();
	default:
		return false;
	}
}

inline bool Value::operator != (const Value& v) const
{
	return !(*this == v);
}

inline bool Value::sync(Event& event)
{
	Value* value = dynamic_cast<Value*>(event.sender);
	if (!value || _type != value->_type)
		return false;

	switch (_etype)
	{
	case vt_pReferenced:
		dynamic_cast<TemplateValue<Referenced*>*>(this)->setValue(
			dynamic_cast<TemplateValue<Referenced*>*>(value)->getValue(), event);
		break;
	case vt_bool:
		dynamic_cast<TemplateValue<bool>*>(this)->setValue(
			dynamic_cast<TemplateValue<bool>*>(value)->getValue(), event);
		break;
	case vt_char:
		dynamic_cast<TemplateValue<char>*>(this)->setValue(
			dynamic_cast<TemplateValue<char>*>(value)->getValue(), event);
		break;
	case vt_unsigned_char:
		dynamic_cast<TemplateValue<unsigned char>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned char>*>(value)->getValue(), event);
		break;
	case vt_short:
		dynamic_cast<TemplateValue<short>*>(this)->setValue(
			dynamic_cast<TemplateValue<short>*>(value)->getValue(), event);
		break;
	case vt_unsigned_short:
		dynamic_cast<TemplateValue<unsigned short>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned short>*>(value)->getValue(), event);
		break;
	case vt_long:
		dynamic_cast<TemplateValue<long>*>(this)->setValue(
			dynamic_cast<TemplateValue<long>*>(value)->getValue(), event);
		break;
	case vt_unsigned_long:
		dynamic_cast<TemplateValue<unsigned long>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned long>*>(value)->getValue(), event);
		break;
	case vt_long_long:
		dynamic_cast<TemplateValue<long long>*>(this)->setValue(
			dynamic_cast<TemplateValue<long long>*>(value)->getValue(), event);
		break;
	case vt_unsigned_long_long:
		dynamic_cast<TemplateValue<unsigned long long>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned long long>*>(value)->getValue(), event);
		break;
	case vt_float:
		dynamic_cast<TemplateValue<float>*>(this)->setValue(
			dynamic_cast<TemplateValue<float>*>(value)->getValue(), event);
		break;
	case vt_double:
		dynamic_cast<TemplateValue<double>*>(this)->setValue(
			dynamic_cast<TemplateValue<double>*>(value)->getValue(), event);
		break;
	case vt_string:
		dynamic_cast<TemplateValue<std::string>*>(this)->setValue(
			dynamic_cast<TemplateValue<std::string>*>(value)->getValue(), event);
		break;
	case vt_wstring:
		dynamic_cast<TemplateValue<std::wstring>*>(this)->setValue(
			dynamic_cast<TemplateValue<std::wstring>*>(value)->getValue(), event);
		break;
	case vt_Point:
		dynamic_cast<TemplateValue<FLTYPE::Point>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Point>*>(value)->getValue(), event);
		break;
	case vt_Vector:
		dynamic_cast<TemplateValue<FLTYPE::Vector>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Vector>*>(value)->getValue(), event);
		break;
	case vt_BBox:
		dynamic_cast<TemplateValue<FLTYPE::BBox>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::BBox>*>(value)->getValue(), event);
		break;
	case vt_HSVColor:
		dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(value)->getValue(), event);
		break;
	case vt_Color:
		dynamic_cast<TemplateValue<FLTYPE::Color>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Color>*>(value)->getValue(), event);
		break;
	case vt_Plane:
		dynamic_cast<TemplateValue<FLTYPE::Plane>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Plane>*>(value)->getValue(), event);
		break;
	case vt_PlaneSet:
		dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(value)->getValue(), event);
		break;
	case vt_Quaternion:
		dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(value)->getValue(), event);
		break;
	case vt_Ray:
		dynamic_cast<TemplateValue<FLTYPE::Ray>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Ray>*>(value)->getValue(), event);
		break;
	case vt_Transform:
		dynamic_cast<TemplateValue<FLTYPE::Transform>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Transform>*>(value)->getValue(), event);
		break;
	case vt_GLfloat4:
		dynamic_cast<TemplateValue<FLTYPE::GLfloat4>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::GLfloat4>*>(value)->getValue(), event);
		break;
	case vt_GLint4:
		dynamic_cast<TemplateValue<FLTYPE::GLint4>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::GLint4>*>(value)->getValue(), event);
		break;
	default:
		return false;
	}

	return true;
}

inline void Value::notify(Event& event)
{
	notifyObservers(event);
}

inline std::ostream& FL::operator<<(std::ostream& os, const Value& v)
{
	switch (v._etype)
	{
	case Value::vt_pReferenced:
		os << dynamic_cast<const TemplateValue<Referenced*>*>(&v)->getValue();
		break;
	case Value::vt_bool:
		os << dynamic_cast<const TemplateValue<bool>*>(&v)->getValue();
		break;
	case Value::vt_char:
		os << dynamic_cast<const TemplateValue<char>*>(&v)->getValue();
		break;
	case Value::vt_unsigned_char:
		os << dynamic_cast<const TemplateValue<unsigned char>*>(&v)->getValue();
		break;
	case Value::vt_short:
		os << dynamic_cast<const TemplateValue<short>*>(&v)->getValue();
		break;
	case Value::vt_unsigned_short:
		os << dynamic_cast<const TemplateValue<unsigned short>*>(&v)->getValue();
		break;
	case Value::vt_long:
		os << dynamic_cast<const TemplateValue<long>*>(&v)->getValue();
		break;
	case Value::vt_unsigned_long:
		os << dynamic_cast<const TemplateValue<unsigned long>*>(&v)->getValue();
		break;
	case Value::vt_long_long:
		os << dynamic_cast<const TemplateValue<long long>*>(&v)->getValue();
		break;
	case Value::vt_unsigned_long_long:
		os << dynamic_cast<const TemplateValue<unsigned long long>*>(&v)->getValue();
		break;
	case Value::vt_float:
		os << dynamic_cast<const TemplateValue<float>*>(&v)->getValue();
		break;
	case Value::vt_double:
		os << dynamic_cast<const TemplateValue<double>*>(&v)->getValue();
		break;
	case Value::vt_string:
		os << dynamic_cast<const TemplateValue<std::string>*>(&v)->getValue();
		break;
	case Value::vt_wstring:
		{
			//convert for os
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			os << converter.to_bytes(dynamic_cast<const TemplateValue<std::wstring>*>(&v)->getValue());
		}
		break;
	case Value::vt_Point:
		os << dynamic_cast<const TemplateValue<FLTYPE::Point>*>(&v)->getValue();
		break;
	case Value::vt_Vector:
		os << dynamic_cast<const TemplateValue<FLTYPE::Vector>*>(&v)->getValue();
		break;
	case Value::vt_BBox:
		os << dynamic_cast<const TemplateValue<FLTYPE::BBox>*>(&v)->getValue();
		break;
	case Value::vt_HSVColor:
		os << dynamic_cast<const TemplateValue<FLTYPE::HSVColor>*>(&v)->getValue();
		break;
	case Value::vt_Color:
		os << dynamic_cast<const TemplateValue<FLTYPE::Color>*>(&v)->getValue();
		break;
	case Value::vt_Plane:
		os << dynamic_cast<const TemplateValue<FLTYPE::Plane>*>(&v)->getValue();
		break;
	case Value::vt_PlaneSet:
		os << dynamic_cast<const TemplateValue<FLTYPE::PlaneSet>*>(&v)->getValue();
		break;
	case Value::vt_Quaternion:
		os << dynamic_cast<const TemplateValue<FLTYPE::Quaternion>*>(&v)->getValue();
		break;
	case Value::vt_Ray:
		os << dynamic_cast<const TemplateValue<FLTYPE::Ray>*>(&v)->getValue();
		break;
	case Value::vt_Transform:
		os << dynamic_cast<const TemplateValue<FLTYPE::Transform>*>(&v)->getValue();
		break;
	case Value::vt_GLfloat4:
		os << dynamic_cast<const TemplateValue<FLTYPE::GLfloat4>*>(&v)->getValue();
		break;
	case Value::vt_GLint4:
		os << dynamic_cast<const TemplateValue<FLTYPE::GLint4>*>(&v)->getValue();
		break;
	}
	return os;

}

inline bool ValueSet::operator == (const ValueSet &vs) const
{
	Values::iterator it1;
	Values::iterator it2;
	for (it1=const_cast<Values*>(&_values)->begin(),
		 it2=const_cast<Values*>(&(vs._values))->begin();
		 it1!=_values.end()&&
		 it2!=vs._values.end();
		 ++it1, ++it2)
	{
		if (*it1 != *it2)
			return false;
	}
	return true;
}

inline bool ValueSet::operator != (const ValueSet &vs) const
{
	return !(*this == vs);
}

}

#endif
