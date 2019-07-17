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
#include "BrushToolDlg.h"
#include "VRenderFrame.h"
#include <Calculate/Count.h>
#include <wx/valnum.h>
#include <wx/stdpaths.h>
#include "Formats/png_resource.h"

//resources
#include "img/icons.h"

#define GM_2_ESTR(x) (1.0 - sqrt(1.0 - (x - 1.0) * (x - 1.0)))

BEGIN_EVENT_TABLE(BrushToolDlg, wxPanel)
	//paint tools
	//brush commands
	EVT_TOOL(ID_BrushUndo, BrushToolDlg::OnBrushUndo)
	EVT_TOOL(ID_BrushRedo, BrushToolDlg::OnBrushRedo)
	EVT_TOOL(ID_BrushAppend, BrushToolDlg::OnBrushAppend)
	EVT_TOOL(ID_BrushDesel, BrushToolDlg::OnBrushDesel)
	EVT_TOOL(ID_BrushDiffuse, BrushToolDlg::OnBrushDiffuse)
	EVT_TOOL(ID_BrushClear, BrushToolDlg::OnBrushClear)
	EVT_TOOL(ID_BrushErase, BrushToolDlg::OnBrushErase)
	EVT_TOOL(ID_BrushCreate, BrushToolDlg::OnBrushCreate)
	EVT_TOOL(ID_BrushSolid, BrushToolDlg::OnBrushSolid)
	//selection adjustment
	//translate
	EVT_COMMAND_SCROLL(ID_BrushSclTranslateSldr, BrushToolDlg::OnBrushSclTranslateChange)
	EVT_TEXT(ID_BrushSclTranslateText, BrushToolDlg::OnBrushSclTranslateText)
	//gm falloff
	EVT_COMMAND_SCROLL(ID_BrushGmFalloffSldr, BrushToolDlg::OnBrushGmFalloffChange)
	EVT_TEXT(ID_BrushGmFalloffText, BrushToolDlg::OnBrushGmFalloffText)
	//2d influence
	EVT_COMMAND_SCROLL(ID_Brush2dinflSldr, BrushToolDlg::OnBrush2dinflChange)
	EVT_TEXT(ID_Brush2dinflText, BrushToolDlg::OnBrush2dinflText)
	//edge detect
	EVT_CHECKBOX(ID_BrushEdgeDetectChk, BrushToolDlg::OnBrushEdgeDetectChk)
	//hidden removal
	EVT_CHECKBOX(ID_BrushHiddenRemovalChk, BrushToolDlg::OnBrushHiddenRemovalChk)
	//select group
	EVT_CHECKBOX(ID_BrushSelectGroupChk, BrushToolDlg::OnBrushSelectGroupChk)
	//estimate thresh
	EVT_CHECKBOX(ID_EstimateThreshChk, BrushToolDlg::OnEstimateThreshChk)
	//brush properties
	//brush size 1
	EVT_COMMAND_SCROLL(ID_BrushSize1Sldr, BrushToolDlg::OnBrushSize1Change)
	EVT_TEXT(ID_BrushSize1Text, BrushToolDlg::OnBrushSize1Text)
	//brush size 2
	EVT_CHECKBOX(ID_BrushSize2Chk, BrushToolDlg::OnBrushSize2Chk)
	EVT_COMMAND_SCROLL(ID_BrushSize2Sldr, BrushToolDlg::OnBrushSize2Change)
	EVT_TEXT(ID_BrushSize2Text, BrushToolDlg::OnBrushSize2Text)
	//brush iterrations
	EVT_RADIOBUTTON(ID_BrushIterWRd, BrushToolDlg::OnBrushIterCheck)
	EVT_RADIOBUTTON(ID_BrushIterSRd, BrushToolDlg::OnBrushIterCheck)
	EVT_RADIOBUTTON(ID_BrushIterSSRd, BrushToolDlg::OnBrushIterCheck)
	//brush size relation
	EVT_RADIOBUTTON(ID_BrushSizeDataRd, BrushToolDlg::OnBrushSizeRelationCheck)
	EVT_RADIOBUTTON(ID_BrushSizeScreenRd, BrushToolDlg::OnBrushSizeRelationCheck)
	//output
	EVT_BUTTON(ID_UpdateBtn, BrushToolDlg::OnUpdateBtn)
	EVT_TOGGLEBUTTON(ID_AutoUpdateBtn, BrushToolDlg::OnAutoUpdateBtn)
	EVT_CHECKBOX(ID_HistoryChk, BrushToolDlg::OnHistoryChk)
	EVT_BUTTON(ID_ClearHistBtn, BrushToolDlg::OnClearHistBtn)
	EVT_KEY_DOWN(BrushToolDlg::OnKeyDown)
	EVT_GRID_SELECT_CELL(BrushToolDlg::OnSelectCell)
