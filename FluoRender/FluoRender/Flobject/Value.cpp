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
#include <Value.hpp>
#include <Object.hpp>
#include <iostream>
#include <sstream>

using namespace fluo;

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
		return (dynamic_cast<TemplateValue<Point>*>(this))->clone();
	case vt_Vector:
		return (dynamic_cast<TemplateValue<Vector>*>(this))->clone();
	case vt_BBox:
		return (dynamic_cast<TemplateValue<BBox>*>(this))->clone();
	case vt_HSVColor:
		return (dynamic_cast<TemplateValue<HSVColor>*>(this))->clone();
	case vt_Color:
		return (dynamic_cast<TemplateValue<Color>*>(this))->clone();
	case vt_Plane:
		return (dynamic_cast<TemplateValue<Plane>*>(this))->clone();
	case vt_PlaneSet:
		return (dynamic_cast<TemplateValue<PlaneSet>*>(this))->clone();
	case vt_Quaternion:
		return (dynamic_cast<TemplateValue<Quaternion>*>(this))->clone();
	case vt_Ray:
		return (dynamic_cast<TemplateValue<Ray>*>(this))->clone();
	case vt_Transform:
		return (dynamic_cast<TemplateValue<Transform>*>(this))->clone();
	case vt_Vector4f:
		return (dynamic_cast<TemplateValue<Vector4f>*>(this))->clone();
	case vt_Vector4i:
		return (dynamic_cast<TemplateValue<Vector4i>*>(this))->clone();
	default:
        return nullptr;
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
        Event newEvent(Event::NOTIFY_NONE);
		refd->removeObserver(this);
        (dynamic_cast<TemplateValue<Referenced*>*>(this))->setValue(nullptr, newEvent);
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

    return nullptr;
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
	auto it = Value::value_map().find(type);
	if (it != Value::value_map().end())
		etype = it->second;

	switch (etype)
	{
	case Value::vt_pReferenced:
		{
            Referenced* v = nullptr;
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
			return addValue(name, s2ws(value));
		}
	case Value::vt_Point:
		{
			Point v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Vector:
		{
			Vector v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_BBox:
		{
			BBox v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_HSVColor:
		{
			HSVColor v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Color:
		{
			Color v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Plane:
		{
			Plane v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_PlaneSet:
		{
			PlaneSet v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Quaternion:
		{
			Quaternion v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Ray:
		{
			Ray v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Transform:
		{
			Transform v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Vector4f:
		{
			Vector4f v;
			iss >> v;
			return addValue(name, v);
		}
	case Value::vt_Vector4i:
		{
			Vector4i v;
			iss >> v;
			return addValue(name, v);
		}
	}
	return false;
}

bool ValueSet::addRvalu(const std::string &name, Referenced* value)
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

bool ValueSet::addValue(const std::string &name, const Point &value)
{
	if (!findValue(name))
	{
		TemplateValue<Point>* val = new TemplateValue<Point>(
			name, "Point", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Vector &value)
{
	if (!findValue(name))
	{
		TemplateValue<Vector>* val = new TemplateValue<Vector>(
			name, "Vector", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const BBox &value)
{
	if (!findValue(name))
	{
		TemplateValue<BBox>* val = new TemplateValue<BBox>(
			name, "BBox", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const HSVColor &value)
{
	if (!findValue(name))
	{
		TemplateValue<HSVColor>* val = new TemplateValue<HSVColor>(
			name, "HSVColor", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Color &value)
{
	if (!findValue(name))
	{
		TemplateValue<Color>* val = new TemplateValue<Color>(
			name, "Color", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Plane &value)
{
	if (!findValue(name))
	{
		TemplateValue<Plane>* val = new TemplateValue<Plane>(
			name, "Plane", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const PlaneSet &value)
{
	if (!findValue(name))
	{
		TemplateValue<PlaneSet>* val = new TemplateValue<PlaneSet>(
			name, "PlaneSet", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Quaternion &value)
{
	if (!findValue(name))
	{
		TemplateValue<Quaternion>* val = new TemplateValue<Quaternion>(
			name, "Quaternion", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Ray &value)
{
	if (!findValue(name))
	{
		TemplateValue<Ray>* val = new TemplateValue<Ray>(
			name, "Ray", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Transform &value)
{
	if (!findValue(name))
	{
		TemplateValue<Transform>* val = new TemplateValue<Transform>(
			name, "Transform", value);

		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Vector4f &value)
{
	if (!findValue(name))
	{
		TemplateValue<Vector4f>* val = new TemplateValue<Vector4f>(
			name, "Vector4f", value);
		_values.insert(std::pair<std::string, ref_ptr<Value>>(name, val));
		return true;
	}
	else
		return false;
}

bool ValueSet::addValue(const std::string &name, const Vector4i &value)
{
	if (!findValue(name))
	{
		TemplateValue<Vector4i>* val = new TemplateValue<Vector4i>(
			name, "Vector4i", value);
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
	auto it = Value::value_map().find(type);
	if (it != Value::value_map().end())
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
			return setValue(name, s2ws(value), event);
		}
	case Value::vt_Point:
		{
			Point v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Vector:
		{
			Vector v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_BBox:
		{
			BBox v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_HSVColor:
		{
			HSVColor v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Color:
		{
			Color v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Plane:
		{
			Plane v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_PlaneSet:
		{
			PlaneSet v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Quaternion:
		{
			Quaternion v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Ray:
		{
			Ray v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Transform:
		{
			Transform v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Vector4f:
		{
			Vector4f v;
			iss >> v;
			return setValue(name, v, event);
		}
	case Value::vt_Vector4i:
		{
			Vector4i v;
			iss >> v;
			return setValue(name, v, event);
		}
	}
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
				if (getRvalu(name, &v))
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
					oss << ws2s(v);
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Point:
			{
				std::get<1>(vt) = type;
				Point v;
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
				Vector v;
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
				BBox v;
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
				HSVColor v;
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
				Color v;
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
				Plane v;
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
				PlaneSet v;
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
				Quaternion v;
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
				Ray v;
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
				Transform v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Vector4f:
			{
				std::get<1>(vt) = type;
				Vector4f v;
				if (getValue(name, v))
				{
					oss << v;
					std::get<2>(vt) = oss.str();
					return true;
				}
			}
		case Value::vt_Vector4i:
			{
				std::get<1>(vt) = type;
				Vector4i v;
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
