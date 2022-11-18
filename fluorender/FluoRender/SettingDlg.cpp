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
#include "SettingDlg.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>
#include <wx/notebook.h>
#include <wx/stdpaths.h>

BEGIN_EVENT_TABLE(SettingDlg, wxPanel)
EVT_BUTTON(ID_SaveBtn, SettingDlg::OnSave)
EVT_BUTTON(ID_CloseBtn, SettingDlg::OnClose)
//project save
EVT_CHECKBOX(ID_PrjSaveChk, SettingDlg::OnProjectSaveCheck)
//real time compress
EVT_CHECKBOX(ID_RealtimeCmpChk, SettingDlg::OnRealtimeCompressCheck)
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
EVT_COMMAND_SCROLL(ID_EyeDistSldr, SettingDlg::OnEyeDistChange)
EVT_TEXT(ID_EyeDistText, SettingDlg::OnEyeDistEdit)
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

	//project save
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Open/Save"), wxVERTICAL);
	m_prj_save_chk = new wxCheckBox(page, ID_PrjSaveChk,
		"Save project when capture viewport or export movie.");
	m_realtime_cmp_chk = new wxCheckBox(page, ID_RealtimeCmpChk,
		"Compress data in graphics memory when loading.");
	group1->Add(10, 5);
	group1->Add(m_prj_save_chk);
	group1->Add(10, 5);
	group1->Add(m_realtime_cmp_chk);
	group1->Add(10, 5);

	//font
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Render View Text"), wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Font:");
	m_font_cmb = new wxComboBox(page, ID_FontCmb, "",
		wxDefaultPosition, wxSize(150, -1), 0, NULL, wxCB_READONLY);
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
		wxDefaultPosition, wxSize(50, -1), 0, NULL);
	for (int df = 3; df < 18; ++df)
		m_font_size_cmb->Append(wxString::Format("%d", int(std::pow(df, 1.5))));
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_font_size_cmb);
	sizer2_1->Add(10, 10);
	st = new wxStaticText(page, 0, "Color:");
	m_text_color_cmb = new wxComboBox(page, ID_TextColorCmb, "",
		wxDefaultPosition, wxSize(100, -1), 0, NULL, wxCB_READONLY);
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
	m_line_width_sldr = new wxSlider(page, ID_LineWidthSldr, 3, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_line_width_text = new wxTextCtrl(page, ID_LineWidthText, "3",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	sizer3_1->Add(m_line_width_sldr, 1, wxEXPAND);
	sizer3_1->Add(m_line_width_text, 0, wxALIGN_CENTER);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);

	//paint history depth
	wxBoxSizer *group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Paint History"), wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	m_paint_hist_depth_sldr = new wxSlider(page, ID_PaintHistDepthSldr, 0, 0, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_paint_hist_depth_text = new wxTextCtrl(page, ID_PaintHistDepthText, "0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
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
	m_peeling_layers_sldr = new wxSlider(page, ID_PeelingLayersSldr, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_peeling_layers_text = new wxTextCtrl(page, ID_PeelingLayersText, "1",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
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
	m_shadow_dir_sldr = new wxSlider(page, ID_ShadowDirSldr, -45, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_dir_text = new wxTextCtrl(page, ID_ShadowDirText, "-45",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp2);
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
	m_pin_threshold_sldr = new wxSlider(page, ID_PinThreshSldr, 100, 10, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pin_threshold_text = new wxTextCtrl(page, ID_PinThreshText, "1000",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
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

	//stereo
	wxBoxSizer* group6 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Stereo && SteamVR"), wxVERTICAL);
	wxBoxSizer *sizer6_1 = new wxBoxSizer(wxHORIZONTAL);
	m_stereo_chk = new wxCheckBox(page, ID_StereoChk,
		"Enable stereo (Install SteamVR and restart. Otherwise side-by-side only.)");
	sizer6_1->Add(m_stereo_chk, 0, wxALIGN_CENTER);
	wxBoxSizer *sizer6_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Eye distance");
	m_eye_dist_sldr = new wxSlider(page, ID_EyeDistSldr, 200, 0, 2000,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_eye_dist_text = new wxTextCtrl(page, ID_EyeDistText, "20.0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
	sizer6_2->Add(st, 0, wxALIGN_CENTER);
	sizer6_2->Add(m_eye_dist_sldr, 1, wxEXPAND);
	sizer6_2->Add(m_eye_dist_text, 0, wxALIGN_CENTER);
	group6->Add(10, 5);
	group6->Add(sizer6_1, 0, wxEXPAND);
	group6->Add(10, 5);
	group6->Add(sizer6_2, 0, wxEXPAND);
	group6->Add(10, 5);

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
	sizerV->Add(group6, 0, wxEXPAND);

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
		"Enable streaming for large datasets.");
	wxString choices[2] = {"Back to front", "Front to back"};
	m_update_order_rbox = new wxRadioBox(page, ID_UpdateOrderRbox,
		"Update order", wxDefaultPosition, wxDefaultSize,
		2, choices, 0, wxRA_SPECIFY_COLS);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Graphics Memory:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_1->Add(st);
	m_graphics_mem_sldr = new wxSlider(page, ID_GraphicsMemSldr, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_graphics_mem_text = new wxTextCtrl(page, ID_GraphicsMemText, "1000",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, wxSize(20, -1));
	sizer2_1->Add(m_graphics_mem_sldr, 1, wxEXPAND);
	sizer2_1->Add(m_graphics_mem_text, 0, wxALIGN_CENTER);
	sizer2_1->Add(st);
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Large Data Size:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_2->Add(st);
	m_large_data_sldr = new wxSlider(page, ID_LargeDataSldr, 20, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_large_data_text = new wxTextCtrl(page, ID_LargeDataText, "200",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, wxSize(20, -1));
	sizer2_2->Add(m_large_data_sldr, 1, wxEXPAND);
	sizer2_2->Add(m_large_data_text, 0, wxALIGN_CENTER);
	sizer2_2->Add(st);
	wxBoxSizer *sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Brick Size:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_3->Add(st);
	m_block_size_sldr = new wxSlider(page, ID_BlockSizeSldr, 7, 6, 12,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_block_size_text = new wxTextCtrl(page, ID_BlockSizeText, "128",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	st = new wxStaticText(page, 0, "vx",
		wxDefaultPosition, wxSize(20, -1));
	sizer2_3->Add(m_block_size_sldr, 1, wxEXPAND);
	sizer2_3->Add(m_block_size_text, 0, wxALIGN_CENTER);
	sizer2_3->Add(st);
	wxBoxSizer *sizer2_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Response Time:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_4->Add(st);
	m_response_time_sldr = new wxSlider(page, ID_ResponseTimeSldr, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_response_time_text = new wxTextCtrl(page, ID_ResponseTimeText, "100",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	st = new wxStaticText(page, 0, "ms",
		wxDefaultPosition, wxSize(20, -1));
	sizer2_4->Add(m_response_time_sldr, 1, wxEXPAND);
	sizer2_4->Add(m_response_time_text, 0, wxALIGN_CENTER);
	sizer2_4->Add(st);
	wxBoxSizer *sizer2_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Detail Level Offset:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_5->Add(st);
	m_detail_level_offset_sldr = new wxSlider(page, ID_DetailLevelOffsetSldr, 0, -5, 5,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_detail_level_offset_text = new wxTextCtrl(page, ID_DetailLevelOffsetText, "0",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int2);
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
		"Data streaming allows rendering and processing datasets of larger size than\n"\
		"available graphics memory. Datasets are divided into bricks. The bricks are\n"\
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

wxWindow* SettingDlg::CreateFormatPage(wxWindow *parent)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//float 2
	wxFloatingPointValidator<double> vald_fp1(2);
	wxStaticText* st;
	wxPanel *page = new wxPanel(parent);

	//override vox
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Override Voxel Size"), wxVERTICAL);
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_override_vox_chk = new wxCheckBox(page, ID_OverrideVoxChk,
		"Get voxel size info from the first opened dataset.");
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
		wxDefaultPosition, wxSize(75, 23), 0, NULL, wxCB_READONLY);
	m_wav_color2_cmb = new wxComboBox(page, ID_WavColor2Cmb, "",
		wxDefaultPosition, wxSize(75, 23), 0, NULL, wxCB_READONLY);
	m_wav_color3_cmb = new wxComboBox(page, ID_WavColor3Cmb, "",
		wxDefaultPosition, wxSize(75, 23), 0, NULL, wxCB_READONLY);
	m_wav_color4_cmb = new wxComboBox(page, ID_WavColor4Cmb, "",
		wxDefaultPosition, wxSize(75, 23), 0, NULL, wxCB_READONLY);
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
	st = new wxStaticText(page, 0, "350", wxDefaultPosition, wxSize(30, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(30, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "450\n480\n488", wxDefaultPosition, wxSize(30, 50));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(30, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "543\n568", wxDefaultPosition, wxSize(30, 35));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(30, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "633", wxDefaultPosition, wxSize(30, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(30, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "700", wxDefaultPosition, wxSize(30, 20));
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
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
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

SettingDlg::SettingDlg(VRenderFrame *frame) :
	wxPanel(frame, wxID_ANY,
		wxDefaultPosition, wxSize(450, 750),
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

	GetSettings();
}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::GetSettings()
{
	m_gmc_mode = 2;
	m_prj_save = false;
	m_save_alpha = false;
	m_save_float = false;
	m_dpi = 72.0f;
	m_realtime_compress = false;
	m_skip_bricks = false;
	m_test_speed = false;
	m_test_param = false;
	m_test_wiref = false;
	m_peeling_layers = 1;
	m_micro_blend = false;
	m_shadow_dir = false;
	m_shadow_dir_x = 0.0;
	m_shadow_dir_y = 0.0;
	m_mouse_int = true;
	m_wav_color1 = 5;
	m_wav_color2 = 5;
	m_wav_color3 = 5;
	m_wav_color4 = 5;
	m_time_id = "_T";
	m_grad_bg = false;
	m_pin_threshold = 10.0;
	m_stereo = false;
	m_eye_dist = 20.0;
	m_override_vox = true;
	m_soft_threshold = 0.0;
	m_run_script = false;
	m_script_file = "";
	m_text_size = 12;
	m_text_color = 0;
	m_font_file = "";
	m_line_width = 3.0;
	m_mem_swap = false;
	m_graphics_mem = 1000.0;
	m_large_data_size = 1000.0;
	m_force_brick_size = 128;
	m_up_time = 100;
	m_update_order = 0;
	m_invalidate_tex = false;
	m_detail_level_offset = 0;
	m_point_volume_mode = 0;
	m_ruler_use_transf = false;
	m_ruler_relax_f1 = 2.0;
	m_ruler_infr = 2.0;
	m_ruler_relax_iter = 10;
	m_ruler_auto_relax = false;
	m_ruler_relax_type = 1;
	m_ruler_df_f = false;
	m_bg_type = 0;
	m_kx = 100;
	m_ky = 100;
	m_varth = 0.0001;
	m_gauth = 1;
	m_ruler_size_thresh = 5;
	m_pvxml_flip_x = false;
	m_pvxml_flip_y = false;
	m_pvxml_seq_type = 1;
	m_api_type = 0;
	m_red_bit = 8;
	m_green_bit = 8;
	m_blue_bit = 8;
	m_alpha_bit = 8;
	m_depth_bit = 24;
	m_samples = 0;
	m_gl_major_ver = 4;
	m_gl_minor_ver = 4;
	m_gl_profile_mask = 2;
	m_cl_platform_id = 0;
	m_cl_device_id = 0;
	m_paint_hist_depth = 0;
	m_stay_top = false;
	m_show_cursor = true;
	m_last_tool = 0;
	//tracking settings
	m_track_iter = 3;
	m_component_size = 25.0;
	m_consistent_color = true;
	m_try_merge = false;
	m_try_split = false;
	m_contact_factor = 0.6;
	m_similarity = 0.5;
	m_use_max_texture_size = false;
	m_max_texture_size = 2048;
	m_no_tex_pack = false;
	m_plane_mode = 0;
	m_ij_mode = 0;

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "fluorender.set";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//gradient magnitude
	if (fconfig.Exists("/gm calculation"))
	{
		fconfig.SetPath("/gm calculation");
		m_gmc_mode = fconfig.Read("mode", 2l);
	}
	//depth peeling
	if (fconfig.Exists("/peeling layers"))
	{
		fconfig.SetPath("/peeling layers");
		m_peeling_layers = fconfig.Read("value", 1l);
	}
	//micro blending
	if (fconfig.Exists("/micro blend"))
	{
		fconfig.SetPath("/micro blend");
		fconfig.Read("mode", &m_micro_blend, false);
	}
	//save project
	if (fconfig.Exists("/save project"))
	{
		fconfig.SetPath("/save project");
		fconfig.Read("mode", &m_prj_save, false);
	}
	//save alpha
	if (fconfig.Exists("/save alpha"))
	{
		fconfig.SetPath("/save alpha");
		fconfig.Read("mode", &m_save_alpha, false);
	}
	//save float
	if (fconfig.Exists("/save float"))
	{
		fconfig.SetPath("/save float");
		fconfig.Read("mode", &m_save_float, false);
	}
	//dpi
	if (fconfig.Exists("/dpi"))
	{
		fconfig.SetPath("/dpi");
		fconfig.Read("value", &m_dpi, 72.0f);
	}
	//realtime compression
	if (fconfig.Exists("/realtime compress"))
	{
		fconfig.SetPath("/realtime compress");
		fconfig.Read("mode", &m_realtime_compress, false);
	}
	//skip empty bricks
	if (fconfig.Exists("/skip bricks"))
	{
		fconfig.SetPath("/skip bricks");
		fconfig.Read("mode", &m_skip_bricks, false);
	}
	//mouse interactions
	if (fconfig.Exists("/mouse int"))
	{
		fconfig.SetPath("/mouse int");
		fconfig.Read("mode", &m_mouse_int, true);
	}
	//shadow
	if (fconfig.Exists("/dir shadow"))
	{
		fconfig.SetPath("/dir shadow");
		fconfig.Read("mode", &m_shadow_dir, false);
		fconfig.Read("x", &m_shadow_dir_x, 0.0);
		fconfig.Read("y", &m_shadow_dir_y, 0.0);
	}
	//rot center anchor thresh
	if (fconfig.Exists("/pin threshold"))
	{
		fconfig.SetPath("/pin threshold");
		fconfig.Read("value", &m_pin_threshold, 10.0);
	}
	//stereo
	if (fconfig.Exists("/stereo"))
	{
		fconfig.SetPath("/stereo");
		fconfig.Read("enable_stereo", &m_stereo, false);
		fconfig.Read("eye dist", &m_eye_dist, 20.0);
	}
	//test mode
	if (fconfig.Exists("/test mode"))
	{
		fconfig.SetPath("/test mode");
		fconfig.Read("speed", &m_test_speed, false);
		fconfig.Read("param", &m_test_param, false);
		fconfig.Read("wiref", &m_test_wiref, false);
	}
	//wavelength to color
	if (fconfig.Exists("/wavelength to color"))
	{
		fconfig.SetPath("/wavelength to color");
		fconfig.Read("c1", &m_wav_color1, 5);
		fconfig.Read("c2", &m_wav_color2, 5);
		fconfig.Read("c3", &m_wav_color3, 5);
		fconfig.Read("c4", &m_wav_color4, 5);
	}
	//time sequence identifier
	if (fconfig.Exists("/time id"))
	{
		fconfig.SetPath("/time id");
		fconfig.Read("value", &m_time_id);
	}
	//gradient background
	if (fconfig.Exists("/grad bg"))
	{
		fconfig.SetPath("/grad bg");
		fconfig.Read("value", &m_grad_bg);
	}
	//save compressed
	if (fconfig.Exists("/save cmp"))
	{
		bool save_cmp;
		fconfig.SetPath("/save cmp");
		fconfig.Read("value", &save_cmp);
		VRenderFrame::SetCompression(save_cmp);
	}
	//override vox
	if (fconfig.Exists("/override vox"))
	{
		fconfig.SetPath("/override vox");
		fconfig.Read("value", &m_override_vox);
	}
	//soft threshold
	if (fconfig.Exists("/soft threshold"))
	{
		fconfig.SetPath("/soft threshold");
		fconfig.Read("value", &m_soft_threshold);
	}
	//run script
	if (fconfig.Exists("/run script"))
	{
		fconfig.SetPath("/run script");
		//fconfig.Read("value", &m_run_script);
		fconfig.Read("file", &m_script_file);
	}
	//paint history depth
	if (fconfig.Exists("/paint history"))
	{
		fconfig.SetPath("/paint history");
		fconfig.Read("value", &m_paint_hist_depth);
	}
	//text font
	if (fconfig.Exists("/text font"))
	{
		fconfig.SetPath("/text font");
		fconfig.Read("file", &m_font_file);
		fconfig.Read("value", &m_text_size);
		fconfig.Read("color", &m_text_color);
	}
	//line width
	if (fconfig.Exists("/line width"))
	{
		fconfig.SetPath("/line width");
		fconfig.Read("value", &m_line_width);
	}
	//full screen
	if (fconfig.Exists("/full screen"))
	{
		fconfig.SetPath("/full screen");
		fconfig.Read("stay top", &m_stay_top);
		fconfig.Read("show cursor", &m_show_cursor);
	}
	//last tool
	if (fconfig.Exists("/last tool"))
	{
		fconfig.SetPath("/last tool");
		fconfig.Read("value", &m_last_tool);
	}
	//memory settings
	if (fconfig.Exists("/memory settings"))
	{
		fconfig.SetPath("/memory settings");
		//enable mem swap
		fconfig.Read("mem swap", &m_mem_swap);
		//graphics memory limit
		fconfig.Read("graphics mem", &m_graphics_mem);
		//large data size
		fconfig.Read("large data size", &m_large_data_size);
		//force brick size
		fconfig.Read("force brick size", &m_force_brick_size);
		//response time
		fconfig.Read("up time", &m_up_time);
		//detail level offset
		fconfig.Read("detail level offset", &m_detail_level_offset);
	}
	EnableStreaming(m_mem_swap);
	//update order
	if (fconfig.Exists("/update order"))
	{
		fconfig.SetPath("/update order");
		fconfig.Read("value", &m_update_order);
	}
	//invalidate texture
	if (fconfig.Exists("/invalidate tex"))
	{
		fconfig.SetPath("/invalidate tex");
		fconfig.Read("value", &m_invalidate_tex);
	}
	//point volume mode
	if (fconfig.Exists("/point volume mode"))
	{
		fconfig.SetPath("/point volume mode");
		fconfig.Read("value", &m_point_volume_mode);
	}
	//ruler settings
	if (fconfig.Exists("/ruler"))
	{
		fconfig.SetPath("/ruler");
		fconfig.Read("use transf", &m_ruler_use_transf);
		fconfig.Read("relax f1", &m_ruler_relax_f1);
		fconfig.Read("infr", &m_ruler_infr);
		fconfig.Read("df_f", &m_ruler_df_f);
		fconfig.Read("relax iter", &m_ruler_relax_iter);
		fconfig.Read("auto relax", &m_ruler_auto_relax);
		fconfig.Read("relax type", &m_ruler_relax_type);
		fconfig.Read("size thresh", &m_ruler_size_thresh);
	}
	//background paramters
	if (fconfig.Exists("/bg int"))
	{
		fconfig.SetPath("/bg int");
		fconfig.Read("type", &m_bg_type);
		fconfig.Read("kx", &m_kx);
		fconfig.Read("ky", &m_ky);
		fconfig.Read("varth", &m_varth);
		fconfig.Read("gauth", &m_gauth);
	}
	//flags for pvxml flipping
	if (fconfig.Exists("/pvxml"))
	{
		fconfig.SetPath("/pvxml");
		fconfig.Read("flip_x", &m_pvxml_flip_x);
		fconfig.Read("flip_y", &m_pvxml_flip_y);
		fconfig.Read("seq_type", &m_pvxml_seq_type);
	}
	//pixel format
	if (fconfig.Exists("/pixel format"))
	{
		fconfig.SetPath("/pixel format");
		fconfig.Read("api_type", &m_api_type);
		fconfig.Read("red_bit", &m_red_bit);
		fconfig.Read("green_bit", &m_green_bit);
		fconfig.Read("blue_bit", &m_blue_bit);
		fconfig.Read("alpha_bit", &m_alpha_bit);
		fconfig.Read("depth_bit", &m_depth_bit);
		fconfig.Read("samples", &m_samples);
	}
	//tracking settings
	if (fconfig.Exists("/tracking"))
	{
		fconfig.SetPath("/tracking");
		fconfig.Read("track_iter", &m_track_iter);
		fconfig.Read("component_size", &m_component_size);
		fconfig.Read("consistent_color", &m_consistent_color);
		fconfig.Read("try_merge", &m_try_merge);
		fconfig.Read("try_split", &m_try_split);
		fconfig.Read("contact_factor", &m_contact_factor);
		fconfig.Read("similarity", &m_similarity);
	}
	//context attrib
	if (fconfig.Exists("/context attrib"))
	{
		fconfig.SetPath("/context attrib");
		fconfig.Read("gl_major_ver", &m_gl_major_ver);
		fconfig.Read("gl_minor_ver", &m_gl_minor_ver);
		fconfig.Read("gl_profile_mask", &m_gl_profile_mask);
	}
	//max texture size
	if (fconfig.Exists("/max texture size"))
	{
		fconfig.SetPath("/max texture size");
		fconfig.Read("use_max_texture_size", &m_use_max_texture_size);
		fconfig.Read("max_texture_size", &m_max_texture_size);
	}
	//no tex pack
	if (fconfig.Exists("/no tex pack"))
	{
		fconfig.SetPath("/no tex pack");
		fconfig.Read("no_tex_pack", &m_no_tex_pack);
	}
	//cl device
	if (fconfig.Exists("/cl device"))
	{
		fconfig.SetPath("/cl device");
		fconfig.Read("platform_id", &m_cl_platform_id);
		fconfig.Read("device_id", &m_cl_device_id);
	}
	//clipping plane display mode
	if (fconfig.Exists("/clipping planes"))
	{
		fconfig.SetPath("/clipping planes");
		fconfig.Read("mode", &m_plane_mode);
	}

	// java paths load.
	if (fconfig.Exists("/Java")) 
	{
		fconfig.SetPath("/Java");
		fconfig.Read("jvm_path", &m_jvm_path);
		fconfig.Read("ij_path", &m_ij_path);
		fconfig.Read("bioformats_path", &m_bioformats_path);
		fconfig.Read("ij_mode", &m_ij_mode);
	}
    
	UpdateUI();
}

void SettingDlg::UpdateUI()
{
	//update user interface
	//save project
	m_prj_save_chk->SetValue(m_prj_save);
	//realtime compression
	m_realtime_cmp_chk->SetValue(m_realtime_compress);
	//mouse interactions
	m_mouse_int_chk->SetValue(m_mouse_int);
	//depth peeling
	m_peeling_layers_sldr->SetValue(m_peeling_layers);
	m_peeling_layers_text->ChangeValue(wxString::Format("%d", m_peeling_layers));
	//micro blending
	m_micro_blend_chk->SetValue(m_micro_blend);
	//shadow direction
	if (m_shadow_dir)
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
		m_shadow_dir_x = 0.0;
		m_shadow_dir_y = 0.0;
	}
	double deg = GetShadowDir();
	m_shadow_dir_sldr->SetValue(int(deg + 0.5));
	m_shadow_dir_text->ChangeValue(wxString::Format("%.2f", deg));
	//rot center anchor thresh
	m_pin_threshold_sldr->SetValue(int(m_pin_threshold*10.0));
	m_pin_threshold_text->ChangeValue(wxString::Format("%.0f", m_pin_threshold*100.0));
	//gradient background
	m_grad_bg_chk->SetValue(m_grad_bg);
	//stereo
	m_stereo_chk->SetValue(m_stereo);
	m_eye_dist_sldr->SetValue(int(m_eye_dist*10.0));
	m_eye_dist_text->ChangeValue(wxString::Format("%.1f", m_eye_dist));
	//override vox
	m_override_vox_chk->SetValue(m_override_vox);
	//wavelength to color
	m_wav_color1_cmb->Select(m_wav_color1 - 1);
	m_wav_color2_cmb->Select(m_wav_color2 - 1);
	m_wav_color3_cmb->Select(m_wav_color3 - 1);
	m_wav_color4_cmb->Select(m_wav_color4 - 1);
	//max texture size
	m_max_texture_size_chk->SetValue(m_use_max_texture_size);
	if (m_use_max_texture_size)
	{
		flvr::ShaderProgram::set_max_texture_size(m_max_texture_size);
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", m_max_texture_size));
		m_max_texture_size_text->Enable();
	}
	else
	{
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", flvr::ShaderProgram::
				max_texture_size()));
		m_max_texture_size_text->Disable();
	}
	//no tex pack
	flvr::ShaderProgram::set_no_tex_upack(m_no_tex_pack);
	//font
	wxString str = m_font_file.BeforeLast('.');
	int font_sel = m_font_cmb->FindString(str);
	if (font_sel != wxNOT_FOUND)
		m_font_cmb->Select(font_sel);
	m_font_size_cmb->SetValue(wxString::Format("%d", m_text_size));
	m_text_color_cmb->Select(m_text_color);
	//line width
	m_line_width_text->SetValue(wxString::Format("%.0f", m_line_width));
	m_line_width_sldr->SetValue(int(m_line_width+0.5));
	//paint history depth
	m_paint_hist_depth_text->ChangeValue(wxString::Format("%d", m_paint_hist_depth));
	m_paint_hist_depth_sldr->SetValue(m_paint_hist_depth);
	//memory settings
	m_streaming_chk->SetValue(m_mem_swap);
	EnableStreaming(m_mem_swap);
	m_update_order_rbox->SetSelection(m_update_order);
	m_graphics_mem_text->ChangeValue(wxString::Format("%d", (int)m_graphics_mem));
	m_graphics_mem_sldr->SetValue((int)(m_graphics_mem / 100.0));
	m_large_data_text->ChangeValue(wxString::Format("%d", (int)m_large_data_size));
	m_large_data_sldr->SetValue((int)(m_large_data_size / 10.0));
	m_block_size_text->ChangeValue(wxString::Format("%d", m_force_brick_size));
	m_block_size_sldr->SetValue(int(log(m_force_brick_size) / log(2.0) + 0.5));
	m_response_time_text->ChangeValue(wxString::Format("%d", m_up_time));
	m_response_time_sldr->SetValue(int(m_up_time / 10.0));
	m_detail_level_offset_text->ChangeValue(wxString::Format("%d", -m_detail_level_offset));
	m_detail_level_offset_sldr->SetValue(-m_detail_level_offset);

	//java
	m_java_jvm_text->SetValue(m_jvm_path);
	m_java_ij_text->SetValue(m_ij_path);
	m_java_bioformats_text->SetValue(m_bioformats_path);
	switch (m_ij_mode)
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

void SettingDlg::SaveSettings()
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "fluorender.set";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	fconfig.Write("ver_major", VERSION_MAJOR_TAG);
	fconfig.Write("ver_minor", VERSION_MINOR_TAG);

	fconfig.SetPath("/gm calculation");
	fconfig.Write("mode", m_gmc_mode);

	fconfig.SetPath("/peeling layers");
	fconfig.Write("value", m_peeling_layers);

	fconfig.SetPath("/micro blend");
	fconfig.Write("mode", m_micro_blend);

	fconfig.SetPath("/save project");
	fconfig.Write("mode", m_prj_save);

	fconfig.SetPath("/dpi");
	fconfig.Write("value", m_dpi);

	fconfig.SetPath("/save alpha");
	fconfig.Write("mode", m_save_alpha);

	fconfig.SetPath("/save float");
	fconfig.Write("mode", m_save_float);

	fconfig.SetPath("/realtime compress");
	fconfig.Write("mode", m_realtime_compress);

	fconfig.SetPath("/skip bricks");
	fconfig.Write("mode", m_skip_bricks);

	fconfig.SetPath("/mouse int");
	fconfig.Write("mode", m_mouse_int);

	fconfig.SetPath("/dir shadow");
	fconfig.Write("mode", m_shadow_dir);
	fconfig.Write("x", m_shadow_dir_x);
	fconfig.Write("y", m_shadow_dir_y);

	fconfig.SetPath("/pin threshold");
	fconfig.Write("value", m_pin_threshold);

	fconfig.SetPath("/stereo");
	fconfig.Write("enable_stereo", m_stereo);
	fconfig.Write("eye dist", m_eye_dist);

	fconfig.SetPath("/test mode");
	fconfig.Write("speed", m_test_speed);
	fconfig.Write("param", m_test_param);
	fconfig.Write("wiref", m_test_wiref);

	fconfig.SetPath("/wavelength to color");
	fconfig.Write("c1", m_wav_color1);
	fconfig.Write("c2", m_wav_color2);
	fconfig.Write("c3", m_wav_color3);
	fconfig.Write("c4", m_wav_color4);

	fconfig.SetPath("/time id");
	fconfig.Write("value", m_time_id);

	fconfig.SetPath("/grad bg");
	fconfig.Write("value", m_grad_bg);

	fconfig.SetPath("/override vox");
	fconfig.Write("value", m_override_vox);

	fconfig.SetPath("/soft threshold");
	fconfig.Write("value", m_soft_threshold);

	fconfig.SetPath("/save cmp");
	fconfig.Write("value", VRenderFrame::GetCompression());

	fconfig.SetPath("/run script");
	fconfig.Write("value", m_run_script);
	fconfig.Write("file", m_script_file);

	fconfig.SetPath("/paint history");
	fconfig.Write("value", m_paint_hist_depth);

	fconfig.SetPath("/text font");
	fconfig.Write("file", m_font_file);
	fconfig.Write("value", m_text_size);
	fconfig.Write("color", m_text_color);

	fconfig.SetPath("/line width");
	fconfig.Write("value", m_line_width);

	//full screen
	fconfig.SetPath("/full screen");
	fconfig.Write("stay top", m_stay_top);
	fconfig.Write("show cursor", m_show_cursor);

	//last tool
	fconfig.SetPath("/last tool");
	fconfig.Write("value", m_last_tool);

	//components
	fconfig.SetPath("/tracking");
	fconfig.Write("track_iter", m_track_iter);
	fconfig.Write("component_size", m_component_size);
	fconfig.Write("consistent_color", m_consistent_color);
	fconfig.Write("try_merge", m_try_merge);
	fconfig.Write("try_split", m_try_split);
	fconfig.Write("contact_factor", m_contact_factor);
	fconfig.Write("similarity", m_similarity);

	//memory settings
	fconfig.SetPath("/memory settings");
	fconfig.Write("mem swap", m_mem_swap);
	fconfig.Write("graphics mem", m_graphics_mem);
	fconfig.Write("large data size", m_large_data_size);
	fconfig.Write("force brick size", m_force_brick_size);
	fconfig.Write("up time", m_up_time);
	fconfig.Write("detail level offset", m_detail_level_offset);
	EnableStreaming(m_mem_swap);

	//update order
	fconfig.SetPath("/update order");
	fconfig.Write("value", m_update_order);

	//invalidate texture
	fconfig.SetPath("/invalidate tex");
	fconfig.Write("value", m_invalidate_tex);

	//point volume mode
	fconfig.SetPath("/point volume mode");
	fconfig.Write("value", m_point_volume_mode);

	//ruler settings
	fconfig.SetPath("/ruler");
	fconfig.Write("use transf", m_ruler_use_transf);
	fconfig.Write("relax f1", m_ruler_relax_f1);
	fconfig.Write("infr", m_ruler_infr);
	fconfig.Write("df_f", m_ruler_df_f);
	fconfig.Write("relax iter", m_ruler_relax_iter);
	fconfig.Write("auto relax", m_ruler_auto_relax);
	fconfig.Write("relax type", m_ruler_relax_type);
	fconfig.Write("size thresh", m_ruler_size_thresh);

	//background intensity
	fconfig.SetPath("/bg int");
	fconfig.Write("type", m_bg_type);
	fconfig.Write("kx", m_kx);
	fconfig.Write("ky", m_ky);
	fconfig.Write("varth", m_varth);
	fconfig.Write("gauth", m_gauth);

	//flags for flipping pvxml
	fconfig.SetPath("/pvxml");
	fconfig.Write("flip_x", m_pvxml_flip_x);
	fconfig.Write("flip_y", m_pvxml_flip_y);
	fconfig.Write("seq_type", m_pvxml_seq_type);

	//pixel format
	fconfig.SetPath("/pixel format");
	fconfig.Write("api_type", m_api_type);
	fconfig.Write("red_bit", m_red_bit);
	fconfig.Write("green_bit", m_green_bit);
	fconfig.Write("blue_bit", m_blue_bit);
	fconfig.Write("alpha_bit", m_alpha_bit);
	fconfig.Write("depth_bit", m_depth_bit);
	fconfig.Write("samples", m_samples);

	//context attrib
	fconfig.SetPath("/context attrib");
	fconfig.Write("gl_major_ver", m_gl_major_ver);
	fconfig.Write("gl_minor_ver", m_gl_minor_ver);
	fconfig.Write("gl_profile_mask", m_gl_profile_mask);

	//max texture size
	fconfig.SetPath("/max texture size");
	fconfig.Write("use_max_texture_size", m_use_max_texture_size);
	fconfig.Write("max_texture_size", m_max_texture_size);

	//no tex pack
	fconfig.SetPath("/no tex pack");
	fconfig.Write("no_tex_pack", m_no_tex_pack);

	//cl device
	fconfig.SetPath("/cl device");
	fconfig.Write("platform_id", m_cl_platform_id);
	fconfig.Write("device_id", m_cl_device_id);

	// java paths
	fconfig.SetPath("/Java");	
 	fconfig.Write("jvm_path", getJVMPath());
	fconfig.Write("ij_path", getIJPath());
	fconfig.Write("bioformats_path", getBioformatsPath());
	fconfig.Write("ij_mode", getIJMode());

	//clipping plane mode
	fconfig.SetPath("/clipping planes");
	if (m_frame && m_frame->GetClippingView())
		m_plane_mode = m_frame->GetClippingView()->GetPlaneMode();
	fconfig.Write("mode", m_plane_mode);

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "fluorender.set";
	SaveConfig(fconfig, dft);
}

void SettingDlg::UpdateTextureSize()
{
	if (!m_use_max_texture_size)
	{
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", flvr::ShaderProgram::
				max_texture_size()));
	}
	else
		flvr::ShaderProgram::set_max_texture_size(m_max_texture_size);
}

bool SettingDlg::GetTestMode(int type)
{
	switch (type)
	{
	case 1:	//speed test
		return m_test_speed;
	case 2:	//param test
		return m_test_param;
	case 3:	//wireframe mode
		return m_test_wiref;
	default:
		return false;
	}
}

int SettingDlg::GetPeelingLyers()
{
	return m_peeling_layers;
}

bool SettingDlg::GetMicroBlend()
{
	return m_micro_blend;
}

void SettingDlg::GetShadowDir(double& x, double &y)
{
	x = m_shadow_dir_x;
	y = m_shadow_dir_y;
}

//events
void SettingDlg::OnSave(wxCommandEvent &event)
{
	SaveSettings();
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
	if (m_prj_save_chk->GetValue())
		m_prj_save = true;
	else
		m_prj_save = false;
}

void SettingDlg::OnRealtimeCompressCheck(wxCommandEvent &event)
{
	if (m_realtime_cmp_chk->GetValue())
		m_realtime_compress = true;
	else
		m_realtime_compress = false;

	VRenderFrame::SetRealtimeCompression(m_realtime_compress);
}

void SettingDlg::OnMouseIntCheck(wxCommandEvent &event)
{
	if (m_mouse_int_chk->GetValue())
		m_mouse_int = true;
	else
		m_mouse_int = false;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
			{
				view->SetAdaptive(m_mouse_int);
				view->RefreshGL(39);
			}
		}
	}
}

void SettingDlg::OnPeelingLayersChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
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
	m_peeling_layers_sldr->SetValue(ival);
	m_peeling_layers = ival;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
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
	if (m_micro_blend_chk->GetValue())
		m_micro_blend = true;
	else
		m_micro_blend = false;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
			{
				view->SetBlendSlices(m_micro_blend);
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
		m_shadow_dir = true;
	}
	else
	{
		m_shadow_dir_sldr->Disable();
		m_shadow_dir_text->Disable();
		m_shadow_dir_x = 0.0;
		m_shadow_dir_y = 0.0;
		m_shadow_dir = false;
	}

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
				view->RefreshGL(39);
		}
	}
}

void SettingDlg::OnShadowDirChange(wxScrollEvent &event)
{
	double deg = double(m_shadow_dir_sldr->GetValue());
	wxString str = wxString::Format("%.2f", deg);
	if (str != m_shadow_dir_text->GetValue())
		m_shadow_dir_text->SetValue(str);
}

void SettingDlg::OnShadowDirEdit(wxCommandEvent &event)
{
	wxString str = m_shadow_dir_text->GetValue();
	double deg;
	str.ToDouble(&deg);
	m_shadow_dir_sldr->SetValue(int(deg));
	SetShadowDir(deg);

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
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
		m_frame->RefreshVRenderViews();
	}
}

void SettingDlg::SetShadowDir(double deg)
{
	m_shadow_dir_x = cos(d2r(deg));
	m_shadow_dir_y = sin(d2r(deg));
}

double SettingDlg::GetShadowDir()
{
	double deg = r2d(atan2(m_shadow_dir_y, m_shadow_dir_x));
	return deg;
}

//gradient background
void SettingDlg::OnGradBgCheck(wxCommandEvent &event)
{
	if (m_grad_bg_chk->GetValue())
		m_grad_bg = true;
	else
		m_grad_bg = false;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
			{
				view->SetGradBg(m_grad_bg);
				view->RefreshGL(39);
			}
		}
	}
}

// Get jvm paths.
wxString SettingDlg::getJVMPath() {
	return m_java_jvm_text->GetValue(); 
}
wxString SettingDlg::getIJPath() {
	return m_java_ij_text->GetValue(); 
}
wxString SettingDlg::getBioformatsPath() {
	return m_java_bioformats_text->GetValue(); 
}
std::vector<std::string> SettingDlg::GetJvmArgs() {
	std::vector<std::string> args;
	args.push_back(getJVMPath().ToStdString());
	args.push_back(getIJPath().ToStdString());
	args.push_back(getBioformatsPath().ToStdString());
	return args;
}


//rot center anchor thresh
void SettingDlg::OnPinThresholdChange(wxScrollEvent &event)
{
	double dval = double(m_pin_threshold_sldr->GetValue());
	wxString str = wxString::Format("%.0f", dval*10.0);
	if (str != m_pin_threshold_text->GetValue())
		m_pin_threshold_text->SetValue(str);
}

void SettingDlg::OnPinThresholdEdit(wxCommandEvent &event)
{
	wxString str = m_pin_threshold_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_pin_threshold_sldr->SetValue(int(dval/10.0));
	m_pin_threshold = dval / 100.0;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
				view->m_vrv->m_pin_scale_thresh = m_pin_threshold;
		}
	}
}

//link rotations
void SettingDlg::OnRotLink(wxCommandEvent& event)
{
	bool linked_rot = m_rot_link_chk->GetValue();
	VRenderGLView::m_linked_rot = linked_rot;
	VRenderGLView::m_master_linked_view = 0;

	if (m_frame && 0 < m_frame->GetViewNum())
	{
		VRenderGLView* view = m_frame->GetView(0);
		if (view)
			view->RefreshGL(39);
	}
}

//stereo
void SettingDlg::OnStereoCheck(wxCommandEvent &event)
{
	m_stereo = m_stereo_chk->GetValue();
	if (m_frame && 0 < m_frame->GetViewNum())
	{
		VRenderGLView* view = m_frame->GetView(0);
		if (view)
		{
			view->SetStereo(m_stereo);
			view->RefreshGL(39);
		}
	}
}

void SettingDlg::OnEyeDistChange(wxScrollEvent &event)
{
	m_eye_dist = double(m_eye_dist_sldr->GetValue()) / 10.0;
	wxString str = wxString::Format("%.1f", m_eye_dist);
	if (str != m_eye_dist_text->GetValue())
		m_eye_dist_text->SetValue(str);
}

void SettingDlg::OnEyeDistEdit(wxCommandEvent &event)
{
	wxString str = m_eye_dist_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_eye_dist_sldr->SetValue(int(dval * 10.0));
	m_eye_dist = dval;

	if (m_frame && 0 < m_frame->GetViewNum())
	{
		VRenderGLView* view = m_frame->GetView(0);
		if (view)
		{
			view->SetEyeDist(m_eye_dist);
			view->RefreshGL(39);
		}
	}
}

//override vox
void SettingDlg::OnOverrideVoxCheck(wxCommandEvent &event)
{
	if (m_override_vox_chk->GetValue())
		m_override_vox = true;
	else
		m_override_vox = false;

	if (m_frame)
		m_frame->GetDataManager()->SetOverrideVox(m_override_vox);
}

//wavelength to color
int SettingDlg::GetWavelengthColor(int n)
{
	switch (n)
	{
	case 1:
		return m_wav_color1;
	case 2:
		return m_wav_color2;
	case 3:
		return m_wav_color3;
	case 4:
		return m_wav_color4;
	default:
		return 0;
	}
}

void SettingDlg::OnWavColor1Change(wxCommandEvent &event)
{
	if (m_wav_color1_cmb)
		m_wav_color1 = m_wav_color1_cmb->GetCurrentSelection() + 1;

	if (m_frame && m_frame->GetDataManager())
		m_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
}

void SettingDlg::OnWavColor2Change(wxCommandEvent &event)
{
	if (m_wav_color2_cmb)
		m_wav_color2 = m_wav_color2_cmb->GetCurrentSelection() + 1;

	if (m_frame && m_frame->GetDataManager())
		m_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
}

void SettingDlg::OnWavColor3Change(wxCommandEvent &event)
{
	if (m_wav_color3_cmb)
		m_wav_color3 = m_wav_color3_cmb->GetCurrentSelection() + 1;

	if (m_frame && m_frame->GetDataManager())
		m_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
}

void SettingDlg::OnWavColor4Change(wxCommandEvent &event)
{
	if (m_wav_color4_cmb)
		m_wav_color4 = m_wav_color4_cmb->GetCurrentSelection() + 1;

	if (m_frame && m_frame->GetDataManager())
		m_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
}

//texture size
void SettingDlg::OnMaxTextureSizeChk(wxCommandEvent &event)
{
	m_use_max_texture_size = m_max_texture_size_chk->GetValue();
	if (m_use_max_texture_size)
	{
		flvr::ShaderProgram::set_max_texture_size(m_max_texture_size);
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", m_max_texture_size));
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
	if (m_use_max_texture_size)
	{
		wxString str = m_max_texture_size_text->GetValue();
		long size;
		if (str.ToLong(&size))
		{
			m_max_texture_size = size;
			flvr::ShaderProgram::set_max_texture_size(size);
		}
	}
}

//memory settings
void SettingDlg::OnStreamingChk(wxCommandEvent &event)
{
	if (m_streaming_chk->GetValue())
		m_mem_swap = true;
	else
		m_mem_swap = false;
	EnableStreaming(m_mem_swap);
}

void SettingDlg::OnUpdateOrderChange(wxCommandEvent &event)
{
	m_update_order = m_update_order_rbox->GetSelection();
}

void SettingDlg::OnGraphicsMemChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
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
	m_graphics_mem_sldr->SetValue(int(val / 100.0));
	m_graphics_mem = val;
}

void SettingDlg::OnLargeDataChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
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
	m_large_data_sldr->SetValue(int(val / 10.0));
	m_large_data_size = val;
}

void SettingDlg::OnBlockSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
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
	m_block_size_sldr->SetValue(int(log(val) / log(2.0) + 0.5));
	m_force_brick_size = val;
}

