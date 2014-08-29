#include "VMovieView.h"
#include "VRenderFrame.h"
#include <wx/aboutdlg.h>
#include <wx/valnum.h>
#include <wx/notebook.h>
#include "png_resource.h"
#include "img/listicon_save.h"
#include "img/refresh.h"

BEGIN_EVENT_TABLE(VMovieView, wxPanel)
	//left column
	EVT_BUTTON(ID_RunBtn, VMovieView::OnRun)
	EVT_BUTTON(ID_PrevBtn, VMovieView::OnPrev)
	EVT_BUTTON(ID_StopBtn, VMovieView::OnStop)
	EVT_BUTTON(ID_ResetAngleBtn, VMovieView::OnResetAngle)
	EVT_COMBOBOX(ID_ViewsCombo, VMovieView::OnViewSelected)
	EVT_RADIOBUTTON(ID_XRd, VMovieView::OnAxisSelected)
	EVT_RADIOBUTTON(ID_YRd, VMovieView::OnAxisSelected)
	EVT_TEXT(ID_AngleStartText, VMovieView::OnStartAngleEditing)
	EVT_TEXT(ID_AngleEndText, VMovieView::OnEndAngleEditing)
	EVT_TEXT(ID_FramesText, VMovieView::OnFramesEditing)
	EVT_TEXT(ID_StepText, VMovieView::OnStepEditing)
	EVT_CHECKBOX(ID_RewindChk, VMovieView::OnRewindChecked)
	//modes
	EVT_RADIOBUTTON(ID_Mt3dRotRd, VMovieView::OnMtChecked)
	EVT_RADIOBUTTON(ID_Mt3dBatRd, VMovieView::OnMtChecked)
	EVT_RADIOBUTTON(ID_Mt4dSeqRd, VMovieView::OnMtChecked)
	EVT_RADIOBUTTON(ID_Mt4dRotRd, VMovieView::OnMtChecked)
	//right column
	EVT_CHECKBOX(ID_FrameChk, VMovieView::OnFrameCheck)
	EVT_BUTTON(ID_ResetBtn, VMovieView::OnResetFrame)
	EVT_TEXT(ID_CenterXText, VMovieView::OnFrameEditing)
	EVT_TEXT(ID_CenterYText, VMovieView::OnFrameEditing)
	EVT_TEXT(ID_WidthText, VMovieView::OnFrameEditing)
	EVT_TEXT(ID_HeightText, VMovieView::OnFrameEditing)
	EVT_SPIN_UP(ID_CenterXSpin, VMovieView::OnFrameSpinUp)
	EVT_SPIN_UP(ID_CenterYSpin, VMovieView::OnFrameSpinUp)
	EVT_SPIN_UP(ID_WidthSpin, VMovieView::OnFrameSpinUp)
	EVT_SPIN_UP(ID_HeightSpin, VMovieView::OnFrameSpinUp)
	EVT_SPIN_DOWN(ID_CenterXSpin, VMovieView::OnFrameSpinDown)
	EVT_SPIN_DOWN(ID_CenterYSpin, VMovieView::OnFrameSpinDown)
	EVT_SPIN_DOWN(ID_WidthSpin, VMovieView::OnFrameSpinDown)
	EVT_SPIN_DOWN(ID_HeightSpin, VMovieView::OnFrameSpinDown)
	//help
	EVT_BUTTON(ID_HelpBtn, VMovieView::OnHelpBtn)
	//4d frame
	EVT_COMMAND_SCROLL(ID_TimeSldr, VMovieView::OnTimeChange)
	EVT_SPIN_UP(ID_TimeSpin, VMovieView::OnTimeSpinUp)
	EVT_SPIN_DOWN(ID_TimeSpin, VMovieView::OnTimeSpinDown)
	EVT_TEXT_ENTER(ID_TimeText, VMovieView::OnTimeEnter)
END_EVENT_TABLE()

