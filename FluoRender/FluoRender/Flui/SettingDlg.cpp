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
#include <SettingDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>
#include <wxSingleSlider.h>
#include <ModalDlg.h>
#include <wx/valnum.h>
#include <wx/notebook.h>
#include <wx/display.h>
#include <wx/valtext.h>
#include <wx/regex.h>

wxWindow* SettingDlg::CreateProjectPage(wxWindow *parent)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st;
	wxPanel *page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//project save
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Open/Save/Script Run/UI"), wxVERTICAL);
	m_prj_save_chk = new wxCheckBox(page, wxID_ANY,
		"Save project when capture viewport or export movie.");
	m_prj_save_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnProjectSaveCheck, this);
	m_prj_save_inc_chk = new wxCheckBox(page, wxID_ANY,
		"Save project in new files with incremental serial numbers.");
	m_prj_save_inc_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnProjectSaveIncCheck, this);
	m_realtime_cmp_chk = new wxCheckBox(page, wxID_ANY,
		"Compress data in graphics memory when loading.");
	m_realtime_cmp_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnRealtimeCompressCheck, this);
	m_script_break_chk = new wxCheckBox(page, wxID_ANY,
		"Allow script information prompts.");
	m_script_break_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnScriptBreakCheck, this);
	m_inverse_slider_chk = new wxCheckBox(page, wxID_ANY,
		"Invert vertical slider orientation.");
	m_inverse_slider_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnInverseSliderCheck, this);
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Set multifunction buttons to:");
	m_mul_func_btn_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> items = {"Sync Channels", "Focused Scroll", "Use Default", "Use ML", "Undo", "Enable/Disable"};
	m_mul_func_btn_comb->Append(items);
	m_mul_func_btn_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnMulFuncBtnComb, this);
	sizer1_1->Add(st);
	sizer1_1->Add(10, 10);
	sizer1_1->Add(m_mul_func_btn_comb);
	sizer1_1->Add(10, 10);
	group1->Add(10, 5);
	group1->Add(m_prj_save_chk);
	group1->Add(10, 5);
	group1->Add(m_prj_save_inc_chk);
	group1->Add(10, 5);
	group1->Add(m_realtime_cmp_chk);
	group1->Add(10, 5);
	group1->Add(m_script_break_chk);
	group1->Add(10, 5);
	group1->Add(m_inverse_slider_chk);
	group1->Add(10, 5);
	group1->Add(sizer1_1);
	group1->Add(10, 5);

	//font
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Render View Text"), wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Font:");
	m_font_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(150, -1)), 0, NULL, wxCB_READONLY);
	m_font_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnFontChange, this);
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_font_cmb);
	sizer2_1->Add(10, 10);
	st = new wxStaticText(page, 0, "Size:");
	m_font_size_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), 0, NULL);
	for (int df = 3; df < 18; ++df)
		m_font_size_cmb->Append(wxString::Format("%d", int(std::round(std::pow(df, 1.5)))));
	m_font_size_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnFontSizeChange, this);
	m_font_size_cmb->Bind(wxEVT_TEXT, &SettingDlg::OnFontSizeChange, this);
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_font_size_cmb);
	sizer2_1->Add(10, 10);
	st = new wxStaticText(page, 0, "Color:");
	m_text_color_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> items2 = { "BG inverted", "Background", "Vol sec color" };
	m_text_color_cmb->Append(items2);
	m_text_color_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnTextColorChange, this);
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_text_color_cmb);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);
	st = new wxStaticText(page, 0,
		"Put TrueType font files into the \"Fonts\" folder.\n"\
		"Restart FluoRender to load new font files.");
	group2->Add(st);
	group2->Add(10, 5);

	//line width
	wxBoxSizer *group3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Line Width"), wxVERTICAL);
	wxBoxSizer *sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	m_line_width_sldr = new wxSingleSlider(page, wxID_ANY, 3, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_line_width_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnLineWidthSldr, this);
	m_line_width_text = new wxTextCtrl(page, wxID_ANY, "3",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_line_width_text->Bind(wxEVT_TEXT, &SettingDlg::OnLineWidthText, this);
	sizer3_1->Add(m_line_width_sldr, 1, wxEXPAND);
	sizer3_1->Add(m_line_width_text, 0, wxALIGN_CENTER);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);

	//paint history depth
	wxBoxSizer *group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Paint History"), wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	m_paint_hist_depth_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_paint_hist_depth_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPaintHistDepthChange, this);
	m_paint_hist_depth_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_paint_hist_depth_text->Bind(wxEVT_TEXT, &SettingDlg::OnPaintHistDepthEdit, this);
	st = new wxStaticText(page, 0,
		"The number of undo steps for paint brush selection.\n" \
		"Set the value to 0 to disable history.\n" \
		"A value greater than 0 slows down speed and increases memory usage.");
	sizer4_1->Add(m_paint_hist_depth_sldr, 1, wxEXPAND);
	sizer4_1->Add(m_paint_hist_depth_text, 0, wxALIGN_CENTER);
	group4->Add(10, 5);
	group4->Add(sizer4_1, 0, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(st);
	group4->Add(10, 5);

	//pencil distance
	wxBoxSizer* group5 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Ruler Point Distance"), wxVERTICAL);
	wxBoxSizer* sizer5_1 = new wxBoxSizer(wxHORIZONTAL);
	m_pencil_dist_sldr = new wxSingleSlider(page, wxID_ANY, 30, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pencil_dist_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPencilDistChange, this);
	m_pencil_dist_text = new wxTextCtrl(page, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_pencil_dist_text->Bind(wxEVT_TEXT, &SettingDlg::OnPencilDistEdit, this);
	st = new wxStaticText(page, 0,
		"The pixel distance between two ruler points for pencil and magnet.\n");
	sizer5_1->Add(m_pencil_dist_sldr, 1, wxEXPAND);
	sizer5_1->Add(m_pencil_dist_text, 0, wxALIGN_CENTER);
	group5->Add(10, 5);
	group5->Add(sizer5_1, 0, wxEXPAND);
	group5->Add(10, 5);
	group5->Add(st);
	group5->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group4, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group5, 0, wxEXPAND);
	sizerV->Add(10, 10);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreateRenderingPage(wxWindow *parent)
{
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	vald_fp2.SetRange(-180.0, 180.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st;

	wxPanel *page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//micro blending
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Micro Blending"), wxVERTICAL);
	m_micro_blend_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Micro Blending");
	m_micro_blend_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnMicroBlendCheck, this);
	st = new wxStaticText(page, 0,
		"Micro Blending works only on multiple channels in the depth mode.\n"\
		"Enable Micro Blending to render the colors of the colocalized voxels correctly.\n"\
		"On certain systems, it may slow down the rendering speed significantly.");
	group1->Add(10, 5);
	group1->Add(m_micro_blend_chk);
	group1->Add(10, 5);
	group1->Add(st);
	group1->Add(10, 5);

	//depth peeling
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Mesh Transparency Quality"), wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_peeling_layers_sldr = new wxSingleSlider(page, wxID_ANY, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_peeling_layers_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPeelingLayersChange, this);
	m_peeling_layers_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_peeling_layers_text->Bind(wxEVT_TEXT, &SettingDlg::OnPeelingLayersEdit, this);
	st = new wxStaticText(page, 0,
		"The number of depth peeling layers for rendering transparent mesh objects.\n"\
		"Set higher numbers only for complex geometries.\n"\
		"It slows down the rendering speed.");
	sizer2_1->Add(m_peeling_layers_sldr, 1, wxEXPAND);
	sizer2_1->Add(m_peeling_layers_text, 0, wxALIGN_CENTER);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(st);
	group2->Add(10, 5);

	//shadow direction
	wxBoxSizer *group3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Shadow Direction"), wxVERTICAL);
	wxBoxSizer *sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	m_shadow_dir_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Directional Shadow");
	m_shadow_dir_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnShadowDirCheck, this);
	m_shadow_dir_sldr = new wxSingleSlider(page, wxID_ANY, -45, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_dir_sldr->SetRangeStyle(2);
	m_shadow_dir_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnShadowDirChange, this);
	m_shadow_dir_text = new wxTextCtrl(page, wxID_ANY, "-45",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_fp2);
	m_shadow_dir_text->Bind(wxEVT_TEXT, &SettingDlg::OnShadowDirEdit, this);
	st = new wxStaticText(page, 0,
		"The direction of the shadows, when shadow is enabled for volume data.");
	sizer3_1->Add(m_shadow_dir_chk, 0, wxALIGN_CENTER);
	sizer3_1->Add(m_shadow_dir_sldr, 1, wxEXPAND);
	sizer3_1->Add(m_shadow_dir_text, 0, wxALIGN_CENTER);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);
	group3->Add(st);
	group3->Add(10, 5);

	//rotations
	wxBoxSizer* group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Rotations"), wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "RC Anchor Start");
	m_pin_threshold_sldr = new wxSingleSlider(page, wxID_ANY, 100, 10, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pin_threshold_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPinThresholdChange, this);
	m_pin_threshold_text = new wxTextCtrl(page, wxID_ANY, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_pin_threshold_text->Bind(wxEVT_TEXT, &SettingDlg::OnPinThresholdEdit, this);
	sizer4_1->Add(st, 0, wxALIGN_CENTER);
	sizer4_1->Add(m_pin_threshold_sldr, 1, wxEXPAND);
	sizer4_1->Add(m_pin_threshold_text, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer4_2 = new wxBoxSizer(wxHORIZONTAL);
	m_rot_link_chk = new wxCheckBox(page, wxID_ANY,
		"Link All Rendering Views' Rotations.");
	m_rot_link_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnRotLink, this);
	sizer4_2->Add(m_rot_link_chk, 0, wxALIGN_CENTER);
	group4->Add(10, 5);
	group4->Add(sizer4_1, 0, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(sizer4_2, 0, wxEXPAND);
	group4->Add(10, 5);

	//gradient background
	wxBoxSizer *group5 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Gradient Background"), wxVERTICAL);
	wxBoxSizer *sizer5_1 = new wxBoxSizer(wxHORIZONTAL);
	m_grad_bg_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Gradient Background");
	m_grad_bg_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnGradBgCheck, this);
	sizer5_1->Add(m_grad_bg_chk, 0, wxALIGN_CENTER);
	group5->Add(10, 5);
	group5->Add(sizer5_1, 0, wxEXPAND);
	group5->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);

	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group4, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group5, 0, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreatePerformancePage(wxWindow *parent)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxIntegerValidator<int> vald_int2;
	wxStaticText* st;
	wxPanel *page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//mouse interactions
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Variable Sample Rate"), wxVERTICAL);
	m_mouse_int_chk = new wxCheckBox(page, wxID_ANY,
		"Reduce volume sample rate for mouse interactions.\n"\
		"Enable this option if mouse interaction speed is slow.");
	m_mouse_int_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnMouseIntCheck, this);
	group1->Add(10, 5);
	group1->Add(m_mouse_int_chk);
	group1->Add(10, 5);

	//memory settings
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Large Data Streaming"), wxVERTICAL);
	m_streaming_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Streaming for Large Data.");
	m_streaming_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnStreamingChk, this);
	wxString choices[2] = {"Back to Front", "Front to Back"};
	m_update_order_rbox = new wxRadioBox(page, wxID_ANY,
		"Update Order", wxDefaultPosition, wxDefaultSize,
		2, choices, 0, wxRA_SPECIFY_COLS);
	m_update_order_rbox->Bind(wxEVT_RADIOBOX, &SettingDlg::OnUpdateOrderChange, this);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Graphics Memory:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_1->Add(st);
	m_graphics_mem_sldr = new wxSingleSlider(page, wxID_ANY, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_graphics_mem_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnGraphicsMemChange, this);
	m_graphics_mem_text = new wxTextCtrl(page, wxID_ANY, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_graphics_mem_text->Bind(wxEVT_TEXT, &SettingDlg::OnGraphicsMemEdit, this);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_1->Add(m_graphics_mem_sldr, 1, wxEXPAND);
	sizer2_1->Add(m_graphics_mem_text, 0, wxALIGN_CENTER);
	sizer2_1->Add(st);
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Large Data Size:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_2->Add(st);
	m_large_data_sldr = new wxSingleSlider(page, wxID_ANY, 20, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_large_data_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnLargeDataChange, this);
	m_large_data_text = new wxTextCtrl(page, wxID_ANY, "200",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_large_data_text->Bind(wxEVT_TEXT, &SettingDlg::OnLargeDataEdit, this);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_2->Add(m_large_data_sldr, 1, wxEXPAND);
	sizer2_2->Add(m_large_data_text, 0, wxALIGN_CENTER);
	sizer2_2->Add(st);
	wxBoxSizer *sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Brick Size:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_3->Add(st);
	m_block_size_sldr = new wxSingleSlider(page, wxID_ANY, 7, 6, 12,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_block_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnBlockSizeChange, this);
	m_block_size_text = new wxTextCtrl(page, wxID_ANY, "128",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_block_size_text->Bind(wxEVT_TEXT, &SettingDlg::OnBlockSizeEdit, this);
	st = new wxStaticText(page, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_3->Add(m_block_size_sldr, 1, wxEXPAND);
	sizer2_3->Add(m_block_size_text, 0, wxALIGN_CENTER);
	sizer2_3->Add(st);
	wxBoxSizer *sizer2_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Response Time:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_4->Add(st);
	m_response_time_sldr = new wxSingleSlider(page, wxID_ANY, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_response_time_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnResponseTimeChange, this);
	m_response_time_text = new wxTextCtrl(page, wxID_ANY, "100",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_response_time_text->Bind(wxEVT_TEXT, &SettingDlg::OnResponseTimeEdit, this);
	st = new wxStaticText(page, 0, "ms",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_4->Add(m_response_time_sldr, 1, wxEXPAND);
	sizer2_4->Add(m_response_time_text, 0, wxALIGN_CENTER);
	sizer2_4->Add(st);
	wxBoxSizer *sizer2_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Detail Level Offset:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_5->Add(st);
	m_detail_level_offset_sldr = new wxSingleSlider(page, wxID_ANY, 0, -5, 5,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_detail_level_offset_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnDetailLevelOffsetChange, this);
	m_detail_level_offset_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int2);
	m_detail_level_offset_text->Bind(wxEVT_TEXT, &SettingDlg::OnDetailLevelOffsetEdit, this);
	sizer2_5->Add(m_detail_level_offset_sldr, 1, wxEXPAND);
	sizer2_5->Add(m_detail_level_offset_text, 0, wxALIGN_CENTER);
	sizer2_5->Add(20, 5);
	group2->Add(10, 5);
	group2->Add(m_streaming_chk);
	group2->Add(10, 10);
	group2->Add(m_update_order_rbox);
	group2->Add(10, 10);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_2, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_3, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_4, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_5, 0, wxEXPAND);
	group2->Add(10, 5);
	st = new wxStaticText(page, 0,
		"Note: Configure these settings before loading data.\n"\
		"Data streaming allows rendering and processing data of larger size than\n"\
		"available graphics memory. Data are divided into bricks. The bricks are\n"\
		"sequentially loaded into graphics memory for rendering and processing.\n"\
		"Different computer hardware may need different settings. You may need to\n"\
		"experiment in order to find the best settings for your computer.");
	group2->Add(st);
	group2->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreateDisplayPage(wxWindow* parent)
{
	wxIntegerValidator<unsigned int> vald_int;
	wxFloatingPointValidator<double> vald_fp1(1);
	wxStaticText* st;
	std::vector<wxString> cmb_str;

	wxPanel* page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//stereo
	wxBoxSizer* group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Stereography / Virtual Reality / Holography"), wxVERTICAL);
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_stereo_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Stereography");
	m_stereo_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnStereoCheck, this);
	sizer1_1->Add(5, 5);
	sizer1_1->Add(m_stereo_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "API:",
		wxDefaultPosition, FromDIP(wxSize(30, -1)));
	m_xr_api_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_xr_api_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnXrApiComb, this);
#ifdef _WIN32
	cmb_str = { "Cardboard", "OpenXR", "OpenVR", "Windows Mixed Reality", "Holographic Remoting" };
#else
	cmb_str = { "Cardboard", "OpenXR", "OpenVR" };
#endif
	m_xr_api_cmb->Append(cmb_str);
	sizer1_2->Add(20, 5);
	sizer1_2->Add(st, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_xr_api_cmb, 0, wxALIGN_CENTER);
#ifdef _WIN32
	st = new wxStaticText(page, 0, "Remote HMD IP:",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	wxArrayString allowedChars;
	allowedChars.Add("0"); allowedChars.Add("1"); allowedChars.Add("2");
	allowedChars.Add("3"); allowedChars.Add("4"); allowedChars.Add("5");
	allowedChars.Add("6"); allowedChars.Add("7"); allowedChars.Add("8");
	allowedChars.Add("9"); allowedChars.Add(".");
	wxTextValidator val_ip(wxFILTER_INCLUDE_CHAR_LIST);
	val_ip.SetIncludes(allowedChars);
	m_holo_ip_text = new wxTextCtrl(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, 20)), 0, val_ip);
	m_holo_ip_text->Bind(wxEVT_TEXT, &SettingDlg::OnHoloIpEdit, this);
	sizer1_2->Add(20, 5);
	sizer1_2->Add(st, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_holo_ip_text, 0, wxALIGN_CENTER);
#endif
	wxBoxSizer* sizer1_3 = new wxBoxSizer(wxHORIZONTAL);
	m_mv_hmd_chk = new wxCheckBox(page, wxID_ANY,
		"Get Model-View Matrix from HMD");
	m_mv_hmd_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnMvHmdCheck, this);
	sizer1_3->Add(20, 5);
	sizer1_3->Add(m_mv_hmd_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_4 = new wxBoxSizer(wxHORIZONTAL);
	m_sbs_chk = new wxCheckBox(page, wxID_ANY,
		"Aspect Ratio for 3D TV");
	m_sbs_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnSBSCheck, this);
	sizer1_4->Add(20, 5);
	sizer1_4->Add(m_sbs_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Eye Distance:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_eye_dist_sldr = new wxSingleSlider(page, wxID_ANY, 200, 0, 2000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_eye_dist_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnEyeDistChange, this);
	m_eye_dist_text = new wxTextCtrl(page, wxID_ANY, "20.0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_fp1);
	m_eye_dist_text->Bind(wxEVT_TEXT, &SettingDlg::OnEyeDistEdit, this);
	sizer1_5->Add(20, 5);
	sizer1_5->Add(st, 0, wxALIGN_CENTER);
	sizer1_5->Add(m_eye_dist_sldr, 1, wxEXPAND);
	sizer1_5->Add(m_eye_dist_text, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_6 = new wxBoxSizer(wxHORIZONTAL);
	m_looking_glass_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Holography");
	m_looking_glass_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnLookingGlassCheck, this);
	sizer1_6->Add(5, 5);
	sizer1_6->Add(m_looking_glass_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Install Looking Glass Bridge and connect a Looking Glass display.\nThen send the full-screen render view to the display.");
	sizer1_7->Add(20, 5);
	sizer1_7->Add(st, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "View Offset:",
		wxDefaultPosition, FromDIP(wxSize(70, 20)));
	m_lg_offset_sldr = new wxSingleSlider(page, wxID_ANY, 20, 0, 90,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_lg_offset_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnLgOffsetChange, this);
	m_lg_offset_text = new wxTextCtrl(page, wxID_ANY, "20",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_lg_offset_text->Bind(wxEVT_TEXT, &SettingDlg::OnLgOffsetEdit, this);
	sizer1_8->Add(20, 5);
	sizer1_8->Add(st, 0, wxALIGN_CENTER);
	sizer1_8->Add(m_lg_offset_sldr, 1, wxEXPAND);
	sizer1_8->Add(m_lg_offset_text, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_9 = new wxBoxSizer(wxHORIZONTAL);
	m_holo_debug_chk = new wxCheckBox(page, wxID_ANY,
		"Show Quilt");
	m_holo_debug_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnHoloDebugCheck, this);
	sizer1_9->Add(20, 5);
	sizer1_9->Add(m_holo_debug_chk, 0, wxALIGN_CENTER);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_2, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_3, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_4, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_5, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_6, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_7, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_8, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_9, 0, wxEXPAND);
	group1->Add(10, 5);

	//full screen display
	wxBoxSizer* group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Fullscreen on Display"), wxVERTICAL);
	wxBoxSizer* sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Display:",
		wxDefaultPosition, FromDIP(wxSize(100, -1)));
	m_disp_id_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_disp_id_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnDispIdComb, this);
	int dn = wxDisplay::GetCount();
	wxDisplay* display = 0;
	wxString disp_name;
	cmb_str.clear();
	for (int i = 0; i < dn; ++i)
	{
		display = new wxDisplay(i);
		disp_name = display->GetName();
		if (disp_name.IsEmpty())
			cmb_str.push_back(wxString::Format("%d", i));
		else
			cmb_str.push_back(wxString::Format("%d", i) + ": " + disp_name);
		delete display;
	}
	m_disp_id_comb->Append(cmb_str);
	sizer2_1->Add(st, 0, wxALIGN_CENTER);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(m_disp_id_comb, 0, wxALIGN_CENTER);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);

	//color depth
	wxBoxSizer* group3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Color Depth of Render View"), wxVERTICAL);
	wxBoxSizer* sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Color Depth:",
		wxDefaultPosition, FromDIP(wxSize(100, -1)));
	m_color_depth_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_color_depth_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnColorDepthComb, this);
	cmb_str = { "8", "10", "16" };
	m_color_depth_comb->Append(cmb_str);
	sizer3_1->Add(st, 0, wxALIGN_CENTER);
	sizer3_1->Add(5, 5);
	sizer3_1->Add(m_color_depth_comb, 0, wxALIGN_CENTER);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);

	wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreateFormatPage(wxWindow *parent)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//float 2
	wxFloatingPointValidator<double> vald_fp1(2);
	wxStaticText* st;
	wxPanel *page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//override vox
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Override Voxel Size"), wxVERTICAL);
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_override_vox_chk = new wxCheckBox(page, wxID_ANY,
		"Get voxel size info from the first opened data set.");
	m_override_vox_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnOverrideVoxCheck, this);
	sizer1_1->Add(m_override_vox_chk, 0, wxALIGN_CENTER);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);

	//wavelength to color
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Default Colors for Excitation Wavelengths (nm) (for raw microscopy formats)"), wxVERTICAL);
	//combo box line
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_wav_color1_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	m_wav_color2_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	m_wav_color3_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	m_wav_color4_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> list = { "Red", "Green", "Blue", "Cyan", "Magenta", "Yellow", "Orange", "White" };
	m_wav_color1_cmb->Append(list);
	m_wav_color2_cmb->Append(list);
	m_wav_color3_cmb->Append(list);
	m_wav_color4_cmb->Append(list);
	m_wav_color1_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnWavColor1Change, this);
	m_wav_color2_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnWavColor2Change, this);
	m_wav_color3_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnWavColor3Change, this);
	m_wav_color4_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnWavColor4Change, this);
	sizer2_1->Add(35, 20);
	sizer2_1->Add(m_wav_color1_cmb);
	sizer2_1->Add(25, 20);
	sizer2_1->Add(m_wav_color2_cmb);
	sizer2_1->Add(25, 20);
	sizer2_1->Add(m_wav_color3_cmb);
	sizer2_1->Add(25, 20);
	sizer2_1->Add(m_wav_color4_cmb);
	sizer2_1->Add(30, 20);
	//static text line
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer2_2->Add(10, 20);
	st = new wxStaticText(page, 0, "350", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "450\n480\n488", wxDefaultPosition, FromDIP(wxSize(30, 50)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "543\n568", wxDefaultPosition, FromDIP(wxSize(30, 35)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "633", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "700", wxDefaultPosition, FromDIP(wxSize(30, 20)));
	sizer2_2->Add(st);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0);
	group2->Add(10, 5);
	group2->Add(sizer2_2, 0);
	group2->Add(10, 5);

	//max texture size
	wxBoxSizer *group3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Max Texture Size"), wxVERTICAL);
	wxBoxSizer *sizer3_1 = new wxBoxSizer(wxHORIZONTAL);
	m_max_texture_size_chk = new wxCheckBox(page, wxID_ANY,
		"Set Max Texture Size");
	m_max_texture_size_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnMaxTextureSizeChk, this);
	m_max_texture_size_text = new wxTextCtrl(page, wxID_ANY, "2048",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_max_texture_size_text->Bind(wxEVT_TEXT, &SettingDlg::OnMaxTextureSizeEdit, this);
	sizer3_1->Add(m_max_texture_size_chk, 0, wxALIGN_CENTER);
	sizer3_1->Add(10, 10);
	sizer3_1->Add(m_max_texture_size_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0,
		"Note:\n"\
		"Set a texture size smaller than 1024 to correctly perform component analysis.\n"\
		"When both the max texture size and data streaming are set, the smaller number\n"\
		"is used to divide a large data set into bricks.\n"\
		"Restart FluoRender after setting this value.");
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0);
	group3->Add(10, 5);
	group3->Add(st, 0);
	group3->Add(10, 5);

	//cl devices
	wxBoxSizer *group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "OpenCL Devices"), wxVERTICAL);
	m_device_tree = new wxTreeCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxTR_FULL_ROW_HIGHLIGHT);
	m_device_tree->Bind(wxEVT_TREE_SEL_CHANGED, &SettingDlg::OnSelChanged, this);
	st = new wxStaticText(page, 0,
		"Select an OpenCL device that is also the rendering GPU.\n"\
		"Restart FluoRender after changing OpenCL devices.");
	group4->Add(10, 5);
	group4->Add(m_device_tree, 1, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(st, 0);
	group4->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group4, 1, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreateJavaPage(wxWindow *parent)
{
	wxStaticText* st;
	wxPanel *page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//JVM settings.
	wxBoxSizer *group1 = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, "Java Settings"), wxVERTICAL);
	wxBoxSizer *sizer1_0 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *sizer1_3 = new wxBoxSizer(wxHORIZONTAL);	

	m_java_jvm_text = new wxTextCtrl(page, wxID_ANY);
	m_java_jvm_text->Bind(wxEVT_TEXT, &SettingDlg::OnJavaJvmEdit, this);
	m_java_ij_text = new wxTextCtrl(page, wxID_ANY);
	m_java_ij_text->Bind(wxEVT_TEXT, &SettingDlg::OnJavaIJEdit, this);
	m_java_bioformats_text = new wxTextCtrl(page, wxID_ANY);
	m_java_bioformats_text->Bind(wxEVT_TEXT, &SettingDlg::OnJavaBioformatsEdit, this);
	m_browse_jvm_btn = new wxButton(page, wxID_ANY, "Browse", wxDefaultPosition);
	m_browse_jvm_btn->Bind(wxEVT_BUTTON, &SettingDlg::onJavaJvmBrowse, this);
	m_browse_ij_btn = new wxButton(page, wxID_ANY, "Browse", wxDefaultPosition);
	m_browse_ij_btn->Bind(wxEVT_BUTTON, &SettingDlg::onJavaIJBrowse, this);
	m_browse_bioformats_btn = new wxButton(page, wxID_ANY, "Browse", wxDefaultPosition);
	m_browse_bioformats_btn->Bind(wxEVT_BUTTON, &SettingDlg::onJavaBioformatsBrowse, this);

	sizer1_1->Add(m_java_jvm_text, 1, wxEXPAND);
	sizer1_1->Add(m_browse_jvm_btn, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_java_ij_text, 1, wxEXPAND);
	sizer1_2->Add(m_browse_ij_btn, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_java_bioformats_text, 1, wxEXPAND);
	sizer1_3->Add(m_browse_bioformats_btn, 0, wxALIGN_CENTER);
	
	group1->Add(5, 5);

	//Added the radio button
	st = new wxStaticText(page, 0, "Package:");
	mp_radio_button_imagej = new wxRadioButton(page, wxID_ANY, "ImageJ", wxDefaultPosition);
	mp_radio_button_imagej->Bind(wxEVT_RADIOBUTTON, &SettingDlg::onJavaRadioButtonImageJ, this);
	mp_radio_button_fiji = new wxRadioButton(page, wxID_ANY, "Fiji", wxDefaultPosition);
	mp_radio_button_fiji->Bind(wxEVT_RADIOBUTTON, &SettingDlg::onJavaRadioButtonFiji, this);
	sizer1_0->Add(st);
	sizer1_0->Add(5, 5);
	sizer1_0->Add(mp_radio_button_imagej);
	sizer1_0->Add(5, 5);
	sizer1_0->Add(mp_radio_button_fiji);
	group1->Add(sizer1_0, 0, wxEXPAND);
	mp_radio_button_imagej->SetValue(true);

	group1->Add(10, 5);
