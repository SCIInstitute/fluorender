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

namespace FL
{
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

	void setValue(const T& value, bool notify=true)
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

	ValueSet* clone() const {return new ValueSet(*this);}

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
	bool addValue(const std::string &name, Referenced* value=0);
	bool addValue(const std::string &name, bool value=false);
	bool addValue(const std::string &name, char value=0);
	bool addValue(const std::string &name, unsigned char value=0);
	bool addValue(const std::string &name, short value=0);
	bool addValue(const std::string &name, unsigned short value=0);
	bool addValue(const std::string &name, long value=0);
	bool addValue(const std::string &name, unsigned long value=0);
	bool addValue(const std::string &name, long long value=0);
	bool addValue(const std::string &name, unsigned long long value=0);
	bool addValue(const std::string &name, float=0.0f);
	bool addValue(const std::string &name, double=0.0);
	bool addValue(const std::string &name, const std::string &value="");
	//bool addValue(const std::string &name, const Vec2f &value=Vec2f());
	//bool addValue(const std::string &name, const Vec3f &value=Vec3f());
	//bool addValue(const std::string &name, const Vec4f &value=Vec4f());
	//bool addValue(const std::string &name, const Vec2d &value=Vec2d());
	//bool addValue(const std::string &name, const Vec3d &value=Vec3d());
	//bool addValue(const std::string &name, const Vec4d &value=Vec4d());
	//bool addValue(const std::string &name, const Quat &value=Quat());
	//bool addValue(const std::string &name, const Planef &value=Planef());
	//bool addValue(const std::string &name, const Planed &value=Planed());
	//bool addValue(const std::string &name, const Matrixf &value=Matrixf());
	//bool addValue(const std::string &name, const Matrixd &vlaue=Matrixd());
	//bool addValue(const std::string &name, const Color3f &value=Color3f());
	//bool addValue(const std::string &name, const Color3d &value=Color3d());
	//bool addValue(const std::string &name, const HsvColor3f &value=HsvColor3f());
	//bool addValue(const std::string &name, const HsvColor3d &value=HsvColor3d());

	/** All the set value functions */
	bool setValue(const std::string &name, Referenced* value);
	bool setValue(const std::string &name, bool value);
	bool setValue(const std::string &name, char value);
	bool setValue(const std::string &name, unsigned char value);
	bool setValue(const std::string &name, short value);
	bool setValue(const std::string &name, unsigned short value);
	bool setValue(const std::string &name, long value);
	bool setValue(const std::string &name, unsigned long value);
	bool setValue(const std::string &name, long long value);
	bool setValue(const std::string &name, unsigned long long value);
	bool setValue(const std::string &name, float value);
	bool setValue(const std::string &name, double value);
	bool setValue(const std::string &name, const std::string &value);
	//bool setValue(const std::string &name, const Vec2f &value);
	//bool setValue(const std::string &name, const Vec3f &value);
	//bool setValue(const std::string &name, const Vec4f &value);
	//bool setValue(const std::string &name, const Vec2d &value);
	//bool setValue(const std::string &name, const Vec3d &value);
	//bool setValue(const std::string &name, const Vec4d &value);
	//bool setValue(const std::string &name, const Quat &value);
	//bool setValue(const std::string &name, const Planef &value);
	//bool setValue(const std::string &name, const Planed &value);
	//bool setValue(const std::string &name, const Matrixf &value);
	//bool setValue(const std::string &name, const Matrixd &value);
	//bool setValue(const std::string &name, const Color3f &value);
	//bool setValue(const std::string &name, const Color3d &value);
	//bool setValue(const std::string &name, const HsvColor3f &value);
	//bool setValue(const std::string &name, const HsvColor3d &value);