wxWindow* VMovieView::CreateSimplePage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);
	
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//sizers
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_2_5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_6_5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);


	//2nd line
	m_mt_3d_rot_rd = new wxRadioButton(page, ID_Mt3dRotRd, "3D Rot.",
		wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_mt_3d_bat_rd = new wxRadioButton(page, ID_Mt3dBatRd, "3D Batch");
	m_mt_4d_seq_rd = new wxRadioButton(page, ID_Mt4dSeqRd, "4D Seq.");
	m_mt_4d_rot_rd = new wxRadioButton(page, ID_Mt4dRotRd, "4D Rot.");
	m_mt_3d_rot_rd->SetValue(true);
	m_mt_3d_bat_rd->SetValue(false);
	m_mt_4d_seq_rd->SetValue(false);
	m_mt_4d_rot_rd->SetValue(false);
	m_movie_type = 1;
	sizer_2->Add(5, 5, 0);
	sizer_2->Add(m_mt_3d_rot_rd, 0, wxALIGN_CENTER);
	sizer_2->Add(3, 5, 0);
	sizer_2->Add(m_mt_3d_bat_rd, 0, wxALIGN_CENTER);
	sizer_2->Add(3, 5, 0);
	sizer_2->Add(m_mt_4d_seq_rd, 0, wxALIGN_CENTER);
	sizer_2->Add(3, 5, 0);
	sizer_2->Add(m_mt_4d_rot_rd, 0, wxALIGN_CENTER);

	//2.5th line
	m_time_sldr = new wxSlider(page, ID_TimeSldr, 1, 1, 360,
		wxDefaultPosition, wxSize(300, 20), wxHORIZONTAL);
	m_time_sldr->Disable();
	m_time_spin = new wxSpinButton(page, ID_TimeSpin,
		wxDefaultPosition, wxSize(20, 20), wxHORIZONTAL);
	m_time_spin->Disable();
	m_time_text = new wxTextCtrl(page, ID_TimeText, "1",
		wxDefaultPosition, wxSize(30, 20), 0, vald_int);
	m_time_text->Disable();
	sizer_2_5->Add(5, 5, 0);
	sizer_2_5->Add(m_time_sldr, 1, wxEXPAND);
	sizer_2_5->Add(m_time_text, 0, wxALIGN_CENTER);
	sizer_2_5->Add(m_time_spin, 0, wxALIGN_CENTER);

	//3rd line
	st = new wxStaticText(page, 0, "Rotation axis:",
		wxDefaultPosition, wxSize(115, 20));
	m_x_rd = new wxRadioButton(page, ID_XRd, "X",
		wxDefaultPosition, wxSize(40, 20), wxRB_GROUP);
	m_y_rd = new wxRadioButton(page, ID_YRd, "Y",
		wxDefaultPosition, wxSize(40, 20)/*, wxRB_GROUP*/);
	m_x_rd->SetValue(false);
	m_y_rd->SetValue(true);
	m_rewind_chk = new wxCheckBox(page, ID_RewindChk, "Rewind");
	sizer_3->Add(5, 5, 0);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_x_rd, 0, wxALIGN_CENTER);
	sizer_3->Add(m_y_rd, 0, wxALIGN_CENTER);
	sizer_3->Add(m_rewind_chk, 0, wxALIGN_CENTER);

	//4th line
	st = new wxStaticText(page, 0, "minus (-)",
		wxDefaultPosition, wxSize(65, 15));
	sizer_4->Add(95, 5, 0);
	sizer_4->Add(st, 0, wxALIGN_BOTTOM);
	st = new wxStaticText(page, 0, "current",
		wxDefaultPosition, wxSize(55, 15));
	sizer_4->Add(st, 0, wxALIGN_BOTTOM);
	st = new wxStaticText(page, 0, "plus (+)",
		wxDefaultPosition, wxSize(65, 15));
	sizer_4->Add(st, 0, wxALIGN_BOTTOM);

	//5th line
	st = new wxStaticText(page, 0, "Rotation Deg.:",
		wxDefaultPosition, wxSize(100, 20));
	m_angle_start_text = new wxTextCtrl(page, ID_AngleStartText, "0",
		wxDefaultPosition, wxSize(55, 20), 0, vald_int);
	sizer_5->Add(st, 0, wxALIGN_CENTER);
	sizer_5->Add(m_angle_start_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "    0",
		wxDefaultPosition, wxSize(65, 20));
	m_angle_end_text = new wxTextCtrl(page, ID_AngleEndText, "360",
		wxDefaultPosition, wxSize(55, 20), 0, vald_int);
	m_prev_btn = new wxButton(page, ID_PrevBtn, "Preview",
		wxDefaultPosition, wxSize(70, 20));
	sizer_5->Add(st, 0, wxALIGN_CENTER);
	sizer_5->Add(m_angle_end_text, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5, 0);
	sizer_5->Add(m_prev_btn, 0, wxALIGN_CENTER);

	//6th line
	sizer_6->Add(10, 5, 0);
	st = new wxStaticText(page, 0, "Deg./Frame:",
		wxDefaultPosition, wxSize(85, 20));
	m_step_text = new wxTextCtrl(page, ID_StepText, "1",
		wxDefaultPosition, wxSize(55, 20), 0, vald_int);
	sizer_6->Add(st, 0, wxALIGN_CENTER);
	sizer_6->Add(5, 5, 0);
	sizer_6->Add(m_step_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "Frames:",
		wxDefaultPosition, wxSize(65, 20));
	m_frames_text = new wxTextCtrl(page, ID_FramesText, "360",
		wxDefaultPosition, wxSize(55, 20), 0, vald_int);
	m_stop_btn = new wxButton(page, ID_StopBtn, "Stop",
		wxDefaultPosition, wxSize(70, 20));
	sizer_6->Add(st, 0, wxALIGN_RIGHT);
	sizer_6->Add(m_frames_text, 0, wxALIGN_CENTER);
	sizer_6->Add(5, 5, 0);
	sizer_6->Add(m_stop_btn, 0, wxALIGN_CENTER);

	//6.5
	st = new wxStaticText(page, 0, "Time from:",
		wxDefaultPosition, wxSize(85, 20));
	m_time_start_text = new wxTextCtrl(page, ID_TimeStartText, "1",
		wxDefaultPosition, wxSize(55, 20));
	sizer_6_5->Add(15, 5, 0);
	sizer_6_5->Add(st, 0, wxALIGN_CENTER);
	sizer_6_5->Add(m_time_start_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "to:",
		wxDefaultPosition, wxSize(65, 20));
	m_time_end_text = new wxTextCtrl(page, ID_TimeEndText, "1",
		wxDefaultPosition, wxSize(55, 20));
	sizer_6_5->Add(st, 0, wxALIGN_RIGHT);
	sizer_6_5->Add(m_time_end_text, 0, wxALIGN_CENTER);
	m_run_btn = new wxButton(page, ID_RunBtn, "Save...",
		wxDefaultPosition, wxSize(80, 20));
	sizer_6_5->Add(5, 5, 0);
	sizer_6_5->Add(m_run_btn, 0, wxALIGN_CENTER);

	//7th line
	m_reset_angle_btn = new wxButton(page, ID_ResetAngleBtn, "Reset",
		wxDefaultPosition, wxSize(60, 20));
	sizer_7->Add(160, 5, 0);
	sizer_7->Add(m_reset_angle_btn, 0, wxALIGN_CENTER);
	
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->Add(sizer_2_5, 1, wxEXPAND);
	sizer_v->Add(sizer_3, 1, wxEXPAND);
	sizer_v->Add(sizer_4, 0, wxEXPAND);
	sizer_v->Add(sizer_5, 1, wxEXPAND);
	sizer_v->Add(sizer_6, 1, wxEXPAND);
	sizer_v->Add(sizer_6_5, 1, wxEXPAND);
	sizer_v->Add(sizer_7, 1, wxEXPAND);
	sizer_v->AddStretchSpacer();

	page->SetSizer(sizer_v);
	return page;
}

