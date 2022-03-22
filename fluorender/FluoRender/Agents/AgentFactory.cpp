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

#include <AgentFactory.hpp>
#include <AnnotationPropAgent.hpp>
#include <BrushToolAgent.hpp>
#include <CalculationAgent.hpp>
#include <ClipPlaneAgent.hpp>
#include <ClKernelAgent.hpp>
#include <ColocalAgent.hpp>
#include <ComponentAgent.hpp>
#include <ConvertAgent.hpp>
#include <CountingAgent.hpp>
#include <ListModel.hpp>
#include <MeasureAgent.hpp>
#include <MeshPropAgent.hpp>
#include <MeshTransAgent.hpp>
#include <MovieAgent.hpp>
#include <NoiseReduceAgent.hpp>
#include <OutAdjustAgent.hpp>
#include <RecorderAgent.hpp>
#include <RenderCanvasAgent.hpp>
#include <RenderFrameAgent.hpp>
#include <RenderviewAgent.hpp>
#include <SettingAgent.hpp>
#include <TrackAgent.hpp>
#include <TreeModel.hpp>
#include <VolumePropAgent.hpp>
//windows
#include <AnnotationPropPanel.h>
#include <BrushToolDlg.h>
#include <CalculationDlg.h>
#include <ClipPlanePanel.h>
#include <ClKernelDlg.h>
#include <ColocalDlg.h>
#include <ComponentDlg.h>
#include <ConvertDlg.h>
#include <CountingDlg.h>
#include <ListPanel.h>
#include <MeasureDlg.h>
#include <MeshPropPanel.h>
#include <MeshTransPanel.h>
#include <MoviePanel.h>
#include <NoiseReduceDlg.h>
#include <OutAdjustPanel.h>
#include <RecorderDlg.h>
#include <RenderCanvas.h>
#include <RenderFrame.h>
#include <RenderviewPanel.h>
#include <SettingDlg.h>
#include <TrackDlg.h>
#include <TreePanel.h>
#include <VolumePropPanel.h>

#define ADD_AFTER_EVENT(obj, name, agt, funct) \
	obj->setValueChangedFunction(name, std::bind(&agt::funct, obj, std::placeholders::_1))

using namespace fluo;

AgentFactory::AgentFactory()
{
	m_name = gstAgentFactory;
}

AgentFactory::~AgentFactory()
{

}

