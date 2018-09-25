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
#include <Formats/png_resource.h>
#include <img/icons.h>

using namespace FUI;

BEGIN_EVENT_TABLE(ListPanel, wxPanel)
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

	m_list_ctrl = new wxDataViewCtrl(this, wxID_ANY);
	m_list_ctrl->EnableDragSource(wxDF_UNICODETEXT);
	m_list_ctrl->EnableDropTarget(wxDF_UNICODETEXT);
	m_list_ctrl->SetDoubleBuffered(true);
	m_list_model = new ListModel;
	m_list_model->SetRoot();
	m_list_ctrl->AssociateModel(m_list_model.get());

	//append columns
	m_list_ctrl->AppendTextColumn("Name", 0,
		wxDATAVIEW_CELL_ACTIVATABLE,
		wxCOL_WIDTH_AUTOSIZE,
		wxALIGN_LEFT,
		wxDATAVIEW_COL_SORTABLE |
		wxDATAVIEW_COL_REORDERABLE |
		wxDATAVIEW_COL_RESIZABLE);
	m_list_ctrl->AppendTextColumn("Type", 1,
		wxDATAVIEW_CELL_ACTIVATABLE,
		wxCOL_WIDTH_AUTOSIZE,
		wxALIGN_LEFT,
		wxDATAVIEW_COL_SORTABLE |
		wxDATAVIEW_COL_REORDERABLE |
		wxDATAVIEW_COL_RESIZABLE);
	m_list_ctrl->AppendTextColumn("Path", 2,
		wxDATAVIEW_CELL_ACTIVATABLE,
		wxCOL_WIDTH_AUTOSIZE,
		wxALIGN_LEFT,
		wxDATAVIEW_COL_SORTABLE |
		wxDATAVIEW_COL_REORDERABLE |
		wxDATAVIEW_COL_RESIZABLE);

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