wxWindow* VMovieView::CreateAdvancedPage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
    RecorderDlg * recorder = new RecorderDlg(m_frame, page);
	(reinterpret_cast<VRenderFrame*>(m_frame))->m_recorder_dlg = recorder;
	sizer_v->Add(recorder,0,wxEXPAND);
	page->SetSizer(sizer_v);
	return page;
}

wxWindow* VMovieView::CreateCroppingPage(wxWindow *parent) {
	wxPanel *page = new wxPanel(parent);
	
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//sizers
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_9 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_10 = new wxBoxSizer(wxHORIZONTAL);
	//vertical sizer
	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	//8th line
	st = new wxStaticText(page, 0, "Enable cropping:",
		wxDefaultPosition, wxSize(110, 20));
	m_frame_chk = new wxCheckBox(page, ID_FrameChk, "");
	m_reset_btn = new wxButton(page, ID_ResetBtn, "Reset",
		wxDefaultPosition, wxSize(110, 22));
	m_reset_btn->SetBitmap(wxGetBitmapFromMemory(refresh));
	sizer_8->Add(5, 5, 0);
	sizer_8->Add(st, 0, wxALIGN_CENTER);
	sizer_8->Add(m_frame_chk, 0, wxALIGN_CENTER);
	sizer_8->Add(100, 5, 0);
	sizer_8->Add(m_reset_btn, 0, wxALIGN_CENTER);

	//9th line
	st = new wxStaticText(page, 0, "Center:  X:",
		wxDefaultPosition, wxSize(85, 20));
	m_center_x_text = new wxTextCtrl(page, ID_CenterXText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_center_x_spin = new wxSpinButton(page, ID_CenterXSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_9->Add(5, 5, 0);
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_x_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "       Y:",
		wxDefaultPosition, wxSize(50, 20));
	m_center_y_text = new wxTextCtrl(page, ID_CenterYText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_center_y_spin = new wxSpinButton(page, ID_CenterYSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_9->Add(st, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_text, 0, wxALIGN_CENTER);
	sizer_9->Add(m_center_y_spin, 0, wxALIGN_CENTER);

	//10th line
	st = new wxStaticText(page, 0, "Size:    Width:",
		wxDefaultPosition, wxSize(85, 20));
	m_width_text = new wxTextCtrl(page, ID_WidthText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_width_spin = new wxSpinButton(page, ID_WidthSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_10->Add(5, 5, 0);
	sizer_10->Add(st, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_text, 0, wxALIGN_CENTER);
	sizer_10->Add(m_width_spin, 0, wxALIGN_CENTER);
	st = new wxStaticText(page, 0, "   Height:",
		wxDefaultPosition, wxSize(50, 20));
	m_height_text = new wxTextCtrl(page, ID_HeightText, "",
		wxDefaultPosition, wxSize(60, 20), 0, vald_int);
	m_height_spin = new wxSpinButton(page, ID_HeightSpin,
		wxDefaultPosition, wxSize(20, 20));
	sizer_10->Add(st, 0, wxALIGN_CENTER);
	sizer_10->Add(m_height_text, 0, wxALIGN_CENTER);
	sizer_10->Add(m_height_spin, 0, wxALIGN_CENTER);

	sizer_v->Add(sizer_8, 1, wxEXPAND);
	sizer_v->Add(sizer_9, 1, wxEXPAND);
	sizer_v->Add(sizer_10, 1, wxEXPAND);
	sizer_v->AddStretchSpacer();

	page->SetSizer(sizer_v);
	return page;

}

VMovieView::VMovieView(wxWindow* frame,
	wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_init(false),
m_frame(frame),
m_rewind(false),
m_reset_time_frame(1),
m_prev_frame(0)
{
	
	//notebook
	wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
	notebook->AddPage(CreateSimplePage(notebook), "Basic");
	notebook->AddPage(CreateAdvancedPage(notebook), "Advanced");
	notebook->AddPage(CreateCroppingPage(notebook), "Cropping");
	
	//renderview selector
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText * st = new wxStaticText(this, 0, "Capture view:",
		wxDefaultPosition, wxSize(120, 20));
	m_views_cmb = new wxComboBox(this, ID_ViewsCombo, "",
		wxDefaultPosition, wxSize(120, -1), 0, NULL, wxCB_READONLY);
	m_help_btn = new wxButton(this, ID_HelpBtn, "?",
		wxDefaultPosition, wxSize(25, 25));

	//the play/rewind/slider/save
	wxBoxSizer *sizerH = new wxBoxSizer(wxHORIZONTAL);
	wxButton * m_play_btn = new wxButton(this, ID_PrevBtn, "|>",
		wxDefaultPosition, wxSize(22, 22));
	sizerH->Add(m_play_btn, 0, wxEXPAND|wxALIGN_CENTER);
	wxButton * m_rewind_btn = new wxButton(this, wxID_ANY, "<<",
		wxDefaultPosition, wxSize(22, 22));
	sizerH->Add(m_rewind_btn, 0, wxEXPAND|wxALIGN_CENTER);
	wxSlider * m_progress_slider = new wxSlider(this, wxID_ANY, 0, 0, 360);
	sizerH->Add(m_progress_slider, 1, wxEXPAND|wxALIGN_CENTER);
	wxTextCtrl * m_progress_text = new wxTextCtrl(this, wxID_ANY, "0.00",
		wxDefaultPosition,wxSize(50, -1));
	sizerH->Add(m_progress_text, 0, wxEXPAND|wxALIGN_CENTER);
	wxButton * m_save_btn = new wxButton(this, ID_RunBtn, "Save...",
		wxDefaultPosition, wxSize(80, 22));
	m_save_btn->SetBitmap(wxGetBitmapFromMemory(listicon_save));
	sizerH->Add(m_save_btn, 0, wxEXPAND|wxALIGN_CENTER);
	

	sizer_1->Add(5, 5, 0);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_views_cmb, 0, wxALIGN_CENTER);
	sizer_1->Add(60, 5, 0);
	sizer_1->Add(m_help_btn, 0, wxALIGN_CENTER);

	
	//interface
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(notebook, 1, wxEXPAND);
	sizerV->AddStretchSpacer();
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(sizerH,0,wxEXPAND);
	SetSizerAndFit(sizerV);
	Layout();
	Init();

	m_init = true;
	DisableRange();
}

VMovieView::~VMovieView()
{
}

void VMovieView::OnViewSelected(wxCommandEvent& event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		vr_frame->m_mov_view = m_views_cmb->GetCurrentSelection();
		GetSettings(vr_frame->m_mov_view);
	}
}

void VMovieView::OnAxisSelected(wxCommandEvent& event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		if (m_x_rd->GetValue())
			vr_frame->m_mov_axis = 1;
		if (m_y_rd->GetValue())
			vr_frame->m_mov_axis = 2;		
	}
}

void VMovieView::OnStartAngleEditing(wxCommandEvent& event)
{
	if (m_init)
	{
		wxString str = m_angle_start_text->GetValue();
		double start_angle = -fabs(STOD(str.fn_str()));
		str = m_angle_end_text->GetValue();
		double end_angle = STOD(str.fn_str());
		str = m_step_text->GetValue();
		double step = STOD(str.fn_str());
		int frames = CalcFrames(step, start_angle, end_angle);
		str = wxString::Format("%d", frames);
		m_frames_text->ChangeValue(str);

		if (start_angle < 0.0 && !m_rewind && end_angle > 0.0)
		{
			m_rewind_chk->SetValue(true);
			m_rewind = m_rewind_chk->GetValue();
		}

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			vr_frame->m_mov_angle_start = m_angle_start_text->GetValue();
	}
}

void VMovieView::OnEndAngleEditing(wxCommandEvent& event)
{
	if (m_init)
	{
		wxString str = m_angle_start_text->GetValue();
		double start_angle = -fabs(STOD(str.fn_str()));
		str = m_angle_end_text->GetValue();
		double end_angle = STOD(str.fn_str());
		str = m_step_text->GetValue();
		double step = STOD(str.fn_str());
		int frames = CalcFrames(step, start_angle, end_angle);
		str = wxString::Format("%d", frames);
		m_frames_text->ChangeValue(str);

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			vr_frame->m_mov_angle_end = m_angle_end_text->GetValue();
	}
}

void VMovieView::OnFramesEditing(wxCommandEvent& event)
{
	if (m_init)
	{
		wxString str = m_angle_start_text->GetValue();
		double start_angle = -fabs(STOD(str.fn_str()));
		str = m_angle_end_text->GetValue();
		double end_angle = STOD(str.fn_str());
		str = m_frames_text->GetValue();
		int frames = STOI(str.fn_str());
		if (frames <= 0)
			frames = 1;
		double step = CalcStep(frames, start_angle, end_angle);
		if (step < 0.01)
			step = 0.01;
		str = wxString::Format("%.0f", step);
		m_step_text->ChangeValue(str);

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			vr_frame->m_mov_frames = m_frames_text->GetValue();
	}
}

void VMovieView::OnStepEditing(wxCommandEvent& event)
{
	if (m_init)
	{
		wxString str = m_angle_start_text->GetValue();
		double start_angle = -fabs(STOD(str.fn_str()));
		str = m_angle_end_text->GetValue();
		double end_angle = STOD(str.fn_str());
		str = m_step_text->GetValue();
		double step = STOD(str.fn_str());
		if (step < 0.01)
			step = 0.01;
		int frames = CalcFrames(step, start_angle, end_angle);
		str = wxString::Format("%d", frames);
		m_frames_text->ChangeValue(str);

		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			vr_frame->m_mov_step = m_step_text->GetValue();
	}
}

void VMovieView::OnRewindChecked(wxCommandEvent& event)
{
	OnStartAngleEditing(event);
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		vr_frame->m_mov_rewind = m_rewind_chk->GetValue();
	m_rewind = m_rewind_chk->GetValue();
}

void VMovieView::GetSettings(int view)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_views_cmb->SetSelection(vr_frame->m_mov_view);
		if (vr_frame->m_mov_axis == 1)
			m_x_rd->SetValue(true);
		else if (vr_frame->m_mov_axis == 2)
			m_y_rd->SetValue(true);
		m_rewind = vr_frame->m_mov_rewind;
		m_rewind_chk->SetValue(m_rewind);
		if (vr_frame->m_mov_angle_start != "")
			m_angle_start_text->SetValue(vr_frame->m_mov_angle_start);
		if (vr_frame->m_mov_angle_end != "")
			m_angle_end_text->SetValue(vr_frame->m_mov_angle_end);
		if (vr_frame->m_mov_step != "")
			m_step_text->SetValue(vr_frame->m_mov_step);
		if (vr_frame->m_mov_frames != "")
			m_frames_text->SetValue(vr_frame->m_mov_frames);

		VRenderView* vrv = (*vr_frame->GetViewList())[view];
		if (vrv)
		{
			if (vrv->GetFrameEnabled())
			{
				m_frame_chk->SetValue(true);
				int x, y, w, h;
				vrv->GetFrame(x, y, w, h);
				m_center_x_text->SetValue(wxString::Format("%d", int(x+w/2.0+0.5)));
				m_center_y_text->SetValue(wxString::Format("%d", int(y+h/2.0+0.5)));
				m_width_text->SetValue(wxString::Format("%d", w));
				m_height_text->SetValue(wxString::Format("%d", h));
			}
			else
				m_frame_chk->SetValue(false);
		}
	}
}

void VMovieView::Init()
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		int i=0;
		m_views_cmb->Clear();
		for (i=0 ; i<vr_frame->GetViewNum(); i++)
		{
			VRenderView* vrv = vr_frame->GetView(i);
			if (vrv && m_views_cmb)
			{
				m_views_cmb->Append(vrv->GetName());
			}
		}
		if (i)
			m_views_cmb->Select(0);

		GetSettings(0);
	}
}

