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
#include <Coordinator.h>
#include <GlobalStates.h>
#include <ConvVolMesh.h>

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

	if (request.HasValue(gstTreeContextMenu))
		ShowContextMenu();

	if (request.HasValue(gstTreeExpandSelItem))
		panel->UpdateExpandSelectedItem();

	if (request.HasValue(gstTreeScrollPos))
		panel->UpdateScrollPos();
}

void TreePanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstTreeAction))
		Action();
	if (request.HasValue(gstAddVolumeGroup))
		AddVolGroup();
	if (request.HasValue(gstAddMeshGroup))
		AddMeshGroup();
	if (request.HasValue(gstRemoveData))
		RemoveData();
	if (request.HasValue(gstRulerLocator))
		RulerLocator();
	if (request.HasValue(gstRulerLine))
		RulerLine();
	if (request.HasValue(gstRulerPolyline))
		RulerPolyline();
	if (request.HasValue(gstRulerPencil))
		RulerPencil();
	if (request.HasValue(gstRulerMovePoint))
		RulerEdit();
	if (request.HasValue(gstRulerDeletePoint))
		RulerDeletePoint();
	if (request.HasValue(gstBrushLocator))
		BrushRuler();
	if (request.HasValue(gstBrushGrow))
		BrushGrow();
	if (request.HasValue(gstBrushAppend))
		BrushAppend();
	if (request.HasValue(gstMeshConvert))
		MeshConvert();
	if (request.HasValue(gstBrushComp))
		BrushComp();
	if (request.HasValue(gstBrushDiffuse))
		BrushDiffuse();
	if (request.HasValue(gstBrushUnsel))
		BrushUnselect();
	if (request.HasValue(gstBrushClear))
		BrushClear();
	if (request.HasValue(gstBrushExtract))
		BrushExtract();
	if (request.HasValue(gstBrushDelete))
		BrushDelete();

	if (request.HasValue(gstRandomizeColor))
		RandomizeColor();
	if (request.HasValue(gstCloseView))
		CloseView();
	if (request.HasValue(gstIsolate))
		Isolate();
	if (request.HasValue(gstShowAll))
		ShowAll();
	if (request.HasValue(gstCopyMask))
		CopyMask();
	if (request.HasValue(gstPasteMask))
		PasteMask(0);
	if (request.HasValue(gstMergeMask))
		PasteMask(1);
	if (request.HasValue(gstExcludeMask))
		PasteMask(2);
	if (request.HasValue(gstIntersectMask))
		PasteMask(3);

	if (request.HasValue(gstTreeExpandSelItem))
		UpdateDataToUI({ gstTreeExpandSelItem });
	if (request.HasValue(gstBrushToolDlg))
		NotifyDataToUI({ gstBrushToolDlg });
	if (request.HasValue(gstMeasureDlg))
		NotifyDataToUI({ gstMeasureDlg });
	if (request.HasValue(gstComponentDlg))
		NotifyDataToUI({ gstComponentDlg });
	if (request.HasValue(gstTrackDlg))
		NotifyDataToUI({ gstTrackDlg });
	if (request.HasValue(gstCalculationDlg))
		NotifyDataToUI({ gstCalculationDlg });
	if (request.HasValue(gstNoiseCancellingDlg))
		NotifyDataToUI({ gstNoiseCancellingDlg });
	if (request.HasValue(gstCountingDlg))
		NotifyDataToUI({ gstCountingDlg });
	if (request.HasValue(gstColocalizationDlg))
		NotifyDataToUI({ gstColocalizationDlg });
	if (request.HasValue(gstConvertDlg))
		NotifyDataToUI({ gstConvertDlg });
	if (request.HasValue(gstOclDlg))
		NotifyDataToUI({ gstOclDlg });
	if (request.HasValue(gstMachineLearningDlg))
		NotifyDataToUI({ gstMachineLearningDlg });
	if (request.HasValue(gstManipPropPanel))
		NotifyDataToUI({ gstManipPropPanel });

	if (request.HasValue(gstTreeSelection))
		Select();
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

