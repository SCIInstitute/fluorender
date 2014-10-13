/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "VRenderView.h"
#include <wx/valnum.h>
#include <wx/notebook.h>

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
	//link render views rotations
    EVT_CHECKBOX(ID_RotLinkChk, SettingDlg::OnRotLink)
	//override vox
	EVT_CHECKBOX(ID_OverrideVoxChk, SettingDlg::OnOverrideVoxCheck)
	//wavelength to color
	EVT_COMBOBOX(ID_WavColor1Cmb, SettingDlg::OnWavColor1Change)
	EVT_COMBOBOX(ID_WavColor2Cmb, SettingDlg::OnWavColor2Change)
	EVT_COMBOBOX(ID_WavColor3Cmb, SettingDlg::OnWavColor3Change)
	EVT_COMBOBOX(ID_WavColor4Cmb, SettingDlg::OnWavColor4Change)
	//memory settings
	EVT_CHECKBOX(ID_StreamingChk, SettingDlg::OnStreamingChk)
	EVT_COMMAND_SCROLL(ID_GraphicsMemSldr, SettingDlg::OnGraphicsMemChange)
	EVT_TEXT(ID_GraphicsMemText, SettingDlg::OnGraphicsMemEdit)
	EVT_COMMAND_SCROLL(ID_LargeDataSldr, SettingDlg::OnLargeDataChange)
	EVT_TEXT(ID_LargeDataText, SettingDlg::OnLargeDataEdit)
	EVT_COMMAND_SCROLL(ID_BlockSizeSldr, SettingDlg::OnBlockSizeChange)
	EVT_TEXT(ID_BlockSizeText, SettingDlg::OnBlockSizeEdit)
	EVT_COMMAND_SCROLL(ID_ResponseTimeSldr, SettingDlg::OnResponseTimeChange)
	EVT_TEXT(ID_ResponseTimeText, SettingDlg::OnResponseTimeEdit)
	//font
	EVT_COMBOBOX(ID_FontCmb, SettingDlg::OnFontChange)
	//script
	EVT_CHECKBOX(ID_RunScriptChk, SettingDlg::OnRunScriptChk)
	//show
	EVT_SHOW(SettingDlg::OnShow)
END_EVENT_TABLE()

wxWindow* SettingDlg::CreateProjectPage(wxWindow *parent)
{
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
		new wxStaticBox(page, wxID_ANY, "Font"), wxVERTICAL);
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Choose a font for the text in render views:");
	m_font_cmb = new wxComboBox(page, ID_FontCmb, "",
		wxDefaultPosition, wxSize(120, -1), 0, NULL, wxCB_READONLY);
	m_font_cmb->Append("Helvetica 12");
	m_font_cmb->Append("Helvetica 18");
	m_font_cmb->Append("Times Roman 10");
	m_font_cmb->Append("Times Roman 24");
	m_font_cmb->Select(0);
	sizer2_1->Add(st);
	sizer2_1->Add(20, 10);
	sizer2_1->Add(m_font_cmb);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);

	//script
	wxBoxSizer *group3 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "4D Script"), wxVERTICAL);
	m_run_script_chk = new wxCheckBox(page, ID_RunScriptChk,
		"Enable execution of a script on 4D data during playback.");
	st = new wxStaticText(page, 0,
		"Compose and save the text file \"script_4d.txt\" in the same folder as the\n"\
		"4D data. See details in the user manual for the syntax of the script.");
	group3->Add(10, 5);
	group3->Add(m_run_script_chk);
	group3->Add(10, 5);
	group3->Add(st);
	group3->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreateRenderingPage(wxWindow *parent)
{
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	vald_fp2.SetRange(-180.0, 180.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	vald_int.SetRange(1, 10);
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
		"Number of depth peeling layers for rendering transparent mesh objects.\n"\
			"Set higher numbers only for complex geometries.\n"\
			"It slows down the rendering speed.");
	sizer2_1->Add(m_peeling_layers_sldr, 1, wxEXPAND|wxALIGN_CENTER);
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
	sizer3_1->Add(m_shadow_dir_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer3_1->Add(m_shadow_dir_text, 0, wxALIGN_CENTER);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);
	group3->Add(st);
	group3->Add(10, 5);

	//gradient background
	wxBoxSizer *group4 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Gradient Background"), wxVERTICAL);
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	m_grad_bg_chk = new wxCheckBox(page, ID_GradBgChk,
		"Enable gradient background");
	sizer4_1->Add(m_grad_bg_chk, 0, wxALIGN_CENTER);
	group4->Add(10, 5);
	group4->Add(sizer4_1, 0, wxEXPAND);
	group4->Add(10, 5);
	//link rotations
	wxBoxSizer* group5 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Rotations"), wxVERTICAL);
	wxBoxSizer *sizer5_1 = new wxBoxSizer(wxHORIZONTAL);
	m_rot_link_chk = new wxCheckBox(page, ID_RotLinkChk,
		"Link all rendering views' rotations.");
	sizer5_1->Add(m_rot_link_chk, 0, wxALIGN_CENTER);
	group5->Add(10, 5);
	group5->Add(sizer5_1, 0, wxEXPAND);
	group5->Add(10, 5);
	// combine gradient and rotations checks
	wxBoxSizer* group4_5 = new wxBoxSizer(wxHORIZONTAL);
	group4_5->Add(group4, 0, wxEXPAND);
	group4_5->AddStretchSpacer();
	group4_5->Add(group5, 0, wxEXPAND);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);

	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group4_5, 0, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

