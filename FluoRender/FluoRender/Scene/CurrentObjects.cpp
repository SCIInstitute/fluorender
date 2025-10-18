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
#include <CurrentObjects.h>
#include <Global.h>
#include <DataManager.h>
#include <Root.h>
#include <RenderView.h>
#include <VolumeGroup.h>
#include <MeshGroup.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <AnnotData.h>
#include <VolumeSelector.h>
#include <CompGenerator.h>

void CurrentObjects::SetRenderView(const std::shared_ptr<RenderView>& view)
{
	render_view = view;
	vol_group.reset();
	mesh_group.reset();
	vol_data.reset();
	mesh_data.reset();
	ann_data.reset();
	if (auto view_ptr = render_view.lock())
	{
		view_ptr->m_cur_vol.reset();
	}
}

void CurrentObjects::SetVolumeGroup(const std::shared_ptr<VolumeGroup>& g)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	bool found = false;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto v = root->GetView(i);
		if (!v)
			continue;
		for (int j = 0; j < v->GetLayerNum(); ++j)
		{
			auto l = v->GetLayer(j);
			if (!l)
				continue;
			if (l->IsA() == 5)
			{
				auto group = std::dynamic_pointer_cast<VolumeGroup>(l);
				if (group == g)
				{
					render_view = v;
					found = true;
					break;
				}
			}
		}
	}
	vol_group = g;
	mesh_group.reset();
	vol_data.reset();
	mesh_data .reset();
	ann_data.reset();
	if (auto view_ptr = render_view.lock())
		view_ptr->m_cur_vol.reset();
}

void CurrentObjects::SetMeshGroup(const std::shared_ptr<MeshGroup>& g)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	bool found = false;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto v = root->GetView(i);
		if (!v)
			continue;
		for (int j = 0; j < v->GetLayerNum(); ++j)
		{
			auto l = v->GetLayer(j);
			if (!l)
				continue;
			if (l->IsA() == 6)
			{
				auto group = std::dynamic_pointer_cast<MeshGroup>(l);
				if (group == g)
				{
					render_view = v;
					found = true;
					break;
				}
			}
		}
	}
	vol_group.reset();
	mesh_group = g;
	vol_data.reset();
	mesh_data.reset();
	ann_data.reset();
	if (auto view_ptr = render_view.lock())
		view_ptr->m_cur_vol.reset();
}

void CurrentObjects::SetVolumeData(const std::shared_ptr<VolumeData>& vd)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	bool found = false;
	for (int i = 0; i < root->GetViewNum() && !found; ++i)
	{
		auto v = root->GetView(i);
		if (!v)
			continue;
		for (int j = 0; j < v->GetLayerNum() && !found; ++j)
		{
			auto l = v->GetLayer(j);
			if (!l)
				continue;
			if (l->IsA() == 2)
			{
				auto vd0 = std::dynamic_pointer_cast<VolumeData>(l);
				if (vd == vd0)
				{
					found = true;
					render_view = v;
					break;
				}
			}
			else if (l->IsA() == 5)
			{
				auto g = std::dynamic_pointer_cast<VolumeGroup>(l);
				for (int k = 0; k < g->GetVolumeNum(); ++k)
				{
					auto vd0 = g->GetVolumeData(k);
					if (vd == vd0)
					{
						found = true;
						render_view = v;
						vol_group = g;
						break;
					}
				}
			}
		}
	}
	vol_data = vd;
	mesh_data.reset();
	mesh_group.reset();
	ann_data.reset();
	if (auto view_ptr = render_view.lock())
		view_ptr->m_cur_vol = vd;
	glbin_vol_selector.SetVolume(vd);
	glbin_comp_generator.SetVolumeData(vd);
}

