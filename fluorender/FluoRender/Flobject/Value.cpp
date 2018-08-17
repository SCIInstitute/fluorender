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

using namespace FL;

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
	//else if (_type == "Vec2f")
	//	return (dynamic_cast<TemplateValue<Vec2f>*>(this))->clone();
	//else if (_type == "Vec3f")
	//	return (dynamic_cast<TemplateValue<Vec3f>*>(this))->clone();
	//else if (_type == "Vec4f")
	//	return (dynamic_cast<TemplateValue<Vec4f>*>(this))->clone();
	//else if (_type == "Vec2d")
	//	return (dynamic_cast<TemplateValue<Vec2d>*>(this))->clone();
	//else if (_type == "Vec3d")
	//	return (dynamic_cast<TemplateValue<Vec3d>*>(this))->clone();
	//else if (_type == "Vec4d")
	//	return (dynamic_cast<TemplateValue<Vec4d>*>(this))->clone();
	//else if (_type == "Quat")
	//	return (dynamic_cast<TemplateValue<Quat>*>(this))->clone();
	//else if (_type == "Planef")
	//	return (dynamic_cast<TemplateValue<Planef>*>(this))->clone();
	//else if (_type == "Planed")
	//	return (dynamic_cast<TemplateValue<Planed>*>(this))->clone();
	//else if (_type == "Matrixf")
	//	return (dynamic_cast<TemplateValue<Matrixf>*>(this))->clone();
	//else if (_type == "Matrixd")
	//	return (dynamic_cast<TemplateValue<Matrixd>*>(this))->clone();
	//else if (_type == "Color3f")
	//	return (dynamic_cast<TemplateValue<Color3f>*>(this))->clone();
	//else if (_type == "Color3d")
	//	return (dynamic_cast<TemplateValue<Color3d>*>(this))->clone();
	//else if (_type == "HsvColor3f")
	//	return (dynamic_cast<TemplateValue<HsvColor3f>*>(this))->clone();
	//else if (_type == "HsvColor3d")
	//	return (dynamic_cast<TemplateValue<HsvColor3d>*>(this))->clone();
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
			(dynamic_cast<TemplateValue<Referenced*>*>(it->second.get()))->setValue(0);
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

//bool ValueSet::addValue(const std::string &name, const Vec2f &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Vec2f>* val = new TemplateValue<Vec2f>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Vec2f";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Vec3f &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Vec3f>* val = new TemplateValue<Vec3f>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Vec3f";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Vec4f &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Vec4f>* val = new TemplateValue<Vec4f>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Vec4f";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Vec2d &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Vec2d>* val = new TemplateValue<Vec2d>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Vec2d";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Vec3d &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Vec3d>* val = new TemplateValue<Vec3d>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Vec3d";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Vec4d &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Vec4d>* val = new TemplateValue<Vec4d>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Vec4d";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Quat &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Quat>* val = new TemplateValue<Quat>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Quat";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Planef &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Planef>* val = new TemplateValue<Planef>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Planef";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Planed &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Planed>* val = new TemplateValue<Planed>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Planed";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Matrixf &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Matrixf>* val = new TemplateValue<Matrixf>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Matrixf";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Matrixd &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Matrixd>* val = new TemplateValue<Matrixd>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Matrixd";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Color3f &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Color3f>* val = new TemplateValue<Color3f>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Color3f";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const Color3d &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<Color3d>* val = new TemplateValue<Color3d>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "Color3d";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const HsvColor3f &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<HsvColor3f>* val = new TemplateValue<HsvColor3f>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "HsvColor3f";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::addValue(const std::string &name, const HsvColor3d &value)
//{
//	if (!findValue(name))
//	{
//		TemplateValue<HsvColor3d>* val = new TemplateValue<HsvColor3d>;
//		val->_value =value;
//		val->_name = name;
//		val->_type = "HsvColor3d";
//
//		_values.push_back(val);
//		return true;
//	}
//	else
//		return false;
//}

