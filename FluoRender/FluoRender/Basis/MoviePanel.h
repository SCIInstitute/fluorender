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
#ifndef _MOVIEPANEL_H_
#define _MOVIEPANEL_H_

#include <PropPanel.h>
#include <wx/aui/auibook.h>
#include <wx/listctrl.h>
#include <wx/spinbutt.h>
#include <wx/tglbtn.h>

class MainFrame;
class RenderCanvas;
class RecorderDlg;
class wxUndoableScrollBar;
class MoviePanel : public PropPanel
{
	enum
	{
		ID_ENLARGE_CHK = 0,
		ID_ENLARGE_SLDR,
		ID_ENLARGE_TEXT,
		ID_MOV_ESTIMATE_TEXT
	};

public:
	MoviePanel(MainFrame* frame,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "MoviePanel");
	~MoviePanel();

	virtual void LoadPerspective();
	virtual void SavePerspective();
	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//common
	void SetFps(double val);
	void SetMovieLength(double val);
	void SetView(int val);
	void SetSliderStyle(bool val);
	//frames
	void SetStartFrame(int val);
	void SetEndFrame(int val);
	void SetScrollFrame(int val, bool notify);
	void SetCurrentFrame(int val, bool notify);
	void SetCurrentTime(double val, bool notify);
	void Play();
	void PlayInv();
	void Rewind();
	void Forward();
	void Loop(bool val);
	void IncFrame();
	void DecFrame();
	void Save(const wxString& filename);

	//keyframe movie
	void SetKeyframeMovie(bool val);
	void GenKey();

	//crop
	void SetCropEnable(bool val);
	void SetCropValues(int, int, int, int);

	//script
	int GetScriptFiles(wxArrayString& list);

private:
	bool m_running;
	RenderCanvas* m_view;
	wxAuiNotebook* m_notebook;
	RecorderDlg* m_advanced_movie;

	//common controls
	wxTextCtrl *m_fps_text;
	wxTextCtrl *m_movie_len_text;
	wxComboBox *m_views_cmb;

	wxToolBar* m_slider_btn;
	wxUndoableScrollBar* m_progress_sldr;

	wxButton* m_start_btn;
	wxTextCtrl *m_start_frame_text;
	wxTextCtrl *m_end_frame_text;
	wxButton* m_end_btn;

	wxButton *m_dec_time_btn;
	wxTextCtrl *m_cur_frame_text;
	wxButton *m_inc_time_btn;

	wxButton *m_rewind_btn;
	wxButton* m_play_inv_btn;
	wxButton *m_play_btn;
	wxButton* m_forward_btn;
	wxToggleButton* m_loop_btn;
	wxTextCtrl *m_progress_text;
	wxButton *m_save_btn;

	//basic movie controls
	wxCheckBox *m_rot_chk;
	wxRadioButton *m_x_rd;
	wxRadioButton *m_y_rd;
	wxRadioButton *m_z_rd;
	wxTextCtrl *m_degree_text;
	wxComboBox *m_rot_int_cmb;

	wxCheckBox *m_seq_chk;
	wxCheckBox *m_bat_chk;

	//key frame
	wxCheckBox* m_keyframe_chk;

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

private:
	wxWindow* CreateSimplePage(wxWindow *parent);
	wxWindow* CreateAdvancedPage(wxWindow *parent);
	wxWindow* CreateAutoKeyPage(wxWindow *parent);
	wxWindow* CreateCroppingPage(wxWindow *parent);
	wxWindow* CreateScriptPage(wxWindow *parent);

	//notebook page change
	void OnNotebookPage(wxAuiNotebookEvent& event);

	//common controls
	void OnFpsEdit(wxCommandEvent& event);
	void OnMovieLenText(wxCommandEvent& event);
	void OnViewSelected(wxCommandEvent& event);
	void OnSliderStyle(wxCommandEvent& event);
	void OnProgressScroll(wxScrollEvent& event);
	void OnStartFrameText(wxCommandEvent& event);
	void OnEndFrameText(wxCommandEvent& event);
	void OnCurFrameText(wxCommandEvent& event);
	void OnCurTimeText(wxCommandEvent& event);
	void OnPlay(wxCommandEvent& event);
	void OnPlayInv(wxCommandEvent& event);
	void OnRewind(wxCommandEvent& event);
	void OnForward(wxCommandEvent& event);
	void OnLoop(wxCommandEvent& event);
	void OnStartFrameBtn(wxCommandEvent& event);
	void OnEndFrameBtn(wxCommandEvent& event);
	void OnIncFrame(wxCommandEvent& event);
	void OnDecFrame(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);

	//basic rotation
	void OnRotateChecked(wxCommandEvent& event);
	void OnRotAxis(wxCommandEvent& event);
	void OnDegreeText(wxCommandEvent &event);
	//rotation interpolation
	void OnRotIntCmb(wxCommandEvent& event);
	//sequence
	void OnSequenceChecked(wxCommandEvent& event);
	void OnBatchChecked(wxCommandEvent& event);

	//keyframe movie
	void OnKeyframeChk(wxCommandEvent& event);

	//auto key
	void OnGenKey(wxCommandEvent& event);

	//crop
	void OnCropCheck(wxCommandEvent& event);
	void OnResetCrop(wxCommandEvent& event);
	void OnEditCrop(wxCommandEvent& event);

	void OnCropSpinUp(wxSpinEvent& event);
	void OnCropSpinDown(wxSpinEvent& event);

	//script
	void OnRunScriptChk(wxCommandEvent &event);
	void OnScriptFileEdit(wxCommandEvent &event);
	void OnScriptClearBtn(wxCommandEvent &event);
	void OnScriptFileBtn(wxCommandEvent &event);
	void OnScriptListSelected(wxListEvent &event);

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
};

#endif//_MOVIEPANEL_H_
