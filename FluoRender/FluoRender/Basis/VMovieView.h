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
#ifndef _VMovieView_H_
#define _VMovieView_H_

#include <compatibility.h>
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/clrpicker.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

#define PROG_SLDR_MAX	361

class VRenderFrame;
class VRenderGLView;
class RecorderDlg;
class VMovieView : public wxPanel
{
	enum
	{
		//time sequence
		ID_SeqChk = ID_MOVIEW_VIEW,
		ID_BatChk,//batch mutual exclusive with seq(4d)
		ID_IncTimeBtn,
		ID_DecTimeBtn,
		ID_StartFrameText,
		ID_EndFrameText,
		ID_CurFrameText,
		//movie len
		ID_MovieLenText,

		//rotations
		ID_RotChk,
		ID_XRd,
		ID_YRd,
		ID_ZRd,
		ID_DegreeText,
		ID_RotIntCmb,

		//bit rate
		ID_BitrateText,

		//fps, view combo, help
		ID_FPS_Text,
		ID_ViewsCombo,

		//main controls
		ID_PlayPause,
		ID_Rewind,
		ID_ProgressSldr,
		ID_ProgressText,
		ID_SaveMovie,

		//4d script
		ID_RunScriptChk,
		ID_ScriptFileText,
		ID_ScriptClearBtn,
		ID_ScriptFileBtn,
		ID_ScriptList,

		//auto key
		ID_AutoKeyList,
		ID_GenKeyBtn,

		//cropping
		ID_CropChk,
		ID_ResetBtn,
		ID_CenterXText,
		ID_CenterYText,
		ID_CenterXSpin,
		ID_CenterYSpin,
		ID_WidthText,
		ID_HeightText,
		ID_WidthSpin,
		ID_HeightSpin,
		ID_Timer,

		//notebook
		ID_Notebook
	};

public:
	VMovieView(VRenderFrame* frame,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "VMovieView");
	~VMovieView();

	void AddView(wxString view);
	void DeleteView(wxString view);
	void SetView(int index);
	int GetView() { return m_view_idx; }

	void UpFrame();
	void DownFrame();
	void SetCurrentTime(size_t t);

	void SetRotate(bool value)
	{
		m_rotate = value;
		if (m_rotate)
		{
			m_x_rd->Enable();
			m_y_rd->Enable();
			m_z_rd->Enable();
			m_degree_text->Enable();
			m_rot_chk->SetValue(true);
		}
		else
		{
			m_x_rd->Disable();
			m_y_rd->Disable();
			m_z_rd->Disable();
			m_degree_text->Disable();
			m_rot_chk->SetValue(false);
		}
	}
	bool GetRotate() { return m_rotate; }
	void SetTimeSeq(bool value);
	bool GetTimeSeq() { return m_time_seq; }
	void SetRotAxis(int value)
	{
		m_rot_axis = value;
		if (m_rot_axis == 0)
			m_x_rd->SetValue(true);
		else if (m_rot_axis == 1)
			m_y_rd->SetValue(true);
		else if (m_rot_axis == 2)
			m_z_rd->SetValue(true);
	}
	int GetRotAxis() { return m_rot_axis; }
	void SetRotDeg(int value)
	{
		m_rot_deg = value;
		m_degree_text->SetValue(wxString::Format("%d", m_rot_deg));
	}
	int GetRotDeg() { return m_rot_deg; }
	void SetMovieLen(double value)
	{
		m_movie_len = value;
		m_movie_len_text->SetValue(wxString::Format("%.2f", m_movie_len));
	}
	double GetMovieLen() { return m_movie_len; }
	void SetFps(double value)
	{
		m_fps = value;
		m_fps_text->SetValue(wxString::Format("%.0f", m_fps));
	}
	double GetFps() { return m_fps; }
	//frames
	void SetStartFrame(int value)
	{
		m_start_frame = value;
		m_start_frame_text->SetValue(wxString::Format("%d", m_start_frame));
	}
	int GetStartFrame() { return m_start_frame; }
	void SetEndFrame(int value)
	{
		m_end_frame = value;
		m_end_frame_text->SetValue(wxString::Format("%d", m_end_frame));
	}
	int GetEndFrame() { return m_end_frame; }
	//cropping
	void SetCrop(bool value);
	bool GetCrop() { return m_crop; }
	void SetCropX(int value) { m_crop_x = value; }
	int GetCropX() { return m_crop_x; }
	void SetCropY(int value) { m_crop_y = value; }
	int GetCropY() { return m_crop_y; }
	void SetCropW(int value) { m_crop_w = value; }
	int GetCropW() { return m_crop_w; }
	void SetCropH(int value) { m_crop_h = value; }
	int GetCropH() { return m_crop_h; }
	void UpdateCrop();