wxWindow* SettingDlg::CreatePerformancePage(wxWindow *parent)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
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
	sizer2_1->Add(m_graphics_mem_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer2_1->Add(m_graphics_mem_text, 0, wxALIGN_CENTER);
	sizer2_1->Add(st);
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Large Data Size:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_2->Add(st);
	m_large_data_sldr = new wxSlider(page, ID_LargeDataSldr, 20, 5, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_large_data_text = new wxTextCtrl(page, ID_LargeDataText, "200",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	st = new wxStaticText(page, 0, "MB",
		wxDefaultPosition, wxSize(20, -1));
	sizer2_2->Add(m_large_data_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer2_2->Add(m_large_data_text, 0, wxALIGN_CENTER);
	sizer2_2->Add(st);
	wxBoxSizer *sizer2_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Block Size:",
		wxDefaultPosition, wxSize(110, -1));
	sizer2_3->Add(st);
	m_block_size_sldr = new wxSlider(page, ID_BlockSizeSldr, 7, 4, 11,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_block_size_text = new wxTextCtrl(page, ID_BlockSizeText, "128",
		wxDefaultPosition, wxSize(40, -1), 0, vald_int);
	st = new wxStaticText(page, 0, "vx",
		wxDefaultPosition, wxSize(20, -1));
	sizer2_3->Add(m_block_size_sldr, 1, wxEXPAND|wxALIGN_CENTER);
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
	sizer2_4->Add(m_response_time_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer2_4->Add(m_response_time_text, 0, wxALIGN_CENTER);
	sizer2_4->Add(st);
	group2->Add(10, 5);
	group2->Add(m_streaming_chk);
	group2->Add(10, 10);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_2, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_3, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_4, 0, wxEXPAND);
	group2->Add(10, 5);
	st = new wxStaticText(page, 0,
		"Data streaming allows rendering datasets of much larger size than available\n"\
		"graphics memory. Datasets are divided into small blocks. Then, these blocks\n"\
		"are loaded into graphics memory and rendered in sequence.\n"\
		"Restart is needed for the settings to take effect.");
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
	wxStaticText* st;
	wxPanel *page = new wxPanel(parent);

	//override vox
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(page,wxID_ANY, "Override Voxel Size"), wxVERTICAL);
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_override_vox_chk = new wxCheckBox(page, ID_OverrideVoxChk,
		"Get voxel size info from the first opened dataset.");
	sizer1_1->Add(m_override_vox_chk, 0, wxALIGN_CENTER);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);

	//wavelength to color
	wxBoxSizer *group2 = new wxStaticBoxSizer(
		new wxStaticBox(page, wxID_ANY, "Default Colors for Excitation Wavelengths (nm) (for OIB/OIF/LSM files)"), wxVERTICAL);
	//combo box line
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_wav_color1_cmb = new wxComboBox(page, ID_WavColor1Cmb, "",
		wxDefaultPosition, wxSize(70, 23), 0, NULL, wxCB_READONLY);
	m_wav_color2_cmb = new wxComboBox(page, ID_WavColor2Cmb, "",
		wxDefaultPosition, wxSize(70, 23), 0, NULL, wxCB_READONLY);
	m_wav_color3_cmb = new wxComboBox(page, ID_WavColor3Cmb, "",
		wxDefaultPosition, wxSize(70, 23), 0, NULL, wxCB_READONLY);
	m_wav_color4_cmb = new wxComboBox(page, ID_WavColor4Cmb, "",
		wxDefaultPosition, wxSize(70, 23), 0, NULL, wxCB_READONLY);
	//1
	m_wav_color1_cmb->Append("Red");
	m_wav_color1_cmb->Append("Green");
	m_wav_color1_cmb->Append("Blue");
	m_wav_color1_cmb->Append("Purple");
	m_wav_color1_cmb->Append("White");
	//2
	m_wav_color2_cmb->Append("Red");
	m_wav_color2_cmb->Append("Green");
	m_wav_color2_cmb->Append("Blue");
	m_wav_color2_cmb->Append("Purple");
	m_wav_color2_cmb->Append("White");
	//3
	m_wav_color3_cmb->Append("Red");
	m_wav_color3_cmb->Append("Green");
	m_wav_color3_cmb->Append("Blue");
	m_wav_color3_cmb->Append("Purple");
	m_wav_color3_cmb->Append("White");
	//4
	m_wav_color4_cmb->Append("Red");
	m_wav_color4_cmb->Append("Green");
	m_wav_color4_cmb->Append("Blue");
	m_wav_color4_cmb->Append("Purple");
	m_wav_color4_cmb->Append("White");
	sizer2_1->Add(30, 20);
	sizer2_1->Add(m_wav_color1_cmb);
	sizer2_1->Add(20, 20);
	sizer2_1->Add(m_wav_color2_cmb);
	sizer2_1->Add(20, 20);
	sizer2_1->Add(m_wav_color3_cmb);
	sizer2_1->Add(20, 20);
	sizer2_1->Add(m_wav_color4_cmb);
	sizer2_1->Add(30, 20);
	//static text line
	wxBoxSizer *sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	sizer2_2->Add(10, 20);
	st = new wxStaticText(page, 0, "350", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "450\n480\n488", wxDefaultPosition, wxSize(25, 50));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "543\n568", wxDefaultPosition, wxSize(25, 30));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "633", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "--", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	sizer2_2->Add(20, 20);
	st = new wxStaticText(page, 0, "700", wxDefaultPosition, wxSize(25, 20));
	sizer2_2->Add(st);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0);
	group2->Add(10, 5);
	group2->Add(sizer2_2, 0);
	group2->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);

	page->SetSizer(sizerV);
	return page;
}

