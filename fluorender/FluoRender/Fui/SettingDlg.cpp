/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
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
		wxDefaultPosition, wxSize(50, -1), 0, NULL, wxCB_READONLY);
	for (int font_size = 10; font_size < 31; font_size += 2)
		m_font_size_cmb->Append(wxString::Format("%d", font_size));
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

SettingDlg::SettingDlg(RenderFrame *frame) :
	wxPanel(frame, wxID_ANY,
		wxDefaultPosition, wxSize(450, 750),
		0, "SettingDlg"),
	m_frame(frame)
{
	m_agent = glbin_agtf->addSettingAgent(gstSettingAgent, *this);
	m_agent->setObject(glbin_root);

	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

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
}

SettingDlg::~SettingDlg()
{
}

//events
void SettingDlg::OnSave(wxCommandEvent &event)
{
	m_agent->SaveSettings();
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
	bool bval = m_prj_save_chk->GetValue();
	m_agent->setValue(gstSaveProjectEnable, bval);
}

void SettingDlg::OnRealtimeCompressCheck(wxCommandEvent &event)
{
	bool bval = m_realtime_cmp_chk->GetValue();
	m_agent->setValue(gstHardwareCompress, bval);

	//RenderFrame::SetRealtimeCompression(m_realtime_compress);
}

void SettingDlg::OnMouseIntCheck(wxCommandEvent &event)
{
	bool bval = m_mouse_int_chk->GetValue();
	m_agent->setValue(gstAdaptive, bval);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstAdaptive, m_mouse_int);
	//	//view->RefreshGL(39);
	//}
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
	m_agent->setValue(gstPeelNum, ival);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstPeelNum, ival);
	//	//view->RefreshGL(39);
	//}
}

void SettingDlg::OnMicroBlendCheck(wxCommandEvent &event)
{
	bool bval = m_micro_blend_chk->GetValue();
	m_agent->setValue(gstMicroBlendEnable, bval);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstMicroBlendEnable, m_micro_blend);
	//	//view->RefreshGL(39);
	//}
}

//shadow direction
void SettingDlg::OnShadowDirCheck(wxCommandEvent &event)
{
	bool bval = m_shadow_dir_chk->GetValue();
	m_shadow_dir_sldr->Enable(bval);
	m_shadow_dir_text->Enable(bval);
	m_agent->setValue(gstShadowDirEnable, bval);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstShadowDirEnable, m_shadow_dir);
	//	view->setValue(gstShadowDirX, m_shadow_dir_x);
	//	view->setValue(gstShadowDirY, m_shadow_dir);
	//	//view->RefreshGL(39);
	//}
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
	m_agent->setValue(gstShadowDirX, cos(fluo::d2r(deg)));
	m_agent->setValue(gstShadowDirY, sin(fluo::d2r(deg)));

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstShadowDirEnable, m_shadow_dir);
	//	view->setValue(gstShadowDirX, m_shadow_dir_x);
	//	view->setValue(gstShadowDirY, m_shadow_dir);
	//	//view->RefreshGL(39);
	//}
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
	//if (m_frame)
	//{
	//	m_frame->SetTextureRendererSettings();
	//	m_frame->RefreshVRenderViews();
	//}
}

//gradient background
void SettingDlg::OnGradBgCheck(wxCommandEvent &event)
{
	bool bval = m_grad_bg_chk->GetValue();
	m_agent->setValue(gstGradBg, bval);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstGradBg, m_grad_bg);
	//	//view->RefreshGL(39);
	//}
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
	m_agent->setValue(gstPinThresh, dval);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstPinThresh, m_pin_threshold);
	//}
}

//link rotations
void SettingDlg::OnRotLink(wxCommandEvent& event)
{
	//bool linked_rot = m_rot_link_chk->GetValue();
	//RenderCanvas::m_linked_rot = linked_rot;
	//RenderCanvas::m_master_linked_view = 0;

	//if (m_frame && 0 < m_frame->GetViewNum())
	//{
	//	RenderCanvas* view = m_frame->GetView(0);
	//	if (view)
	//		view->RefreshGL(39);
	//}
}