void SettingDlg::OnResponseTimeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
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
	m_response_time_sldr->SetValue(int(val / 10.0));
	m_up_time = val;
}

void SettingDlg::OnDetailLevelOffsetChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_detail_level_offset_text->GetValue())
		m_detail_level_offset_text->SetValue(str);
}

void SettingDlg::OnDetailLevelOffsetEdit(wxCommandEvent &event)
{
	wxString str = m_detail_level_offset_text->GetValue();
	long val;
	str.ToLong(&val);
	m_detail_level_offset_sldr->SetValue(val);
	m_detail_level_offset = -val;

	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
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
		m_font_file = str + ".ttf";
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		wxString loc = exePath + GETSLASH() + "Fonts" +
			GETSLASH() + str + ".ttf";

		if (m_frame)
		{
			flvr::TextRenderer::text_texture_manager_.load_face(loc.ToStdString());
			flvr::TextRenderer::text_texture_manager_.SetSize(m_text_size);
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				VRenderGLView* view = m_frame->GetView(i);
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
		m_text_size = size;

		if (m_frame)
		{
			flvr::TextRenderer::text_texture_manager_.SetSize(m_text_size);
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				VRenderGLView* view = m_frame->GetView(i);
				if (view)
					view->RefreshGL(39);
			}
		}
	}
}