END_EVENT_TABLE()

BrushToolDlg::BrushToolDlg(wxWindow *frame, wxWindow *parent)
	: wxPanel(parent, wxID_ANY,
	wxDefaultPosition,
	wxSize(500, 500),
	0, "BrushToolDlg"),
	m_frame(parent),
	m_cur_view(0),
	m_max_value(255.0),
	m_dft_gm_falloff(0.0),
	m_dft_scl_translate(0.0),
	m_hold_history(false)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

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
	wxBitmap bitmap;
	bitmap = wxGetBitmapFromMemory(undo);
#ifdef _DARWIN
	m_toolbar->SetToolBitmapSize(bitmap.GetSize());
#endif
	m_toolbar->AddTool(
		ID_BrushUndo, "Undo", bitmap,
		"Rollback previous brush operation");
	bitmap = wxGetBitmapFromMemory(redo);
	m_toolbar->AddTool(
		ID_BrushRedo, "Redo", bitmap,
		"Redo the rollback brush operation");
	m_toolbar->AddSeparator();
	bitmap = wxGetBitmapFromMemory(brush_append);
	m_toolbar->AddCheckTool(ID_BrushAppend, "Select",
		bitmap, wxNullBitmap,
		"Highlight structures by painting on the render view (hold Shift)");
	bitmap = wxGetBitmapFromMemory(brush_diffuse);
	m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
		bitmap, wxNullBitmap,
		"Diffuse highlighted structures by painting (hold Z)");
	bitmap = wxGetBitmapFromMemory(brush_solid);
	m_toolbar->AddCheckTool(ID_BrushSolid, "Solid",
		bitmap, wxNullBitmap,
		"Highlight structures with solid mask");
	bitmap = wxGetBitmapFromMemory(brush_desel);
	m_toolbar->AddCheckTool(ID_BrushDesel, "Unselect",
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
	m_toolbar->AddTool(ID_BrushCreate, "Extract",
		bitmap, "Extract highlighted structures out and create a new volume");
	m_toolbar->Realize();

	//Selection adjustment
	wxBoxSizer *sizer1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Selection Settings"),
		wxVERTICAL);
	//stop at boundary
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_estimate_thresh_chk = new wxCheckBox(this, ID_EstimateThreshChk, "Auto Clear:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_edge_detect_chk = new wxCheckBox(this, ID_BrushEdgeDetectChk, "Edge Detect:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_hidden_removal_chk = new wxCheckBox(this, ID_BrushHiddenRemovalChk, "Visible Only:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_select_group_chk = new wxCheckBox(this, ID_BrushSelectGroupChk, "Apply to Group:",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer1_1->Add(m_estimate_thresh_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_edge_detect_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_hidden_removal_chk, 0, wxALIGN_CENTER);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_select_group_chk, 0, wxALIGN_CENTER);
	//threshold4
	wxBoxSizer *sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, wxSize(70, 20));
	m_brush_scl_translate_sldr = new wxSlider(this, ID_BrushSclTranslateSldr, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_scl_translate_text = new wxTextCtrl(this, ID_BrushSclTranslateText, "0.0",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp1);
	sizer1_2->Add(5, 5);
	sizer1_2->Add(st, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_brush_scl_translate_sldr, 1, wxEXPAND);
	sizer1_2->Add(m_brush_scl_translate_text, 0, wxALIGN_CENTER);
	sizer1_2->Add(15, 15);
	//gm falloff
	wxBoxSizer *sizer1_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Edge STR:",
		wxDefaultPosition, wxSize(70, 20));
	m_brush_gm_falloff_sldr = new wxSlider(this, ID_BrushGmFalloffSldr, 0, 0, 1000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_gm_falloff_text = new wxTextCtrl(this, ID_BrushGmFalloffText, "0.000",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp3);
	sizer1_3->Add(5, 5);
	sizer1_3->Add(st, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_brush_gm_falloff_sldr, 1, wxEXPAND);
	sizer1_3->Add(m_brush_gm_falloff_text, 0, wxALIGN_CENTER);
	sizer1_3->Add(15, 15);
	//2d
	wxBoxSizer *sizer1_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "2D Adj. Infl.:",
		wxDefaultPosition, wxSize(70, 20));
	m_brush_2dinfl_sldr = new wxSlider(this, ID_Brush2dinflSldr, 100, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_INVERSE);
	m_brush_2dinfl_text = new wxTextCtrl(this, ID_Brush2dinflText, "1.00",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
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
		wxDefaultPosition, wxSize(80, 20), wxALIGN_RIGHT);
	m_brush_size1_sldr = new wxSlider(this, ID_BrushSize1Sldr, 10, 1, 300,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_size1_text = new wxTextCtrl(this, ID_BrushSize1Text, "10",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	sizer2_2->Add(5, 5);
	sizer2_2->Add(st, 0, wxALIGN_CENTER);
	sizer2_2->Add(m_brush_size1_sldr, 1, wxEXPAND);
	sizer2_2->Add(m_brush_size1_text, 0, wxALIGN_CENTER);
	sizer2_2->Add(15, 15);
	//brush size 2
	wxBoxSizer *sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	m_brush_size2_chk = new wxCheckBox(this, ID_BrushSize2Chk, "GrowSize",
		wxDefaultPosition, wxSize(80, 20), wxALIGN_RIGHT);
	st = new wxStaticText(this, 0, ":",
		wxDefaultPosition, wxSize(5, 20), wxALIGN_RIGHT);
	m_brush_size2_sldr = new wxSlider(this, ID_BrushSize2Sldr, 30, 1, 300,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_brush_size2_text = new wxTextCtrl(this, ID_BrushSize2Text, "30",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	m_brush_size2_chk->SetValue(true);
	m_brush_size2_sldr->Enable();
	m_brush_size2_text->Enable();
	sizer2_3->Add(m_brush_size2_chk, 0, wxALIGN_CENTER);
	sizer2_3->Add(st, 0, wxALIGN_CENTER);
	sizer2_3->Add(m_brush_size2_sldr, 1, wxEXPAND);
	sizer2_3->Add(m_brush_size2_text, 0, wxALIGN_CENTER);
	sizer2_3->Add(15, 15);
	//iterations
	wxBoxSizer *sizer2_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Growth:",
		wxDefaultPosition, wxSize(70, 20));
	m_brush_iterw_rb = new wxRadioButton(this, ID_BrushIterWRd, "Weak",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_brush_iters_rb = new wxRadioButton(this, ID_BrushIterSRd, "Normal",
		wxDefaultPosition, wxDefaultSize);
	m_brush_iterss_rb = new wxRadioButton(this, ID_BrushIterSSRd, "Strong",
		wxDefaultPosition, wxDefaultSize);
	m_brush_iterw_rb->SetValue(false);
	m_brush_iters_rb->SetValue(true);
	m_brush_iterss_rb->SetValue(false);
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
		wxDefaultPosition, wxSize(70, 20));
	m_brush_size_data_rb = new wxRadioButton(this, ID_BrushSizeDataRd, "Data",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_brush_size_screen_rb = new wxRadioButton(this, ID_BrushSizeScreenRd, "Screen",
		wxDefaultPosition, wxDefaultSize);
	m_brush_size_data_rb->SetValue(true);
	m_brush_size_screen_rb->SetValue(false);
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

	//output
	wxBoxSizer *sizer3 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Output"),
		wxVERTICAL);
	wxBoxSizer *sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	m_update_btn = new wxButton(this, ID_UpdateBtn, "Get Sel. Size",
		wxDefaultPosition, wxSize(75, -1));
	m_auto_update_btn = new wxToggleButton(this, ID_AutoUpdateBtn,
		"Auto Update", wxDefaultPosition, wxSize(75, -1));
	m_history_chk = new wxCheckBox(this, ID_HistoryChk,
		"Hold History", wxDefaultPosition, wxSize(85, 20), wxALIGN_LEFT);
	m_clear_hist_btn = new wxButton(this, ID_ClearHistBtn,
		"Clear History", wxDefaultPosition, wxSize(75, -1));
	sizer3_1->Add(m_update_btn, 0, wxALIGN_CENTER);
	sizer3_1->Add(m_auto_update_btn, 0, wxALIGN_CENTER);
	sizer3_1->AddStretchSpacer(1);
	sizer3_1->Add(m_history_chk, 0, wxALIGN_CENTER);
	sizer3_1->Add(5, 5);
	sizer3_1->Add(m_clear_hist_btn, 0, wxALIGN_CENTER);
	//grid
	m_output_grid = new wxGrid(this, ID_OutputGrid);
	m_output_grid->CreateGrid(0, 4);
	m_output_grid->SetColLabelValue(0, "Voxel Count");
	m_output_grid->SetColLabelValue(1, "Voxel Count\n(Int. Weighted)");
	m_output_grid->SetColLabelValue(2, "Physical Size");
	m_output_grid->SetColLabelValue(3, "Physical Size\n(Int. Weighted)");
	m_output_grid->Fit();
	sizer3->Add(5, 5);
	sizer3->Add(sizer3_1, 0, wxEXPAND);
	sizer3->Add(5, 5);
	sizer3->Add(m_output_grid, 1, wxEXPAND);
	sizer3->Add(5, 5);

	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(10, 10);
	sizer_v->Add(m_toolbar, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer1, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer2, 0, wxEXPAND);
	sizer_v->Add(10, 10);
	sizer_v->Add(sizer3, 1, wxEXPAND);
	sizer_v->Add(10, 10);

	SetSizer(sizer_v);
	Layout();
}

BrushToolDlg::~BrushToolDlg()
{
}

void BrushToolDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv)
		return;

	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		sel_vol = vr_frame->GetCurSelVol();
		vr_frame->GetNoiseCancellingDlg()->GetSettings(vrv);
		vr_frame->GetCountingDlg()->GetSettings(vrv);
		//vr_frame->GetColocalizationDlg()->GetSettings(vrv);
	}

	m_cur_view = vrv;
	double dval = 0.0;
	int ival = 0;
	bool bval = false;

	//threshold range
	if (sel_vol)
	{
		m_max_value = sel_vol->GetMaxValue();
		//falloff
		m_brush_scl_translate_sldr->SetRange(0, int(m_max_value*10.0+0.5));
		//m_brush_scl_translate_text->SetValue(wxString::Format("%.1f", m_dft_scl_translate*m_max_value));
	}
	//selection strength
	dval = vrv->GetBrushSclTranslate();
	m_dft_scl_translate = dval;
	m_brush_scl_translate_sldr->SetValue(int(dval*m_max_value*10.0+0.5));
	m_brush_scl_translate_text->ChangeValue(wxString::Format("%.1f", m_dft_scl_translate*m_max_value));
	//gm falloff
	dval = vrv->GetBrushGmFalloff();
	m_dft_gm_falloff = dval;
	m_brush_gm_falloff_sldr->SetValue(int(GM_2_ESTR(dval)*1000.0 + 0.5));
	m_brush_gm_falloff_text->ChangeValue(wxString::Format("%.3f", GM_2_ESTR(dval)));
	//2d influence
	dval = vrv->GetW2d();
	m_brush_2dinfl_sldr->SetValue(int(dval*100.0+0.5));
	m_brush_2dinfl_text->ChangeValue(wxString::Format("%.2f", dval));
	//edge detect
	bval = vrv->GetEdgeDetect();
	m_edge_detect_chk->SetValue(bval);
	if (bval)
	{
		m_brush_gm_falloff_sldr->Enable();
		m_brush_gm_falloff_text->Enable();
	}
	else
	{
		m_brush_gm_falloff_sldr->Disable();
		m_brush_gm_falloff_text->Disable();
	}
	//hidden removal
	bval = vrv->GetHiddenRemoval();
	m_hidden_removal_chk->SetValue(bval);
	//select group
	bval = vrv->GetSelectGroup();
	m_select_group_chk->SetValue(bval);
	//estimate threshold
	bval = vrv->GetEstimateThresh();
	m_estimate_thresh_chk->SetValue(bval);

	//size1
	dval = vrv->GetBrushSize1();
	m_brush_size1_sldr->SetValue(int(dval));
	m_brush_size1_text->ChangeValue(wxString::Format("%.0f", dval));
	//size2
	m_brush_size2_chk->SetValue(vrv->GetUseBrushSize2());
	if (vrv->GetUseBrushSize2())
	{
		m_brush_size2_sldr->Enable();
		m_brush_size2_text->Enable();
	}
	else
	{
		m_brush_size2_sldr->Disable();
		m_brush_size2_text->Disable();
	}
	dval = vrv->GetBrushSize2();
	m_brush_size2_sldr->SetValue(int(dval));
	m_brush_size2_text->ChangeValue(wxString::Format("%.0f", dval));

	//iteration number
	ival = vrv->GetBrushIteration();
	if (ival<=BRUSH_TOOL_ITER_WEAK)
	{
		m_brush_iterw_rb->SetValue(true);
		m_brush_iters_rb->SetValue(false);
		m_brush_iterss_rb->SetValue(false);
	}
	else if (ival<=BRUSH_TOOL_ITER_NORMAL)
	{
		m_brush_iterw_rb->SetValue(false);
		m_brush_iters_rb->SetValue(true);
		m_brush_iterss_rb->SetValue(false);
	}
	else
	{
		m_brush_iterw_rb->SetValue(false);
		m_brush_iters_rb->SetValue(false);
		m_brush_iterss_rb->SetValue(true);
	}

	//brush size relation
	bval = vrv->GetBrushSizeData();
	if (bval)
	{
		m_brush_size_data_rb->SetValue(true);
		m_brush_size_screen_rb->SetValue(false);
	}
	else
	{
		m_brush_size_data_rb->SetValue(false);
		m_brush_size_screen_rb->SetValue(true);
	}

	//output
	m_history_chk->SetValue(m_hold_history);

	UpdateUndoRedo();
}

