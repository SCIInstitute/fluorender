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

#include <ObjectFactory.hpp>

using namespace fluo;

unsigned int ObjectFactory::global_id_ = 0;

ObjectFactory::ObjectFactory() :
	Node(),
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

bool ObjectFactory::setDefaultValues(boost::property_tree::ptree &pt, const ValueCollection &names)
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
		if (!names.empty() && names.find(name) == names.end())
			continue;
		std::string type = sub_pt.get<std::string>("<xmlattr>.type");
		std::string value = sub_pt.get<std::string>("<xmlattr>.value");
		ValueTuple vt{name, type, value};
		//if no this value, add one, otherwise change its value
		if (!object->addValue(vt))
			object->setValueTuple(vt);
	}

	return true;
}

bool ObjectFactory::convDefaultValues(boost::property_tree::ptree &pt, const ValueCollection &names)
{
	Object* object = getDefault();
	if (!object)
		return false;

	using boost::property_tree::ptree;
	if (!pt.empty())
		pt.clear();
	ptree parent;
	//get all value names
	ValueCollection all_names =
		object->getValueCollection();
	for (auto it = all_names.begin();
		it != all_names.end(); ++it)
	{
		ValueTuple vt;
		std::get<0>(vt) = *it;
		if (object->getValue(vt))
		{
			std::string name = std::get<0>(vt);
			if (!names.empty() && names.find(name) == names.end())
				continue;
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

bool ObjectFactory::replaceDefaultValues(boost::property_tree::ptree &pt, const ValueCollection &names)
{
	Object* object = getDefault();
	if (!object)
		return false;

	using boost::property_tree::ptree;
	auto parent = pt.begin();
	//sub tree of value set
	ptree pt_vs = parent->second;
	ptree pt_vs2;

	for (auto& i : pt_vs)
	{
		std::string child_name;
		ptree pt_child;
		std::tie(child_name, pt_child) = i;
		if (child_name != "Value")
			continue;
		std::string value_name = pt_child.get<std::string>("<xmlattr>.name");
		ValueTuple vt;
		std::get<0>(vt) = value_name;
		if (object->getValue(vt))
		{
			std::string type;
			std::string val;
			if (names.find(value_name) != names.end())
			{
				type = std::get<1>(vt);
				val = std::get<2>(vt);
			}
			else
			{
				type = pt_child.get<std::string>("<xmlattr>.type");
				val = pt_child.get<std::string>("<xmlattr>.value");
			}
			ptree child;
			child.put("<xmlattr>.name", value_name);
			child.put("<xmlattr>.type", type);
			child.put("<xmlattr>.value", val);
			pt_vs2.add_child("Value", child);
		}
	}
	pt.clear();
	pt.add_child("ValueSet", pt_vs2);
	return true;
}

void ObjectFactory::propValuesToDefault(Object* obj, const ValueCollection &names)
{
	Object* def_obj = getDefault();
	if (!def_obj || ! obj)
		return;

	if (names.empty())
		obj->propAllValues(def_obj);
	else
		obj->propValues(names, def_obj);
}

void ObjectFactory::propValuesFromDefault(Object* obj, const ValueCollection &names)
{
	Object* def_obj = getDefault();
	if (!def_obj || !obj)
		return;

	if (names.empty())
		def_obj->propAllValues(obj);
	else
		def_obj->propValues(names, obj);
}

bool ObjectFactory::readDefault(std::istream &is, const ValueCollection &names)
{
	using boost::property_tree::ptree;
	ptree pt;
	try
	{
		read_xml(is, pt);
	}
	catch (...)
	{
		return false;
	}
	setDefaultValues(pt, names);

	return true;
}

bool ObjectFactory::writeDefault(std::ostream &os, const ValueCollection &names, int indent)
{
	using boost::property_tree::ptree;
	ptree pt;
	convDefaultValues(pt, names);
	//write_xml(os, pt,
	//	boost::property_tree::xml_writer_make_settings< std::string >('\t', 1));
	try
	{
		write_xml_element(os, ptree::key_type(), pt, -1,
			boost::property_tree::xml_writer_make_settings< std::string >('\t', indent));
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool ObjectFactory::readDefault(const ValueCollection &names)
{
	std::string filename;
	getValue(default_setting_filename_value_name_, filename);

	using boost::property_tree::ptree;
	ptree pt;
	try
	{
		read_xml(filename, pt);
	}
	catch (...)
	{
		return false;
	}
	setDefaultValues(pt, names);

	return true;
}

bool ObjectFactory::writeDefault(const ValueCollection &names)
{
	std::string filename;
	getValue(default_setting_filename_value_name_, filename);

	using boost::property_tree::ptree;
	ptree pt;
	//read original first
	try
	{
		read_xml(filename, pt);
	}
	catch (...)
	{
		return false;
	}
	replaceDefaultValues(pt, names);
	try
	{
		write_xml(filename, pt, std::locale(),
			boost::property_tree::xml_writer_make_settings< std::string >('\t', 1));
	}
	catch (...)
	{
		return false;
	}

	return true;
}

void ObjectFactory::createDefault()
{
	if (!getDefault())
	{
		Object* object = new Object();
		object->setName(default_object_name_);
		objects_.push_front(object);
	}
}

Object* ObjectFactory::build(Object* obj)
{
	if (obj)
	{
		//not used in parent class
        return nullptr;
	}
	else
	{
		unsigned int default_id = 0;
		return clone(default_id);
	}
}

Object* ObjectFactory::clone(Object* object)
{
	incCounter();

	Object* new_object = object->clone(CopyOp::DEEP_COPY_OBJECTS);
	new_object->setId(local_id_);
	std::string name = "object" + std::to_string(local_id_);
	new_object->setName(name);

	objects_.push_front(new_object);

	//notify observers
	Event event;
	event.init(Event::EVENT_NODE_ADDED,
		this, object);
	notifyObservers(event);

	return new_object;
}

Object* ObjectFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
		return clone(object);
	else
        return nullptr;
}