void TreePanelAgent::ShowContextMenu()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	MenuData data;
	int type = glbin_current.GetType();
	bool expanded = panel->GetTreeExpanded();
	switch (type)
	{
	case 0:  //root
		if (expanded)
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Collapse"});
		else
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Expand"});
		break;
	case 1:  //view
	{
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ToggleDisp,
			L"Toggle Visibility"});
		if (expanded)
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Collapse" });
		else
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Expand" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RandomizeColor,
			L"Randomize Colors" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_AddVolGroup,
			L"Add Volume Group" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_AddMeshGroup,
			L"Add Mesh Group" });
		Root* root = glbin_data_manager.GetRoot();
		std::wstring view_name;
		if (root)
			view_name = root->GetView(0)->GetName();
		if (panel->GetSelItemText() != view_name)
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_CloseView,
				L"Close" });
	}
	break;
	case 2:  //volume data
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ToggleDisp,
			L"Toggle Visibility" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Isolate,
			L"Isolate" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ShowAll,
			L"Show All" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RandomizeColor,
			L"Randomize Colors" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_AddVolGroup,
			L"Add Volume Group" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RemoveData,
			L"Delete" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_CopyMask,
			L"Copy Mask" });
		if (glbin_vol_selector.GetCopyMaskVolume())
		{
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_PasteMask,
				L"Paste Mask" });
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_MergeMask,
				L"Merge Mask" });
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_ExcludeMask,
				L"Exclude Mask" });
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_IntersectMask,
				L"Intersect Mask" });
		}
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Ocl,
			L"Volume Filter..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Brush,
			L"Paint Brush..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Measurement,
			L"Measurement..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Component,
			L"Component Analyzer..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Track,
			L"Tracking..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Calculation,
			L"Calculations..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_NoiseReduct,
			L"Noise Reduction..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_VolumeSize,
			L"Volume Size..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Colocalization,
			L"Colocalization..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Convert,
			L"Convert..." });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_MachineLearning,
			L"Machine Learning Manager..." });
		break;
	case 3:  //mesh data
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ToggleDisp,
			L"Toggle Visibility" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Isolate,
			L"Isolate" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ShowAll,
			L"Show All" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RandomizeColor,
			L"Randomize Colors" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_AddMeshGroup,
			L"Add Mesh Group" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RemoveData,
			L"Delete" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ManipulateData,
			L"Manipulate" });
		break;
	case 4:  //annotations
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ToggleDisp,
			L"Toggle Visibility" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RemoveData,
			L"Delete" });
		break;
	case 5:  //data group
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ToggleDisp,
			L"Toggle Visibility" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Isolate,
			L"Isolate" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ShowAll,
			L"Show All" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		if (expanded)
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Collapse" });
		else
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Expand" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RandomizeColor,
			L"Randomize Colors" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_AddVolGroup,
			L"Add Volume Group" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RemoveData,
			L"Delete" });
		break;
	case 6:  //mesh group
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ToggleDisp,
			L"Toggle Visibility" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_Isolate,
			L"Isolate" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_ShowAll,
			L"Show All" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		if (expanded)
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Collapse" });
		else
			data.push_back(MenuItemData{
				MenuItemData::Type::Action,
				TreePanel::ID_Expand,
				L"Expand" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Separator });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RandomizeColor,
			L"Randomize Colors" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_AddMeshGroup,
			L"Add Mesh Group" });
		data.push_back(MenuItemData{
			MenuItemData::Type::Action,
			TreePanel::ID_RemoveData,
			L"Delete" });
		break;
	}
	panel->ShowContextMenu(data);
}

void TreePanelAgent::Action()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool bval = panel->GetCtrlDown();
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
	std::set<Agent*> target{ this };
	auto view = glbin_current.render_view.lock();
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void TreePanelAgent::AddVolGroup()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name = view->AddGroup(L"");
	auto group = view->GetGroup(name);
	glbin_current.SetVolumeGroup(group);

	NotifyDataToUI({ gstTreeCtrl, gstCurrentSelect });
}

void TreePanelAgent::AddMeshGroup()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	std::wstring name = view->AddMGroup(L"");
	auto group = view->GetMGroup(name);
	glbin_current.SetMeshGroup(group);

	NotifyDataToUI({ gstTreeCtrl, gstCurrentSelect });
}