void BrushToolDlg::SelectBrush(int id)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	m_toolbar->ToggleTool(ID_BrushSolid, false);

	switch (id)
	{
	case ID_BrushAppend:
		m_toolbar->ToggleTool(ID_BrushAppend, true);
		break;
	case ID_BrushDiffuse:
		m_toolbar->ToggleTool(ID_BrushDiffuse, true);
		break;
	case ID_BrushDesel:
		m_toolbar->ToggleTool(ID_BrushDesel, true);
		break;
	case ID_BrushSolid:
		m_toolbar->ToggleTool(ID_BrushSolid, true);
	}

	GetSettings(m_cur_view);
}

void BrushToolDlg::UpdateUndoRedo()
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VolumeData* vd = vr_frame->GetCurSelVol();
		if (vd && vd->GetTexture())
		{
			m_toolbar->EnableTool(ID_BrushUndo,
				vd->GetTexture()->get_undo());
			m_toolbar->EnableTool(ID_BrushRedo,
				vd->GetTexture()->get_redo());
		}
	}
}

//brush commands
void BrushToolDlg::OnBrushAppend(wxCommandEvent &event)
{
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	m_toolbar->ToggleTool(ID_BrushSolid, false);

	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
	{
		if (m_toolbar->GetToolState(ID_BrushAppend))
			frame->GetTree()->SelectBrush(TreePanel::ID_BrushAppend);
		else
			frame->GetTree()->SelectBrush(0);
		frame->GetTree()->BrushAppend();
	}
	GetSettings(m_cur_view);
}

