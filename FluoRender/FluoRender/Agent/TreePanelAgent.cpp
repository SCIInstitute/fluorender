/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <TreePanelAgent.h>
#include <TreePanel.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <RenderView.h>
#include <VolumeSelector.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <DataManager.h>
#include <Root.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <AnnotData.h>
#include <VolumeGroup.h>
#include <MeshGroup.h>

TreePanelAgent::TreePanelAgent(
	TreePanel* panel) :
	Agent(panel)
{

}

TreePanel* TreePanelAgent::GetPanel() const
{
	return static_cast<TreePanel*>(GetOwner());
}

void TreePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty();

	bool rebuild_tree =
		update_all ||
		request.HasValue(gstTreeCtrl) ||
		request.HasValue(gstTreeLayerName);

	if (rebuild_tree)
	{
		auto treeData = BuildTreeData();
		panel->UpdateTree(treeData);

		auto selData = BuildTreeSelectionData();
		panel->UpdateTreeSelection(selData);
	}
	else
	{
		if (request.HasValue(gstTreeIcons))
		{
			auto iconData = BuildTreeIconData();
			panel->UpdateTreeIcons(iconData);
		}

		if (request.HasValue(gstTreeColors))
		{
			auto colorData = BuildTreeColorData();
			panel->UpdateTreeColors(colorData);
		}

		if (request.HasValue(gstCurrentSelect))
		{
			auto selData = BuildTreeSelectionData();
			panel->UpdateTreeSelection(selData);
		}
	}

	if (update_all || request.HasValue(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
		panel->UpdateFreehandToolState(int_mode, sel_mode, rul_mode);
	}
}

void TreePanelAgent::UpdateData(const UpdateRequest& request)
{

}

TreeUpdateData TreePanelAgent::BuildTreeData()
{
	TreeUpdateData data;

	data.root.id = 0;
	data.root.type = TreeNodeType::Root;
	data.root.name = L"Scene Graph";

	Root* root = glbin_data_manager.GetRoot();

	if (!root)
		return data;

	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);

		if (!view)
			continue;

		TreeItemData viewNode;

		BuildViewNode(view, viewNode);

		data.root.children.push_back(
			std::move(viewNode));
	}

	return data;
}

TreeIconUpdateData TreePanelAgent::BuildTreeIconData()
{
	TreeIconUpdateData data;

	Root* root = glbin_data_manager.GetRoot();

	if (!root)
		return data;

	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);

		if (!view)
			continue;

		// view visibility
		data.items.push_back(
			{
				GetNodeId(view),
				view->GetDraw()
			});

		for (int j = 0; j < view->GetLayerNum(); ++j)
		{
			auto layer = view->GetLayer(j);

			if (!layer)
				continue;

			CollectIconUpdates(layer, data);
		}
	}

	return data;
}

TreeColorUpdateData TreePanelAgent::BuildTreeColorData()
{
	TreeColorUpdateData data;

	Root* root = glbin_data_manager.GetRoot();

	if (!root)
		return data;

	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);

		if (!view)
			continue;

		for (int j = 0; j < view->GetLayerNum(); ++j)
		{
			auto layer = view->GetLayer(j);

			if (!layer)
				continue;

			CollectColorUpdates(layer, data);
		}
	}

	return data;
}

TreeSelectionData TreePanelAgent::BuildTreeSelectionData()
{
	TreeSelectionData data;

	switch (glbin_current.GetType())
	{
	case 1:
		if (auto view = glbin_current.render_view.lock())
			data.selectedId = GetNodeId(view);
		break;

	case 2:
		if (auto vd = glbin_current.vol_data.lock())
			data.selectedId = GetNodeId(vd);
		break;

	case 3:
		if (auto md = glbin_current.mesh_data.lock())
			data.selectedId = GetNodeId(md);
		break;

	case 4:
		if (auto ann = glbin_current.ann_data.lock())
			data.selectedId = GetNodeId(ann);
		break;

	case 5:
		if (auto group = glbin_current.vol_group.lock())
			data.selectedId = GetNodeId(group);
		break;

	case 6:
		if (auto group = glbin_current.mesh_group.lock())
			data.selectedId = GetNodeId(group);
		break;

	default:
		data.selectedId = 0;
		break;
	}

	return data;
}

