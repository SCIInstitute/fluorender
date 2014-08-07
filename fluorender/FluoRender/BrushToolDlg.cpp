#include "BrushToolDlg.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>
#include "Formats/png_resource.h"

//resources
#include "img/listicon_brushappend.h"
#include "img/listicon_brushclear.h"
#include "img/listicon_brushcreate.h"
#include "img/listicon_brushdesel.h"
#include "img/listicon_brushdiffuse.h"
#include "img/listicon_brusherase.h"
#include "img/listicon_qmark.h"

BEGIN_EVENT_TABLE(BrushToolDlg, wxPanel)
//paint tools
//brush commands
EVT_TOOL(ID_BrushAppend, BrushToolDlg::OnBrushAppend)
EVT_TOOL(ID_BrushDesel, BrushToolDlg::OnBrushDesel)
EVT_TOOL(ID_BrushDiffuse, BrushToolDlg::OnBrushDiffuse)
EVT_TOOL(ID_BrushClear, BrushToolDlg::OnBrushClear)
EVT_TOOL(ID_BrushErase, BrushToolDlg::OnBrushErase)
EVT_TOOL(ID_BrushCreate, BrushToolDlg::OnBrushCreate)
//help
EVT_TOOL(ID_HelpBtn, BrushToolDlg::OnHelpBtn)
//selection adjustment
//translate
EVT_COMMAND_SCROLL(ID_BrushSclTranslateSldr, BrushToolDlg::OnBrushSclTranslateChange)
EVT_TEXT(ID_BrushSclTranslateText, BrushToolDlg::OnBrushSclTranslateText)
//2d influence
EVT_COMMAND_SCROLL(ID_Brush2dinflSldr, BrushToolDlg::OnBrush2dinflChange)
EVT_TEXT(ID_Brush2dinflText, BrushToolDlg::OnBrush2dinflText)
//edge detect
EVT_CHECKBOX(ID_BrushEdgeDetectChk, BrushToolDlg::OnBrushEdgeDetectChk)
//hidden removal
EVT_CHECKBOX(ID_BrushHiddenRemovalChk, BrushToolDlg::OnBrushHiddenRemovalChk)
//select group
EVT_CHECKBOX(ID_BrushSelectGroupChk, BrushToolDlg::OnBrushSelectGroupChk)
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
//component analyzer
EVT_COMMAND_SCROLL(ID_CAThreshSldr, BrushToolDlg::OnCAThreshChange)
EVT_TEXT(ID_CAThreshText, BrushToolDlg::OnCAThreshText)
EVT_CHECKBOX(ID_CAIgnoreMaxChk, BrushToolDlg::OnCAIgnoreMaxChk)
EVT_BUTTON(ID_CAAnalyzeBtn, BrushToolDlg::OnCAAnalyzeBtn)
EVT_BUTTON(ID_CAMultiChannBtn, BrushToolDlg::OnCAMultiChannBtn)
EVT_BUTTON(ID_CARandomColorBtn, BrushToolDlg::OnCARandomColorBtn)
EVT_BUTTON(ID_CAAnnotationsBtn, BrushToolDlg::OnCAAnnotationsBtn)
//noise removal
EVT_COMMAND_SCROLL(ID_NRSizeSldr, BrushToolDlg::OnNRSizeChange)
EVT_TEXT(ID_NRSizeText, BrushToolDlg::OnNRSizeText)
EVT_BUTTON(ID_NRAnalyzeBtn, BrushToolDlg::OnNRAnalyzeBtn)
EVT_BUTTON(ID_NRRemoveBtn, BrushToolDlg::OnNRRemoveBtn)

//calculations
//operands
EVT_BUTTON(ID_CalcLoadABtn, BrushToolDlg::OnLoadA)
EVT_BUTTON(ID_CalcLoadBBtn, BrushToolDlg::OnLoadB)
//operators
EVT_BUTTON(ID_CalcSubBtn, BrushToolDlg::OnCalcSub)
EVT_BUTTON(ID_CalcAddBtn, BrushToolDlg::OnCalcAdd)
EVT_BUTTON(ID_CalcDivBtn, BrushToolDlg::OnCalcDiv)
EVT_BUTTON(ID_CalcIscBtn, BrushToolDlg::OnCalcIsc)
//one-operators
EVT_BUTTON(ID_CalcFillBtn, BrushToolDlg::OnCalcFill)
END_EVENT_TABLE()