//set value functions
bool ValueSet::setValue(const std::string &name, Referenced* value)
{
	Value* val = findValue(name);
	if (val && val->_type=="Referenced*")
	{
		(dynamic_cast<TemplateValue<Referenced*>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, bool value)
{
	Value* val = findValue(name);
	if (val && val->_type=="bool")
	{
		(dynamic_cast<TemplateValue<bool>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, char value)
{
	Value* val = findValue(name);
	if (val && val->_type=="char")
	{
		(dynamic_cast<TemplateValue<char>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned char value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned char")
	{
		(dynamic_cast<TemplateValue<unsigned char>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, short value)
{
	Value* val = findValue(name);
	if (val && val->_type=="short")
	{
		(dynamic_cast<TemplateValue<short>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned short value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned short")
	{
		(dynamic_cast<TemplateValue<unsigned short>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, long value)
{
	Value* val = findValue(name);
	if (val && val->_type=="long")
	{
		(dynamic_cast<TemplateValue<long>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned long value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned long")
	{
		(dynamic_cast<TemplateValue<unsigned long>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, long long value)
{
	Value* val = findValue(name);
	if (val && val->_type=="long long")
	{
		(dynamic_cast<TemplateValue<long long>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, unsigned long long value)
{
	Value* val = findValue(name);
	if (val && val->_type=="unsigned long long")
	{
		(dynamic_cast<TemplateValue<unsigned long long>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, float value)
{
	Value* val = findValue(name);
	if (val && val->_type=="float")
	{
		(dynamic_cast<TemplateValue<float>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, double value)
{
	Value* val = findValue(name);
	if (val && val->_type=="double")
	{
		(dynamic_cast<TemplateValue<double>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const std::string &value)
{
	Value* val = findValue(name);
	if (val && val->_type=="string")
	{
		(dynamic_cast<TemplateValue<std::string>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

bool ValueSet::setValue(const std::string &name, const std::wstring &value)
{
	Value* val = findValue(name);
	if (val && val->_type == "wstring")
	{
		(dynamic_cast<TemplateValue<std::wstring>*>(val))->setValue(value);
		return true;
	}
	else
		return false;
}

//bool ValueSet::setValue(const std::string &name, const Vec2f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec2f")
//	{
//		(dynamic_cast<TemplateValue<Vec2f>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Vec3f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec3f")
//	{
//		(dynamic_cast<TemplateValue<Vec3f>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Vec4f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec4f")
//	{
//		(dynamic_cast<TemplateValue<Vec4f>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Vec2d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec2d")
//	{
//		(dynamic_cast<TemplateValue<Vec2d>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Vec3d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec3d")
//	{
//		(dynamic_cast<TemplateValue<Vec3d>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Vec4d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec4d")
//	{
//		(dynamic_cast<TemplateValue<Vec4d>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Quat &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Quat")
//	{
//		(dynamic_cast<TemplateValue<Quat>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Planef &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Planef")
//	{
//		(dynamic_cast<TemplateValue<Planef>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Planed &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Planed")
//	{
//		(dynamic_cast<TemplateValue<Planed>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Matrixf &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Matrixf")
//	{
//		(dynamic_cast<TemplateValue<Matrixf>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Matrixd &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Matrixd")
//	{
//		(dynamic_cast<TemplateValue<Matrixd>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Color3f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Color3f")
//	{
//		(dynamic_cast<TemplateValue<Color3f>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const Color3d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Color3d")
//	{
//		(dynamic_cast<TemplateValue<Color3d>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const HsvColor3f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="HsvColor3f")
//	{
//		(dynamic_cast<TemplateValue<HsvColor3f>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::setValue(const std::string &name, const HsvColor3d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="HsvColor3d")
//	{
//		(dynamic_cast<TemplateValue<HsvColor3d>*>(val))->setValue(value);
//		return true;
//	}
//	else
//		return false;
//}

//get value functions
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

//bool ValueSet::getValue(const std::string &name, Vec2f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec2f")
//	{
//		value = (dynamic_cast<TemplateValue<Vec2f>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Vec3f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec3f")
//	{
//		value = (dynamic_cast<TemplateValue<Vec3f>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Vec4f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec4f")
//	{
//		value = (dynamic_cast<TemplateValue<Vec4f>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Vec2d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec2d")
//	{
//		value = (dynamic_cast<TemplateValue<Vec2d>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Vec3d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec3d")
//	{
//		value = (dynamic_cast<TemplateValue<Vec3d>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Vec4d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Vec4d")
//	{
//		value = (dynamic_cast<TemplateValue<Vec4d>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Quat &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Quat")
//	{
//		value = (dynamic_cast<TemplateValue<Quat>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Planef &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Planef")
//	{
//		value = (dynamic_cast<TemplateValue<Planef>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Planed &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Planed")
//	{
//		value = (dynamic_cast<TemplateValue<Planed>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Matrixf &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Matrixf")
//	{
//		value = (dynamic_cast<TemplateValue<Matrixf>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Matrixd &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Matrixd")
//	{
//		value = (dynamic_cast<TemplateValue<Matrixd>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Color3f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Color3f")
//	{
//		value = (dynamic_cast<TemplateValue<Color3f>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, Color3d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="Color3d")
//	{
//		value = (dynamic_cast<TemplateValue<Color3d>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, HsvColor3f &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="HsvColor3f")
//	{
//		value = (dynamic_cast<TemplateValue<HsvColor3f>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}
//
//bool ValueSet::getValue(const std::string &name, HsvColor3d &value)
//{
//	Value* val = findValue(name);
//	if (val && val->_type=="HsvColor3d")
//	{
//		value = (dynamic_cast<TemplateValue<HsvColor3d>*>(val))->getValue();
//		return true;
//	}
//	else
//		return false;
//}