void TreePanelAgent::RemoveData()
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
			//moved to request for sync panel with scenegraph
			//m_frame->DeleteRenderViewPanel(name);
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

	NotifyViewUpdate({ gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex, gstRenderViewPanel });
}

void TreePanelAgent::RulerLocator()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	NotifyDataToUI({ gstFreehandToolState });
}

void TreePanelAgent::RulerLine()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Line);
	NotifyDataToUI({ gstFreehandToolState });
}

void TreePanelAgent::RulerPolyline()
{
	glbin_states.ToggleRulerMode(flrd::RulerMode::Polyline);
	NotifyDataToUI({ gstFreehandToolState });
}

void TreePanelAgent::RulerPencil()
{
	glbin_states.ToggleIntMode(InteractiveMode::Pencil);
	NotifyDataToUI({ gstFreehandToolState });
}

void TreePanelAgent::RulerEdit()
{
	glbin_states.ToggleIntMode(InteractiveMode::EditRulerPoint);
	NotifyDataToUI({ gstFreehandToolState });
}

void TreePanelAgent::RulerDeletePoint()
{
	glbin_states.ToggleIntMode(InteractiveMode::RulerDelPoint);
	NotifyDataToUI({ gstFreehandToolState });
}

void TreePanelAgent::BrushRuler()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::SingleSelect);
	glbin_states.ToggleRulerMode(flrd::RulerMode::Locator);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void TreePanelAgent::BrushGrow()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Grow);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void TreePanelAgent::BrushAppend()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Append);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
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

	NotifyViewUpdate({ gstVolMeshInfo, gstBrushThreshold, gstCompThreshold, gstVolMeshThresh, gstListCtrl, gstTreeCtrl });
}

void TreePanelAgent::BrushComp()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Segment);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void TreePanelAgent::BrushDiffuse()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Diffuse);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void TreePanelAgent::BrushUnselect()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Eraser);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void TreePanelAgent::BrushClear()
{
	glbin_vol_selector.Clear();
	NotifyViewUpdate({ gstNull });
}

void TreePanelAgent::BrushExtract()
{
	glbin_vol_selector.Extract();
	NotifyViewUpdate({ gstTreeCtrl, gstTreeSelection });
}

void TreePanelAgent::BrushDelete()
{
	glbin_vol_selector.Erase();
	NotifyViewUpdate({ gstTreeCtrl, gstTreeSelection });
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
			//m_frame->DeleteRenderViewPanel(i - 1);
			root->DeleteView(i - 1);
		}
	}

	root->GetView(0)->ClearAll();

	glbin_current.SetRoot();

	NotifyViewUpdate({ gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex, gstRenderViewPanel });
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

	NotifyViewUpdate({ gstTreeIcons, gstTreeScrollPos });
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

	NotifyViewUpdate({ gstTreeIcons });
}

void TreePanelAgent::CloseView()
{
	auto view = glbin_current.render_view.lock();
	if (view)
	{
		std::wstring name = view->GetName();
		//m_frame->DeleteRenderViewPanel(name);
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->DeleteView(name);
	}

	glbin_current.SetRoot();
	NotifyViewUpdate({ gstTreeCtrl, gstListCtrl, gstMovViewList, gstMovViewIndex, gstRenderViewPanel });
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
	NotifyViewUpdate({ gstTreeIcons });
}

void TreePanelAgent::ShowAll()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	view->ShowAll();
	NotifyViewUpdate({ gstTreeIcons });
}

void TreePanelAgent::CopyMask()
{
	glbin_vol_selector.CopyMask(false);
}

void TreePanelAgent::PasteMask(int ival)
{
	glbin_vol_selector.PasteMask(ival);
	NotifyViewUpdate({ gstBrushCountAutoUpdate, gstColocalAutoUpdate });
}

void TreePanelAgent::Select()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	//select data
	auto name = panel->GetSelItemText();
	LayerInfo* item_data = panel->GetSelItemData();
	Root* root = glbin_data_manager.GetRoot();

	fluo::ValueCollection vc;
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
			auto par_name = panel->GetSelItemParentText();
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
			auto par_name = panel->GetSelItemParentText();
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
	NotifyViewUpdate({ vc });
}

