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
#include <SettingDlg.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>
#include <wxSingleSlider.h>
#include <Global/Global.h>
#include <wx/valnum.h>
#include <wx/notebook.h>
#include <wx/stdpaths.h>
#include <wx/display.h>

BEGIN_EVENT_TABLE(SettingDlg, wxPanel)
EVT_BUTTON(ID_SaveBtn, SettingDlg::OnSave)
EVT_BUTTON(ID_CloseBtn, SettingDlg::OnClose)
//project save
EVT_CHECKBOX(ID_PrjSaveChk, SettingDlg::OnProjectSaveCheck)
EVT_CHECKBOX(ID_PrjSaveIncChk, SettingDlg::OnProjectSaveIncCheck)
//real time compress
EVT_CHECKBOX(ID_RealtimeCmpChk, SettingDlg::OnRealtimeCompressCheck)
//script break
EVT_CHECKBOX(ID_ScriptBreakChk, SettingDlg::OnScriptBreakCheck)
//inverse sliders
EVT_CHECKBOX(ID_InverseSliderChk, SettingDlg::OnInverseSliderCheck)
//multi function button
EVT_COMBOBOX(ID_MulFuncBtnComb, SettingDlg::OnMulFuncBtnComb)
//mouse interactions
EVT_CHECKBOX(ID_MouseIntChk, SettingDlg::OnMouseIntCheck)
//depth peeling
EVT_COMMAND_SCROLL(ID_PeelingLayersSldr, SettingDlg::OnPeelingLayersChange)
EVT_TEXT(ID_PeelingLayersText, SettingDlg::OnPeelingLayersEdit)
//micro blend
EVT_CHECKBOX(ID_MicroBlendChk, SettingDlg::OnMicroBlendCheck)
//shadow direction
EVT_CHECKBOX(ID_ShadowDirChk, SettingDlg::OnShadowDirCheck)
EVT_COMMAND_SCROLL(ID_ShadowDirSldr, SettingDlg::OnShadowDirChange)
EVT_TEXT(ID_ShadowDirText, SettingDlg::OnShadowDirEdit)
//gradient background
EVT_CHECKBOX(ID_GradBgChk, SettingDlg::OnGradBgCheck)
//rot center anchor threshold
EVT_COMMAND_SCROLL(ID_PinThreshSldr, SettingDlg::OnPinThresholdChange)
EVT_TEXT(ID_PinThreshText, SettingDlg::OnPinThresholdEdit)
//link render views rotations
EVT_CHECKBOX(ID_RotLinkChk, SettingDlg::OnRotLink)
//stereo
EVT_CHECKBOX(ID_StereoChk, SettingDlg::OnStereoCheck)
EVT_CHECKBOX(ID_SBSChk, SettingDlg::OnSBSCheck)
EVT_COMMAND_SCROLL(ID_EyeDistSldr, SettingDlg::OnEyeDistChange)
EVT_TEXT(ID_EyeDistText, SettingDlg::OnEyeDistEdit)
//display id
EVT_COMBOBOX(ID_DispIdCombo, SettingDlg::OnDispIdComb)
//color depth
EVT_COMBOBOX(ID_ColorDepthCombo, SettingDlg::OnColorDepthComb)
//override vox
EVT_CHECKBOX(ID_OverrideVoxChk, SettingDlg::OnOverrideVoxCheck)
//wavelength to color
EVT_COMBOBOX(ID_WavColor1Cmb, SettingDlg::OnWavColor1Change)
EVT_COMBOBOX(ID_WavColor2Cmb, SettingDlg::OnWavColor2Change)
EVT_COMBOBOX(ID_WavColor3Cmb, SettingDlg::OnWavColor3Change)
EVT_COMBOBOX(ID_WavColor4Cmb, SettingDlg::OnWavColor4Change)
//texture size
EVT_CHECKBOX(ID_MaxTextureSizeChk, SettingDlg::OnMaxTextureSizeChk)
EVT_TEXT(ID_MaxTextureSizeText, SettingDlg::OnMaxTextureSizeEdit)
//memory settings
EVT_CHECKBOX(ID_StreamingChk, SettingDlg::OnStreamingChk)
EVT_RADIOBOX(ID_UpdateOrderRbox, SettingDlg::OnUpdateOrderChange)
EVT_COMMAND_SCROLL(ID_GraphicsMemSldr, SettingDlg::OnGraphicsMemChange)
EVT_TEXT(ID_GraphicsMemText, SettingDlg::OnGraphicsMemEdit)
EVT_COMMAND_SCROLL(ID_LargeDataSldr, SettingDlg::OnLargeDataChange)
EVT_TEXT(ID_LargeDataText, SettingDlg::OnLargeDataEdit)
EVT_COMMAND_SCROLL(ID_BlockSizeSldr, SettingDlg::OnBlockSizeChange)
EVT_TEXT(ID_BlockSizeText, SettingDlg::OnBlockSizeEdit)
EVT_COMMAND_SCROLL(ID_ResponseTimeSldr, SettingDlg::OnResponseTimeChange)
EVT_TEXT(ID_ResponseTimeText, SettingDlg::OnResponseTimeEdit)
EVT_COMMAND_SCROLL(ID_DetailLevelOffsetSldr, SettingDlg::OnDetailLevelOffsetChange)
EVT_TEXT(ID_DetailLevelOffsetText, SettingDlg::OnDetailLevelOffsetEdit)
//font
EVT_COMBOBOX(ID_FontCmb, SettingDlg::OnFontChange)
EVT_COMBOBOX(ID_FontSizeCmb, SettingDlg::OnFontSizeChange)
EVT_TEXT(ID_FontSizeCmb, SettingDlg::OnFontSizeChange)
EVT_COMBOBOX(ID_TextColorCmb, SettingDlg::OnTextColorChange)
//line width
EVT_COMMAND_SCROLL(ID_LineWidthSldr, SettingDlg::OnLineWidthSldr)
EVT_TEXT(ID_LineWidthText, SettingDlg::OnLineWidthText)
//paint history depth
EVT_COMMAND_SCROLL(ID_PaintHistDepthSldr, SettingDlg::OnPaintHistDepthChange)
EVT_TEXT(ID_PaintHistDepthText, SettingDlg::OnPaintHistDepthEdit)
//pencil distance
EVT_COMMAND_SCROLL(ID_PencilDistSldr, SettingDlg::OnPencilDistChange)
EVT_TEXT(ID_PencilDistText, SettingDlg::OnPencilDistEdit)
//Java settings
EVT_TEXT(ID_JavaJVMText, SettingDlg::OnJavaJvmEdit)
EVT_TEXT(ID_JavaIJText, SettingDlg::OnJavaIJEdit)
EVT_TEXT(ID_JavaBioformatsText, SettingDlg::OnJavaBioformatsEdit)
EVT_BUTTON(ID_JavaJvmBrowseBtn, SettingDlg::onJavaJvmBrowse)
EVT_BUTTON(ID_JavaIJBrowseBtn, SettingDlg::onJavaIJBrowse)
EVT_BUTTON(ID_JavaBioformatsBrowseBtn, SettingDlg::onJavaBioformatsBrowse)
EVT_RADIOBUTTON(ID_RadioButtonImageJ, SettingDlg::onJavaRadioButtonImageJ)
EVT_RADIOBUTTON(ID_RadioButtonFiji, SettingDlg::onJavaRadioButtonFiji)
//device tree
EVT_TREE_SEL_CHANGED(ID_DeviceTree, SettingDlg::OnSelChanged)
//show
EVT_SHOW(SettingDlg::OnShow)
END_EVENT_TABLE()

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
	m_prj_save_chk = new wxCheckBox(page, ID_PrjSaveChk,
		"Save project when capture viewport or export movie.");
	m_prj_save_inc_chk = new wxCheckBox(page, ID_PrjSaveIncChk,
		"Save project in new files with incremental serial numbers.");
	m_realtime_cmp_chk = new wxCheckBox(page, ID_RealtimeCmpChk,
		"Compress data in graphics memory when loading.");
	m_script_break_chk = new wxCheckBox(page, ID_ScriptBreakChk,
		"Allow script information prompts.");
	m_inverse_slider_chk = new wxCheckBox(page, ID_InverseSliderChk,
		"Invert vertical slider orientation.");
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Set multifunction buttons to:");
	m_mul_func_btn_comb = new wxComboBox(page, ID_MulFuncBtnComb, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> items = {"Sync Channels", "Focused Scroll", "Use Default", "Use ML", "Undo", "Enable/Disable"};
	m_mul_func_btn_comb->Append(items);
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
	m_font_cmb = new wxComboBox(page, ID_FontCmb, "",
		wxDefaultPosition, FromDIP(wxSize(150, -1)), 0, NULL, wxCB_READONLY);
	//populate fonts
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	wxString loc = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + "*.ttf";
	wxLogNull logNo;
	wxArrayString list;
	wxString file = wxFindFirstFile(loc);
	while (!file.empty())
	{
		file = wxFileNameFromPath(file);
		file = file.BeforeLast('.');
		list.Add(file);
		file = wxFindNextFile();
	}
	list.Sort();
	for (size_t i = 0; i < list.GetCount(); ++i)
		m_font_cmb->Append(list[i]);
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_font_cmb);
	sizer2_1->Add(10, 10);
	st = new wxStaticText(page, 0, "Size:");
	m_font_size_cmb = new wxComboBox(page, ID_FontSizeCmb, "",
		wxDefaultPosition, FromDIP(wxSize(50, -1)), 0, NULL);
	for (int df = 3; df < 18; ++df)
		m_font_size_cmb->Append(wxString::Format("%d", int(std::round(std::pow(df, 1.5)))));
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_font_size_cmb);
	sizer2_1->Add(10, 10);
	st = new wxStaticText(page, 0, "Color:");
	m_text_color_cmb = new wxComboBox(page, ID_TextColorCmb, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	m_text_color_cmb->Append("BG inverted");
	m_text_color_cmb->Append("Background");
	m_text_color_cmb->Append("Vol sec color");
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
	m_line_width_sldr = new wxSingleSlider(page, ID_LineWidthSldr, 3, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_line_width_text = new wxTextCtrl(page, ID_LineWidthText, "3",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	sizer3_1->Add(m_line_width_sldr, 1, wxEXPAND);
	sizer3_1->Add(m_line_width_text, 0, wxALIGN_CENTER);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);

	//paint history depth
	wxBoxSizer *group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Paint History"), wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	m_paint_hist_depth_sldr = new wxSingleSlider(page, ID_PaintHistDepthSldr, 0, 0, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_paint_hist_depth_text = new wxTextCtrl(page, ID_PaintHistDepthText, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
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
	m_pencil_dist_sldr = new wxSingleSlider(page, ID_PencilDistSldr, 30, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pencil_dist_text = new wxTextCtrl(page, ID_PencilDistText, "30",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
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
	m_micro_blend_chk = new wxCheckBox(page, ID_MicroBlendChk,
		"Enable Micro Blending");
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
	m_peeling_layers_sldr = new wxSingleSlider(page, ID_PeelingLayersSldr, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_peeling_layers_text = new wxTextCtrl(page, ID_PeelingLayersText, "1",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
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
	m_shadow_dir_chk = new wxCheckBox(page, ID_ShadowDirChk,
		"Enable directional shadow");
	m_shadow_dir_sldr = new wxSingleSlider(page, ID_ShadowDirSldr, -45, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_dir_sldr->SetRangeStyle(2);
	m_shadow_dir_text = new wxTextCtrl(page, ID_ShadowDirText, "-45",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_fp2);
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

	//link rotations
	wxBoxSizer* group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Rotations"), wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Rot. center anchor start");
	m_pin_threshold_sldr = new wxSingleSlider(page, ID_PinThreshSldr, 100, 10, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pin_threshold_text = new wxTextCtrl(page, ID_PinThreshText, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	sizer4_1->Add(st, 0, wxALIGN_CENTER);
	sizer4_1->Add(m_pin_threshold_sldr, 1, wxEXPAND);
	sizer4_1->Add(m_pin_threshold_text, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer4_2 = new wxBoxSizer(wxHORIZONTAL);
	m_rot_link_chk = new wxCheckBox(page, ID_RotLinkChk,
		"Link all rendering views' rotations.");
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
	m_grad_bg_chk = new wxCheckBox(page, ID_GradBgChk,
		"Enable gradient background");
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
	m_mouse_int_chk = new wxCheckBox(page, ID_MouseIntChk,
		"Reduce volume sample rate for mouse interactions.\n"\
		"Enable this option if mouse interaction speed is slow.");
	group1->Add(10, 5);
	group1->Add(m_mouse_int_chk);
	group1->Add(10, 5);

	//memory settings
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Large Data Streaming"), wxVERTICAL);
	m_streaming_chk = new wxCheckBox(page, ID_StreamingChk,
		"Enable streaming for large data.");
	wxString choices[2] = {"Back to front", "Front to back"};
	m_update_order_rbox = new wxRadioBox(page, ID_UpdateOrderRbox,
		"Update order", wxDefaultPosition, wxDefaultSize,
		2, choices, 0, wxRA_SPECIFY_COLS);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Graphics Memory:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_1->Add(st);
	m_graphics_mem_sldr = new wxSingleSlider(page, ID_GraphicsMemSldr, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_graphics_mem_text = new wxTextCtrl(page, ID_GraphicsMemText, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_1->Add(m_graphics_mem_sldr, 1, wxEXPAND);
	sizer2_1->Add(m_graphics_mem_text, 0, wxALIGN_CENTER);
	sizer2_1->Add(st);
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Large Data Size:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_2->Add(st);
	m_large_data_sldr = new wxSingleSlider(page, ID_LargeDataSldr, 20, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_large_data_text = new wxTextCtrl(page, ID_LargeDataText, "200",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_2->Add(m_large_data_sldr, 1, wxEXPAND);
	sizer2_2->Add(m_large_data_text, 0, wxALIGN_CENTER);
	sizer2_2->Add(st);
	wxBoxSizer *sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Brick Size:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_3->Add(st);
	m_block_size_sldr = new wxSingleSlider(page, ID_BlockSizeSldr, 7, 6, 12,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_block_size_text = new wxTextCtrl(page, ID_BlockSizeText, "128",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	st = new wxStaticText(page, 0, "vx",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_3->Add(m_block_size_sldr, 1, wxEXPAND);
	sizer2_3->Add(m_block_size_text, 0, wxALIGN_CENTER);
	sizer2_3->Add(st);
	wxBoxSizer *sizer2_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Response Time:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_4->Add(st);
	m_response_time_sldr = new wxSingleSlider(page, ID_ResponseTimeSldr, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_response_time_text = new wxTextCtrl(page, ID_ResponseTimeText, "100",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	st = new wxStaticText(page, 0, "ms",
		wxDefaultPosition, FromDIP(wxSize(20, -1)));
	sizer2_4->Add(m_response_time_sldr, 1, wxEXPAND);
	sizer2_4->Add(m_response_time_text, 0, wxALIGN_CENTER);
	sizer2_4->Add(st);
	wxBoxSizer *sizer2_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Detail Level Offset:",
		wxDefaultPosition, FromDIP(wxSize(110, -1)));
	sizer2_5->Add(st);
	m_detail_level_offset_sldr = new wxSingleSlider(page, ID_DetailLevelOffsetSldr, 0, -5, 5,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_detail_level_offset_text = new wxTextCtrl(page, ID_DetailLevelOffsetText, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int2);
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
	wxPanel* page = new wxPanel(parent);
	page->SetBackgroundColour(((wxNotebook*)parent)->GetThemeBackgroundColour());

	//stereo
	wxBoxSizer* group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Stereo && SteamVR"), wxVERTICAL);
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_stereo_chk = new wxCheckBox(page, ID_StereoChk,
		"Enable stereo (Install SteamVR and restart. Otherwise side-by-side only.)");
	sizer1_1->Add(m_stereo_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	m_sbs_chk = new wxCheckBox(page, ID_SBSChk,
		"Aspect Ratio for 3D TV");
	sizer1_2->Add(m_sbs_chk, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer1_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Eye distance");
	m_eye_dist_sldr = new wxSingleSlider(page, ID_EyeDistSldr, 200, 0, 2000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_eye_dist_text = new wxTextCtrl(page, ID_EyeDistText, "20.0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_fp1);
	sizer1_3->Add(st, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_eye_dist_sldr, 1, wxEXPAND);
	sizer1_3->Add(m_eye_dist_text, 0, wxALIGN_CENTER);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_2, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_3, 0, wxEXPAND);
	group1->Add(10, 5);

	//full screen display
	wxBoxSizer* group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Fullscreen on Display"), wxVERTICAL);
	wxBoxSizer* sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Display ID:",
		wxDefaultPosition, FromDIP(wxSize(50, -1)));
	m_disp_id_comb = new wxComboBox(page, ID_DispIdCombo, "",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, NULL, wxCB_READONLY);
	int dn = wxDisplay::GetCount();
	std::vector<wxString> list1;
	for (int i = 0; i < dn; ++i)
		list1.push_back(wxString::Format("%d", i));
	m_disp_id_comb->Append(list1);
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
		wxDefaultPosition, FromDIP(wxSize(50, -1)));
	m_color_depth_comb = new wxComboBox(page, ID_ColorDepthCombo, "",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> list2 = { "8", "10", "16" };
	m_color_depth_comb->Append(list2);
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
	m_override_vox_chk = new wxCheckBox(page, ID_OverrideVoxChk,
		"Get voxel size info from the first opened data set.");
	sizer1_1->Add(m_override_vox_chk, 0, wxALIGN_CENTER);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);

	//wavelength to color
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Default Colors for Excitation Wavelengths (nm) (for raw microscopy formats)"), wxVERTICAL);
	//combo box line
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_wav_color1_cmb = new wxComboBox(page, ID_WavColor1Cmb, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	m_wav_color2_cmb = new wxComboBox(page, ID_WavColor2Cmb, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	m_wav_color3_cmb = new wxComboBox(page, ID_WavColor3Cmb, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	m_wav_color4_cmb = new wxComboBox(page, ID_WavColor4Cmb, "",
		wxDefaultPosition, FromDIP(wxSize(75, 23)), 0, NULL, wxCB_READONLY);
	//1
	m_wav_color1_cmb->Append("Red");
	m_wav_color1_cmb->Append("Green");
	m_wav_color1_cmb->Append("Blue");
	m_wav_color1_cmb->Append("Cyan");
	m_wav_color1_cmb->Append("Magenta");
	m_wav_color1_cmb->Append("Yellow");
	m_wav_color1_cmb->Append("Orange");
	m_wav_color1_cmb->Append("White");
	//2
	m_wav_color2_cmb->Append("Red");
	m_wav_color2_cmb->Append("Green");
	m_wav_color2_cmb->Append("Blue");
	m_wav_color2_cmb->Append("Cyan");
	m_wav_color2_cmb->Append("Magenta");
	m_wav_color2_cmb->Append("Yellow");
	m_wav_color2_cmb->Append("Orange");
	m_wav_color2_cmb->Append("White");
	//3
	m_wav_color3_cmb->Append("Red");
	m_wav_color3_cmb->Append("Green");
	m_wav_color3_cmb->Append("Blue");
	m_wav_color3_cmb->Append("Cyan");
	m_wav_color3_cmb->Append("Magenta");
	m_wav_color3_cmb->Append("Yellow");
	m_wav_color3_cmb->Append("Orange");
	m_wav_color3_cmb->Append("White");
	//4
	m_wav_color4_cmb->Append("Red");
	m_wav_color4_cmb->Append("Green");
	m_wav_color4_cmb->Append("Blue");
	m_wav_color4_cmb->Append("Cyan");
	m_wav_color4_cmb->Append("Magenta");
	m_wav_color4_cmb->Append("Yellow");
	m_wav_color4_cmb->Append("Orange");
	m_wav_color4_cmb->Append("White");
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
	m_max_texture_size_chk = new wxCheckBox(page, ID_MaxTextureSizeChk,
		"Set max texture size");
	m_max_texture_size_text = new wxTextCtrl(page, ID_MaxTextureSizeText, "2048",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
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
	m_device_tree = new wxTreeCtrl(page, ID_DeviceTree,
		wxDefaultPosition, wxDefaultSize,
		wxTR_FULL_ROW_HIGHLIGHT);
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

	m_java_jvm_text = new wxTextCtrl(page, ID_JavaJVMText);
	m_java_ij_text = new wxTextCtrl(page, ID_JavaIJText);
	m_java_bioformats_text = new wxTextCtrl(page, ID_JavaBioformatsText);
	m_browse_jvm_btn = new wxButton(page, ID_JavaJvmBrowseBtn, "Browse", wxDefaultPosition);
	m_browse_ij_btn = new wxButton(page, ID_JavaIJBrowseBtn, "Browse", wxDefaultPosition);
	m_browse_bioformats_btn = new wxButton(page, ID_JavaBioformatsBrowseBtn, "Browse", wxDefaultPosition);

	sizer1_1->Add(m_java_jvm_text, 1, wxEXPAND);
	sizer1_1->Add(m_browse_jvm_btn, 0, wxALIGN_CENTER);
	sizer1_2->Add(m_java_ij_text, 1, wxEXPAND);
	sizer1_2->Add(m_browse_ij_btn, 0, wxALIGN_CENTER);
	sizer1_3->Add(m_java_bioformats_text, 1, wxEXPAND);
	sizer1_3->Add(m_browse_bioformats_btn, 0, wxALIGN_CENTER);
	
	group1->Add(5, 5);

	//Added the radio button
	st = new wxStaticText(page, 0, "Package:");
	mp_radio_button_imagej = new wxRadioButton(page, ID_RadioButtonImageJ, "ImageJ", wxDefaultPosition);
	mp_radio_button_fiji = new wxRadioButton(page, ID_RadioButtonFiji, "Fiji", wxDefaultPosition);
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
	wxPanel(frame, wxID_ANY,
		wxDefaultPosition,
		frame->FromDIP(wxSize(450, 750)),
		0, "SettingDlg"),
	m_frame(frame)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
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
	m_save_btn = new wxButton(this, ID_SaveBtn, "Done");
	m_close_btn = new wxButton(this, ID_CloseBtn, "Cancel");
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

	UpdateUI();
}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::UpdateUI()
{
	//update user interface
	//save project
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
	//mouse interactions
	m_mouse_int_chk->SetValue(glbin_settings.m_mouse_int);
	//depth peeling
	m_peeling_layers_sldr->ChangeValue(glbin_settings.m_peeling_layers);
	m_peeling_layers_text->ChangeValue(wxString::Format("%d", glbin_settings.m_peeling_layers));
	//micro blending
	m_micro_blend_chk->SetValue(glbin_settings.m_micro_blend);
	//shadow direction
	if (glbin_settings.m_shadow_dir)
	{
		m_shadow_dir_chk->SetValue(true);
		m_shadow_dir_sldr->Enable();
		m_shadow_dir_text->Enable();
	}
	else
	{
		m_shadow_dir_chk->SetValue(false);
		m_shadow_dir_sldr->Disable();
		m_shadow_dir_text->Disable();
		glbin_settings.m_shadow_dir_x = 0.0;
		glbin_settings.m_shadow_dir_y = 0.0;
	}
	double deg = GetShadowDir();
	m_shadow_dir_sldr->ChangeValue(std::round(deg));
	m_shadow_dir_text->ChangeValue(wxString::Format("%.2f", deg));
	//rot center anchor thresh
	m_pin_threshold_sldr->ChangeValue(std::round(glbin_settings.m_pin_threshold*10.0));
	m_pin_threshold_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_pin_threshold*100.0));
	//gradient background
	m_grad_bg_chk->SetValue(glbin_settings.m_grad_bg);
	//stereo
	m_stereo_chk->SetValue(glbin_settings.m_stereo);
	if (glbin_settings.m_stereo)
		m_sbs_chk->Enable();
	else
		m_sbs_chk->Disable();
	m_sbs_chk->SetValue(glbin_settings.m_sbs);
	m_eye_dist_sldr->ChangeValue(std::round(glbin_settings.m_eye_dist*10.0));
	m_eye_dist_text->ChangeValue(wxString::Format("%.1f", glbin_settings.m_eye_dist));
	//display id
	m_disp_id_comb->Select(glbin_settings.m_disp_id);
	//color depth
	m_color_depth_comb->Select(glbin_settings.m_color_depth);
	//override vox
	m_override_vox_chk->SetValue(glbin_settings.m_override_vox);
	//wavelength to color
	m_wav_color1_cmb->Select(glbin_settings.m_wav_color1 - 1);
	m_wav_color2_cmb->Select(glbin_settings.m_wav_color2 - 1);
	m_wav_color3_cmb->Select(glbin_settings.m_wav_color3 - 1);
	m_wav_color4_cmb->Select(glbin_settings.m_wav_color4 - 1);
	//max texture size
	m_max_texture_size_chk->SetValue(glbin_settings.m_use_max_texture_size);
	if (glbin_settings.m_use_max_texture_size)
	{
		flvr::ShaderProgram::set_max_texture_size(glbin_settings.m_max_texture_size);
		m_max_texture_size_text->ChangeValue(
			wxString::Format("%d", glbin_settings.m_max_texture_size));
		m_max_texture_size_text->Enable();
	}
	else
	{
		m_max_texture_size_text->ChangeValue(
			wxString::Format("%d", flvr::ShaderProgram::
				max_texture_size()));
		m_max_texture_size_text->Disable();
	}
	//no tex pack
	flvr::ShaderProgram::set_no_tex_upack(glbin_settings.m_no_tex_pack);
	//font
	wxString str = glbin_settings.m_font_file.BeforeLast('.');
	int font_sel = m_font_cmb->FindString(str);
	if (font_sel != wxNOT_FOUND)
		m_font_cmb->Select(font_sel);
	m_font_size_cmb->ChangeValue(wxString::Format("%d", glbin_settings.m_text_size));
	m_text_color_cmb->Select(glbin_settings.m_text_color);
	//line width
	m_line_width_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_line_width));
	m_line_width_sldr->ChangeValue(std::round(glbin_settings.m_line_width));
	//paint history depth
	m_paint_hist_depth_text->ChangeValue(wxString::Format("%d", glbin_brush_def.m_paint_hist_depth));
	m_paint_hist_depth_sldr->ChangeValue(glbin_brush_def.m_paint_hist_depth);
	//pencil distance
	m_pencil_dist_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_pencil_dist));
	m_pencil_dist_sldr->ChangeValue(glbin_settings.m_pencil_dist);
	//memory settings
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

	//java
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

void SettingDlg::UpdateDeviceTree()
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

void SettingDlg::UpdateTextureSize()
{
	if (!glbin_settings.m_use_max_texture_size)
	{
		m_max_texture_size_text->ChangeValue(
			wxString::Format("%d", flvr::ShaderProgram::
				max_texture_size()));
	}
	else
		flvr::ShaderProgram::set_max_texture_size(glbin_settings.m_max_texture_size);
}

//events
void SettingDlg::OnSave(wxCommandEvent &event)
{
	glbin_settings.Save();
	if (m_frame)
		m_frame->ShowPane(this, false);
}

void SettingDlg::OnClose(wxCommandEvent &event)
{
	if (m_frame)
		m_frame->ShowPane(this, false);
}

void SettingDlg::OnShow(wxShowEvent &event)
{
	//GetSettings();
}

void SettingDlg::OnProjectSaveCheck(wxCommandEvent &event)
{
	glbin_settings.m_prj_save = m_prj_save_chk->GetValue();
}

void SettingDlg::OnProjectSaveIncCheck(wxCommandEvent& event)
{
	glbin_settings.m_prj_save_inc = m_prj_save_inc_chk->GetValue();
}

void SettingDlg::OnRealtimeCompressCheck(wxCommandEvent &event)
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
	m_frame->UpdateProps({ gstMultiFuncTips });
}

void SettingDlg::OnMouseIntCheck(wxCommandEvent &event)
{
	glbin_settings.m_mouse_int = m_mouse_int_chk->GetValue();

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetAdaptive(glbin_settings.m_mouse_int);
				view->RefreshGL(39);
			}
		}
	}
}

void SettingDlg::OnPeelingLayersChange(wxScrollEvent &event)
{
	int ival = m_peeling_layers_sldr->GetValue();;
	wxString str = wxString::Format("%d", ival);
	if (str != m_peeling_layers_text->GetValue())
		m_peeling_layers_text->SetValue(str);
}

void SettingDlg::OnPeelingLayersEdit(wxCommandEvent &event)
{
	wxString str = m_peeling_layers_text->GetValue();
	long ival;
	str.ToLong(&ival);
	if (ival <= 0)
		return;
	m_peeling_layers_sldr->ChangeValue(ival);
	glbin_settings.m_peeling_layers = ival;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetPeelingLayers(ival);
				view->RefreshGL(39);
			}
		}
	}
}

void SettingDlg::OnMicroBlendCheck(wxCommandEvent &event)
{
	glbin_settings.m_micro_blend = m_micro_blend_chk->GetValue();

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetBlendSlices(glbin_settings.m_micro_blend);
				view->RefreshGL(39);
			}
		}
	}
}

//shadow direction
void SettingDlg::OnShadowDirCheck(wxCommandEvent &event)
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

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
				view->RefreshGL(39);
		}
	}
}

void SettingDlg::OnShadowDirChange(wxScrollEvent &event)
{
	double deg = m_shadow_dir_sldr->GetValue();
	wxString str = wxString::Format("%.2f", deg);
	if (str != m_shadow_dir_text->GetValue())
		m_shadow_dir_text->SetValue(str);
}

void SettingDlg::OnShadowDirEdit(wxCommandEvent &event)
{
	wxString str = m_shadow_dir_text->GetValue();
	double deg;
	str.ToDouble(&deg);
	m_shadow_dir_sldr->ChangeValue(std::round(deg));
	SetShadowDir(deg);

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
				view->RefreshGL(39);
		}
	}
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
	if (m_frame)
	{
		m_frame->SetTextureRendererSettings();
		m_frame->RefreshCanvases();
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
void SettingDlg::OnGradBgCheck(wxCommandEvent &event)
{
	glbin_settings.m_grad_bg = m_grad_bg_chk->GetValue();

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
			{
				view->SetGradBg(glbin_settings.m_grad_bg);
				view->RefreshGL(39);
			}
		}
	}
}

//rot center anchor thresh
void SettingDlg::OnPinThresholdChange(wxScrollEvent &event)
{
	double dval = m_pin_threshold_sldr->GetValue();
	wxString str = wxString::Format("%.0f", dval*10.0);
	if (str != m_pin_threshold_text->GetValue())
		m_pin_threshold_text->SetValue(str);
}

void SettingDlg::OnPinThresholdEdit(wxCommandEvent &event)
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

	if (m_frame && 0 < m_frame->GetViewNum())
	{
		RenderCanvas* view = m_frame->GetView(0);
		if (view)
			view->RefreshGL(39);
	}
}

//stereo
void SettingDlg::OnStereoCheck(wxCommandEvent &event)
{
	glbin_settings.m_stereo = m_stereo_chk->GetValue();
	m_sbs_chk->Enable(glbin_settings.m_stereo);
	if (m_frame && 0 < m_frame->GetViewNum())
	{
		RenderCanvas* view = m_frame->GetView(0);
		if (view)
		{
			view->SetStereo(glbin_settings.m_stereo);
			view->RefreshGL(39);
		}
	}
}

void SettingDlg::OnSBSCheck(wxCommandEvent& event)
{
	glbin_settings.m_sbs = m_sbs_chk->GetValue();
	if (m_frame && 0 < m_frame->GetViewNum())
	{
		RenderCanvas* view = m_frame->GetView(0);
		if (view)
		{
			view->SetSBS(glbin_settings.m_sbs);
			view->RefreshGL(39);
		}
	}
}

void SettingDlg::OnEyeDistChange(wxScrollEvent &event)
{
	glbin_settings.m_eye_dist = m_eye_dist_sldr->GetValue() / 10.0;
	wxString str = wxString::Format("%.1f", glbin_settings.m_eye_dist);
	if (str != m_eye_dist_text->GetValue())
		m_eye_dist_text->SetValue(str);
}

void SettingDlg::OnEyeDistEdit(wxCommandEvent &event)
{
	wxString str = m_eye_dist_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_eye_dist_sldr->ChangeValue(std::round(dval * 10.0));
	glbin_settings.m_eye_dist = dval;

	if (m_frame && 0 < m_frame->GetViewNum())
	{
		RenderCanvas* view = m_frame->GetView(0);
		if (view)
		{
			view->SetEyeDist(glbin_settings.m_eye_dist);
			view->RefreshGL(39);
		}
	}
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
void SettingDlg::OnOverrideVoxCheck(wxCommandEvent &event)
{
	glbin_settings.m_override_vox = m_override_vox_chk->GetValue();
	glbin_data_manager.SetOverrideVox(glbin_settings.m_override_vox);
}

void SettingDlg::OnWavColor1Change(wxCommandEvent &event)
{
	if (m_wav_color1_cmb)
		glbin_settings.m_wav_color1 = m_wav_color1_cmb->GetCurrentSelection() + 1;
	glbin_data_manager.SetWavelengthColor(
			glbin_settings.m_wav_color1,
			glbin_settings.m_wav_color2,
			glbin_settings.m_wav_color3,
			glbin_settings.m_wav_color4);
}

void SettingDlg::OnWavColor2Change(wxCommandEvent &event)
{
	if (m_wav_color2_cmb)
		glbin_settings.m_wav_color2 = m_wav_color2_cmb->GetCurrentSelection() + 1;
	glbin_data_manager.SetWavelengthColor(
			glbin_settings.m_wav_color1,
			glbin_settings.m_wav_color2,
			glbin_settings.m_wav_color3,
			glbin_settings.m_wav_color4);
}

void SettingDlg::OnWavColor3Change(wxCommandEvent &event)
{
	if (m_wav_color3_cmb)
		glbin_settings.m_wav_color3 = m_wav_color3_cmb->GetCurrentSelection() + 1;
	glbin_data_manager.SetWavelengthColor(
			glbin_settings.m_wav_color1,
			glbin_settings.m_wav_color2,
			glbin_settings.m_wav_color3,
			glbin_settings.m_wav_color4);
}

void SettingDlg::OnWavColor4Change(wxCommandEvent &event)
{
	if (m_wav_color4_cmb)
		glbin_settings.m_wav_color4 = m_wav_color4_cmb->GetCurrentSelection() + 1;
	glbin_data_manager.SetWavelengthColor(
			glbin_settings.m_wav_color1,
			glbin_settings.m_wav_color2,
			glbin_settings.m_wav_color3,
			glbin_settings.m_wav_color4);
}

//texture size
void SettingDlg::OnMaxTextureSizeChk(wxCommandEvent &event)
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

void SettingDlg::OnMaxTextureSizeEdit(wxCommandEvent &event)
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
void SettingDlg::OnStreamingChk(wxCommandEvent &event)
{
	if (m_streaming_chk->GetValue())
		glbin_settings.m_mem_swap = true;
	else
		glbin_settings.m_mem_swap = false;
	EnableStreaming(glbin_settings.m_mem_swap);
}

void SettingDlg::OnUpdateOrderChange(wxCommandEvent &event)
{
	glbin_settings.m_update_order = m_update_order_rbox->GetSelection();
}

void SettingDlg::OnGraphicsMemChange(wxScrollEvent &event)
{
	int ival = m_graphics_mem_sldr->GetValue();
	wxString str = wxString::Format("%d", ival * 100);
	if (str != m_graphics_mem_text->GetValue())
		m_graphics_mem_text->SetValue(str);
}

void SettingDlg::OnGraphicsMemEdit(wxCommandEvent &event)
{
	wxString str = m_graphics_mem_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val <= 0.0)
		return;
	m_graphics_mem_sldr->ChangeValue(std::round(val / 100.0));
	glbin_settings.m_graphics_mem = val;
}

void SettingDlg::OnLargeDataChange(wxScrollEvent &event)
{
	int ival = m_large_data_sldr->GetValue();
	wxString str = wxString::Format("%d", ival * 10);
	if (str != m_large_data_text->GetValue())
		m_large_data_text->SetValue(str);
}

void SettingDlg::OnLargeDataEdit(wxCommandEvent &event)
{
	wxString str = m_large_data_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val < 0.0)
		return;
	m_large_data_sldr->ChangeValue(std::round(val / 10.0));
	glbin_settings.m_large_data_size = val;
}