void VMovieView::AddView(wxString view)
{
	if (m_views_cmb)
		m_views_cmb->Append(view);
}

void VMovieView::DeleteView(wxString view)
{
	if (!m_views_cmb)
		return;
	int cur_sel = m_views_cmb->GetCurrentSelection();
	int del = m_views_cmb->FindString(view, true);
	if (del != wxNOT_FOUND)
		m_views_cmb->Delete(del);
	if (cur_sel == del)
		m_views_cmb->Select(0);
}

void VMovieView::SetView(int index)
{
	if (m_views_cmb)
	{
		m_views_cmb->SetSelection(index);
	}
}

void VMovieView::ResetAngle()
{
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			vrv->ResetMovieAngle();
			if (m_movie_type == 3 || m_movie_type == 4)
			{
				vrv->Set4DSeqFrame(m_reset_time_frame, false);
				SetTimeFrame(m_reset_time_frame);
				m_reset_time_frame = 1;
			}
			else if (m_movie_type == 2)
			{
				vrv->Set3DBatFrame(m_reset_time_frame-m_prev_frame);
				SetTimeFrame(m_reset_time_frame);
				m_reset_time_frame = 1;
			}
		}
	}
}

void VMovieView::OnPrev(wxCommandEvent& event)
{
	wxString str;
	double start_angle;
	double end_angle;
	double step;
	int rot_axis = 2;
	bool rewind;

	str = m_angle_start_text->GetValue();
	start_angle = -fabs(STOD(str.fn_str()));
	str = m_angle_end_text->GetValue();
	end_angle = STOD(str.fn_str());
	str = m_step_text->GetValue();
	step = STOD(str.fn_str());
	if (step < 0.01)
		step = 0.01;
	if (m_x_rd->GetValue())
		rot_axis = 1;
	if (m_y_rd->GetValue())
		rot_axis = 2;
	rewind = m_rewind_chk->GetValue();
	int frames = CalcFrames(step, start_angle, end_angle);
	str = wxString::Format("%d", frames);
	int length = str.length();

	if (end_angle == 0.0 && !rewind)
		end_angle = start_angle;

	int begin_frame = GetTimeFrame();
	int end_frame = GetEndTime();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			wxString str_null = "";

			switch (m_movie_type)
			{
			case 1:
				vrv->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, str_null, rewind, length);
				break;
			case 2:
				vrv->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, str_null, rewind, length);
				vrv->Set3DBatCapture(str_null, begin_frame, end_frame);
				break;
			case 3:
				vrv->Set4DSeqCapture(str_null, begin_frame, end_frame, true);
				break;
			case 4:
				vrv->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, str_null, rewind, length);
				vrv->Set4DSeqCapture(str_null, begin_frame, end_frame, false);
				break;
			}
		}
	}

	m_reset_time_frame = GetTimeFrame();
}

