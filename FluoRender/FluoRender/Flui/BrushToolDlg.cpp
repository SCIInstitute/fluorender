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
	PropPanel(frame, frame,
	wxDefaultPosition,
	frame->FromDIP(wxSize(500, 620)),
	0, "BrushToolDlg"),
	m_max_value(255.0),
	m_hold_history(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

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
	m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
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
	m_toolbar2 = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
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
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Selection Settings"),
		wxVERTICAL);
	//stop at boundary
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_edge_detect_chk = new wxCheckBox(this, wxID_ANY, "Edge Detect:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_edge_detect_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushEdgeDetectChk, this);
	m_hidden_removal_chk = new wxCheckBox(this, wxID_ANY, "Visible Only:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_hidden_removal_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushHiddenRemovalChk, this);
	m_select_group_chk = new wxCheckBox(this, wxID_ANY, "Apply to Group:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_select_group_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushSelectGroupChk, this);
	m_accurate_bricks_chk = new wxCheckBox(this, wxID_ANY, "Cross Bricks:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_accurate_bricks_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnAccurateBricksCheck, this);
	sizer1_1->Add(m_edge_detect_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_hidden_removal_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_accurate_bricks_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_select_group_chk, 0, wxALIGN_CENTER);
	//threshold4
	wxBoxSizer *sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_brush_scl_translate_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_scl_translate_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushSclTranslateChange, this);
	m_brush_scl_translate_text = new wxTextCtrl(this, wxID_ANY, "0.0",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_fp1);
	m_brush_scl_translate_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushSclTranslateText, this);
	sizer1_2->Add(5, 5);
	sizer1_2->Add(st, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_brush_scl_translate_sldr, 1, wxEXPAND);
	sizer1_2->Add(m_brush_scl_translate_text, 0, wxALIGN_CENTER);
	sizer1_2->Add(15, 15);
	//gm falloff
	wxBoxSizer *sizer1_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Edge STR:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_brush_gm_falloff_sldr = new wxSingleSlider(this, wxID_ANY, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_gm_falloff_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushGmFalloffChange, this);
	m_brush_gm_falloff_text = new wxTextCtrl(this, wxID_ANY, "0.000",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_fp3);
	m_brush_gm_falloff_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushGmFalloffText, this);
	sizer1_3->Add(5, 5);
	sizer1_3->Add(st, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_brush_gm_falloff_sldr, 1, wxEXPAND);
	sizer1_3->Add(m_brush_gm_falloff_text, 0, wxALIGN_CENTER);
	sizer1_3->Add(15, 15);
	//2d
	wxBoxSizer *sizer1_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "2D Adj. Infl.:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_brush_2dinfl_sldr = new wxSingleSlider(this, wxID_ANY, 100, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_INVERSE);
	m_brush_2dinfl_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrush2dinflChange, this);
	m_brush_2dinfl_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_fp2);
	m_brush_2dinfl_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrush2dinflText, this);
	sizer1_4->Add(5, 5);
	sizer1_4->Add(st, 0, wxALIGN_CENTER);
	sizer1_4->Add(m_brush_2dinfl_sldr, 1, wxEXPAND);
	sizer1_4->Add(m_brush_2dinfl_text, 0, wxALIGN_CENTER);
	sizer1_4->Add(15, 15);
	//sizer1
	sizer1->Add(5, 5);
	sizer1->Add(sizer1_1, 0, wxEXPAND);
	sizer1->Add(5, 5);
	sizer1->Add(sizer1_2, 0, wxEXPAND);
	sizer1->Add(sizer1_3, 0, wxEXPAND);
	sizer1->Add(sizer1_4, 0, wxEXPAND);
	sizer1->Hide(sizer1_4, true);
	sizer1->Add(5, 5);

	//Brush properties
	wxBoxSizer *sizer2 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Brush Properties"),
		wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Brush sizes can also be set with mouse wheel in painting mode.");
	sizer2_1->Add(5, 5);
	sizer2_1->Add(st, 0, wxALIGN_CENTER);
	//brush size 1
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Center Size:",
		wxDefaultPosition, FromDIP(wxSize(80, 20)), wxALIGN_RIGHT);
	m_brush_size1_sldr = new wxSingleSlider(this, wxID_ANY, 10, 1, 300,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_size1_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushSize1Change, this);
	m_brush_size1_text = new wxTextCtrl(this, wxID_ANY, "10",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_int);
	m_brush_size1_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushSize1Text, this);
	sizer2_2->Add(5, 5);
	sizer2_2->Add(st, 0, wxALIGN_CENTER);
	sizer2_2->Add(m_brush_size1_sldr, 1, wxEXPAND);
	sizer2_2->Add(m_brush_size1_text, 0, wxALIGN_CENTER);
	sizer2_2->Add(15, 15);
	//brush size 2
	wxBoxSizer *sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	m_brush_size2_chk = new wxCheckBox(this, wxID_ANY, "GrowSize",
		wxDefaultPosition, FromDIP(wxSize(80, 20)), wxALIGN_RIGHT);
	m_brush_size2_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnBrushSize2Chk, this);
	st = new wxStaticText(this, 0, ":",
		wxDefaultPosition, FromDIP(wxSize(5, 20)), wxALIGN_RIGHT);
	m_brush_size2_sldr = new wxSingleSlider(this, wxID_ANY, 30, 1, 300,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_size2_sldr->Bind(wxEVT_SCROLL_CHANGED, &BrushToolDlg::OnBrushSize2Change, this);
	m_brush_size2_text = new wxTextCtrl(this, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_int);
	m_brush_size2_text->Bind(wxEVT_TEXT, &BrushToolDlg::OnBrushSize2Text, this);
	sizer2_3->Add(m_brush_size2_chk, 0, wxALIGN_CENTER);
	sizer2_3->Add(st, 0, wxALIGN_CENTER);
	sizer2_3->Add(m_brush_size2_sldr, 1, wxEXPAND);
	sizer2_3->Add(m_brush_size2_text, 0, wxALIGN_CENTER);
	sizer2_3->Add(15, 15);
	//iterations
	wxBoxSizer *sizer2_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Growth:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_brush_iterw_rb = new wxRadioButton(this, wxID_ANY, "Weak",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_brush_iters_rb = new wxRadioButton(this, wxID_ANY, "Normal",
		wxDefaultPosition, wxDefaultSize);
	m_brush_iterss_rb = new wxRadioButton(this, wxID_ANY, "Strong",
		wxDefaultPosition, wxDefaultSize);
	m_brush_iterw_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushIterCheck, this);
	m_brush_iters_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushIterCheck, this);
	m_brush_iterss_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushIterCheck, this);
	sizer2_4->Add(5, 5);
	sizer2_4->Add(st, 0, wxALIGN_CENTER);
	sizer2_4->Add(m_brush_iterw_rb, 0, wxALIGN_CENTER);
	sizer2_4->Add(15, 15);
	sizer2_4->Add(m_brush_iters_rb, 0, wxALIGN_CENTER);
	sizer2_4->Add(15, 15);
	sizer2_4->Add(m_brush_iterss_rb, 0, wxALIGN_CENTER);
	//size relation
	wxBoxSizer *sizer2_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Dependent:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_brush_size_data_rb = new wxRadioButton(this, wxID_ANY, "Data",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_brush_size_screen_rb = new wxRadioButton(this, wxID_ANY, "Screen",
		wxDefaultPosition, wxDefaultSize);
	m_brush_size_data_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushSizeRelationCheck, this);
	m_brush_size_screen_rb->Bind(wxEVT_RADIOBUTTON, &BrushToolDlg::OnBrushSizeRelationCheck, this);
	sizer2_5->Add(5, 5);
	sizer2_5->Add(st, 0, wxALIGN_CENTER);
	sizer2_5->Add(m_brush_size_data_rb, 0, wxALIGN_CENTER);
	sizer2_5->Add(15, 15);
	sizer2_5->Add(m_brush_size_screen_rb, 0, wxALIGN_CENTER);
	//sizer2
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_4, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_5, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_2, 0, wxEXPAND);
	sizer2->Add(sizer2_3, 0, wxEXPAND);
	sizer2->Add(5, 5);
	sizer2->Add(sizer2_1, 0, wxEXPAND);
	sizer2->Add(5, 5);

	//alignment
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Align Render View to Selection"), wxVERTICAL);
	wxBoxSizer* sizer31 = new wxBoxSizer(wxHORIZONTAL);
	m_align_center_chk = new wxCheckBox(this, wxID_ANY,
		"Move to Center", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_align_center_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnAlignCenterCheck, this);
	sizer31->Add(5, 5);
	sizer31->Add(m_align_center_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer32 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Tri Axes:",
		wxDefaultPosition, wxDefaultSize);
	m_align_xyz = new wxButton(this, ID_AlignXYZ, "XYZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yxz = new wxButton(this, ID_AlignYXZ, "YXZ",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zxy = new wxButton(this, ID_AlignZXY, "ZXY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xzy = new wxButton(this, ID_AlignXZY, "XZY",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_yzx = new wxButton(this, ID_AlignYZX, "YZX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_zyx = new wxButton(this, ID_AlignZYX, "ZYX",
		wxDefaultPosition, FromDIP(wxSize(65, 22)));
	m_align_xyz->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_yxz->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_zxy->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_xzy->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_yzx->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	m_align_zyx->Bind(wxEVT_BUTTON, &BrushToolDlg::OnAlignPca, this);
	sizer32->Add(5, 5);
	sizer32->Add(st, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	sizer32->Add(m_align_xyz, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	sizer32->Add(m_align_yxz, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	sizer32->Add(m_align_zxy, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	sizer32->Add(m_align_xzy, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	sizer32->Add(m_align_yzx, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	sizer32->Add(m_align_zyx, 0, wxALIGN_CENTER);
	sizer32->Add(5, 5);
	//
	sizer3->Add(10, 10);
	sizer3->Add(sizer31, 0, wxEXPAND);
	sizer3->Add(10, 10);
	sizer3->Add(sizer32, 0, wxEXPAND);
	sizer3->Add(10, 10);

	//output
	wxBoxSizer *sizer4 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	m_update_btn = new wxButton(this, wxID_ANY, "Paint Size",
		wxDefaultPosition, wxDefaultSize);
	m_update_btn->Bind(wxEVT_BUTTON, &BrushToolDlg::OnUpdateBtn, this);
	m_history_chk = new wxCheckBox(this, wxID_ANY,
		"Hold History", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_history_chk->Bind(wxEVT_CHECKBOX, &BrushToolDlg::OnHistoryChk, this);
	m_clear_hist_btn = new wxButton(this, wxID_ANY,
		"Clear History", wxDefaultPosition, wxDefaultSize);
	m_clear_hist_btn->Bind(wxEVT_BUTTON, &BrushToolDlg::OnClearHistBtn, this);
	sizer4_1->Add(m_update_btn, 0, wxALIGN_CENTER);
	sizer4_1->AddStretchSpacer(1);
	sizer4_1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer4_1->Add(5, 5);
	sizer4_1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(this, wxID_ANY);
	m_output_grid->CreateGrid(0, 5);
	m_output_grid->SetColLabelValue(0, "Voxel Count");
	m_output_grid->SetColLabelValue(1, "Voxel Count\n(Int. Weighted)");
	m_output_grid->SetColLabelValue(2, "Average\nIntensity");
	m_output_grid->SetColLabelValue(3, "Physical Size");
	m_output_grid->SetColLabelValue(4, "Physical Size\n(Int. Weighted)");
	//m_output_grid->Fit();
	m_output_grid->Bind(wxEVT_GRID_SELECT_CELL, &BrushToolDlg::OnSelectCell, this);
	sizer4->Add(5, 5);
	sizer4->Add(sizer4_1, 0, wxEXPAND);
	sizer4->Add(5, 5);
	sizer4->Add(m_output_grid, 1, wxEXPAND);
	sizer4->Add(5, 5);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer3, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer4, 1, wxEXPAND);
	sizer_v->Add(10, 10);

	Bind(wxEVT_KEY_DOWN, &BrushToolDlg::OnKeyDown, this);
	Bind(wxEVT_SIZE, &BrushToolDlg::OnSize, this);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

BrushToolDlg::~BrushToolDlg()
{
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
		auto view = glbin_current.render_view.lock();
		int mode = view ? view->GetIntMode() : 0;
		bval = mode == 2 || mode == 10;
		ival = glbin_vol_selector.GetMode();
		m_toolbar->ToggleTool(ID_BrushGrow, bval && ival == 9);
		m_toolbar->ToggleTool(ID_BrushAppend, bval && ival == 2);
		m_toolbar->ToggleTool(ID_BrushComp, bval && ival == 10);
		m_toolbar->ToggleTool(ID_BrushSingle, bval && ival == 1);
		m_toolbar->ToggleTool(ID_BrushDiffuse, bval && ival == 4);
		m_toolbar->ToggleTool(ID_BrushSolid, bval && ival == 8);
		m_toolbar->ToggleTool(ID_BrushUnsel, bval && ival == 3);
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
		int i1 = glbin_vol_selector.GetIterWeak();
		int i2 = glbin_vol_selector.GetIterNormal();
		m_brush_iterw_rb->SetValue(ival <= i1);
		m_brush_iters_rb->SetValue(ival > i1 && ival <= i2);
		m_brush_iterss_rb->SetValue(ival > i2);
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
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 10 || mode == 12;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 9)
		mode = 0;
	else
		mode = 9;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	FluoRefresh(0, { gstFreehandToolState }, {-1});
}

void BrushToolDlg::BrushAppend()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 2;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 2)
		mode = 0;
	else
		mode = 2;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2 }, {-1});
}

void BrushToolDlg::BrushComp()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 2;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 10)
		mode = 0;
	else
		mode = 10;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2 }, {-1});
}

