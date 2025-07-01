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
#ifndef _MOVIEPANEL_H_
#define _MOVIEPANEL_H_

#include <PropPanel.h>
#include <wx/listctrl.h>
#include <wx/spinbutt.h>
#include <wx/tglbtn.h>
#include <vector>
#include <string>

#define UITEXT_NBPG0 "Basic"
#define UITEXT_NBPG1 "Keyframes"
#define UITEXT_NBPG2 "Presets"
#define UITEXT_NBPG3 "Crop"
#define UITEXT_NBPG4_0 "Scripts"
#define UITEXT_NBPG4_1 "Scripts (Enabled)"

class RenderView;
class wxUndoableScrollBar;

class KeyListCtrl : public wxListCtrl
{
public:
	KeyListCtrl(wxWindow* parent, MainFrame* frame,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxLC_REPORT | wxLC_SINGLE_SEL);
	~KeyListCtrl();

	void SelectItemSilently(int i)
	{
		//DBGPRINT(L"Select Key List Item: %d\n", i);
		m_silent_select = true;
		Freeze(); // Optional: prevents flicker during updates

		// Clear all selections
		long item = -1;
		while ((item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1) {
			SetItemState(item, 0, wxLIST_STATE_SELECTED);
		}

		SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		EnsureVisible(i); // Optional: scrolls to the item

		Thaw(); // Optional: resumes screen updates
		m_silent_select = false;
	}

	void Append(int id, int time, int duration, int interp, const std::wstring& description);
	void DeleteSel();
	void DeleteAll();
	wxString GetText(long item, int col);
	void SetText(long item, int col, wxString& str);
	void Update();
	void UpdateText();

private:
	MainFrame* m_frame;
	wxImageList* m_images;

	wxTextCtrl* m_frame_text;
	wxTextCtrl* m_duration_text;
	wxComboBox* m_interpolation_cmb;
	wxTextCtrl* m_description_text;

	long m_editing_item;
	long m_dragging_to_item;
	bool m_silent_select = false;

private:
	void EndEdit(bool update = true);

private:
	void OnSelection(wxListEvent& event);
	void OnEndSelection(wxListEvent& event);
	void OnFrameText(wxCommandEvent& event);
	void OnDurationText(wxCommandEvent& event);
	void OnInterpoCmb(wxCommandEvent& event);
	void OnDescritionText(wxCommandEvent& event);
	void OnBeginDrag(wxListEvent& event);
	void OnDragging(wxMouseEvent& event);
	void OnEndDrag(wxMouseEvent& event);

	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnMouseScroll(wxMouseEvent& event);
};

class MoviePanel : public TabbedPanel
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

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//common
	void SetFps(double val);
	void SetMovieLength(double val);
	void SetView(int val);
	void SetSliderStyle(bool val);
	//frames
	void SetFullFrame(int val);
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
	void Save(const std::wstring& filename);

	//keyframe movie
	void SetKeyframeMovie(bool val);

	//crop
	void SetCropEnable(bool val);
	void SetCropValues(int, int, int, int);
	void SetScalebarPos(int pos);
	void SetScalebarValues(int x, int y);

	//script
	size_t GetScriptFiles(std::vector<std::wstring>& list);
	void EnableScript(bool val, const std::wstring& filename = L"");

private:
	bool m_running;
	RenderView* m_view;

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
	wxTextCtrl* m_full_frame_text;

	wxButton *m_rewind_btn;
	wxToggleButton* m_play_inv_btn;
	wxToggleButton *m_play_btn;
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
	//sequence
	wxCheckBox *m_seq_chk;
	wxCheckBox *m_bat_chk;
	wxButton* m_seq_dec_btn;
	wxButton* m_seq_inc_btn;
	wxTextCtrl* m_seq_num_text;
	wxTextCtrl* m_seq_total_text;

	//key frame
	wxCheckBox* m_keyframe_chk;
	//recorder controls
	//list ctrl
	KeyListCtrl* m_keylist;
	//default duration
	wxTextCtrl* m_duration_text;
	//default interpolation
	wxComboBox* m_interpolation_cmb;
	//set key
	wxButton* m_set_key_btn;
	//insert key
	//wxButton *m_insert_key_btn;
	//delete key
	wxButton* m_del_key_btn;
	//delete all keys
	wxButton* m_del_all_btn;
	//lock cam center object
	wxCheckBox* m_cam_lock_chk;
	wxComboBox* m_cam_lock_cmb;
	wxButton* m_cam_lock_btn;

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

