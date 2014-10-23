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
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/clrpicker.h>
#include <wx/notebook.h>
#include <cstring>
#include "RecorderDlg.h"
#include "compatibility.h"
#include "QVideoEncoder.h"

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
		ID_IncTimeBtn,
		ID_DecTimeBtn,
		ID_CurrentTimeText,

		//rotations
		ID_RotChk,
		ID_XRd,
		ID_YRd,
		ID_ZRd,
		ID_DegreeEndText,

		//movie time
		ID_MovieTimeText,

		//bit rate
		ID_BitrateText,

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
		ID_HeightSpin,
		ID_Timer
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
	void DisableRot();
	void EnableRot();
	void DisableTime();
	void EnableTime();

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
	wxButton *m_inc_time_btn;
	wxButton *m_dec_time_btn;
	wxTextCtrl *m_time_current_text;

	wxCheckBox *m_rot_chk;
	wxRadioButton *m_x_rd;
	wxRadioButton *m_y_rd;
	wxRadioButton *m_z_rd;

	wxTextCtrl *m_degree_end;

	wxTextCtrl *m_movie_time;
	wxTextCtrl *m_bitrate_text;

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

	wxTextCtrl *m_estimated_size_text;
	size_t m_Mbitrate;

private:
	wxWindow* m_frame;
	int m_last_frame;
	double m_starting_rot;
    wxTimer m_timer;
	wxString m_filename;
	bool m_running, m_record;
	RecorderDlg * m_advanced_movie;
	wxNotebook * m_notebook;
	int m_current_page;
	QVideoEncoder encoder_;
	wxString filetype_;

private:
	void GetSettings(int view=0);

	//set the renderview and progress bars/text
	void SetRendering(double pcnt);
	void SetProgress(double pcnt);

	//write frames to file
	void WriteFrameToFile();

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
	void OnUpFrame(wxCommandEvent& event);
	void OnDownFrame(wxCommandEvent& event);
	void OnTimeText(wxCommandEvent& event);

	//checkboxes
	void OnSequenceChecked(wxCommandEvent& event);
	void OnRotateChecked(wxCommandEvent& event);

    //timer for playback.
    void OnTimer(wxTimerEvent& event);

	DECLARE_EVENT_TABLE();
};

// this class allows option altering in the dialog.
class MyFileDialog : public wxFileDialog
{
public:
	MyFileDialog(VMovieView * parent, const wxString &title,
		const wxString& defaultdir, const wxString &dftfile,
		const wxString& wildcard, long style) :
	wxFileDialog(parent, title, defaultdir,
		dftfile, wildcard, style),
		m_mv(parent) {}
	wxWindow* CreateExtraCaptureControl(wxWindow* parent);
	void AddOptions();
	
	void OnCh1Check(wxCommandEvent &event);
    void OnMovieQuality(wxCommandEvent &event);
	void OnChEmbedCheck(wxCommandEvent &event);

	enum
	{
		ID_BitrateText,
		ID_EstSizeText,
		ID_CompressChk,
		ID_EmbedChk,
	};

	VMovieView * m_mv;
	wxTextCtrl * m_bitrate_text;
	wxTextCtrl * m_estimated_size_text;
	wxCheckBox * m_ch_embed;
	wxCheckBox * m_ch1;
};

#endif//_VMovieView_H_
