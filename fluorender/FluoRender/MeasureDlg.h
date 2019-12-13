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
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <Distance/RulerHandler.h>
#include "FLIVR/Color.h"
#include <Distance/DistCalculator.h>
#include <Distance/RulerAlign.h>

#ifndef _MEASUREDLG_H_
#define _MEASUREDLG_H_

using namespace std;
using namespace FLIVR;

class VRenderView;

class RulerListCtrl : public wxListCtrl
{
	enum
	{
		ID_NameText = ID_MEASURE1,
		ID_CenterText,
		ID_ColorPicker,
	};

public:
	RulerListCtrl(wxWindow *frame,
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style=wxLC_REPORT|wxLC_SINGLE_SEL);
	~RulerListCtrl();

	void Append(bool enable, unsigned int id, wxString name,
		wxString &color, int branches, double length, wxString &unit,
		double angle, wxString &center, bool time_dep,
		int time, wxString &extra, wxString &points);
	void UpdateRulers(VRenderView* vrv=0);

	bool GetCurrSelection(std::vector<int> &sel);
	void DeleteSelection();
	void DeleteAll(bool cur_time=false);

	void Export(wxString filename);

	wxString GetText(long item, int col);
	void SetText(long item, int col, wxString &str);

	friend class MeasureDlg;

private:
	//wxWindow* m_frame;
	VRenderView *m_view;
	wxImageList *m_images;
	wxTextCtrl *m_name_text;
	wxTextCtrl *m_center_text;
	wxColourPickerCtrl *m_color_picker;
	long m_editing_item;
	bool m_ruler_df_f;

private:
	void EndEdit(bool update=true);
	void OnKeyDown(wxKeyEvent& event);
	void OnSelection(wxListEvent &event);
	void OnEndSelection(wxListEvent &event);
	void OnNameText(wxCommandEvent& event);
	void OnCenterText(wxCommandEvent& event);
	void OnColorChange(wxColourPickerEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnScroll(wxMouseEvent& event);
	void OnTextFocus(wxCommandEvent& event);
	void OnAct(wxListEvent &event);
	void OnContextMenu(wxContextMenuEvent &event);

	DECLARE_EVENT_TABLE()
protected: //Possible TODO
	wxSize GetSizeAvailableForScrollTarget(const wxSize& size) {
		return size - GetEffectiveMinSize();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MeasureDlg : public wxPanel
{
public:
	enum
	{
		ID_LocatorBtn = ID_MEASURE2,
		ID_ProbeBtn,
		ID_ProtractorBtn,
		ID_RulerBtn,
		ID_RulerMPBtn,
		ID_EllipseBtn,
		ID_RulerMoveBtn,
		ID_RulerEditBtn,
		ID_RulerFlipBtn,
		ID_RulerAvgBtn,
		ID_LockBtn,
		ID_RelaxBtn,
		ID_RelaxValueSpin,
		ID_DeleteBtn,
		ID_DeleteAllBtn,
		ID_ProfileBtn,
		ID_DistanceBtn,
		ID_ProjectBtn,
		ID_ExportBtn,
		ID_ViewPlaneRd,
		ID_MaxIntensityRd,
		ID_AccIntensityRd,
		ID_UseTransferChk,
		ID_TransientChk,
		ID_DF_FChk
	};

	MeasureDlg(wxWindow* frame,
		wxWindow* parent);
	~MeasureDlg();

	void GetSettings(VRenderView* vrv);
	VRenderView* GetView();
	void UpdateList();

	//processing
	void Relax();
	void Relax(int idx);
	void Project(int idx);
	void SetEdit() { m_edited = true; }

private:
	wxWindow* m_frame;
	//current view
	VRenderView* m_view;
	FL::RulerHandler *m_rhdl;

	//list ctrl
	RulerListCtrl *m_rulerlist;
	//tool bar
	wxToolBar *m_toolbar1;
	wxToolBar *m_toolbar2;
	wxToolBar *m_toolbar3;
	wxSpinCtrlDouble* m_relax_value_spin;
	//options
	wxRadioButton *m_view_plane_rd;
	wxRadioButton *m_max_intensity_rd;
	wxRadioButton *m_acc_intensity_rd;
	wxCheckBox *m_use_transfer_chk;
	wxCheckBox *m_transient_chk;
	wxCheckBox *m_df_f_chk;

	FL::DistCalculator m_calculator;
	FL::RulerAlign m_aligner;
	bool m_edited;

private:
	void OnNewLocator(wxCommandEvent& event);
	void OnNewProbe(wxCommandEvent& event);
	void OnNewProtractor(wxCommandEvent& event);
	void OnNewRuler(wxCommandEvent& event);
	void OnNewRulerMP(wxCommandEvent& event);
	void OnEllipse(wxCommandEvent& event);
	void OnRulerMove(wxCommandEvent& event);
	void OnRulerEdit(wxCommandEvent& event);
	void OnRulerFlip(wxCommandEvent& event);
	void OnRulerAvg(wxCommandEvent& event);
	void OnLock(wxCommandEvent& event);
	void OnRelax(wxCommandEvent& event);
	void OnRelaxValueSpin(wxSpinDoubleEvent& event);
	void OnRelaxValueText(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	void OnDeleteAll(wxCommandEvent& event);
	void OnProfile(wxCommandEvent& event);
	void OnDistance(wxCommandEvent& event);
	void OnProject(wxCommandEvent& event);
	void OnExport(wxCommandEvent& event);
	void OnIntensityMethodCheck(wxCommandEvent& event);
	void OnUseTransferCheck(wxCommandEvent& event);
	void OnTransientCheck(wxCommandEvent& event);
	void OnDF_FCheck(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_MEASUREDLG_H_
