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

#include <ListPanelAgent.h>
#include <ListPanel.h>
#include <Global.h>
#include <Names.h>
#include <DataManager.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <AnnotData.h>
#include <CurrentObjects.h>
#include <Root.h>
#include <RenderView.h>
#include <Coordinator.h>
#include <ModalDlg.h>
#include <MainSettings.h>

ListPanelAgent::ListPanelAgent(
	ListPanel* panel) :
	Agent(panel)
{

}

ListPanel* ListPanelAgent::GetPanel() const
{
	return static_cast<ListPanel*>(GetWindow());
}

void ListPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty();

	if (update_all || request.HasValue(gstListCtrl) || request.HasValue(gstTreeLayerName))
		UpdateList();

	if (update_all || request.HasValue(gstCurrentSelect))
		UpdateSelection();
}

void ListPanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstAddListSelToView))
	{
		AddSelectionToView();
	}
	if (request.HasValue(gstListSaveSelection))
	{
		SaveSelection();
	}
	if (request.HasValue(gstListBakeSelection))
	{
		BakeSelection();
	}
	if (request.HasValue(gstListSaveSelMask))
	{
		SaveSelMask();
	}
	if (request.HasValue(gstListDeleteSelection))
	{
		DeleteSelection();
	}
	if (request.HasValue(gstListDeleteAll))
	{
		DeleteAll();
	}
}

void ListPanelAgent::SetSelName(const std::wstring& name)
{
	std::wstring new_name = name;
	for (int i = 1; glbin_data_manager.CheckNames(new_name); i++)
		new_name = new_name + L"_" + std::to_wstring(i);
	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
			vd->SetName(new_name);
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
			md->SetName(new_name);
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
			ann->SetName(new_name);
	}
	break;
	}

	NotifyViewUpdate({ gstTreeLayerName });
}

void ListPanelAgent::SetCurrentSelection(ListItemType type, const std::wstring& name)
{
	switch (type)
	{
	case ListItemType::Volume:
		glbin_current.SetVolumeData(glbin_data_manager.GetVolumeData(name));
		break;
	case ListItemType::Mesh:
		glbin_current.SetMeshData(glbin_data_manager.GetMeshData(name));
		break;
	case ListItemType::Annot:
		glbin_current.SetAnnotData(glbin_data_manager.GetAnnotData(name));
	}
	NotifyViewUpdate({ gstCurrentSelect });
}

ListContextInfo ListPanelAgent::GetListContextInfo()
{
	ListContextInfo info{ ListItemType::Invalid, false };
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return info;
	int type = glbin_current.GetType();
	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			info.type = ListItemType::Volume;
			info.path_valid = !(vd->GetPath().empty());
		}
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
		{
			info.type = ListItemType::Mesh;
			info.path_valid = !(md->GetPath().empty());
		}
	}
	break;
	case 4://annotat
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
		{
			info.type = ListItemType::Annot;
			info.path_valid = !(ann->GetPath().empty());
		}
	}
	break;
	}
	return info;
}

std::vector<std::wstring> ListPanelAgent::GetViewNames()
{
	std::vector<std::wstring> list;
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return list;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);
		list.push_back(view->GetName());
	}
	return list;
}

void ListPanelAgent::UpdateList()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	m_suppress_event = true;

	panel->DeleteAllListItems();

	for (int i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
	{
		auto vd = glbin_data_manager.GetVolumeData(i);
		if (vd)
		{
			std::wstring name = vd->GetName();
			std::wstring path = vd->GetPath();
			panel->AppendListItem(ListItemType::Volume, name, path);
		}
	}

	for (int i = 0; i < glbin_data_manager.GetMeshNum(); i++)
	{
		auto md = glbin_data_manager.GetMeshData(i);
		if (md)
		{
			std::wstring name = md->GetName();
			std::wstring path = md->GetPath();
			panel->AppendListItem(ListItemType::Mesh, name, path);
		}
	}

	for (int i = 0; i < glbin_data_manager.GetAnnotNum(); i++)
	{
		auto ann = glbin_data_manager.GetAnnotData(i);
		if (ann)
		{
			std::wstring name = ann->GetName();
			std::wstring path = ann->GetPath();
			panel->AppendListItem(ListItemType::Annot, name, path);
		}
	}

	m_suppress_event = false;
}

