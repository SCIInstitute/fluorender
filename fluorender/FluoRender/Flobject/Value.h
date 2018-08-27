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

namespace FL
{
//name, type, value
typedef std::tuple<std::string, std::string, std::string> ValueTuple;

class Value : public Referenced
{
public:
	Value(std::string name = "", std::string type = "") : Referenced(), _name(name), _type(type) {}

	Value* clone();

	virtual const char* className() const { return "Value"; }

	std::string getName() { return _name; }
	std::string getType() { return _type; }

	inline bool operator == (const Value& v) const;
	inline bool operator != (const Value& v) const;

	inline bool sync(Value* value);

	friend inline std::ostream& operator<<(std::ostream& os, const Value& v);

protected:
	virtual ~Value();

	std::string _name;
	std::string _type;

	friend class ValueSet;
};

template<typename T>
class TemplateValue : public Value
{
public:

	TemplateValue() : Value() {}

	TemplateValue(const std::string& name, const std::string& type, const T& value) :
	Value(name,type), _value(value) {}

	TemplateValue<T>* clone()
	{
		TemplateValue<T>* tv = new TemplateValue<T>;
		tv->_value = _value;
		tv->_name = _name;
		tv->_type = _type;
		return tv;
	}

	void setValue(const T& value, bool notify)
	{
		if (value != _value)
		{
			_value = value;
			if (notify)
				notifyObserversOfChange();
		}
	}

	const T& getValue() const {return _value;}

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

	Value* findValue(const std::string &name);
	bool addValue(Value* value);
	bool removeValue(Value* value);
	bool removeValue(const std::string &name);
	//reset Referenced pointer to NULL
	bool resetRefPtr(Referenced* value);
	//value sync
	bool syncValue(Value* value);

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

	/** All the set value functions */
	bool setValue(ValueTuple&);
	//generic types
	bool setValue(const std::string &name, Referenced* value, bool notify = true);
	bool setValue(const std::string &name, bool value, bool notify = true);
	bool setValue(const std::string &name, char value, bool notify = true);
	bool setValue(const std::string &name, unsigned char value, bool notify = true);
	bool setValue(const std::string &name, short value, bool notify = true);
	bool setValue(const std::string &name, unsigned short value, bool notify = true);
	bool setValue(const std::string &name, long value, bool notify = true);
	bool setValue(const std::string &name, unsigned long value, bool notify = true);
	bool setValue(const std::string &name, long long value, bool notify = true);
	bool setValue(const std::string &name, unsigned long long value, bool notify = true);
	bool setValue(const std::string &name, float value, bool notify = true);
	bool setValue(const std::string &name, double value, bool notify = true);
	bool setValue(const std::string &name, const std::string &value, bool notify = true);
	bool setValue(const std::string &name, const std::wstring &value, bool notify = true);
	//add FluoRender's special types here
	bool setValue(const std::string &name, const FLTYPE::Point &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::Vector &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::BBox &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::HSVColor &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::Color &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::Plane &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::PlaneSet &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::Quaternion &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::Ray &value, bool notify = true);
	bool setValue(const std::string &name, const FLTYPE::Transform &value, bool notify = true);

	//toggle value for bool, result in value
	bool toggleValue(const std::string &name, bool &value, bool notify = true);

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

