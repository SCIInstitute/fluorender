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

#include <Flobject/Referenced.h>
#include <Flobject/ref_ptr.h>
#include <Flobject/Observer.h>
#include <Flobject/CopyOp.h>
#include <Flobject/Value.h>
#include <stack>
#include <string>
#include <vector>

namespace FL
{
typedef std::stack<ref_ptr<ValueSet> > ValueSetStack;
class Object;
typedef std::vector<Object*> ObjectList;
class ObjectFactory;

class Object : public Referenced, public Observer
{
public:

	Object();

	Object(const Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY);

	virtual Object* clone(const CopyOp& copyop) const { return new Object(*this, copyop); };

	virtual bool isSameKindAs(const Object*) const {return true;}

	virtual const char* className() const { return "Object"; }

	inline const unsigned int getId() const { return _id; }

	inline void setName(const std::string& name) { m_name = name; }

	inline const char* getName() const { return m_name.c_str(); }

	virtual void objectDeleted(void*);
	virtual void objectChanged(void*, const std::string &exp);

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

	/** All the set value functions */
	bool setValue(ValueTuple& vt, bool notify = true);
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
	//FluoRender's special types
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

	//toggle value for bool
	bool toggleValue(const std::string &name, bool &value, bool notify = true);

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

	//get value the class
	Value* getValue(const std::string &name)
	{
		if (_vs_stack.top())
			return _vs_stack.top()->findValue(name);
		else
			return 0;
	}

	//sync value
	//observer's value updates when this updates
	bool syncValue(const std::string &name, Observer* obsrvr);
	bool unsyncValue(const std::string &name, Observer* obsrvr);
	bool syncValues(const std::vector<std::string> &names, Observer* obsrvr);
	bool unsyncValues(const std::vector<std::string> &names, Observer* obsrvr);
	bool syncAllValues(Observer* obsrvr);
	bool unsyncAllValues(Observer* obsrvr);
	//propagate value
	bool propValue(const std::string &name, Object* obj);
	bool propValues(const std::vector<std::string> &names, Object* obj);
	bool propAllValues(Object* obj);

	std::vector<std::string> getValueNames();
	std::string getValueType(const std::string &name)
	{
		if (_vs_stack.top())
		{
			Value* value = _vs_stack.top()->findValue(name);
			if (value)
				return value->getType();
		}
		return "";
	}

protected:
	virtual ~Object();

	inline void setId(const unsigned int id) { _id = id; }

	/** ID of an object is non-zero. */
	unsigned int _id;
	// object name
	std::string m_name;

	/** a stack of value sets */
	ValueSetStack _vs_stack;

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