	wxTextCtrl *m_crop_x_text;
	wxSpinButton* m_crop_x_spin;
	wxTextCtrl *m_crop_y_text;
	wxSpinButton* m_crop_y_spin;

	wxTextCtrl *m_crop_w_text;
	wxSpinButton* m_crop_w_spin;
	wxTextCtrl *m_crop_h_text;
	wxSpinButton* m_crop_h_spin;

	wxRadioButton* m_sb_tl_rb;
	wxRadioButton* m_sb_tr_rb;
	wxRadioButton* m_sb_bl_rb;
	wxRadioButton* m_sb_br_rb;

	wxTextCtrl* m_sb_dx_text;
	wxSpinButton* m_sb_dx_spin;
	wxTextCtrl* m_sb_dy_text;
	wxSpinButton* m_sb_dy_spin;

private:
	wxWindow* CreateSimplePage(wxWindow *parent);
	wxWindow* CreateKeyframePage(wxWindow *parent);
	wxWindow* CreateTemplatePage(wxWindow *parent);
	wxWindow* CreateCropPage(wxWindow *parent);
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
	void OnFullFrameText(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);

	//basic rotation
	void OnRotateChecked(wxCommandEvent& event);
	void OnRotAxis(wxCommandEvent& event);
	void OnDegreeText(wxCommandEvent& event);
	//rotation interpolation
	void OnRotIntCmb(wxCommandEvent& event);
	//sequence
	void OnSequenceChecked(wxCommandEvent& event);
	void OnBatchChecked(wxCommandEvent& event);
	void OnSeqDecBtn(wxCommandEvent& event);
	void OnSeqNumText(wxCommandEvent& event);
	void OnSeqIncBtn(wxCommandEvent& event);

	//keyframe movie
	void OnAct(wxListEvent& event);
	void OnKeyframeChk(wxCommandEvent& event);
	void OnDurationText(wxCommandEvent& event);
	void OnInterpolation(wxCommandEvent& event);
	void OnInsKey(wxCommandEvent& event);
	void OnDelKey(wxCommandEvent& event);
	void OnDelAll(wxCommandEvent& event);
	void OnCamLockChk(wxCommandEvent& event);
	void OnCamLockCmb(wxCommandEvent& event);
	void OnCamLockBtn(wxCommandEvent& event);

	//auto key
	void OnGenKey(wxCommandEvent& event);

	//crop
	void OnCropCheck(wxCommandEvent& event);
	void OnResetCrop(wxCommandEvent& event);
	void OnEditCrop(wxCommandEvent& event);

	void OnCropSpinUp(wxSpinEvent& event);
	void OnCropSpinDown(wxSpinEvent& event);

	void OnSbRadio(wxCommandEvent& event);
	void OnSbEdit(wxCommandEvent& event);
	void OnSbSpinUp(wxSpinEvent& event);
	void OnSbSpinDown(wxSpinEvent& event);

	//script
	void OnRunScriptChk(wxCommandEvent& event);
	void OnScriptFileEdit(wxCommandEvent& event);
	void OnScriptClearBtn(wxCommandEvent& event);
	void OnScriptFileBtn(wxCommandEvent& event);
	void OnScriptListSelected(wxListEvent &event);

	//help
	void OnChEnlargeCheck(wxCommandEvent& event);
	void OnSlEnlargeScroll(wxScrollEvent& event);
	void OnTxEnlargeText(wxCommandEvent& event);
	void OnMovieQuality(wxCommandEvent& event);
	void OnCh1Check(wxCommandEvent& event);
	void OnCh2Check(wxCommandEvent& event);
	void OnCh3Check(wxCommandEvent& event);
	void OnDpiText(wxCommandEvent& event);
	void OnChEmbedCheck(wxCommandEvent& event);
	static wxWindow* CreateExtraCaptureControl(wxWindow* parent);
};

#endif//_MOVIEPANEL_H_
