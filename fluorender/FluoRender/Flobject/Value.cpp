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
#include <Flobject/Value.h>
#include <Flobject/Object.h>
#include <iostream>
#include <sstream>

using namespace flrd;

Value::~Value()
{
}

Value* Value::clone()
{
	if (_type == "Referenced*")
		return (dynamic_cast<TemplateValue<Referenced*>*>(this))->clone();
	else if (_type == "bool")
		return (dynamic_cast<TemplateValue<bool>*>(this))->clone();
	else if (_type == "char")
		return (dynamic_cast<TemplateValue<char>*>(this))->clone();
	else if (_type == "unsigned char")
		return (dynamic_cast<TemplateValue<unsigned char>*>(this))->clone();
	else if (_type == "short")
		return (dynamic_cast<TemplateValue<short>*>(this))->clone();
	else if (_type == "unsigned short")
		return (dynamic_cast<TemplateValue<unsigned short>*>(this))->clone();
	else if (_type == "long")
		return (dynamic_cast<TemplateValue<long>*>(this))->clone();
	else if (_type == "unsigned long")
		return (dynamic_cast<TemplateValue<unsigned long>*>(this))->clone();
	else if (_type == "long long")
		return (dynamic_cast<TemplateValue<long long>*>(this))->clone();
	else if (_type == "unsigned long long")
		return (dynamic_cast<TemplateValue<unsigned long long>*>(this))->clone();
	else if (_type == "float")
		return (dynamic_cast<TemplateValue<float>*>(this))->clone();
	else if (_type == "double")
		return (dynamic_cast<TemplateValue<double>*>(this))->clone();
	else if (_type == "string")
		return (dynamic_cast<TemplateValue<std::string>*>(this))->clone();
	else if (_type == "wstring")
		return (dynamic_cast<TemplateValue<std::wstring>*>(this))->clone();
	else if (_type == "Point")
		return (dynamic_cast<TemplateValue<fluo::Point>*>(this))->clone();
	else if (_type == "Vector")
		return (dynamic_cast<TemplateValue<fluo::Vector>*>(this))->clone();
	else if (_type == "BBox")
		return (dynamic_cast<TemplateValue<fluo::BBox>*>(this))->clone();
	else if (_type == "HSVColor")
		return (dynamic_cast<TemplateValue<fluo::HSVColor>*>(this))->clone();
	else if (_type == "Color")
		return (dynamic_cast<TemplateValue<fluo::Color>*>(this))->clone();
	else if (_type == "Plane")
		return (dynamic_cast<TemplateValue<fluo::Plane>*>(this))->clone();
	else if (_type == "PlaneSet")
		return (dynamic_cast<TemplateValue<fluo::PlaneSet>*>(this))->clone();
	else if (_type == "Quaternion")
		return (dynamic_cast<TemplateValue<fluo::Quaternion>*>(this))->clone();
	else if (_type == "Ray")
		return (dynamic_cast<TemplateValue<fluo::Ray>*>(this))->clone();
	else if (_type == "Transform")
		return (dynamic_cast<TemplateValue<fluo::Transform>*>(this))->clone();
	else
		return 0;
}

ValueSet::ValueSet(const ValueSet& vs, const CopyOp& copyop)
{
	if (copyop.getCopyFlags() & CopyOp::SHALLOW_COPY)
		_values = vs._values;
	else
	{
		for (auto it = vs._values.begin();
			it != vs._values.end(); ++it)
		{
			Value* value = it->second.get();
			addValue(value->clone());
		}
	}
}

ValueSet::~ValueSet()
{
}

Value* ValueSet::findValue(const std::string &name)
{
	auto it = _values.find(name);
	if (it != _values.end())
		return it->second.get();

	return 0;
}

bool ValueSet::addValue(Value* value)
{
	if (!value) return false;
	auto it = _values.find(value->_name);
	if (it != _values.end())
		return false;

	_values.insert(std::pair<std::string, ref_ptr<Value>>(value->_name, value));
	return true;
}

bool ValueSet::removeValue(Value* value)
{
	if (!value) return false;
	auto it = _values.find(value->_name);
	if (it != _values.end())
	{
		if (it->second.get() == value)
		{
			_values.erase(it);
			return true;
		}
	}

	return false;
}

