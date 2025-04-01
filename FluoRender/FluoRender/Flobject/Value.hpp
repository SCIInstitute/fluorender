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
#ifndef FL_VALUE_HPP
#define FL_VALUE_HPP

#include <Referenced.hpp>
#include <ref_ptr.hpp>
#include <CopyOp.hpp>
#include <Observer.hpp>
#include <Event.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <tuple>
#include <unordered_set>
//FluoRender's special types
#include <BBox.h>
#include <Color.h>
#include <Plane.h>
#include <Point.h>
#include <Quaternion.h>
#include <Ray.h>
#include <Transform.h>
#include <Vector.h>
#include <Vector4f.h>
#include <Vector4i.h>
#include <compatibility.h>

namespace fluo
{
	//name, type, value
	typedef std::tuple<std::string, std::string, std::string> ValueTuple;
	//only value names
	typedef std::unordered_set<std::string> ValueCollection;
	//sorted value names
	typedef std::vector<std::string> ValueVector;
	//parameters are used to customize a processor
	typedef std::vector<ValueTuple> ParameterList;

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
			vt_Vector4f,
			vt_Vector4i,
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
		bool isReferenced() { return _etype == vt_pReferenced; }
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

		//static bool _precise;//if equal is precise
		//static std::unordered_map<std::string, ValueType> _value_map;
		//use static functions instead of variables to make sure initialization
		//https://isocpp.org/wiki/faq/ctors#construct-on-first-use-v2
		static bool& precise()
		{
			static bool bval = false;
			return bval;
		}

		static std::unordered_map<std::string, ValueType>& value_map()
		{
			static std::unordered_map<std::string, ValueType> mval
			{
				{"Referenced*", vt_pReferenced},
				{"bool", vt_bool},
				{"char", vt_char},
				{"unsigned char", vt_unsigned_char},
				{"short", vt_short},
				{"unsigned short", vt_unsigned_short},
				{"long", vt_long},
				{"unsigned long", vt_unsigned_long},
				{"long long", vt_long_long},
				{"unsigned long long", vt_unsigned_long_long},
				{"float", vt_float},
				{"double", vt_double},
				{"string", vt_string},
				{"wstring", vt_wstring},
				{"Point", vt_Point},
				{"Vector", vt_Vector},
				{"BBox", vt_BBox},
				{"HSVColor", vt_HSVColor},
				{"Color", vt_Color},
				{"Plane", vt_Plane},
				{"PlaneSet", vt_PlaneSet},
				{"Quaternion", vt_Quaternion},
				{"Ray", vt_Ray},
				{"Transform", vt_Transform},
				{"Vector4f", vt_Vector4f},
				{"Vector4i", vt_Vector4i}
			};
			return mval;
		}

