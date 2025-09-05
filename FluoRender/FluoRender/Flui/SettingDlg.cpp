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
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <RenderViewPanel.h>
#include <ShaderProgram.h>
#include <KernelProgram.h>
#include <Texture.h>
#include <TextRenderer.h>
#include <wxSingleSlider.h>
#include <ModalDlg.h>
#include <wx/valnum.h>
#include <wx/notebook.h>
#include <wx/display.h>
#include <wx/valtext.h>
#include <wx/regex.h>

SettingDlg::SettingDlg(MainFrame *frame) :
	TabbedPanel(frame, frame,
		wxDefaultPosition,
		frame->FromDIP(wxSize(500, 620)),
		0, "SettingDlg")
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	Freeze();
	SetDoubleBuffered(true);

	//notebook
	m_notebook = new wxAuiNotebook(this, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE |
		wxAUI_NB_MULTILINE | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);
	m_notebook->AddPage(CreateProjectPage(m_notebook), "Project", true);
	m_notebook->AddPage(CreateRenderingPage(m_notebook), "Rendering");
	m_notebook->AddPage(CreatePerformancePage(m_notebook), "Performance");
	m_notebook->AddPage(CreateAutomationPage(m_notebook), "Automation");
	m_notebook->AddPage(CreateDisplayPage(m_notebook), "Display");
	m_notebook->AddPage(CreateFormatPage(m_notebook), "File format");
	m_notebook->AddPage(CreateJavaPage(m_notebook), "ImageJ Link");

	//interface
	wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(m_notebook, 1, wxEXPAND | wxALL);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);

	Thaw();
}

SettingDlg::~SettingDlg()
{
}