//ch1
void VMovieView::OnCh1Check(wxCommandEvent &event)
{
	wxCheckBox* ch1 = (wxCheckBox*)event.GetEventObject();
	if (ch1)
		VRenderFrame::SetCompression(ch1->GetValue());
}

//embde project
void VMovieView::OnChEmbedCheck(wxCommandEvent &event)
{
	wxCheckBox* ch_embed = (wxCheckBox*)event.GetEventObject();
	if (ch_embed)
		VRenderFrame::SetEmbedProject(ch_embed->GetValue());
}

wxWindow* VMovieView::CreateExtraCaptureControl(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, 0, wxDefaultPosition, wxSize(400, 90));

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(panel, wxID_ANY, "Additional Options"), wxVERTICAL);

	//compressed
	wxCheckBox* ch1 = new wxCheckBox(panel, wxID_HIGHEST+3004,
		"Lempel-Ziv-Welch Compression");
	ch1->Connect(ch1->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
		wxCommandEventHandler(VMovieView::OnCh1Check), NULL, panel);
	if (ch1)
		ch1->SetValue(VRenderFrame::GetCompression());

	//copy all files check box
	wxCheckBox* ch_embed = 0;
	if (VRenderFrame::GetSaveProject())
	{
		ch_embed = new wxCheckBox(panel, wxID_HIGHEST+3005,
			"Embed all files in the project folder");
		ch_embed->Connect(ch_embed->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(VMovieView::OnChEmbedCheck), NULL, panel);
		ch_embed->SetValue(VRenderFrame::GetEmbedProject());
	}

	//group
	group1->Add(10, 10);
	group1->Add(ch1);
	group1->Add(10, 10);
	if (VRenderFrame::GetSaveProject() &&
		ch_embed)
	{
		group1->Add(ch_embed);
		group1->Add(10, 10);
	}

	panel->SetSizer(group1);
	panel->Layout();

	return panel;
}