BrushToolDlg::BrushToolDlg(wxWindow *frame, wxWindow *parent)
: wxPanel(parent, wxID_ANY,
      wxPoint(500, 150),
      wxSize(400, 550),
      0, "BrushToolDlg"),
   m_frame(parent),
   m_cur_view(0),
   m_vol1(0),
   m_vol2(0),
   m_max_value(255.0),
   m_dft_ini_thresh(0.0),
   m_dft_gm_falloff(0.0),
   m_dft_scl_falloff(0.0),
   m_dft_scl_translate(0.0),
   m_dft_ca_falloff(1.0),
   m_dft_ca_min(0.0),
   m_dft_ca_max(0.0),
   m_dft_ca_thresh(0.0),
   m_dft_nr_thresh(0.0),
   m_dft_nr_size(0.0)
{
   if (!((VRenderFrame*)m_frame)->GetFreeVersion())
      SetSize(400, 750);
   //validator: floating point 1
   wxFloatingPointValidator<double> vald_fp1(1);
   //validator: floating point 2
   wxFloatingPointValidator<double> vald_fp2(2);
   //validator: integer
   wxIntegerValidator<unsigned int> vald_int;

   wxStaticText *st = 0;

   //group1
   //paint tools
   wxBoxSizer *group1 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY, "Selection"),
         wxVERTICAL);

   //toolbar
   m_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
         wxTB_FLAT|wxTB_TOP|wxTB_NODIVIDER|wxTB_TEXT);
   m_toolbar->AddCheckTool(ID_BrushAppend, "Select",
         wxGetBitmapFromMemory(listicon_brushappend),
         wxNullBitmap,
         "Highlight structures by painting on the render view (hold Shift)");
   m_toolbar->AddCheckTool(ID_BrushDiffuse, "Diffuse",
         wxGetBitmapFromMemory(listicon_brushdiffuse),
         wxNullBitmap,
         "Diffuse highlighted structures by painting (hold Z)");
   m_toolbar->AddCheckTool(ID_BrushDesel, "Unselect",
         wxGetBitmapFromMemory(listicon_brushdesel),
         wxNullBitmap,
         "Reset highlighted structures by painting (hold X)");
   m_toolbar->AddTool(ID_BrushClear, "Reset All",
         wxGetBitmapFromMemory(listicon_brushclear),
         "Reset all highlighted structures");
   m_toolbar->AddSeparator();
   m_toolbar->AddTool(ID_BrushErase, "Erase",
         wxGetBitmapFromMemory(listicon_brusherase),
         "Erase highlighted structures");
   m_toolbar->AddTool(ID_BrushCreate, "Extract",
         wxGetBitmapFromMemory(listicon_brushcreate),
         "Extract highlighted structures out and create a new volume");
   m_toolbar->AddSeparator();
   m_toolbar->AddTool(ID_HelpBtn, "Help",
         wxGetBitmapFromMemory(listicon_qmark),
         "Help on the settings");
   m_toolbar->Realize();

   //Selection adjustment
   wxBoxSizer *sizer11 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY, "Selection Settings"),
         wxVERTICAL);
   //stop at boundary
   wxBoxSizer *sizer11_1 = new wxBoxSizer(wxHORIZONTAL);
   m_edge_detect_chk = new wxCheckBox(this, ID_BrushEdgeDetectChk, "Edge Detect:",
         wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
   m_hidden_removal_chk = new wxCheckBox(this, ID_BrushHiddenRemovalChk, "Visible Only:",
         wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
   m_select_group_chk = new wxCheckBox(this, ID_BrushSelectGroupChk, "Select Group:",
         wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
   sizer11_1->Add(m_edge_detect_chk, 0, wxALIGN_CENTER);
   sizer11_1->Add(5, 5);
   sizer11_1->Add(m_hidden_removal_chk, 0, wxALIGN_CENTER);
   sizer11_1->Add(5, 5);
   sizer11_1->Add(m_select_group_chk, 0, wxALIGN_CENTER);
   //threshold4
   wxBoxSizer *sizer11_2 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Threshold:",
         wxDefaultPosition, wxSize(70, 20));
   m_brush_scl_translate_sldr = new wxSlider(this, ID_BrushSclTranslateSldr, 0, 0, 2550,
         wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
   m_brush_scl_translate_text = new wxTextCtrl(this, ID_BrushSclTranslateText, "0.0",
         wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
   sizer11_2->Add(5, 5);
   sizer11_2->Add(st, 0, wxALIGN_CENTER);
   sizer11_2->Add(m_brush_scl_translate_sldr, 1, wxEXPAND|wxALIGN_CENTER);
   sizer11_2->Add(m_brush_scl_translate_text, 0, wxALIGN_CENTER);
   sizer11_2->Add(15, 15);
   //2d
   wxBoxSizer *sizer11_3 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "2D Adj. Infl.:",
         wxDefaultPosition, wxSize(70, 20));
   m_brush_2dinfl_sldr = new wxSlider(this, ID_Brush2dinflSldr, 100, 0, 200,
         wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE);
   m_brush_2dinfl_text = new wxTextCtrl(this, ID_Brush2dinflText, "1.00",
         wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
   sizer11_3->Add(5, 5);
   sizer11_3->Add(st, 0, wxALIGN_CENTER);
   sizer11_3->Add(m_brush_2dinfl_sldr, 1, wxEXPAND|wxALIGN_CENTER);
   sizer11_3->Add(m_brush_2dinfl_text, 0, wxALIGN_CENTER);
   sizer11_3->Add(15, 15);
   //sizer21
   sizer11->Add(sizer11_1, 0, wxEXPAND|wxALIGN_CENTER);
   sizer11->Add(5, 5);
   sizer11->Add(sizer11_2, 0, wxEXPAND|wxALIGN_CENTER);
   sizer11->Add(sizer11_3, 0, wxEXPAND|wxALIGN_CENTER);
   sizer11->Hide(sizer11_3, true);

   //Brush properties
   wxBoxSizer *sizer12 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY, "Brush Properties"),
         wxVERTICAL);
   wxBoxSizer *sizer12_1 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Brush sizes can be also set with mouse wheel in painting mode.");
   sizer12_1->Add(5, 5);
   sizer12_1->Add(st, 0, wxALIGN_CENTER);
   //brush size 1
   wxBoxSizer *sizer12_2 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Center Size:",
         wxDefaultPosition, wxSize(70, 20));
   m_brush_size1_sldr = new wxSlider(this, ID_BrushSize1Sldr, 10, 1, 300,
         wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
   m_brush_size1_text = new wxTextCtrl(this, ID_BrushSize1Text, "10",
         wxDefaultPosition, wxSize(40, 20), 0, vald_int);
   sizer12_2->Add(5, 5);
   sizer12_2->Add(st, 0, wxALIGN_CENTER);
   sizer12_2->Add(m_brush_size1_sldr, 1, wxEXPAND|wxALIGN_CENTER);
   sizer12_2->Add(m_brush_size1_text, 0, wxALIGN_CENTER);
   st = new wxStaticText(this, 0, "px",
         wxDefaultPosition, wxSize(15, 15));
   sizer12_2->Add(st, 0, wxALIGN_CENTER);
   //brush size 2
   wxBoxSizer *sizer12_3 = new wxBoxSizer(wxHORIZONTAL);
   m_brush_size2_chk = new wxCheckBox(this, ID_BrushSize2Chk, "GrowSize",
         wxDefaultPosition, wxSize(90, 20), wxALIGN_RIGHT);
   st = new wxStaticText(this, 0, ":",
         wxDefaultPosition, wxSize(5, 20), wxALIGN_RIGHT);
   m_brush_size2_sldr = new wxSlider(this, ID_BrushSize2Sldr, 30, 1, 300,
         wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
   m_brush_size2_text = new wxTextCtrl(this, ID_BrushSize2Text, "30",
         wxDefaultPosition, wxSize(40, 20), 0, vald_int);
   m_brush_size2_chk->SetValue(true);
   m_brush_size2_sldr->Enable();
   m_brush_size2_text->Enable();
   sizer12_3->Add(m_brush_size2_chk, 0, wxALIGN_CENTER);
   sizer12_3->Add(st, 0, wxALIGN_CENTER);
   sizer12_3->Add(m_brush_size2_sldr, 1, wxEXPAND|wxALIGN_CENTER);
   sizer12_3->Add(m_brush_size2_text, 0, wxALIGN_CENTER);
   st = new wxStaticText(this, 0, "px",
         wxDefaultPosition, wxSize(15, 15));
   sizer12_3->Add(st, 0, wxALIGN_CENTER);
   //iterations
   wxBoxSizer *sizer12_4 = new wxBoxSizer(wxHORIZONTAL);
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
   sizer12_4->Add(5, 5);
   sizer12_4->Add(st, 0, wxALIGN_CENTER);
   sizer12_4->Add(m_brush_iterw_rb, 0, wxALIGN_CENTER);
   sizer12_4->Add(15, 15);
   sizer12_4->Add(m_brush_iters_rb, 0, wxALIGN_CENTER);
   sizer12_4->Add(15, 15);
   sizer12_4->Add(m_brush_iterss_rb, 0, wxALIGN_CENTER);
   //sizer12
   sizer12->Add(sizer12_1, 0, wxEXPAND|wxALIGN_CENTER);
   sizer12->Add(5, 5);
   sizer12->Add(sizer12_2, 0, wxEXPAND|wxALIGN_CENTER);
   sizer12->Add(sizer12_3, 0, wxEXPAND|wxALIGN_CENTER);
   sizer12->Add(5, 5);
   sizer12->Add(sizer12_4, 0, wxEXPAND|wxALIGN_CENTER);

   //component analyzer
   wxBoxSizer *sizer13 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY, "Component Analyzer"),
         wxVERTICAL);
   //threshold of ccl
   wxBoxSizer *sizer13_1 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Threshold:",
         wxDefaultPosition, wxSize(75, 20));
   m_ca_thresh_sldr = new wxSlider(this, ID_CAThreshSldr, 0, 0, 2550,
         wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
   m_ca_thresh_text = new wxTextCtrl(this, ID_CAThreshText, "0.0",
         wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
   sizer13_1->Add(st, 0, wxALIGN_CENTER);
   sizer13_1->Add(m_ca_thresh_sldr, 1, wxALIGN_CENTER|wxEXPAND);
   sizer13_1->Add(m_ca_thresh_text, 0, wxALIGN_CENTER);
   m_ca_analyze_btn = new wxButton(this, ID_CAAnalyzeBtn, "Analyze",
         wxDefaultPosition, wxSize(-1, 23));
   sizer13_1->Add(m_ca_analyze_btn, 0, wxALIGN_CENTER);
   //size of ccl
   wxBoxSizer *sizer13_2 = new wxBoxSizer(wxHORIZONTAL);
   m_ca_select_only_chk = new wxCheckBox(this, ID_CASelectOnlyChk, "Selct. Only",
         wxDefaultPosition, wxSize(75, 20));
   sizer13_2->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);
   sizer13_2->AddStretchSpacer();
   st = new wxStaticText(this, 0, "Min:",
         wxDefaultPosition, wxSize(40, 15));
   sizer13_2->Add(st, 0, wxALIGN_CENTER);
   m_ca_min_text = new wxTextCtrl(this, ID_CAMinText, "0",
         wxDefaultPosition, wxSize(40, 20), 0, vald_int);
   sizer13_2->Add(m_ca_min_text, 0, wxALIGN_CENTER);
   st = new wxStaticText(this, 0, "vx",
         wxDefaultPosition, wxSize(15, 15));
   sizer13_2->Add(st, 0, wxALIGN_CENTER);
   sizer13_2->AddStretchSpacer();
   st = new wxStaticText(this, 0, "Max:",
         wxDefaultPosition, wxSize(40, 15));
   sizer13_2->Add(st, 0, wxALIGN_CENTER);
   m_ca_max_text = new wxTextCtrl(this, ID_CAMaxText, "1000",
         wxDefaultPosition, wxSize(40, 20), 0, vald_int);
   sizer13_2->Add(m_ca_max_text, 0, wxALIGN_CENTER);
   st = new wxStaticText(this, 0, "vx",
         wxDefaultPosition, wxSize(15, 15));
   sizer13_2->Add(st, 0, wxALIGN_CENTER);
   sizer13_2->AddStretchSpacer();
   m_ca_ignore_max_chk = new wxCheckBox(this, ID_CAIgnoreMaxChk, "Ignore Max");
   sizer13_2->Add(m_ca_ignore_max_chk, 0, wxALIGN_CENTER);
   //text result
   wxBoxSizer *sizer13_3 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Components:");
   sizer13_3->Add(st, 0, wxALIGN_CENTER);
   m_ca_comps_text = new wxTextCtrl(this, ID_CACompsText, "0",
         wxDefaultPosition, wxSize(70, 20), wxTE_READONLY);
   sizer13_3->Add(m_ca_comps_text, 0, wxALIGN_CENTER);
   st = new wxStaticText(this, 0, "Total Volume:");
   sizer13_3->Add(st, 0, wxALIGN_CENTER);
   m_ca_volume_text = new wxTextCtrl(this, ID_CAVolumeText, "0",
         wxDefaultPosition, wxSize(70, 20), wxTE_READONLY);
   sizer13_3->Add(m_ca_volume_text, 0, wxALIGN_CENTER);
	m_ca_size_map_chk = new wxCheckBox(this, ID_CASizeMapChk, "Size-Color",
		wxDefaultPosition, wxSize(75, 20));
	sizer13_3->Add(m_ca_size_map_chk, 0, wxALIGN_CENTER);   
    //export
   wxBoxSizer *sizer13_4 = new wxBoxSizer(wxHORIZONTAL);
   sizer13_4->AddStretchSpacer();
   st = new wxStaticText(this, 0, "Export:  ");
   sizer13_4->Add(st, 0, wxALIGN_CENTER);
   m_ca_multi_chann_btn = new wxButton(this, ID_CAMultiChannBtn, "Multi-Channels",
         wxDefaultPosition, wxSize(-1, 23));
   m_ca_random_color_btn = new wxButton(this, ID_CARandomColorBtn, "Random Colors",
         wxDefaultPosition, wxSize(-1, 23));
   m_ca_annotations_btn = new wxButton(this, ID_CAAnnotationsBtn, "Show Annotations",
         wxDefaultPosition, wxSize(-1, 23));
   sizer13_4->Add(m_ca_multi_chann_btn, 0, wxALIGN_CENTER);
   sizer13_4->Add(m_ca_random_color_btn, 0, wxALIGN_CENTER);
   sizer13_4->Add(m_ca_annotations_btn, 0, wxALIGN_CENTER);
   //sizer13
   sizer13->Add(sizer13_1, 0, wxEXPAND|wxALIGN_CENTER);
   sizer13->Add(sizer13_2, 0, wxEXPAND|wxALIGN_CENTER);
   sizer13->Add(sizer13_3, 0, wxEXPAND|wxALIGN_CENTER);
   sizer13->Add(sizer13_4, 0, wxEXPAND|wxALIGN_CENTER);
   //noise removal
   wxBoxSizer *sizer14 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY, "Noise Removal"),
         wxVERTICAL);
   //size threshold
   wxBoxSizer *sizer14_1 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Size Thresh.:",
         wxDefaultPosition, wxSize(75, -1));
   m_nr_size_sldr = new wxSlider(this, ID_NRSizeSldr, 10, 1, 100,
         wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
   m_nr_size_text = new wxTextCtrl(this, ID_NRSizeText, "10",
         wxDefaultPosition, wxSize(40, -1), 0, vald_int);
   sizer14_1->Add(st, 0, wxALIGN_CENTER);
   sizer14_1->Add(m_nr_size_sldr, 1, wxALIGN_CENTER|wxEXPAND);
   sizer14_1->Add(m_nr_size_text, 0, wxALIGN_CENTER);
   st = new wxStaticText(this, 0, "vx",
         wxDefaultPosition, wxSize(15, 15));
   sizer14_1->Add(st, 0, wxALIGN_CENTER);
   //buttons
   wxBoxSizer *sizer14_2 = new wxBoxSizer(wxHORIZONTAL);
   m_nr_analyze_btn = new wxButton(this, ID_NRAnalyzeBtn, "Analyze",
         wxDefaultPosition, wxSize(-1, 23));
   m_nr_remove_btn = new wxButton(this, ID_NRRemoveBtn, "Remove",
         wxDefaultPosition, wxSize(-1, 23));
   sizer14_2->AddStretchSpacer();
   sizer14_2->Add(m_nr_analyze_btn, 0, wxALIGN_CENTER);
   sizer14_2->Add(m_nr_remove_btn, 0, wxALIGN_CENTER);
   //sizer14
   sizer14->Add(sizer14_1, 0, wxEXPAND|wxALIGN_CENTER);
   sizer14->Add(sizer14_2, 0, wxEXPAND|wxALIGN_CENTER);

   //group1
   group1->Add(m_toolbar, 0, wxEXPAND);
   group1->Add(5, 5);
   group1->Add(sizer11, 0, wxEXPAND|wxALIGN_CENTER);
   group1->Add(sizer12, 0, wxEXPAND|wxALIGN_CENTER);
   group1->Add(sizer13, 0, wxEXPAND|wxALIGN_CENTER);
   group1->Add(sizer14, 0, wxEXPAND|wxALIGN_CENTER);
   if (((VRenderFrame*)m_frame)->GetFreeVersion())
   {
      group1->Hide(sizer13);
      group1->Hide(sizer14);
   }

   //group 2
   //calculations
   wxBoxSizer* group2 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY, "Calculations"),
         wxVERTICAL);
   //operand A
   wxBoxSizer *sizer21 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Volume A:",
         wxDefaultPosition, wxSize(75, 20));
   m_calc_load_a_btn = new wxButton(this, ID_CalcLoadABtn, "Load",
         wxDefaultPosition, wxSize(50, 20));
   m_calc_a_text = new wxTextCtrl(this, ID_CalcAText, "",
         wxDefaultPosition, wxSize(200, 20));
   sizer21->Add(st, 0, wxALIGN_CENTER);
   sizer21->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
   sizer21->Add(m_calc_a_text, 1, wxEXPAND|wxALIGN_CENTER);
   //operand B
   wxBoxSizer *sizer22 = new wxBoxSizer(wxHORIZONTAL);
   st = new wxStaticText(this, 0, "Volume B:",
         wxDefaultPosition, wxSize(75, 20));
   m_calc_load_b_btn = new wxButton(this, ID_CalcLoadBBtn, "Load",
         wxDefaultPosition, wxSize(50, 20));
   m_calc_b_text = new wxTextCtrl(this, ID_CalcBText, "",
         wxDefaultPosition, wxSize(200, 20));
   sizer22->Add(st, 0, wxALIGN_CENTER);
   sizer22->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
   sizer22->Add(m_calc_b_text, 1, wxEXPAND|wxALIGN_CENTER);
   //single operators
   wxBoxSizer *sizer23 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY,
            "Single-valued Operators (Require A)"), wxVERTICAL);
   //sizer23
   m_calc_fill_btn = new wxButton(this, ID_CalcFillBtn, "Fill Holes",
         wxDefaultPosition, wxDefaultSize);
   sizer23->Add(m_calc_fill_btn, 0, wxEXPAND|wxALIGN_CENTER);
   //two operators
   wxBoxSizer *sizer24 = new wxStaticBoxSizer(
         new wxStaticBox(this, wxID_ANY,
            "Two-valued Operators (Require both A and B)"), wxHORIZONTAL);
   m_calc_sub_btn = new wxButton(this, ID_CalcSubBtn, "Subtract",
         wxDefaultPosition, wxSize(50, 25));
   m_calc_add_btn = new wxButton(this, ID_CalcAddBtn, "Add",
         wxDefaultPosition, wxSize(50, 25));
   m_calc_div_btn = new wxButton(this, ID_CalcDivBtn, "Divide",
         wxDefaultPosition, wxSize(50, 25));
   m_calc_isc_btn = new wxButton(this, ID_CalcIscBtn, "AND",
         wxDefaultPosition, wxSize(50, 25));
   sizer24->Add(m_calc_sub_btn, 1, wxEXPAND|wxALIGN_CENTER);
   sizer24->Add(m_calc_add_btn, 1, wxEXPAND|wxALIGN_CENTER);
   sizer24->Add(m_calc_div_btn, 1, wxEXPAND|wxALIGN_CENTER);
   sizer24->Add(m_calc_isc_btn, 1, wxEXPAND|wxALIGN_CENTER);
   //group
   group2->Add(5, 5);
   group2->Add(sizer21, 0, wxEXPAND|wxALIGN_CENTER);
   group2->Add(5, 5);
   group2->Add(sizer22, 0, wxEXPAND|wxALIGN_CENTER);
   group2->Add(5, 5);
   group2->Add(sizer23, 0, wxEXPAND|wxALIGN_CENTER);
   group2->Add(5, 5);
   group2->Add(sizer24, 0, wxEXPAND|wxALIGN_CENTER);

   //all controls
   wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
   sizerV->Add(10, 10);
   sizerV->Add(group1, 0, wxEXPAND);
   sizerV->Add(10, 20);
   sizerV->Add(group2, 0, wxEXPAND);

   SetSizerAndFit(sizerV);
   Layout();

   LoadDefault();
}