bool ValueSet::removeValue(const std::string &name)
{
	Value* value = findValue(name);
	if (value && removeValue(value))
		return true;
	else
		return false;
}

//reset Referenced pointer to NULL
bool ValueSet::resetRefPtr(Referenced* value)
{
	if (!value) return false;
	for (Values::iterator it=_values.begin();
		it!=_values.end(); ++it)
	{
		if (it->second->getType() == "Referenced*" &&
			(dynamic_cast<TemplateValue<Referenced*>*>(it->second.get()))->getValue() == value)
		{
			(dynamic_cast<TemplateValue<Referenced*>*>(it->second.get()))->setValue(0, true);
			return true;
		}
	}

	return false;
}

//value sync
bool ValueSet::syncValue(Value* value)
{
	if (!value) return false;
	Value* my_value = findValue(value->_name);
	if (my_value->getType() == value->getType())
		return my_value->sync(value);

	return false;
}

//add value functions
bool ValueSet::addValue(ValueTuple& vt)
{
	std::string name = std::get<0>(vt);
	std::string type = std::get<1>(vt);
	std::string value = std::get<2>(vt);
	std::istringstream iss(value);

	if (type == "Referenced*")
	{
		Referenced* v = 0;
		return addValue(name, v);
	}
	else if (type == "bool")
	{
		bool v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "char")
	{
		char v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "unsigned char")
	{
		unsigned char v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "short")
	{
		short v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "unsigned short")
	{
		unsigned short v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "long")
	{
		long v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "unsigned long")
	{
		unsigned long v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "long long")
	{
		long long v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "unsigned long long")
	{
		unsigned long long v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "float")
	{
		float v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "double")
	{
		double v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "string")
	{
		return addValue(name, value);
	}
	else if (type == "wstring")
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		return addValue(name, converter.from_bytes(value));
	}
	else if (type == "Point")
	{
		fluo::Point v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "Vector")
	{
		fluo::Vector v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "BBox")
	{
		fluo::BBox v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "HSVColor")
	{
		fluo::HSVColor v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "Color")
	{
		fluo::Color v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "Plane")
	{
		fluo::Plane v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "PlaneSet")
	{
		fluo::PlaneSet v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "Quaternion")
	{
		fluo::Quaternion v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "Ray")
	{
		fluo::Ray v;
		iss >> v;
		return addValue(name, v);
	}
	else if (type == "Transform")
	{
		fluo::Transform v;
		iss >> v;
		return addValue(name, v);
	}

	return false;
}

