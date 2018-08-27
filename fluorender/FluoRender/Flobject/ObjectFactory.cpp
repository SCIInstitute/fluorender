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

#include <Flobject/ObjectFactory.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace FL;

unsigned int ObjectFactory::global_id_ = 0;

ObjectFactory::ObjectFactory() :
	local_id_(0)
{
	m_name = "object factory";
	default_object_name_ = "default object";
	createDefault();
}

ObjectFactory::~ObjectFactory()
{

}

void ObjectFactory::createDefault()
{
	if (!getDefault())
	{
		Object* object = new Object();
		object->setName(default_object_name_);
		objects_.push_back(object);
	}
}

bool ObjectFactory::readDefault(std::istream &is)
{
	return true;
}

bool ObjectFactory::writeDefault(std::ostream &os)
{
	return true;
}

bool ObjectFactory::readDefault(std::string &filename)
{
	Object* object = getDefault();
	if (!object)
		return false;

	using boost::property_tree::ptree;
	ptree pt;
	read_xml(filename, pt);

	//a default setting tree has two levels
	//first level: type of the object
	//second level: name="value name" type="value type" value="default value"
	std::string parent_name;
	auto parent = pt.begin();
	parent_name = parent->first;
	for (const auto& i : pt.get_child(parent_name))
	{

	}

	return true;
}

bool ObjectFactory::writeDefault(std::string &filename)
{
	return true;
}

Object* ObjectFactory::build()
{
	incCounter();

	Object* object = new Object();
	object->setId(local_id_);
	std::string name = "object" + std::to_string(local_id_);
	object->setName(name);

	objects_.push_back(object);

	return object;
}

Object* ObjectFactory::clone(Object* object)
{
	incCounter();

	Object* new_object = object->clone(CopyOp::DEEP_COPY_OBJECTS);
	new_object->setId(local_id_);
	std::string name = "object" + std::to_string(local_id_);
	new_object->setName(name);

	objects_.push_back(new_object);

	return new_object;
}

Object* ObjectFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
		return clone(object);
	else
		return 0;
}