void BrushToolDlg::OnBrushDiffuse(wxCommandEvent &event)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	m_toolbar->ToggleTool(ID_BrushSolid, false);

	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
	{
		if (m_toolbar->GetToolState(ID_BrushDiffuse))
			frame->GetTree()->SelectBrush(TreePanel::ID_BrushDiffuse);
		else
			frame->GetTree()->SelectBrush(0);
		frame->GetTree()->BrushDiffuse();
	}
	GetSettings(m_cur_view);
}

void BrushToolDlg::OnBrushSolid(wxCommandEvent &event)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);

	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushSolid(m_toolbar->GetToolState(ID_BrushSolid));
	GetSettings(m_cur_view);
}

void BrushToolDlg::OnBrushDesel(wxCommandEvent &event)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushSolid, false);

	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
	{
		if (m_toolbar->GetToolState(ID_BrushDesel))
			frame->GetTree()->SelectBrush(TreePanel::ID_BrushDesel);
		else
			frame->GetTree()->SelectBrush(0);
		frame->GetTree()->BrushDesel();
	}
	GetSettings(m_cur_view);
}

void BrushToolDlg::OnBrushClear(wxCommandEvent &event)
{
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushClear();
}

void BrushToolDlg::OnBrushErase(wxCommandEvent &event)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	m_toolbar->ToggleTool(ID_BrushSolid, false);

	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushErase();
}