BrushToolDlg::~BrushToolDlg()
{
   SaveDefault();
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
      vr_frame->GetColocalizationDlg()->GetSettings(vrv);
   }

   m_cur_view = vrv;
   double dval = 0.0;
   int ival = 0;
   bool bval = false;

   //selection strength
   dval = vrv->GetBrushSclTranslate();
   m_brush_scl_translate_sldr->SetValue(int(dval*1000.0+0.5));
   m_brush_scl_translate_text->ChangeValue(wxString::Format("%.2f", dval));
   //2d influence
   dval = vrv->GetW2d();
   m_brush_2dinfl_sldr->SetValue(int(dval*100.0+0.5));
   m_brush_2dinfl_text->ChangeValue(wxString::Format("%.2f", dval));
   //edge detect
   bval = vrv->GetEdgeDetect();
   m_edge_detect_chk->SetValue(bval);
   //hidden removal
   bval = vrv->GetHiddenRemoval();
   m_hidden_removal_chk->SetValue(bval);
   //select group
   bval = vrv->GetSelectGroup();
   m_select_group_chk->SetValue(bval);

   //size1
   dval = vrv->GetBrushSize1();
   m_brush_size1_sldr->SetValue(int(dval));
   m_brush_size1_text->ChangeValue(wxString::Format("%.0f", dval));
   //size2
   m_brush_size2_chk->SetValue(vrv->GetBrushSize2Link());
   if (vrv->GetBrushSize2Link())
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

   //threshold range
   if (sel_vol)
   {
      m_max_value = sel_vol->GetMaxValue();
      //falloff
      m_brush_scl_translate_sldr->SetRange(0, int(m_max_value*10.0));
      m_brush_scl_translate_text->SetValue(wxString::Format("%.1f", m_dft_scl_translate*m_max_value));
      m_ca_thresh_sldr->SetRange(0, int(m_max_value*10.0));
      m_ca_thresh_sldr->SetValue(int(m_dft_ca_thresh*m_max_value*10.0+0.5));
      m_ca_thresh_text->ChangeValue(wxString::Format("%.1f", m_dft_ca_thresh*m_max_value));
   }

}