void ListPanelAgent::UpdateSelection()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	int type = glbin_current.GetType();
	std::wstring name;
	ListItemType list_type = ListItemType::Invalid;
	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (vd)
			name = vd->GetName();
		list_type = ListItemType::Volume;
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (md)
			name = md->GetName();
		list_type = ListItemType::Mesh;
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (ann)
			name = ann->GetName();
		list_type = ListItemType::Annot;
	}
	break;
	}

	panel->SelectListItem(list_type, name);
}

void ListPanelAgent::AddSelectionToView()
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	fluo::ValueCollection vc;
	bool view_empty = true;
	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			break;

		std::wstring name = vd->GetName();
		auto vd_add = vd;

		for (int i = 0; i < root->GetViewNum(); ++i)
		{
			auto v = root->GetView(i);
			if (v && v->GetVolumeData(name))
			{
				vd_add = glbin_data_manager.DuplicateVolumeData(vd);
				break;
			}
		}

		int chan_num = view->GetAny();
		view_empty = chan_num > 0 ? false : view_empty;
		fluo::Color color(1.0, 1.0, 1.0);
		if (chan_num == 0)
			color = fluo::Color(1.0, 0.0, 0.0);
		else if (chan_num == 1)
			color = fluo::Color(0.0, 1.0, 0.0);
		else if (chan_num == 2)
			color = fluo::Color(0.0, 0.0, 1.0);

		if (chan_num >= 0 && chan_num < 3)
			vd_add->SetColor(color);

		auto group = view->AddVolumeData(vd_add);
		glbin_current.SetVolumeData(vd_add);
		if (view->GetChannelMixMode() == ChannelMixMode::Depth)
			vc.insert(gstUpdateSync);
		vc.insert(gstVolumePropPanel);
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			break;
		int chan_num = view->GetAny();
		view_empty = chan_num > 0 ? false : view_empty;
		view->AddMeshData(md);
		vc.insert(gstMeshPropPanel);
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			break;
		int chan_num = view->GetAny();
		view_empty = chan_num > 0 ? false : view_empty;
		view->AddAnnotData(ann);
		vc.insert(gstAnnotatPropPanel);
	}
	break;
	}

	//update
	if (view_empty)
		view->InitView(INIT_BOUNDS | INIT_CENTER | INIT_TRANSL | INIT_ROTATE);
	else
		view->InitView(INIT_BOUNDS | INIT_CENTER);
	vc.insert({ gstListCtrl, gstTreeCtrl, gstCurrentSelect });

	NotifyViewUpdate(vc, { glbin_coordinator.FindRenderCanvasAgent(view) });
}

