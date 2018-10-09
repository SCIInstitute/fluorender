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

#include <Fui/AgentFactory.h>
#include <Fui/ListModel.h>
#include <Fui/TreeModel.h>
#include <Fui/TreePanel.h>
#include <Fui/VolumePropAgent.h>
#include <Fui/VolumePropPanel.h>
#include <Fui/RenderCanvasAgent.h>
#include <VRenderGLView.h>
#include <Fui/OutAdjustAgent.h>
#include <Fui/OutAdjustPanel.h>
#include <Fui/ClipPlaneAgent.h>
#include <Fui/ClipPlanePanel.h>
#include <Fui/MeshPropAgent.h>
#include <Fui/MeshPropPanel.h>

using namespace FUI;

AgentFactory::AgentFactory()
{
	m_name = "agent factory";
}

AgentFactory::~AgentFactory()
{

}

ListModel* AgentFactory::getOrAddListModel(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ListModel*>(result);

	//not found
	ListModel* list_model = new ListModel();
	if (list_model)
	{
		list_model->setName(name);
		objects_.push_front(list_model);
		notifyObserversNodeAdded(this, list_model);
	}

	return list_model;
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
		objects_.push_front(tree_model);
		notifyObserversNodeAdded(this, tree_model);
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
		volume_prop_agent->setAfterFunction("luminance",
			std::bind(&VolumePropAgent::OnLuminanceChanged, volume_prop_agent));
		volume_prop_agent->setAfterFunction("color",
			std::bind(&VolumePropAgent::OnColorChanged, volume_prop_agent));
		objects_.push_front(volume_prop_agent);
		notifyObserversNodeAdded(this, volume_prop_agent);
	}

	return volume_prop_agent;
}

RenderCanvasAgent* AgentFactory::getOrAddRenderCanvasAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<RenderCanvasAgent*>(result);

	//not found
	RenderCanvasAgent* render_canvas_agent =
		new RenderCanvasAgent(static_cast<VRenderGLView&>(window));
	if (render_canvas_agent)
	{
		render_canvas_agent->setName(name);
		objects_.push_front(render_canvas_agent);
		notifyObserversNodeAdded(this, render_canvas_agent);
	}

	return render_canvas_agent;
}

OutAdjustAgent* AgentFactory::getOrAddOutAdjustAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<OutAdjustAgent*>(result);

	//not found
	OutAdjustAgent* out_adjust_agent =
		new OutAdjustAgent(static_cast<OutAdjustPanel&>(window));
	if (out_adjust_agent)
	{
		out_adjust_agent->setName(name);
		objects_.push_front(out_adjust_agent);
		notifyObserversNodeAdded(this, out_adjust_agent);
	}

	return out_adjust_agent;
}

ClipPlaneAgent* AgentFactory::getOrAddClipPlaneAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<ClipPlaneAgent*>(result);

	//not found
	ClipPlaneAgent* clip_plane_agent =
		new ClipPlaneAgent(static_cast<ClipPlanePanel&>(window));
	if (clip_plane_agent)
	{
		clip_plane_agent->setName(name);
		objects_.push_front(clip_plane_agent);
		notifyObserversNodeAdded(this, clip_plane_agent);
	}

	return clip_plane_agent;
}

MeshPropAgent* AgentFactory::getOrAddMeshPropAgent(const std::string &name, wxWindow &window)
{
	InterfaceAgent* result = findFirst(name);
	if (result)
		return dynamic_cast<MeshPropAgent*>(result);

	//not found
	MeshPropAgent* mesh_prop_agent =
		new MeshPropAgent(static_cast<MeshPropPanel&>(window));
	if (mesh_prop_agent)
	{
		mesh_prop_agent->setName(name);
		objects_.push_front(mesh_prop_agent);
		notifyObserversNodeAdded(this, mesh_prop_agent);
	}

	return mesh_prop_agent;
}