void BrushToolDlg::SelectBrush(int id)
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

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
   }
}

//brush commands
void BrushToolDlg::OnBrushAppend(wxCommandEvent &event)
{
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetTree())
   {
      if (m_toolbar->GetToolState(ID_BrushAppend))
         frame->GetTree()->SelectBrush(TreePanel::ID_BrushAppend);
      else
         frame->GetTree()->SelectBrush(0);
      frame->GetTree()->BrushAppend();
   }
}

void BrushToolDlg::OnBrushDiffuse(wxCommandEvent &event)
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetTree())
   {
      if (m_toolbar->GetToolState(ID_BrushDiffuse))
         frame->GetTree()->SelectBrush(TreePanel::ID_BrushDiffuse);
      else
         frame->GetTree()->SelectBrush(0);
      frame->GetTree()->BrushDiffuse();
   }
}

void BrushToolDlg::OnBrushDesel(wxCommandEvent &event)
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);

   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetTree())
   {
      if (m_toolbar->GetToolState(ID_BrushDesel))
         frame->GetTree()->SelectBrush(TreePanel::ID_BrushDesel);
      else
         frame->GetTree()->SelectBrush(0);
      frame->GetTree()->BrushDesel();
   }
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

   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetTree())
      frame->GetTree()->BrushErase();
}