void VMovieView::OnRun(wxCommandEvent& event)
{
	wxString str;
	double start_angle;
	double end_angle;
	double step;
	int rot_axis = 2;
	bool rewind;

	str = m_angle_start_text->GetValue();
	start_angle = -fabs(STOD(str.fn_str()));
	str = m_angle_end_text->GetValue();
	end_angle = STOD(str.fn_str());
	str = m_step_text->GetValue();
	step = STOD(str.fn_str());
	if (step < 0.01)
		step = 0.01;
	if (m_x_rd->GetValue())
		rot_axis = 1;
	if (m_y_rd->GetValue())
		rot_axis = 2;
	rewind = m_rewind_chk->GetValue();
	int frames = CalcFrames(step, start_angle, end_angle);
	str = wxString::Format("%d", frames);
	int length = str.length();

	if (end_angle == 0.0 && !rewind)
		end_angle = start_angle;

	int begin_frame = GetStartTime();
	int end_frame = GetEndTime();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		if (vr_frame->GetSettingDlg())
			VRenderFrame::SetSaveProject(vr_frame->GetSettingDlg()->GetProjSave());

		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			wxFileDialog *fopendlg = new wxFileDialog(
				this, "Save Movie Sequence", 
				"", "", "*.tif", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
			fopendlg->SetExtraControlCreator(CreateExtraCaptureControl);

			int rval = fopendlg->ShowModal();
			if (rval == wxID_OK)
			{
				wxString filename = fopendlg->GetPath();
				switch (m_movie_type)
				{
				case 1:
					vrv->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, filename, rewind, length);
					break;
				case 2:
					vrv->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, filename, rewind, length);
					vrv->Set3DBatCapture(filename, 0, end_frame);
					break;
				case 3:
					vrv->Set4DSeqCapture(filename, begin_frame, end_frame, false);
					break;
				case 4:
					vrv->Set3DRotCapture(start_angle, end_angle, step, frames, rot_axis, filename, rewind, length);
					vrv->Set4DSeqCapture(filename, begin_frame, end_frame, false);
					break;
				}

				if (vr_frame->GetSettingDlg() &&
					vr_frame->GetSettingDlg()->GetProjSave())
				{
					wxString new_folder;
					new_folder = filename + "_project";
					CREATE_DIR(new_folder.fn_str());
					wxString prop_file = new_folder + "/" + fopendlg->GetFilename() + "_project.vrp";
					vr_frame->SaveProject(prop_file);
				}
			}

			delete fopendlg;
		}
	}

	m_reset_time_frame = GetTimeFrame();
}

void VMovieView::OnStop(wxCommandEvent& event)
{
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
			vrv->StopMovie();
	}
}

void VMovieView::OnResetAngle(wxCommandEvent &event)
{
	ResetAngle();
}

int VMovieView::CalcFrames(double step, double start, double end)
{
	int result = 0;
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			double x, y, z;
			vrv->GetRotations(x, y, z);
			double current;
			if (m_x_rd->GetValue())
				current = x;
			if (m_y_rd->GetValue())
				current = y;

			if (start < 0 && int(end) ==0)
			{
				if (m_rewind_chk->GetValue())
					result = int(fabs(start)*2.0/step+0.5);
				else
					result = int(fabs(start)/step+0.5);
			}
			else
			{
				if (m_rewind_chk->GetValue())
					result = int((fabs(start)+fabs(start-end)+fabs(end))/step+0.5);
				else
					result = int((fabs(start)+fabs(start-end))/step+0.5);
			}
		}
	}

	return result;
}

double VMovieView::CalcStep(int frames, double start, double end)
{
	double result = 0.01;
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			double x, y, z;
			vrv->GetRotations(x, y, z);
			double current;
			if (m_x_rd->GetValue())
				current = x;
			if (m_y_rd->GetValue())
				current = y;

			if (start < 0 && int(end) == 0)
			{
				if (m_rewind_chk->GetValue())
					result = fabs(start)*2.0/frames;
				else
					result = fabs(start)/frames;
			}
			else
			{
				if (m_rewind_chk->GetValue())
					result = (fabs(start)+fabs(start-end)+fabs(end))/frames;
				else
					result = (fabs(start)+fabs(start-end))/frames;
			}
		}
	}

	return result;
}