void SettingDlg::OnBlockSizeChange(wxScrollEvent &event)
{
	int ival = m_block_size_sldr->GetValue();
	wxString str = wxString::Format("%d", 2 << (ival - 1));
	if (str != m_block_size_text->GetValue())
		m_block_size_text->SetValue(str);
}

void SettingDlg::OnBlockSizeEdit(wxCommandEvent &event)
{
	wxString str = m_block_size_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val <= 0.0)
		return;
	m_block_size_sldr->ChangeValue(std::round(log(val) / log(2.0)));
	glbin_settings.m_force_brick_size = val;
}

void SettingDlg::OnResponseTimeChange(wxScrollEvent &event)
{
	int ival = m_response_time_sldr->GetValue();
	wxString str = wxString::Format("%d", ival * 10);
	if (str != m_response_time_text->GetValue())
		m_response_time_text->SetValue(str);
}

void SettingDlg::OnResponseTimeEdit(wxCommandEvent &event)
{
	wxString str = m_response_time_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val <= 0.0)
		return;
	m_response_time_sldr->ChangeValue(std::round(val / 10.0));
	glbin_settings.m_up_time = val;
}

void SettingDlg::OnDetailLevelOffsetChange(wxScrollEvent &event)
{
	int ival = m_detail_level_offset_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_detail_level_offset_text->GetValue())
		m_detail_level_offset_text->SetValue(str);
}