void BrushToolDlg::BrushSingle()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 2;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 1)
		mode = 0;
	else
		mode = 1;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	glbin_vol_selector.SetEstimateThreshold(mode == 1);
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2 }, {-1});
}

void BrushToolDlg::BrushDiffuse()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 2;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 4)
		mode = 0;
	else
		mode = 4;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2 }, {-1});
}

void BrushToolDlg::BrushSolid()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 2;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 8)
		mode = 0;
	else
		mode = 8;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2 }, {-1});
}

void BrushToolDlg::BrushUnsel()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	int mode = view->GetIntMode();
	bool bval = mode == 2;
	int ival = glbin_vol_selector.GetMode();
	if (bval && ival == 3)
		mode = 0;
	else
		mode = 3;
	glbin_vol_selector.SetMode(mode);
	glbin_states.m_brush_mode_toolbar = mode;
	glbin_states.m_brush_mode_shortcut = 0;
	FluoRefresh(0, { gstFreehandToolState, gstBrushSize1, gstBrushSize2 }, {-1});
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
void BrushToolDlg::OnBrushIterCheck(wxCommandEvent& event)
{
	if (m_brush_iterw_rb->GetValue())
	{
		glbin_vol_selector.SetBrushIteration(glbin_vol_selector.GetIterWeak());
	}
	else if (m_brush_iters_rb->GetValue())
	{
		glbin_vol_selector.SetBrushIteration(glbin_vol_selector.GetIterNormal());
	}
	else if (m_brush_iterss_rb->GetValue())
	{
		glbin_vol_selector.SetBrushIteration(glbin_vol_selector.GetIterStrong());
	}
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
