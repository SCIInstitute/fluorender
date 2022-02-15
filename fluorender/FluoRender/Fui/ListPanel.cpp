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
#include <ListPanel.h>
#include <VRenderFrame.h>
#include <Global.hpp>
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <Annotations.hpp>
#include <VolumeFactory.hpp>
#include <MeshFactory.hpp>
#include <AnnotationFactory.hpp>
#include <AgentFactory.hpp>
#include <png_resource.h>
#include <boost/algorithm/string.hpp>
#include <wx/valnum.h>
//resources
#include <img/icons.h>

BEGIN_EVENT_TABLE(ListPanel, wxPanel)
EVT_TOOL(ID_AddToView, ListPanel::OnAddToView)
EVT_TOOL(ID_Rename, ListPanel::OnRename)
EVT_TOOL(ID_Save, ListPanel::OnSave)
EVT_TOOL(ID_Bake, ListPanel::OnBake)
EVT_TOOL(ID_SaveMask, ListPanel::OnSaveMask)
EVT_TOOL(ID_Delete, ListPanel::OnDelete)
EVT_TOOL(ID_DeleteAll, ListPanel::OnDeleteAll)
EVT_DATAVIEW_ITEM_BEGIN_DRAG(ID_ListCtrl, ListPanel::OnBeginDrag)
EVT_DATAVIEW_ITEM_DROP_POSSIBLE(ID_ListCtrl, ListPanel::OnDropPossible)
EVT_DATAVIEW_ITEM_DROP(ID_ListCtrl, ListPanel::OnDrop)
EVT_DATAVIEW_COLUMN_SORTED(ID_ListCtrl, ListPanel::OnListSorted)
END_EVENT_TABLE()