void BrushToolDlg::OnBrushCreate(wxCommandEvent &event)
{
	m_toolbar->ToggleTool(ID_BrushAppend, false);
	m_toolbar->ToggleTool(ID_BrushDiffuse, false);
	m_toolbar->ToggleTool(ID_BrushDesel, false);
	m_toolbar->ToggleTool(ID_BrushSolid, false);

	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushCreate();
}

void BrushToolDlg::OnBrushUndo(wxCommandEvent &event)
{
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (sel_vol && sel_vol->GetTexture())
	{
		sel_vol->GetTexture()->mask_undos_backward();
		sel_vol->GetVR()->clear_tex_pool();
	}
	vr_frame->RefreshVRenderViews();
	UpdateUndoRedo();
}

void BrushToolDlg::OnBrushRedo(wxCommandEvent &event)
{
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (sel_vol && sel_vol->GetTexture())
	{
		sel_vol->GetTexture()->mask_undos_forward();
		sel_vol->GetVR()->clear_tex_pool();
	}
	vr_frame->RefreshVRenderViews();
	UpdateUndoRedo();
}

//selection adjustment
//scalar translate
void BrushToolDlg::OnBrushSclTranslateChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	double val = double(ival)/10.0;
	wxString str = wxString::Format("%.1f", val);
	m_brush_scl_translate_text->SetValue(str);
}

