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
#include <Fui/TreePanel.h>
#include <VRenderFrame.h>
#include <Global/Global.h>
#include <Scenegraph/VolumeData.h>
#include <Formats/png_resource.h>
#include <img/icons.h>
#include <img/tick.xpm>
#include <img/cross.xpm>

using namespace FUI;

BEGIN_EVENT_TABLE(TreePanel, wxPanel)
END_EVENT_TABLE()

TreePanel::TreePanel(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name):
	wxPanel(parent, id, pos, size, style, name),
	m_frame(frame)
{
	//create tool bar
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER);
	wxBitmap bitmap = wxGetBitmapFromMemory(toggle_disp);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddTool(ID_ToggleView, "Toggle View", bitmap,
		"Toggle the visibility of current selection");
	bitmap = wxGetBitmapFromMemory(add_group);
	m_toolbar->AddTool(ID_AddGroup, "Add Group", bitmap,
		"Add a volume data group to the selected view");
	bitmap = wxGetBitmapFromMemory(add_mgroup);
	m_toolbar->AddTool(ID_AddMGroup, "Add Mesh Group", bitmap,
		"Add a mesh data group to the selected view");
	bitmap = wxGetBitmapFromMemory(delet);
	m_toolbar->AddTool(ID_RemoveData, "Delete", bitmap,
		"Delete current selection");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_append);
	m_toolbar->AddCheckTool(ID_BrushAppend, "Highlight",
		bitmap, wxNullBitmap,
		"Highlight structures by painting on the render view (hold Shift)");
	bitmap = wxGetBitmapFromMemory(brush_diffuse);
	m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
		bitmap, wxNullBitmap,
		"Diffuse highlighted structures by painting (hold Z)");
	bitmap = wxGetBitmapFromMemory(brush_desel);
	m_toolbar->AddCheckTool(ID_BrushDesel, "Reset",
		bitmap, wxNullBitmap,
		"Reset highlighted structures by painting (hold X)");
	bitmap = wxGetBitmapFromMemory(brush_clear);
	m_toolbar->AddTool(ID_BrushClear, "Reset All",
		bitmap, "Reset all highlighted structures");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_erase);
	m_toolbar->AddTool(ID_BrushErase, "Erase",
		bitmap, "Erase highlighted structures");
	bitmap = wxGetBitmapFromMemory(brush_create);
	m_toolbar->AddTool(ID_BrushCreate, "Extract", bitmap,
		"Extract highlighted structures out and create a new volume");
	m_toolbar->Realize();

	//create data tree
	m_tree_ctrl = new wxDataViewCtrl(this, wxID_ANY);
	m_tree_ctrl->EnableDragSource(wxDF_UNICODETEXT);
	m_tree_ctrl->EnableDropTarget(wxDF_UNICODETEXT);
	//wxImageList *images = new wxImageList(16, 16, true);
	//wxIcon icons[2];
	//icons[0] = wxIcon(cross_xpm);
	//icons[1] = wxIcon(tick_xpm);
	//images->Add(icons[0]);
	//images->Add(icons[1]);
	//m_tree_ctrl->AssignImageList(images);
	m_tree_ctrl->SetDoubleBuffered(true);
	//add columns
	//name
	wxDataViewTextRenderer *tr =
		new wxDataViewTextRenderer("string");
	wxDataViewColumn *column0 =
		new wxDataViewColumn("Name", tr, 0, 200, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_tree_ctrl->AppendColumn(column0);
	//type
	tr = new wxDataViewTextRenderer("string");
	wxDataViewColumn *column1 =
		new wxDataViewColumn("Type", tr, 1, 200, wxALIGN_LEFT,
			wxDATAVIEW_COL_SORTABLE |
			wxDATAVIEW_COL_REORDERABLE |
			wxDATAVIEW_COL_RESIZABLE);
	m_tree_ctrl->AppendColumn(column1);

	//organize positions
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(m_tree_ctrl, 1, wxEXPAND);
	SetSizer(sizer_v);
	Layout();
}

TreePanel::~TreePanel()
{

}

void TreePanel::SetScenegraph(FL::Node* root)
{
	m_tree_model = new TreeModel;
	m_tree_model->SetRoot(root);

	m_tree_ctrl->AssociateModel(m_tree_model.get());
	m_tree_model->ItemAdded(
		wxDataViewItem(0),
		wxDataViewItem((void*)m_tree_model->GetRoot()));
}

//seelction
void TreePanel::UpdateSelection()
{

}

wxString TreePanel::GetCurrentSel()
{
	return "";
}

void TreePanel::Select(wxString view, wxString name)
{

}

void TreePanel::SelectBrush(int id)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	//m_datatree->m_fixed = false;

	switch (id)
	{
	case ID_BrushAppend:
		m_toolbar->ToggleTool(ID_BrushAppend, true);
		//m_datatree->m_fixed = true;
		break;
	case ID_BrushDiffuse:
		m_toolbar->ToggleTool(ID_BrushDiffuse, true);
		//m_datatree->m_fixed = true;
		break;
	case ID_BrushDesel:
		m_toolbar->ToggleTool(ID_BrushDesel, true);
		//m_datatree->m_fixed = true;
		break;
	}
}

int TreePanel::GetBrushSelected()
{
	if (m_toolbar->GetToolState(ID_BrushAppend))
		return ID_BrushAppend;
	else if (m_toolbar->GetToolState(ID_BrushDiffuse))
		return ID_BrushDiffuse;
	else if (m_toolbar->GetToolState(ID_BrushDesel))
		return ID_BrushDesel;
	else
		return 0;
}

void TreePanel::BrushAppend()
{

}

void TreePanel::BrushDiffuse()
{

}

void TreePanel::BrushDesel()
{

}

void TreePanel::BrushClear()
{

}

void TreePanel::BrushErase()
{

}

void TreePanel::BrushCreate()
{

}

void TreePanel::BrushSolid(bool state)
{

}