void SettingDlg::OnTextColorChange(wxCommandEvent &event)
{
	m_text_color = m_text_color_cmb->GetCurrentSelection();
	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			VRenderGLView* view = m_frame->GetView(i);
			if (view)
				view->RefreshGL(39);
		}
	}
}

//line width
void SettingDlg::OnLineWidthSldr(wxScrollEvent &event)
{
	int ival = event.GetPosition();
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
		m_line_width_sldr->SetValue(ival);
		m_line_width = ival;
		if (m_frame)
		{
			for (int i = 0; i < m_frame->GetViewNum(); i++)
			{
				VRenderGLView* view = m_frame->GetView(i);
				if (view)
					view->RefreshGL(39);
			}
		}
	}
}

//paint history depth
void SettingDlg::OnPaintHistDepthChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_paint_hist_depth_text->GetValue())
		m_paint_hist_depth_text->SetValue(str);
}

void SettingDlg::OnPaintHistDepthEdit(wxCommandEvent &event)
{
	wxString str = m_paint_hist_depth_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	m_paint_hist_depth_sldr->SetValue(ival);
	m_paint_hist_depth = ival;
	if (m_frame)
		m_frame->SetTextureUndos();
}

// Java settings.
void SettingDlg::OnJavaJvmEdit(wxCommandEvent &event)
{
	wxString str = m_java_jvm_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
}