void ListPanelAgent::SaveSelection()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			break;
		fluo::Quaternion q = vd->GetClippingBox().GetRotation();
		vd->SetResample(false);

		SaveVolumeOptions initial_options;

		initial_options.compress =
			glbin_settings.m_save_compress;

		initial_options.crop =
			glbin_settings.m_save_crop;

		initial_options.filter =
			glbin_settings.m_save_filter;

		if (auto vd = glbin_current.vol_data.lock())
		{
			initial_options.resize =
				vd->GetResample();

			auto sz =
				vd->GetResampledSize();

			initial_options.size_x = sz.intx();
			initial_options.size_y = sz.inty();
			initial_options.size_z = sz.intz();
		}

		SaveVolumeHook hook(initial_options);

		ModalDlg dlg(
			panel,
			"Save Volume Data",
			"",
			"",
			"Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"
			"Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"
			"Utah Nrrd file (*.nrrd)|*.nrrd",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		dlg.SetCustomizeHook(hook);

		if (dlg.ShowModal() != wxID_OK)
			break;

		const SaveVolumeOptions& selected_options =
			hook.GetOptions();

		// Persist settings.

		glbin_settings.m_save_compress =
			selected_options.compress;

		glbin_settings.m_save_crop =
			selected_options.crop;

		glbin_settings.m_save_filter =
			selected_options.filter;

		std::wstring filename =
			dlg.GetPath().ToStdWstring();

		// Apply resize settings if needed.
		if (selected_options.resize)
		{
			vd->SetResample(true);

			vd->SetResampledSize(
				fluo::Vector(
					selected_options.size_x,
					selected_options.size_y,
					selected_options.size_z));
		}
		else
		{
			vd->SetResample(false);
		}

		vd->Save(
			filename,
			dlg.GetFilterIndex(),
			3,
			false,
			selected_options.crop,
			selected_options.filter,
			false,
			selected_options.compress,
			selected_options.crop,
			fluo::Point(),
			q,
			fluo::Vector(),
			false);
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			break;
		ModalDlg fopendlg(
			panel, "Save Mesh Data", "", "",
			"OBJ file (*.obj)|*.obj",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int rval = fopendlg.ShowModal();

		if (rval == wxID_OK)
		{
			std::wstring filename = fopendlg.GetPath().ToStdWstring();

			md->Save(filename);
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			break;
		ModalDlg fopendlg(
			panel, "Save AnnotData", "", "",
			"Text file (*.txt)|*.txt",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int rval = fopendlg.ShowModal();

		if (rval == wxID_OK)
		{
			std::wstring filename = fopendlg.GetPath().ToStdWstring();

			ann->Save(filename);
		}
	}
	break;
	}

	UpdateDataToUI({ gstListCtrl });
}

void ListPanelAgent::BakeSelection()
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	SaveVolumeOptions initial_options;

	initial_options.compress =
		glbin_settings.m_save_compress;

	initial_options.crop =
		glbin_settings.m_save_crop;

	initial_options.filter =
		glbin_settings.m_save_filter;

	if (auto vd = glbin_current.vol_data.lock())
	{
		initial_options.resize =
			vd->GetResample();

		auto sz =
			vd->GetResampledSize();

		initial_options.size_x = sz.intx();
		initial_options.size_y = sz.inty();
		initial_options.size_z = sz.intz();
	}

	SaveVolumeHook hook(initial_options);

	ModalDlg dlg(
		panel, "Bake Volume Data", "", "",
		"Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
		"Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
		"Utah Nrrd file (*.nrrd)|*.nrrd",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	dlg.SetCustomizeHook(hook);

	if (dlg.ShowModal() != wxID_OK)
		return;

	const SaveVolumeOptions& selected_options =
		hook.GetOptions();

	// Persist settings.

	glbin_settings.m_save_compress =
		selected_options.compress;

	glbin_settings.m_save_crop =
		selected_options.crop;

	glbin_settings.m_save_filter =
		selected_options.filter;

	std::wstring filename =
		dlg.GetPath().ToStdWstring();

	// Apply resize settings if needed.
	if (selected_options.resize)
	{
		vd->SetResample(true);

		vd->SetResampledSize(
			fluo::Vector(
				selected_options.size_x,
				selected_options.size_y,
				selected_options.size_z));
	}
	else
	{
		vd->SetResample(false);
	}


	fluo::Quaternion q = vd->GetClippingBox().GetRotation();
	vd->Save(
		filename,
		dlg.GetFilterIndex(),
		3,
		false,
		selected_options.crop,
		selected_options.filter,
		true,
		selected_options.compress,
		selected_options.crop,
		fluo::Point(),
		q,
		fluo::Vector(),
		false);

	UpdateDataToUI({ gstListCtrl });
}

void ListPanelAgent::SaveSelMask()
{
	auto vd = glbin_current.vol_data.lock();
	if (vd)
	{
		vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
		vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
	}
}

void ListPanelAgent::DeleteSelection()
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	int type = glbin_current.GetType();

	switch (type)
	{
	case 2://volume
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			break;
		std::wstring name = vd->GetName();
		//from view
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
			{
				view->RemoveVolumeData(name);
			}
		}
		//from datamanager
		int index = glbin_data_manager.GetVolumeIndex(name);
		if (index != -1)
		{
			glbin_data_manager.RemoveVolumeData(index);
		}
	}
	break;
	case 3://mesh
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			break;
		std::wstring name = md->GetName();
		//from view
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
			{
				view->RemoveMeshData(name);
			}
		}
		//from datamanager
		int index = glbin_data_manager.GetMeshIndex(name);
		if (index != -1)
		{
			glbin_data_manager.RemoveMeshData(index);
		}
	}
	break;
	case 4://annotations
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			break;
		std::wstring name = ann->GetName();
		//from view
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			auto view = root->GetView(i);
			if (view)
				view->RemoveAnnotData(name);
		}
		//from datamanager
		int index = glbin_data_manager.GetAnnotIndex(name);
		if (index != -1)
			glbin_data_manager.RemoveAnnotData(index);
	}
	break;
	}

	glbin_current.SetRoot();

	NotifyViewUpdate({ gstTreeCtrl, gstListCtrl });
}

void ListPanelAgent::DeleteAll()
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	for (int i = 0; i < root->GetViewNum(); ++i)
	{
		auto view = root->GetView(i);
		if (view)
			view->ClearAll();
	}
	glbin_data_manager.ClearAll();
	glbin_current.SetRoot();

	NotifyViewUpdate({ gstTreeCtrl, gstListCtrl });
}