void TreePanelAgent::BuildViewNode(
	std::shared_ptr<RenderView> view,
	TreeItemData& node)
{
	node.id = GetNodeId(view);

	node.type = TreeNodeType::View;

	node.name = view->GetName();

	node.visible = view->GetDraw();

	view->OrganizeLayers();

	for (int i = 0; i < view->GetLayerNum(); ++i)
	{
		auto layer = view->GetLayer(i);

		if (!layer)
			continue;

		TreeItemData child;

		BuildLayerNode(layer, child);

		node.children.push_back(
			std::move(child));
	}
}

void TreePanelAgent::BuildLayerNode(
	std::shared_ptr<TreeLayer> layer,
	TreeItemData& node)
{
	switch (layer->IsA())
	{
	case 2:
		BuildVolumeNode(
			std::dynamic_pointer_cast<VolumeData>(layer),
			node);
		break;

	case 3:
		BuildMeshNode(
			std::dynamic_pointer_cast<MeshData>(layer),
			node);
		break;

	case 4:
		BuildAnnotNode(
			std::dynamic_pointer_cast<AnnotData>(layer),
			node);
		break;

	case 5:
		BuildVolumeGroupNode(
			std::dynamic_pointer_cast<VolumeGroup>(layer),
			node);
		break;

	case 6:
		BuildMeshGroupNode(
			std::dynamic_pointer_cast<MeshGroup>(layer),
			node);
		break;
	}
}

void TreePanelAgent::BuildVolumeNode(
	std::shared_ptr<VolumeData> vd,
	TreeItemData& node)
{
	if (!vd)
		return;

	node.id = GetNodeId(vd);

	node.type = TreeNodeType::Volume;

	node.name = vd->GetName();

	node.visible = vd->GetDisp();

	auto c = vd->GetColor();

	node.color =
	{
		uint8_t(c.r() * 255),
		uint8_t(c.g() * 255),
		uint8_t(c.b() * 255)
	};
}

void TreePanelAgent::BuildMeshNode(
	std::shared_ptr<MeshData> md,
	TreeItemData& node)
{
	if (!md)
		return;

	node.id = GetNodeId(md);

	node.type = TreeNodeType::Mesh;

	node.name = md->GetName();

	node.visible = md->GetDisp();

	auto c = md->GetColor();

	node.color =
	{
		uint8_t(c.r() * 255),
		uint8_t(c.g() * 255),
		uint8_t(c.b() * 255)
	};
}

void TreePanelAgent::BuildAnnotNode(
	std::shared_ptr<AnnotData> ann,
	TreeItemData& node)
{
	if (!ann)
		return;

	node.id = GetNodeId(ann);

	node.type = TreeNodeType::Annotation;

	node.name = ann->GetName();

	node.visible = ann->GetDisp();

	auto c = ann->GetColor();

	node.color =
	{
		uint8_t(c.r() * 255),
		uint8_t(c.g() * 255),
		uint8_t(c.b() * 255)
	};
}

void TreePanelAgent::BuildVolumeGroupNode(
	std::shared_ptr<VolumeGroup> group,
	TreeItemData& node)
{
	if (!group)
		return;

	node.id = GetNodeId(group);

	node.type = TreeNodeType::VolumeGroup;

	node.name = group->GetName();

	node.visible = group->GetDisp();

	for (int i = 0; i < group->GetVolumeNum(); ++i)
	{
		auto vd = group->GetVolumeData(i);

		if (!vd)
			continue;

		TreeItemData child;

		BuildVolumeNode(vd, child);

		node.children.push_back(
			std::move(child));
	}
}

void TreePanelAgent::BuildMeshGroupNode(
	std::shared_ptr<MeshGroup> group,
	TreeItemData& node)
{
	if (!group)
		return;

	node.id = GetNodeId(group);

	node.type = TreeNodeType::MeshGroup;

	node.name = group->GetName();

	node.visible = group->GetDisp();

	for (int i = 0; i < group->GetMeshNum(); ++i)
	{
		auto md = group->GetMeshData(i);

		if (!md)
			continue;

		TreeItemData child;

		BuildMeshNode(md, child);

		node.children.push_back(
			std::move(child));
	}
}