void BrushToolDlg::OnBrushSclTranslateText(wxCommandEvent &event)
{
	wxString str = m_brush_scl_translate_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_dft_scl_translate = val/m_max_value;
	m_brush_scl_translate_sldr->SetValue(int(val*10.0+0.5));

	//set translate
	if (m_cur_view)
		m_cur_view->SetBrushSclTranslate(m_dft_scl_translate);
}

//gm falloff
void BrushToolDlg::OnBrushGmFalloffChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	double val = double(ival) / 1000.0;
	wxString str = wxString::Format("%.3f", val);
	m_brush_gm_falloff_text->SetValue(str);
}

void BrushToolDlg::OnBrushGmFalloffText(wxCommandEvent &event)
{
	wxString str = m_brush_gm_falloff_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_dft_gm_falloff = GM_2_ESTR(val);
	m_brush_gm_falloff_sldr->SetValue(int(val*1000.0+0.5));

	//set gm falloff
	if (m_cur_view)
		m_cur_view->SetBrushGmFalloff(m_dft_gm_falloff);
}

//2d influence
void BrushToolDlg::OnBrush2dinflChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	double val = double(ival)/100.0;
	wxString str = wxString::Format("%.2f", val);
	m_brush_2dinfl_text->SetValue(str);
}

void BrushToolDlg::OnBrush2dinflText(wxCommandEvent &event)
{
	wxString str = m_brush_2dinfl_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_brush_2dinfl_sldr->SetValue(int(val*100.0));

	//set 2d weight
	if (m_cur_view)
		m_cur_view->SetW2d(val);
}

//edge detect
void BrushToolDlg::OnBrushEdgeDetectChk(wxCommandEvent &event)
{
	bool edge_detect = m_edge_detect_chk->GetValue();

	if (edge_detect)
	{
		m_brush_gm_falloff_sldr->Enable();
		m_brush_gm_falloff_text->Enable();
	}
	else
	{
		m_brush_gm_falloff_sldr->Disable();
		m_brush_gm_falloff_text->Disable();
	}

	//set edge detect
	if (m_cur_view)
		m_cur_view->SetEdgeDetect(edge_detect);
}

//hidden removal
void BrushToolDlg::OnBrushHiddenRemovalChk(wxCommandEvent &event)
{
	bool hidden_removal = m_hidden_removal_chk->GetValue();

	//set hidden removal
	if (m_cur_view)
		m_cur_view->SetHiddenRemoval(hidden_removal);
}

//select group
void BrushToolDlg::OnBrushSelectGroupChk(wxCommandEvent &event)
{
	bool select_group = m_select_group_chk->GetValue();

	//set select group
	if (m_cur_view)
		m_cur_view->SetSelectGroup(select_group);
}

//estimate threshold
void BrushToolDlg::OnEstimateThreshChk(wxCommandEvent &event)
{
	bool value = m_estimate_thresh_chk->GetValue();

	//
	if (m_cur_view)
		m_cur_view->SetEstimateThresh(value);
}

//brush size 1
void BrushToolDlg::OnBrushSize1Change(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_brush_size1_text->SetValue(str);
}