void BrushToolDlg::OnBrushCreate(wxCommandEvent &event)
{
   m_toolbar->ToggleTool(ID_BrushAppend, false);
   m_toolbar->ToggleTool(ID_BrushDiffuse, false);
   m_toolbar->ToggleTool(ID_BrushDesel, false);

   VRenderFrame* frame = (VRenderFrame*)m_frame;
   if (frame && frame->GetTree())
      frame->GetTree()->BrushCreate();
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

   //set falloff
   if (m_cur_view)
      m_cur_view->SetBrushSclTranslate(m_dft_scl_translate);
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

//component analyze
void BrushToolDlg::OnCAThreshChange(wxScrollEvent &event)
{
   int ival = event.GetPosition();
   wxString str = wxString::Format("%.1f", double(ival)/10.0);
   m_ca_thresh_text->SetValue(str);
}

void BrushToolDlg::OnCAThreshText(wxCommandEvent &event)
{
   wxString str = m_ca_thresh_text->GetValue();
   double val;
   str.ToDouble(&val);
   m_dft_ca_thresh = val/m_max_value;
   m_ca_thresh_sldr->SetValue(int(val*10.0+0.5));

   //change mask threshold
   VolumeData* sel_vol = 0;
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
      sel_vol = vr_frame->GetCurSelVol();
   if (sel_vol)
      sel_vol->SetMaskThreshold(m_dft_ca_thresh);
   vr_frame->RefreshVRenderViews();
}

void BrushToolDlg::OnCAAnalyzeBtn(wxCommandEvent &event)
{
   if (m_cur_view)
   {
      bool select = m_ca_select_only_chk->GetValue();
      double min_voxels, max_voxels;
      wxString str = m_ca_min_text->GetValue();
      str.ToDouble(&min_voxels);
      str = m_ca_max_text->GetValue();
      str.ToDouble(&max_voxels);
      bool ignore_max = m_ca_ignore_max_chk->GetValue();
	  bool size_map = m_ca_size_map_chk->GetValue();

	  int comps = m_cur_view->CompAnalysis(min_voxels, ignore_max?-1.0:max_voxels, m_dft_ca_thresh, m_dft_ca_falloff, select, true, size_map);
      int volume = m_cur_view->GetVolumeSelector()->GetVolumeNum();
      //change mask threshold
      VolumeData* sel_vol = 0;
      VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
      if (vr_frame)
         sel_vol = vr_frame->GetCurSelVol();
      if (sel_vol)
      {
         sel_vol->SetUseMaskThreshold(true);
         sel_vol->SetMaskThreshold(m_dft_ca_thresh);
      }
      m_ca_comps_text->SetValue(wxString::Format("%d", comps));
      m_ca_volume_text->SetValue(wxString::Format("%d", volume));
      if (vr_frame)
         vr_frame->RefreshVRenderViews();
   }
}

void BrushToolDlg::OnCAIgnoreMaxChk(wxCommandEvent &event)
{
   if (m_ca_ignore_max_chk->GetValue())
      m_ca_max_text->Disable();
   else
      m_ca_max_text->Enable();
}

void BrushToolDlg::OnCAMultiChannBtn(wxCommandEvent &event)
{
   if (m_cur_view)
   {
      bool select = m_ca_select_only_chk->GetValue();
      m_cur_view->CompExport(0, select);
   }
}

void BrushToolDlg::OnCARandomColorBtn(wxCommandEvent &event)
{
   if (m_cur_view)
   {
      bool select = m_ca_select_only_chk->GetValue();
      m_cur_view->CompExport(1, select);
   }
}

void BrushToolDlg::OnCAAnnotationsBtn(wxCommandEvent &event)
{
   if (m_cur_view)
      m_cur_view->ShowAnnotations();
}

//noise removal
void BrushToolDlg::OnNRSizeChange(wxScrollEvent &event)
{
   int ival = event.GetPosition();
   wxString str = wxString::Format("%d", ival);
   m_nr_size_text->SetValue(str);
}

void BrushToolDlg::OnNRSizeText(wxCommandEvent &event)
{
   wxString str = m_nr_size_text->GetValue();
   double val;
   str.ToDouble(&val);
   m_nr_size_sldr->SetValue(int(val));
}

void BrushToolDlg::OnNRAnalyzeBtn(wxCommandEvent &event)
{
   if (m_cur_view)
   {
      double min_voxels, max_voxels;
      min_voxels = 0.0;
      wxString str = m_nr_size_text->GetValue();
      str.ToDouble(&max_voxels);

      int comps = m_cur_view->NoiseAnalysis(0.0, max_voxels, m_dft_ca_thresh);
      m_cur_view->RefreshGL();
      //m_ca_comps_text->SetValue(wxString::Format("%d", comps));
   }
}

void BrushToolDlg::OnNRRemoveBtn(wxCommandEvent &event)
{
   if (m_cur_view)
   {
      double max_voxels;
      wxString str = m_nr_size_text->GetValue();
      str.ToDouble(&max_voxels);

      m_cur_view->NoiseRemoval(int(max_voxels), m_dft_ca_thresh);
      m_cur_view->RefreshGL();
   }
}

//help button
void BrushToolDlg::OnHelpBtn(wxCommandEvent &event)
{
   ::wxLaunchDefaultBrowser(HELP_PAINT);
}

//save default
void BrushToolDlg::SaveDefault()
{
   wxFileConfig fconfig("FluoRender default brush settings");
   double val;
   wxString str;
   //brush properties
   fconfig.Write("brush_ini_thresh", m_dft_ini_thresh);
   fconfig.Write("brush_gm_falloff", m_dft_gm_falloff);
   fconfig.Write("brush_scl_falloff", m_dft_scl_falloff);
   fconfig.Write("brush_scl_translate", m_dft_scl_translate);
   //edge detect
   fconfig.Write("edge_detect", m_edge_detect_chk->GetValue());
   //hidden removal
   fconfig.Write("hidden_removal", m_hidden_removal_chk->GetValue());
   //select group
   fconfig.Write("select_group", m_select_group_chk->GetValue());
   //2d influence
   str = m_brush_2dinfl_text->GetValue();
   str.ToDouble(&val);
   fconfig.Write("brush_2dinfl", val);
   //size 1
   str = m_brush_size1_text->GetValue();
   str.ToDouble(&val);
   fconfig.Write("brush_size1", val);
   //size2 link
   fconfig.Write("use_brush_size2", m_brush_size2_chk->GetValue());
   //size 2
   str = m_brush_size2_text->GetValue();
   str.ToDouble(&val);
   fconfig.Write("brush_size2", val);
   //iterations
   int ival = m_brush_iterw_rb->GetValue()?1:
      m_brush_iters_rb->GetValue()?2:
      m_brush_iterss_rb->GetValue()?3:0;
   fconfig.Write("brush_iters", ival);
   //component analyzer
   //selected only
   fconfig.Write("ca_select_only", m_ca_select_only_chk->GetValue());
   //min voxel
   str = m_ca_min_text->GetValue();
   str.ToDouble(&val);
   fconfig.Write("ca_min", val);
   //max voxel
   str = m_ca_max_text->GetValue();
   str.ToDouble(&val);
   fconfig.Write("ca_max", val);
   //ignore max
   fconfig.Write("ca_ignore_max", m_ca_ignore_max_chk->GetValue());
   //thresh
   fconfig.Write("ca_thresh", m_dft_ca_thresh);
	//falloff
	fconfig.Write("ca_falloff", m_dft_ca_falloff);
	//size map
	fconfig.Write("ca_size_map", m_ca_size_map_chk->GetValue());	
   //noise removal
   //nr thresh
   fconfig.Write("nr_thresh", m_dft_nr_thresh);
   //nr_size
   fconfig.Write("nr_size", m_dft_nr_size);
#ifdef _DARWIN
    wxString dft = wxString(getenv("HOME")) + "/Fluorender.settings/";
    mkdir(dft,0777);
    chmod(dft,0777);
    dft = dft + "default_brush_settings.dft";
#else
    wxString dft = "default_brush_settings.dft";
#endif
   wxFileOutputStream os(dft);
   fconfig.Save(os);
}

//load default
void BrushToolDlg::LoadDefault()
{
#ifdef _DARWIN
    
    wxString dft = wxString(getenv("HOME")) + "/Fluorender.settings/default_brush_settings.dft";
    std::ifstream tmp(dft);
    if (!tmp.good())
        dft = "FluoRender.app/Contents/Resources/default_brush_settings.dft";
    else
        tmp.close();
#else
    wxString dft = "default_brush_settings.dft";
#endif
   wxFileInputStream is(dft);
   if (!is.IsOk())
      return;
   wxFileConfig fconfig(is);

   wxString str;
   double val;
   int ival;
   bool bval;

   //brush properties
   fconfig.Read("brush_ini_thresh", &m_dft_ini_thresh);
   fconfig.Read("brush_gm_falloff", &m_dft_gm_falloff);
   fconfig.Read("brush_scl_falloff", &m_dft_scl_falloff);
   if (fconfig.Read("brush_scl_translate", &m_dft_scl_translate))
   {
      str = wxString::Format("%.1f", m_dft_scl_translate*m_max_value);
      m_brush_scl_translate_sldr->SetRange(0, int(m_max_value*10.0));
      m_brush_scl_translate_text->SetValue(str);
   }
   //edge detect
   if (fconfig.Read("edge_detect", &bval))
      m_edge_detect_chk->SetValue(bval);
   //hidden removal
   if (fconfig.Read("hidden_removal", &bval))
      m_hidden_removal_chk->SetValue(bval);
   //select group
   if (fconfig.Read("select_group", &bval))
      m_select_group_chk->SetValue(bval);
   //2d influence
   if (fconfig.Read("brush_2dinfl", &val))
   {
      str = wxString::Format("%.2f", val);
      m_brush_2dinfl_text->SetValue(str);
   }
   //size 1
   if (fconfig.Read("brush_size1", &val) && val>0.0)
   {
      str = wxString::Format("%d", (int)val);
      m_brush_size1_text->SetValue(str);
   }
   //size 2 link
   if (fconfig.Read("use_brush_size2", &bval))
   {
      m_brush_size2_chk->SetValue(bval);
      if (bval)
      {
         m_brush_size2_sldr->Enable();
         m_brush_size2_text->Enable();
      }
      else
      {
         m_brush_size2_sldr->Disable();
         m_brush_size2_text->Disable();
      }
   }
   //size 2
   if (fconfig.Read("brush_size2", &val) && val>0.0)
   {
      str = wxString::Format("%d", (int)val);
      m_brush_size2_text->SetValue(str);
   }
   //iterations
   if (fconfig.Read("brush_iters", &ival))
   {
      switch (ival)
      {
      case 1:
         m_brush_iterw_rb->SetValue(true);
         m_brush_iters_rb->SetValue(false);
         m_brush_iterss_rb->SetValue(false);
         break;
      case 2:
         m_brush_iterw_rb->SetValue(false);
         m_brush_iters_rb->SetValue(true);
         m_brush_iterss_rb->SetValue(false);
         break;
      case 3:
         m_brush_iterw_rb->SetValue(false);
         m_brush_iters_rb->SetValue(false);
         m_brush_iterss_rb->SetValue(true);
         break;
      }
   }
   //component analyzer
   //selected only
   if (fconfig.Read("ca_select_only", &bval))
      m_ca_select_only_chk->SetValue(bval);
   //min voxel
   if (fconfig.Read("ca_min", &ival))
   {
      m_dft_ca_min = ival;
      str = wxString::Format("%d", ival);
      m_ca_min_text->SetValue(str);
   }
   //max voxel
   if (fconfig.Read("ca_max", &ival))
   {
      m_dft_ca_max = ival;
      str = wxString::Format("%d", ival);
      m_ca_max_text->SetValue(str);
   }
   //ignore max
   if (fconfig.Read("ca_ignore_max", &bval))
   {
      m_ca_ignore_max_chk->SetValue(bval);
      if (bval)
         m_ca_max_text->Disable();
      else
         m_ca_max_text->Enable();
   }
   //thresh
   if (fconfig.Read("ca_thresh", &m_dft_ca_thresh))
   {
      str = wxString::Format("%.1f", m_dft_ca_thresh*m_max_value);
      m_ca_thresh_sldr->SetRange(0, int(m_max_value*10.0));
      m_ca_thresh_text->SetValue(str);
   }
	//falloff
	fconfig.Read("ca_falloff", &m_dft_ca_falloff);
	//size map
	if (fconfig.Read("ca_size_map", &bval))
		m_ca_size_map_chk->SetValue(bval);   
   //nr thresh
   fconfig.Read("nr_thresh", &m_dft_nr_thresh);
   //nr size
   if (fconfig.Read("nr_size", &m_dft_nr_size))
   {
      str = wxString::Format("%d", (int)val);
      m_nr_size_text->SetValue(str);
   }
}

//calculations
//operands
void BrushToolDlg::OnLoadA(wxCommandEvent &event)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_vol1 = vr_frame->GetCurSelVol();
      if (m_vol1)
         m_calc_a_text->SetValue(m_vol1->GetName());
   }
}