#ifdef _WIN32
    st = new wxStaticText(page, 0, "Path to the root folder of ImageJ or Fiji (e.g., \"ImageJ\" or \"Fiji.app\")");
#else
    st = new wxStaticText(page, 0, "Path to folder \"ImageJ.app\" or \"Fiji.app\" (e.g., \"ImageJ/ImageJ.app\" or \"Fiji.app\") :");
#endif
    group1->Add(st);
    group1->Add(sizer1_2, 0, wxEXPAND);
    group1->Add(10, 10);

#ifdef _WIN32
	st = new wxStaticText(page, 0, "Path to file \"jvm.dll\" (e.g., \"ImageJ\\jre\\bin\\server\\jvm.dll\"):");
#else
	st = new wxStaticText(page, 0, "Path to file \"libjvm.dylib\" (e.g., \"ImageJ/jre/lib/server/libjvm.dylib\"):");
#endif
	group1->Add(st);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 10);
    
#ifdef _WIN32
	st = new wxStaticText(page, 0, "Path to file \"bioformats_package.jar\" (e.g., \"ImageJ\\plugins\\bioformats_package.jar\"):");
#else
	st = new wxStaticText(page, 0, "Path to file \"bioformats_package.jar\" (e.g., \"ImageJ/plugins/bioformats_package.jar\"):");