SettingDlg::SettingDlg(wxWindow *frame, wxWindow *parent) :
wxPanel(parent, wxID_ANY,
		 wxDefaultPosition, wxSize(450, 750),
		 0, "SettingDlg"),
m_frame(frame)
{
	//notebook
	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	notebook->AddPage(CreateProjectPage(notebook), "Project");
	notebook->AddPage(CreateRenderingPage(notebook), "Rendering");
	notebook->AddPage(CreatePerformancePage(notebook), "Performance");
	notebook->AddPage(CreateFormatPage(notebook), "File format");

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
	m_realtime_compress = false;
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
	m_override_vox = true;
	m_soft_threshold = 0.0;
	m_run_script = false;
	m_text_font = BITMAP_FONT_TYPE_HELVETICA_12;
	m_mem_swap = false;
	m_graphics_mem = 1000.0;
	m_large_data_size = 1000.0;
	m_force_brick_size = 128;
	m_up_time = 100;
	m_update_order = 0;
	m_point_volume_mode = 0;
	m_ruler_use_transf = false;
	m_ruler_time_dep = true;
	m_pvxml_flip_x = false;
	m_pvxml_flip_y = false;

	wxFileInputStream is(SETTING_FILE_NAME);
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
	//realtime compression
	if (fconfig.Exists("/realtime compress"))
	{
		fconfig.SetPath("/realtime compress");
		fconfig.Read("mode", &m_realtime_compress, false);
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
		fconfig.Read("value", &m_run_script);
	}
	//text font
	if (fconfig.Exists("/text font"))
	{
		fconfig.SetPath("/text font");
		fconfig.Read("value", &m_text_font);
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
	}
	EnableStreaming(m_mem_swap);
	//update order
	if (fconfig.Exists("/update order"))
	{
		fconfig.SetPath("/update order");
		fconfig.Read("value", &m_update_order);
	}
	//point volume mode
	if (fconfig.Exists("/point volume mode"))
	{
		fconfig.SetPath("/point volume mode");
		fconfig.Read("value", &m_point_volume_mode);
	}
	//ruler use transfer function
	if (fconfig.Exists("/ruler use transf"))
	{
		fconfig.SetPath("/ruler use transf");
		fconfig.Read("value", &m_ruler_use_transf);
	}
	//ruler time dependent
	if (fconfig.Exists("/ruler time dep"))
	{
		fconfig.SetPath("/ruler time dep");
		fconfig.Read("value", &m_ruler_time_dep);
	}
	//flags for pvxml flipping
	if (fconfig.Exists("/pvxml flip"))
	{
		fconfig.SetPath("/pvxml flip");
		fconfig.Read("x", &m_pvxml_flip_x);
		fconfig.Read("y", &m_pvxml_flip_y);
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
	m_shadow_dir_sldr->SetValue(int(deg+0.5));
	m_shadow_dir_text->ChangeValue(wxString::Format("%.2f", deg));
	//gradient background
	m_grad_bg_chk->SetValue(m_grad_bg);
	//override vox
	m_override_vox_chk->SetValue(m_override_vox);
	//wavelength to color
	m_wav_color1_cmb->Select(m_wav_color1-1);
	m_wav_color2_cmb->Select(m_wav_color2-1);
	m_wav_color3_cmb->Select(m_wav_color3-1);
	m_wav_color4_cmb->Select(m_wav_color4-1);
	//font
	m_font_cmb->Select(m_text_font-3);
	//script
	m_run_script_chk->SetValue(m_run_script);
	//memory settings
	m_streaming_chk->SetValue(m_mem_swap);
	m_graphics_mem_text->SetValue(wxString::Format("%d", (int)m_graphics_mem));
	m_large_data_text->SetValue(wxString::Format("%d", (int)m_large_data_size));
	m_block_size_text->SetValue(wxString::Format("%d", m_force_brick_size));
	m_response_time_text->SetValue(wxString::Format("%d", m_up_time));
}

void SettingDlg::SaveSettings()
{
	wxFileConfig fconfig("FluoRender Settings");

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

	fconfig.SetPath("/realtime compress");
	fconfig.Write("mode", m_realtime_compress);

	fconfig.SetPath("/mouse int");
	fconfig.Write("mode", m_mouse_int);

	fconfig.SetPath("/dir shadow");
	fconfig.Write("mode", m_shadow_dir);
	fconfig.Write("x", m_shadow_dir_x);
	fconfig.Write("y", m_shadow_dir_y);

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

	fconfig.SetPath("/text font");
	fconfig.Write("value", m_text_font);

	//memory settings
	fconfig.SetPath("/memory settings");
	fconfig.Write("mem swap", m_mem_swap);
	fconfig.Write("graphics mem", m_graphics_mem);
	fconfig.Write("large data size", m_large_data_size);
	fconfig.Write("force brick size", m_force_brick_size);
	fconfig.Write("up time", m_up_time);

	//update order
	fconfig.SetPath("/update order");
	fconfig.Write("value", m_update_order);

	//point volume mode
	fconfig.SetPath("/point volume mode");
	fconfig.Write("value", m_point_volume_mode);

	//ruler use transfer function
	fconfig.SetPath("/ruler use transf");
	fconfig.Write("value", m_ruler_use_transf);

	//ruler time dependent
	fconfig.SetPath("/ruler time dependent");
	fconfig.Write("value", m_ruler_time_dep);

	//flags for flipping pvxml
	fconfig.SetPath("/pvxml flip");
	fconfig.Write("x", m_pvxml_flip_x);
	fconfig.Write("y", m_pvxml_flip_y);

	wxFileOutputStream os(SETTING_FILE_NAME);
	fconfig.Save(os);
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
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		vr_frame->ShowPane(this, false);
}

void SettingDlg::OnClose(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		vr_frame->ShowPane(this, false);
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
			{
				vrv->SetAdaptive(m_mouse_int);
				vrv->RefreshGL();
			}
		}
	}
}

