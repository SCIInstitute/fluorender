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

#include "MeshFactory.hpp"
#include <Mesh/MeshGroup.hpp>

#include <iostream>

using namespace fluo;

MeshFactory::MeshFactory()
{
	m_name = "mesh factory";
	default_object_name_ = "default mesh";

	addValue("current", (MeshData*)(0));//current mesh data (for processing etc)
}

MeshFactory::~MeshFactory()
{

}

void MeshFactory::createDefault()
{
	if (!getDefault())
	{
		MeshData* md = new MeshData();
		md->setName(default_object_name_);
		objects_.push_front(md);

        std::cout << "Debugging starts here." << std::endl;

		//add default values here
		md->addValue("data path", std::wstring());//path to obj file
		md->addValue("bounds", FLTYPE::BBox());//bounding box
		md->addValue("bounds tf", FLTYPE::BBox());//bounding box after transformation
		md->addValue("center", FLTYPE::Point());

		//resolution for clipping planes
		md->addValue("clip planes", FLTYPE::PlaneSet(6));
		md->addValue("res x", long(0));
		md->addValue("res y", long(0));
		md->addValue("res z", long(0));
		//clip distance
		md->addValue("clip dist x", double(1));
		md->addValue("clip dist y", double(1));
		md->addValue("clip dist z", double(1));

		md->addValue("display", bool(true));
		md->addValue("draw bounds", bool(false));

		//lighting
		md->addValue("light enable", bool(true));
		md->addValue("depth atten", bool(false));
		md->addValue("da int", double(0.5));
		md->addValue("da start", double(0));
		md->addValue("da end", double(1));
		md->addValue("mat amb", FLTYPE::Color());
		md->addValue("color", FLTYPE::Color());
		md->addValue("mat spec", FLTYPE::Color());
		md->addValue("mat shine", double(0));
		md->addValue("alpha", double(1));
		md->addValue("randomize color", bool(false));

		//shadow
		md->addValue("shadow enable", bool(false));
		md->addValue("shadow int", double(1));

		//size limiter
		md->addValue("limit enable", bool(false));
		md->addValue("limit", long(0));

		//transforms
		md->addValue("trans x", double(0));
		md->addValue("trans y", double(0));
		md->addValue("trans z", double(0));
		md->addValue("rot x", double(0));
		md->addValue("rot y", double(0));
		md->addValue("rot z", double(0));
		md->addValue("scale x", double(1));
		md->addValue("scale y", double(1));
		md->addValue("scale z", double(1));

		//legend
		md->addValue("legend", bool(true));

		//viewport
		md->addValue("viewport", FLTYPE::GLint4());

		//selected on the ui
		md->addValue("selected", bool(false));
	}
}

#define ADD_BEFORE_EVENT(obj, name, funct) \
	obj->setValueChangingFunction(name, std::bind(&MeshData::funct, obj, std::placeholders::_1))

#define ADD_AFTER_EVENT(obj, name, funct) \
	obj->setValueChangedFunction(name, std::bind(&MeshData::funct, obj, std::placeholders::_1))

void MeshFactory::setEventHandler(MeshData* md)
{
	ADD_AFTER_EVENT(md, "viewport", OnViewportChanged);
	ADD_AFTER_EVENT(md, "light enable", OnLightEnableChanged);
	ADD_AFTER_EVENT(md, "depth atten", OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, "da int", OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, "da start", OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, "da end", OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, "mat amb", OnMaterialChanged);
	ADD_AFTER_EVENT(md, "color", OnMaterialChanged);
	ADD_AFTER_EVENT(md, "mat spec", OnMaterialChanged);
	ADD_AFTER_EVENT(md, "mat shine", OnMaterialChanged);
	ADD_AFTER_EVENT(md, "alpha", OnMaterialChanged);
	ADD_AFTER_EVENT(md, "bounds", OnBoundsChanged);
	ADD_AFTER_EVENT(md, "randomize color", OnRandomizeColor);
}

MeshData* MeshFactory::build(MeshData* md)
{
	unsigned int default_id = 0;
	return clone(default_id);
}

MeshData* MeshFactory::clone(MeshData* md)
{
	if (!md)
		return 0;

	incCounter();

	Object* new_md = md->clone(CopyOp::DEEP_COPY_ALL);
	new_md->setId(global_id_);
	std::string name = "mesh" + std::to_string(local_id_);
	new_md->setName(name);

	objects_.push_front(new_md);
    setValue("current", new_md);


	setEventHandler(dynamic_cast<MeshData*>(new_md));

	//notify observers
	Event event;
	event.init(Event::EVENT_NODE_ADDED,
		this, new_md);
	notifyObservers(event);

	return dynamic_cast<MeshData*>(new_md);
}

MeshData* MeshFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		MeshData* md = dynamic_cast<MeshData*>(object);
		if (md)
			return clone(md);
	}
	return 0;
}

MeshGroup* MeshFactory::buildGroup(MeshData* md)
{
	MeshGroup* group;
	if (md)
		group = new MeshGroup(*md, CopyOp::DEEP_COPY_ALL);//shallow copy might be ok
	else
		group = new MeshGroup(*getDefault(), CopyOp::DEEP_COPY_ALL);
	if (group)
	{
		group->setName("Group");
		group->setValueChangedFunction(
			"randomize color", std::bind(
				&MeshGroup::OnRandomizeColor,
				group, std::placeholders::_1));
	}
	return group;
}