void BrushToolDlg::OnBrushSize1Text(wxCommandEvent &event)
{
	wxString str = m_brush_size1_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_brush_size1_sldr->SetValue(int(val));

	//set size1
	if (m_cur_view)
	{
		m_cur_view->SetBrushSize(val, -1.0);
		if (m_cur_view->GetIntMode()==2)
			m_cur_view->RefreshGL();
	}
}

//brush size 2
void BrushToolDlg::OnBrushSize2Chk(wxCommandEvent &event)
{
	wxString str = m_brush_size1_text->GetValue();
	double val1;
	str.ToDouble(&val1);
	str = m_brush_size2_text->GetValue();
	double val2;
	str.ToDouble(&val2);

	if (m_brush_size2_chk->GetValue())
	{
		m_brush_size2_sldr->Enable();
		m_brush_size2_text->Enable();
		if (m_cur_view)
		{
			m_cur_view->SetUseBrushSize2(true);
			m_cur_view->SetBrushSize(val1, val2);
			m_cur_view->RefreshGL();
		}
	}
	else
	{
		m_brush_size2_sldr->Disable();
		m_brush_size2_text->Disable();
		if (m_cur_view)
		{
			m_cur_view->SetUseBrushSize2(false);
			m_cur_view->SetBrushSize(val1, val2);
			m_cur_view->RefreshGL();
		}
	}
}

void BrushToolDlg::OnBrushSize2Change(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_brush_size2_text->SetValue(str);
}

void BrushToolDlg::OnBrushSize2Text(wxCommandEvent &event)
{
	wxString str = m_brush_size2_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_brush_size2_sldr->SetValue(int(val));

	//set size2
	if (m_cur_view)
	{
		m_cur_view->SetBrushSize(-1.0, val);
		if (m_cur_view->GetIntMode()==2)
			m_cur_view->RefreshGL();
	}
}

//brush iterations
void BrushToolDlg::OnBrushIterCheck(wxCommandEvent& event)
{
	if (m_brush_iterw_rb->GetValue())
	{
		if (m_cur_view)
			m_cur_view->SetBrushIteration(BRUSH_TOOL_ITER_WEAK);
	}
	else if (m_brush_iters_rb->GetValue())
	{
		if (m_cur_view)
			m_cur_view->SetBrushIteration(BRUSH_TOOL_ITER_NORMAL);
	}
	else if (m_brush_iterss_rb->GetValue())
	{
		if (m_cur_view)
			m_cur_view->SetBrushIteration(BRUSH_TOOL_ITER_STRONG);
	}
}

//brush size relation
void BrushToolDlg::OnBrushSizeRelationCheck(wxCommandEvent& event)
{
	if (m_brush_size_data_rb->GetValue())
	{
		if (m_cur_view)
			m_cur_view->SetBrushSizeData(true);
	}
	else if (m_brush_size_screen_rb->GetValue())
	{
		if (m_cur_view)
			m_cur_view->SetBrushSizeData(false);
	}
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
		wxString::Format("%f", data.size) + unit);
	m_output_grid->SetCellValue(0, 3,
		wxString::Format("%f", data.wsize) + unit);
	//m_output_grid->Fit();
}

void BrushToolDlg::OnUpdateBtn(wxCommandEvent& event)
{
	GridData data;
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame)
		return;
	sel_vol = vr_frame->GetCurSelVol();
	if (!sel_vol)
		return;

	FL::CountVoxels counter(sel_vol);
	counter.SetUseMask(true);
	counter.Count();
	data.voxel_sum = counter.GetSum();
	double scale = sel_vol->GetScalarScale();
	data.voxel_wsum = counter.GetWeightedSum() * scale;
	double spcx, spcy, spcz;
	sel_vol->GetSpacings(spcx, spcy, spcz);
	double vvol = spcx * spcy * spcz;
	vvol = vvol == 0.0 ? 1.0 : vvol;
	data.size = data.voxel_sum * vvol;
	data.wsize = data.voxel_wsum * vvol;
	wxString unit;
	if (m_cur_view)
	{
		switch (m_cur_view->m_glview->m_sb_unit)
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

void BrushToolDlg::OnAutoUpdateBtn(wxCommandEvent& event)
{
	if (m_cur_view)
		m_cur_view->m_glview->m_count = m_auto_update_btn->GetValue();
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
	event.Skip();
}

void BrushToolDlg::OnSelectCell(wxGridEvent& event)
{
	int r = event.GetRow();
	int c = event.GetCol();
	m_output_grid->SelectBlock(r, c, r, c);
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