	if (_type == "Referenced*")
	{
		return dynamic_cast<const TemplateValue<Referenced*>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<Referenced*>*>(&v)->getValue();
	}
	else if (_type == "bool")
	{
		return dynamic_cast<const TemplateValue<bool>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<bool>*>(&v)->getValue();
	}
	else if (_type == "char")
	{
		return dynamic_cast<const TemplateValue<char>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<char>*>(&v)->getValue();
	}
	else if (_type == "unsigned char")
	{
		return dynamic_cast<const TemplateValue<unsigned char>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned char>*>(&v)->getValue();
	}
	else if (_type == "short")
	{
		return dynamic_cast<const TemplateValue<short>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<short>*>(&v)->getValue();
	}
	else if (_type == "unsigned short")
	{
		return dynamic_cast<const TemplateValue<unsigned short>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned short>*>(&v)->getValue();
	}
	else if (_type == "long")
	{
		return dynamic_cast<const TemplateValue<long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<long>*>(&v)->getValue();
	}
	else if (_type == "unsigned long")
	{
		return dynamic_cast<const TemplateValue<unsigned long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned long>*>(&v)->getValue();
	}
	else if (_type == "long long")
	{
		return dynamic_cast<const TemplateValue<long long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<long long>*>(&v)->getValue();
	}
	else if (_type == "unsigned long long")
	{
		return dynamic_cast<const TemplateValue<unsigned long long>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<unsigned long long>*>(&v)->getValue();
	}
	else if (_type == "float")
	{
		return dynamic_cast<const TemplateValue<float>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<float>*>(&v)->getValue();
	}
	else if (_type == "double")
	{
		return dynamic_cast<const TemplateValue<double>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<double>*>(&v)->getValue();
	}
	else if (_type == "string")
	{
		return dynamic_cast<const TemplateValue<std::string>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<std::string>*>(&v)->getValue();
	}
	else if (_type == "wstring")
	{
		return dynamic_cast<const TemplateValue<std::wstring>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<std::wstring>*>(&v)->getValue();
	}
	else if (_type == "Point")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Point>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Point>*>(&v)->getValue();
	}
	else if (_type == "Vector")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Vector>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Vector>*>(&v)->getValue();
	}
	else if (_type == "BBox")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::BBox>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::BBox>*>(&v)->getValue();
	}
	else if (_type == "HSVColor")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::HSVColor>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::HSVColor>*>(&v)->getValue();
	}
	else if (_type == "Color")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Color>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Color>*>(&v)->getValue();
	}
	else if (_type == "Plane")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Plane>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Plane>*>(&v)->getValue();
	}
	else if (_type == "PlaneSet")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::PlaneSet>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::PlaneSet>*>(&v)->getValue();
	}
	else if (_type == "Quaternion")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Quaternion>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Quaternion>*>(&v)->getValue();
	}
	else if (_type == "Ray")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Ray>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Ray>*>(&v)->getValue();
	}
	else if (_type == "Transform")
	{
		return dynamic_cast<const TemplateValue<FLTYPE::Transform>*>(this)->getValue() ==
			dynamic_cast<const TemplateValue<FLTYPE::Transform>*>(&v)->getValue();
	}
	else return false;
}

inline bool Value::operator != (const Value& v) const
{
	return !(*this == v);
}

inline bool Value::sync(Value* value)
{
	if (_type != value->_type ||
		_name != value->_name)
		return false;

	if (_type == "Referenced*")
	{
		dynamic_cast<TemplateValue<Referenced*>*>(this)->setValue(
			dynamic_cast<TemplateValue<Referenced*>*>(value)->getValue(), true);
	}
	else if (_type == "bool")
	{
		dynamic_cast<TemplateValue<bool>*>(this)->setValue(
			dynamic_cast<TemplateValue<bool>*>(value)->getValue(), true);
	}
	else if (_type == "char")
	{
		dynamic_cast<TemplateValue<char>*>(this)->setValue(
			dynamic_cast<TemplateValue<char>*>(value)->getValue(), true);
	}
	else if (_type == "unsigned char")
	{
		dynamic_cast<TemplateValue<unsigned char>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned char>*>(value)->getValue(), true);
	}
	else if (_type == "short")
	{
		dynamic_cast<TemplateValue<short>*>(this)->setValue(
			dynamic_cast<TemplateValue<short>*>(value)->getValue(), true);
	}
	else if (_type == "unsigned short")
	{
		dynamic_cast<TemplateValue<unsigned short>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned short>*>(value)->getValue(), true);
	}
	else if (_type == "long")
	{
		dynamic_cast<TemplateValue<long>*>(this)->setValue(
			dynamic_cast<TemplateValue<long>*>(value)->getValue(), true);
	}
	else if (_type == "unsigned long")
	{
		dynamic_cast<TemplateValue<unsigned long>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned long>*>(value)->getValue(), true);
	}
	else if (_type == "long long")
	{
		dynamic_cast<TemplateValue<long long>*>(this)->setValue(
			dynamic_cast<TemplateValue<long long>*>(value)->getValue(), true);
	}
	else if (_type == "unsigned long long")
	{
		dynamic_cast<TemplateValue<unsigned long long>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned long long>*>(value)->getValue(), true);
	}
	else if (_type == "float")
	{
		dynamic_cast<TemplateValue<float>*>(this)->setValue(
			dynamic_cast<TemplateValue<float>*>(value)->getValue(), true);
	}
	else if (_type == "double")
	{
		dynamic_cast<TemplateValue<double>*>(this)->setValue(
			dynamic_cast<TemplateValue<double>*>(value)->getValue(), true);
	}
	else if (_type == "string")
	{
		dynamic_cast<TemplateValue<std::string>*>(this)->setValue(
			dynamic_cast<TemplateValue<std::string>*>(value)->getValue(), true);
	}
	else if (_type == "wstring")
	{
		dynamic_cast<TemplateValue<std::wstring>*>(this)->setValue(
			dynamic_cast<TemplateValue<std::wstring>*>(value)->getValue(), true);
	}
	else if (_type == "Point")
	{
		dynamic_cast<TemplateValue<FLTYPE::Point>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Point>*>(value)->getValue(), true);
	}
	else if (_type == "Vector")
	{
		dynamic_cast<TemplateValue<FLTYPE::Vector>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Vector>*>(value)->getValue(), true);
	}
	else if (_type == "BBox")
	{
		dynamic_cast<TemplateValue<FLTYPE::BBox>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::BBox>*>(value)->getValue(), true);
	}
	else if (_type == "HSVColor")
	{
		dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(value)->getValue(), true);
	}
	else if (_type == "Color")
	{
		dynamic_cast<TemplateValue<FLTYPE::Color>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Color>*>(value)->getValue(), true);
	}
	else if (_type == "Plane")
	{
		dynamic_cast<TemplateValue<FLTYPE::Plane>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Plane>*>(value)->getValue(), true);
	}
	else if (_type == "PlaneSet")
	{
		dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(value)->getValue(), true);
	}
	else if (_type == "Quaternion")
	{
		dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(value)->getValue(), true);
	}
	else if (_type == "Ray")
	{
		dynamic_cast<TemplateValue<FLTYPE::Ray>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Ray>*>(value)->getValue(), true);
	}
	else if (_type == "Transform")
	{
		dynamic_cast<TemplateValue<FLTYPE::Transform>*>(this)->setValue(
			dynamic_cast<TemplateValue<FLTYPE::Transform>*>(value)->getValue(), true);
	}
	else return false;

	return true;
}