void CurrentObjects::SetMeshData(const std::shared_ptr<MeshData>& md)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	bool found = false;
	for (int i = 0; i < root->GetViewNum() && !found; ++i)
	{
		auto v = root->GetView(i);
		if (!v)
			continue;
		for (int j = 0; j < v->GetLayerNum() && !found; ++j)
		{
			auto l = v->GetLayer(j);
			if (!l)
				continue;
			if (l->IsA() == 3)
			{
				auto md0 = std::dynamic_pointer_cast<MeshData>(l);
				if (md == md0)
				{
					found = true;
					render_view = v;
					break;
				}
			}
			else if (l->IsA() == 6)
			{
				auto g = std::dynamic_pointer_cast<MeshGroup>(l);
				for (int k = 0; k < g->GetMeshNum(); ++k)
				{
					auto md0 = g->GetMeshData(k);
					if (md == md0)
					{
						found = true;
						render_view = v;
						mesh_group = g;
						break;
					}
				}
			}
		}
	}
	mesh_data = md;
	vol_group.reset();
	vol_data.reset();
	ann_data.reset();
	if (auto view_ptr = render_view.lock())
		view_ptr->m_cur_vol.reset();
}

void CurrentObjects::SetAnnotData(const std::shared_ptr<AnnotData>& ann)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	bool found = false;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto v = root->GetView(i);
		if (!v)
			continue;
		for (int j = 0; j < v->GetLayerNum(); ++j)
		{
			auto l = v->GetLayer(j);
			if (!l)
				continue;
			if (l->IsA() == 4)
			{
				auto a0 = std::dynamic_pointer_cast<AnnotData>(l);
				if (a0 == ann)
				{
					render_view = v;
					found = true;
					break;
				}
			}
		}
	}
	ann_data = ann;
	vol_group.reset();
	mesh_group.reset();
	vol_data.reset();
	mesh_data.reset();
	if (auto view_ptr = render_view.lock())
		view_ptr->m_cur_vol.reset();
}

void CurrentObjects::SetSel(const std::wstring& str)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	bool found = false;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto v = root->GetView(i);
		if (!v)
			continue;
		if (v->GetName() == str)
		{
			SetRenderView(v);
			return;
		}
		for (int j = 0; j < v->GetLayerNum(); ++j)
		{
			auto l = v->GetLayer(j);
			if (!l)
				continue;
			if (l->IsA() == 2)
			{
				SetVolumeData(std::dynamic_pointer_cast<VolumeData>(l));
				return;
			}
			else if (l->IsA() == 3)
			{
				SetMeshData(std::dynamic_pointer_cast<MeshData>(l));
				return;
			}
			else if (l->IsA() == 4)
			{
				SetAnnotData(std::dynamic_pointer_cast<AnnotData>(l));
				return;
			}
			else if (l->IsA() == 5)
			{
				auto g = std::dynamic_pointer_cast<VolumeGroup>(l);
				if (!g)
					continue;
				if (g->GetName() == str)
				{
					SetVolumeGroup(g);
					return;
				}
				for (int k = 0; k < g->GetVolumeNum(); ++k)
				{
					auto vd = g->GetVolumeData(k);
					if (vd && vd->GetName() == str)
					{
						SetVolumeData(vd);
						return;
					}
				}
			}
			else if (l->IsA() == 6)
			{
				auto g = std::dynamic_pointer_cast<MeshGroup>(l);
				if (!g)
					continue;
				if (g->GetName() == str)
				{
					SetMeshGroup(g);
					return;
				}
				for (int k = 0; k < g->GetMeshNum(); ++k)
				{
					auto md = g->GetMeshData(k);
					if (md && md->GetName() == str)
					{
						SetMeshData(md);
						return;
					}
				}
			}
		}
	}
}

int CurrentObjects::GetViewId(RenderView* view)
{
	Root* root = glbin_data_manager.GetRoot();
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto v = root->GetView(i);
		if (view)
		{
			if (v.get() == view)
				return i;
		}
		else
		{
			if (v == render_view.lock())
				return i;
		}
	}
	return -1;
}

int CurrentObjects::GetDrawingViewId()
{
	if (auto vptr = render_view_drawing.lock())
		return vptr->Id();
	return 0;
}

flrd::RulerList* CurrentObjects::GetRulerList()
{
	if (auto vptr = render_view.lock())
		return vptr->GetRulerList();
	return nullptr;
}

flrd::Ruler* CurrentObjects::GetRuler()
{
	if (auto vptr = render_view.lock())
		return vptr->GetCurRuler();
	return nullptr;
}

TrackGroup* CurrentObjects::GetTrackGroup()
{
	if (auto vptr = render_view.lock())
		return vptr->GetTrackGroup();
	return nullptr;
}