//stereo
void SettingDlg::OnStereoCheck(wxCommandEvent &event)
{
	bool bval = m_stereo_chk->GetValue();
	m_agent->setValue(gstVrEnable, bval);
	//fluo::Renderview* view = glbin_root->getChild(0)->asRenderview();
	//if (!view) return;
	//view->setValue(gstVrEnable, bval);
	//view->RefreshGL(39);
}

void SettingDlg::OnEyeDistChange(wxScrollEvent &event)
{
	double dval = double(m_eye_dist_sldr->GetValue()) / 10.0;
	wxString str = wxString::Format("%.1f", dval);
	if (str != m_eye_dist_text->GetValue())
		m_eye_dist_text->SetValue(str);
}

void SettingDlg::OnEyeDistEdit(wxCommandEvent &event)
{
	wxString str = m_eye_dist_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_eye_dist_sldr->SetValue(int(dval * 10.0));
	m_agent->setValue(gstVrEyeOffset, dval);
	//fluo::Renderview* view = glbin_root->getChild(0)->asRenderview();
	//if (!view) return;
	//view->setValue(gstVrEyeOffset, dval);
	//view->RefreshGL(39);
}

//override vox
void SettingDlg::OnOverrideVoxCheck(wxCommandEvent &event)
{
	bool bval = m_override_vox_chk->GetValue();
	m_agent->setValue(gstOverrideVoxSpc, bval);

	//if (m_frame)
	//	m_frame->GetDataManager()->SetOverrideVox(m_override_vox);
}

void SettingDlg::OnWavColor1Change(wxCommandEvent &event)
{
	long lval = m_wav_color1_cmb->GetCurrentSelection() + 1;
	m_agent->setValue(gstWaveColor1, lval);

	//if (m_frame && m_frame->GetDataManager())
	//	m_frame->GetDataManager()->SetWavelengthColor(
	//		m_wav_color1,
	//		m_wav_color2,
	//		m_wav_color3,
	//		m_wav_color4);
}

void SettingDlg::OnWavColor2Change(wxCommandEvent &event)
{
	long lval = m_wav_color2_cmb->GetCurrentSelection() + 1;
	m_agent->setValue(gstWaveColor2, lval);

	//if (m_frame && m_frame->GetDataManager())
	//	m_frame->GetDataManager()->SetWavelengthColor(
	//		m_wav_color1,
	//		m_wav_color2,
	//		m_wav_color3,
	//		m_wav_color4);
}

void SettingDlg::OnWavColor3Change(wxCommandEvent &event)
{
	long lval = m_wav_color3_cmb->GetCurrentSelection() + 1;
	m_agent->setValue(gstWaveColor3, lval);

	//if (m_frame && m_frame->GetDataManager())
	//	m_frame->GetDataManager()->SetWavelengthColor(
	//		m_wav_color1,
	//		m_wav_color2,
	//		m_wav_color3,
	//		m_wav_color4);
}

void SettingDlg::OnWavColor4Change(wxCommandEvent &event)
{
	long lval = m_wav_color4_cmb->GetCurrentSelection() + 1;
	m_agent->setValue(gstWaveColor4, lval);

	//if (m_frame && m_frame->GetDataManager())
	//	m_frame->GetDataManager()->SetWavelengthColor(
	//		m_wav_color1,
	//		m_wav_color2,
	//		m_wav_color3,
	//		m_wav_color4);
}

//texture size
void SettingDlg::OnMaxTextureSizeChk(wxCommandEvent &event)
{
	bool bval = m_max_texture_size_chk->GetValue();
	m_agent->setValue(gstMaxTextureSizeEnable, bval);
}

void SettingDlg::OnMaxTextureSizeEdit(wxCommandEvent &event)
{
	wxString str = m_max_texture_size_text->GetValue();
	long size;
	if (str.ToLong(&size))
		m_agent->setValue(gstMaxTextureSize, size);
}

//memory settings
void SettingDlg::OnStreamingChk(wxCommandEvent &event)
{
	bool bval = m_streaming_chk->GetValue();
	m_agent->setValue(gstStreamEnable, bval);
	EnableStreaming(bval);
}