void SettingDlg::OnPeelingLayersChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_peeling_layers_text->SetValue(str);
}

void SettingDlg::OnPeelingLayersEdit(wxCommandEvent &event)
{
	wxString str = m_peeling_layers_text->GetValue();
	long ival;
	str.ToLong(&ival);
	if (ival<=0)
		return;
	m_peeling_layers_sldr->SetValue(ival);
	m_peeling_layers = ival;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
			{
				vrv->SetPeelingLayers(ival);
				vrv->RefreshGL();
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
			{
				vrv->SetBlendSlices(m_micro_blend);
				vrv->RefreshGL();
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
				vrv->RefreshGL();
		}
	}
}

void SettingDlg::OnShadowDirChange(wxScrollEvent &event)
{
	double deg = double(m_shadow_dir_sldr->GetValue());
	wxString str = wxString::Format("%.2f", deg);
	m_shadow_dir_text->SetValue(str);
}

void SettingDlg::OnShadowDirEdit(wxCommandEvent &event)
{
	wxString str = m_shadow_dir_text->GetValue();
	double deg;
	str.ToDouble(&deg);
	m_shadow_dir_sldr->SetValue(int(deg));
	SetShadowDir(deg);

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
				vrv->RefreshGL();
		}
	}
}

void SettingDlg::EnableStreaming(bool enable)
{
	if (enable)
	{
		m_graphics_mem_sldr->Enable();
		m_graphics_mem_text->Enable();
		m_large_data_sldr->Enable();
		m_large_data_text->Enable();
		m_block_size_sldr->Enable();
		m_block_size_text->Enable();
		m_response_time_sldr->Enable();
		m_response_time_text->Enable();
	}
	else
	{
		m_graphics_mem_sldr->Disable();
		m_graphics_mem_text->Disable();
		m_large_data_sldr->Disable();
		m_large_data_text->Disable();
		m_block_size_sldr->Disable();
		m_block_size_text->Disable();
		m_response_time_sldr->Disable();
		m_response_time_text->Disable();
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
			{
				vrv->SetGradBg(m_grad_bg);
				vrv->RefreshGL();
			}
		}
	}
}