bool ValueSet::addValue(const std::string &name, Referenced* value)
{
	if (!findValue(name))
	{
		TemplateValue<Referenced*>* val = new TemplateValue<Referenced*>;
		val->_value = value;
		val->_name = name;
		val->_type = "Referenced*";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, bool value)
{
	if (!findValue(name))
	{
		TemplateValue<bool>* val = new TemplateValue<bool>;
		val->_value =value;
		val->_name = name;
		val->_type = "bool";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, char value)
{
	if (!findValue(name))
	{
		TemplateValue<char>* val = new TemplateValue<char>;
		val->_value =value;
		val->_name = name;
		val->_type = "char";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, unsigned char value)
{
	if (!findValue(name))
	{
		TemplateValue<unsigned char>* val = new TemplateValue<unsigned char>;
		val->_value =value;
		val->_name = name;
		val->_type = "unsigned char";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, short value)
{
	if (!findValue(name))
	{
		TemplateValue<short>* val = new TemplateValue<short>;
		val->_value =value;
		val->_name = name;
		val->_type = "short";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, unsigned short value)
{
	if (!findValue(name))
	{
		TemplateValue<unsigned short>* val = new TemplateValue<unsigned short>;
		val->_value =value;
		val->_name = name;
		val->_type = "unsigned short";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, long value)
{
	if (!findValue(name))
	{
		TemplateValue<long>* val = new TemplateValue<long>;
		val->_value =value;
		val->_name = name;
		val->_type = "long";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, unsigned long value)
{
	if (!findValue(name))
	{
		TemplateValue<unsigned long>* val = new TemplateValue<unsigned long>;
		val->_value =value;
		val->_name = name;
		val->_type = "unsigned long";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, long long value)
{
	if (!findValue(name))
	{
		TemplateValue<long long>* val = new TemplateValue<long long>;
		val->_value =value;
		val->_name = name;
		val->_type = "long long";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, unsigned long long value)
{
	if (!findValue(name))
	{
		TemplateValue<unsigned long long>* val = new TemplateValue<unsigned long long>;
		val->_value =value;
		val->_name = name;
		val->_type = "unsigned long long";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, float value)
{
	if (!findValue(name))
	{
		TemplateValue<float>* val = new TemplateValue<float>;
		val->_value =value;
		val->_name = name;
		val->_type = "float";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, double value)
{
	if (!findValue(name))
	{
		TemplateValue<double>* val = new TemplateValue<double>;
		val->_value =value;
		val->_name = name;
		val->_type = "double";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const std::string &value)
{
	if (!findValue(name))
	{
		TemplateValue<std::string>* val = new TemplateValue<std::string>;
		val->_value =value;
		val->_name = name;
		val->_type = "string";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const std::wstring &value)
{
	if (!findValue(name))
	{
		TemplateValue<std::wstring>* val = new TemplateValue<std::wstring>;
		val->_value = value;
		val->_name = name;
		val->_type = "wstring";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Point &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Point>* val = new TemplateValue<fluo::Point>;
		val->_value =value;
		val->_name = name;
		val->_type = "Point";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Vector &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Vector>* val = new TemplateValue<fluo::Vector>;
		val->_value = value;
		val->_name = name;
		val->_type = "Vector";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::BBox &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::BBox>* val = new TemplateValue<fluo::BBox>;
		val->_value = value;
		val->_name = name;
		val->_type = "BBox";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::HSVColor &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::HSVColor>* val = new TemplateValue<fluo::HSVColor>;
		val->_value = value;
		val->_name = name;
		val->_type = "HSVColor";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Color &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Color>* val = new TemplateValue<fluo::Color>;
		val->_value = value;
		val->_name = name;
		val->_type = "Color";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Plane &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Plane>* val = new TemplateValue<fluo::Plane>;
		val->_value = value;
		val->_name = name;
		val->_type = "Plane";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::PlaneSet &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::PlaneSet>* val = new TemplateValue<fluo::PlaneSet>;
		val->_value = value;
		val->_name = name;
		val->_type = "PlaneSet";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Quaternion &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Quaternion>* val = new TemplateValue<fluo::Quaternion>;
		val->_value = value;
		val->_name = name;
		val->_type = "Quaternion";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Ray &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Ray>* val = new TemplateValue<fluo::Ray>;
		val->_value = value;
		val->_name = name;
		val->_type = "Ray";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const fluo::Transform &value)
{
	if (!findValue(name))
	{
		TemplateValue<fluo::Transform>* val = new TemplateValue<fluo::Transform>;
		val->_value = value;
		val->_name = name;
		val->_type = "Transform";

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

//set value functions
bool ValueSet::setValue(ValueTuple& vt, bool notify)
{
	std::string name = std::get<0>(vt);
	std::string type = std::get<1>(vt);
	std::string value = std::get<2>(vt);
	std::istringstream iss(value);

	if (type == "Referenced*")
	{
		//cannot set value without the address
		//Referenced* v = 0;
		//return setValue(name, v);
		return false;
	}
	else if (type == "bool")
	{
		bool v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "char")
	{
		char v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "unsigned char")
	{
		unsigned char v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "short")
	{
		short v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "unsigned short")
	{
		unsigned short v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "long")
	{
		long v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "unsigned long")
	{
		unsigned long v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "long long")
	{
		long long v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "unsigned long long")
	{
		unsigned long long v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "float")
	{
		float v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "double")
	{
		double v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "string")
	{
		return setValue(name, value, notify);
	}
	else if (type == "wstring")
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		return setValue(name, converter.from_bytes(value), notify);
	}
	else if (type == "Point")
	{
		fluo::Point v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "Vector")
	{
		fluo::Vector v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "BBox")
	{
		fluo::BBox v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "HSVColor")
	{
		fluo::HSVColor v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "Color")
	{
		fluo::Color v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "Plane")
	{
		fluo::Plane v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "PlaneSet")
	{
		fluo::PlaneSet v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "Quaternion")
	{
		fluo::Quaternion v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "Ray")
	{
		fluo::Ray v;
		iss >> v;
		return setValue(name, v, notify);
	}
	else if (type == "Transform")
	{
		fluo::Transform v;
		iss >> v;
		return setValue(name, v, notify);
	}

	return false;
}

bool ValueSet::setValue(const std::string &name, Referenced* value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="Referenced*")
	{
		(dynamic_cast<TemplateValue<Referenced*>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, bool value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="bool")
	{
		(dynamic_cast<TemplateValue<bool>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, char value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="char")
	{
		(dynamic_cast<TemplateValue<char>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned char value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned char")
	{
		(dynamic_cast<TemplateValue<unsigned char>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, short value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="short")
	{
		(dynamic_cast<TemplateValue<short>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned short value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned short")
	{
		(dynamic_cast<TemplateValue<unsigned short>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, long value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="long")
	{
		(dynamic_cast<TemplateValue<long>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned long value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned long")
	{
		(dynamic_cast<TemplateValue<unsigned long>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, long long value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="long long")
	{
		(dynamic_cast<TemplateValue<long long>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned long long value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned long long")
	{
		(dynamic_cast<TemplateValue<unsigned long long>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, float value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="float")
	{
		(dynamic_cast<TemplateValue<float>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, double value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="double")
	{
		(dynamic_cast<TemplateValue<double>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const std::string &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="string")
	{
		(dynamic_cast<TemplateValue<std::string>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const std::wstring &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "wstring")
	{
		(dynamic_cast<TemplateValue<std::wstring>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Point &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type=="Point")
	{
		(dynamic_cast<TemplateValue<fluo::Point>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Vector &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "Vector")
	{
		(dynamic_cast<TemplateValue<fluo::Vector>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::BBox &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "BBox")
	{
		(dynamic_cast<TemplateValue<fluo::BBox>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::HSVColor &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "HSVColor")
	{
		(dynamic_cast<TemplateValue<fluo::HSVColor>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Color &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "Color")
	{
		(dynamic_cast<TemplateValue<fluo::Color>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Plane &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "Plane")
	{
		(dynamic_cast<TemplateValue<fluo::Plane>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::PlaneSet &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "PlaneSet")
	{
		(dynamic_cast<TemplateValue<fluo::PlaneSet>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Quaternion &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "Quaternion")
	{
		(dynamic_cast<TemplateValue<fluo::Quaternion>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Ray &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "Ray")
	{
		(dynamic_cast<TemplateValue<fluo::Ray>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const fluo::Transform &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "Transform")
	{
		(dynamic_cast<TemplateValue<fluo::Transform>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

//toggle
bool ValueSet::toggleValue(const std::string &name, bool &value, bool notify)
{
	Value* val = findValue(name);
	if (val && val->_type == "bool")
	{
		value = (dynamic_cast<TemplateValue<bool>*>(val))->getValue();
		value = !value;
		(dynamic_cast<TemplateValue<bool>*>(val))->setValue(value, notify);
		return true;
	}
	else
		return false;
}

//get value functions
bool ValueSet::getValue(ValueTuple& vt)
{
	std::string name = std::get<0>(vt);
	std::string type;
	std::ostringstream oss;

	Value* val = findValue(name);
	if (val)
	{
		type = val->getType();
		if (type == "Referenced*")
		{
			std::get<1>(vt) = type;
			//tentative:
			//get id if it's an object
			Referenced* v;
			if (getValue(name, &v))
			{
				Object* obj = dynamic_cast<Object*>(v);
				if (obj)
				{
					oss << obj->getId();
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		}
		else if (type == "bool")
		{
			std::get<1>(vt) = type;
			bool v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "char")
		{
			std::get<1>(vt) = type;
			char v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "unsigned char")
		{
			std::get<1>(vt) = type;
			unsigned char v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "short")
		{
			std::get<1>(vt) = type;
			short v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "unsigned short")
		{
			std::get<1>(vt) = type;
			unsigned short v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "long")
		{
			std::get<1>(vt) = type;
			long v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "unsigned long")
		{
			std::get<1>(vt) = type;
			unsigned long v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "long long")
		{
			std::get<1>(vt) = type;
			long long v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "unsigned long long")
		{
			std::get<1>(vt) = type;
			unsigned long long v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "float")
		{
			std::get<1>(vt) = type;
			float v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "double")
		{
			std::get<1>(vt) = type;
			double v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "string")
		{
			std::get<1>(vt) = type;
			std::string v;
			if (getValue(name, v))
			{
				std::get<2>(vt) = v;
				return true;
			}
		}
		else if (type == "wstring")
		{
			std::get<1>(vt) = type;
			std::wstring v;
			if (getValue(name, v))
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
				oss << converter.to_bytes(v);
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Point")
		{
			std::get<1>(vt) = type;
			fluo::Point v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Vector")
		{
			std::get<1>(vt) = type;
			fluo::Vector v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "BBox")
		{
			std::get<1>(vt) = type;
			fluo::BBox v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "HSVColor")
		{
			std::get<1>(vt) = type;
			fluo::HSVColor v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Color")
		{
			std::get<1>(vt) = type;
			fluo::Color v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Plane")
		{
			std::get<1>(vt) = type;
			fluo::Plane v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "PlaneSet")
		{
			std::get<1>(vt) = type;
			fluo::PlaneSet v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Quaternion")
		{
			std::get<1>(vt) = type;
			fluo::Quaternion v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Ray")
		{
			std::get<1>(vt) = type;
			fluo::Ray v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
		else if (type == "Transform")
		{
			std::get<1>(vt) = type;
			fluo::Transform v;
			if (getValue(name, v))
			{
				oss << v;
				std::get<2>(vt) = oss.str();
				return true;
			}
		}
	}

	return false;
}

bool ValueSet::getValue(const std::string &name, Referenced** value)
{
	Value* val = findValue(name);
	if (val && val->_type=="Referenced*")
	{
		*value = const_cast<Referenced*>((dynamic_cast< TemplateValue< Referenced * > * >(val))->getValue());
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, bool &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="bool")
	{
		value = (dynamic_cast<TemplateValue<bool>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, char &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="char")
	{
		value = (dynamic_cast<TemplateValue<char>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, unsigned char &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned char")
	{
		value = (dynamic_cast<TemplateValue<unsigned char>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, short &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="short")
	{
		value = (dynamic_cast<TemplateValue<short>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, unsigned short &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned short")
	{
		value = (dynamic_cast<TemplateValue<unsigned short>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, long &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="long")
	{
		value = (dynamic_cast<TemplateValue<long>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, unsigned long &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned long")
	{
		value = (dynamic_cast<TemplateValue<unsigned long>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, long long &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="long long")
	{
		value = (dynamic_cast<TemplateValue<long long>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, unsigned long long &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned long long")
	{
		value = (dynamic_cast<TemplateValue<unsigned long long>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, float &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="float")
	{
		value = (dynamic_cast<TemplateValue<float>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, double &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="double")
	{
		value = (dynamic_cast<TemplateValue<double>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, std::string &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="string")
	{
		value = (dynamic_cast<TemplateValue<std::string>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, std::wstring &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "wstring")
	{
		value = (dynamic_cast<TemplateValue<std::wstring>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Point &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="Point")
	{
		value = (dynamic_cast<TemplateValue<fluo::Point>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Vector &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "Vector")
	{
		value = (dynamic_cast<TemplateValue<fluo::Vector>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::BBox &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "BBox")
	{
		value = (dynamic_cast<TemplateValue<fluo::BBox>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::HSVColor &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "HSVColor")
	{
		value = (dynamic_cast<TemplateValue<fluo::HSVColor>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Color &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "Color")
	{
		value = (dynamic_cast<TemplateValue<fluo::Color>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Plane &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "Plane")
	{
		value = (dynamic_cast<TemplateValue<fluo::Plane>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::PlaneSet &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "PlaneSet")
	{
		value = (dynamic_cast<TemplateValue<fluo::PlaneSet>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Quaternion &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "Quaternion")
	{
		value = (dynamic_cast<TemplateValue<fluo::Quaternion>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Ray &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "Ray")
	{
		value = (dynamic_cast<TemplateValue<fluo::Ray>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, fluo::Transform &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "Transform")
	{
		value = (dynamic_cast<TemplateValue<fluo::Transform>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