wxWindow* SettingDlg::CreateProjectPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st;
	//project save
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Open/Save/Script Run/UI");
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

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 5, 10); // 2 columns, 5px hgap, 10px vgap
	gridSizer->AddGrowableCol(1, 1); // Make the right column growable
	st = new wxStaticText(page, 0, "Set multifunction buttons to:");
	m_mul_func_btn_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> items = {"Sync Channels", "Focused Scroll", "Use Default", "Use ML", "Undo", "Enable/Disable"};
	m_mul_func_btn_comb->Append(items);
	m_mul_func_btn_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnMulFuncBtnComb, this);
	gridSizer->Add(st, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
	gridSizer->Add(m_mul_func_btn_comb, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
	st = new wxStaticText(page, 0, "Project and config file type:");
	m_config_file_type_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(100, -1)), 0, NULL, wxCB_READONLY);
	std::vector<wxString> items2 = { "INI", "XML", "JSON" };
	m_config_file_type_comb->Append(items2);
	m_config_file_type_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnConfigFileTypeComb, this);
	gridSizer->Add(st, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
	gridSizer->Add(m_config_file_type_comb, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
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
	group1->Add(gridSizer);
	group1->Add(10, 5);

	//font
	wxStaticBoxSizer *group2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Render View Text");
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
	std::vector<wxString> items3 = { "BG inverted", "Background", "Vol sec color" };
	m_text_color_cmb->Append(items3);
	m_text_color_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnTextColorChange, this);
	sizer2_1->Add(st);
	sizer2_1->Add(10, 10);
	sizer2_1->Add(m_text_color_cmb);
	wxBoxSizer* sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"Put TrueType font files into the \"Fonts\" folder. "\
		"Restart FluoRender to load new font files.");
	st->Wrap(FromDIP(450));
	sizer2_2->Add(st, 1, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_2, 0, wxEXPAND);
	group2->Add(10, 5);

	//line width
	wxStaticBoxSizer *group3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Line Width");
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
	wxStaticBoxSizer *group4 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Paint History");
	wxBoxSizer *sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	m_paint_hist_depth_sldr = new wxSingleSlider(page, wxID_ANY, 0, 0, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_paint_hist_depth_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPaintHistDepthChange, this);
	m_paint_hist_depth_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_paint_hist_depth_text->Bind(wxEVT_TEXT, &SettingDlg::OnPaintHistDepthEdit, this);
	sizer4_1->Add(m_paint_hist_depth_sldr, 1, wxEXPAND);
	sizer4_1->Add(m_paint_hist_depth_text, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer4_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"The number of undo steps for paint brush selection. " \
		"Set the value to 0 to disable history. " \
		"A value greater than 0 slows down speed and increases memory usage.");
	st->Wrap(FromDIP(450));
	sizer4_2->Add(st, 1, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(sizer4_1, 0, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(sizer4_2, 0, wxEXPAND);
	group4->Add(10, 5);

	//pencil distance
	wxStaticBoxSizer* group5 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Ruler Point Distance");
	wxBoxSizer* sizer5_1 = new wxBoxSizer(wxHORIZONTAL);
	m_pencil_dist_sldr = new wxSingleSlider(page, wxID_ANY, 30, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pencil_dist_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPencilDistChange, this);
	m_pencil_dist_text = new wxTextCtrl(page, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_pencil_dist_text->Bind(wxEVT_TEXT, &SettingDlg::OnPencilDistEdit, this);
	sizer5_1->Add(m_pencil_dist_sldr, 1, wxEXPAND);
	sizer5_1->Add(m_pencil_dist_text, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer5_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"The pixel distance between two ruler points for pencil and magnet.");
	st->Wrap(FromDIP(450));
	sizer5_2->Add(st, 1, wxEXPAND);
	group5->Add(10, 5);
	group5->Add(sizer5_1, 0, wxEXPAND);
	group5->Add(10, 5);
	group5->Add(sizer5_2, 0, wxEXPAND);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* SettingDlg::CreateAutomationPage(wxWindow* parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 6, 10); // 2 columns, 5px hgap, 10px vgap
	gridSizer->AddGrowableCol(0, 1); // Make the text column growable

	std::vector<std::string> keys = {
		"histogram",
		"paint size",
		"comp gen",
		"colocalize",
		"relax ruler",
		"conv vol mesh"};

	m_automate_combo.insert({
		keys[0],
		ComboEntry{ 0, keys[0],
		"Compute histograms for volume data",
		{ "Disable", "Enable", "Disable for large data" }, nullptr } });
	m_automate_combo.insert({
		keys[1],
		ComboEntry{ 1, keys[1],
		"Compute information on brush selection",
		{ "Disable", "Enable", "Disable for large data" }, nullptr } });
	m_automate_combo.insert({
		keys[2],
		ComboEntry{ 2, keys[2],
		"Generate volume components when settings change",
		{ "Disable", "Enable", "Disable for large data" }, nullptr } });
	m_automate_combo.insert({
		keys[3],
		ComboEntry{ 3, keys[3],
		"Update colocalization result when settings change",
		{ "Disable", "Enable", "Disable for large data" }, nullptr } });
	m_automate_combo.insert({
		keys[4],
		ComboEntry{ 4, keys[4],
		"Relax a multipoint ruler after creation",
		{ "Disable", "Enable", "Disable for large data" }, nullptr } });
	m_automate_combo.insert({
		keys[5],
		ComboEntry{ 5, keys[5],
		"Update mesh after conversion from volume data",
		{ "Disable", "Enable", "Disable for large data" }, nullptr } });

	for (const auto& key : keys)
	{
		auto it = m_automate_combo.find(key);
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;

			wxStaticText* label = new wxStaticText(page, wxID_ANY, entry.label);
			entry.combo = new wxComboBox(page, entry.id, "", wxDefaultPosition, wxDefaultSize, entry.options, wxCB_READONLY);
			entry.combo->Bind(wxEVT_COMBOBOX, &SettingDlg::OnAutomationCombo, this);

			gridSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
			gridSizer->Add(entry.combo, 1, wxEXPAND);
		}
	}

	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* st = new wxStaticText(page, wxID_ANY, "Automatically perform these functions:");
	sizer1->Add(10, 10);
	sizer1->Add(st, wxALIGN_CENTER);

	wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer1, 0, wxEXPAND);
	sizerV->Add(gridSizer, 1, wxALL | wxEXPAND, 15);

	page->SetSizerAndFit(sizerV);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* SettingDlg::CreateRenderingPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	vald_fp2.SetRange(-180.0, 180.0);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxStaticText* st;

	//micro blending
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Micro Blending");
	m_micro_blend_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Micro Blending");
	m_micro_blend_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnMicroBlendCheck, this);
	wxBoxSizer* sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"Micro Blending works only on multiple channels in the depth mode. "\
		"Enable Micro Blending to render the colors of the colocalized voxels correctly. "\
		"On certain systems, it may slow down the rendering speed significantly.");
	st->Wrap(FromDIP(450));
	sizer1_1->Add(st, 1, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(m_micro_blend_chk, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);

	//depth peeling
	wxStaticBoxSizer *group2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Mesh Transparency Quality");
	wxBoxSizer *sizer2_1 = new wxBoxSizer(wxHORIZONTAL);
	m_peeling_layers_sldr = new wxSingleSlider(page, wxID_ANY, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_peeling_layers_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnPeelingLayersChange, this);
	m_peeling_layers_text = new wxTextCtrl(page, wxID_ANY, "1",
		wxDefaultPosition, FromDIP(wxSize(40, 20)), wxTE_RIGHT, vald_int);
	m_peeling_layers_text->Bind(wxEVT_TEXT, &SettingDlg::OnPeelingLayersEdit, this);
	sizer2_1->Add(m_peeling_layers_sldr, 1, wxEXPAND);
	sizer2_1->Add(m_peeling_layers_text, 0, wxALIGN_CENTER);
	wxBoxSizer* sizer2_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"The number of depth peeling layers for rendering transparent mesh objects. "\
		"Set higher numbers only for complex geometries. "\
		"It slows down the rendering speed.");
	st->Wrap(FromDIP(450));
	sizer2_2->Add(st, 1, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);
	group2->Add(sizer2_2, 0, wxEXPAND);
	group2->Add(10, 5);

	//rotations
	wxStaticBoxSizer* group3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Rotations");
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
	group3->Add(10, 5);
	group3->Add(sizer4_1, 0, wxEXPAND);
	group3->Add(10, 5);
	group3->Add(sizer4_2, 0, wxEXPAND);
	group3->Add(10, 5);

	//gradient background
	wxStaticBoxSizer *group4 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Gradient Background");
	wxBoxSizer *sizer5_1 = new wxBoxSizer(wxHORIZONTAL);
	m_grad_bg_chk = new wxCheckBox(page, wxID_ANY,
		"Enable Gradient Background");
	m_grad_bg_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnGradBgCheck, this);
	sizer5_1->Add(m_grad_bg_chk, 0, wxALIGN_CENTER);
	group4->Add(10, 5);
	group4->Add(sizer5_1, 0, wxEXPAND);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* SettingDlg::CreatePerformancePage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	wxIntegerValidator<int> vald_int2;
	wxStaticText* st;
	std::vector<wxString> items = { "Disable", "Enable", "Enable for large data" };

	//large data definition
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Large Data Definition");
	wxFlexGridSizer* sizer1_1 = new wxFlexGridSizer(4, 5, 10); // 4 columns, 5px hgap, 10px vgap
	sizer1_1->AddGrowableCol(1, 1); // Make the slider column growable
	m_large_data_sldr = new wxSingleSlider(page, wxID_ANY, 20, 0, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_large_data_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnLargeDataChange, this);
	m_large_data_text = new wxTextCtrl(page, wxID_ANY, "200",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_large_data_text->Bind(wxEVT_TEXT, &SettingDlg::OnLargeDataEdit, this);
	sizer1_1->Add(new wxStaticText(page, 0, "Size Threshold:"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_1->Add(m_large_data_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer1_1->Add(m_large_data_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_1->Add(new wxStaticText(page, 0, "MB"),
		0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* sizer1_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"Large data refers to volumes that use more graphics memory than the specified threshold. "\
		"When data exceeds this size, certain performance-enhancing options may become available "\
		"to help maintain smooth interaction and rendering.");
	st->Wrap(FromDIP(450));
	sizer1_2->Add(st, 1, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);
	group1->Add(sizer1_2, 0, wxEXPAND);
	group1->Add(10, 5);

	//mouse interaction
	wxStaticBoxSizer *group2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Variable Sample Rate");
	wxFlexGridSizer* sizer2_1 = new wxFlexGridSizer(2, 5, 10); // 2 columns, 5px hgap, 10px vgap
	sizer2_1->AddGrowableCol(0, 1); // Make the right column growable
	st = new wxStaticText(page, 0, "Reduce volume rendering quality during interactions");
	m_mouse_int_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_mouse_int_comb->Append(items);
	m_mouse_int_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnMouseIntComb, this);
	sizer2_1->Add(st, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer2_1->Add(m_mouse_int_comb, 1, wxEXPAND | wxRIGHT, 5);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);

	//streaming settings
	wxStaticBoxSizer *group3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Streamed Rendering");
	//slider group
	wxFlexGridSizer* sizer3_1 = new wxFlexGridSizer(2, 5, 10); // 2 columns, 5px hgap, 10px vgap
	sizer3_1->AddGrowableCol(0, 1); // Make the right column growable
	//enable streaming
	m_streaming_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_streaming_comb->Append(items);
	m_streaming_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnStreamingComb, this);
	sizer3_1->Add(new wxStaticText(page, 0, "Enable streamed rendering"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_1->Add(m_streaming_comb, 1, wxEXPAND | wxRIGHT, 5);
	//update order
	std::vector<wxString> update_options = {"Back to Front", "Front to Back"};
	m_update_order_comb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	m_update_order_comb->Append(update_options);
	m_update_order_comb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnUpdateOrderChange, this);
	sizer3_1->Add(new wxStaticText(page, 0, "Update order"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_1->Add(m_update_order_comb, 1, wxEXPAND | wxRIGHT, 5);

	//slider group
	wxFlexGridSizer* sizer3_2 = new wxFlexGridSizer(4, 5, 10); // 4 columns, 5px hgap, 10px vgap
	sizer3_2->AddGrowableCol(1, 1); // Make the slider column growable
	//graphics memory
	m_graphics_mem_sldr = new wxSingleSlider(page, wxID_ANY, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_graphics_mem_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnGraphicsMemChange, this);
	m_graphics_mem_text = new wxTextCtrl(page, wxID_ANY, "1000",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_graphics_mem_text->Bind(wxEVT_TEXT, &SettingDlg::OnGraphicsMemEdit, this);
	sizer3_2->Add(new wxStaticText(page, 0, "Graphics Memory:"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(m_graphics_mem_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer3_2->Add(m_graphics_mem_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(new wxStaticText(page, 0, "MB"),
		0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	//brick size
	m_block_size_sldr = new wxSingleSlider(page, wxID_ANY, 7, 6, 12,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_block_size_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnBlockSizeChange, this);
	m_block_size_text = new wxTextCtrl(page, wxID_ANY, "128",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_block_size_text->Bind(wxEVT_TEXT, &SettingDlg::OnBlockSizeEdit, this);
	sizer3_2->Add(new wxStaticText(page, 0, "Brick Size:"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(m_block_size_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer3_2->Add(m_block_size_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(new wxStaticText(page, 0, "voxel"),
		0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	//response time
	m_response_time_sldr = new wxSingleSlider(page, wxID_ANY, 10, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_response_time_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnResponseTimeChange, this);
	m_response_time_text = new wxTextCtrl(page, wxID_ANY, "100",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int);
	m_response_time_text->Bind(wxEVT_TEXT, &SettingDlg::OnResponseTimeEdit, this);
	sizer3_2->Add(new wxStaticText(page, 0, "Response Time:"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(m_response_time_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer3_2->Add(m_response_time_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(new wxStaticText(page, 0, "ms"),
		0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
	//detail level offset
	m_detail_level_offset_sldr = new wxSingleSlider(page, wxID_ANY, 0, -5, 5,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_detail_level_offset_sldr->Bind(wxEVT_SCROLL_CHANGED, &SettingDlg::OnDetailLevelOffsetChange, this);
	m_detail_level_offset_text = new wxTextCtrl(page, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(40, -1)), wxTE_RIGHT, vald_int2);
	m_detail_level_offset_text->Bind(wxEVT_TEXT, &SettingDlg::OnDetailLevelOffsetEdit, this);
	sizer3_2->Add(new wxStaticText(page, 0, "Detail Level Offset:"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(m_detail_level_offset_sldr, 1, wxEXPAND | wxRIGHT, 5);
	sizer3_2->Add(m_detail_level_offset_text, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer3_2->Add(new wxStaticText(page, 0, ""),
		0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);

	group3->Add(10, 5);
	group3->Add(sizer3_1, 0, wxEXPAND);
	group3->Add(10, 5);
	group3->Add(sizer3_2, 0, wxEXPAND);
	group3->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(group3, 0, wxEXPAND);

	page->SetSizer(sizerV);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* SettingDlg::CreateDisplayPage(wxWindow* parent)
{
	wxScrolledWindow* page = new wxScrolledWindow(parent);

	wxIntegerValidator<unsigned int> vald_int;
	wxFloatingPointValidator<double> vald_fp1(1);
	wxStaticText* st;
	std::vector<wxString> cmb_str;

	//stereo
	wxStaticBoxSizer* group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Stereography / Virtual Reality / Holography");
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
		wxDefaultPosition, FromDIP(wxSize(80, 20)));
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
	st = new wxStaticText(page, 0, "Install Looking Glass Bridge and connect a Looking Glass display. Then send the full-screen render view to the display.");
	st->Wrap(FromDIP(450));
	sizer1_7->Add(st, 1, wxALIGN_CENTER);
	wxBoxSizer* sizer1_8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0, "Depth Range:",
		wxDefaultPosition, FromDIP(wxSize(80, 20)));
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
	//lg settings
	wxFlexGridSizer* sizer1_9 = new wxFlexGridSizer(2, 5, 10); // 2 columns, 5px hgap, 10px vgap
	sizer1_9->AddGrowableCol(0, 1); // Make the right column growable
	//mode
	m_lg_camera_mode_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> items2 =
		{ "Camera Shift + Lens Shift", "Camera Shift + Swing", "Object Rotation" };
	m_lg_camera_mode_cmb->Append(items2);
	m_lg_camera_mode_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnLgCameraModeComb, this);
	sizer1_9->Add(new wxStaticText(page, 0, "Holography Mode"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_9->Add(m_lg_camera_mode_cmb, 1, wxEXPAND | wxRIGHT, 5);
	//quilt display
	m_lg_quilt_cmb = new wxComboBox(page, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
	std::vector<wxString> items1 = { "Center View", "First View", "Last View", "All Views"};
	m_lg_quilt_cmb->Append(items1);
	m_lg_quilt_cmb->Bind(wxEVT_COMBOBOX, &SettingDlg::OnLgQuiltComb, this);
	sizer1_9->Add(new wxStaticText(page, 0, "Quilt Display"),
		0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
	sizer1_9->Add(m_lg_quilt_cmb, 1, wxEXPAND | wxRIGHT, 5);
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
	wxStaticBoxSizer* group2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Fullscreen on Display");
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
		wxDisplay display(i);
		disp_name = display.GetName();
		if (disp_name.IsEmpty())
			cmb_str.push_back(wxString::Format("%d", i));
		else
			cmb_str.push_back(wxString::Format("%d", i) + ": " + disp_name);
	}
	m_disp_id_comb->Append(cmb_str);
	sizer2_1->Add(st, 0, wxALIGN_CENTER);
	sizer2_1->Add(5, 5);
	sizer2_1->Add(m_disp_id_comb, 0, wxALIGN_CENTER);
	group2->Add(10, 5);
	group2->Add(sizer2_1, 0, wxEXPAND);
	group2->Add(10, 5);

	//color depth
	wxStaticBoxSizer* group3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Color Depth of Render View");
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* SettingDlg::CreateFormatPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;
	//float 2
	wxFloatingPointValidator<double> vald_fp1(2);
	wxStaticText* st;
	//override vox
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Override Voxel Size");
	wxBoxSizer *sizer1_1 = new wxBoxSizer(wxHORIZONTAL);
	m_override_vox_chk = new wxCheckBox(page, wxID_ANY,
		"Get voxel size info from the first opened data set.");
	m_override_vox_chk->Bind(wxEVT_CHECKBOX, &SettingDlg::OnOverrideVoxCheck, this);
	sizer1_1->Add(m_override_vox_chk, 0, wxALIGN_CENTER);
	group1->Add(10, 5);
	group1->Add(sizer1_1, 0, wxEXPAND);
	group1->Add(10, 5);

	//wavelength to color
	wxStaticBoxSizer *group2 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Default Colors for Excitation Wavelengths (nm) (for raw microscopy formats)");
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
	wxStaticBoxSizer *group3 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Max Texture Size");
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
	wxBoxSizer* sizer3_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"Set a texture size smaller than 1024 to correctly perform component analysis. "\
		"When both the max texture size and data streaming are set, the smaller number "\
		"is used to divide a large data set into bricks. "\
		"Restart FluoRender after setting this value.");
	st->Wrap(FromDIP(450));
	sizer3_2->Add(st, 1, wxEXPAND);
	group3->Add(10, 5);
	group3->Add(sizer3_1, 0);
	group3->Add(10, 5);
	group3->Add(sizer3_2, 0);
	group3->Add(10, 5);

	//cl devices
	wxStaticBoxSizer *group4 = new wxStaticBoxSizer(
		wxVERTICAL, page, "OpenCL Devices");
	m_device_tree = new wxTreeCtrl(page, wxID_ANY,
		wxDefaultPosition, wxDefaultSize,
		wxTR_FULL_ROW_HIGHLIGHT);
	m_device_tree->Bind(wxEVT_TREE_SEL_CHANGED, &SettingDlg::OnSelChanged, this);
	wxBoxSizer* sizer4_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"Select an OpenCL device that is also the rendering GPU. "\
		"Restart FluoRender after changing OpenCL devices.");
	st->Wrap(FromDIP(450));
	sizer4_1->Add(st, 1, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(m_device_tree, 1, wxEXPAND);
	group4->Add(10, 5);
	group4->Add(sizer4_1, 0, wxEXPAND);
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
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
}

wxWindow* SettingDlg::CreateJavaPage(wxWindow *parent)
{
	wxScrolledWindow *page = new wxScrolledWindow(parent);

	wxStaticText* st;
	//JVM settings.
	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, page, "Java Settings");
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

	wxBoxSizer* sizer1_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(page, 0,
		"Restart FluoRender for Java settings to take effect. " \
		"Bioformats and JVM are required when ImageJ is selected.");
	st->Wrap(FromDIP(450));
	sizer1_4->Add(st, 1, wxEXPAND);
	group1->Add(sizer1_4, 0, wxEXPAND);
	group1->Add(10, 5);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);
	sizerV->Add(10, 10);

	page->SetSizer(sizerV);
	page->SetAutoLayout(true);
	page->SetScrollRate(10, 10);
	return page;
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
		//config file type
		m_config_file_type_comb->Select(glbin_settings.m_config_file_type);
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
				list.push_back(entry.path().stem().string());
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
		m_mouse_int_comb->SetSelection(glbin_settings.m_interactive_quality);

	//memory settings
	if (update_all || FOUND_VALUE(gstStreamEnable))
	{
		m_streaming_comb->SetSelection(glbin_settings.m_stream_rendering);
		m_update_order_comb->SetSelection(glbin_settings.m_update_order);
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

	//automate page
	if (update_all || FOUND_VALUE(gstAutomate))
	{
		auto it = m_automate_combo.find("histogram");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_histogram);
		}
		it = m_automate_combo.find("paint size");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_paint_size);
		}
		it = m_automate_combo.find("comp gen");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_comp_gen);
		}
		it = m_automate_combo.find("colocalize");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_colocalize);
		}
		it = m_automate_combo.find("relax ruler");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_relax_ruler);
		}
		it = m_automate_combo.find("conv vol mesh");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_conv_vol_mesh);
		}
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
			m_lg_quilt_cmb->Disable();
			m_lg_camera_mode_cmb->Disable();
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
			m_lg_quilt_cmb->Disable();
			m_lg_camera_mode_cmb->Disable();
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
			m_lg_quilt_cmb->Enable();
			m_lg_camera_mode_cmb->Enable();
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
		m_lg_quilt_cmb->Select(glbin_settings.m_hologram_debug);
		m_lg_camera_mode_cmb->Select(glbin_settings.m_hologram_camera_mode);
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
		//m_device_tree->SetFocus();
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

void SettingDlg::OnConfigFileTypeComb(wxCommandEvent& event)
{
	glbin_settings.m_config_file_type = m_config_file_type_comb->GetCurrentSelection();
}

void SettingDlg::OnMouseIntComb(wxCommandEvent& event)
{
	glbin_settings.m_interactive_quality = m_mouse_int_comb->GetCurrentSelection();
	//FluoRefresh(3, { gstNull });
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
	glbin.set_linked_rot(linked_rot);
	glbin.set_master_linked_view(0);
	FluoRefresh(3, { gstNull });
}

//stereo
void SettingDlg::OnStereoCheck(wxCommandEvent& event)
{
	bool bval = m_stereo_chk->GetValue();
	glbin_settings.m_hologram_mode = bval ? 1 : 0;
	FluoRefresh(0, { gstHologramMode });
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
	glbin_settings.m_hologram_mode = bval ? 2 : 0;
	FluoRefresh(0, { gstHologramMode });
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

void SettingDlg::OnLgQuiltComb(wxCommandEvent& event)
{
	glbin_settings.m_hologram_debug = m_lg_quilt_cmb->GetCurrentSelection();
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnLgCameraModeComb(wxCommandEvent& event)
{
	glbin_settings.m_hologram_camera_mode = m_lg_camera_mode_cmb->GetCurrentSelection();
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
void SettingDlg::OnStreamingComb(wxCommandEvent& event)
{
	glbin_settings.m_stream_rendering = m_streaming_comb->GetSelection();
	glbin_data_manager.UpdateStreamMode(-1.0);
	FluoRefresh(3, { gstNull });
}

void SettingDlg::OnUpdateOrderChange(wxCommandEvent& event)
{
	glbin_settings.m_update_order = m_update_order_comb->GetSelection();
	FluoRefresh(3, { gstNull });
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
	std::wstring str = m_font_cmb->GetValue().ToStdWstring();
	if (str.empty())
		return;

	glbin_settings.m_font_file = str + L".ttf";
	std::filesystem::path p = std::filesystem::current_path();
	p = p / "Fonts" / (str + L".ttf");
	glbin_text_tex_manager.load_face(p.wstring());
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
	ModalDlg fopendlg(
		m_frame, "Choose the jvm dll file",
		"", "", "*.dll", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#else
	ModalDlg fopendlg(
		m_frame, "Choose the libjvm.dylib file",
		"", "", "*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		m_java_jvm_text->ChangeValue(filename);
		glbin_settings.m_jvm_path = filename;
	}
}

void SettingDlg::onJavaIJBrowse(wxCommandEvent& event)
{
#ifdef _WIN32	
	wxDirDialog fopendlg(
		m_frame, "Choose the imageJ/fiji directory",
		"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
#else
	ModalDlg fopendlg(
		m_frame, "Choose the imageJ/fiji app",
		"", "", "*.app", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
#ifdef _DARWIN
		//filename = filename + "/Contents/Java/ij.jar";
#endif
		m_java_ij_text->ChangeValue(filename);
		glbin_settings.m_ij_path = filename;
	}
}

void SettingDlg::onJavaBioformatsBrowse(wxCommandEvent& event)
{
	ModalDlg fopendlg(
		m_frame, "Choose the bioformats jar",
		"", "", "*.jar", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		m_java_bioformats_text->ChangeValue(filename);
		glbin_settings.m_bioformats_path = filename;
	}
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

void SettingDlg::OnAutomationCombo(wxCommandEvent& event)
{
	int id = event.GetId();
	int index = event.GetSelection();

	switch (id)
	{
	case 0://histogram
		glbin_automate_def.m_histogram = index;
		break;
	case 1://paint size
		glbin_automate_def.m_paint_size = index;
		break;
	case 2://compo gen
		glbin_automate_def.m_comp_gen = index;
		break;
	case 3://colocalize
		glbin_automate_def.m_colocalize = index;
		break;
	case 4://relax ruler
		glbin_automate_def.m_relax_ruler = index;
		break;
	}
}