void TreePanelAgent::CollectIconUpdates(
	std::shared_ptr<TreeLayer> layer,
	TreeIconUpdateData& data)
{
	switch (layer->IsA())
	{
	case 2: // volume
	{
		auto vd =
			std::dynamic_pointer_cast<VolumeData>(layer);

		if (!vd)
			break;

		data.items.push_back(
			{
				GetNodeId(vd),
				vd->GetDisp()
			});
	}
	break;

	case 3: // mesh
	{
		auto md =
			std::dynamic_pointer_cast<MeshData>(layer);

		if (!md)
			break;

		data.items.push_back(
			{
				GetNodeId(md),
				md->GetDisp()
			});
	}
	break;

	case 4: // annotation
	{
		auto ann =
			std::dynamic_pointer_cast<AnnotData>(layer);

		if (!ann)
			break;

		data.items.push_back(
			{
				GetNodeId(ann),
				ann->GetDisp()
			});
	}
	break;

	case 5: // volume group
	{
		auto group =
			std::dynamic_pointer_cast<VolumeGroup>(layer);

		if (!group)
			break;

		data.items.push_back(
			{
				GetNodeId(group),
				group->GetDisp()
			});

		for (int i = 0; i < group->GetVolumeNum(); ++i)
		{
			auto vd = group->GetVolumeData(i);

			if (!vd)
				continue;

			data.items.push_back(
				{
					GetNodeId(vd),
					vd->GetDisp()
				});
		}
	}
	break;

	case 6: // mesh group
	{
		auto group =
			std::dynamic_pointer_cast<MeshGroup>(layer);

		if (!group)
			break;

		data.items.push_back(
			{
				GetNodeId(group),
				group->GetDisp()
			});

		for (int i = 0; i < group->GetMeshNum(); ++i)
		{
			auto md = group->GetMeshData(i);

			if (!md)
				continue;

			data.items.push_back(
				{
					GetNodeId(md),
					md->GetDisp()
				});
		}
	}
	break;
	}
}

void TreePanelAgent::CollectColorUpdates(
	std::shared_ptr<TreeLayer> layer,
	TreeColorUpdateData& data)
{
	switch (layer->IsA())
	{
	case 2: // volume
	{
		auto vd =
			std::dynamic_pointer_cast<VolumeData>(layer);

		if (!vd)
			break;

		auto c = vd->GetColor();

		data.items.push_back(
			{
				GetNodeId(vd),
				{
					uint8_t(c.r() * 255),
					uint8_t(c.g() * 255),
					uint8_t(c.b() * 255)
				}
			});
	}
	break;

	case 3: // mesh
	{
		auto md =
			std::dynamic_pointer_cast<MeshData>(layer);

		if (!md)
			break;

		auto c = md->GetColor();

		data.items.push_back(
			{
				GetNodeId(md),
				{
					uint8_t(c.r() * 255),
					uint8_t(c.g() * 255),
					uint8_t(c.b() * 255)
				}
			});
	}
	break;

	case 4: // annotation
	{
		auto ann =
			std::dynamic_pointer_cast<AnnotData>(layer);

		if (!ann)
			break;

		auto c = ann->GetColor();

		data.items.push_back(
			{
				GetNodeId(ann),
				{
					uint8_t(c.r() * 255),
					uint8_t(c.g() * 255),
					uint8_t(c.b() * 255)
				}
			});
	}
	break;

	case 5: // volume group
	{
		auto group =
			std::dynamic_pointer_cast<VolumeGroup>(layer);

		if (!group)
			break;

		for (int i = 0; i < group->GetVolumeNum(); ++i)
		{
			auto vd = group->GetVolumeData(i);

			if (!vd)
				continue;

			auto c = vd->GetColor();

			data.items.push_back(
				{
					GetNodeId(vd),
					{
						uint8_t(c.r() * 255),
						uint8_t(c.g() * 255),
						uint8_t(c.b() * 255)
					}
				});
		}
	}
	break;

	case 6: // mesh group
	{
		auto group =
			std::dynamic_pointer_cast<MeshGroup>(layer);

		if (!group)
			break;

		for (int i = 0; i < group->GetMeshNum(); ++i)
		{
			auto md = group->GetMeshData(i);

			if (!md)
				continue;

			auto c = md->GetColor();

			data.items.push_back(
				{
					GetNodeId(md),
					{
						uint8_t(c.r() * 255),
						uint8_t(c.g() * 255),
						uint8_t(c.b() * 255)
					}
				});
		}
	}
	break;
	}
}

