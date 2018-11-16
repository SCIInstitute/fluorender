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

using namespace FL;

Value::~Value()
{
}

Value* Value::clone()
{
	switch (_etype)
	{
	case vt_pReferenced:
		return (dynamic_cast<TemplateValue<Referenced*>*>(this))->clone();
	case vt_bool:
		return (dynamic_cast<TemplateValue<bool>*>(this))->clone();
	case vt_char:
		return (dynamic_cast<TemplateValue<char>*>(this))->clone();
	case vt_unsigned_char:
		return (dynamic_cast<TemplateValue<unsigned char>*>(this))->clone();
	case vt_short:
		return (dynamic_cast<TemplateValue<short>*>(this))->clone();
	case vt_unsigned_short:
		return (dynamic_cast<TemplateValue<unsigned short>*>(this))->clone();
	case vt_long:
		return (dynamic_cast<TemplateValue<long>*>(this))->clone();
	case vt_unsigned_long:
		return (dynamic_cast<TemplateValue<unsigned long>*>(this))->clone();
	case vt_long_long:
		return (dynamic_cast<TemplateValue<long long>*>(this))->clone();
	case vt_unsigned_long_long:
		return (dynamic_cast<TemplateValue<unsigned long long>*>(this))->clone();
	case vt_float:
		return (dynamic_cast<TemplateValue<float>*>(this))->clone();
	case vt_double:
		return (dynamic_cast<TemplateValue<double>*>(this))->clone();
	case vt_string:
		return (dynamic_cast<TemplateValue<std::string>*>(this))->clone();
	case vt_wstring:
		return (dynamic_cast<TemplateValue<std::wstring>*>(this))->clone();
	case vt_Point:
		return (dynamic_cast<TemplateValue<FLTYPE::Point>*>(this))->clone();
	case vt_Vector:
		return (dynamic_cast<TemplateValue<FLTYPE::Vector>*>(this))->clone();
	case vt_BBox:
		return (dynamic_cast<TemplateValue<FLTYPE::BBox>*>(this))->clone();
	case vt_HSVColor:
		return (dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(this))->clone();
	case vt_Color:
		return (dynamic_cast<TemplateValue<FLTYPE::Color>*>(this))->clone();
	case vt_Plane:
		return (dynamic_cast<TemplateValue<FLTYPE::Plane>*>(this))->clone();
	case vt_PlaneSet:
		return (dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(this))->clone();
	case vt_Quaternion:
		return (dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(this))->clone();
	case vt_Ray:
		return (dynamic_cast<TemplateValue<FLTYPE::Ray>*>(this))->clone();
	case vt_Transform:
		return (dynamic_cast<TemplateValue<FLTYPE::Transform>*>(this))->clone();
	case vt_GLfloat4:
		return (dynamic_cast<TemplateValue<FLTYPE::GLfloat4>*>(this))->clone();
	case vt_GLint4:
		return (dynamic_cast<TemplateValue<FLTYPE::GLint4>*>(this))->clone();
	default:
		return 0;
	}
}

//observer functions
void Value::objectDeleted(Event& event)
{
	Referenced* refd = event.sender;
	if (refd && _etype == vt_pReferenced &&
		(dynamic_cast<TemplateValue<Referenced*>*>(
		this))->getValue() == refd)
	{
		refd->removeObserver(this);
		(dynamic_cast<TemplateValue<Referenced*>*>(this))->setValue(0, Event(Event::NOTIFY_NONE));
	}

	//remove observee
	removeObservee(refd);
}

void Value::processNotification(Event& event)
{
	Referenced* refd = event.sender;
	if (!refd)
		return;

	if (event.getNotifyFlags() & Event::NOTIFY_VALUE &&
		event.type == Event::EVENT_VALUE_CHANGED)
	{
		//I have tried to notify the observers of the value
		//but that is very dangerous, as an infinite loop may occur
		//here value is used to stop notifications when no change is detected
		sync(event);
	}
}

ValueSet::ValueSet(const ValueSet& vs, const CopyOp& copyop)
{
	if (copyop.getCopyFlags() & CopyOp::DEEP_COPY_VALUES)
	{
		for (auto it = vs._values.begin();
			it != vs._values.end(); ++it)
		{
			Value* value = it->second.get();
			addValue(value->clone());
		}
	}
	else
		_values = vs._values;
}

ValueSet::~ValueSet()
{
}

void ValueSet::clear()
{
	_values.clear();
}

Value* ValueSet::findValue(const std::string &name)
{
	auto it = _values.find(name);
	if (it != _values.end())
		return it->second.get();

	return 0;
}