//
void VMovieView::OnFrameCheck(wxCommandEvent& event)
{
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			if (m_frame_chk->GetValue())
			{
				int x, y, w, h;
				vrv->GetFrame(x, y, w, h);
				if (w<0||h<0)
				{
					w = vrv->GetGLSize().x;
					h = vrv->GetGLSize().y;
					int size;
					if (w > h)
					{
						size = h;
						x = int((w-h)/2.0);
						y = 0;
					}
					else
					{
						size = w;
						x = 0;
						y = int((h-w)/2.0);
					}
					vrv->SetFrame(x, y, size, size);

				}
				vrv->GetFrame(x, y, w, h);
				m_center_x_text->SetValue(wxString::Format("%d", int(x+w/2.0+0.5)));
				m_center_y_text->SetValue(wxString::Format("%d", int(y+h/2.0+0.5)));
				m_width_text->SetValue(wxString::Format("%d", w));
				m_height_text->SetValue(wxString::Format("%d", h));
				vrv->EnableFrame();
			}
			else
				vrv->DisableFrame();
				
			vrv->RefreshGL();
		}
	}
}

void VMovieView::OnResetFrame(wxCommandEvent& event)
{
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			int x, y, w, h;

			w = vrv->GetGLSize().x;
			h = vrv->GetGLSize().y;
			int size;
			if (w > h)
			{
				size = h;
				x = int((w-h)/2.0);
				y = 0;
			}
			else
			{
				size = w;
				x = 0;
				y = int((h-w)/2.0);
			}
			vrv->SetFrame(x, y, size, size);

			vrv->GetFrame(x, y, w, h);
			m_center_x_text->SetValue(wxString::Format("%d", int(x+w/2.0+0.5)));
			m_center_y_text->SetValue(wxString::Format("%d", int(y+h/2.0+0.5)));
			m_width_text->SetValue(wxString::Format("%d", w));
			m_height_text->SetValue(wxString::Format("%d", h));
			vrv->RefreshGL();
		}
	}
}

void VMovieView::OnFrameEditing(wxCommandEvent& event)
{
	wxString str;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		str = m_views_cmb->GetValue();
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			int x, y, w, h;
			wxString temp;

			temp = m_center_x_text->GetValue();
			x = STOI(temp.fn_str());
			temp = m_center_y_text->GetValue();
			y = STOI(temp.fn_str());
			temp = m_width_text->GetValue();
			w = STOI(temp.fn_str());
			temp = m_height_text->GetValue();
			h = STOI(temp.fn_str());

			vrv->SetFrame(int(x-w/2.0+0.5), int(y-h/2.0+0.5), w, h);
			if (vrv->GetFrameEnabled())
				vrv->RefreshGL();
		}
	}
}

void VMovieView::OnFrameSpinUp(wxSpinEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_CenterXSpin:
		text_ctrl = m_center_x_text;
		break;
	case ID_CenterYSpin:
		text_ctrl = m_center_y_text;
		break;
	case ID_WidthSpin:
		text_ctrl = m_width_text;
		break;
	case ID_HeightSpin:
		text_ctrl = m_height_text;
		break;
	}
	if (text_ctrl)
	{
		wxString str_val = text_ctrl->GetValue();
		char str[256];
		sprintf(str, "%d", STOI(str_val.fn_str())+1);
		text_ctrl->SetValue(str);
		wxCommandEvent e;
		OnFrameEditing(e);
	}
}

void VMovieView::OnFrameSpinDown(wxSpinEvent& event)
{
	int sender_id = event.GetId();
	wxTextCtrl* text_ctrl = 0;
	switch (sender_id)
	{
	case ID_CenterXSpin:
		text_ctrl = m_center_x_text;
		break;
	case ID_CenterYSpin:
		text_ctrl = m_center_y_text;
		break;
	case ID_WidthSpin:
		text_ctrl = m_width_text;
		break;
	case ID_HeightSpin:
		text_ctrl = m_height_text;
		break;
	}
	if (text_ctrl)
	{
		wxString str_val = text_ctrl->GetValue();
		char str[256];
		sprintf(str, "%d", STOI(str_val.fn_str())-1);
		text_ctrl->SetValue(str);
		wxCommandEvent e;
		OnFrameEditing(e);
	}
}

void VMovieView::OnMtChecked(wxCommandEvent &event)
{
	if (m_mt_3d_rot_rd->GetValue())
		SetMovieType(1);
	else if (m_mt_3d_bat_rd->GetValue())
		SetMovieType(2);
	else if (m_mt_4d_seq_rd->GetValue())
		SetMovieType(3);
	else if (m_mt_4d_rot_rd->GetValue())
		SetMovieType(4);
}

void VMovieView::DisableRot()
{
	m_x_rd->Disable();
	m_y_rd->Disable();
	m_angle_start_text->Disable();
	m_angle_end_text->Disable();
	m_step_text->Disable();
	m_frames_text->Disable();
	m_rewind_chk->Disable();
}

void VMovieView::EnableRot()
{
	m_x_rd->Enable();
	m_y_rd->Enable();
	m_angle_start_text->Enable();
	m_angle_end_text->Enable();
	m_step_text->Enable();
	m_frames_text->Enable();
	m_rewind_chk->Enable();
}

void VMovieView::OnHelpBtn(wxCommandEvent &event)
{
	::wxLaunchDefaultBrowser(HELP_MOVIE);
}

void VMovieView::Get4DFrames()
{
	wxString str = m_views_cmb->GetValue();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			int start_frame = 0;
			int end_frame = 0;
			int cur_frame = 0;
			vrv->Get4DSeqFrames(start_frame, end_frame, cur_frame);
			m_time_sldr->SetRange(start_frame, end_frame);
			m_time_sldr->SetValue(cur_frame);
			m_time_text->SetValue(wxString::Format("%d", cur_frame));
			m_time_start_text->SetValue(wxString::Format("%d", start_frame));
			m_time_end_text->SetValue(wxString::Format("%d", end_frame));
			m_prev_frame = cur_frame;
		}
	}
}