void SettingDlg::OnUpdateOrderChange(wxCommandEvent &event)
{
	long lval = m_update_order_rbox->GetSelection();
	m_agent->setValue(gstStreamOrder, lval);
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
	m_agent->setValue(gstGpuMemSize, val);
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
	m_agent->setValue(gstLargeDataSize, val);
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
	m_agent->setValue(gstBrickSize, long(val));
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
	m_agent->setValue(gstResponseTime, long(val));
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
	m_agent->setValue(gstLodOffset, val);

	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstLevelOffset, val);
	//	//view->RefreshGL(39);
	//}
}

//font
void SettingDlg::OnFontChange(wxCommandEvent &event)
{
	wxString str = m_font_cmb->GetValue();
	if (str.IsEmpty()) return;
	std::string sval = str.ToStdString() + ".tff";
	m_agent->setValue(gstFontFile, sval);
}

void SettingDlg::OnFontSizeChange(wxCommandEvent &event)
{
	wxString str = m_font_size_cmb->GetValue();
	long size;
	if (str.ToLong(&size))
		m_agent->setValue(gstTextSize, double(size));
	//{
	//	m_text_size = size;

	//	flvr::TextRenderer::text_texture_manager_.SetSize(m_text_size);
	//	for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//	{
	//		fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//		if (!view) continue;
	//		view->setValue(gstTextSize, m_text_size);
	//		//view->RefreshGL(39);
	//	}
	//}
}

void SettingDlg::OnTextColorChange(wxCommandEvent &event)
{
	long lval = m_text_color_cmb->GetCurrentSelection();
	m_agent->setValue(gstTextColorMode, lval);
	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstTextColorMode, m_text_color);
	//	//view->RefreshGL(39);
	//}
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
	long ival;
	if (str.ToLong(&ival))
	{
		m_agent->setValue(gstLineWidth, double(ival));
		m_line_width_sldr->SetValue(ival);
		//m_line_width = ival;
		//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
		//{
		//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
		//	if (!view) continue;
		//	view->setValue(gstLineWidth, m_line_width);
		//	//view->RefreshGL(39);
		//}
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
	long ival;
	if (str.ToLong(&ival))
	{
		m_agent->setValue(gstPaintHistory, ival);
		m_paint_hist_depth_sldr->SetValue(ival);
	}
	//if (m_frame)
	//	m_frame->SetTextureUndos();
}

// Java settings.
void SettingDlg::OnJavaJvmEdit(wxCommandEvent &event)
{
	wxString str = m_java_jvm_text->GetValue();
	m_agent->setValue(gstJvmPath, str.ToStdString());
}

void SettingDlg::OnJavaIJEdit(wxCommandEvent &event)
{
	wxString str = m_java_ij_text->GetValue();
	m_agent->setValue(gstImagejPath, str.ToStdString());
}

void SettingDlg::OnJavaBioformatsEdit(wxCommandEvent &event)
{
	wxString str = m_java_bioformats_text->GetValue();
	m_agent->setValue(gstBioformatsPath, str.ToStdString());
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

void SettingDlg::onJavaRadioButtonImageJ(wxCommandEvent &event)
{
	m_java_jvm_text->Enable(true);
	m_java_bioformats_text->Enable(true);
	m_browse_jvm_btn->Enable(true);
	m_browse_bioformats_btn->Enable(true);
	m_agent->setValue(gstImagejMode, long(0));
}

void SettingDlg::onJavaRadioButtonFiji(wxCommandEvent &event) {
	m_java_jvm_text->Enable(false);	
	m_java_bioformats_text->Enable(false);
	m_browse_jvm_btn->Enable(false);
	m_browse_bioformats_btn->Enable(false);
	m_agent->setValue(gstImagejMode, long(1));
}

//device tree
void SettingDlg::OnSelChanged(wxTreeEvent& event)
{
	wxTreeItemId sel = m_device_tree->GetSelection();
	if (!sel.IsOk())
		return;
	int i = 0, j = 0;
	int pid = 0, did = 0;
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
					pid = i;
					did = j;
				}
				dvitem = m_device_tree->GetNextChild(pfitem, dvck);
				j++;
			}
			pfitem = m_device_tree->GetNextChild(root, pfck);
			i++;
		}
	}
	m_agent->setValue(gstClPlatformId, long(pid));
	m_agent->setValue(gstClDeviceId, long(did));
}