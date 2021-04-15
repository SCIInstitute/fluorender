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

#include "Global.hpp"
#include "Names.hpp"
#include <Annotations/AnnotationFactory.hpp>
#include <Mesh/MeshFactory.hpp>
#include <Volume/VolumeFactory.hpp>
#include <FUI/Panels/AgentFactory.hpp>
#include <Renderer2DFactory.hpp>
#include <Renderer3DFactory.hpp>
#include <SearchVisitor.hpp>

using namespace fluo;

Global Global::instance_;
Global::Global()
{
	origin_ = ref_ptr<Group>(new Group());
	origin_->setName(flstrOrigin);
	BuildFactories();
}

void Global::BuildFactories()
{
	Group* factory_group = new Group();
	factory_group->setName(flstrFactoryGroup);
	origin_->addChild(factory_group);
	BUILD_AND_ADD(VolumeFactory, factory_group);
	BUILD_AND_ADD(MeshFactory, factory_group);
	BUILD_AND_ADD(AnnotationFactory, factory_group);
	BUILD_AND_ADD(AgentFactory, factory_group);
	BUILD_AND_ADD(Renderer2DFactory, factory_group);
	BUILD_AND_ADD(Renderer3DFactory, factory_group);
}

Object* Global::get(const std::string &name)
{
	SearchVisitor visitor;
	visitor.matchName(name);
	origin_->accept(visitor);
	ObjectList* list = visitor.getResult();
	if (list->empty())
		return 0;
	else
		return (*list)[0];
}

VolumeFactory* Global::getVolumeFactory()
{
	Object* obj = get(flstrVolumeFactory);
	if (!obj)
		return 0;
	return dynamic_cast<VolumeFactory*>(obj);
}

MeshFactory* Global::getMeshFactory()
{
	Object* obj = get(flstrMeshFactory);
	if (!obj)
		return 0;
	return dynamic_cast<MeshFactory*>(obj);
}

AnnotationFactory* Global::getAnnotationFactory()
{
	Object* obj = get(flstrAnnotationFactory);
	if (!obj)
		return 0;
	return dynamic_cast<AnnotationFactory*>(obj);
}

AgentFactory* Global::getAgentFactory()
{
	Object* obj = get(flstrAgentFactory);
	if (!obj)
		return 0;
	return dynamic_cast<AgentFactory*>(obj);
}

Renderer2DFactory* Global::getRenderer2DFactory()
{
	Object* obj = get(flstrRenderer2DFactory);
	if (!obj)
		return 0;
	return dynamic_cast<Renderer2DFactory*>(obj);
}

Renderer3DFactory* Global::getRenderer3DFactory()
{
	Object* obj = get(flstrRenderer3DFactory);
	if (!obj)
		return 0;
	return dynamic_cast<Renderer3DFactory*>(obj);
}
