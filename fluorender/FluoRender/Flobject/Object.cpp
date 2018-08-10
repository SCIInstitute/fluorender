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
#include <Flobject/Object.h>
//#include "Value.h"

using namespace FL;

Object::Object():
  Referenced(),
  _id(0)
{
	//ValueSet* value_set = new ValueSet();
	//_vs_stack.push(value_set);
}

Object::Object(const Object& obj, const CopyOp& copyop):
  Referenced(),
  _id(0)
{
	//if (ref_copy)
	//	_vs_stack.push(obj._vs_stack.top());
	//else
	//	_vs_stack.push(obj._vs_stack.top()->clone());
}

Object::~Object()
{
}

//observer functions
void Object::objectDeleted(void* ptr)
{
	Referenced* refd = static_cast<Referenced*>(ptr);
	//if (refd->className() == std::string("Object"))
	//	_vs_stack.top()->resetRefPtr(refd);

	//remove observee
	removeObservee(refd);
}

void Object::objectChanged(void* ptr, const std::string &exp)
{
	Referenced* refd = static_cast<Referenced*>(ptr);
	//if (refd->className() == std::string("Value"))
	//	_vs_stack.top()->syncValue(dynamic_cast<Value*>(refd));
}

//sync value
bool Object::syncValue(const std::string &name, Observer* obsrvr)
{
	//if (_vs_stack.top())
	//{
	//	Value* value = _vs_stack.top()->findValue(name);
	//	if (value)
	//	{
	//		value->addObserver(obsrvr);
	//		return true;
	//	}
	//}
	return false;
}

//unsync value
bool Object::unsyncValue(const std::string &name, Observer* obsrvr)
{
	//if (_vs_stack.top())
	//{
	//	Value* value = _vs_stack.top()->findValue(name);
	//	if (value)
	//	{
	//		value->removeObserver(obsrvr);
	//		return true;
	//	}
	//}
	return false;
}
