#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/clrpicker.h>
#include "compatibility.h"

#ifndef _VMovieView_H_
#define _VMovieView_H_

class VMovieView : public wxPanel
{
	enum
	{
		ID_ViewsCombo = wxID_HIGHEST+701,
		ID_Mt3dRotRd,
		ID_Mt3dBatRd,
		ID_Mt4dSeqRd,
		ID_Mt4dRotRd,
		//time slider
		ID_TimeSldr,
		ID_TimeSpin,
		ID_TimeText,
		ID_TimeStartText,
		ID_TimeEndText,
		//rotations
		ID_XRd,
		ID_YRd,
		ID_AngleStartText,
		ID_AngleEndText,
		ID_CurAngleText,
		ID_RewindChk,
		ID_StepText,
		ID_FramesText,
		ID_RunBtn,
		ID_FrameChk,
		ID_CenterXText,
		ID_CenterYText,
		ID_WidthText,
		ID_HeightText,
		ID_ResetBtn,
		ID_CenterXSpin,
		ID_CenterYSpin,
		ID_WidthSpin,
		ID_HeightSpin,
		ID_PrevBtn,
		ID_StopBtn,
		ID_ResetAngleBtn,
		//help
		ID_HelpBtn
	};

public:
	VMovieView(wxWindow* frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "VMovieView");
	~VMovieView();

	void AddView(wxString view);
	void DeleteView(wxString view);
	void SetView(int index);

	void SetTimeFrame(int frame);
	int GetTimeFrame()
	{
		return m_time_sldr->GetValue();
	}
	int GetStartTime()
	{
		return int(STOD(m_time_start_text->GetValue().fn_str()));
	}
	int GetEndTime()
	{
		return int(STOD(m_time_end_text->GetValue().fn_str()));
	}
	int GetMovieType()
	{
		return m_movie_type;
	}
	void SetMovieType(int type);
	int CalcFrames(double step, double start, double end);
	void ResetAngle();

public:
	//controls
	//1
	wxComboBox *m_views_cmb;
	wxButton *m_help_btn;
	//2
	wxRadioButton *m_mt_3d_rot_rd;
	wxRadioButton *m_mt_3d_bat_rd;
	wxRadioButton *m_mt_4d_seq_rd;
	wxRadioButton *m_mt_4d_rot_rd;
	//2.5
	wxSlider *m_time_sldr;
	wxSpinButton *m_time_spin;
	wxTextCtrl *m_time_text;
	wxTextCtrl *m_time_start_text;
	wxTextCtrl *m_time_end_text;
	//3
	wxRadioButton *m_x_rd;
	wxRadioButton *m_y_rd;
	//4
	//5
	wxTextCtrl *m_angle_start_text;
	wxTextCtrl *m_angle_end_text;
	//6
	wxTextCtrl *m_step_text;
	wxTextCtrl *m_frames_text;
	wxCheckBox *m_rewind_chk;
	//7
	wxButton *m_prev_btn;
	wxButton *m_stop_btn;
	wxButton *m_reset_angle_btn;
	wxButton *m_run_btn;
	//8
	wxCheckBox *m_frame_chk;
	wxButton *m_reset_btn;
	//9
	wxTextCtrl *m_center_x_text;
	wxSpinButton* m_center_x_spin;
	wxTextCtrl *m_center_y_text;
	wxSpinButton* m_center_y_spin;
	//10
	wxTextCtrl *m_width_text;
	wxSpinButton* m_width_spin;
	wxTextCtrl *m_height_text;
	wxSpinButton* m_height_spin;

private:
	bool m_init;
	wxWindow* m_frame;

	bool m_rewind;
	//bool m_mt_rot;
	//bool m_mt_4d;
	int m_movie_type;
	//1-3D rotation; 2-3D batch; 3-4D; 4-4D rotation
	int m_reset_time_frame;
	int m_prev_frame;

private:
	void GetSettings(int view=0);
	double CalcStep(int frames, double start, double end);

	void DisableRot();
	void EnableRot();
	void DisableTime();
	void EnableTime();
	void DisableRange();
	void EnableRange();

	//4d movie slider
	void Get4DFrames();

	//3d batch
	void Get3DFrames();

private:
	wxWindow* CreateSimplePage(wxWindow *parent);
	wxWindow* CreateAdvancedPage(wxWindow *parent);
	wxWindow* CreateCroppingPage(wxWindow *parent);
	void Init();

	//left column
	void OnCh1Check(wxCommandEvent &event);
	void OnChEmbedCheck(wxCommandEvent &event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
	void OnRun(wxCommandEvent& event);
	void OnPrev(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnResetAngle(wxCommandEvent& event);

	void OnViewSelected(wxCommandEvent& event);
	void OnAxisSelected(wxCommandEvent& event);
	void OnStartAngleEditing(wxCommandEvent& event);
	void OnEndAngleEditing(wxCommandEvent& event);
	void OnFramesEditing(wxCommandEvent& event);
	void OnStepEditing(wxCommandEvent& event);
	void OnRewindChecked(wxCommandEvent& event);

	void OnMtChecked(wxCommandEvent& event);

	//right column
	void OnFrameCheck(wxCommandEvent& event);
	void OnResetFrame(wxCommandEvent& event);
	void OnFrameEditing(wxCommandEvent& event);

	void OnFrameSpinUp(wxSpinEvent& event);
	void OnFrameSpinDown(wxSpinEvent& event);

	//help
	void OnHelpBtn(wxCommandEvent& event);

	//time slider
	void OnTimeChange(wxScrollEvent &event);
	void OnTimeSpinUp(wxSpinEvent& event);
	void OnTimeSpinDown(wxSpinEvent& event);
	void OnTimeEnter(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_VMovieView_H_