void BrushToolDlg::OnLoadB(wxCommandEvent &event)
{
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_vol2 = vr_frame->GetCurSelVol();
      if (m_vol2)
         m_calc_b_text->SetValue(m_vol2->GetName());
   }
}

//operators
void BrushToolDlg::OnCalcSub(wxCommandEvent &event)
{
   if (!m_vol1 || !m_vol2)
      return;

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_cur_view = 0;
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         wxString str = m_vol1->GetName();
         if (vrv && vrv->GetVolumeData(str))
         {
            m_cur_view = vrv;
            break;
         }
      }

      if (!m_cur_view)
         m_cur_view = vr_frame->GetView(0);

      if (m_cur_view)
      {
         m_cur_view->SetVolumeA(m_vol1);
         m_cur_view->SetVolumeB(m_vol2);
         m_cur_view->Calculate(1);
      }
   }
}

void BrushToolDlg::OnCalcAdd(wxCommandEvent &event)
{
   if (!m_vol1 || !m_vol2)
      return;

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_cur_view = 0;
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         wxString str = m_vol1->GetName();
         if (vrv && vrv->GetVolumeData(str))
         {
            m_cur_view = vrv;
            break;
         }
      }

      if (!m_cur_view)
         m_cur_view = vr_frame->GetView(0);

      if (m_cur_view)
      {
         m_cur_view->SetVolumeA(m_vol1);
         m_cur_view->SetVolumeB(m_vol2);
         m_cur_view->Calculate(2);
      }
   }
}