void TreePanelAgent::Select()
{
	if (!m_datatree)
		return;

	wxTreeItemId sel_item = m_datatree->GetSelection();

	if (!sel_item.IsOk())
		return;

	fluo::ValueCollection vc;

	//select data
	std::wstring name = m_datatree->GetItemText(sel_item).ToStdWstring();
	LayerInfo* item_data = (LayerInfo*)m_datatree->GetItemData(sel_item);
	Root* root = glbin_data_manager.GetRoot();

	if (item_data)
	{
		switch (item_data->type)
		{
		case 0://root
			glbin_current.SetRoot();
			break;
		case 1://view
		{
			if (root)
			{
				auto view = root->GetView(name);
				glbin_current.SetRenderView(view);
			}
		}
		break;
		case 2://volume data
		{
			auto vd = glbin_data_manager.GetVolumeData(name);
			glbin_current.SetVolumeData(vd);
			vc.insert(gstVolumePropPanel);
		}
		break;
		case 3://mesh data
		{
			auto md = glbin_data_manager.GetMeshData(name);
			glbin_current.SetMeshData(md);
			vc.insert(gstMeshPropPanel);
		}
		break;
		case 4://annotations
		{
			auto ann = glbin_data_manager.GetAnnotData(name);
			glbin_current.SetAnnotData(ann);
			vc.insert(gstAnnotatPropPanel);
		}
		break;
		case 5://volume group
		{
			std::wstring par_name = m_datatree->GetItemText(m_datatree->GetItemParent(sel_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(par_name);
				if (view)
				{
					auto group = view->GetGroup(name);
					glbin_current.SetVolumeGroup(group);
				}
			}
		}
		break;
		case 6://mesh group
		{
			std::wstring par_name = m_datatree->GetItemText(m_datatree->GetItemParent(sel_item)).ToStdWstring();
			if (root)
			{
				auto view = root->GetView(par_name);
				if (view)
				{
					auto group = view->GetMGroup(name);
					glbin_current.SetMeshGroup(group);
				}
			}
		}
		break;
		}
	}

	vc.insert({ gstCurrentSelect, gstUpdateSync });
	FluoRefresh(1, { vc });
}

void TreePanelAgent::Action()
{
	bool bval = wxGetKeyState(WXK_CONTROL);
	fluo::ValueCollection vc;

	if (bval)
	{
		RandomizeColor();
		vc.insert(gstTreeColors);
	}
	else
	{
		ToggleDisplay();
		vc.insert(gstTreeIcons);
	}
	FluoRefresh(2, vc);
}

void TreePanelAgent::AddVolGroup()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name = view->AddGroup(L"");
	auto group = view->GetGroup(name);
	glbin_current.SetVolumeGroup(group);

	FluoRefresh(0, { gstTreeCtrl, gstCurrentSelect }, { -1 });
}

void TreePanelAgent::AddMeshGroup()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name = view->AddMGroup(L"");
	auto group = view->GetMGroup(name);
	glbin_current.SetMeshGroup(group);

	FluoRefresh(0, { gstTreeCtrl, gstCurrentSelect }, { -1 });
}

void TreePanelAgent::RemoveData()
{
	DeleteSelection();
	glbin_current.SetRoot();

	FluoRefresh(0, { gstTreeCtrl, gstCurrentSelect });
}

