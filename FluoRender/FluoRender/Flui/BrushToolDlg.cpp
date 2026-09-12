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
#include <BrushToolDlg.h>
#include <BrushToolDlgAgent.h>
#include <GridHelper.h>
#include <RenderView.h>
#include <VolumeSelector.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
#include <wx/clipbrd.h>
//resources
#include <png_resource.h>
#include <icons.h>

BrushToolDlg::BrushToolDlg(
	wxWindow *parent):
	TabbedPanel(parent,
		wxDefaultPosition,
		parent->FromDIP(wxSize(500, 620)),
		0, "BrushToolDlg"),
	m_hold_history(false),
	m_auto_update_timer(this)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
	m_notebook->AddPage(CreateToolPage(m_notebook), "Tools", true);
	m_notebook->AddPage(CreateListPage(m_notebook), "Information");
	m_notebook->AddPage(CreateAlignPage(m_notebook), "Align");

	Bind(wxEVT_SIZE, &BrushToolDlg::OnSize, this);
	Bind(wxEVT_TIMER, &BrushToolDlg::OnAutoUpdateTimer, this);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
	Thaw();
}

BrushToolDlg::~BrushToolDlg()
{
}

wxWindow* BrushToolDlg::CreateToolPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: floating point 3
	wxFloatingPointValidator<double> vald_fp3(3);
	vald_fp3.SetRange(0.0, 1.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	//toolbar
	m_toolbar = new wxToolBar(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT);
	wxBitmapBundle bitmap;
	bitmap = wxGetBitmap(undo);
	m_toolbar->AddTool(
		ID_BrushUndo, "Undo", bitmap,
		"Rollback previous brush operation");
	m_toolbar->SetToolLongHelp(ID_BrushUndo, "Rollback previous brush operation");
	bitmap = wxGetBitmap(redo);
	m_toolbar->AddTool(
		ID_BrushRedo, "Redo", bitmap,
		"Redo the rollback brush operation");
	m_toolbar->SetToolLongHelp(ID_BrushRedo, "Redo the rollback brush operation");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmap(grow);
	m_toolbar->AddCheckTool(ID_BrushGrow, "Grow",
		bitmap, wxNullBitmap,
		"Click and hold mouse button to grow selection mask from a point",
		"Click and hold mouse button to grow selection mask from a point");
	bitmap = wxGetBitmap(brush_append);
	m_toolbar->AddCheckTool(ID_BrushAppend, "Select",
		bitmap, wxNullBitmap,
		"Highlight structures by painting on the render view (hold Shift)",
		"Highlight structures by painting on the render view (hold Shift)");
	bitmap = wxGetBitmap(brush_comp);
	m_toolbar->AddCheckTool(ID_BrushComp, "Segmnt",
		bitmap, wxNullBitmap,
		"Select structures and then segment them into components",
		"Select structures and then segment them into components");
	bitmap = wxGetBitmap(brush_mesh);
	m_toolbar->AddCheckTool(ID_BrushMesh, "Mesh",
		bitmap, wxNullBitmap,
		"Select structures and then convert them to mesh",
		"Select structures and then convert them to mesh");
	bitmap = wxGetBitmap(brush_single);
	m_toolbar->AddCheckTool(ID_BrushSingle, "Isol.",
		bitmap, wxNullBitmap,
		"Select and isolate a structure by painting",
		"Select and isolate a structure by painting");
	bitmap = wxGetBitmap(brush_diffuse);
	m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diff.",
		bitmap, wxNullBitmap,
		"Diffuse highlighted structures by painting (hold Z)",
		"Diffuse highlighted structures by painting (hold Z)");
	bitmap = wxGetBitmap(brush_solid);
	m_toolbar->AddCheckTool(ID_BrushSolid, "Solid",
		bitmap, wxNullBitmap,
		"Highlight structures with solid mask",
		"Highlight structures with solid mask");
	bitmap = wxGetBitmap(brush_unsel);
	m_toolbar->AddCheckTool(ID_BrushUnsel, "Unsel.",
		bitmap, wxNullBitmap,
		"Remove the highlights by painting (hold X)",
		"Remove the highlights by painting (hold X)");
	m_toolbar->Bind(wxEVT_TOOL, &BrushToolDlg::OnToolBar, this);
	m_toolbar->Realize();

	//clear tools
	m_toolbar2 = new wxToolBar(page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_TOP | wxTB_NODIVIDER | wxTB_TEXT);
	bitmap = wxGetBitmap(brush_clear);
	m_toolbar2->AddTool(ID_BrushClear, "Clear",
		bitmap, "Clear all highlights");
	m_toolbar2->SetToolLongHelp(ID_BrushClear, "Clear all highlights");
	bitmap = wxGetBitmap(brush_extract);
	m_toolbar2->AddTool(ID_BrushExtract, "Extract",
		bitmap, "Extract highlighted structures and create a new volume");
	m_toolbar2->SetToolLongHelp(ID_BrushExtract, "Extract highlighted structures and create a new volume");
	bitmap = wxGetBitmap(brush_delete);
	m_toolbar2->AddTool(ID_BrushDelete, "Delete",
		bitmap, "Delete highlighted structures");
	m_toolbar2->SetToolLongHelp(ID_BrushDelete, "Delete highlighted structures");
	m_toolbar2->AddSeparator();
	//mask tools
	bitmap = wxGetBitmap(mask_copy);
	m_toolbar2->AddTool(
		ID_MaskCopy, "Copy", bitmap,
		"Copy current selection mask to clipboard");
	m_toolbar2->SetToolLongHelp(ID_MaskCopy, "Copy current selection mask to clipboard");
	bitmap = wxGetBitmap(copy_data);
	m_toolbar2->AddTool(
		ID_MaskCopyData, "Data", bitmap,
		"Copy current channel data as mask to clipboard");
	m_toolbar2->SetToolLongHelp(ID_MaskCopyData, "Copy current channel data as mask to clipboard");
	m_toolbar2->AddSeparator();
	bitmap = wxGetBitmap(mask_paste);
	m_toolbar2->AddTool(
		ID_MaskPaste, "Paste", bitmap,
		"Paste selection mask from clipboard");
	m_toolbar2->SetToolLongHelp(ID_MaskPaste, "Paste selection mask from clipboard");
	bitmap = wxGetBitmap(mask_union);
	m_toolbar2->AddTool(
		ID_MaskMerge, "Merge", bitmap,
		"Merge selection mask from clipboard with current");
	m_toolbar2->SetToolLongHelp(ID_MaskMerge, "Merge selection mask from clipboard with current");
	bitmap = wxGetBitmap(mask_exclude);
	m_toolbar2->AddTool(
		ID_MaskExclude, "Exclude", bitmap,
		"Exclude clipboard's selection mask from current");
	m_toolbar2->SetToolLongHelp(ID_MaskExclude, "Exclude clipboard's selection mask from current");
	bitmap = wxGetBitmap(mask_intersect);
	m_toolbar2->AddTool(
		ID_MaskIntersect, "Intersect", bitmap,
		"Intersect selection mask from clipboard with current");
	m_toolbar2->SetToolLongHelp(ID_MaskIntersect, "Intersect selection mask from clipboard with current");
	m_toolbar2->Bind(wxEVT_TOOL, &BrushToolDlg::OnToolBar, this);
	m_toolbar2->Realize();

	//Selection adjustment
	wxStaticBoxSizer *sizer1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Selection Settings");
	//stop at boundary
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_hidden_removal_chk = new wxCheckBox(page, wxID_ANY, "Visible Only:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_hidden_removal_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushHiddenRemovalChk, this);
	m_select_group_chk = new wxCheckBox(page, wxID_ANY, "Apply to Group:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_select_group_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushSelectGroupChk, this);
	m_accurate_bricks_chk = new wxCheckBox(page, wxID_ANY, "Cross Bricks:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_accurate_bricks_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnAccurateBricksCheck, this);
	sizer1_1->Add(m_hidden_removal_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_accurate_bricks_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_select_group_chk, 0, wxALIGN_CENTER);
	//threshold4
	wxFlexGridSizer* sizer1_2 = new wxFlexGridSizer(4, 5, 10); // 4 columns, 5px hgap, 10px vgap
	sizer1_2->AddGrowableCol(2, 1); // Make the slider column growable
	st = new wxStaticText(page, 0, "Threshold:");
	m_brush_scl_translate_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_scl_translate_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushSclTranslateChange, this);
	m_brush_scl_translate_text = new wxTextCtrl(page, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_fp1);
	m_brush_scl_translate_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushSclTranslateText, this);
	sizer1_2->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_2->AddSpacer(1);
	sizer1_2->Add(m_brush_scl_translate_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer1_2->Add(m_brush_scl_translate_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	//gm falloff
	st = new wxStaticText(page, 0, "Edge Detect:");
	m_edge_detect_chk = new wxCheckBox(page, wxID_ANY, "");
	m_edge_detect_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushEdgeDetectChk, this);
	m_brush_gm_falloff_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_gm_falloff_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushGmFalloffChange, this);
	m_brush_gm_falloff_text = new wxTextCtrl(page, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_fp3);
	m_brush_gm_falloff_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushGmFalloffText, this);
	sizer1_2->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_2->Add(m_edge_detect_chk, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_brush_gm_falloff_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer1_2->Add(m_brush_gm_falloff_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	//2d
	st = new wxStaticText(page, 0, "Out Weight:");
	m_brush_2dinfl_sldr = new wxSingleSlider(page, wxID_ANY, 100, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_2dinfl_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrush2dinflChange, this);
	m_brush_2dinfl_text = new wxTextCtrl(page, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_fp2);
	m_brush_2dinfl_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrush2dinflText, this);
	sizer1_2->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_2->AddSpacer(1);
	sizer1_2->Add(m_brush_2dinfl_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer1_2->Add(m_brush_2dinfl_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	//sizer1
	sizer1->Add(5, 5);
	sizer1->Add(sizer1_1, 0, wxEXPAND);
	sizer1->Add(5, 5);
	sizer1->Add(sizer1_2, 0, wxEXPAND);
	sizer1->Add(5, 5);

	//Brush properties
	wxStaticBoxSizer *sizer2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Brush Properties");
	//size relation
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Unit Size:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_brush_size_data_rb = new wxRadioButton(page, wxID_ANY, "Data Voxel",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_brush_size_screen_rb = new wxRadioButton(page, wxID_ANY, "Display Pixel",
		wxDefaultPosition, wxDefaultSize);
	m_brush_size_data_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushSizeRelationCheck, this);
	m_brush_size_screen_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushSizeRelationCheck, this);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(st, 0, wxALIGN_CENTER);
	sizer2_1->Add(m_brush_size_data_rb, 0, wxALIGN_CENTER);
	sizer2_1->Add(15, 15);
	sizer2_1->Add(m_brush_size_screen_rb, 0, wxALIGN_CENTER);
	//align
	wxFlexGridSizer* sizer2_2 = new wxFlexGridSizer(4, 5, 10); // 4 columns, 5px hgap, 10px vgap
	sizer2_2->AddGrowableCol(2, 1); // Make the slider column growable
	//iterations
	st = new wxStaticText(page, 0, "Grow Rate:");
	m_brush_iter_sldr = new wxSingleSlider(page, wxID_ANY, 10, 0, 50,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_iter_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushIterChange, this);
	m_brush_iter_text = new wxTextCtrl(page, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_brush_iter_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushIterText, this);
	sizer2_2->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer2_2->AddSpacer(1);
	sizer2_2->Add(m_brush_iter_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer2_2->Add(m_brush_iter_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	//brush size 1
	st = new wxStaticText(page, 0, "Seed Size:");
	m_brush_size1_sldr = new wxSingleSlider(page, wxID_ANY, 10, 1, 300,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_size1_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushSize1Change, this);
	m_brush_size1_text = new wxTextCtrl(page, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_brush_size1_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushSize1Text, this);
	sizer2_2->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer2_2->AddSpacer(1);
	sizer2_2->Add(m_brush_size1_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer2_2->Add(m_brush_size1_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	//brush size 2
	st = new wxStaticText(page, 0, "Grow Size:");
	m_brush_size2_chk = new wxCheckBox(page, wxID_ANY, "");
	m_brush_size2_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushSize2Chk, this);
	m_brush_size2_sldr = new wxSingleSlider(page, wxID_ANY, 30, 1, 300,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_size2_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushSize2Change, this);
	m_brush_size2_text = new wxTextCtrl(page, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_brush_size2_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushSize2Text, this);
	sizer2_2->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer2_2->Add(m_brush_size2_chk, 0, wxALIGN_CENTER);
	sizer2_2->Add(m_brush_size2_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer2_2->Add(m_brush_size2_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	//note
	wxBoxSizer* sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"A selection brush has a center to set seeds and a grow region outside. " \
		"Brush sizes can also be set with mouse wheel in painting mode.");
	st->Wrap(FromDIP(450));
	sizer2_3->Add(st, 1, wxEXPAND);
	//sizer2
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_1, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_2, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_3, 0, wxEXPAND);
	sizer2->Add(5, 5);

	//sizer
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* BrushToolDlg::CreateListPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	//output
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_update_btn = new wxButton(page, wxID_ANY, "Update",
		wxDefaultPosition, wxDefaultSize);
	m_update_btn->Bind(wxEVT_BUTTON, &BrushToolDlg::OnUpdateBtn, this);
	m_history_chk = new wxCheckBox(page, wxID_ANY,
		"Hold History", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_history_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnHistoryChk, this);
	m_clear_hist_btn = new wxButton(page, wxID_ANY,
		"Clear History", wxDefaultPosition, wxDefaultSize);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &BrushToolDlg::OnClearHistBtn, this);
	sizer1->Add(m_update_btn, 0, wxALIGN_CENTER);
	sizer1->AddStretchSpacer(1);
	sizer1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(page, wxID_ANY);
	m_output_grid->CreateGrid(0, 5);
	m_output_grid->SetColLabelValue(0, "Voxel Count");
	m_output_grid->SetColLabelValue(1, "Voxel Count\n(Int. Weighted)");
	m_output_grid->SetColLabelValue(2, "Average\nIntensity");
	m_output_grid->SetColLabelValue(3, "Physical Size");
	m_output_grid->SetColLabelValue(4, "Physical Size\n(Int. Weighted)");
	//m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &BrushToolDlg::OnSelectCell, this);
	m_output_grid->Bind(wxEVT_KEY_DOWN, &BrushToolDlg::OnKeyDown, this);

	//sizer
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(5, 5);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(5, 5);
	sizer_v->Add(m_output_grid, 1, wxEXPAND);
	sizer_v->Add(5, 5);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* BrushToolDlg::CreateAlignPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxStaticText *st = 0;

	//alignment
	wxStaticBoxSizer *sizer1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Align Render View to Selection");
	wxBoxSizer* sizer11 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center_chk = new wxCheckBox(page, wxID_ANY,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_align_center_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnAlignCenterCheck, this);
	sizer11->Add(5, 5);
	sizer11->Add(m_align_center_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Tri Axes:",
		wxDefaultPosition, wxDefaultSize);
	m_align_xyz = new wxButton(page, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yxz = new wxButton(page, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zxy = new wxButton(page, ID_AlignZXY, "ZXY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xzy = new wxButton(page, ID_AlignXZY, "XZY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yzx = new wxButton(page, ID_AlignYZX, "YZX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zyx = new wxButton(page, ID_AlignZYX, "ZYX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xyz->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_yxz->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_zxy->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_xzy->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_yzx->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_zyx->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	sizer12->Add(5, 5);
	sizer12->Add(st, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	sizer12->Add(m_align_xyz, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	sizer12->Add(m_align_yxz, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	sizer12->Add(m_align_zxy, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	sizer12->Add(m_align_xzy, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	sizer12->Add(m_align_yzx, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	sizer12->Add(m_align_zyx, 0, wxALIGN_CENTER);
	sizer12->Add(5, 5);
	//
	sizer1->Add(10, 10);
	sizer1->Add(sizer11, 0, wxEXPAND);
	sizer1->Add(10, 10);
	sizer1->Add(sizer12, 0, wxEXPAND);
	sizer1->Add(10, 10);

	//sizer
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);

	page->SetSizer(sizer_v);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

void BrushToolDlg::EnableUndo(bool bval1, bool bval2)
{
	m_toolbar->EnableTool(ID_BrushUndo, bval1);
	m_toolbar->EnableTool(ID_BrushRedo, bval2);
}

void BrushToolDlg::ToggleBrushes(InteractiveMode int_mode, flrd::SelectMode sel_mode)
{
	m_toolbar->ToggleTool(ID_BrushGrow, int_mode == InteractiveMode::Grow);
	m_toolbar->ToggleTool(ID_BrushAppend, sel_mode == flrd::SelectMode::Append);
	m_toolbar->ToggleTool(ID_BrushComp, sel_mode == flrd::SelectMode::Segment);
	m_toolbar->ToggleTool(ID_BrushMesh, sel_mode == flrd::SelectMode::Mesh);
	m_toolbar->ToggleTool(ID_BrushSingle, sel_mode == flrd::SelectMode::SingleSelect);
	m_toolbar->ToggleTool(ID_BrushDiffuse, sel_mode == flrd::SelectMode::Diffuse);
	m_toolbar->ToggleTool(ID_BrushSolid, sel_mode == flrd::SelectMode::Solid);
	m_toolbar->ToggleTool(ID_BrushUnsel, sel_mode == flrd::SelectMode::Eraser);
}

void BrushToolDlg::EnableMask(bool bval)
{
	m_toolbar2->EnableTool(ID_MaskPaste, bval);
	m_toolbar2->EnableTool(ID_MaskMerge, bval);
	m_toolbar2->EnableTool(ID_MaskExclude, bval);
	m_toolbar2->EnableTool(ID_MaskIntersect, bval);
}

void BrushToolDlg::UpdateEdgeDetect(bool bval)
{
	m_edge_detect_chk->SetValue(bval);
	m_brush_gm_falloff_sldr->Enable(bval);
	m_brush_gm_falloff_text->Enable(bval);
}

void BrushToolDlg::UpdateHiddenRemoval(bool bval)
{
	m_hidden_removal_chk->SetValue(bval);
}

void BrushToolDlg::UpdateSelectGroup(bool bval)
{
	m_select_group_chk->SetValue(bval);
}

void BrushToolDlg::UpdateUpdateOrder(bool bval)
{
	m_accurate_bricks_chk->SetValue(bval);
}

void BrushToolDlg::UpdateBrushThreshold(double dval, double range)
{
	m_brush_scl_translate_sldr->SetRange(0, std::round(range * 10.0));
	m_brush_scl_translate_sldr->ChangeValue(std::round(dval * range * 10.0));
	m_brush_scl_translate_text->ChangeValue(wxString::Format("%.1f", dval * range));
}

void BrushToolDlg::UpdateBrushGmFalloff(double dval)
{
	m_brush_gm_falloff_sldr->ChangeValue(std::round(dval * 1000.0));
	m_brush_gm_falloff_text->ChangeValue(wxString::Format("%.3f", dval));
}

void BrushToolDlg::UpdateBrush2dInf(double dval)
{
	m_brush_2dinfl_sldr->ChangeValue(std::round(dval * 100.0));
	m_brush_2dinfl_text->ChangeValue(wxString::Format("%.2f", dval));
}

void BrushToolDlg::UpdateBrushSize1(double dval)
{
	m_brush_size1_sldr->ChangeValue(std::round(dval));
	m_brush_size1_text->ChangeValue(wxString::Format("%.0f", dval));
}

void BrushToolDlg::UpdateBrushSize2(bool bval, double dval)
{
	m_brush_size2_chk->SetValue(bval);
	m_brush_size2_sldr->Enable(bval);
	m_brush_size2_text->Enable(bval);
	m_brush_size2_sldr->ChangeValue(std::round(dval));
	m_brush_size2_text->ChangeValue(wxString::Format("%.0f", dval));
}

void BrushToolDlg::UpdateBrushIter(int ival)
{
	m_brush_iter_sldr->ChangeValue(ival);
	m_brush_iter_text->ChangeValue(wxString::Format("%d", ival));
}

void BrushToolDlg::UpdateBrushSizeRel(bool bval)
{
	m_brush_size_data_rb->SetValue(bval);
	m_brush_size_screen_rb->SetValue(!bval);
}

void BrushToolDlg::UpdateAlignCenter(bool bval)
{
	m_align_center_chk->SetValue(bval);
}

void BrushToolDlg::UpdateBrushHistoryEnable()
{
	m_history_chk->SetValue(m_hold_history);
}

void BrushToolDlg::CopyData()
{
	auto text =
		GridHelper::CopySelection(
			m_output_grid);

	if (text.empty())
		return;

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(
			new wxTextDataObject(text));

		wxTheClipboard->Close();
	}
}

void BrushToolDlg::UpdateGrid(const GridData& data)
{
	GridPopulateOptions options;
	options.append_rows = false;
	options.remove_extra_rows = !m_hold_history;
	options.remove_extra_cols = !m_hold_history;

	GridHelper::Populate(
		m_output_grid,
		data,
		options);

	m_output_grid->ClearSelection();
}

//brush commands
void BrushToolDlg::OnToolBar(wxCommandEvent& event)
{
	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (!agent)
		return;
	int id = event.GetId();
	fluo::ValueCollection vc;

	switch (id)
	{
	case ID_BrushUndo:
		vc.insert(gstSelUndo);
		break;
	case ID_BrushRedo:
		vc.insert(gstSelRedo);
		break;
	case ID_BrushGrow:
		vc.insert(gstBrushGrow);
		break;
	case ID_BrushAppend:
		vc.insert(gstBrushAppend);
		break;
	case ID_BrushComp:
		vc.insert(gstBrushComp);
		break;
	case ID_BrushMesh:
		vc.insert(gstBrushMesh);
		break;
	case ID_BrushSingle:
		vc.insert(gstBrushSingle);
		break;
	case ID_BrushDiffuse:
		vc.insert(gstBrushDiffuse);
		break;
	case ID_BrushSolid:
		vc.insert(gstBrushSolid);
		break;
	case ID_BrushUnsel:
		vc.insert(gstBrushUnsel);
		break;
	case ID_BrushClear:
		vc.insert(gstBrushClear);
		break;
	case ID_BrushExtract:
		vc.insert(gstBrushExtract);
		break;
	case ID_BrushDelete:
		vc.insert(gstBrushDelete);
		break;
	case ID_MaskCopy:
		vc.insert(gstMaskCopy);
		break;
	case ID_MaskCopyData:
		vc.insert(gstMaskCopyData);
		break;
	case ID_MaskPaste:
		vc.insert(gstMaskPaste);
		break;
	case ID_MaskMerge:
		vc.insert(gstMaskMerge);
		break;
	case ID_MaskExclude:
		vc.insert(gstMaskExclude);
		break;
	case ID_MaskIntersect:
		vc.insert(gstMaskIntersect);
		break;
	}
	agent->UpdateUIToData(vc);
}

//selection adjustment
//scalar translate
void BrushToolDlg::OnBrushSclTranslateChange(wxScrollEvent& event)
{
	int ival = m_brush_scl_translate_sldr->GetValue();
	double val = double(ival)/10.0;
	wxString str = wxString::Format("%.1f", val);
	if (str != m_brush_scl_translate_text->GetValue())
		m_brush_scl_translate_text->SetValue(str);
}

void BrushToolDlg::OnBrushSclTranslateText(wxCommandEvent& event)
{
	wxString str = m_brush_scl_translate_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_brush_scl_translate_sldr->ChangeValue(std::round(val*10.0));

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushSclTranslate(val);

	LaunchAutoUpdateTimer();
}

//gm falloff
void BrushToolDlg::OnBrushGmFalloffChange(wxScrollEvent& event)
{
	int ival = m_brush_gm_falloff_sldr->GetValue();
	double val = double(ival) / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	if (str != m_brush_gm_falloff_text->GetValue())
		m_brush_gm_falloff_text->SetValue(str);
}

void BrushToolDlg::OnBrushGmFalloffText(wxCommandEvent& event)
{
	wxString str = m_brush_gm_falloff_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_brush_gm_falloff_sldr->ChangeValue(std::round(val*1000.0));

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushGmFalloff(val);

	LaunchAutoUpdateTimer();
}

//2d influence
void BrushToolDlg::OnBrush2dinflChange(wxScrollEvent& event)
{
	int ival = m_brush_2dinfl_sldr->GetValue();
	double val = double(ival)/100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_brush_2dinfl_text->GetValue())
		m_brush_2dinfl_text->SetValue(str);
}

void BrushToolDlg::OnBrush2dinflText(wxCommandEvent& event)
{
	wxString str = m_brush_2dinfl_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_brush_2dinfl_sldr->ChangeValue(std::round(val*100.0));

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetW2d(val);

	LaunchAutoUpdateTimer();
}

//edge detect
void BrushToolDlg::OnBrushEdgeDetectChk(wxCommandEvent& event)
{
	bool bval = m_edge_detect_chk->GetValue();

	m_brush_gm_falloff_sldr->Enable(bval);
	m_brush_gm_falloff_text->Enable(bval);

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetEdgeDetect(bval);

	LaunchAutoUpdateTimer();
}

//hidden removal
void BrushToolDlg::OnBrushHiddenRemovalChk(wxCommandEvent& event)
{
	bool bval = m_hidden_removal_chk->GetValue();

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetHiddenRemoval(bval);
}

//select group
void BrushToolDlg::OnBrushSelectGroupChk(wxCommandEvent& event)
{
	bool bval = m_select_group_chk->GetValue();

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetSelectGroup(bval);

	LaunchAutoUpdateTimer();
}

//brick accuracy
void BrushToolDlg::OnAccurateBricksCheck(wxCommandEvent& event)
{
	bool bval = m_accurate_bricks_chk->GetValue();

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetUpdateOrder(bval);
}

//brush size 1
void BrushToolDlg::OnBrushSize1Change(wxScrollEvent& event)
{
	int ival = m_brush_size1_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_brush_size1_text->GetValue())
		m_brush_size1_text->SetValue(str);
}

void BrushToolDlg::OnBrushSize1Text(wxCommandEvent& event)
{
	wxString str = m_brush_size1_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_brush_size1_sldr->ChangeValue(std::round(dval));

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushSize1(dval);
}

//brush size 2
void BrushToolDlg::OnBrushSize2Chk(wxCommandEvent& event)
{
	wxString str = m_brush_size1_text->GetValue();
	double dval1;
	str.ToDouble(&dval1);
	str = m_brush_size2_text->GetValue();
	double dval2;
	str.ToDouble(&dval2);

	bool bval = m_brush_size2_chk->GetValue();
	m_brush_size2_sldr->Enable(bval);
	m_brush_size2_text->Enable(bval);

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushSize2Enable(bval, dval1, dval2);
}

void BrushToolDlg::OnBrushSize2Change(wxScrollEvent& event)
{
	int ival = m_brush_size2_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_brush_size2_text->GetValue())
		m_brush_size2_text->SetValue(str);
}

void BrushToolDlg::OnBrushSize2Text(wxCommandEvent& event)
{
	wxString str = m_brush_size2_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_brush_size2_sldr->ChangeValue(std::round(dval));

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushSize2(dval);
}

//brush iterations
void BrushToolDlg::OnBrushIterChange(wxScrollEvent& event)
{
	int ival = m_brush_iter_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_brush_iter_text->GetValue())
		m_brush_iter_text->SetValue(str);
}

void BrushToolDlg::OnBrushIterText(wxCommandEvent& event)
{
	wxString str = m_brush_iter_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_brush_iter_sldr->ChangeValue(ival);

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushIteration(ival);

	LaunchAutoUpdateTimer();
}

//brush size relation
void BrushToolDlg::OnBrushSizeRelationCheck(wxCommandEvent& event)
{
	bool bval1 = m_brush_size_data_rb->GetValue();
	bool bval2 = m_brush_size_screen_rb->GetValue();
	bool bval = bval1 == bval2 ? true : bval1;

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetBrushSizeData(bval);
}

//align
void BrushToolDlg::OnAlignCenterCheck(wxCommandEvent& event)
{
	bool bval = m_align_center_chk->GetValue();

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->SetAlignCenter(bval);
}

void BrushToolDlg::OnAlignPca(wxCommandEvent& event)
{
	int ival = event.GetId();
	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
	{
		agent->SetAlignAxis(ival);
		agent->UpdateUIToData({ gstAlignPca });
	}
}

void BrushToolDlg::OnUpdateBtn(wxCommandEvent& event)
{
	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->UpdateDataToUI({ gstBrushCountResult });
}

void BrushToolDlg::OnHistoryChk(wxCommandEvent& event)
{
	m_hold_history = m_history_chk->GetValue();
}

void BrushToolDlg::OnClearHistBtn(wxCommandEvent& event)
{
	m_output_grid->DeleteRows(0, m_output_grid->GetNumberRows());
}

void BrushToolDlg::OnKeyDown(wxKeyEvent& event)
{
	if (wxGetKeyState(WXK_CONTROL))
	{
		if (event.GetKeyCode() == wxKeyCode('C'))
			CopyData();
		//else if (event.GetKeyCode() == wxKeyCode('V'))
		//	PasteData();
	}
}

void BrushToolDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
}

void BrushToolDlg::OnSize(wxSizeEvent& event)
{
	if (!m_output_grid)
		return;

	wxSize size = GetSize();
	wxPoint p1 = GetScreenPosition();
	wxPoint p2 = m_output_grid->GetScreenPosition();
	int height, margin;
	if (m_output_grid->GetNumberRows())
		height = m_output_grid->GetRowSize(0) * 8;
	else
		height = 80;
	margin = size.y + p1.y - p2.y - 20;
	if (margin > height)
		size.y = margin;
	else
		size.y = height;
	size.x -= 15;
	m_output_grid->SetMaxSize(size);
}

//auto update
void BrushToolDlg::LaunchAutoUpdateTimer()
{
	m_auto_update_timer.Start(100);
}

void BrushToolDlg::OnAutoUpdateTimer(wxTimerEvent& event)
{
	m_auto_update_timer.Stop();

	auto agent = m_agent->As<BrushToolDlgAgent>();
	if (agent)
		agent->UpdateUIToData({ gstTimerSegment });
}

