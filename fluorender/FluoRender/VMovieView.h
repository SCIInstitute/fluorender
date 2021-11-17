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

#include "compatibility.h"
#include "QVideoEncoder.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/clrpicker.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>

class VRenderFrame;
class VRenderView;
class RecorderDlg;
class VMovieView : public wxPanel
{
	enum
	{
		//time sequence
		ID_SeqChk = ID_MOVIEW_VIEW,
		ID_BatChk,//batch mutual exclusive with seq(4d)
		ID_TimeStartText,
		ID_TimeEndText,
		ID_IncTimeBtn,
		ID_DecTimeBtn,
		ID_CurrentTimeText,
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
		ID_FrameChk,
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
	void SetMovAxis(int value)
	{
		m_mov_axis = value;
		if (m_mov_axis == 0)
			m_x_rd->SetValue(true);
		else if (m_mov_axis == 1)
			m_y_rd->SetValue(true);
		else if (m_mov_axis == 2)
			m_z_rd->SetValue(true);
	}
	int GetMovAxis() { return m_mov_axis; }
	void SetDegree(int value)
	{
		m_degree = value;
		m_degree_text->SetValue(wxString::Format("%d", m_degree));
	}
	int GetDegree() { return m_degree; }

	void SetBitRate(double value) { m_Mbitrate = value; }
	void SetFileName(wxString &filename) { m_filename = filename; }
	void Run();
	bool GetRunning() { return m_running; }
	void GetScriptSettings();
	int GetCurrentPage() { return m_current_page; }
	void SetCurrentPage(int page);

	//timer
	void TimerRun()
	{
		m_timer.Start(int(1000.0 / m_fps + 0.5));
	}
	void ResumeRun()
	{
		if (m_timer_run)
			TimerRun();
	}
	void HoldRun()
	{
		if (m_timer.IsRunning())
		{
			m_timer_run = true;
			m_timer.Stop();
		}
		else
			m_timer_run = false;
	}

public:
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
	wxTextCtrl *m_time_start_text;
	wxTextCtrl *m_time_end_text;
	wxButton *m_inc_time_btn;
	wxButton *m_dec_time_btn;
	wxTextCtrl *m_time_current_text;
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

	static wxTextCtrl *m_estimated_size_text;
	static double m_Mbitrate;

private:
	VRenderFrame* m_frame;
	VRenderView* m_view;
	RecorderDlg* m_advanced_movie;
	int m_view_idx;//index to current renderview

	bool m_rotate;
	bool m_time_seq;
	int m_seq_mode;//0:none; 1:4d; 2:bat
	int m_mov_axis;	//0-x;1-y;2-z
	int m_degree;
	int m_rot_int_type;//0-linear; 1-smooth

	int m_last_frame;
	double m_starting_rot;
	wxTimer m_timer;
	double m_cur_time;
	wxString m_filename;
	bool m_running, m_record;
	wxNotebook* m_notebook;
	int m_current_page;
	QVideoEncoder encoder_;
	wxString filetype_;
	double m_fps;
	int m_start_time;
	int m_end_time;
	int m_current_time;
	static double m_movie_len;
	bool m_delayed_stop;
	bool m_timer_run;//for temporary hold

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
	void OnFrameCheck(wxCommandEvent& event);
	void OnResetFrame(wxCommandEvent& event);
	void OnFrameEditing(wxCommandEvent& event);

	void OnFrameSpinUp(wxSpinEvent& event);
	void OnFrameSpinDown(wxSpinEvent& event);

	//help
	void OnChEnlargeCheck(wxCommandEvent &event);
	void OnSlEnlargeScroll(wxScrollEvent &event);
	void OnTxEnlargeText(wxCommandEvent &event);
	void OnMovieQuality(wxCommandEvent &event);
	void OnCh1Check(wxCommandEvent &event);
	void OnCh2Check(wxCommandEvent &event);
	void OnCh3Check(wxCommandEvent &event);
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
	void OnTimeEnter(wxCommandEvent& event);
	void OnUpFrame(wxCommandEvent& event);
	void OnDownFrame(wxCommandEvent& event);
	void OnCurrentTimeText(wxCommandEvent& event);
	void OnTimeStartText(wxCommandEvent& event);
	void OnTimeEndText(wxCommandEvent& event);
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