	void SetBitRate(double value) { m_Mbitrate = value; }
	void SetFileName(wxString &filename) { m_filename = filename; }
	void Run();
	bool GetRunning() { return m_running; }
	void GetScriptSettings(bool sel);
	int GetCurrentPage() { return m_current_page; }
	void SetCurrentPage(int page);

	//play
	void Prev();
	void Stop();
	void Rewind();
	void Reset();
	//timer
	void TimerRun()
	{
		m_timer.Start(int(1000.0 / m_fps + 0.5));
	}
	void ResumeRun()
	{
		if (m_timer_hold)
		{
			TimerRun();
			m_timer_hold = false;
		}
	}
	void HoldRun()
	{
		if (!m_timer_hold && m_timer.IsRunning())
		{
			m_timer_hold = true;
			m_timer.Stop();
		}
	}

private:
	//controls
	wxTextCtrl *m_fps_text;
	wxComboBox *m_views_cmb;

	wxButton *m_play_btn;
	wxButton *m_rewind_btn;
	wxSlider *m_progress_sldr;
	wxTextCtrl *m_progress_text;
	wxButton *m_save_btn;

	//basic movie controls
	wxCheckBox *m_seq_chk;
	wxCheckBox *m_bat_chk;
	wxButton *m_inc_time_btn;
	wxButton *m_dec_time_btn;
	wxTextCtrl *m_cur_frame_text;
	wxTextCtrl *m_start_frame_text;
	wxTextCtrl *m_end_frame_text;
	wxTextCtrl *m_movie_len_text;

	wxCheckBox *m_rot_chk;
	wxRadioButton *m_x_rd;
	wxRadioButton *m_y_rd;
	wxRadioButton *m_z_rd;
	wxTextCtrl *m_degree_text;
	wxComboBox *m_rot_int_cmb;

	//script
	wxCheckBox *m_run_script_chk;
	wxTextCtrl *m_script_file_text;
	wxButton *m_script_clear_btn;
	wxButton *m_script_file_btn;
	wxListCtrl *m_script_list;

	//auto key controls
	wxListCtrl* m_auto_key_list;
	wxButton* m_gen_keys_btn;

	//cropping
	wxCheckBox *m_crop_chk;
	wxButton *m_reset_btn;

	wxTextCtrl *m_center_x_text;
	wxSpinButton* m_center_x_spin;
	wxTextCtrl *m_center_y_text;
	wxSpinButton* m_center_y_spin;

	wxTextCtrl *m_width_text;
	wxSpinButton* m_width_spin;
	wxTextCtrl *m_height_text;
	wxSpinButton* m_height_spin;

	static wxTextCtrl *m_estimated_size_text;
	static double m_Mbitrate;

private:
	VRenderFrame* m_frame;
	VRenderGLView* m_view;
	RecorderDlg* m_advanced_movie;
	int m_view_idx;//index to current renderview

	wxNotebook* m_notebook;
	int m_current_page;
	wxTimer m_timer;