void TreePanelAgent::RulerLocator()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void TreePanelAgent::RulerLine()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Line);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void TreePanelAgent::RulerPolyline()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Polyline);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void TreePanelAgent::RulerPencil()
{
	glbin_states.ToggleIntMode(InteractiveMode::Pencil);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void TreePanelAgent::RulerEdit()
{
	glbin_states.ToggleIntMode(InteractiveMode::EditRulerPoint);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void TreePanelAgent::RulerDeletePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerDelPoint);
	FluoRefresh(0, { gstFreehandToolState }, { -1 });
}

void TreePanelAgent::BrushRuler()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::SingleSelect);
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void TreePanelAgent::BrushGrow()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Grow);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void TreePanelAgent::BrushAppend()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Append);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void TreePanelAgent::BrushComp()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Segment);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void TreePanelAgent::BrushDiffuse()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Diffuse);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void TreePanelAgent::BrushUnselect()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Eraser);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, { -1 });
}

void TreePanelAgent::BrushClear()
{
	glbin_vol_selector.Clear();
	FluoRefresh(3, { gstNull });
}

void TreePanelAgent::BrushExtract()
{
	glbin_vol_selector.Extract();
	FluoRefresh(0, { gstTreeCtrl, gstTreeSelection });
}

void TreePanelAgent::BrushDelete()
{
	glbin_vol_selector.Erase();
	FluoRefresh(3, { gstTreeCtrl, gstTreeSelection });
}

void TreePanelAgent::MeshConvert()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	glbin_conv_vol_mesh.SetVolumeData(vd);
	glbin_conv_vol_mesh.Update(true);
	auto md = glbin_conv_vol_mesh.GetMeshData();
	if (md)
	{
		auto temp = glbin_data_manager.GetMeshData(md->GetName());
		if (!temp)
		{
			glbin_data_manager.AddMeshData(md);
			auto view = glbin_current.render_view.lock();
			if (view)
				view->AddMeshData(md);
		}
	}
	if (!glbin_conv_vol_mesh.GetMerged())
		glbin_conv_vol_mesh.MergeVertices(false);
	glbin_conv_vol_mesh.Smooth(true);

	FluoRefresh(0, { gstVolMeshInfo, gstBrushThreshold, gstCompThreshold, gstVolMeshThresh, gstListCtrl, gstTreeCtrl },
		{ glbin_current.GetViewId() });
}

void TreePanelAgent::DeleteSelection()
{
	int type = glbin_current.GetType();
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;

	switch (type)
	{
	case 1://view
	{
		std::wstring view0_name = root->GetView(0)->GetName();
		std::wstring name = view->GetName();
		if (name != view0_name)
		{
			m_frame->DeleteRenderViewPanel(name);
			root->DeleteView(name);
		}
	}
	break;
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			vd->SetDisp(true);
			std::wstring name = vd->GetName();
			view->RemoveVolumeData(name);
		}
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
		{
			md->SetDisp(true);
			std::wstring name = md->GetName();
			view->RemoveMeshData(name);
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
		{
			ann->SetDisp(true);
			std::wstring name = ann->GetName();
			view->RemoveAnnotData(name);
		}
	}
	break;
	case 5://volume group
	{
		auto group = glbin_current.vol_group.lock();
		if (group)
		{
			std::wstring name = group->GetName();
			view->RemoveGroup(name);
		}
	}
	break;
	case 6://mesh group
	{
		auto group = glbin_current.mesh_group.lock();
		if (group)
		{
			std::wstring name = group->GetName();
			view->RemoveGroup(name);
		}
	}
	}

	glbin_current.SetRoot();

	FluoRefresh(0, { gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex });
}

//delete
void TreePanelAgent::DeleteAll()
{
	//delete all views other than the first one
	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (int i = root->GetViewNum(); i > 1; --i)
		{
			m_frame->DeleteRenderViewPanel(i - 1);
			root->DeleteView(i - 1);
		}
	}

	root->GetView(0)->ClearAll();

	glbin_current.SetRoot();

	FluoRefresh(0, { gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex });
}

void TreePanelAgent::Expand()
{
	wxTreeItemId sel_item = m_datatree->GetSelection();
	if (m_datatree->IsExpanded(sel_item))
		m_datatree->Collapse(sel_item);
	else
		m_datatree->Expand(sel_item);
}