bool ValueSet::containsValue(Value* value)
{
	if (!value)
		return false;
	Value* result = findValue(value->getName());
	if (result && result == value)
		return true;
	return false;
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

//add value functions
bool ValueSet::addValue(ValueTuple& vt)
{
	std::string name = std::get<0>(vt);
	std::string type = std::get<1>(vt);
	std::string value = std::get<2>(vt);
	std::istringstream iss(value);

	Value::ValueType etype(Value::vt_null);
	auto it = Value::_value_map.find(type);
	if (it != Value::_value_map.end())
		etype = it->second;

	switch (etype)
	{
	case Value::vt_pReferenced:
		{
			Referenced* v = 0;
			return addValue(name, v);
		}
	case Value::vt_bool:
		{
			bool v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_char:
		{
			char v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_unsigned_char:
		{
			unsigned char v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_short:
		{
			short v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_unsigned_short:
		{
			unsigned short v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_long:
		{
			long v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_unsigned_long:
		{
			unsigned long v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_long_long:
		{
			long long v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_unsigned_long_long:
		{
			unsigned long long v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_float:
		{
			float v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_double:
		{
			double v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_string:
		{
			return addValue(name, value);
		}
	case Value::vt_wstring:
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			return addValue(name, converter.from_bytes(value));
		}
	case Value::vt_Point:
		{
			FLTYPE::Point v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Vector:
		{
			FLTYPE::Vector v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_BBox:
		{
			FLTYPE::BBox v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_HSVColor:
		{
			FLTYPE::HSVColor v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Color:
		{
			FLTYPE::Color v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Plane:
		{
			FLTYPE::Plane v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_PlaneSet:
		{
			FLTYPE::PlaneSet v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Quaternion:
		{
			FLTYPE::Quaternion v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Ray:
		{
			FLTYPE::Ray v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Transform:
		{
			FLTYPE::Transform v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_GLfloat4:
		{
			FLTYPE::GLfloat4 v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_GLint4:
		{
			FLTYPE::GLint4 v;
			iss >> v;
			return addValue(name, v);
		}
	}
	return false;
}

bool ValueSet::addValue(const std::string &name, Referenced* value)
{
	if (!findValue(name))
	{
		TemplateValue<Referenced*>* val = new TemplateValue<Referenced*>(
			name, "Referenced*", value);
		if (value)
			value->addObserver(val);
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
		TemplateValue<bool>* val = new TemplateValue<bool>(
			name, "bool", value);
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
		TemplateValue<char>* val = new TemplateValue<char>(
			name, "char", value);
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
		TemplateValue<unsigned char>* val = new TemplateValue<unsigned char>(
			name, "unsigned char", value);
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
		TemplateValue<short>* val = new TemplateValue<short>(
			name, "short", value);
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
		TemplateValue<unsigned short>* val = new TemplateValue<unsigned short>(
			name, "unsigned short", value);
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
		TemplateValue<long>* val = new TemplateValue<long>(
			name, "long", value);
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
		TemplateValue<unsigned long>* val = new TemplateValue<unsigned long>(
			name, "unsigned long", value);
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
		TemplateValue<long long>* val = new TemplateValue<long long>(
			name, "long long", value);
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
		TemplateValue<unsigned long long>* val = new TemplateValue<unsigned long long>(
			name, "unsigned long long", value);
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
		TemplateValue<float>* val = new TemplateValue<float>(
			name, "float", value);
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
		TemplateValue<double>* val = new TemplateValue<double>(
			name, "double", value);
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
		TemplateValue<std::string>* val = new TemplateValue<std::string>(
			name, "string", value);
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
		TemplateValue<std::wstring>* val = new TemplateValue<std::wstring>(
			name, "wstring", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Point &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Point>* val = new TemplateValue<FLTYPE::Point>(
			name, "Point", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Vector &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Vector>* val = new TemplateValue<FLTYPE::Vector>(
			name, "Vector", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::BBox &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::BBox>* val = new TemplateValue<FLTYPE::BBox>(
			name, "BBox", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::HSVColor &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::HSVColor>* val = new TemplateValue<FLTYPE::HSVColor>(
			name, "HSVColor", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Color &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Color>* val = new TemplateValue<FLTYPE::Color>(
			name, "Color", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Plane &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Plane>* val = new TemplateValue<FLTYPE::Plane>(
			name, "Plane", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::PlaneSet &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::PlaneSet>* val = new TemplateValue<FLTYPE::PlaneSet>(
			name, "PlaneSet", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Quaternion &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Quaternion>* val = new TemplateValue<FLTYPE::Quaternion>(
			name, "Quaternion", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Ray &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Ray>* val = new TemplateValue<FLTYPE::Ray>(
			name, "Ray", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::Transform &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::Transform>* val = new TemplateValue<FLTYPE::Transform>(
			name, "Transform", value);

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::GLfloat4 &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::GLfloat4>* val = new TemplateValue<FLTYPE::GLfloat4>(
			name, "GLfloat4", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const FLTYPE::GLint4 &value)
{
	if (!findValue(name))
	{
		TemplateValue<FLTYPE::GLint4>* val = new TemplateValue<FLTYPE::GLint4>(
			name, "GLint4", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

//set value functions
bool ValueSet::setValue(ValueTuple& vt, Event& event)
{
	std::string name = std::get<0>(vt);
	std::string type = std::get<1>(vt);
	std::string value = std::get<2>(vt);
	std::istringstream iss(value);

	Value::ValueType etype(Value::vt_null);
	auto it = Value::_value_map.find(type);
	if (it != Value::_value_map.end())
		etype = it->second;

	switch (etype)
	{
	case Value::vt_pReferenced:
		{
			//cannot set value without the address
			//Referenced* v = 0;
			//return setValue(name, v);
			return false;
		}
	case Value::vt_bool:
		{
			bool v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_char:
		{
			char v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_unsigned_char:
		{
			unsigned char v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_short:
		{
			short v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_unsigned_short:
		{
			unsigned short v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_long:
		{
			long v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_unsigned_long:
		{
			unsigned long v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_long_long:
		{
			long long v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_unsigned_long_long:
		{
			unsigned long long v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_float:
		{
			float v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_double:
		{
			double v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_string:
		{
			return setValue(name, value, event);
		}
	case Value::vt_wstring:
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			return setValue(name, converter.from_bytes(value), event);
		}
	case Value::vt_Point:
		{
			FLTYPE::Point v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Vector:
		{
			FLTYPE::Vector v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_BBox:
		{
			FLTYPE::BBox v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_HSVColor:
		{
			FLTYPE::HSVColor v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Color:
		{
			FLTYPE::Color v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Plane:
		{
			FLTYPE::Plane v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_PlaneSet:
		{
			FLTYPE::PlaneSet v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Quaternion:
		{
			FLTYPE::Quaternion v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Ray:
		{
			FLTYPE::Ray v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Transform:
		{
			FLTYPE::Transform v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_GLfloat4:
		{
			FLTYPE::GLfloat4 v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_GLint4:
		{
			FLTYPE::GLint4 v;
			iss >> v;
			return setValue(name, v, event);
		}
	}
	return false;
}

bool ValueSet::setValue(const std::string &name, Referenced* value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_pReferenced)
	{
		Referenced* old_refd = dynamic_cast<TemplateValue<Referenced*>*>(val)->getValue();
		if (old_refd != value && old_refd)
			old_refd->removeObserver(val);
		(dynamic_cast<TemplateValue<Referenced*>*>(val))->setValue(value, event);
		if (old_refd != value && value)
			value->addObserver(val);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, bool value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_bool)
	{
		(dynamic_cast<TemplateValue<bool>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, char value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_char)
	{
		(dynamic_cast<TemplateValue<char>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned char value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_unsigned_char)
	{
		(dynamic_cast<TemplateValue<unsigned char>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, short value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_short)
	{
		(dynamic_cast<TemplateValue<short>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned short value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_unsigned_short)
	{
		(dynamic_cast<TemplateValue<unsigned short>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, long value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_long)
	{
		(dynamic_cast<TemplateValue<long>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned long value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_unsigned_long)
	{
		(dynamic_cast<TemplateValue<unsigned long>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, long long value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_long_long)
	{
		(dynamic_cast<TemplateValue<long long>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned long long value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_unsigned_long_long)
	{
		(dynamic_cast<TemplateValue<unsigned long long>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, float value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_float)
	{
		(dynamic_cast<TemplateValue<float>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, double value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_double)
	{
		(dynamic_cast<TemplateValue<double>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const std::string &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_string)
	{
		(dynamic_cast<TemplateValue<std::string>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const std::wstring &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_wstring)
	{
		(dynamic_cast<TemplateValue<std::wstring>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Point &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Point)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Point>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Vector &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Vector)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Vector>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::BBox &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_BBox)
	{
		(dynamic_cast<TemplateValue<FLTYPE::BBox>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::HSVColor &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_HSVColor)
	{
		(dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Color &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Color)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Color>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Plane &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Plane)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Plane>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::PlaneSet &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_PlaneSet)
	{
		(dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Quaternion &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Quaternion)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Ray &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Ray)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Ray>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::Transform &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Transform)
	{
		(dynamic_cast<TemplateValue<FLTYPE::Transform>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::GLfloat4 &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_GLfloat4)
	{
		(dynamic_cast<TemplateValue<FLTYPE::GLfloat4>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const FLTYPE::GLint4 &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_GLint4)
	{
		(dynamic_cast<TemplateValue<FLTYPE::GLint4>*>(val))->setValue(value, event);
		return true;
	}
	else
		return false;
}

//toggle
bool ValueSet::toggleValue(const std::string &name, bool &value, Event& event)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_bool)
	{
		value = (dynamic_cast<TemplateValue<bool>*>(val))->getValue();
		value = !value;
		(dynamic_cast<TemplateValue<bool>*>(val))->setValue(value, event);
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
		switch (val->_etype)
		{
		case Value::vt_pReferenced:
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
		case Value::vt_bool:
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
		case Value::vt_char:
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
		case Value::vt_unsigned_char:
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
		case Value::vt_short:
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
		case Value::vt_unsigned_short:
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
		case Value::vt_long:
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
		case Value::vt_unsigned_long:
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
		case Value::vt_long_long:
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
		case Value::vt_unsigned_long_long:
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
		case Value::vt_float:
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
		case Value::vt_double:
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
		case Value::vt_string:
			{
				std::get<1>(vt) = type;
				std::string v;
				if (getValue(name, v))
				{
					std::get<2>(vt) = v;
					return true;
				}
			}
		case Value::vt_wstring:
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
		case Value::vt_Point:
			{
				std::get<1>(vt) = type;
				FLTYPE::Point v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Vector:
			{
				std::get<1>(vt) = type;
				FLTYPE::Vector v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_BBox:
			{
				std::get<1>(vt) = type;
				FLTYPE::BBox v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_HSVColor:
			{
				std::get<1>(vt) = type;
				FLTYPE::HSVColor v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Color:
			{
				std::get<1>(vt) = type;
				FLTYPE::Color v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Plane:
			{
				std::get<1>(vt) = type;
				FLTYPE::Plane v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_PlaneSet:
			{
				std::get<1>(vt) = type;
				FLTYPE::PlaneSet v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Quaternion:
			{
				std::get<1>(vt) = type;
				FLTYPE::Quaternion v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Ray:
			{
				std::get<1>(vt) = type;
				FLTYPE::Ray v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Transform:
			{
				std::get<1>(vt) = type;
				FLTYPE::Transform v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_GLfloat4:
			{
				std::get<1>(vt) = type;
				FLTYPE::GLfloat4 v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_GLint4:
			{
				std::get<1>(vt) = type;
				FLTYPE::GLint4 v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		}
	}

	return false;
}

bool ValueSet::getValue(const std::string &name, Referenced** value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_pReferenced)
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
	if (val && val->_etype == Value::vt_bool)
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
	if (val && val->_etype == Value::vt_char)
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
	if (val && val->_etype == Value::vt_unsigned_char)
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
	if (val && val->_etype == Value::vt_short)
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
	if (val && val->_etype == Value::vt_unsigned_short)
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
	if (val && val->_etype == Value::vt_long)
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
	if (val && val->_etype == Value::vt_unsigned_long)
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
	if (val && val->_etype == Value::vt_long_long)
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
	if (val && val->_etype == Value::vt_unsigned_long_long)
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
	if (val && val->_etype == Value::vt_float)
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
	if (val && val->_etype == Value::vt_double)
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
	if (val && val->_etype == Value::vt_string)
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
	if (val && val->_etype == Value::vt_wstring)
	{
		value = (dynamic_cast<TemplateValue<std::wstring>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Point &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Point)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Point>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Vector &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Vector)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Vector>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::BBox &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_BBox)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::BBox>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::HSVColor &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_HSVColor)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::HSVColor>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Color &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Color)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Color>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Plane &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Plane)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Plane>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::PlaneSet &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_PlaneSet)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::PlaneSet>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Quaternion &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Quaternion)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Quaternion>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Ray &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Ray)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Ray>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::Transform &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_Transform)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::Transform>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::GLfloat4 &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_GLfloat4)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::GLfloat4>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

bool ValueSet::getValue(const std::string &name, FLTYPE::GLint4 &value)
{
	Value* val = findValue(name);
	if (val && val->_etype == Value::vt_GLint4)
	{
		value = (dynamic_cast<TemplateValue<FLTYPE::GLint4>*>(val))->getValue();
		return true;
	}
	else
		return false;
}

