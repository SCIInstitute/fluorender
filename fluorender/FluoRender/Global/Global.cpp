/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <Global.hpp>
#include <Timer.hpp>
#include <Input.hpp>
#include <AnnotationFactory.hpp>
#include <MeshFactory.hpp>
#include <VolumeFactory.hpp>
#include <RenderviewFactory.hpp>
#include <AgentFactory.hpp>
//#include <Renderer2DFactory.hpp>
//#include <Renderer3DFactory.hpp>
#include <Root.hpp>
#include <SearchVisitor.hpp>
#include <DecycleVisitor.hpp>

using namespace fluo;

Global Global::instance_;
Global::Global()
{
	origin_ = ref_ptr<Group>(new Group());
	origin_->setName(gstOrigin);
	BuildStopWatch();
	BuildInput();
	BuildFactories();
	BuildPaths();
	BuildRoot();
}

void Global::BuildStopWatch()
{
	StopWatch* sw = new StopWatch();
	sw->setName(gstStopWatch);
	origin_->addChild(sw);
}

void Global::BuildInput()
{
	Flinput* input = new Flinput();
	input->setName(gstInput);
	origin_->addChild(input);
}

void Global::BuildFactories()
{
	Group* factory_group = new Group();
	factory_group->setName(gstFactoryGroup);
	origin_->addChild(factory_group);
	BUILD_AND_ADD(VolumeFactory, factory_group);
	BUILD_AND_ADD(MeshFactory, factory_group);
	BUILD_AND_ADD(AnnotationFactory, factory_group);
	BUILD_AND_ADD(RenderviewFactory, factory_group);
	BUILD_AND_ADD(AgentFactory, factory_group);
	//BUILD_AND_ADD(Renderer2DFactory, factory_group);
	//BUILD_AND_ADD(Renderer3DFactory, factory_group);
	
	//create default
	getVolumeFactory()->createDefault();
	getMeshFactory()->createDefault();
	getAnnotationFactory()->createDefault();
	getRenderviewFactory()->createDefault();
}

void Global::BuildPaths()
{
	Node* paths = new Node();
	paths->setName(gstPaths);
	origin_->addChild(paths);
}

void Global::BuildRoot()
{
	Root* root = new Root();
	root->setName(gstRoot);
	root->addValue(gstCurrentSelect, long(0));
	root->addRvalu(gstCurrentView, (Referenced*)(0));
	root->addRvalu(gstCurrentVolume, (Referenced*)(0));
	root->addRvalu(gstCurrentMesh, (Referenced*)(0));
	root->addRvalu(gstCurrentVolumeGroup, (Referenced*)(0));
	root->addRvalu(gstCurrentMeshGroup, (Referenced*)(0));
	root->addRvalu(gstCurrentAnnotations, (Referenced*)(0));
	//copy source
	root->addRvalu(gstSourceVolume, (Referenced*)(0));
	root->addValue(gstSourceMode, long(0));
	origin_->addChild(root);
}

void Global::initIcons()
{
	shown_icon_list_.init(true);
	hidden_icon_list_.init(false);
}

void Global::decycle()
{
	DecycleVisitor decycle(*origin_);
	decycle.removeCycle();
}

void Global::clear()
{
	decycle();
	origin_->removeAllChildren();
}

Object* Global::get(const std::string &name, Group* start)
{
	SearchVisitor visitor;
	visitor.matchName(name);
	if (start)
		start->accept(visitor);
	else
		origin_->accept(visitor);
	ObjectList* list = visitor.getResult();
	if (list->empty())
		return 0;
	else
		return (*list)[0];
}

StopWatch* Global::getStopWatch()
{
	Object* obj = get(gstStopWatch);
	if (!obj)
		return 0;
	return dynamic_cast<StopWatch*>(obj);
}

Flinput* Global::getInput()
{
	Object* obj = get(gstInput);
	if (!obj)
		return 0;
	return dynamic_cast<Flinput*>(obj);
}

VolumeFactory* Global::getVolumeFactory()
{
	Object* obj = get(gstVolumeFactory);
	if (!obj)
		return 0;
	return dynamic_cast<VolumeFactory*>(obj);
}

MeshFactory* Global::getMeshFactory()
{
	Object* obj = get(gstMeshFactory);
	if (!obj)
		return 0;
	return dynamic_cast<MeshFactory*>(obj);
}

AnnotationFactory* Global::getAnnotationFactory()
{
	Object* obj = get(gstAnnotationFactory);
	if (!obj)
		return 0;
	return dynamic_cast<AnnotationFactory*>(obj);
}

RenderviewFactory* Global::getRenderviewFactory()
{
	Object* obj = get(gstRenderviewFactory);
	if (!obj)
		return 0;
	return dynamic_cast<RenderviewFactory*>(obj);
}

AgentFactory* Global::getAgentFactory()
{
	Object* obj = get(gstAgentFactory);
	if (!obj)
		return 0;
	return dynamic_cast<AgentFactory*>(obj);
}

//Renderer2DFactory* Global::getRenderer2DFactory()
//{
//	Object* obj = get(flstrRenderer2DFactory);
//	if (!obj)
//		return 0;
//	return dynamic_cast<Renderer2DFactory*>(obj);
//}
//
//Renderer3DFactory* Global::getRenderer3DFactory()
//{
//	Object* obj = get(flstrRenderer3DFactory);
//	if (!obj)
//		return 0;
//	return dynamic_cast<Renderer3DFactory*>(obj);
//}

Root* Global::getRoot()
{
	Object* obj = get(gstRoot);
	return dynamic_cast<Root*>(obj);
}

bool Global::checkName(const std::string &name)
{
	Object* obj = get(name);
	return (obj != nullptr);
}

//paths
void Global::setExecutablePath(const std::wstring &path)
{
	Object* paths = get(gstPaths);
	if (!paths)
		return;
	paths->addSetValue(gstExecutablePath, path);
}

std::wstring Global::getExecutablePath()
{
	Object* paths = get(gstPaths);
	if (!paths)
		return L"";
	std::wstring path;
	paths->getValue(gstExecutablePath, path);
	return path;
}

//list panel operations
size_t Global::getObjNumInList()
{
	size_t volume_num = getVolumeFactory()->getNum();
	size_t mesh_num = getMeshFactory()->getNum();
	size_t annotations_num = getAnnotationFactory()->getNum();

	return volume_num + mesh_num + annotations_num;
}

Object* Global::getObjInList(size_t i)
{
	size_t volume_num = getVolumeFactory()->getNum();
	size_t mesh_num = getMeshFactory()->getNum();
	size_t annotations_num = getAnnotationFactory()->getNum();

	if (i < volume_num)
		return getVolumeFactory()->get(i);
	else if (i < volume_num + mesh_num)
		return getMeshFactory()->get(i - volume_num);
	else if (i < volume_num + mesh_num + annotations_num)
		return getAnnotationFactory()->get(i - volume_num - mesh_num);
	return 0;
}

void Global::addListObserver(Observer* obsrvr)
{
	getVolumeFactory()->addObserver(obsrvr);
	getMeshFactory()->addObserver(obsrvr);
	getAnnotationFactory()->addObserver(obsrvr);
}

void Global::saveAllMasks()
{
}
