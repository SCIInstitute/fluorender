/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

namespace FL
{
typedef std::stack<ref_ptr<ValueSet> > ValueSetStack;

class Object : public Referenced, public Observer
{
public:

	Object();

	Object(const Object& obj, const CopyOp& copyop = CopyOp::SHALLOW_COPY);

	virtual Object* clone(const CopyOp&) const { return new Object(*this); };

	virtual bool isSameKindAs(const Object*) const {return true;}

	virtual const char* className() const { return "Object"; }

	inline void setId( const unsigned int id ) { _id = id; }

	inline const unsigned int getId() const { return _id; }

	virtual void objectDeleted(void*);
	virtual void objectChanged(void*, const std::string &exp);

	//add a value
	virtual bool addValue(const std::string &name, Referenced* value);
	virtual bool addValue(const std::string &name, bool value);
	virtual bool addValue(const std::string &name, char value);
	virtual bool addValue(const std::string &name, unsigned char value);
	virtual bool addValue(const std::string &name, short value);
	virtual bool addValue(const std::string &name, unsigned short value);
	virtual bool addValue(const std::string &name, long value);
	virtual bool addValue(const std::string &name, unsigned long value);
	virtual bool addValue(const std::string &name, long long value);
	virtual bool addValue(const std::string &name, unsigned long long value);
	virtual bool addValue(const std::string &name, float);
	virtual bool addValue(const std::string &name, double);
	virtual bool addValue(const std::string &name, const std::string &value);
	//virtual bool addValue(const std::string &name, const Vec2f &value);
	//virtual bool addValue(const std::string &name, const Vec3f &value);
	//virtual bool addValue(const std::string &name, const Vec4f &value);
	//virtual bool addValue(const std::string &name, const Vec2d &value);
	//virtual bool addValue(const std::string &name, const Vec3d &value);
	//virtual bool addValue(const std::string &name, const Vec4d &value);
	//virtual bool addValue(const std::string &name, const Quat &value);
	//virtual bool addValue(const std::string &name, const Planef &value);
	//virtual bool addValue(const std::string &name, const Planed &value);
	//virtual bool addValue(const std::string &name, const Matrixf &value);
	//virtual bool addValue(const std::string &name, const Matrixd &value);
	//virtual bool addValue(const std::string &name, const Color3f &value);
	//virtual bool addValue(const std::string &name, const Color3d &value);
	//virtual bool addValue(const std::string &name, const HsvColor3f &value);
	//virtual bool addValue(const std::string &name, const HsvColor3d &value);

	/** All the set value functions */
	virtual bool setValue(const std::string &name, Referenced* value);
	virtual bool setValue(const std::string &name, bool value);
	virtual bool setValue(const std::string &name, char value);
	virtual bool setValue(const std::string &name, unsigned char value);
	virtual bool setValue(const std::string &name, short value);
	virtual bool setValue(const std::string &name, unsigned short value);
	virtual bool setValue(const std::string &name, long value);
	virtual bool setValue(const std::string &name, unsigned long value);
	virtual bool setValue(const std::string &name, long long value);
	virtual bool setValue(const std::string &name, unsigned long long value);
	virtual bool setValue(const std::string &name, float value);
	virtual bool setValue(const std::string &name, double value);
	virtual bool setValue(const std::string &name, const std::string &value);
	//virtual bool setValue(const std::string &name, const Vec2f &value);
	//virtual bool setValue(const std::string &name, const Vec3f &value);
	//virtual bool setValue(const std::string &name, const Vec4f &value);
	//virtual bool setValue(const std::string &name, const Vec2d &value);
	//virtual bool setValue(const std::string &name, const Vec3d &value);
	//virtual bool setValue(const std::string &name, const Vec4d &value);
	//virtual bool setValue(const std::string &name, const Quat &value);
	//virtual bool setValue(const std::string &name, const Planef &value);
	//virtual bool setValue(const std::string &name, const Planed &value);
	//virtual bool setValue(const std::string &name, const Matrixf &value);
	//virtual bool setValue(const std::string &name, const Matrixd &value);
	//virtual bool setValue(const std::string &name, const Color3f &value);
	//virtual bool setValue(const std::string &name, const Color3d &value);
	//virtual bool setValue(const std::string &name, const HsvColor3f &value);
	//virtual bool setValue(const std::string &name, const HsvColor3d &value);

	/** All the get value functions */
	virtual bool getValue(const std::string &name, Referenced** value);
	virtual bool getValue(const std::string &name, bool &value);
	virtual bool getValue(const std::string &name, char &value);
	virtual bool getValue(const std::string &name, unsigned char &value);
	virtual bool getValue(const std::string &name, short &value);
	virtual bool getValue(const std::string &name, unsigned short &value);
	virtual bool getValue(const std::string &name, long &value);
	virtual bool getValue(const std::string &name, unsigned long &value);
	virtual bool getValue(const std::string &name, long long &value);
	virtual bool getValue(const std::string &name, unsigned long long &value);
	virtual bool getValue(const std::string &name, float &value);
	virtual bool getValue(const std::string &name, double &value);
	virtual bool getValue(const std::string &name, std::string &value);
	//virtual bool getValue(const std::string &name, Vec2f &value);
	//virtual bool getValue(const std::string &name, Vec3f &value);
	//virtual bool getValue(const std::string &name, Vec4f &value);
	//virtual bool getValue(const std::string &name, Vec2d &value);
	//virtual bool getValue(const std::string &name, Vec3d &value);
	//virtual bool getValue(const std::string &name, Vec4d &value);
	//virtual bool getValue(const std::string &name, Quat &value);
	//virtual bool getValue(const std::string &name, Planef &value);
	//virtual bool getValue(const std::string &name, Planed &value);
	//virtual bool getValue(const std::string &name, Matrixf &value);
	//virtual bool getValue(const std::string &name, Matrixd &value);
	//virtual bool getValue(const std::string &name, Color3f &value);
	//virtual bool getValue(const std::string &name, Color3d &value);
	//virtual bool getValue(const std::string &name, HsvColor3f &value);
	//virtual bool getValue(const std::string &name, HsvColor3d &value);

	//sync value
	virtual bool syncValue(const std::string &name, Observer* obsrvr);
	virtual bool unsyncValue(const std::string &name, Observer* obsrvr);

protected:
	virtual ~Object();

	/** ID of an object is non-zero. */
	unsigned int _id;

	/** a stack of value sets */
	ValueSetStack _vs_stack;

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
	T* newObject = osg::clone(t, copyop);
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
