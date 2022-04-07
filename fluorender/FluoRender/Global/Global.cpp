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
#include <Input.hpp>
#include <AnnotationFactory.hpp>
#include <MeshFactory.hpp>
#include <VolumeFactory.hpp>
#include <RenderviewFactory.hpp>
#include <AgentFactory.hpp>
#include <AsyncTimerFactory.hpp>
#include <StopWatchFactory.hpp>
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
	BuildInput();
	BuildFactories();
	BuildPaths();
	BuildRoot();
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
	BUILD_AND_ADD(AsyncTimerFactory, factory_group);
	BUILD_AND_ADD(StopWatchFactory, factory_group);
	//BUILD_AND_ADD(Renderer2DFactory, factory_group);
	//BUILD_AND_ADD(Renderer3DFactory, factory_group);
	
	//create default
	getVolumeFactory()->createDefault();
	getMeshFactory()->createDefault();
	getAnnotationFactory()->createDefault();
	getRenderviewFactory()->createDefault();
	getAsyncTimerFactory()->createDefault();
	getStopWatchFactory()->createDefault();
}

void Global::BuildPaths()
{
	Node* paths = new Node();
	paths->setName(gstPaths);
	origin_->addChild(paths);
}

#define ADD_VALUE(name, v) \
	root->addValue(name, v)

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
	//others to sync with children
	ADD_VALUE(gstSaveProjectEnable, bool(false));
	ADD_VALUE(gstProjectPath, std::wstring(L""));
	ADD_VALUE(gstCaptureAlpha, bool(false));
	ADD_VALUE(gstCaptureFloat, bool(false));
	ADD_VALUE(gstCaptureCompress, bool(false));
	ADD_VALUE(gstHardwareCompress, bool(false));
	ADD_VALUE(gstSkipBrick, bool(false));
	ADD_VALUE(gstTestSpeed, bool(false));
	ADD_VALUE(gstTestParam, bool(false));
	ADD_VALUE(gstTestWiref, bool(false));
	ADD_VALUE(gstPeelNum, long(1));
	ADD_VALUE(gstMicroBlendEnable, bool(false));
	ADD_VALUE(gstShadowDirEnable, bool(false));
	ADD_VALUE(gstShadowDirX, double(0));
	ADD_VALUE(gstShadowDirY, double(0));
	ADD_VALUE(gstAdaptive, bool(true));
	ADD_VALUE(gstWaveColor1, long(5));
	ADD_VALUE(gstWaveColor2, long(5));
	ADD_VALUE(gstWaveColor3, long(5));
	ADD_VALUE(gstWaveColor4, long(5));
	ADD_VALUE(gstTimeFileId, std::string("_T"));
	ADD_VALUE(gstGradBg, bool(false));
	ADD_VALUE(gstPinThresh, double(10));
	ADD_VALUE(gstVrEnable, bool(false));
	ADD_VALUE(gstVrEyeOffset, double(20));
	ADD_VALUE(gstOverrideVoxSpc, bool(true));
	ADD_VALUE(gstSoftThresh, double(0));
	ADD_VALUE(gstRunScript, bool(false));
	ADD_VALUE(gstScriptFile, std::wstring(L""));
	ADD_VALUE(gstTextSize, double(14));
	ADD_VALUE(gstTextColorMode, long(0));
	ADD_VALUE(gstFontFile, std::string(""));
	ADD_VALUE(gstLineWidth, double(3));
	ADD_VALUE(gstStreamEnable, bool(false));
	ADD_VALUE(gstGpuMemSize, double(1000));
	ADD_VALUE(gstLargeDataSize, double(1000));
	ADD_VALUE(gstBrickSize, long(128));
	ADD_VALUE(gstResponseTime, long(100));
	ADD_VALUE(gstStreamOrder, long(0));
	ADD_VALUE(gstLodOffset, long(0));
	ADD_VALUE(gstPointVolumeMode, long(0));
	ADD_VALUE(gstRulerUseTransf, bool(false));
	ADD_VALUE(gstRulerTransient, bool(true));
	ADD_VALUE(gstRulerF1, double(2));
	ADD_VALUE(gstRulerInfr, double(2));
	ADD_VALUE(gstRulerRelaxIter, long(10));
	ADD_VALUE(gstRulerRelax, bool(false));
	ADD_VALUE(gstRulerRelaxType, long(1));
	ADD_VALUE(gstRulerDfoverf, bool(false));
	ADD_VALUE(gstRulerSizeThresh, long(5));
	ADD_VALUE(gstPvxmlFlipX, bool(false));
	ADD_VALUE(gstPvxmlFlipY, bool(false));
	ADD_VALUE(gstPvxmlSeqType, long(1));
	ADD_VALUE(gstApiType, long(0));
	ADD_VALUE(gstOutputBitR, long(8));
	ADD_VALUE(gstOutputBitG, long(8));
	ADD_VALUE(gstOutputBitB, long(8));
	ADD_VALUE(gstOutputBitA, long(8));
	ADD_VALUE(gstOutputBitD, long(24));
	ADD_VALUE(gstOutputSamples, long(0));
	ADD_VALUE(gstGlVersionMajor, long(4));
	ADD_VALUE(gstGlVersionMinor, long(6));
	ADD_VALUE(gstGlProfileMask, long(2));
	ADD_VALUE(gstClPlatformId, long(0));
	ADD_VALUE(gstClDeviceId, long(0));
	ADD_VALUE(gstPaintHistory, long(0));
	ADD_VALUE(gstStayOnTop, bool(false));
	ADD_VALUE(gstShowCursor, bool(true));
	ADD_VALUE(gstLastTool, long(0));
	ADD_VALUE(gstTrackIter, long(3));
	ADD_VALUE(gstCompSizeLimit, long(5));
	ADD_VALUE(gstCompConsistent, bool(true));
	ADD_VALUE(gstTryMerge, bool(false));
	ADD_VALUE(gstTrySplit, bool(false));
	ADD_VALUE(gstContactFactor, double(0.6));
	ADD_VALUE(gstSimilarity, double(0.5));
	ADD_VALUE(gstMaxTextureSizeEnable, bool(false));
	ADD_VALUE(gstMaxTextureSize, long(2048));
	ADD_VALUE(gstNoTexPack, bool(false));
	ADD_VALUE(gstClipPlaneMode, long(0));
	ADD_VALUE(gstImagejMode, long(0));
	ADD_VALUE(gstJvmPath, std::string(""));
	ADD_VALUE(gstImagejPath, std::string(""));
	ADD_VALUE(gstBioformatsPath, std::string(""));
	ADD_VALUE(gstOpenSlices, bool(false));
	ADD_VALUE(gstOpenChanns, bool(false));
	ADD_VALUE(gstOpenDigitOrder, long(0));
	ADD_VALUE(gstOpenSeriesNum, long(0));
	ADD_VALUE(gstEmbedDataInProject, bool(false));
	ADD_VALUE(gstLoadMask, bool(true));
	ADD_VALUE(gstLoadLabel, bool(true));
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

AsyncTimer* Global::getAsyncTimer(const std::string &name)
{
	return glbin_atmf->findFirst(name);
}

StopWatch* Global::getStopWatch(const std::string &name)
{
	return glbin_swhf->findFirst(name);
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

AsyncTimerFactory* Global::getAsyncTimerFactory()
{
	Object* obj = get(gstAsyncTimerFactory);
	if (!obj)
		return 0;
	return dynamic_cast<AsyncTimerFactory*>(obj);
}

StopWatchFactory* Global::getStopWatchFactory()
{
	Object* obj = get(gstStopWatchFactory);
	if (!obj)
		return 0;
	return dynamic_cast<StopWatchFactory*>(obj);
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