ListPanel::ListPanel(
	wxWindow *parent,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size, style, name)
{
	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(view);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddTool(ID_AddToView, "Add to View",
		bitmap, "Add: Add the selected dataset to render view");
	bitmap = wxGetBitmapFromMemory(rename);
	m_toolbar->AddTool(ID_Rename, "Rename",
		bitmap, "Rename: Rename the selected dataset");
	bitmap = wxGetBitmapFromMemory(save);
	m_toolbar->AddTool(ID_Save, "Save As",
		bitmap, "Save: Save the selected volume dataset");
	bitmap = wxGetBitmapFromMemory(bake);
	m_toolbar->AddTool(ID_Bake, "Bake",
		bitmap, "Bake: Apply the volume properties and save");
	bitmap = wxGetBitmapFromMemory(save_mask);
	m_toolbar->AddTool(ID_SaveMask, "Save Mask",
		bitmap, "Save Mask: Save its mask to a file");
	bitmap = wxGetBitmapFromMemory(delet);
	m_toolbar->AddTool(ID_Delete, "Delete",
		bitmap, "Delete: Delete the selected dataset");
	bitmap = wxGetBitmapFromMemory(del_all);
	m_toolbar->AddTool(ID_DeleteAll, "Delete All",
		bitmap, "Delete All: Delete all datasets");
	m_toolbar->Realize();

	m_list_ctrl = new wxDataViewCtrl(this, ID_ListCtrl,
		wxDefaultPosition, wxDefaultSize,
		wxDV_MULTIPLE | wxDV_ROW_LINES);
	m_list_ctrl->EnableDragSource(wxDF_UNICODETEXT);
	m_list_ctrl->EnableDropTarget(wxDF_UNICODETEXT);
	m_list_ctrl->SetDoubleBuffered(true);
	m_list_ctrl->SetIndent(1);
	m_list_model = glbin_agtf->getOrAddListModel("ListPanel", *this);
	//append columns
	//name
	wxDataViewIconTextRenderer *itr =
		new wxDataViewIconTextRenderer();
	wxDataViewColumn *column0 =
		new wxDataViewColumn("Name", itr, 0, 120, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_list_ctrl->AppendColumn(column0);
	//type
	wxDataViewTextRenderer *tr = new wxDataViewTextRenderer("string");
	wxDataViewColumn *column1 =
		new wxDataViewColumn("Type", tr, 1, 80, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_list_ctrl->AppendColumn(column1);
	//path
	tr = new wxDataViewTextRenderer("string");
	wxDataViewColumn *column2 =
		new wxDataViewColumn("Path", tr, 2, 200, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_list_ctrl->AppendColumn(column2);
	m_list_ctrl->AllowMultiColumnSort(true);
	if (m_list_model)
	{
		m_list_model->setObject(0);
		m_list_ctrl->AssociateModel(m_list_model);
	}

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(m_list_ctrl, 1, wxEXPAND);
	SetSizer(sizer_v);
	Layout();
}

ListPanel::~ListPanel()
{
}

void ListPanel::OnAddToView(wxCommandEvent& event)
{
}

void ListPanel::OnRename(wxCommandEvent& event)
{
}

void ListPanel::OnSave(wxCommandEvent& event)
{
}

void ListPanel::OnBake(wxCommandEvent& event)
{
}

void ListPanel::OnSaveMask(wxCommandEvent& event)
{
}

void ListPanel::OnDelete(wxCommandEvent& event)
{
}

void ListPanel::OnDeleteAll(wxCommandEvent &event)
{
}

void ListPanel::OnBeginDrag(wxDataViewEvent &event)
{
	wxDataViewItem item(event.GetItem());
	fluo::Referenced* refd = static_cast<fluo::Referenced*>(item.GetID());
	if (!refd)
	{
		event.Veto();
		return;
	}
	fluo::Node* node = dynamic_cast<fluo::Node*>(refd);
	if (!node)
	{
		event.Veto();
		return;
	}
	//multiple selections
	wxDataViewItemArray sel;
	m_list_ctrl->GetSelections(sel);
	wxString names;
	for (auto it = sel.begin();
		it != sel.end(); ++it)
	{
		fluo::Node* sel_node = (fluo::Node*)it->GetID();
		if (sel_node)
		{
			names += sel_node->getName();
			if (it != std::prev(sel.end()))
				names += '\n';
		}
	}

	wxTextDataObject *wxobj = new wxTextDataObject;
	wxobj->SetText(names);
	event.SetDataObject(wxobj);
	event.SetDragFlags(wxDrag_AllowMove); // allows both copy and move
}

void ListPanel::OnDropPossible(wxDataViewEvent &event)
{
	event.Allow();
}

void ListPanel::OnDrop(wxDataViewEvent &event)
{
	wxDataViewItem item(event.GetItem());

	wxTextDataObject wxobj;
	wxobj.SetData(wxDF_UNICODETEXT, event.GetDataSize(), event.GetDataBuffer());
	wxString source_names = wxobj.GetText();
	fluo::ValueCollection source_name_list;
	boost::split(source_name_list, source_names.ToStdString(), [](char c) {return c == '\n'; });
	for (auto it = source_name_list.begin();
		it != source_name_list.end(); ++it)
		;
	//m_list_model->MoveNode(*it, target_node);
}

void ListPanel::OnListSorted(wxDataViewEvent &event)
{

}

/*BEGIN_EVENT_TABLE(DataListCtrl, wxListCtrl)
EVT_LIST_ITEM_ACTIVATED(wxID_ANY, DataListCtrl::OnAct)
EVT_LIST_ITEM_SELECTED(wxID_ANY, DataListCtrl::OnSelect)
EVT_CONTEXT_MENU(DataListCtrl::OnContextMenu)
EVT_MENU_RANGE(Menu_View_start, Menu_View_start + 10, DataListCtrl::OnAddToView)
EVT_MENU(Menu_Del, DataListCtrl::OnDelete)
EVT_MENU(Menu_Rename, DataListCtrl::OnRename)
EVT_MENU(Menu_Save, DataListCtrl::OnSave)
EVT_MENU(Menu_Bake, DataListCtrl::OnBake)
EVT_MENU(Menu_SaveMask, DataListCtrl::OnSaveMask)
EVT_KEY_DOWN(DataListCtrl::OnKeyDown)
EVT_KEY_UP(DataListCtrl::OnKeyUp)
EVT_MOUSE_EVENTS(DataListCtrl::OnMouse)
EVT_TEXT_ENTER(ID_RenameText, DataListCtrl::OnEndEditName)
EVT_SCROLLWIN(DataListCtrl::OnScroll)
EVT_MOUSEWHEEL(DataListCtrl::OnScroll)
END_EVENT_TABLE()

fluo::VolumeData* DataListCtrl::m_vd = 0;

DataListCtrl::DataListCtrl(
	VRenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxListCtrl(parent, wxID_ANY, pos, size, style),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxListItem itemCol;
	itemCol.SetText("Type");
	this->InsertColumn(0, itemCol);

	itemCol.SetText("Name");
	this->InsertColumn(1, itemCol);

	itemCol.SetText("Path");
	this->InsertColumn(2, itemCol);

	m_rename_text = new wxTextCtrl(this, ID_RenameText, "",
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	m_rename_text->Hide();
}

DataListCtrl::~DataListCtrl()
{
	delete m_rename_text;
}

void DataListCtrl::Append(int type, const wxString &name, const wxString &path)
{
	long tmp = 0;
	if (type == 1)
		tmp = InsertItem(GetItemCount(), "Volume");
	else if (type == 2)
		tmp = InsertItem(GetItemCount(), "Mesh");
	else if (type == 3)
		tmp = InsertItem(GetItemCount(), "Annotations");

	SetItem(tmp, 1, name);
	SetItem(tmp, 2, path);
	SetColumnWidth(0, wxLIST_AUTOSIZE);
	SetColumnWidth(1, wxLIST_AUTOSIZE);
	SetColumnWidth(2, wxLIST_AUTOSIZE);
}

wxString DataListCtrl::GetText(long item, int col)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	return info.GetText();
}

void DataListCtrl::SetText(long item, int col, const wxString &str)
{
	wxListItem info;
	info.SetId(item);
	info.SetColumn(col);
	info.SetMask(wxLIST_MASK_TEXT);
	GetItem(info);
	info.SetText(str);
	SetItem(info);
}

void DataListCtrl::SetSelection(int type, const wxString &name)
{
	wxString str_type;
	switch (type)
	{
	case 1:
		str_type = "Volume";
		break;
	case 2:
		str_type = "Mesh";
		break;
	case 3:
		str_type = "Annotations";
		break;
	}

	long item = -1;
	for (;;)
	{
		item = GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_DONTCARE);
		if (item != -1)
		{
			wxString item_type = GetText(item, 0);
			wxString item_name = GetText(item, 1);

			if (item_type == str_type &&
				item_name == name)
			{
				SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
				break;
			}
		}
		else
			break;
	}
}

void DataListCtrl::SaveSelMask()
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1 && GetItemText(item) == "Volume")
	{
		wxString name = GetText(item, 1);
		fluo::VolumeData* vd = glbin_volf->findFirst(name.ToStdString());
		if (vd)
		{
			long time, chan;
			vd->getValue(gstTime, time);
			vd->getValue(gstChannel, chan);
			vd->SaveMask(true, time, chan);
			vd->SaveLabel(true, time, chan);
		}
	}
}

void DataListCtrl::SaveAllMasks()
{
	long item = GetNextItem(-1);
	while (item != -1)
	{
		if (GetItemText(item) == "Volume")
		{
			wxString name = GetText(item, 1);
			fluo::VolumeData* vd = glbin_volf->findFirst(name.ToStdString());
			if (vd)
			{
				long time, chan;
				vd->getValue(gstTime, time);
				vd->getValue(gstChannel, chan);
				vd->SaveMask(true, time, chan);
				vd->SaveLabel(true, time, chan);
			}
		}
		item = GetNextItem(item);
	}
}

void DataListCtrl::OnContextMenu(wxContextMenuEvent &event)
{
	if (GetSelectedItemCount() > 0)
	{
		wxPoint point = event.GetPosition();
		// If from keyboard
		if (point.x == -1 && point.y == -1)
		{
			wxSize size = GetSize();
			point.x = size.x / 2;
			point.y = size.y / 2;
		}
		else
		{
			point = ScreenToClient(point);
		}

		wxMenu menu;
		wxMenu *add_to_menu = new wxMenu;
		for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
		{
			fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
			if (!view) continue;

			add_to_menu->Append(Menu_View_start + i,
				view->getName());
		}

		menu.Append(Menu_AddTo, "Add to", add_to_menu);
		if (GetSelectedItemCount() == 1)
		{
			menu.Append(Menu_Del, "Delete");
			menu.Append(Menu_Rename, "Rename");
			//save/save as
			long item = GetNextItem(-1,
				wxLIST_NEXT_ALL,
				wxLIST_STATE_SELECTED);
			if (item != -1)
			{
				if (GetItemText(item) == "Volume")
				{
					wxString name = GetText(item, 1);
					fluo::VolumeData* vd = glbin_volf->findFirst(name.ToStdString());
					if (vd)
					{
						std::wstring path;
						vd->getValue(gstDataPath, path);
						if (path.empty())
							menu.Append(Menu_Save, "Save...");
						else
							menu.Append(Menu_Save, "Save As...");
						menu.Append(Menu_Bake, "Bake...");
						menu.Append(Menu_SaveMask, "Save Mask");
					}
				}
				else if (GetItemText(item) == "Mesh")
				{
					wxString name = GetText(item, 1);
					fluo::MeshData* md = glbin_mshf->findFirst(name.ToStdString());
					if (md)
					{
						std::wstring path;
						md->getValue(gstDataPath, path);
						if (path.empty())
							menu.Append(Menu_Save, "Save...");
						else
							menu.Append(Menu_Save, "Save As...");
					}
				}
				else if (GetItemText(item) == "Annotations")
				{
					wxString name = GetText(item, 1);
					fluo::Annotations* ann = glbin_annf->findFirst(name.ToStdString());
					if (ann)
					{
						std::wstring path;
						ann->getValue(gstDataPath, path);
						if (path.empty())
							menu.Append(Menu_Save, "Save...");
						else
							menu.Append(Menu_Save, "Save As...");
					}
				}
			}
		}

		PopupMenu(&menu, point.x, point.y);
	}
}

void DataListCtrl::AddToView(int menu_index, long item)
{
	if (!m_frame)
		return;
	bool view_empty = true;
	wxString name = "";

	fluo::Renderview* view = glbin_root->getChild(menu_index)->asRenderview();
	if (GetItemText(item) == "Volume")
	{
		name = GetText(item, 1);
		fluo::VolumeData* vd = glbin_volf->findFirst(name.ToStdString());
		if (vd)
		{
			if (view)
			{
				fluo::VolumeData* vd_add = vd;

				for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
				{
					fluo::Renderview* v = glbin_root->getChild(i)->asRenderview();
					if (v && v->findFirstChild(name.ToStdString()))
					{
						vd_add = glbin_volf->build(vd);
						break;
					}
				}

				int chan_num = view->GetFullVolListSize();
				view_empty = chan_num > 0 ? false : view_empty;
				fluo::Color color(1.0, 1.0, 1.0);
				if (chan_num == 0)
					color = fluo::Color(1.0, 0.0, 0.0);
				else if (chan_num == 1)
					color = fluo::Color(0.0, 1.0, 0.0);
				else if (chan_num == 2)
					color = fluo::Color(0.0, 0.0, 1.0);

				if (chan_num >= 0 && chan_num < 3)
					vd_add->setValue(gstColor, color);

				fluo::VolumeGroup *group = view->addVolumeData(vd_add, 0);
				m_frame->OnSelection(2, view, group, vd_add, 0);
				long lval;
				view->getValue(gstMixMethod, lval);
				if (lval == fluo::Renderview::MIX_METHOD_MULTI)
				{
					OutAdjustPanel* adjust_view = m_frame->GetAdjustView();
					if (adjust_view)
					{
						adjust_view->SetView(view);
						adjust_view->UpdateSync();
					}
				}
			}
		}
	}
	else if (GetItemText(item) == "Mesh")
	{
		name = GetText(item, 1);
		fluo::MeshData* md = glbin_mshf->findFirst(name.ToStdString());
		if (md)
		{
			if (view)
			{
				int chan_num = view->GetFullVolListSize() + view->GetFullMeshListSize();
				view_empty = chan_num > 0 ? false : view_empty;
				view->addMeshData(md, 0);
			}
		}
	}
	else if (GetItemText(item) == "Annotations")
	{
		name = GetText(item, 1);
		fluo::Annotations* ann = glbin_annf->findFirst(name.ToStdString());
		if (ann)
		{
			if (view)
			{
				int chan_num = view->GetFullVolListSize() + view->GetFullMeshListSize();;
				view_empty = chan_num > 0 ? false : view_empty;
				view->addChild(ann);
			}
		}
	}

	//update
	if (view)
	{
		if (view_empty)
			view->InitView(
				fluo::Renderview::INIT_BOUNDS |
				fluo::Renderview::INIT_CENTER |
				fluo::Renderview::INIT_TRANSL |
				fluo::Renderview::INIT_ROTATE);
		else
			view->InitView(
				fluo::Renderview::INIT_BOUNDS |
				fluo::Renderview::INIT_CENTER);
		//view->Update(39);
	}
	m_frame->UpdateTree(name);
}

void DataListCtrl::OnAddToView(wxCommandEvent& event)
{
	int menu_index = event.GetId() - Menu_View_start;
	int num = GetSelectedItemCount();

	if (num > 0)
	{
		long item = -1;
		for (;; )
		{
			item = GetNextItem(item,
				wxLIST_NEXT_ALL,
				wxLIST_STATE_SELECTED);
			if (item == -1)
				break;

			AddToView(menu_index, item);
		}
	}
}

void DataListCtrl::OnDelete(wxCommandEvent &event)
{
	DeleteSelection();
}

void DataListCtrl::OnRename(wxCommandEvent& event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxRect rect;
		GetSubItemRect(item, 1, rect);
		wxString str = GetText(item, 1);
		m_rename_text->SetPosition(rect.GetTopLeft());
		m_rename_text->SetSize(rect.GetSize());
		m_rename_text->SetValue(str);
		m_rename_text->SetFocus();
		m_rename_text->SetSelection(-1, -1);
		m_rename_text->Show();
	}
}

//crop
void DataListCtrl::OnCropCheck(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		VRenderFrame::SetCrop(ch1->GetValue());
}

//compress
void DataListCtrl::OnCompCheck(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		VRenderFrame::SetCompression(ch1->GetValue());
}

void DataListCtrl::OnResizeCheck(wxCommandEvent &event)
{
	wxCheckBox* comp_chk = (wxCheckBox*)event.GetEventObject();
	if (!comp_chk)
		return;
	bool resize = comp_chk->GetValue();
	wxWindow* panel = comp_chk->GetParent();
	if (!panel)
		return;
	wxTextCtrl* size_x_txt = (wxTextCtrl*)panel->FindWindow(ID_RESIZE_X_TXT);
	wxTextCtrl* size_y_txt = (wxTextCtrl*)panel->FindWindow(ID_RESIZE_Y_TXT);
	wxTextCtrl* size_z_txt = (wxTextCtrl*)panel->FindWindow(ID_RESIZE_Z_TXT);
	//set size values
	if (size_x_txt && size_y_txt && size_z_txt)
	{
		if (m_vd && resize)
		{
			long nx, ny, nz;
			m_vd->getValue(gstResX, nx);
			m_vd->getValue(gstResY, ny);
			m_vd->getValue(gstResZ, nz);
			size_x_txt->SetValue(wxString::Format("%d", nx));
			size_y_txt->SetValue(wxString::Format("%d", ny));
			size_z_txt->SetValue(wxString::Format("%d", nz));
		}
		else
		{
			size_x_txt->SetValue("");
			size_y_txt->SetValue("");
			size_z_txt->SetValue("");
		}
	}
	if (m_vd)
		m_vd->setValue(gstResize, resize);
}

void DataListCtrl::OnSizeXText(wxCommandEvent &event)
{
	wxTextCtrl* size_x_txt = (wxTextCtrl*)event.GetEventObject();
	if (size_x_txt && m_vd)
	{
		long lval;
		size_x_txt->GetValue().ToLong(&lval);
		m_vd->setValue(gstResizeX, lval);
	}
}

void DataListCtrl::OnSizeYText(wxCommandEvent &event)
{
	wxTextCtrl* size_y_txt = (wxTextCtrl*)event.GetEventObject();
	if (size_y_txt && m_vd)
	{
		long lval;
		size_y_txt->GetValue().ToLong(&lval);
		m_vd->setValue(gstResizeY, lval);
	}
}

void DataListCtrl::OnSizeZText(wxCommandEvent &event)
{
	wxTextCtrl* size_z_txt = (wxTextCtrl*)event.GetEventObject();
	if (size_z_txt && m_vd)
	{
		long lval;
		size_z_txt->GetValue().ToLong(&lval);
		m_vd->setValue(gstResizeZ, lval);
	}
}

void DataListCtrl::OnFilterChange(wxCommandEvent &event)
{
	wxComboBox* combo = (wxComboBox*)event.GetEventObject();
	if (combo)
		VRenderFrame::SetFilter(combo->GetSelection());
}

wxWindow* DataListCtrl::CreateExtraControl(wxWindow* parent)
{
	wxIntegerValidator<unsigned int> vald_int;

	wxPanel* panel = new wxPanel(parent);
#ifdef _DARWIN
	panel->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

	//compressed
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* comp_chk = new wxCheckBox(panel, ID_LZW_COMP,
		"Lempel-Ziv-Welch Compression");
	comp_chk->Connect(comp_chk->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(DataListCtrl::OnCompCheck), NULL, panel);
	comp_chk->SetValue(VRenderFrame::GetCompression());
	sizer1->Add(10, 10);
	sizer1->Add(comp_chk);
	//crop
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* crop_chk = new wxCheckBox(panel, ID_CROP,
		"Use Clipping Planes to Crop");
	crop_chk->Connect(crop_chk->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(DataListCtrl::OnCropCheck), NULL, panel);
	crop_chk->SetValue(VRenderFrame::GetCrop());
	sizer2->Add(10, 10);
	sizer2->Add(crop_chk);
	//resize
	wxBoxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox* resize_chk = new wxCheckBox(panel, ID_RESIZE_CHK,
		"Resize");
	resize_chk->Connect(resize_chk->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(DataListCtrl::OnResizeCheck), NULL, panel);
	wxTextCtrl* size_x_txt = new wxTextCtrl(panel, ID_RESIZE_X_TXT, "",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	size_x_txt->Connect(size_x_txt->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(DataListCtrl::OnSizeXText), NULL, panel);
	wxTextCtrl* size_y_txt = new wxTextCtrl(panel, ID_RESIZE_Y_TXT, "",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	size_y_txt->Connect(size_y_txt->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(DataListCtrl::OnSizeYText), NULL, panel);
	wxTextCtrl* size_z_txt = new wxTextCtrl(panel, ID_RESIZE_Z_TXT, "",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	size_z_txt->Connect(size_z_txt->GetId(), wxEVT_TEXT,
		wxCommandEventHandler(DataListCtrl::OnSizeZText), NULL, panel);
	wxComboBox* combo = new wxComboBox(panel, ID_FILTER,
		"Filter", wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	combo->Connect(combo->GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED,
		wxCommandEventHandler(DataListCtrl::OnFilterChange), NULL, panel);
	std::vector<std::string> combo_list;
	combo_list.push_back("Nearest neighbor");
	combo_list.push_back("Bilinear");
	combo_list.push_back("Trilinear");
	combo_list.push_back("Box");
	for (size_t i = 0; i < combo_list.size(); ++i)
		combo->Append(combo_list[i]);
	combo->SetSelection(VRenderFrame::GetFilter());

	if (m_vd)
	{
		bool resize;
		long nx, ny, nz;
		m_vd->getValue(gstResize, resize);
		m_vd->getValue(gstResizeX, nx);
		m_vd->getValue(gstResizeY, ny);
		m_vd->getValue(gstResizeZ, nz);
		resize_chk->SetValue(resize);
		if (resize)
		{
			size_x_txt->SetValue(std::to_string(nx));
			size_y_txt->SetValue(std::to_string(ny));
			size_z_txt->SetValue(std::to_string(nz));
		}
	}
	sizer3->Add(10, 10);
	sizer3->Add(resize_chk, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(size_x_txt, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(size_y_txt, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(size_z_txt, 0, wxALIGN_CENTER);
	sizer3->Add(10, 10);
	sizer3->Add(combo, 0, wxALIGN_CENTER);

	//group
	group1->Add(10, 10);
	group1->Add(sizer1);
	group1->Add(10, 10);
	group1->Add(sizer2);
	group1->Add(10, 10);
	group1->Add(sizer3);
	group1->Add(10, 20);

	panel->SetSizerAndFit(group1);
	panel->Layout();

	return panel;
}

void DataListCtrl::OnSave(wxCommandEvent& event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1)
	{
		wxString name = GetText(item, 1);

		if (GetItemText(item) == "Volume")
		{

			if (m_frame)
				m_vd = glbin_volf->findFirst(name.ToStdString());
			else
				return;
			fluo::Quaternion q;
			fluo::Renderview* view = glbin_root->getCurrentRenderview();
			if (view)
				view->getValue(gstClipRotQ, q);
			if (m_vd)
			{
				m_vd->setValue(gstResize, false);
				m_vd->setValue(gstResizeX, 0);
				m_vd->setValue(gstResizeY, 0);
				m_vd->setValue(gstResizeZ, 0);
			}

			wxFileDialog *fopendlg = new wxFileDialog(
				m_frame, "Save Volume Data", "", "",
				"Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
				"Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
				"Utah Nrrd file (*.nrrd)|*.nrrd",
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			fopendlg->SetExtraControlCreator(CreateExtraControl);

			int rval = fopendlg->ShowModal();

			if (rval == wxID_OK)
			{
				wxString filename = fopendlg->GetPath();
				if (m_vd)
				{
					m_vd->SaveData(filename.ToStdWstring(), fopendlg->GetFilterIndex(),
						VRenderFrame::GetCrop(), VRenderFrame::GetFilter(),
						false, VRenderFrame::GetCompression(), q);
					std::wstring str;
					m_vd->getValue(gstDataPath, str);
					SetText(item, 2, str);
				}
			}
			delete fopendlg;

			if (m_vd)
			{
				m_vd->setValue(gstResize, false);
				m_vd->setValue(gstResizeX, 0);
				m_vd->setValue(gstResizeY, 0);
				m_vd->setValue(gstResizeZ, 0);
			}
		}
		else if (GetItemText(item) == "Mesh")
		{
			wxFileDialog *fopendlg = new wxFileDialog(
				m_frame, "Save Mesh Data", "", "",
				"OBJ file (*.obj)|*.obj",
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

			int rval = fopendlg->ShowModal();

			if (rval == wxID_OK)
			{
				wxString filename = fopendlg->GetPath();

				if (m_frame)
				{
					fluo::MeshData* md = glbin_mshf->findFirst(name.ToStdString());
					if (md)
					{
						md->SaveData(filename.ToStdString());
						std::wstring str;
						md->getValue(gstDataPath, str);
						SetText(item, 2, str);
					}
				}
			}
			delete fopendlg;
		}
		else if (GetItemText(item) == "Annotations")
		{
			wxFileDialog *fopendlg = new wxFileDialog(
				m_frame, "Save Annotations", "", "",
				"Text file (*.txt)|*.txt",
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

			int rval = fopendlg->ShowModal();

			if (rval == wxID_OK)
			{
				wxString filename = fopendlg->GetPath();

				if (m_frame)
				{
					fluo::Annotations* ann = glbin_annf->findFirst(name.ToStdString());
					if (ann)
					{
						ann->SaveData(filename.ToStdWstring());
						std::wstring wstr;
						ann->getValue(gstDataPath, wstr);
						SetText(item, 2, wstr);
					}
				}
			}
			delete fopendlg;
		}
	}
}

void DataListCtrl::OnBake(wxCommandEvent& event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (item != -1 && GetItemText(item) == "Volume")
	{
		wxString name = GetText(item, 1);

		wxFileDialog *fopendlg = new wxFileDialog(
			m_frame, "Bake Volume Data", "", "",
			"Muti-page Tiff file (*.tif, *.tiff)|*.tif;*.tiff|"\
			"Single-page Tiff sequence (*.tif)|*.tif;*.tiff|"\
			"Utah Nrrd file (*.nrrd)|*.nrrd",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		fopendlg->SetExtraControlCreator(CreateExtraControl);

		int rval = fopendlg->ShowModal();

		if (rval == wxID_OK)
		{
			wxString filename = fopendlg->GetPath();

			if (m_frame)
			{
				fluo::Quaternion q;
				fluo::Renderview* view = glbin_root->getCurrentRenderview();
				if (view)
					view->getValue(gstClipRotQ, q);
				fluo::VolumeData* vd = glbin_volf->findFirst(name.ToStdString());
				if (vd)
				{
					vd->SaveData(filename.ToStdWstring(), fopendlg->GetFilterIndex(),
						VRenderFrame::GetCrop(), VRenderFrame::GetFilter(),
						true, VRenderFrame::GetCompression(), q);
					std::wstring str;
					vd->getValue(gstDataPath, str);
					SetText(item, 2, str);
				}
			}
		}

		delete fopendlg;
	}
}

void DataListCtrl::OnSaveMask(wxCommandEvent& event)
{
	SaveSelMask();
}

void DataListCtrl::OnSelect(wxListEvent &event)
{
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxString name = GetText(item, 1);
		if (m_frame)
			m_frame->GetTree()->Select("", name);
	}
}

void DataListCtrl::OnAct(wxListEvent &event)
{
	int index = 0;
	long item = GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);
	if (m_frame && item != -1)
	{
		index = m_frame->GetViewNum() - 1;
		AddToView(index, item);
	}
}

void DataListCtrl::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE ||
		event.GetKeyCode() == WXK_BACK)
		DeleteSelection();
	//event.Skip();
}

void DataListCtrl::OnKeyUp(wxKeyEvent& event)
{
	event.Skip();
}

void DataListCtrl::OnMouse(wxMouseEvent &event)
{
	if (event.Button(wxMOUSE_BTN_ANY))
		m_rename_text->Hide();
	event.Skip();
}

void DataListCtrl::EndEdit(bool update)
{
	if (!m_rename_text->IsShown())
		return;

	if (update)
	{
		wxString new_name = m_rename_text->GetValue();

		long item = GetNextItem(-1,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_SELECTED);
		if (item != -1)
		{
			wxString name = GetText(item, 1);

			if (new_name != name)
			{
				std::string new_name2 = new_name.ToStdString();
				for (int i = 1; glbin.checkName(new_name2); i++)
					new_name2 = new_name + "_" + std::to_string(i);
				fluo::Object* obj = glbin.get(name.ToStdString());
				if (obj)
					obj->setName(new_name2);

				//update ui
				SetText(item, 1, new_name2);
				m_frame->UpdateTree();
			}
		}
	}

	m_rename_text->Hide();
}

void DataListCtrl::OnEndEditName(wxCommandEvent& event)
{
	EndEdit();
}

void DataListCtrl::DeleteSelection()
{
	//wxString name = "";

	//if (m_frame && GetSelectedItemCount() > 0)
	//{
	//	long item = GetNextItem(-1,
	//		wxLIST_NEXT_ALL,
	//		wxLIST_STATE_SELECTED);
	//	if (item != -1)
	//	{
	//		if (GetItemText(item) == "Volume")
	//		{
	//			name = GetText(item, 1);
	//			int i;
	//			//from view
	//			for (int i = 0; i < m_frame->GetViewNum(); i++)
	//			{
	//				RenderCanvas* view = m_frame->GetView(i);
	//				if (view)
	//				{
	//					view->RemoveVolumeDataDup(name.ToStdString());
	//				}
	//			}
	//			//from datamanager
	//			glbin_volf->remove(name.ToStdString());
	//		}
	//		else if (GetItemText(item) == "Mesh")
	//		{
	//			name = GetText(item, 1);
	//			int i;
	//			//from view
	//			for (int i = 0; i < m_frame->GetViewNum(); i++)
	//			{
	//				RenderCanvas* view = m_frame->GetView(i);
	//				if (view)
	//				{
	//					view->RemoveMeshData(name.ToStdString());
	//				}
	//			}
	//			//from datamanager
	//			glbin_mshf->remove(name.ToStdString());
	//		}
	//		else if (GetItemText(item) == "Annotations")
	//		{
	//			name = GetText(item, 1);
	//			int i;
	//			//from view
	//			for (int i = 0; i < m_frame->GetViewNum(); i++)
	//			{
	//				RenderCanvas* view = m_frame->GetView(i);
	//				if (view)
	//					view->RemoveAnnotations(name.ToStdString());
	//			}
	//			//from datamanager
	//			glbin_annf->remove(name.ToStdString());
	//		}
	//	}
	//	m_frame->UpdateList();
	//	m_frame->UpdateTree(name);
	//	m_frame->RefreshVRenderViews();
	//}
}

void DataListCtrl::DeleteAll()
{
	//wxString name = "";

	//long item = GetNextItem(-1);
	//while (item != -1 && m_frame)
	//{
	//	if (GetItemText(item) == "Volume")
	//	{
	//		name = GetText(item, 1);
	//		int i;
	//		//from view
	//		for (int i = 0; i < m_frame->GetViewNum(); i++)
	//		{
	//			RenderCanvas* view = m_frame->GetView(i);
	//			if (view)
	//				view->RemoveVolumeDataDup(name.ToStdString());
	//		}
	//		//from datamanager
	//		glbin_volf->remove(name.ToStdString());
	//	}
	//	else if (GetItemText(item) == "Mesh")
	//	{
	//		name = GetText(item, 1);
	//		int i;
	//		//from view
	//		for (int i = 0; i < m_frame->GetViewNum(); i++)
	//		{
	//			RenderCanvas* view = m_frame->GetView(i);
	//			if (view)
	//				view->RemoveMeshData(name.ToStdString());
	//		}
	//		//from datamanager
	//		glbin_mshf->remove(name.ToStdString());
	//	}
	//	else if (GetItemText(item) == "Annotations")
	//	{
	//		name = GetText(item, 1);
	//		int i;
	//		//from view
	//		for (int i = 0; i < m_frame->GetViewNum(); i++)
	//		{
	//			RenderCanvas* view = m_frame->GetView(i);
	//			if (view)
	//				view->RemoveAnnotations(name.ToStdString());
	//		}
	//		//from datamanager
	//		glbin_annf->remove(name.ToStdString());
	//	}

	//	item = GetNextItem(item);
	//}

	//DeleteAllItems();
	//if (m_frame)
	//{
	//	m_frame->UpdateTree();
	//	m_frame->RefreshVRenderViews();
	//}
}

void DataListCtrl::OnScroll(wxScrollWinEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

void DataListCtrl::OnScroll(wxMouseEvent& event)
{
	EndEdit(false);
	event.Skip(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ListPanel, wxPanel)
EVT_TOOL(ID_AddToView, ListPanel::OnAddToView)
EVT_TOOL(ID_Rename, ListPanel::OnRename)
EVT_TOOL(ID_Save, ListPanel::OnSave)
EVT_TOOL(ID_Bake, ListPanel::OnBake)
EVT_TOOL(ID_SaveMask, ListPanel::OnSaveMask)
EVT_TOOL(ID_Delete, ListPanel::OnDelete)
EVT_TOOL(ID_DeleteAll, ListPanel::OnDeleteAll)
END_EVENT_TABLE()

ListPanel::ListPanel(VRenderFrame *frame,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString& name) :
	wxPanel(frame, wxID_ANY, pos, size, style, name)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//create data list
	m_datalist = new DataListCtrl(frame, this);

	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(view);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddTool(ID_AddToView, "Add to View",
		bitmap, "Add: Add the selected dataset to render view");
	bitmap = wxGetBitmapFromMemory(rename);
	m_toolbar->AddTool(ID_Rename, "Rename",
		bitmap, "Rename: Rename the selected dataset");
	bitmap = wxGetBitmapFromMemory(save);
	m_toolbar->AddTool(ID_Save, "Save As",
		bitmap, "Save: Save the selected volume dataset");
	bitmap = wxGetBitmapFromMemory(bake);
	m_toolbar->AddTool(ID_Bake, "Bake",
		bitmap, "Bake: Apply the volume properties and save");
	bitmap = wxGetBitmapFromMemory(save_mask);
	m_toolbar->AddTool(ID_SaveMask, "Save Mask",
		bitmap, "Save Mask: Save its mask to a file");
	bitmap = wxGetBitmapFromMemory(delet);
	m_toolbar->AddTool(ID_Delete, "Delete",
		bitmap, "Delete: Delete the selected dataset");
	bitmap = wxGetBitmapFromMemory(del_all);
	m_toolbar->AddTool(ID_DeleteAll, "Delete All",
		bitmap, "Delete All: Delete all datasets");
	m_toolbar->Realize();

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);

	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(m_datalist, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();
}

ListPanel::~ListPanel()
{
}

void ListPanel::Append(int type, const wxString &name, const wxString &path)
{
	if (m_datalist)
		m_datalist->Append(type, name, path);
}

wxString ListPanel::GetText(long item, int col)
{
	wxString str = "";
	if (m_datalist)
		str = m_datalist->GetText(item, col);
	return str;
}

void ListPanel::SetText(long item, int col, const wxString &str)
{
	if (m_datalist)
		m_datalist->SetText(item, col, str);
}

void ListPanel::DeleteAllItems()
{
	if (m_datalist)
		m_datalist->DeleteAllItems();
}

void ListPanel::SetSelection(int type, const wxString &name)
{
	if (m_datalist)
		m_datalist->SetSelection(type, name);
}

void ListPanel::SaveAllMasks()
{
	if (m_datalist)
		m_datalist->SaveAllMasks();
}

void ListPanel::OnAddToView(wxCommandEvent& event)
{
	wxListEvent list_event;
	m_datalist->OnAct(list_event);
}

void ListPanel::OnRename(wxCommandEvent& event)
{
	m_datalist->OnRename(event);
}

void ListPanel::OnSave(wxCommandEvent& event)
{
	m_datalist->OnSave(event);
}

void ListPanel::OnBake(wxCommandEvent& event)
{
	m_datalist->OnBake(event);
}

void ListPanel::OnSaveMask(wxCommandEvent& event)
{
	m_datalist->OnSaveMask(event);
}

void ListPanel::OnDelete(wxCommandEvent& event)
{
	m_datalist->DeleteSelection();
}

void ListPanel::OnDeleteAll(wxCommandEvent &event)
{
	m_datalist->DeleteAll();
}
*/