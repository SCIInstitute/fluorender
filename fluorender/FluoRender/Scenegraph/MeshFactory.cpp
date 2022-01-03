/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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
#include <MeshGroup.hpp>
#include <Names.hpp>

#include <iostream>

using namespace fluo;

MeshFactory::MeshFactory()
{
	m_name = gstMeshFactory;
	default_object_name_ = gstDefaultMesh;

	addRvalu(gstCurrent, (MeshData*)(0));//current mesh data (for processing etc)
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

		//std::cout << "Debugging starts here." << std::endl;

		//add default values here
		md->addValue(gstDataPath, std::wstring());//path to obj file
		md->addValue(gstBounds, BBox());//bounding box
		md->addValue(gstBoundsTf, BBox());//bounding box after transformation
		md->addValue(gstCenter, Point());

		//resolution for clipping planes
		md->addValue(gstClipPlanes, PlaneSet(6));
		md->addValue(gstResX, long(0));
		md->addValue(gstResY, long(0));
		md->addValue(gstResZ, long(0));
		//clip distance
		md->addValue(gstClipDistX, double(1));
		md->addValue(gstClipDistY, double(1));
		md->addValue(gstClipDistZ, double(1));

		md->addValue(gstDisplay, bool(true));
		md->addValue(gstDrawBounds, bool(false));

		//lighting
		md->addValue(gstShadingEnable, bool(true));
		md->addValue(gstDepthAtten, bool(false));
		md->addValue(gstDaInt, double(0.5));
		md->addValue(gstDaStart, double(0));
		md->addValue(gstDaEnd, double(1));
		md->addValue(gstColor, Color());
		md->addValue(gstMatAmb, double(1));
		md->addValue(gstMatDiff, double(1));
		md->addValue(gstMatSpec, double(1));
		md->addValue(gstMatShine, double(10));
		md->addValue(gstAlpha, double(1));
		md->addValue(gstRandomizeColor, bool(false));

		//shadow
		md->addValue(gstShadowEnable, bool(false));
		md->addValue(gstShadowInt, double(1));

		//size limiter
		md->addValue(gstLimitEnable, bool(false));
		md->addValue(gstLimit, long(0));

		//transforms
		md->addValue(gstTransX, double(0));
		md->addValue(gstTransY, double(0));
		md->addValue(gstTransZ, double(0));
		md->addValue(gstRotX, double(0));
		md->addValue(gstRotY, double(0));
		md->addValue(gstRotZ, double(0));
		md->addValue(gstScaleX, double(1));
		md->addValue(gstScaleY, double(1));
		md->addValue(gstScaleZ, double(1));

		//legend
		md->addValue(gstLegend, bool(true));

		//viewport
		md->addValue(gstViewport, Vector4i());

		//sync group
		md->addValue(gstSyncGroup, bool(false));

		//selected on the ui
		md->addValue(gstSelected, bool(false));
	}
}

#define ADD_BEFORE_EVENT(obj, name, funct) \
	obj->setValueChangingFunction(name, std::bind(&MeshData::funct, obj, std::placeholders::_1))

#define ADD_AFTER_EVENT(obj, name, funct) \
	obj->setValueChangedFunction(name, std::bind(&MeshData::funct, obj, std::placeholders::_1))

void MeshFactory::setEventHandler(MeshData* md)
{
	ADD_AFTER_EVENT(md, gstViewport, OnViewportChanged);
	ADD_AFTER_EVENT(md, gstShadingEnable, OnLightEnableChanged);
	ADD_AFTER_EVENT(md, gstDepthAtten, OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, gstDaInt, OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, gstDaStart, OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, gstDaEnd, OnDepthAttenChanged);
	ADD_AFTER_EVENT(md, gstColor, OnMaterialChanged);
	ADD_AFTER_EVENT(md, gstMatAmb, OnMaterialChanged);
	ADD_AFTER_EVENT(md, gstMatDiff, OnMaterialChanged);
	ADD_AFTER_EVENT(md, gstMatSpec, OnMaterialChanged);
	ADD_AFTER_EVENT(md, gstMatSpec, OnMaterialChanged);
	ADD_AFTER_EVENT(md, gstAlpha, OnMaterialChanged);
	ADD_AFTER_EVENT(md, gstBounds, OnBoundsChanged);
	ADD_AFTER_EVENT(md, gstRandomizeColor, OnRandomizeColor);
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
	setRvalu(gstCurrent, new_md);


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