#endif
	group1->Add(st);
	group1->Add(sizer1_3, 0, wxEXPAND);
	group1->Add(10, 10);

	st = new wxStaticText(page, 0,
		"Note:\n" \
		"Restart FluoRender for Java settings to take effect.\n" \
		"Bioformats and JVM are required when ImageJ is selected.");
	group1->Add(st);
	group1->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);

	page->SetSizer(sizerV);
	return page;
}

SettingDlg::SettingDlg(MainFrame *frame) :
	PropPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(450, 750)),
		0, "SettingDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	notebook->AddPage(CreateProjectPage(notebook), "Project");
	notebook->AddPage(CreateRenderingPage(notebook), "Rendering");
	notebook->AddPage(CreatePerformancePage(notebook), "Performance");
	notebook->AddPage(CreateDisplayPage(notebook), "Display");
	notebook->AddPage(CreateFormatPage(notebook), "File format");
	notebook->AddPage(CreateJavaPage(notebook), "ImageJ Link");

	//buttons
	wxBoxSizer *group_b = new wxBoxSizer(wxHORIZONTAL);
	m_save_btn = new wxButton(this, wxID_ANY, "Done");
	m_save_btn->Bind(wxEVT_BUTTON, &SettingDlg::OnSave, this);
	m_close_btn = new wxButton(this, wxID_ANY, "Cancel");
	m_close_btn->Bind(wxEVT_BUTTON, &SettingDlg::OnClose, this);
	group_b->Add(m_close_btn, 0, wxALIGN_CENTER);
	group_b->AddStretchSpacer(1);
	group_b->Add(m_save_btn, 0, wxALIGN_CENTER);
	group_b->Add(10, 10);

	//interface
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(notebook, 1, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group_b, 0, wxEXPAND);
	SetSizerAndFit(sizerV);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	Thaw();
}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::FluoUpdate(const fluo::ValueCollection& vc)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = vc.empty();

	//project page
	//project save
	if (update_all || FOUND_VALUE(gstSaveProjectEnable))
	{
		m_prj_save_chk->SetValue(glbin_settings.m_prj_save);
		m_prj_save_inc_chk->SetValue(glbin_settings.m_prj_save_inc);
		//realtime compression
		m_realtime_cmp_chk->SetValue(glbin_settings.m_realtime_compress);
		//script break
		m_script_break_chk->SetValue(glbin_settings.m_script_break);
		//inverse slider
		m_inverse_slider_chk->SetValue(glbin_settings.m_inverse_slider);
		//multifunc button
		m_mul_func_btn_comb->Select(glbin_settings.m_mulfunc);
	}

	//font
	if (update_all || FOUND_VALUE(gstFontFile))
	{
		//populate fonts
		std::filesystem::path p = std::filesystem::current_path();
		p /= "Fonts";
		std::vector<std::string> list;
		for (const auto& entry : std::filesystem::directory_iterator(p))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".ttf")
			{
				list.push_back(entry.path().string());
			}
		}
		std::sort(list.begin(), list.end());

		for (size_t i = 0; i < list.size(); ++i)
			m_font_cmb->Append(list[i]);
	}
	if (update_all || FOUND_VALUE(gstSettingsFont))
	{
		std::filesystem::path p(glbin_settings.m_font_file);
		wxString str = p.stem().string();
		int font_sel = m_font_cmb->FindString(str);
		if (font_sel != wxNOT_FOUND)
			m_font_cmb->Select(font_sel);
		m_font_size_cmb->ChangeValue(wxString::Format("%d", glbin_settings.m_text_size));
		m_text_color_cmb->Select(glbin_settings.m_text_color);
	}

	//line width
	if (update_all || FOUND_VALUE(gstLineWidth))
	{
		m_line_width_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_line_width));
		m_line_width_sldr->ChangeValue(std::round(glbin_settings.m_line_width));
	}

	//paint history depth
	if (update_all || FOUND_VALUE(gstPaintHistory))
	{
		m_paint_hist_depth_text->ChangeValue(wxString::Format("%d", glbin_brush_def.m_paint_hist_depth));
		m_paint_hist_depth_sldr->ChangeValue(glbin_brush_def.m_paint_hist_depth);
	}

	//pencil distance
	if (update_all || FOUND_VALUE(gstPencilDist))
	{
		m_pencil_dist_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_pencil_dist));
		m_pencil_dist_sldr->ChangeValue(glbin_settings.m_pencil_dist);
	}

	//micro blending
	if (update_all || FOUND_VALUE(gstMicroBlendEnable))
		m_micro_blend_chk->SetValue(glbin_settings.m_micro_blend);

	//depth peeling
	if (update_all || FOUND_VALUE(gstPeelNum))
	{
		m_peeling_layers_sldr->ChangeValue(glbin_settings.m_peeling_layers);
		m_peeling_layers_text->ChangeValue(wxString::Format("%d", glbin_settings.m_peeling_layers));
	}

	//shadow direction
	if (update_all || FOUND_VALUE(gstShadowDir))
	{
		bool bval = glbin_settings.m_shadow_dir;
		m_shadow_dir_chk->SetValue(bval);
		m_shadow_dir_sldr->Enable(bval);
		m_shadow_dir_text->Enable(bval);
		double deg = GetShadowDir();
		m_shadow_dir_sldr->ChangeValue(std::round(deg));
		m_shadow_dir_text->ChangeValue(wxString::Format("%.2f", deg));
	}

	//rotations
	if (update_all || FOUND_VALUE(gstSettingsRot))
	{
		//rot center anchor thresh
		m_pin_threshold_sldr->ChangeValue(std::round(glbin_settings.m_pin_threshold*10.0));
		m_pin_threshold_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_pin_threshold*100.0));
	}

	//gradient background
	if (update_all || FOUND_VALUE(gstGradBg))
		m_grad_bg_chk->SetValue(glbin_settings.m_grad_bg);

	//performance page
	//mouse interactions
	if (update_all || FOUND_VALUE(gstMouseInt))
		m_mouse_int_chk->SetValue(glbin_settings.m_mouse_int);

	//memory settings
	if (update_all || FOUND_VALUE(gstStreamEnable))
	{
		m_streaming_chk->SetValue(glbin_settings.m_mem_swap);
		EnableStreaming(glbin_settings.m_mem_swap);
		m_update_order_rbox->SetSelection(glbin_settings.m_update_order);
		m_graphics_mem_text->ChangeValue(wxString::Format("%d", (int)glbin_settings.m_graphics_mem));
		m_graphics_mem_sldr->ChangeValue(std::round(glbin_settings.m_graphics_mem / 100.0));
		m_large_data_text->ChangeValue(wxString::Format("%d", (int)glbin_settings.m_large_data_size));
		m_large_data_sldr->ChangeValue(std::round(glbin_settings.m_large_data_size / 10.0));
		m_block_size_text->ChangeValue(wxString::Format("%d", glbin_settings.m_force_brick_size));
		m_block_size_sldr->ChangeValue(std::round(log(glbin_settings.m_force_brick_size) / log(2.0)));
		m_response_time_text->ChangeValue(wxString::Format("%d", glbin_settings.m_up_time));
		m_response_time_sldr->ChangeValue(std::round(glbin_settings.m_up_time / 10.0));
		m_detail_level_offset_text->ChangeValue(wxString::Format("%d", -glbin_settings.m_detail_level_offset));
		m_detail_level_offset_sldr->ChangeValue(-glbin_settings.m_detail_level_offset);
	}

	//display page
	//stereo
	if (update_all || FOUND_VALUE(gstHologramMode))
	{
		if (glbin_settings.m_hologram_mode == 0)
		{
#ifdef _WIN32
			m_holo_ip_text->Disable();
#endif
			m_stereo_chk->SetValue(false);
			m_xr_api_cmb->Disable();
			m_mv_hmd_chk->Disable();
			m_sbs_chk->Disable();
			m_eye_dist_sldr->Disable();
			m_eye_dist_text->Disable();
			m_looking_glass_chk->SetValue(false);
			m_lg_offset_sldr->Disable();
			m_lg_offset_text->Disable();
			m_holo_debug_chk->Disable();
		}
		else if (glbin_settings.m_hologram_mode == 1)
		{
#ifdef _WIN32
			if (glbin_settings.m_xr_api == 4)
				m_holo_ip_text->Enable();
			else
				m_holo_ip_text->Disable();
#endif
			m_stereo_chk->SetValue(true);
			m_xr_api_cmb->Enable();
			m_mv_hmd_chk->Enable();
			m_sbs_chk->Enable();
			m_eye_dist_sldr->Enable();
			m_eye_dist_text->Enable();
			m_looking_glass_chk->SetValue(false);
			m_lg_offset_sldr->Disable();
			m_lg_offset_text->Disable();
			m_holo_debug_chk->Disable();
		}
		else if (glbin_settings.m_hologram_mode == 2)
		{
#ifdef _WIN32
			m_holo_ip_text->Disable();
#endif
			m_stereo_chk->SetValue(false);
			m_xr_api_cmb->Disable();
			m_mv_hmd_chk->Disable();
			m_sbs_chk->Disable();
			m_eye_dist_sldr->Disable();
			m_eye_dist_text->Disable();
			m_looking_glass_chk->SetValue(true);
			m_lg_offset_sldr->Enable();
			m_lg_offset_text->Enable();
			m_holo_debug_chk->Enable();
		}
#ifdef _WIN32
		m_holo_ip_text->ChangeValue(wxString(glbin_settings.m_holo_ip));
#endif
		m_mv_hmd_chk->SetValue(glbin_settings.m_mv_hmd);
		m_xr_api_cmb->Select(glbin_settings.m_xr_api);
		m_sbs_chk->SetValue(glbin_settings.m_sbs);
		m_eye_dist_sldr->ChangeValue(std::round(glbin_settings.m_eye_dist * 10.0));
		m_eye_dist_text->ChangeValue(wxString::Format("%.1f", glbin_settings.m_eye_dist));
		m_lg_offset_sldr->ChangeValue(glbin_settings.m_lg_offset);
		m_lg_offset_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_lg_offset));
		m_holo_debug_chk->SetValue(glbin_settings.m_hologram_debug);
	}

	//display id
	if (update_all || FOUND_VALUE(gstFullscreenDisplay))
		m_disp_id_comb->Select(glbin_settings.m_disp_id);

	//color depth
	if (update_all || FOUND_VALUE(gstDisplayColorDepth))
		m_color_depth_comb->Select(glbin_settings.m_color_depth);

	//format page
	//override vox
	if (update_all || FOUND_VALUE(gstOverrideVoxSpc))
		m_override_vox_chk->SetValue(glbin_settings.m_override_vox);

	//wavelength to color
	if (update_all || FOUND_VALUE(gstWavelengthColors))
	{
		m_wav_color1_cmb->Select(glbin_settings.m_wav_color1 - 1);
		m_wav_color2_cmb->Select(glbin_settings.m_wav_color2 - 1);
		m_wav_color3_cmb->Select(glbin_settings.m_wav_color3 - 1);
		m_wav_color4_cmb->Select(glbin_settings.m_wav_color4 - 1);
	}

	//max texture size
	if (update_all || FOUND_VALUE(gstMaxTextureSize))
	{
		bool bval = glbin_settings.m_use_max_texture_size;
		m_max_texture_size_chk->SetValue(bval);
		m_max_texture_size_text->Enable(bval);
		wxString str;
		if (bval)
			str = wxString::Format("%d", glbin_settings.m_max_texture_size);
		else
			str = wxString::Format("%d", flvr::ShaderProgram::max_texture_size());
		m_max_texture_size_text->ChangeValue(str);
	}

	if (update_all || FOUND_VALUE(gstDeviceTree))
	{
		m_device_tree->DeleteAllItems();
		//cl device tree
		std::vector<flvr::CLPlatform>* devices = flvr::KernelProgram::GetDeviceList();
		int pid = flvr::KernelProgram::get_platform_id();
		int did = flvr::KernelProgram::get_device_id();
		wxTreeItemId root = m_device_tree->AddRoot("Computer");
		std::string name;
		if (devices)
		{
			for (int i = 0; i < devices->size(); ++i)
			{
				flvr::CLPlatform* platform = &((*devices)[i]);
				name = platform->vendor;
				name.back() = ';';
				name += " " + platform->name;
				wxTreeItemId pfitem = m_device_tree->AppendItem(root, name);
				for (int j = 0; j < platform->devices.size(); ++j)
				{
					flvr::CLDevice* device = &(platform->devices[j]);
					name = device->vendor;
					name.back() = ';';
					name += " " + device->name;
					wxTreeItemId dvitem = m_device_tree->AppendItem(pfitem, name);
					if (i == pid && j == did)
						m_device_tree->SelectItem(dvitem);
				}
			}
		}
		m_device_tree->ExpandAll();
		m_device_tree->SetFocus();
	}

	//java
	if (update_all || FOUND_VALUE(gstSettingsJava))
	{
		m_java_jvm_text->ChangeValue(glbin_settings.m_jvm_path);
		m_java_ij_text->ChangeValue(glbin_settings.m_ij_path);
		m_java_bioformats_text->ChangeValue(glbin_settings.m_bioformats_path);
		switch (glbin_settings.m_ij_mode)
		{
		case 0:
			mp_radio_button_imagej->SetValue(true);
			m_java_jvm_text->Enable(true);
			m_java_bioformats_text->Enable(true);
			m_browse_jvm_btn->Enable(true);
			m_browse_bioformats_btn->Enable(true);
			break;
		case 1:
			mp_radio_button_fiji->SetValue(true);
			m_java_jvm_text->Enable(false);
			m_java_bioformats_text->Enable(false);
			m_browse_jvm_btn->Enable(false);
			m_browse_bioformats_btn->Enable(false);
			break;
		}
	}
}