		friend class ValueSet;
	};

	template<typename T>
	class TemplateValue : public Value
	{
	public:

		TemplateValue(const std::string& name, const std::string& type, const T& value) :
			Value(name, type), _value(value)
		{
			if (value_map().empty())
			{
				_etype = vt_null;
				return;
			}
			auto it = value_map().find(type);
			if (it != value_map().end())
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
				//may need more work
				Event changing_event(event);
				changing_event.type = Event::EVENT_VALUE_CHANGING;
				notifyObservers(changing_event);
				_value = value;
				event.type = Event::EVENT_VALUE_CHANGED;
				notifyObservers(event);
			}
		}

		const T& getValue() const { return _value; }

		bool equal(const T& value) const
		{
			if (precise())
				return _value == value;
			else
			{
				//better not to create a new template value for comparison
				ref_ptr<TemplateValue<T>> pvalue(new TemplateValue<T>(_name, _type, value));
				return *this == *pvalue;
			}
		}

		T _value;

	protected:
		virtual ~TemplateValue() {}

	};

	typedef std::unordered_map<std::string, ref_ptr<Value>> Values;
	class ValueSet : public Referenced
	{
	public:

		ValueSet() : Referenced() {}

		ValueSet(const ValueSet& vs, const CopyOp& copyop = CopyOp::SHALLOW_COPY);

		ValueSet* clone(const CopyOp& copyop) const { return new ValueSet(*this, copyop); }

		virtual const char* className() const { return "ValueSet"; }

		inline bool operator == (const ValueSet& vs) const;
		inline bool operator != (const ValueSet& vs) const;

		void clear();

		Value* findValue(const std::string &name);
		bool containsValue(Value* value);
		bool addValue(Value* value);
		bool removeValue(Value* value);
		bool removeValue(const std::string &name);

		//add value functions
		bool addValue(ValueTuple&);
		//generic types
		bool addRvalu(const std::string &name, Referenced* value);
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
		bool addValue(const std::string &name, const Point &value);
		bool addValue(const std::string &name, const Vector &value);
		bool addValue(const std::string &name, const BBox &value);
		bool addValue(const std::string &name, const HSVColor &value);
		bool addValue(const std::string &name, const Color &value);
		bool addValue(const std::string &name, const Plane &value);
		bool addValue(const std::string &name, const PlaneSet &value);
		bool addValue(const std::string &name, const Quaternion &value);
		bool addValue(const std::string &name, const Ray &value);
		bool addValue(const std::string &name, const Transform &value);
		bool addValue(const std::string &name, const Vector4f &value);
		bool addValue(const std::string &name, const Vector4i &value);

		/** All the set value functions */
		bool setValue(ValueTuple& vt, Event& event);
		bool setRvalu(const std::string &name, Referenced* value, Event& event)
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
		template<typename T>
		bool setValue(const std::string& name, T value, Event& event)
		{
			Value *val = findValue(name);
			if (val)
			{
				(reinterpret_cast<TemplateValue<T>*>(val))->setValue(value, event);
				return true;
			}
			else
				return false;
		}

		//toggle value for bool, result in value
		bool toggleValue(const std::string &name, bool &value, Event& event);

		/** All the get value functions */
		bool getValue(ValueTuple&);
		bool getRvalu(const std::string &name, Referenced** value)
		{
			Value* val = findValue(name);
			if (val)
			{
				*value = const_cast<Referenced*>((dynamic_cast<TemplateValue<Referenced*>*>(val))->getValue());
				return true;
			}
			else
			{
				return false;
			}

		}
		template <typename V>
		bool getValue(const std::string &name, V &value)
		{
			Value* val = findValue(name);

			if (val)
			{
				value = (reinterpret_cast<TemplateValue<V>*>(val))->getValue();
				return true;
			}
			else
			{
				return false;
			}

		}

		Values& getValues() { return _values; }
		const Values& getValues() const { return _values; }

	protected:
		virtual ~ValueSet();

		Values _values;

		template<typename T>
		friend class TemplateValue;
	};

	typedef std::vector<ref_ptr<ValueSet>> ValueSetStack;

	//A value cache canbe seen as a group of values with
	//temporary saving/restoring capabilities, i.e., the value set stack
	class ValueCache : public Referenced
	{
		ValueCache() : Referenced() {}
		ValueCache(const ValueCollection &names) :
			Referenced(),
			names_(names)
		{}
		ValueCache(const std::string &name, const ValueCollection &names) :
			Referenced(),
			name_(name),
			names_(names)
		{}

		virtual const char* className() const { return "ValueCache"; }

		void setName(const std::string &name) { name_ = name; }

		std::string getName() const { return name_; }

		void setValueCollection(const ValueCollection &names) { names_ = names; }

		ValueCollection getValueCollection() const { return names_; }

		ValueSet* getValueSet() { if (vs_stack_.empty()) return nullptr; else return vs_stack_.back().get(); }

		inline void pushValueSet(ValueSet* vs = nullptr);

		inline void popValueSet();

	private:
		std::string name_;
		ValueCollection names_;
		ValueSetStack vs_stack_;
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
			if (precise())
				return dynamic_cast<const TemplateValue<float>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<float>*>(&v)->getValue();
			else
				return fabs(dynamic_cast<const TemplateValue<float>*>(this)->getValue() -
					dynamic_cast<const TemplateValue<float>*>(&v)->getValue()) < Epsilon();
		case vt_double:
			if (precise())
				return dynamic_cast<const TemplateValue<double>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<double>*>(&v)->getValue();
			else
				return fabs(reinterpret_cast<const TemplateValue<double>*>(this)->getValue() -
					reinterpret_cast<const TemplateValue<double>*>(&v)->getValue()) < Epsilon(10);
		case vt_string:
			return dynamic_cast<const TemplateValue<std::string>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<std::string>*>(&v)->getValue();
		case vt_wstring:
			return dynamic_cast<const TemplateValue<std::wstring>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<std::wstring>*>(&v)->getValue();
		case vt_Point:
			return dynamic_cast<const TemplateValue<Point>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Point>*>(&v)->getValue();
		case vt_Vector:
			return dynamic_cast<const TemplateValue<Vector>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Vector>*>(&v)->getValue();
		case vt_BBox:
			return dynamic_cast<const TemplateValue<BBox>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<BBox>*>(&v)->getValue();
		case vt_HSVColor:
			return dynamic_cast<const TemplateValue<HSVColor>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<HSVColor>*>(&v)->getValue();
		case vt_Color:
			return dynamic_cast<const TemplateValue<Color>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Color>*>(&v)->getValue();
		case vt_Plane:
			return dynamic_cast<const TemplateValue<Plane>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Plane>*>(&v)->getValue();
		case vt_PlaneSet:
			return dynamic_cast<const TemplateValue<PlaneSet>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<PlaneSet>*>(&v)->getValue();
		case vt_Quaternion:
			return dynamic_cast<const TemplateValue<Quaternion>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Quaternion>*>(&v)->getValue();
		case vt_Ray:
			return dynamic_cast<const TemplateValue<Ray>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Ray>*>(&v)->getValue();
		case vt_Transform:
			return dynamic_cast<const TemplateValue<Transform>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Transform>*>(&v)->getValue();
		case vt_Vector4f:
			return dynamic_cast<const TemplateValue<Vector4f>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Vector4f>*>(&v)->getValue();
		case vt_Vector4i:
			return dynamic_cast<const TemplateValue<Vector4i>*>(this)->getValue() ==
				dynamic_cast<const TemplateValue<Vector4i>*>(&v)->getValue();
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
		{
			Referenced* old_refd = dynamic_cast<TemplateValue<Referenced*>*>(this)->getValue();
			Referenced* refd = dynamic_cast<TemplateValue<Referenced*>*>(value)->getValue();
			if (refd != old_refd && old_refd)
				old_refd->removeObserver(this);
			dynamic_cast<TemplateValue<Referenced*>*>(this)->setValue(refd, event);
			if (refd != old_refd && refd)
				refd->addObserver(this);
		}
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
			dynamic_cast<TemplateValue<Point>*>(this)->setValue(
				dynamic_cast<TemplateValue<Point>*>(value)->getValue(), event);
			break;
		case vt_Vector:
			dynamic_cast<TemplateValue<Vector>*>(this)->setValue(
				dynamic_cast<TemplateValue<Vector>*>(value)->getValue(), event);
			break;
		case vt_BBox:
			dynamic_cast<TemplateValue<BBox>*>(this)->setValue(
				dynamic_cast<TemplateValue<BBox>*>(value)->getValue(), event);
			break;
		case vt_HSVColor:
			dynamic_cast<TemplateValue<HSVColor>*>(this)->setValue(
				dynamic_cast<TemplateValue<HSVColor>*>(value)->getValue(), event);
			break;
		case vt_Color:
			dynamic_cast<TemplateValue<Color>*>(this)->setValue(
				dynamic_cast<TemplateValue<Color>*>(value)->getValue(), event);
			break;
		case vt_Plane:
			dynamic_cast<TemplateValue<Plane>*>(this)->setValue(
				dynamic_cast<TemplateValue<Plane>*>(value)->getValue(), event);
			break;
		case vt_PlaneSet:
			dynamic_cast<TemplateValue<PlaneSet>*>(this)->setValue(
				dynamic_cast<TemplateValue<PlaneSet>*>(value)->getValue(), event);
			break;
		case vt_Quaternion:
			dynamic_cast<TemplateValue<Quaternion>*>(this)->setValue(
				dynamic_cast<TemplateValue<Quaternion>*>(value)->getValue(), event);
			break;
		case vt_Ray:
			dynamic_cast<TemplateValue<Ray>*>(this)->setValue(
				dynamic_cast<TemplateValue<Ray>*>(value)->getValue(), event);
			break;
		case vt_Transform:
			dynamic_cast<TemplateValue<Transform>*>(this)->setValue(
				dynamic_cast<TemplateValue<Transform>*>(value)->getValue(), event);
			break;
		case vt_Vector4f:
			dynamic_cast<TemplateValue<Vector4f>*>(this)->setValue(
				dynamic_cast<TemplateValue<Vector4f>*>(value)->getValue(), event);
			break;
		case vt_Vector4i:
			dynamic_cast<TemplateValue<Vector4i>*>(this)->setValue(
				dynamic_cast<TemplateValue<Vector4i>*>(value)->getValue(), event);
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

	inline std::ostream& operator<<(std::ostream& os, const Value& v)
	{
		os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
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
			os << ws2s(dynamic_cast<const TemplateValue<std::wstring>*>(&v)->getValue());
			break;
		case Value::vt_Point:
			os << dynamic_cast<const TemplateValue<Point>*>(&v)->getValue();
			break;
		case Value::vt_Vector:
			os << dynamic_cast<const TemplateValue<Vector>*>(&v)->getValue();
			break;
		case Value::vt_BBox:
			os << dynamic_cast<const TemplateValue<BBox>*>(&v)->getValue();
			break;
		case Value::vt_HSVColor:
			os << dynamic_cast<const TemplateValue<HSVColor>*>(&v)->getValue();
			break;
		case Value::vt_Color:
			os << dynamic_cast<const TemplateValue<Color>*>(&v)->getValue();
			break;
		case Value::vt_Plane:
			os << dynamic_cast<const TemplateValue<Plane>*>(&v)->getValue();
			break;
		case Value::vt_PlaneSet:
			os << dynamic_cast<const TemplateValue<PlaneSet>*>(&v)->getValue();
			break;
		case Value::vt_Quaternion:
			os << dynamic_cast<const TemplateValue<Quaternion>*>(&v)->getValue();
			break;
		case Value::vt_Ray:
			os << dynamic_cast<const TemplateValue<Ray>*>(&v)->getValue();
			break;
		case Value::vt_Transform:
			os << dynamic_cast<const TemplateValue<Transform>*>(&v)->getValue();
			break;
		case Value::vt_Vector4f:
			os << dynamic_cast<const TemplateValue<Vector4f>*>(&v)->getValue();
			break;
		case Value::vt_Vector4i:
			os << dynamic_cast<const TemplateValue<Vector4i>*>(&v)->getValue();
			break;
		}
		return os;

	}

	inline bool ValueSet::operator == (const ValueSet &vs) const
	{
		Values::iterator it1;
		Values::iterator it2;
		for (it1 = const_cast<Values*>(&_values)->begin(),
			it2 = const_cast<Values*>(&(vs._values))->begin();
			it1 != _values.end() &&
			it2 != vs._values.end();
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

	inline void ValueCache::pushValueSet(ValueSet* vs)
	{
		if (vs_stack_.empty())
		{
			if (!vs)
				return;
			ValueSet* vs_pushed = new ValueSet(*vs, CopyOp::DEEP_COPY_ALL);
			vs_stack_.push_back(vs_pushed);
		}
		else
		{
			bool copy_values = true;
			if (!vs)
			{
				copy_values = false;
				vs = getValueSet();
			}
			ValueSet* vs_pushed = new ValueSet(*(vs_stack_.back()), CopyOp::DEEP_COPY_ALL);
			vs_stack_.insert(vs_stack_.end() - 2, vs_pushed);
			//copy values
			for (auto it = vs->getValues().begin();
				it != vs->getValues().end(); ++it)
			{
				Event notifyNone(Event::NOTIFY_NONE); // this needed to be created before hand
				ValueTuple vt;
				std::get<0>(vt) = it->second->getName();
				vs->getValue(vt);
				vs_stack_.back()->setValue(vt, notifyNone);
			}
		}
	}

	inline void ValueCache::popValueSet()
	{
		if (vs_stack_.size() > 1)
		{
			//copy values
			ValueSet* vs = (*(vs_stack_.end() - 2)).get();
			for (auto it = vs->getValues().begin();
				it != vs->getValues().end(); ++it)
			{
				Event notifyAll; // this needed to be created before hand
				ValueTuple vt;
				std::get<0>(vt) = it->second->getName();
				vs->getValue(vt);
				vs_stack_.back()->setValue(vt, notifyAll);
			}
			//pop
			vs_stack_.erase(vs_stack_.end() - 2);
		}
		else if (vs_stack_.size() == 1)
			vs_stack_.pop_back();
	}
}

#endif