void SettingDlg::OnDetailLevelOffsetEdit(wxCommandEvent &event)
{
	wxString str = m_detail_level_offset_text->GetValue();
	long val;
	str.ToLong(&val);
	m_detail_level_offset_sldr->ChangeValue(val);
	glbin_settings.m_detail_level_offset = -val;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
				view->RefreshGL(39);
		}
	}
}

//font
void SettingDlg::OnFontChange(wxCommandEvent &event)
{
	wxString str = m_font_cmb->GetValue();
	if (str != "")
	{
		glbin_settings.m_font_file = str + ".ttf";
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		wxString loc = exePath + GETSLASH() + "Fonts" +
			GETSLASH() + str + ".ttf";

		if (m_frame)
		{
			flvr::TextRenderer::text_texture_manager_.load_face(loc.ToStdString());
			flvr::TextRenderer::text_texture_manager_.SetSize(glbin_settings.m_text_size);
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				RenderCanvas* view = m_frame->GetView(i);
				if (view)
					view->RefreshGL(39);
			}
		}
	}
}

void SettingDlg::OnFontSizeChange(wxCommandEvent &event)
{
	wxString str = m_font_size_cmb->GetValue();
	long size;
	if (str.ToLong(&size))
	{
		glbin_settings.m_text_size = size;

		if (m_frame)
		{
			flvr::TextRenderer::text_texture_manager_.SetSize(glbin_settings.m_text_size);
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				RenderCanvas* view = m_frame->GetView(i);
				if (view)
					view->RefreshGL(39);
			}
		}
	}
}