	/** All the get value functions */
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
	//bool getValue(const std::string &name, Vec2f &value);
	//bool getValue(const std::string &name, Vec3f &value);
	//bool getValue(const std::string &name, Vec4f &value);
	//bool getValue(const std::string &name, Vec2d &value);
	//bool getValue(const std::string &name, Vec3d &value);
	//bool getValue(const std::string &name, Vec4d &value);
	//bool getValue(const std::string &name, Quat &value);
	//bool getValue(const std::string &name, Planef &value);
	//bool getValue(const std::string &name, Planed &value);
	//bool getValue(const std::string &name, Matrixf &value);
	//bool getValue(const std::string &name, Matrixd &value);
	//bool getValue(const std::string &name, Color3f &value);
	//bool getValue(const std::string &name, Color3d &value);
	//bool getValue(const std::string &name, HsvColor3f &value);
	//bool getValue(const std::string &name, HsvColor3d &value);

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
	//else if (_type == "Vec2f")
	//{
	//	return dynamic_cast<const TemplateValue<Vec2f>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Vec2f>*>(&v)->getValue();
	//}
	//else if (_type == "Vec3f")
	//{
	//	return dynamic_cast<const TemplateValue<Vec3f>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Vec3f>*>(&v)->getValue();
	//}
	//else if (_type == "Vec4f")
	//{
	//	return dynamic_cast<const TemplateValue<Vec4f>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Vec4f>*>(&v)->getValue();
	//}
	//else if (_type == "Vec2d")
	//{
	//	return dynamic_cast<const TemplateValue<Vec2d>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Vec2d>*>(&v)->getValue();
	//}
	//else if (_type == "Vec3d")
	//{
	//	return dynamic_cast<const TemplateValue<Vec3d>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Vec3d>*>(&v)->getValue();
	//}
	//else if (_type == "Vec4d")
	//{
	//	return dynamic_cast<const TemplateValue<Vec4d>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Vec4d>*>(&v)->getValue();
	//}
	//else if (_type == "Quat")
	//{
	//	return dynamic_cast<const TemplateValue<Quat>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Quat>*>(&v)->getValue();
	//}
	//else if (_type == "Planef")
	//{
	//	return dynamic_cast<const TemplateValue<Planef>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Planef>*>(&v)->getValue();
	//}
	//else if (_type == "Planed")
	//{
	//	return dynamic_cast<const TemplateValue<Planed>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Planed>*>(&v)->getValue();
	//}
	//else if (_type == "Matrixf")
	//{
	//	return dynamic_cast<const TemplateValue<Matrixf>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Matrixf>*>(&v)->getValue();
	//}
	//else if (_type == "Matrixd")
	//{
	//	return dynamic_cast<const TemplateValue<Matrixd>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Matrixd>*>(&v)->getValue();
	//}
	//else if (_type == "Color3f")
	//{
	//	return dynamic_cast<const TemplateValue<Color3f>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Color3f>*>(&v)->getValue();
	//}
	//else if (_type == "Color3d")
	//{
	//	return dynamic_cast<const TemplateValue<Color3d>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<Color3d>*>(&v)->getValue();
	//}
	//else if (_type == "HsvColor3f")
	//{
	//	return dynamic_cast<const TemplateValue<HsvColor3f>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<HsvColor3f>*>(&v)->getValue();
	//}
	//else if (_type == "HsvColor3d")
	//{
	//	return dynamic_cast<const TemplateValue<HsvColor3d>*>(this)->getValue() ==
	//		dynamic_cast<const TemplateValue<HsvColor3d>*>(&v)->getValue();
	//}
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
			dynamic_cast<TemplateValue<Referenced*>*>(value)->getValue());
	}
	else if (_type == "bool")
	{
		dynamic_cast<TemplateValue<bool>*>(this)->setValue(
			dynamic_cast<TemplateValue<bool>*>(value)->getValue());
	}
	else if (_type == "char")
	{
		dynamic_cast<TemplateValue<char>*>(this)->setValue(
			dynamic_cast<TemplateValue<char>*>(value)->getValue());
	}
	else if (_type == "unsigned char")
	{
		dynamic_cast<TemplateValue<unsigned char>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned char>*>(value)->getValue());
	}
	else if (_type == "short")
	{
		dynamic_cast<TemplateValue<short>*>(this)->setValue(
			dynamic_cast<TemplateValue<short>*>(value)->getValue());
	}
	else if (_type == "unsigned short")
	{
		dynamic_cast<TemplateValue<unsigned short>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned short>*>(value)->getValue());
	}
	else if (_type == "long")
	{
		dynamic_cast<TemplateValue<long>*>(this)->setValue(
			dynamic_cast<TemplateValue<long>*>(value)->getValue());
	}
	else if (_type == "unsigned long")
	{
		dynamic_cast<TemplateValue<unsigned long>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned long>*>(value)->getValue());
	}
	else if (_type == "long long")
	{
		dynamic_cast<TemplateValue<long long>*>(this)->setValue(
			dynamic_cast<TemplateValue<long long>*>(value)->getValue());
	}
	else if (_type == "unsigned long long")
	{
		dynamic_cast<TemplateValue<unsigned long long>*>(this)->setValue(
			dynamic_cast<TemplateValue<unsigned long long>*>(value)->getValue());
	}
	else if (_type == "float")
	{
		dynamic_cast<TemplateValue<float>*>(this)->setValue(
			dynamic_cast<TemplateValue<float>*>(value)->getValue());
	}
	else if (_type == "double")
	{
		dynamic_cast<TemplateValue<double>*>(this)->setValue(
			dynamic_cast<TemplateValue<double>*>(value)->getValue());
	}
	else if (_type == "string")
	{
		dynamic_cast<TemplateValue<std::string>*>(this)->setValue(
			dynamic_cast<TemplateValue<std::string>*>(value)->getValue());
	}
	//else if (_type == "Vec2f")
	//{
	//	dynamic_cast<TemplateValue<Vec2f>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Vec2f>*>(value)->getValue());
	//}
	//else if (_type == "Vec3f")
	//{
	//	dynamic_cast<TemplateValue<Vec3f>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Vec3f>*>(value)->getValue());
	//}
	//else if (_type == "Vec4f")
	//{
	//	dynamic_cast<TemplateValue<Vec4f>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Vec4f>*>(value)->getValue());
	//}
	//else if (_type == "Vec2d")
	//{
	//	dynamic_cast<TemplateValue<Vec2d>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Vec2d>*>(value)->getValue());
	//}
	//else if (_type == "Vec3d")
	//{
	//	dynamic_cast<TemplateValue<Vec3d>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Vec3d>*>(value)->getValue());
	//}
	//else if (_type == "Vec4d")
	//{
	//	dynamic_cast<TemplateValue<Vec4d>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Vec4d>*>(value)->getValue());
	//}
	//else if (_type == "Quat")
	//{
	//	dynamic_cast<TemplateValue<Quat>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Quat>*>(value)->getValue());
	//}
	//else if (_type == "Planef")
	//{
	//	dynamic_cast<TemplateValue<Planef>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Planef>*>(value)->getValue());
	//}
	//else if (_type == "Planed")
	//{
	//	dynamic_cast<TemplateValue<Planed>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Planed>*>(value)->getValue());
	//}
	//else if (_type == "Matrixf")
	//{
	//	dynamic_cast<TemplateValue<Matrixf>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Matrixf>*>(value)->getValue());
	//}
	//else if (_type == "Matrixd")
	//{
	//	dynamic_cast<TemplateValue<Matrixd>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Matrixd>*>(value)->getValue());
	//}
	//else if (_type == "Color3f")
	//{
	//	dynamic_cast<TemplateValue<Color3f>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Color3f>*>(value)->getValue());
	//}
	//else if (_type == "Color3d")
	//{
	//	dynamic_cast<TemplateValue<Color3d>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<Color3d>*>(value)->getValue());
	//}
	//else if (_type == "HsvColor3f")
	//{
	//	dynamic_cast<TemplateValue<HsvColor3f>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<HsvColor3f>*>(value)->getValue());
	//}
	//else if (_type == "HsvColor3d")
	//{
	//	dynamic_cast<TemplateValue<HsvColor3d>*>(this)->setValue(
	//		dynamic_cast<TemplateValue<HsvColor3d>*>(value)->getValue());
	//}
	else return false;

	return true;
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