void VMovieView::Get3DFrames()
{
	wxString str = m_views_cmb->GetValue();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			int start_frame = 0;
			int end_frame = 0;
			int cur_frame = 0;
			vrv->Get3DBatFrames(start_frame, end_frame, cur_frame);
			m_time_sldr->SetRange(start_frame, end_frame);
			m_time_sldr->SetValue(cur_frame);
			m_time_text->SetValue(wxString::Format("%d", cur_frame));
			m_time_start_text->SetValue(wxString::Format("%d", start_frame));
			m_time_end_text->SetValue(wxString::Format("%d", end_frame));
			m_prev_frame = cur_frame;
		}
	}
}

void VMovieView::DisableTime()
{
	m_time_sldr->Disable();
	m_time_spin->Disable();
	m_time_text->Disable();
}

void VMovieView::EnableTime()
{
	m_time_sldr->Enable();
	m_time_spin->Enable();
	m_time_text->Enable();
}

void VMovieView::DisableRange()
{
	m_time_start_text->Disable();
	m_time_end_text->Disable();
}

void VMovieView::EnableRange()
{
	m_time_start_text->Enable();
	m_time_end_text->Enable();
}

void VMovieView::OnTimeChange(wxScrollEvent &event)
{
	int frame = event.GetPosition();
	m_time_text->SetValue(wxString::Format("%d", frame));
	wxString str = m_views_cmb->GetValue();

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			switch (m_movie_type)
			{
			case 1:
				break;
			case 2:
				vrv->Set3DBatFrame(frame-m_prev_frame);
				break;
			case 3:
				vrv->Set4DSeqFrame(frame, true);
				break;
			case 4:
				vrv->Set4DSeqFrame(frame, true);
				break;
			}
		}
	}
	m_prev_frame = frame;
}

void VMovieView::OnTimeSpinUp(wxSpinEvent &event)
{
	int iVal = m_time_sldr->GetValue();
	iVal++;
	m_time_sldr->SetValue(iVal);
	iVal = m_time_sldr->GetValue();
	m_time_text->SetValue(wxString::Format("%d", iVal));

	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			switch (m_movie_type)
			{
			case 1:
				break;
			case 2:
				vrv->Set3DBatFrame(iVal-m_prev_frame);
				break;
			case 3:
				vrv->Set4DSeqFrame(iVal ,true);
				break;
			case 4:
				vrv->Set4DSeqFrame(iVal, true);
				break;
			}
		}
	}
	m_prev_frame = iVal;
}

void VMovieView::OnTimeSpinDown(wxSpinEvent &event)
{
	int iVal = m_time_sldr->GetValue();
	iVal--;
	m_time_sldr->SetValue(iVal);
	iVal = m_time_sldr->GetValue();
	m_time_text->SetValue(wxString::Format("%d", iVal));

	wxString str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			switch (m_movie_type)
			{
			case 1:
				break;
			case 2:
				vrv->Set3DBatFrame(iVal-m_prev_frame);
				break;
			case 3:
				vrv->Set4DSeqFrame(iVal ,true);
				break;
			case 4:
				vrv->Set4DSeqFrame(iVal, true);
				break;
			}
		}
	}
	m_prev_frame = iVal;
}

void VMovieView::OnTimeEnter(wxCommandEvent& event)
{
	wxString str = m_time_text->GetValue();
	long iVal;
	str.ToLong(&iVal);
	m_time_sldr->SetValue(iVal);
	iVal = m_time_sldr->GetValue();
	m_time_text->SetValue(wxString::Format("%d", iVal));

	str = m_views_cmb->GetValue();
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		VRenderView* vrv = vr_frame->GetView(str);
		if (vrv)
		{
			switch (m_movie_type)
			{
			case 1:
				break;
			case 2:
				vrv->Set3DBatFrame(iVal-m_prev_frame);
				break;
			case 3:
				vrv->Set4DSeqFrame(iVal ,true);
				break;
			case 4:
				vrv->Set4DSeqFrame(iVal, true);
				break;
			}
		}
	}
	m_prev_frame = iVal;
}

void VMovieView::SetTimeFrame(int frame)
{
	m_time_sldr->SetValue(frame);
	int iVal = m_time_sldr->GetValue();
	m_time_text->SetValue(wxString::Format("%d", iVal));
	m_prev_frame = frame;
}

void VMovieView::SetMovieType(int type)
{
	m_mt_3d_rot_rd->SetValue(false);
	m_mt_3d_bat_rd->SetValue(false);
	m_mt_4d_seq_rd->SetValue(false);
	m_mt_4d_rot_rd->SetValue(false);

	switch (type)
	{
	case 1:
		m_movie_type = 1;
		EnableRot();
		DisableRange();
		DisableTime();
		m_mt_3d_rot_rd->SetValue(true);
		break;
	case 2:
		m_movie_type = 2;
		Get3DFrames();
		EnableRot();
		EnableTime();
		DisableRange();
		m_mt_3d_bat_rd->SetValue(true);
		break;
	case 3:
		m_movie_type = 3;
		Get4DFrames();
		DisableRot();
		EnableTime();
		EnableRange();
		m_mt_4d_seq_rd->SetValue(true);
		break;
	case 4:
		m_movie_type = 4;
		Get4DFrames();
		EnableRot();
		EnableTime();
		EnableRange();
		m_mt_4d_rot_rd->SetValue(true);
		break;
	}
}