	//basic
	bool m_rotate;
	bool m_time_seq;
	int m_seq_mode;//0:none; 1:4d; 2:bat
	int m_rot_axis;	//0-x;1-y;2-z
	int m_rot_deg;
	int m_rot_int_type;//0-linear; 1-smooth
	static double m_movie_len;//length in sec
	double m_cur_time;//time in sec
	double m_fps;
	int m_start_frame;
	int m_end_frame;
	int m_cur_frame;

	//cropping
	bool m_crop;//enable cropping
	int m_crop_x;
	int m_crop_y;
	int m_crop_w;
	int m_crop_h;

	//save controls
	int m_last_frame;//last frame nunmber to save
	double m_starting_rot;//starting degree of rotation
	bool m_running, m_record;
	bool m_delayed_stop;
	bool m_timer_hold;//for temporary hold
	//save
	wxString m_filename;
	wxString filetype_;

private:
	void GetSettings();
	int GetScriptFiles(wxArrayString& list);
	void AddScriptToList();

	//set the renderview and progress bars/text
	void SetRendering(double pcnt, bool rewind=false);
	void SetProgress(double pcnt);

	//write frames to file
	void WriteFrameToFile(int total_frames);

private:
	wxWindow* CreateSimplePage(wxWindow *parent);
	wxWindow* CreateAdvancedPage(wxWindow *parent);
	wxWindow* CreateAutoKeyPage(wxWindow *parent);
	wxWindow* CreateCroppingPage(wxWindow *parent);
	wxWindow* CreateScriptPage(wxWindow *parent);
	void Init();
	void GenKey();

	//basic rotation
	void OnRotateChecked(wxCommandEvent& event);
	void OnRotAxis(wxCommandEvent& event);
	void OnDegreeText(wxCommandEvent &event);
	//rotation interpolation
	void OnRotIntCmb(wxCommandEvent& event);

	//left column
	void OnRun(wxCommandEvent& event);
	void OnPrev(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);
	void OnRewind(wxCommandEvent& event);

	void OnViewSelected(wxCommandEvent& event);

	//right column
	void OnCropCheck(wxCommandEvent& event);
	void OnResetCrop(wxCommandEvent& event);
	void OnEditCrop(wxCommandEvent& event);

	void OnCropSpinUp(wxSpinEvent& event);
	void OnCropSpinDown(wxSpinEvent& event);

	//help
	void OnChEnlargeCheck(wxCommandEvent &event);
	void OnSlEnlargeScroll(wxScrollEvent &event);
	void OnTxEnlargeText(wxCommandEvent &event);
	void OnMovieQuality(wxCommandEvent &event);
	void OnCh1Check(wxCommandEvent &event);
	void OnCh2Check(wxCommandEvent &event);
	void OnCh3Check(wxCommandEvent &event);
	void OnDpiText(wxCommandEvent& event);
	void OnChEmbedCheck(wxCommandEvent &event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);

	//script
	void OnRunScriptChk(wxCommandEvent &event);
	void OnScriptFileEdit(wxCommandEvent &event);
	void OnScriptClearBtn(wxCommandEvent &event);
	void OnScriptFileBtn(wxCommandEvent &event);
	void OnScriptListSelected(wxListEvent &event);

	//auto key
	void OnListItemAct(wxListEvent &event);
	void OnGenKey(wxCommandEvent& event);

	//time slider
	void OnTimeChange(wxScrollEvent &event);
	void OnTimeText(wxCommandEvent& event);
	void OnUpFrame(wxCommandEvent& event);
	void OnDownFrame(wxCommandEvent& event);
	void OnCurFrameText(wxCommandEvent& event);
	void OnStartFrameText(wxCommandEvent& event);
	void OnEndFrameText(wxCommandEvent& event);
	void OnMovieLenText(wxCommandEvent& event);
	void OnFpsEdit(wxCommandEvent& event);

	//checkboxes
	void OnSequenceChecked(wxCommandEvent& event);
	void OnBatchChecked(wxCommandEvent& event);

	//timer for playback.
	void OnTimer(wxTimerEvent& event);

	//notebook page change
	void OnNbPageChange(wxBookCtrlEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_VMovieView_H_