AnnotationPropAgent* AgentFactory::addAnnotationPropAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<AnnotationPropAgent*>(result);

	//not found
	AnnotationPropAgent* agent =
		new AnnotationPropAgent(static_cast<AnnotationPropPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

BrushToolAgent* AgentFactory::addBrushToolAgent(const std::string &name, wxWindow &window, wxWindow &window2)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<BrushToolAgent*>(result);

	//not found
	BrushToolAgent* agent =
		new BrushToolAgent(static_cast<BrushToolDlg&>(window), static_cast<TreePanel&>(window2));
	if (agent)
	{
		agent->setName(name);
		ADD_AFTER_EVENT(agent, gstInterMode, BrushToolAgent, OnInterModeChanged);
		ADD_AFTER_EVENT(agent, gstSelUndo, BrushToolAgent, OnSelUndo);
		ADD_AFTER_EVENT(agent, gstSelRedo, BrushToolAgent, OnSelRedo);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

CalculationAgent* AgentFactory::addCalculationAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<CalculationAgent*>(result);

	//not found
	CalculationAgent* agent =
		new CalculationAgent(static_cast<CalculationDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ClipPlaneAgent* AgentFactory::addClipPlaneAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ClipPlaneAgent*>(result);

	//not found
	ClipPlaneAgent* agent =
		new ClipPlaneAgent(static_cast<ClipPlanePanel&>(window));
	if (agent)
	{
		agent->setName(name);
		//own values
		agent->addRvalu(gstVolumeA, (Referenced*)(0));
		agent->addRvalu(gstVolumeB, (Referenced*)(0));
		ADD_AFTER_EVENT(agent, gstClipX1, ClipPlaneAgent, OnClipXChanged);
		ADD_AFTER_EVENT(agent, gstClipX2, ClipPlaneAgent, OnClipXChanged);
		ADD_AFTER_EVENT(agent, gstClipY1, ClipPlaneAgent, OnClipYChanged);
		ADD_AFTER_EVENT(agent, gstClipY2, ClipPlaneAgent, OnClipYChanged);
		ADD_AFTER_EVENT(agent, gstClipZ1, ClipPlaneAgent, OnClipZChanged);
		ADD_AFTER_EVENT(agent, gstClipZ2, ClipPlaneAgent, OnClipZChanged);
		ADD_AFTER_EVENT(agent, gstClipDistX, ClipPlaneAgent, OnClipDistXChanged);
		ADD_AFTER_EVENT(agent, gstClipDistY, ClipPlaneAgent, OnClipDistYChanged);
		ADD_AFTER_EVENT(agent, gstClipDistZ, ClipPlaneAgent, OnClipDistZChanged);
		ADD_AFTER_EVENT(agent, gstClipLinkX, ClipPlaneAgent, OnClipLinkXChanged);
		ADD_AFTER_EVENT(agent, gstClipLinkY, ClipPlaneAgent, OnClipLinkYChanged);
		ADD_AFTER_EVENT(agent, gstClipLinkZ, ClipPlaneAgent, OnClipLinkZChanged);
		ADD_AFTER_EVENT(agent, gstClipRotX, ClipPlaneAgent, OnClipRotXChanged);
		ADD_AFTER_EVENT(agent, gstClipRotY, ClipPlaneAgent, OnClipRotYChanged);
		ADD_AFTER_EVENT(agent, gstClipRotZ, ClipPlaneAgent, OnClipRotZChanged);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ClKernelAgent* AgentFactory::addClKernelAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ClKernelAgent*>(result);

	//not found
	ClKernelAgent* agent =
		new ClKernelAgent(static_cast<ClKernelDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ColocalAgent* AgentFactory::addColocalAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ColocalAgent*>(result);

	//not found
	ColocalAgent* agent =
		new ColocalAgent(static_cast<ColocalDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		//additiona settings
		agent->addValue(gstUseSelection, false);
		agent->addValue(gstAutoUpdate, false);
		agent->addValue(gstHoldHistory, false);
		agent->addValue(gstColocalMethod, long(0));
		agent->addValue(gstIntWeighted, false);
		agent->addValue(gstGetRatio, false);
		agent->addValue(gstPhysSize, false);
		agent->addValue(gstColormapEnable, false);
		agent->addValue(gstColormapLow, double(0));
		agent->addValue(gstColormapHigh, double(1));
		ADD_AFTER_EVENT(agent, gstAutoUpdate, ColocalAgent, OnAutoUpdateChanged);
		ADD_AFTER_EVENT(agent, gstUseSelection, ColocalAgent, OnSettingChanged);
		ADD_AFTER_EVENT(agent, gstColocalMethod, ColocalAgent, OnAutoUpdateChanged);
		ADD_AFTER_EVENT(agent, gstIntWeighted, ColocalAgent, OnAutoUpdateChanged);
		ADD_AFTER_EVENT(agent, gstGetRatio, ColocalAgent, OnAutoUpdateChanged);
		ADD_AFTER_EVENT(agent, gstPhysSize, ColocalAgent, OnAutoUpdateChanged);
		ADD_AFTER_EVENT(agent, gstColormapEnable, ColocalAgent, OnAutoUpdateChanged);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ComponentAgent* AgentFactory::addComponentAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ComponentAgent*>(result);

	//not found
	ComponentAgent* agent =
		new ComponentAgent(static_cast<ComponentDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		agent->addValue(gstUseSelection, false);
		agent->addValue(gstAutoUpdate, false);
		agent->addValue(gstHoldHistory, false);
		agent->addValue(gstRecordCmd, false);
		agent->addValue(gstRunCmd, false);
		agent->addValue(gstIteration, long(0));
		agent->addValue(gstThreshold, double(0));
		agent->addValue(gstUseDistField, bool(false));
		agent->addValue(gstUseDiffusion, bool(false));
		agent->addValue(gstDistFieldStrength, double(0));
		agent->addValue(gstDistFieldFilterSize, long(0));
		agent->addValue(gstMaxDist, long(0));
		agent->addValue(gstDistFieldThresh, double(0));
		agent->addValue(gstDiffusionFalloff, double(0));
		agent->addValue(gstUseDensityField, bool(false));
		agent->addValue(gstDensityFieldThresh, double(0));
		agent->addValue(gstDensityVarThresh, double(0));
		agent->addValue(gstDensityWindowSize, long(0));
		agent->addValue(gstDensityStatsSize, long(0));
		agent->addValue(gstFixateEnable, bool(false));
		agent->addValue(gstFixateSize, long(0));
		agent->addValue(gstCleanEnable, bool(false));
		agent->addValue(gstCleanIteration, long(0));
		agent->addValue(gstCleanSize, long(0));
		agent->addValue(gstClusterMethod, long(0));
		agent->addValue(gstClusterNum, long(0));
		agent->addValue(gstClusterMaxIter, long(0));
		agent->addValue(gstClusterTol, double(0));
		agent->addValue(gstClusterSize, long(0));
		agent->addValue(gstClusterEps, double(0));
		ADD_AFTER_EVENT(agent, gstIteration, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstThreshold, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstUseDistField, ComponentAgent, OnUseDistField);
		ADD_AFTER_EVENT(agent, gstUseDiffusion, ComponentAgent, OnUseDiffusion);
		ADD_AFTER_EVENT(agent, gstDistFieldStrength, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstDistFieldFilterSize, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstMaxDist, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstDistFieldThresh, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstDiffusionFalloff, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstUseDensityField, ComponentAgent, OnUseDensityField);
		ADD_AFTER_EVENT(agent, gstDensityFieldThresh, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstDensityVarThresh, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstDensityWindowSize, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstDensityStatsSize, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstFixateEnable, ComponentAgent, OnFixateEnable);
		ADD_AFTER_EVENT(agent, gstFixateSize, ComponentAgent, OnFixateSize);
		ADD_AFTER_EVENT(agent, gstCleanEnable, ComponentAgent, OnCleanEnable);
		ADD_AFTER_EVENT(agent, gstCleanIteration, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstCleanSize, ComponentAgent, OnAutoUpdate);
		ADD_AFTER_EVENT(agent, gstClusterMethod, ComponentAgent, OnClusterMethod);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ConvertAgent* AgentFactory::addConvertAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ConvertAgent*>(result);

	//not found
	ConvertAgent* agent =
		new ConvertAgent(static_cast<ConvertDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

CountingAgent* AgentFactory::addCountingAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<CountingAgent*>(result);

	//not found
	CountingAgent* agent =
		new CountingAgent(static_cast<CountingDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ListModel* AgentFactory::addListModel(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ListModel*>(result);

	//not found
	ListModel* agent =
		new ListModel(static_cast<ListPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		agent->setNodeAddedFunction(
			std::bind(&ListModel::OnItemAdded,
				agent, std::placeholders::_1));
		agent->setNodeRemovedFunction(
			std::bind(&ListModel::OnItemRemoved,
				agent, std::placeholders::_1));
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

MeasureAgent* AgentFactory::addMeasureAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<MeasureAgent*>(result);

	//not found
	MeasureAgent* agent =
		new MeasureAgent(static_cast<MeasureDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

MeshPropAgent* AgentFactory::addMeshPropAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<MeshPropAgent*>(result);

	//not found
	MeshPropAgent* agent =
		new MeshPropAgent(static_cast<MeshPropPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

MeshTransAgent* AgentFactory::addMeshTransAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<MeshTransAgent*>(result);

	//not found
	MeshTransAgent* agent =
		new MeshTransAgent(static_cast<MeshTransPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

MovieAgent* AgentFactory::addMovieAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<MovieAgent*>(result);

	//not found
	MovieAgent* agent =
		new MovieAgent(static_cast<MoviePanel&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

NoiseReduceAgent* AgentFactory::addNoiseReduceAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<NoiseReduceAgent*>(result);

	//not found
	NoiseReduceAgent* agent =
		new NoiseReduceAgent(static_cast<NoiseReduceDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

OutAdjustAgent* AgentFactory::addOutAdjustAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<OutAdjustAgent*>(result);

	//not found
	OutAdjustAgent* agent =
		new OutAdjustAgent(static_cast<OutAdjustPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		ADD_AFTER_EVENT(agent, gstGammaR, OutAdjustAgent, OnGammaRChanged);
		ADD_AFTER_EVENT(agent, gstGammaG, OutAdjustAgent, OnGammaGChanged);
		ADD_AFTER_EVENT(agent, gstGammaB, OutAdjustAgent, OnGammaBChanged);
		ADD_AFTER_EVENT(agent, gstBrightnessR, OutAdjustAgent, OnBrightnessRChanged);
		ADD_AFTER_EVENT(agent, gstBrightnessG, OutAdjustAgent, OnBrightnessGChanged);
		ADD_AFTER_EVENT(agent, gstBrightnessB, OutAdjustAgent, OnBrightnessBChanged);
		ADD_AFTER_EVENT(agent, gstEqualizeR, OutAdjustAgent, OnEqualizeRChanged);
		ADD_AFTER_EVENT(agent, gstEqualizeG, OutAdjustAgent, OnEqualizeGChanged);
		ADD_AFTER_EVENT(agent, gstEqualizeB, OutAdjustAgent, OnEqualizeBChanged);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

RecorderAgent* AgentFactory::addRecorderAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<RecorderAgent*>(result);

	//not found
	RecorderAgent* agent =
		new RecorderAgent(static_cast<RecorderDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

RenderCanvasAgent* AgentFactory::addRenderCanvasAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<RenderCanvasAgent*>(result);

	//not found
	RenderCanvasAgent* agent =
		new RenderCanvasAgent(static_cast<RenderCanvas&>(window));
	if (agent)
	{
		agent->setName(name);
		ADD_AFTER_EVENT(agent, gstBounds, RenderCanvasAgent, OnBoundsChanged);
		ADD_AFTER_EVENT(agent, gstFocus, RenderCanvasAgent, OnFocusChanged);
		agent->setDefaultValueChangedFunction(
			std::bind(&RenderCanvasAgent::handleValueChanged,
				agent, std::placeholders::_1));
		agent->setNodeAddedFunction(
			std::bind(&RenderCanvasAgent::OnSceneChanged,
				agent, std::placeholders::_1));
		agent->setNodeRemovedFunction(
			std::bind(&RenderCanvasAgent::OnSceneChanged,
				agent, std::placeholders::_1));
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

RenderFrameAgent* AgentFactory::addRenderFrameAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<RenderFrameAgent*>(result);

	//not found
	RenderFrameAgent* agent =
		new RenderFrameAgent(static_cast<RenderFrame&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

RenderviewAgent* AgentFactory::addRenderviewAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<RenderviewAgent*>(result);

	//not found
	RenderviewAgent* agent =
		new RenderviewAgent(static_cast<RenderviewPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

SettingAgent* AgentFactory::addSettingAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<SettingAgent*>(result);

	//not found
	SettingAgent* agent =
		new SettingAgent(static_cast<SettingDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

TrackAgent* AgentFactory::addTrackAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<TrackAgent*>(result);

	//not found
	TrackAgent* agent =
		new TrackAgent(static_cast<TrackDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

TreeModel* AgentFactory::addTreeModel(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<TreeModel*>(result);

	//not found
	TreeModel* tree_model =
			new TreeModel(static_cast<TreePanel&>(window));
	if (tree_model)
	{
		tree_model->setName(name);
		//may need to consider how to handle remote/indirect value changes by names
		tree_model->setDefaultValueChangedFunction(
			std::bind(&TreeModel::OnDisplayChanged,
				tree_model, std::placeholders::_1));
		tree_model->setNodeAddedFunction(
			std::bind(&TreeModel::OnItemAdded,
				tree_model, std::placeholders::_1));
		tree_model->setNodeRemovedFunction(
			std::bind(&TreeModel::OnItemRemoved,
				tree_model, std::placeholders::_1));
		objects_.push_front(tree_model);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, tree_model);
		notifyObservers(event);
	}

	return tree_model;
}

VolumePropAgent* AgentFactory::addVolumePropAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<VolumePropAgent*>(result);

	//not found
	VolumePropAgent* agent =
			new VolumePropAgent(static_cast<VolumePropPanel&>(window));
	if (agent)
	{
		agent->setName(name);
		ADD_AFTER_EVENT(agent, gstLuminance, VolumePropAgent, OnLuminanceChanged);
		ADD_AFTER_EVENT(agent, gstColor, VolumePropAgent, OnColorChanged);
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

//get
AnnotationPropAgent* AgentFactory::getAnnotationPropAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<AnnotationPropAgent*>(result);
}

BrushToolAgent* AgentFactory::getBrushToolAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<BrushToolAgent*>(result);
}

CalculationAgent* AgentFactory::getCalculationAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<CalculationAgent*>(result);
}

ClipPlaneAgent* AgentFactory::getClipPlaneAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<ClipPlaneAgent*>(result);
}

ClKernelAgent* AgentFactory::getClKernelAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<ClKernelAgent*>(result);
}

ColocalAgent* AgentFactory::getColocalAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<ColocalAgent*>(result);
}

ComponentAgent* AgentFactory::getComponentAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<ComponentAgent*>(result);
}

ConvertAgent* AgentFactory::getConvertAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<ConvertAgent*>(result);
}

CountingAgent* AgentFactory::getCountingAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<CountingAgent*>(result);
}

ListModel* AgentFactory::getListModel(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<ListModel*>(result);
}

MeasureAgent* AgentFactory::getMeasureAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<MeasureAgent*>(result);
}

MeshPropAgent* AgentFactory::getMeshPropAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<MeshPropAgent*>(result);
}

MeshTransAgent* AgentFactory::getMeshTransAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<MeshTransAgent*>(result);
}

MovieAgent* AgentFactory::getMovieAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<MovieAgent*>(result);
}

NoiseReduceAgent* AgentFactory::getNoiseReduceAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<NoiseReduceAgent*>(result);
}

OutAdjustAgent* AgentFactory::getOutAdjustAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<OutAdjustAgent*>(result);
}

RecorderAgent* AgentFactory::getRecorderAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<RecorderAgent*>(result);
}

RenderCanvasAgent* AgentFactory::getRenderCanvasAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<RenderCanvasAgent*>(result);
}

RenderFrameAgent* AgentFactory::getRenderFrameAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<RenderFrameAgent*>(result);
}

RenderviewAgent* AgentFactory::getRenderviewAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<RenderviewAgent*>(result);
}

SettingAgent* AgentFactory::getSettingAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<SettingAgent*>(result);
}

TrackAgent* AgentFactory::getTrackAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<TrackAgent*>(result);
}

TreeModel* AgentFactory::getTreeModel(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<TreeModel*>(result);
}

VolumePropAgent* AgentFactory::getVolumePropAgent(const std::string &name)
{
	InterfaceAgent* result = findFirst(name);
	return dynamic_cast<VolumePropAgent*>(result);
}