void SettingDlg::OnTextColorChange(wxCommandEvent &event)
{
	glbin_settings.m_text_color = m_text_color_cmb->GetCurrentSelection();
	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetView(i);
			if (view)
				view->RefreshGL(39);
		}
	}
}

//line width
void SettingDlg::OnLineWidthSldr(wxScrollEvent &event)
{
	int ival = m_line_width_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_line_width_text->GetValue())
		m_line_width_text->SetValue(str);
}

void SettingDlg::OnLineWidthText(wxCommandEvent &event)
{
	wxString str = m_line_width_text->GetValue();
	unsigned long ival;
	if (str.ToULong(&ival))
	{
		m_line_width_sldr->ChangeValue(ival);
		glbin_settings.m_line_width = ival;
		if (m_frame)
		{
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				RenderCanvas* view = m_frame->GetView(i);
				if (view)
					view->RefreshGL(39);
			}
		}
	}
}

//paint history depth
void SettingDlg::OnPaintHistDepthChange(wxScrollEvent &event)
{
	int ival = m_paint_hist_depth_sldr->GetValue();
	wxString str = wxString::Format("%d", ival);
	if (str != m_paint_hist_depth_text->GetValue())
		m_paint_hist_depth_text->SetValue(str);
}

void SettingDlg::OnPaintHistDepthEdit(wxCommandEvent &event)
{
	wxString str = m_paint_hist_depth_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	m_paint_hist_depth_sldr->ChangeValue(ival);
	glbin_brush_def.m_paint_hist_depth = ival;
	if (m_frame)
		m_frame->SetTextureUndos();
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
void SettingDlg::OnJavaJvmEdit(wxCommandEvent &event)
{
	glbin_settings.m_jvm_path = m_java_jvm_text->GetValue();
}

void SettingDlg::OnJavaIJEdit(wxCommandEvent &event)
{
	glbin_settings.m_ij_path = m_java_ij_text->GetValue();
}

void SettingDlg::OnJavaBioformatsEdit(wxCommandEvent &event)
{
	glbin_settings.m_bioformats_path = m_java_bioformats_text->GetValue();
}

void SettingDlg::onJavaJvmBrowse(wxCommandEvent &event)
{
#ifdef _WIN32
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose the jvm dll file",
		"", "", "*.dll", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#else
	wxFileDialog* fopendlg = new wxFileDialog(
		m_frame, "Choose the libjvm.dylib file",
		"", "", "*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_java_jvm_text->ChangeValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaIJBrowse(wxCommandEvent &event)
{
#ifdef _WIN32	
	//wxFileDialog *fopendlg = new wxFileDialog(
	//	m_frame, "Choose the imageJ/fiji directory",
	//	"", "", "*.jar", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	wxDirDialog *fopendlg = new wxDirDialog(
		m_frame, "Choose the imageJ/fiji directory",
		"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
#else
	wxFileDialog* fopendlg = new wxFileDialog(
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
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaBioformatsBrowse(wxCommandEvent &event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose the bioformats jar",
		"", "", "*.jar", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_java_bioformats_text->ChangeValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaRadioButtonImageJ(wxCommandEvent &event) {
	m_java_jvm_text->Enable(true);
	m_java_bioformats_text->Enable(true);
	m_browse_jvm_btn->Enable(true);
	m_browse_bioformats_btn->Enable(true);
	glbin_settings.m_ij_mode = 0;
}

void SettingDlg::onJavaRadioButtonFiji(wxCommandEvent &event) {
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