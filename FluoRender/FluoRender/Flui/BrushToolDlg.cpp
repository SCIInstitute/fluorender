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
#include <BrushToolDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <NoiseCancellingDlg.h>
#include <CountingDlg.h>
#include <TreePanel.h>
#include <Count.h>
#include <Texture.h>
#include <VolumeSelector.h>
#include <Colocalize.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <RulerAlign.h>
#include <BrushDefault.h>
#include <GlobalStates.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>
//resources
#include <png_resource.h>
#include <icons.h>

#define GM_2_ESTR(x) (1.0 - sqrt(1.0 - (x - 1.0) * (x - 1.0)))

BrushToolDlg::BrushToolDlg(
	MainFrame *frame):
	TabbedPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 620)),
	0, "BrushToolDlg"),
	m_max_value(255.0),
	m_hold_history(false)
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
	m_toolbar->AddCheckTool(ID_BrushComp, "Segment",
		bitmap, wxNullBitmap,
		"Select structures and then segment them into components",
		"Select structures and then segment them into components");
	bitmap = wxGetBitmap(brush_single);
	m_toolbar->AddCheckTool(ID_BrushSingle, "Isolate",
		bitmap, wxNullBitmap,
		"Select and isolate a structure by painting",
		"Select and isolate a structure by painting");
	bitmap = wxGetBitmap(brush_diffuse);
	m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
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

void BrushToolDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	auto sel_vol = glbin_current.vol_data.lock();

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty() || FOUND_VALUE(gstCurrentSelect);

	double dval = 0.0;
	int ival = 0;
	bool bval = false;
	//threshold range
	if (sel_vol)
		m_max_value = sel_vol->GetMaxValue();

	if (update_all || FOUND_VALUE(gstSelUndo) || FOUND_VALUE(gstCurrentSelect))
	{
		if (sel_vol && sel_vol->GetTexture())
		{
			m_toolbar->EnableTool(ID_BrushUndo,
				sel_vol->GetTexture()->get_undo());
			m_toolbar->EnableTool(ID_BrushRedo,
				sel_vol->GetTexture()->get_redo());
		}
		else
		{
			m_toolbar->EnableTool(ID_BrushUndo, false);
			m_toolbar->EnableTool(ID_BrushRedo, false);
		}
	}

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		m_toolbar->ToggleTool(ID_BrushGrow, sel_mode == flrd::SelectMode::Grow);
		m_toolbar->ToggleTool(ID_BrushAppend, sel_mode == flrd::SelectMode::Append);
		m_toolbar->ToggleTool(ID_BrushComp, sel_mode == flrd::SelectMode::Segment);
		m_toolbar->ToggleTool(ID_BrushSingle, sel_mode == flrd::SelectMode::SingleSelect);
		m_toolbar->ToggleTool(ID_BrushDiffuse, sel_mode == flrd::SelectMode::Diffuse);
		m_toolbar->ToggleTool(ID_BrushSolid, sel_mode == flrd::SelectMode::Solid);
		m_toolbar->ToggleTool(ID_BrushUnsel, sel_mode == flrd::SelectMode::Eraser);
	}

	if (update_all || FOUND_VALUE(gstSelMask) || FOUND_VALUE(gstCurrentSelect))
	{
		bval = glbin_vol_selector.GetCopyMaskVolume() != 0;
		m_toolbar2->EnableTool(ID_MaskPaste, bval);
		m_toolbar2->EnableTool(ID_MaskMerge, bval);
		m_toolbar2->EnableTool(ID_MaskExclude, bval);
		m_toolbar2->EnableTool(ID_MaskIntersect, bval);
	}

	if (update_all || FOUND_VALUE(gstSelOptions))
	{
		//edge detect
		bval = glbin_vol_selector.GetEdgeDetect();
		m_edge_detect_chk->SetValue(bval);
		m_brush_gm_falloff_sldr->Enable(bval);
		m_brush_gm_falloff_text->Enable(bval);
		//hidden removal
		bval = glbin_vol_selector.GetHiddenRemoval();
		m_hidden_removal_chk->SetValue(bval);
		//select group
		bval = glbin_vol_selector.GetSelectGroup();
		m_select_group_chk->SetValue(bval);
		//brick acuracy
		bval = glbin_vol_selector.GetUpdateOrder();
		m_accurate_bricks_chk->SetValue(bval);
	}

	//selection strength
	if (update_all || FOUND_VALUE(gstBrushThreshold))
	{
		m_brush_scl_translate_sldr->SetRange(0, std::round(m_max_value * 10.0));
		flrd::VolumeSelector* vs = &glbin_vol_selector;
		dval = glbin_vol_selector.GetBrushSclTranslate();
		m_brush_scl_translate_sldr->ChangeValue(std::round(dval * m_max_value * 10.0));
		m_brush_scl_translate_text->ChangeValue(wxString::Format("%.1f", dval * m_max_value));
	}

	//gm falloff
	if (update_all || FOUND_VALUE(gstBrushGmFalloff))
	{
		dval = glbin_vol_selector.GetBrushGmFalloff();
		m_brush_gm_falloff_sldr->ChangeValue(std::round(GM_2_ESTR(dval) * 1000.0));
		m_brush_gm_falloff_text->ChangeValue(wxString::Format("%.3f", GM_2_ESTR(dval)));
	}

	//2d influence
	if (update_all || FOUND_VALUE(gstBrush2dInf))
	{
		dval = glbin_vol_selector.GetW2d();
		m_brush_2dinfl_sldr->ChangeValue(std::round(dval * 100.0));
		m_brush_2dinfl_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	//size1
	if (update_all || FOUND_VALUE(gstBrushSize1))
	{
		dval = glbin_vol_selector.GetBrushSize1();
		m_brush_size1_sldr->ChangeValue(std::round(dval));
		m_brush_size1_text->ChangeValue(wxString::Format("%.0f", dval));
	}

	//size2
	if (update_all || FOUND_VALUE(gstBrushSize2))
	{
		bval = glbin_vol_selector.GetUseBrushSize2();
		m_brush_size2_chk->SetValue(bval);
		m_brush_size2_sldr->Enable(bval);
		m_brush_size2_text->Enable(bval);
		dval = glbin_vol_selector.GetBrushSize2();
		m_brush_size2_sldr->ChangeValue(std::round(dval));
		m_brush_size2_text->ChangeValue(wxString::Format("%.0f", dval));
	}

	//iteration number
	if (update_all || FOUND_VALUE(gstBrushIter))
	{
		ival = glbin_vol_selector.GetBrushIteration();
		m_brush_iter_sldr->ChangeValue(ival);
		m_brush_iter_text->ChangeValue(wxString::Format("%d", ival));
	}

	//brush size relation
	if (update_all || FOUND_VALUE(gstBrushSizeRel))
	{
		bval = glbin_vol_selector.GetBrushSizeData();
		m_brush_size_data_rb->SetValue(bval);
		m_brush_size_screen_rb->SetValue(!bval);
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		m_align_center_chk->SetValue(bval);
	}

	//output
	if (update_all || FOUND_VALUE(gstBrushHistoryEnable))
		m_history_chk->SetValue(m_hold_history);

	if (sel_vol && FOUND_VALUE(gstBrushCountResult))
	{
		GridData data;
		flrd::CountVoxels counter;
		counter.SetVolumeData(sel_vol);
		counter.Count();
		data.voxel_sum = counter.GetSum();
		double scale = sel_vol->GetScalarScale();
		data.voxel_wsum = counter.GetWeightedSum() * scale;
		if (data.voxel_sum)
		{
			data.avg_int = data.voxel_wsum / data.voxel_sum;
			if (sel_vol->GetBits() == 8)
				data.avg_int *= 255.0;
			else if (sel_vol->GetBits() == 16)
				data.avg_int *= sel_vol->GetMaxValue();
		}
		double spcx, spcy, spcz;
		sel_vol->GetSpacings(spcx, spcy, spcz);
		double vvol = spcx * spcy * spcz;
		vvol = vvol == 0.0 ? 1.0 : vvol;
		data.size = data.voxel_sum * vvol;
		data.wsize = data.voxel_wsum * vvol;
		wxString unit;
		auto view = glbin_current.render_view.lock();
		if (view)
		{
			switch (view->m_sb_unit)
			{
			case 0:
				unit = L"nm\u00B3";
				break;
			case 1:
			default:
				unit = L"\u03BCm\u00B3";
				break;
			case 2:
				unit = L"mm\u00B3";
				break;
			}
		}
		SetOutput(data, unit);
	}

	if (FOUND_VALUE(gstBrushSpeedResult))
	{
		if (glbin_vol_selector.m_test_speed)
		{
			GridData data;
			data.size = glbin_vol_selector.GetSpanSec();
			data.wsize = data.size;
			wxString unit = "Sec.";
			SetOutput(data, unit);
		}
	}
}

