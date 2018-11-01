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
#include <Fui/ListPanel.h>
#include <VRenderFrame.h>
#include <Global/Global.h>
#include <Scenegraph/VolumeData.h>
#include <boost/algorithm/string.hpp>
#include <Formats/png_resource.h>
#include <img/icons.h>

using namespace FUI;

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

ListPanel::ListPanel(wxWindow *frame,
	wxWindow *parent,
	wxWindowID id,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString& name) :
	wxPanel(parent, id, pos, size, style, name),
	m_frame(frame)
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
	m_list_model =
		FL::Global::instance().getAgentFactory().
		getOrAddListModel("ListPanel", *this);
	//append columns
	//name
	wxDataViewTextRenderer *tr =
		new wxDataViewTextRenderer("string");
	wxDataViewColumn *column0 =
		new wxDataViewColumn("Name", tr, 0, 120, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_list_ctrl->AppendColumn(column0);
	//type
	tr = new wxDataViewTextRenderer("string");
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
	//multiple selections
	wxDataViewItemArray sel;
	m_list_ctrl->GetSelections(sel);
	wxString names;
	for (auto it = sel.begin();
		it != sel.end(); ++it)
	{
		FL::Node* sel_node = (FL::Node*)it->GetID();
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
	std::vector<std::string> source_name_list;
	boost::split(source_name_list, source_names.ToStdString(), [](char c) {return c == '\n'; });
	for (auto it = source_name_list.begin();
		it != source_name_list.end(); ++it)
		;
		//m_list_model->MoveNode(*it, target_node);
}

void ListPanel::OnListSorted(wxDataViewEvent &event)
{

}