//events
void SettingDlg::OnSave(wxCommandEvent& event)
{
	glbin_settings.Save();
	if (m_frame)
		m_frame->ShowPanel(this, false);
}

void SettingDlg::OnClose(wxCommandEvent& event)
{
	if (m_frame)
		m_frame->ShowPanel(this, false);
}

void SettingDlg::OnProjectSaveCheck(wxCommandEvent& event)
{
	glbin_settings.m_prj_save = m_prj_save_chk->GetValue();
}

void SettingDlg::OnProjectSaveIncCheck(wxCommandEvent& event)
{
	glbin_settings.m_prj_save_inc = m_prj_save_inc_chk->GetValue();
}

void SettingDlg::OnRealtimeCompressCheck(wxCommandEvent& event)
{
	glbin_settings.m_realtime_compress = m_realtime_cmp_chk->GetValue();
}

void SettingDlg::OnScriptBreakCheck(wxCommandEvent& event)
{
	glbin_settings.m_script_break = m_script_break_chk->GetValue();
}

void SettingDlg::OnInverseSliderCheck(wxCommandEvent& event)
{
	glbin_settings.m_inverse_slider = m_inverse_slider_chk->GetValue();
}

void SettingDlg::OnMulFuncBtnComb(wxCommandEvent& event)
{
	glbin_settings.m_mulfunc = m_mul_func_btn_comb->GetCurrentSelection();
	FluoRefresh(3, { gstMultiFuncTips }, { -1 });
}