void BrushToolDlg::BrushUndo()
{
	glbin_vol_selector.UndoMask();
	FluoRefresh(2, { gstSelUndo });
}

void BrushToolDlg::BrushRedo()
{
	glbin_vol_selector.RedoMask();
	FluoRefresh(2, { gstSelUndo });
}

void BrushToolDlg::BrushGrow()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Grow);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void BrushToolDlg::BrushAppend()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Append);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void BrushToolDlg::BrushComp()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Segment);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void BrushToolDlg::BrushSingle()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::SingleSelect);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void BrushToolDlg::BrushDiffuse()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Diffuse);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void BrushToolDlg::BrushSolid()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Solid);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

void BrushToolDlg::BrushUnsel()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Eraser);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter }, {-1});
}

//toolbar2
void BrushToolDlg::BrushClear()
{
	glbin_vol_selector.Clear();
	FluoRefresh(3, { gstNull });
}

void BrushToolDlg::BrushExtract()
{
	glbin_vol_selector.Extract();
	FluoRefresh(0, { gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
}

void BrushToolDlg::BrushDelete()
{
	glbin_vol_selector.Erase();
	FluoRefresh(0, { gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
}

void BrushToolDlg::MaskCopy()
{
	glbin_vol_selector.CopyMask(false);
	FluoRefresh(2, { gstSelMask });
}

void BrushToolDlg::MaskCopyData()
{
	glbin_vol_selector.CopyMask(true);
	FluoRefresh(2, { gstSelMask });
}

void BrushToolDlg::MaskPaste()
{
	fluo::ValueCollection vc;
	glbin_vol_selector.PasteMask(0);
	vc.insert(gstSelUndo);
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
		vc.insert(gstColocalResult);
	FluoRefresh(0, vc);
}

void BrushToolDlg::MaskMerge()
{
	fluo::ValueCollection vc;
	glbin_vol_selector.PasteMask(1);
	vc.insert(gstSelUndo);
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
		vc.insert(gstColocalResult);
	FluoRefresh(0, vc);
}

void BrushToolDlg::MaskExclude()
{
	fluo::ValueCollection vc;
	glbin_vol_selector.PasteMask(2);
	vc.insert(gstSelUndo);
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
		vc.insert(gstColocalResult);
	FluoRefresh(0, vc);
}

void BrushToolDlg::MaskIntersect()
{
	fluo::ValueCollection vc;
	glbin_vol_selector.PasteMask(3);
	vc.insert(gstSelUndo);
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
		vc.insert(gstColocalResult);
	FluoRefresh(0, vc);
}

//brush commands
void BrushToolDlg::OnToolBar(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id)
	{
	case ID_BrushUndo:
		BrushUndo();
		break;
	case ID_BrushRedo:
		BrushRedo();
		break;
	case ID_BrushGrow:
		BrushGrow();
		break;
	case ID_BrushAppend:
		BrushAppend();
		break;
	case ID_BrushComp:
		BrushComp();
		break;
	case ID_BrushSingle:
		BrushSingle();
		break;
	case ID_BrushDiffuse:
		BrushDiffuse();
		break;
	case ID_BrushSolid:
		BrushSolid();
		break;
	case ID_BrushUnsel:
		BrushUnsel();
		break;
	case ID_BrushClear:
		BrushClear();
		break;
	case ID_BrushExtract:
		BrushExtract();
		break;
	case ID_BrushDelete:
		BrushDelete();
		break;
	case ID_MaskCopy:
		MaskCopy();
		break;
	case ID_MaskCopyData:
		MaskCopyData();
		break;
	case ID_MaskPaste:
		MaskPaste();
		break;
	case ID_MaskMerge:
		MaskMerge();
		break;
	case ID_MaskExclude:
		MaskExclude();
		break;
	case ID_MaskIntersect:
		MaskIntersect();
		break;
	}
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

	//set translate
	glbin_vol_selector.SetBrushSclTranslate(val / m_max_value);

	if (!glbin_vol_selector.GetThUpdate())
		return;

	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	fluo::ValueCollection vc;
	int sx = 2;
	vc.insert({ gstSelUndo, gstBrushThreshold });
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
	{
		vc.insert(gstColocalResult);
		sx = 0;
	}
	FluoRefresh(sx, vc, { glbin_current.GetViewId() });
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

	//set gm falloff
	glbin_vol_selector.SetBrushGmFalloff(GM_2_ESTR(val));

	if (!glbin_vol_selector.GetThUpdate())
		return;

	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	fluo::ValueCollection vc;
	int sx = 2;
	vc.insert({ gstSelUndo, gstBrushThreshold });
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
	{
		vc.insert(gstColocalResult);
		sx = 0;
	}
	FluoRefresh(sx, vc, { glbin_current.GetViewId() });
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

	//set 2d weight
	glbin_vol_selector.SetW2d(val);

	if (!glbin_vol_selector.GetThUpdate())
		return;

	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	fluo::ValueCollection vc;
	int sx = 2;
	vc.insert({ gstSelUndo, gstBrushThreshold });
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
	{
		vc.insert(gstColocalResult);
		sx = 0;
	}
	FluoRefresh(sx, vc, { glbin_current.GetViewId() });
}

//edge detect
void BrushToolDlg::OnBrushEdgeDetectChk(wxCommandEvent& event)
{
	bool bval = m_edge_detect_chk->GetValue();

	m_brush_gm_falloff_sldr->Enable(bval);
	m_brush_gm_falloff_text->Enable(bval);

	//set edge detect
	glbin_vol_selector.SetEdgeDetect(bval);

	if (!glbin_vol_selector.GetThUpdate())
		return;

	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	fluo::ValueCollection vc;
	int sx = 2;
	vc.insert({ gstSelUndo, gstBrushThreshold });
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
	{
		vc.insert(gstColocalResult);
		sx = 0;
	}
	FluoRefresh(sx, vc, { glbin_current.GetViewId() });
}

//hidden removal
void BrushToolDlg::OnBrushHiddenRemovalChk(wxCommandEvent& event)
{
	bool hidden_removal = m_hidden_removal_chk->GetValue();

	//set hidden removal
	glbin_vol_selector.SetHiddenRemoval(hidden_removal);
}

//select group
void BrushToolDlg::OnBrushSelectGroupChk(wxCommandEvent& event)
{
	bool select_group = m_select_group_chk->GetValue();

	//set select group
	glbin_vol_selector.SetSelectGroup(select_group);

	if (!glbin_vol_selector.GetThUpdate())
		return;

	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	fluo::ValueCollection vc;
	int sx = 2;
	vc.insert({ gstSelUndo, gstBrushThreshold });
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
	{
		vc.insert(gstColocalResult);
		sx = 0;
	}
	FluoRefresh(sx, vc, { glbin_current.GetViewId() });
}

//estimate threshold
//void BrushToolDlg::OnEstimateThreshChk(wxCommandEvent& event)
//{
//	bool value = m_estimate_thresh_chk->GetValue();
//
//	glbin_vol_selector.SetEstimateThreshold(value);
//}

//brick accuracy
void BrushToolDlg::OnAccurateBricksCheck(wxCommandEvent& event)
{
	bool value = m_accurate_bricks_chk->GetValue();

	glbin_vol_selector.SetUpdateOrder(value);
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
	double val;
	str.ToDouble(&val);
	m_brush_size1_sldr->ChangeValue(std::round(val));

	//set size1
	glbin_vol_selector.SetBrushSize(val, -1.0);
	FluoRefresh(3, { gstNull });
}

//brush size 2
void BrushToolDlg::OnBrushSize2Chk(wxCommandEvent& event)
{
	wxString str = m_brush_size1_text->GetValue();
	double val1;
	str.ToDouble(&val1);
	str = m_brush_size2_text->GetValue();
	double val2;
	str.ToDouble(&val2);

	bool bval = m_brush_size2_chk->GetValue();
	m_brush_size2_sldr->Enable(bval);
	m_brush_size2_text->Enable(bval);
	glbin_vol_selector.SetUseBrushSize2(bval);
	glbin_vol_selector.SetBrushSize(val1, val2);
	FluoRefresh(3, { gstNull });
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
	double val;
	str.ToDouble(&val);
	m_brush_size2_sldr->ChangeValue(std::round(val));

	//set size2
	glbin_vol_selector.SetBrushSize(-1.0, val);
	FluoRefresh(3, { gstNull });
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
	long val;
	str.ToLong(&val);
	m_brush_iter_sldr->ChangeValue(val);

	glbin_vol_selector.SetBrushIteration(val);

	if (!glbin_vol_selector.GetThUpdate())
		return;

	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	fluo::ValueCollection vc;
	int sx = 2;
	vc.insert({ gstSelUndo, gstBrushThreshold });
	if (glbin_vol_selector.GetAutoPaintSize())
		vc.insert(gstBrushCountResult);
	if (glbin_colocalizer.GetAutoColocalize())
	{
		vc.insert(gstColocalResult);
		sx = 0;
	}
	FluoRefresh(sx, vc, { glbin_current.GetViewId() });
}

//brush size relation
void BrushToolDlg::OnBrushSizeRelationCheck(wxCommandEvent& event)
{
	if (m_brush_size_data_rb->GetValue())
	{
		glbin_vol_selector.SetBrushSizeData(true);
	}
	else if (m_brush_size_screen_rb->GetValue())
	{
		glbin_vol_selector.SetBrushSizeData(false);
	}
}

//align
void BrushToolDlg::OnAlignCenterCheck(wxCommandEvent& event)
{
	bool bval = m_align_center_chk->GetValue();
	glbin_aligner.SetAlignCenter(bval);
	FluoRefresh(1, { gstAlignCenter }, { -1 });
}

void BrushToolDlg::OnAlignPca(wxCommandEvent& event)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	glbin_aligner.SetVolumeData(vd);
	glbin_aligner.SetAxisType(event.GetId());
	glbin_aligner.SetView(glbin_current.render_view.lock());
	glbin_aligner.AlignPca(false);
	FluoRefresh(3, { gstNull }, { glbin_current.GetViewId()});
}

//output
void BrushToolDlg::SetOutput(const GridData &data, const wxString &unit)
{
	if (m_output_grid->GetNumberRows()==0 ||
		m_hold_history)
	{
		m_output_grid->InsertRows();
	}
	m_output_grid->SetCellValue(0, 0,
		wxString::Format("%d", data.voxel_sum));
	m_output_grid->SetCellValue(0, 1,
		wxString::Format("%f", data.voxel_wsum));
	m_output_grid->SetCellValue(0, 2,
		wxString::Format("%f", data.avg_int));
	m_output_grid->SetCellValue(0, 3,
		wxString::Format("%f", data.size) + unit);
	m_output_grid->SetCellValue(0, 4,
		wxString::Format("%f", data.wsize) + unit);
	//m_output_grid->Fit();
	//m_output_grid->AutoSizeColumns();
	m_output_grid->ClearSelection();
}

void BrushToolDlg::OnUpdateBtn(wxCommandEvent& event)
{
	FluoUpdate({ gstBrushCountResult });
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
		else if (event.GetKeyCode() == wxKeyCode('V'))
			PasteData();
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

void BrushToolDlg::CopyData()
{
	int i, k;
	wxString copy_data;
	bool something_in_this_line;

	copy_data.Clear();

	bool t = m_output_grid->IsSelection();

	for (i = 0; i < m_output_grid->GetNumberRows(); i++)
	{
		something_in_this_line = false;
		for (k = 0; k < m_output_grid->GetNumberCols(); k++)
		{
			if (m_output_grid->IsInSelection(i, k))
			{
				if (something_in_this_line == false)
				{  // first field in this line => may need a linefeed
					if (copy_data.IsEmpty() == false)
					{     // ... if it is not the very first field
						copy_data = copy_data + wxT("\n");  // next LINE
					}
					something_in_this_line = true;
				}
				else
				{
					// if not the first field in this line we need a field seperator (TAB)
					copy_data = copy_data + wxT("\t");  // next COLUMN
				}
				copy_data = copy_data + m_output_grid->GetCellValue(i, k);    // finally we need the field value :-)
			}
		}
	}

	if (wxTheClipboard->Open())
	{
		// This data objects are held by the clipboard,
		// so do not delete them in the app.
		wxTheClipboard->SetData(new wxTextDataObject(copy_data));
		wxTheClipboard->Close();
	}
}

void BrushToolDlg::PasteData()
{
/*	wxString copy_data;
	wxString cur_field;
	wxString cur_line;
	int i, k, k2;

	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_TEXT))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			copy_data = data.GetText();
		}
		wxTheClipboard->Close();
	}

	i = m_output_grid->GetGridCursorRow();
	k = m_output_grid->GetGridCursorCol();
	k2 = k;

	do
	{
		cur_line = copy_data.BeforeFirst('\n');
		copy_data = copy_data.AfterFirst('\n');
		do
		{
			cur_field = cur_line.BeforeFirst('\t');
			cur_line = cur_line.AfterFirst('\t');
			m_output_grid->SetCellValue(i, k, cur_field);
			k++;
		} while (cur_line.IsEmpty() == false);
		i++;
		k = k2;
	} while (copy_data.IsEmpty() == false);
*/
}