void SettingDlg::OnJavaIJEdit(wxCommandEvent &event)
{
	wxString str = m_java_ij_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
}

void SettingDlg::OnJavaBioformatsEdit(wxCommandEvent &event)
{
	wxString str = m_java_bioformats_text->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
}

void SettingDlg::onJavaJvmBrowse(wxCommandEvent &event)
{
#ifdef _WIN32
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose the jvm dll file",
		"", "", "*.dll", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#else
    wxFileDialog *fopendlg = new wxFileDialog(
                                              m_frame, "Choose the libjvm.dylib file",
                                              "", "", "*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif
    
	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_java_jvm_text->SetValue(filename);
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
    wxFileDialog *fopendlg = new wxFileDialog(
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
		m_java_ij_text->SetValue(filename);
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
		m_java_bioformats_text->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void SettingDlg::onJavaRadioButtonImageJ(wxCommandEvent &event) {
	m_java_jvm_text->Enable(true);
	m_java_bioformats_text->Enable(true);
	m_browse_jvm_btn->Enable(true);
	m_browse_bioformats_btn->Enable(true);
	m_ij_mode = 0;
}

void SettingDlg::onJavaRadioButtonFiji(wxCommandEvent &event) {
	m_java_jvm_text->Enable(false);	
	m_java_bioformats_text->Enable(false);
	m_browse_jvm_btn->Enable(false);
	m_browse_bioformats_btn->Enable(false);
	m_ij_mode = 1;
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
					SetCLPlatformID(i);
					SetCLDeviceID(j);
				}
				dvitem = m_device_tree->GetNextChild(pfitem, dvck);
				j++;
			}
			pfitem = m_device_tree->GetNextChild(root, pfck);
			i++;
		}
	}
}