void BrushToolDlg::OnCalcDiv(wxCommandEvent &event)
{
   if (!m_vol1 || !m_vol2)
      return;

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_cur_view = 0;
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         wxString str = m_vol1->GetName();
         if (vrv && vrv->GetVolumeData(str))
         {
            m_cur_view = vrv;
            break;
         }
      }

      if (!m_cur_view)
         m_cur_view = vr_frame->GetView(0);

      if (m_cur_view)
      {
         m_cur_view->SetVolumeA(m_vol1);
         m_cur_view->SetVolumeB(m_vol2);
         m_cur_view->Calculate(3);
      }
   }
}

void BrushToolDlg::OnCalcIsc(wxCommandEvent &event)
{
   if (!m_vol1 || !m_vol2)
      return;

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_cur_view = 0;
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         wxString str = m_vol1->GetName();
         if (vrv && vrv->GetVolumeData(str))
         {
            m_cur_view = vrv;
            break;
         }
      }

      if (!m_cur_view)
         m_cur_view = vr_frame->GetView(0);

      if (m_cur_view)
      {
         m_cur_view->SetVolumeA(m_vol1);
         m_cur_view->SetVolumeB(m_vol2);
         m_cur_view->Calculate(4);
      }
   }
}

//one-operators
void BrushToolDlg::OnCalcFill(wxCommandEvent &event)
{
   if (!m_vol1)
      return;

   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame)
   {
      m_cur_view = 0;
      for (int i=0; i<vr_frame->GetViewNum(); i++)
      {
         VRenderView* vrv = vr_frame->GetView(i);
         wxString str = m_vol1->GetName();
         if (vrv && vrv->GetVolumeData(str))
         {
            m_cur_view = vrv;
            break;
         }
      }

      if (!m_cur_view)
         m_cur_view = vr_frame->GetView(0);

      if (m_cur_view)
      {
         m_cur_view->SetVolumeA(m_vol1);
         m_vol2 = 0;
         m_cur_view->SetVolumeB(0);
         m_calc_b_text->Clear();
         m_cur_view->Calculate(9);
      }
   }
}