void TreePanelAgent::ToggleDisplay()
{
	int type = glbin_current.GetType();

	switch (type)
	{
	case 1://view
	{
		//view
		auto view = glbin_current.render_view.lock();
		if (view)
			view->ToggleDraw();
	}
	break;
	case 2://volume data
	{
		//volume
		auto view = glbin_current.render_view.lock();
		auto vd = glbin_current.vol_data.lock();
		if (view && vd)
		{
			vd->ToggleDisp();
			view->SetVolPopDirty();
		}
	}
	break;
	case 3://mesh data
	{
		//mesh
		auto view = glbin_current.render_view.lock();
		auto md = glbin_current.mesh_data.lock();
		if (view && md)
		{
			md->ToggleDisp();
			view->SetMeshPopDirty();
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
			ann->ToggleDisp();
	}
	break;
	case 5://volume group
	{
		//volume group
		auto view = glbin_current.render_view.lock();
		auto group = glbin_current.vol_group.lock();
		if (view && group)
		{
			group->ToggleDisp();
			view->SetVolPopDirty();
		}
	}
	break;
	case 6://mesh group
	{
		//mesh group
		auto view = glbin_current.render_view.lock();
		auto group = glbin_current.mesh_group.lock();
		if (view && group)
		{
			group->ToggleDisp();
			view->SetMeshPopDirty();
		}
	}
	}

	m_scroll_pos = GetScrollPos(wxVERTICAL);
	SetScrollPos(wxVERTICAL, m_scroll_pos);

	FluoRefresh(0, { gstTreeIcons });
}

void TreePanelAgent::RandomizeColor()
{
	int type = glbin_current.GetType();

	switch (type)
	{
	case 1://view
	{
		//view
		auto view = glbin_current.render_view.lock();
		if (view)
			view->RandomizeColor();
	}
	break;
	case 2://volume data
	{
		//volume
		auto vd = glbin_current.vol_data.lock();
		if (vd)
			vd->RandomizeColor();
	}
	break;
	case 3://mesh data
	{
		//mesh
		auto md = glbin_current.mesh_data.lock();
		if (md)
			md->RandomizeColor();
	}
	break;
	case 5://volume group
	{
		//volume group
		auto group = glbin_current.vol_group.lock();
		if (group)
			group->RandomizeColor();
	}
	break;
	case 6://mesh group
	{
		//mesh group
		auto group = glbin_current.mesh_group.lock();
		if (group)
			group->RandomizeColor();
	}
	}

	FluoRefresh(0, { gstTreeIcons });
}

void TreePanelAgent::CloseView()
{
	auto view = glbin_current.render_view.lock();
	if (view)
	{
		std::wstring name = view->GetName();
		m_frame->DeleteRenderViewPanel(name);
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->DeleteView(name);
	}

	glbin_current.SetRoot();
	FluoRefresh(0, { gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex });
}

void TreePanelAgent::Isolate()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name;
	int type = glbin_current.GetType();
	switch (type)
	{
	case 1://view
		break;
	case 2://volume
		if (auto cur_vd = glbin_current.vol_data.lock())
			name = cur_vd->GetName();
		break;
	case 3://mesh
		if (auto cur_md = glbin_current.mesh_data.lock())
			name = cur_md->GetName();
		break;
	case 4://annotations
		if (auto cur_ann = glbin_current.ann_data.lock())
			name = cur_ann->GetName();
		break;
	case 5://volume group
		if (auto cur_group = glbin_current.vol_group.lock())
			name = cur_group->GetName();
		break;
	case 6://mesh group
		if (auto cur_group = glbin_current.mesh_group.lock())
			name = cur_group->GetName();
		break;
	}

	view->Isolate(type, name);
	FluoRefresh(2, { gstTreeIcons });
}

void TreePanelAgent::ShowAll()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	view->ShowAll();
	FluoRefresh(2, { gstTreeIcons });
}

void TreePanelAgent::ManipulateData()
{
	m_frame->UpdateProps({ gstManipPropPanel });
}