void SettingDlg::OnMouseIntCheck(wxCommandEvent& event)
{
	glbin_settings.m_mouse_int = m_mouse_int_chk->GetValue();
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnPeelingLayersChange(wxScrollEvent& event)
{
	int ival = m_peeling_layers_sldr->GetValue();;
	wxString str = wxString::Format("%d", ival);
	if (str != m_peeling_layers_text->GetValue())
		m_peeling_layers_text->SetValue(str);
}

void SettingDlg::OnPeelingLayersEdit(wxCommandEvent& event)
{
	wxString str = m_peeling_layers_text->GetValue();
	long ival;
	str.ToLong(&ival);
	if (ival <= 0)
		return;
	m_peeling_layers_sldr->ChangeValue(ival);
	glbin_settings.m_peeling_layers = ival;
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnMicroBlendCheck(wxCommandEvent& event)
{
	glbin_settings.m_micro_blend = m_micro_blend_chk->GetValue();
	FluoRefresh(3, { gstNull });
}

//shadow direction
void SettingDlg::OnShadowDirCheck(wxCommandEvent& event)
{
	if (m_shadow_dir_chk->GetValue())
	{
		m_shadow_dir_sldr->Enable();
		m_shadow_dir_text->Enable();

		wxString str;
		str = m_shadow_dir_text->GetValue();
		double deg;
		str.ToDouble(&deg);
		SetShadowDir(deg);
		glbin_settings.m_shadow_dir = true;
	}
	else
	{
		m_shadow_dir_sldr->Disable();
		m_shadow_dir_text->Disable();
		glbin_settings.m_shadow_dir_x = 0.0;
		glbin_settings.m_shadow_dir_y = 0.0;
		glbin_settings.m_shadow_dir = false;
	}
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnShadowDirChange(wxScrollEvent& event)
{
	double deg = m_shadow_dir_sldr->GetValue();
	wxString str = wxString::Format("%.2f", deg);
	if (str != m_shadow_dir_text->GetValue())
		m_shadow_dir_text->SetValue(str);
}

void SettingDlg::OnShadowDirEdit(wxCommandEvent& event)
{
	wxString str = m_shadow_dir_text->GetValue();
	double deg;
	str.ToDouble(&deg);
	m_shadow_dir_sldr->ChangeValue(std::round(deg));
	SetShadowDir(deg);
	FluoRefresh(3, { gstNull });
}

void SettingDlg::EnableStreaming(bool enable)
{
	if (enable)
	{
		m_update_order_rbox->Enable(0, true);
		m_update_order_rbox->Enable(1, true);
		m_graphics_mem_sldr->Enable();
		m_graphics_mem_text->Enable();
		m_large_data_sldr->Enable();
		m_large_data_text->Enable();
		m_block_size_sldr->Enable();
		m_block_size_text->Enable();
		m_response_time_sldr->Enable();
		m_response_time_text->Enable();
		m_detail_level_offset_sldr->Enable();
		m_detail_level_offset_text->Enable();
	}
	else
	{
		m_update_order_rbox->Enable(0, false);
		m_update_order_rbox->Enable(1, false);
		m_graphics_mem_sldr->Disable();
		m_graphics_mem_text->Disable();
		m_large_data_sldr->Disable();
		m_large_data_text->Disable();
		m_block_size_sldr->Disable();
		m_block_size_text->Disable();
		m_response_time_sldr->Disable();
		m_response_time_text->Disable();
		m_detail_level_offset_sldr->Disable();
		m_detail_level_offset_text->Disable();
	}
}

void SettingDlg::SetShadowDir(double deg)
{
	glbin_settings.m_shadow_dir_x = cos(d2r(deg));
	glbin_settings.m_shadow_dir_y = sin(d2r(deg));
}

double SettingDlg::GetShadowDir()
{
	double deg = r2d(atan2(glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x));
	return deg;
}

//gradient background
void SettingDlg::OnGradBgCheck(wxCommandEvent& event)
{
	glbin_settings.m_grad_bg = m_grad_bg_chk->GetValue();
	FluoRefresh(3, { gstNull });
}

//rot center anchor thresh
void SettingDlg::OnPinThresholdChange(wxScrollEvent& event)
{
	double dval = m_pin_threshold_sldr->GetValue();
	wxString str = wxString::Format("%.0f", dval*10.0);
	if (str != m_pin_threshold_text->GetValue())
		m_pin_threshold_text->SetValue(str);
}

void SettingDlg::OnPinThresholdEdit(wxCommandEvent& event)
{
	wxString str = m_pin_threshold_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_pin_threshold_sldr->ChangeValue(std::round(dval/10.0));
	glbin_settings.m_pin_threshold = dval / 100.0;
}

//link rotations
void SettingDlg::OnRotLink(wxCommandEvent& event)
{
	bool linked_rot = m_rot_link_chk->GetValue();
	RenderCanvas::m_linked_rot = linked_rot;
	RenderCanvas::m_master_linked_view = 0;
	FluoRefresh(3, { gstNull });
}

//stereo
void SettingDlg::OnStereoCheck(wxCommandEvent& event)
{
	bool bval = m_stereo_chk->GetValue();
	if (bval)
	{
		m_looking_glass_chk->SetValue(false);
		glbin_settings.m_hologram_mode = 1;
	}
	else
		glbin_settings.m_hologram_mode = 0;
	FluoRefresh(2, { gstHologramMode });
}

void SettingDlg::OnXrApiComb(wxCommandEvent& event)
{
	glbin_settings.m_xr_api = m_xr_api_cmb->GetCurrentSelection();
	if (glbin_settings.m_xr_api == 4)
		glbin_settings.m_eye_dist = 0;
	else
		glbin_settings.m_eye_dist = 20;
	FluoRefresh(2, { gstHologramMode });
}

void SettingDlg::OnMvHmdCheck(wxCommandEvent& event)
{
	glbin_settings.m_mv_hmd = m_mv_hmd_chk->GetValue();
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnSBSCheck(wxCommandEvent& event)
{
	glbin_settings.m_sbs = m_sbs_chk->GetValue();
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnEyeDistChange(wxScrollEvent& event)
{
	glbin_settings.m_eye_dist = m_eye_dist_sldr->GetValue() / 10.0;
	wxString str = wxString::Format("%.1f", glbin_settings.m_eye_dist);
	if (str != m_eye_dist_text->GetValue())
		m_eye_dist_text->SetValue(str);
}

void SettingDlg::OnEyeDistEdit(wxCommandEvent& event)
{
	wxString str = m_eye_dist_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_eye_dist_sldr->ChangeValue(std::round(dval * 10.0));
	glbin_settings.m_eye_dist = dval;
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnHoloIpEdit(wxCommandEvent& event)
{
	wxString str = m_holo_ip_text->GetValue();
	wxRegEx regex("^(([0-9]{1,3})\\.){3}([0-9]{1,3})$");
	if (regex.Matches(str))
	{
		glbin_settings.m_holo_ip = str;
	}
}

void SettingDlg::OnLookingGlassCheck(wxCommandEvent& event)
{
	bool bval = m_looking_glass_chk->GetValue();
	if (bval)
	{
		m_stereo_chk->SetValue(false);
		glbin_settings.m_hologram_mode = 2;
	}
	else
		glbin_settings.m_hologram_mode = 0;
	FluoRefresh(2, { gstHologramMode });
}

void SettingDlg::OnLgOffsetChange(wxScrollEvent& event)
{
	glbin_settings.m_lg_offset = m_lg_offset_sldr->GetValue();
	wxString str = wxString::Format("%.0f", glbin_settings.m_lg_offset);
	if (str != m_lg_offset_text->GetValue())
		m_lg_offset_text->SetValue(str);
}

void SettingDlg::OnLgOffsetEdit(wxCommandEvent& event)
{
	wxString str = m_lg_offset_text->GetValue();
	long lval;
	str.ToLong(&lval);
	m_lg_offset_sldr->ChangeValue(lval);
	glbin_settings.m_lg_offset = lval;
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnHoloDebugCheck(wxCommandEvent& event)
{
	glbin_settings.m_hologram_debug = m_holo_debug_chk->GetValue();
	FluoRefresh(3, { gstNull });
}

//display id
void SettingDlg::OnDispIdComb(wxCommandEvent& event)
{
	glbin_settings.m_disp_id = m_disp_id_comb->GetCurrentSelection();
}

//color depth
void SettingDlg::OnColorDepthComb(wxCommandEvent& event)
{
	int val = m_color_depth_comb->GetCurrentSelection();
	switch (val)
	{
	case 0://8
		glbin_settings.m_color_depth = val;
		glbin_settings.m_red_bit = 8;
		glbin_settings.m_green_bit = 8;
		glbin_settings.m_blue_bit = 8;
		glbin_settings.m_alpha_bit = 8;
		break;
	case 1://10
		glbin_settings.m_color_depth = val;
		glbin_settings.m_red_bit = 10;
		glbin_settings.m_green_bit = 10;
		glbin_settings.m_blue_bit = 10;
		glbin_settings.m_alpha_bit = 2;
		break;
	case 2://16
		glbin_settings.m_color_depth = val;
		glbin_settings.m_red_bit = 16;
		glbin_settings.m_green_bit = 16;
		glbin_settings.m_blue_bit = 16;
		glbin_settings.m_alpha_bit = 16;
		break;
	}
}

//override vox
void SettingDlg::OnOverrideVoxCheck(wxCommandEvent& event)
{
	glbin_settings.m_override_vox = m_override_vox_chk->GetValue();
}

void SettingDlg::OnWavColor1Change(wxCommandEvent& event)
{
	if (m_wav_color1_cmb)
		glbin_settings.m_wav_color1 = m_wav_color1_cmb->GetCurrentSelection() + 1;
}

void SettingDlg::OnWavColor2Change(wxCommandEvent& event)
{
	if (m_wav_color2_cmb)
		glbin_settings.m_wav_color2 = m_wav_color2_cmb->GetCurrentSelection() + 1;
}

void SettingDlg::OnWavColor3Change(wxCommandEvent& event)
{
	if (m_wav_color3_cmb)
		glbin_settings.m_wav_color3 = m_wav_color3_cmb->GetCurrentSelection() + 1;
}

void SettingDlg::OnWavColor4Change(wxCommandEvent& event)
{
	if (m_wav_color4_cmb)
		glbin_settings.m_wav_color4 = m_wav_color4_cmb->GetCurrentSelection() + 1;
}

//texture size
void SettingDlg::OnMaxTextureSizeChk(wxCommandEvent& event)
{
	glbin_settings.m_use_max_texture_size = m_max_texture_size_chk->GetValue();
	if (glbin_settings.m_use_max_texture_size)
	{
		flvr::ShaderProgram::set_max_texture_size(glbin_settings.m_max_texture_size);
		m_max_texture_size_text->ChangeValue(
			wxString::Format("%d", glbin_settings.m_max_texture_size));
		m_max_texture_size_text->Enable();
	}
	else
	{
		flvr::ShaderProgram::reset_max_texture_size();
		m_max_texture_size_text->Disable();
	}
}

void SettingDlg::OnMaxTextureSizeEdit(wxCommandEvent& event)
{
	if (glbin_settings.m_use_max_texture_size)
	{
		wxString str = m_max_texture_size_text->GetValue();
		long size;
		if (str.ToLong(&size))
		{
			glbin_settings.m_max_texture_size = size;
			flvr::ShaderProgram::set_max_texture_size(size);
		}
	}
}

//memory settings
void SettingDlg::OnStreamingChk(wxCommandEvent& event)
{
	if (m_streaming_chk->GetValue())
		glbin_settings.m_mem_swap = true;
	else
		glbin_settings.m_mem_swap = false;
	EnableStreaming(glbin_settings.m_mem_swap);
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnUpdateOrderChange(wxCommandEvent& event)
{
	glbin_settings.m_update_order = m_update_order_rbox->GetSelection();
}

void SettingDlg::OnGraphicsMemChange(wxScrollEvent& event)
{
	int ival = m_graphics_mem_sldr->GetValue();
	wxString str = wxString::Format("%d", ival * 100);
	if (str != m_graphics_mem_text->GetValue())
		m_graphics_mem_text->SetValue(str);
}

void SettingDlg::OnGraphicsMemEdit(wxCommandEvent& event)
{
	wxString str = m_graphics_mem_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val <= 0.0)
		return;
	m_graphics_mem_sldr->ChangeValue(std::round(val / 100.0));
	glbin_settings.m_graphics_mem = val;
}

void SettingDlg::OnLargeDataChange(wxScrollEvent& event)
{
	int ival = m_large_data_sldr->GetValue();
	wxString str = wxString::Format("%d", ival * 10);
	if (str != m_large_data_text->GetValue())
		m_large_data_text->SetValue(str);
}

void SettingDlg::OnLargeDataEdit(wxCommandEvent& event)
{
	wxString str = m_large_data_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val < 0.0)
		return;
	m_large_data_sldr->ChangeValue(std::round(val / 10.0));
	glbin_settings.m_large_data_size = val;
}

void SettingDlg::OnBlockSizeChange(wxScrollEvent& event)
{
	int ival = m_block_size_sldr->GetValue();
	wxString str = wxString::Format("%d", 2 << (ival - 1));
	if (str != m_block_size_text->GetValue())
		m_block_size_text->SetValue(str);
}

void SettingDlg::OnBlockSizeEdit(wxCommandEvent& event)
{
	wxString str = m_block_size_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val <= 0.0)
		return;
	m_block_size_sldr->ChangeValue(std::round(log(val) / log(2.0)));
	glbin_settings.m_force_brick_size = val;
}

void SettingDlg::OnResponseTimeChange(wxScrollEvent& event)
{
	int ival = m_response_time_sldr->GetValue();
	wxString str = wxString::Format("%d", ival * 10);
	if (str != m_response_time_text->GetValue())
		m_response_time_text->SetValue(str);
}

void SettingDlg::OnResponseTimeEdit(wxCommandEvent& event)
{
	wxString str = m_response_time_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val <= 0.0)
		return;
	m_response_time_sldr->ChangeValue(std::round(val / 10.0));
	glbin_settings.m_up_time = val;
}

void SettingDlg::OnDetailLevelOffsetChange(wxScrollEvent& event)
{
	int ival = m_detail_level_offset_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_detail_level_offset_text->GetValue())
		m_detail_level_offset_text->SetValue(str);
}

void SettingDlg::OnDetailLevelOffsetEdit(wxCommandEvent& event)
{
	wxString str = m_detail_level_offset_text->GetValue();
	long val;
	str.ToLong(&val);
	m_detail_level_offset_sldr->ChangeValue(val);
	glbin_settings.m_detail_level_offset = -val;
	FluoRefresh(3, { gstNull });
}

//font
void SettingDlg::OnFontChange(wxCommandEvent& event)
{
	std::string str = m_font_cmb->GetValue().ToStdString();
	if (str.empty())
		return;

	glbin_settings.m_font_file = str + ".ttf";
	std::filesystem::path p = std::filesystem::current_path();
	p = p / "Fonts" / (str + ".ttf");
	glbin_text_tex_manager.load_face(p.string());
	glbin_text_tex_manager.SetSize(glbin_settings.m_text_size);
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnFontSizeChange(wxCommandEvent& event)
{
	wxString str = m_font_size_cmb->GetValue();
	long size;
	if (!str.ToLong(&size))
		return;

	glbin_settings.m_text_size = size;
	glbin_text_tex_manager.SetSize(glbin_settings.m_text_size);
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnTextColorChange(wxCommandEvent& event)
{
	glbin_settings.m_text_color = m_text_color_cmb->GetCurrentSelection();
	FluoRefresh(3, { gstNull });
}

//line width
void SettingDlg::OnLineWidthSldr(wxScrollEvent& event)
{
	int ival = m_line_width_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_line_width_text->GetValue())
		m_line_width_text->SetValue(str);
}

void SettingDlg::OnLineWidthText(wxCommandEvent& event)
{
	wxString str = m_line_width_text->GetValue();
	unsigned long ival;
	if (!str.ToULong(&ival))
		return;

	m_line_width_sldr->ChangeValue(ival);
	glbin_settings.m_line_width = ival;
	FluoRefresh(3, { gstNull });
}

//paint history depth
void SettingDlg::OnPaintHistDepthChange(wxScrollEvent& event)
{
	int ival = m_paint_hist_depth_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_paint_hist_depth_text->GetValue())
		m_paint_hist_depth_text->SetValue(str);
}

void SettingDlg::OnPaintHistDepthEdit(wxCommandEvent& event)
{
	wxString str = m_paint_hist_depth_text->GetValue();
	unsigned long ival;
	if (!str.ToULong(&ival))
		return;
	m_paint_hist_depth_sldr->ChangeValue(ival);
	glbin_brush_def.m_paint_hist_depth = ival;
	flvr::Texture::mask_undo_num_ = (size_t)(ival);
}

//pencil distance
void SettingDlg::OnPencilDistChange(wxScrollEvent& event)
{
	int ival = m_pencil_dist_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_pencil_dist_text->GetValue())
		m_pencil_dist_text->SetValue(str);
}

void SettingDlg::OnPencilDistEdit(wxCommandEvent& event)
{
	wxString str = m_pencil_dist_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	m_pencil_dist_sldr->ChangeValue(ival);
	glbin_settings.m_pencil_dist = ival;
}

// Java settings.
void SettingDlg::OnJavaJvmEdit(wxCommandEvent& event)
{
	glbin_settings.m_jvm_path = m_java_jvm_text->GetValue();
}

void SettingDlg::OnJavaIJEdit(wxCommandEvent& event)
{
	glbin_settings.m_ij_path = m_java_ij_text->GetValue();
}

void SettingDlg::OnJavaBioformatsEdit(wxCommandEvent& event)
{
	glbin_settings.m_bioformats_path = m_java_bioformats_text->GetValue();
}

void SettingDlg::onJavaJvmBrowse(wxCommandEvent& event)
{
#ifdef _WIN32
	ModalDlg *fopendlg = new ModalDlg(
		m_frame, "Choose the jvm dll file",
		"", "", "*.dll", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#else
	ModalDlg* fopendlg = new ModalDlg(
		m_frame, "Choose the libjvm.dylib file",
		"", "", "*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_java_jvm_text->ChangeValue(filename);
		glbin_settings.m_jvm_path = filename;
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaIJBrowse(wxCommandEvent& event)
{
#ifdef _WIN32	
	//ModalDlg *fopendlg = new ModalDlg(
	//	m_frame, "Choose the imageJ/fiji directory",
	//	"", "", "*.jar", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	wxDirDialog *fopendlg = new wxDirDialog(
		m_frame, "Choose the imageJ/fiji directory",
		"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
#else
	ModalDlg* fopendlg = new ModalDlg(
		m_frame, "Choose the imageJ/fiji app",
		"", "", "*.app", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
#ifdef _DARWIN
		//filename = filename + "/Contents/Java/ij.jar";
#endif
		m_java_ij_text->ChangeValue(filename);
		glbin_settings.m_ij_path = filename;
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaBioformatsBrowse(wxCommandEvent& event)
{
	ModalDlg *fopendlg = new ModalDlg(
		m_frame, "Choose the bioformats jar",
		"", "", "*.jar", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_java_bioformats_text->ChangeValue(filename);
		glbin_settings.m_bioformats_path = filename;
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaRadioButtonImageJ(wxCommandEvent& event)
{
	m_java_jvm_text->Enable(true);
	m_java_bioformats_text->Enable(true);
	m_browse_jvm_btn->Enable(true);
	m_browse_bioformats_btn->Enable(true);
	glbin_settings.m_ij_mode = 0;
}

void SettingDlg::onJavaRadioButtonFiji(wxCommandEvent& event)
{
	m_java_jvm_text->Enable(false);	
	m_java_bioformats_text->Enable(false);
	m_browse_jvm_btn->Enable(false);
	m_browse_bioformats_btn->Enable(false);
	glbin_settings.m_ij_mode = 1;
}

//device tree
void SettingDlg::OnSelChanged(wxTreeEvent& event)
{
	wxTreeItemId sel = m_device_tree->GetSelection();
	if (!sel.IsOk())
		return;
	int i = 0, j = 0;
	wxTreeItemIdValue pfck;
	wxTreeItemId root = m_device_tree->GetRootItem();
	if (root.IsOk())
	{
		wxTreeItemId pfitem = m_device_tree->GetFirstChild(root, pfck);
		while (pfitem.IsOk())
		{
			j = 0;
			wxTreeItemIdValue dvck;
			wxTreeItemId dvitem = m_device_tree->GetFirstChild(pfitem, dvck);
			while (dvitem.IsOk())
			{
				if (dvitem == sel)
				{
					glbin_settings.m_cl_platform_id = i;
					glbin_settings.m_cl_device_id = j;
				}
				dvitem = m_device_tree->GetNextChild(pfitem, dvck);
				j++;
			}
			pfitem = m_device_tree->GetNextChild(root, pfck);
			i++;
		}
	}
}