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
		//time sequence
		ID_SeqChk,
		ID_TimeStartText,
		ID_TimeEndText,

		//rotations
		ID_RotChk,
		ID_XRd,
		ID_YRd,
		ID_ZRd,
		ID_DegreeStartText,
		ID_DegreeEndText,

		//movie time
		ID_MovieTimeText,

		//fps, view combo, help
		ID_FPS_Text,
		ID_ViewsCombo = wxID_HIGHEST+701,
		ID_HelpBtn,

		//main controls
		ID_PlayPause,
		ID_Rewind,
		ID_ProgressSldr,
		ID_ProgressText,
		ID_SaveMovie,

		//cropping
		ID_FrameChk,
		ID_ResetBtn,
		ID_CenterXText,
		ID_CenterYText,
		ID_CenterXSpin,
		ID_CenterYSpin,
		ID_WidthText,
		ID_HeightText,
		ID_WidthSpin,
		ID_HeightSpin
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
	int GetProgress()
	{
		return m_progress_sldr->GetValue();
	}
	int GetStartTime()
	{
		return int(STOD(m_time_start_text->GetValue().fn_str()));
	}
	int GetEndTime()
	{
		return int(STOD(m_time_end_text->GetValue().fn_str()));
	}
	void MovieDone();
	void SetProgress(double pcnt);

public:
	//controls
	wxTextCtrl *m_fps_text;
	wxComboBox *m_views_cmb;
	wxButton *m_help_btn;

	wxButton *m_play_btn;
	wxButton *m_rewind_btn;
	wxSlider *m_progress_sldr;
	wxTextCtrl *m_progress_text;
	wxButton *m_save_btn;

	//basic movie controls
	wxCheckBox *m_seq_chk;
	wxTextCtrl *m_time_start_text;
	wxTextCtrl *m_time_end_text;

	
	wxCheckBox *m_rot_chk;
	wxRadioButton *m_x_rd;
	wxRadioButton *m_y_rd;
	wxRadioButton *m_z_rd;

	wxTextCtrl *m_degree_start;
	wxTextCtrl *m_degree_end;

	wxTextCtrl *m_movie_time;

	//cropping
	wxCheckBox *m_frame_chk;
	wxButton *m_reset_btn;

	wxTextCtrl *m_center_x_text;
	wxSpinButton* m_center_x_spin;
	wxTextCtrl *m_center_y_text;
	wxSpinButton* m_center_y_spin;

	wxTextCtrl *m_width_text;
	wxSpinButton* m_width_spin;
	wxTextCtrl *m_height_text;
	wxSpinButton* m_height_spin;

	bool m_running;


private:
	wxWindow* m_frame;
	int m_slider_pause_pos;
	double m_starting_rot;

private:
	void GetSettings(int view=0);
	void DisableRot();
	void EnableRot();
	void DisableTime();
	void EnableTime();

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
	void OnRewind(wxCommandEvent& event);

	void OnViewSelected(wxCommandEvent& event);

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
	void OnTimeEnter(wxCommandEvent& event);

	//checkboxes
	void OnSequenceChecked(wxCommandEvent& event);
	void OnRotateChecked(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_VMovieView_H_
