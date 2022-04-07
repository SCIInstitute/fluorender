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
#ifndef _MOVIEPANEL_H_
#define _MOVIEPANEL_H_

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include <wx/clrpicker.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <MovieAgent.hpp>

class RenderFrame;
class RecorderDlg;
namespace fluo
{
	class Renderview;
}
class MoviePanel : public wxPanel
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
	MoviePanel(RenderFrame* frame,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "MoviePanel");
	~MoviePanel();

	void AssociateRenderview(fluo::Renderview* view)
	{
		m_agent->setObject(view);
	}

	friend class fluo::MovieAgent;

private:
	fluo::MovieAgent* m_agent;

	//controls
	wxTextCtrl *m_fps_text;
	wxComboBox *m_views_cmb;

	wxButton *m_play_btn;
	wxButton *m_rewind_btn;
	wxSlider *m_progress_sldr;
	wxTextCtrl *m_progress_text;
	wxButton *m_save_btn;

	wxNotebook* m_notebook;

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

private:
	RenderFrame* m_frame;
	RecorderDlg* m_advanced_movie;

private:
	wxWindow* CreateSimplePage(wxWindow *parent);
	wxWindow* CreateAdvancedPage(wxWindow *parent);
	wxWindow* CreateAutoKeyPage(wxWindow *parent);
	wxWindow* CreateCroppingPage(wxWindow *parent);
	wxWindow* CreateScriptPage(wxWindow *parent);

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
	void Rewind();
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

	//notebook page change
	void OnNbPageChange(wxBookCtrlEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_MOVIEPANEL_H_