//link rotations
void SettingDlg::OnRotLink(wxCommandEvent& event)
{
	
   VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
   if (vr_frame && 0 < vr_frame->GetViewNum()) {
     VRenderView* view = vr_frame->GetView(0);
	 if (view) {
		view->OnRotLink(m_rot_link_chk->GetValue());
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->GetDataManager()->SetOverrideVox(m_override_vox);
	}
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

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetDataManager())
	{
		vr_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
	}
}

void SettingDlg::OnWavColor2Change(wxCommandEvent &event)
{
	if (m_wav_color2_cmb)
		m_wav_color2 = m_wav_color2_cmb->GetCurrentSelection() + 1;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetDataManager())
	{
		vr_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
	}
}

void SettingDlg::OnWavColor3Change(wxCommandEvent &event)
{
	if (m_wav_color3_cmb)
		m_wav_color3 = m_wav_color3_cmb->GetCurrentSelection() + 1;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetDataManager())
	{
		vr_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
	}
}

void SettingDlg::OnWavColor4Change(wxCommandEvent &event)
{
	if (m_wav_color4_cmb)
		m_wav_color4 = m_wav_color4_cmb->GetCurrentSelection() + 1;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame && vr_frame->GetDataManager())
	{
		vr_frame->GetDataManager()->SetWavelengthColor(
			m_wav_color1,
			m_wav_color2,
			m_wav_color3,
			m_wav_color4);
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

void SettingDlg::OnGraphicsMemChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival*100);
	m_graphics_mem_text->SetValue(str);
}

void SettingDlg::OnGraphicsMemEdit(wxCommandEvent &event)
{
	wxString str = m_graphics_mem_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val<=0.0)
		return;
	m_graphics_mem_sldr->SetValue(int(val/100.0));
	m_graphics_mem = val;
}

void SettingDlg::OnLargeDataChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival*10);
	m_large_data_text->SetValue(str);
}

void SettingDlg::OnLargeDataEdit(wxCommandEvent &event)
{
	wxString str = m_large_data_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val<=0.0)
		return;
	m_large_data_sldr->SetValue(int(val/10.0));
	m_large_data_size = val;
}

void SettingDlg::OnBlockSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", 2<<(ival-1));
	m_block_size_text->SetValue(str);
}

void SettingDlg::OnBlockSizeEdit(wxCommandEvent &event)
{
	wxString str = m_block_size_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val<=0.0)
		return;
	m_block_size_sldr->SetValue(int(log(val)/log(2.0)+0.5));
	m_force_brick_size = val;
}

void SettingDlg::OnResponseTimeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival*10);
	m_response_time_text->SetValue(str);
}

void SettingDlg::OnResponseTimeEdit(wxCommandEvent &event)
{
	wxString str = m_response_time_text->GetValue();
	double val;
	str.ToDouble(&val);
	if (val<=0.0)
		return;
	m_response_time_sldr->SetValue(int(val/10.0));
	m_up_time = val;
}

//font
void SettingDlg::OnFontChange(wxCommandEvent &event)
{
	switch (m_font_cmb->GetCurrentSelection())
	{
	case 0://helvetica 12
		m_text_font = 3;
		break;
	case 1://helvetica 18
		m_text_font = 4;
		break;
	case 2://roman 10
		m_text_font = 5;
		break;
	case 3://roman 24
		m_text_font = 6;
		break;
	default:
		break;
	}

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		for (int i=0 ; i<(int)vr_frame->GetViewList()->size() ; i++)
		{
			VRenderView* vrv = (*vr_frame->GetViewList())[i];
			if (vrv)
				vrv->RefreshGL();
		}
	}
}

//script
void SettingDlg::OnRunScriptChk(wxCommandEvent &event)
{
	m_run_script = m_run_script_chk->GetValue();
}