inline std::ostream& FL::operator<<(std::ostream& os, const Value& v)
{
	if (v._type == "Referenced*")
		os << dynamic_cast<const TemplateValue<Referenced*>*>(&v)->getValue();
	if (v._type == "bool")
		os << dynamic_cast<const TemplateValue<bool>*>(&v)->getValue();
	if (v._type == "char")
		os << dynamic_cast<const TemplateValue<char>*>(&v)->getValue();
	if (v._type == "unsigned char")
		os << dynamic_cast<const TemplateValue<unsigned char>*>(&v)->getValue();
	if (v._type == "short")
		os << dynamic_cast<const TemplateValue<short>*>(&v)->getValue();
	if (v._type == "unsigned short")
		os << dynamic_cast<const TemplateValue<unsigned short>*>(&v)->getValue();
	if (v._type == "long")
		os << dynamic_cast<const TemplateValue<long>*>(&v)->getValue();
	if (v._type == "unsigned long")
		os << dynamic_cast<const TemplateValue<unsigned long>*>(&v)->getValue();
	if (v._type == "long long")
		os << dynamic_cast<const TemplateValue<long long>*>(&v)->getValue();
	if (v._type == "unsigned long long")
		os << dynamic_cast<const TemplateValue<unsigned long long>*>(&v)->getValue();
	if (v._type == "float")
		os << dynamic_cast<const TemplateValue<float>*>(&v)->getValue();
	if (v._type == "double")
		os << dynamic_cast<const TemplateValue<double>*>(&v)->getValue();
	if (v._type == "string")
		os << dynamic_cast<const TemplateValue<std::string>*>(&v)->getValue();
	if (v._type == "wstring")
	{
		//convert for os
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		os << converter.to_bytes(dynamic_cast<const TemplateValue<std::wstring>*>(&v)->getValue());
	}
	if (v._type == "Point")
		os << dynamic_cast<const TemplateValue<FLTYPE::Point>*>(&v)->getValue();
	if (v._type == "Vector")
		os << dynamic_cast<const TemplateValue<FLTYPE::Vector>*>(&v)->getValue();
	if (v._type == "BBox")
		os << dynamic_cast<const TemplateValue<FLTYPE::BBox>*>(&v)->getValue();
	if (v._type == "HSVColor")
		os << dynamic_cast<const TemplateValue<FLTYPE::HSVColor>*>(&v)->getValue();
	if (v._type == "Color")
		os << dynamic_cast<const TemplateValue<FLTYPE::Color>*>(&v)->getValue();
	if (v._type == "Plane")
		os << dynamic_cast<const TemplateValue<FLTYPE::Plane>*>(&v)->getValue();
	if (v._type == "PlaneSet")
		os << dynamic_cast<const TemplateValue<FLTYPE::PlaneSet>*>(&v)->getValue();
	if (v._type == "Quaternion")
		os << dynamic_cast<const TemplateValue<FLTYPE::Quaternion>*>(&v)->getValue();
	if (v._type == "Ray")
		os << dynamic_cast<const TemplateValue<FLTYPE::Ray>*>(&v)->getValue();
	if (v._type == "Transform")
		os << dynamic_cast<const TemplateValue<FLTYPE::Transform>*>(&v)->getValue();
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
