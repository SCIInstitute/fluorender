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

using namespace flrd;

unsigned int ObjectFactory::global_id_ = 0;

ObjectFactory::ObjectFactory() :
	local_id_(0)
{
	m_name = "object factory";
	default_object_name_ = "default object";
	default_setting_filename_value_name_ = "default filename";//maybe not necessary
	std::string filename = "";
	addValue(default_setting_filename_value_name_, filename);
}

ObjectFactory::~ObjectFactory()
{

}

void ObjectFactory::objectChanged(void* ptr, const std::string &exp)
{
	Object::objectChanged(ptr, exp);//actually unnecessary, since there is nothing to sync
	Referenced* refd = static_cast<Referenced*>(ptr);
	if (refd->className() == std::string("Value"))
	{
		Value* value = dynamic_cast<Value*>(refd);
		if (value->getName() == default_setting_filename_value_name_)
			readDefault();
	}
}

bool ObjectFactory::setDefaultValues(boost::property_tree::ptree &pt)
{
	Object* object = getDefault();
	if (!object)
	{
		createDefault();
		object = getDefault();
		if (!object)
			return false;
	}

	using boost::property_tree::ptree;
	//a default setting tree has two levels
	//first level: type of the object
	//second level: name="value name" type="value type" value="default value"
	std::string parent_name;
	auto parent = pt.begin();
	parent_name = parent->first;
	for (const auto& i : pt.get_child(parent_name))
	{
		std::string child_name;
		ptree sub_pt;
		std::tie(child_name, sub_pt) = i;
		if (child_name != "Value")
			continue;
		std::string name = sub_pt.get<std::string>("<xmlattr>.name");
		std::string type = sub_pt.get<std::string>("<xmlattr>.type");
		std::string value = sub_pt.get<std::string>("<xmlattr>.value");
		ValueTuple vt{name, type, value};
		//if no this value, add one, otherwise change its value
		if (!object->addValue(vt))
			object->setValue(vt);
	}

	return true;
}

bool ObjectFactory::convDefaultValues(boost::property_tree::ptree &pt)
{
	Object* object = getDefault();
	if (!object)
		return false;

	using boost::property_tree::ptree;
	if (!pt.empty())
		pt.clear();
	ptree parent;
	//get all value names
	std::vector<std::string> names =
		object->getValueNames();
	for (auto it = names.begin();
		it != names.end(); ++it)
	{
		ValueTuple vt;
		std::get<0>(vt) = *it;
		if (object->getValue(vt))
		{
			std::string name = std::get<0>(vt);
			std::string type = std::get<1>(vt);
			std::string val = std::get<2>(vt);
			ptree child;
			child.put("<xmlattr>.name", name);
			child.put("<xmlattr>.type", type);
			child.put("<xmlattr>.value", val);
			parent.add_child("Value", child);
		}
	}

	//add parent
	pt.add_child("ValueSet", parent);
	return true;
}

bool ObjectFactory::readDefault(std::istream &is)
{
	using boost::property_tree::ptree;
	ptree pt;
	read_xml(is, pt);
	setDefaultValues(pt);

	return true;
}

bool ObjectFactory::writeDefault(std::ostream &os, int indent)
{
	using boost::property_tree::ptree;
	ptree pt;
	convDefaultValues(pt);
	//write_xml(os, pt,
	//	boost::property_tree::xml_writer_make_settings< std::string >('\t', 1));
	write_xml_element(os, ptree::key_type(), pt, -1,
		boost::property_tree::xml_writer_make_settings< std::string >('\t', indent));

	return true;
}

bool ObjectFactory::readDefault()
{
	std::string filename;
	getValue(default_setting_filename_value_name_, filename);

	using boost::property_tree::ptree;
	ptree pt;
	read_xml(filename, pt);
	setDefaultValues(pt);

	return true;
}

bool ObjectFactory::writeDefault()
{
	std::string filename;
	getValue(default_setting_filename_value_name_, filename);

	using boost::property_tree::ptree;
	ptree pt;
	convDefaultValues(pt);
	write_xml(filename, pt, std::locale(),
		boost::property_tree::xml_writer_make_settings< std::string >('\t', 1));

	return true;
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

Object* ObjectFactory::build()
{
	unsigned int default_id = 0;
	return clone(default_id);
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

