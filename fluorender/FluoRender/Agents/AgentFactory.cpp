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

using namespace fluo;

AgentFactory::AgentFactory()
{
	m_name = gstAgentFactory;
}

AgentFactory::~AgentFactory()
{

}

AnnotationPropAgent* AgentFactory::getOrAddAnnotationPropAgent(const std::string &name, wxWindow &window)
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

BrushToolAgent* AgentFactory::getOrAddBrushToolAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<BrushToolAgent*>(result);

	//not found
	BrushToolAgent* agent =
		new BrushToolAgent(static_cast<BrushToolDlg&>(window));
	if (agent)
	{
		agent->setName(name);
		agent->setValueChangedFunction(gstInterMode,
			std::bind(&BrushToolAgent::OnInterModeChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstSelUndo,
			std::bind(&BrushToolAgent::OnSelUndo,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstSelRedo,
			std::bind(&BrushToolAgent::OnSelRedo,
				agent, std::placeholders::_1));
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

CalculationAgent* AgentFactory::getOrAddCalculationAgent(const std::string &name, wxWindow &window)
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

ClipPlaneAgent* AgentFactory::getOrAddClipPlaneAgent(const std::string &name, wxWindow &window)
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
		agent->setValueChangedFunction(gstClipX1,
			std::bind(&ClipPlaneAgent::OnClipXChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipX2,
			std::bind(&ClipPlaneAgent::OnClipXChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipY1,
			std::bind(&ClipPlaneAgent::OnClipYChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipY2,
			std::bind(&ClipPlaneAgent::OnClipYChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipZ1,
			std::bind(&ClipPlaneAgent::OnClipZChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipZ2,
			std::bind(&ClipPlaneAgent::OnClipZChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipDistX,
			std::bind(&ClipPlaneAgent::OnClipDistXChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipDistY,
			std::bind(&ClipPlaneAgent::OnClipDistYChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipDistZ,
			std::bind(&ClipPlaneAgent::OnClipDistZChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipLinkX,
			std::bind(&ClipPlaneAgent::OnClipLinkXChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipLinkY,
			std::bind(&ClipPlaneAgent::OnClipLinkYChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipLinkZ,
			std::bind(&ClipPlaneAgent::OnClipLinkZChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipRotX,
			std::bind(&ClipPlaneAgent::OnClipRotXChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipRotY,
			std::bind(&ClipPlaneAgent::OnClipRotYChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstClipRotZ,
			std::bind(&ClipPlaneAgent::OnClipRotZChanged,
				agent, std::placeholders::_1));
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ClKernelAgent* AgentFactory::getOrAddClKernelAgent(const std::string &name, wxWindow &window)
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

ColocalAgent* AgentFactory::getOrAddColocalAgent(const std::string &name, wxWindow &window)
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
		agent->setValueChangedFunction(gstAutoUpdate,
			std::bind(&ColocalAgent::OnAutoUpdateChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstUseSelection,
			std::bind(&ColocalAgent::OnSettingChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstColocalMethod,
			std::bind(&ColocalAgent::OnSettingChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstIntWeighted,
			std::bind(&ColocalAgent::OnSettingChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstGetRatio,
			std::bind(&ColocalAgent::OnSettingChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstPhysSize,
			std::bind(&ColocalAgent::OnSettingChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstColormapEnable,
			std::bind(&ColocalAgent::OnSettingChanged,
				agent, std::placeholders::_1));
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ComponentAgent* AgentFactory::getOrAddComponentAgent(const std::string &name, wxWindow &window)
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
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

ConvertAgent* AgentFactory::getOrAddConvertAgent(const std::string &name, wxWindow &window)
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

CountingAgent* AgentFactory::getOrAddCountingAgent(const std::string &name, wxWindow &window)
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

ListModel* AgentFactory::getOrAddListModel(const std::string &name, wxWindow &window)
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

MeasureAgent* AgentFactory::getOrAddMeasureAgent(const std::string &name, wxWindow &window)
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

MeshPropAgent* AgentFactory::getOrAddMeshPropAgent(const std::string &name, wxWindow &window)
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

MeshTransAgent* AgentFactory::getOrAddMeshTransAgent(const std::string &name, wxWindow &window)
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

MovieAgent* AgentFactory::getOrAddMovieAgent(const std::string &name, wxWindow &window)
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

NoiseReduceAgent* AgentFactory::getOrAddNoiseReduceAgent(const std::string &name, wxWindow &window)
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

OutAdjustAgent* AgentFactory::getOrAddOutAdjustAgent(const std::string &name, wxWindow &window)
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
		agent->setValueChangedFunction(gstGammaR,
			std::bind(&OutAdjustAgent::OnGammaRChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstGammaG,
			std::bind(&OutAdjustAgent::OnGammaGChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstGammaB,
			std::bind(&OutAdjustAgent::OnGammaBChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstBrightnessR,
			std::bind(&OutAdjustAgent::OnBrightnessRChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstBrightnessG,
			std::bind(&OutAdjustAgent::OnBrightnessGChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstBrightnessB,
			std::bind(&OutAdjustAgent::OnBrightnessBChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstEqualizeR,
			std::bind(&OutAdjustAgent::OnEqualizeRChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstEqualizeG,
			std::bind(&OutAdjustAgent::OnEqualizeGChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstEqualizeB,
			std::bind(&OutAdjustAgent::OnEqualizeBChanged,
				agent, std::placeholders::_1));
		objects_.push_front(agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, agent);
		notifyObservers(event);
	}

	return agent;
}

RecorderAgent* AgentFactory::getOrAddRecorderAgent(const std::string &name, wxWindow &window)
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

RenderCanvasAgent* AgentFactory::getOrAddRenderCanvasAgent(const std::string &name, wxWindow &window)
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
		agent->setValueChangedFunction(gstBounds,
			std::bind(&RenderCanvasAgent::OnBoundsChanged,
				agent, std::placeholders::_1));
		agent->setValueChangedFunction(gstFocus,
			std::bind(&RenderCanvasAgent::OnFocusChanged,
				agent, std::placeholders::_1));
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

RenderFrameAgent* AgentFactory::getOrAddRenderFrameAgent(const std::string &name, wxWindow &window)
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

RenderviewAgent* AgentFactory::getOrAddRenderviewAgent(const std::string &name, wxWindow &window)
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

SettingAgent* AgentFactory::getOrAddSettingAgent(const std::string &name, wxWindow &window)
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

TrackAgent* AgentFactory::getOrAddTrackAgent(const std::string &name, wxWindow &window)
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

TreeModel* AgentFactory::getOrAddTreeModel(const std::string &name, wxWindow &window)
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

VolumePropAgent* AgentFactory::getOrAddVolumePropAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<VolumePropAgent*>(result);

	//not found
	VolumePropAgent* volume_prop_agent =
			new VolumePropAgent(static_cast<VolumePropPanel&>(window));
	if (volume_prop_agent)
	{
		volume_prop_agent->setName(name);
		volume_prop_agent->setValueChangedFunction(gstLuminance,
			std::bind(&VolumePropAgent::OnLuminanceChanged,
				volume_prop_agent, std::placeholders::_1));
		volume_prop_agent->setValueChangedFunction(gstColor,
			std::bind(&VolumePropAgent::OnColorChanged,
				volume_prop_agent, std::placeholders::_1));
		objects_.push_front(volume_prop_agent);
		Event event;
		event.init(Event::EVENT_NODE_ADDED,
			this, volume_prop_agent);
		notifyObservers(event);
	}

	return volume_prop_agent;
}

