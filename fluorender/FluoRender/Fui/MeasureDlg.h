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
#ifndef _MEASUREDLG_H_
#define _MEASUREDLG_H_

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <MeasureAgent.hpp>

class RenderFrame;
namespace fluo
{
	class Renderview;
}
class RulerListCtrl : public wxListCtrl
{
	enum
	{
		ID_NameText = ID_MEASURE1,
		ID_CenterText,
		ID_ColorPicker,
		ID_ToggleDisp
	};

public:
	RulerListCtrl(
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style=wxLC_REPORT|wxLC_SINGLE_SEL);
	~RulerListCtrl();

	void Append(bool enable, unsigned int id,
		wxString name, unsigned int group, int count,
		wxString &color, int branches, double length, wxString &unit,
		double angle, wxString &center, bool time_dep,
		int time, wxString &extra, wxString &points);
	void AdjustSize();

	bool GetCurrSelection(std::vector<int> &sel);
	void ClearSelection();
	wxString GetText(long item, int col);
	void SetText(long item, int col, wxString &str);

	friend class MeasureDlg;
	friend class fluo::MeasureAgent;

private:
	fluo::MeasureAgent* m_agent;

	wxImageList *m_images;
	wxTextCtrl *m_name_text;
	wxTextCtrl *m_center_text;
	wxColourPickerCtrl *m_color_picker;
	long m_editing_item;

private:
	void OnContextMenu(wxContextMenuEvent &event);
	void OnToggleDisp(wxCommandEvent& event);
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
		ID_GrowBtn,
		ID_PencilBtn,
		ID_RulerMoveBtn,
		ID_RulerEditBtn,
		ID_RulerDelBtn,
		ID_RulerFlipBtn,
		ID_RulerAvgBtn,
		ID_LockBtn,
		ID_PruneBtn,
		ID_RelaxBtn,
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
		ID_DF_FChk,
		ID_AutoRelaxBtn,
		ID_RelaxValueSpin,
		ID_RelaxDataCmb,
		//ruler list
		ID_NewGroup,
		ID_GroupText,
		ID_ChgGroup,
		ID_SelGroup,
		ID_DispTglGroup,
		//align
		ID_AlignCenter,
		ID_AlignX,
		ID_AlignY,
		ID_AlignZ,
		ID_AlignNX,
		ID_AlignNY,
		ID_AlignNZ,
		ID_AlignXYZ,
		ID_AlignYXZ,
		ID_AlignZXY,
		ID_AlignXZY,
		ID_AlignYZX,
		ID_AlignZYX,
	};

	MeasureDlg(RenderFrame* frame);
	~MeasureDlg();

	void AssociateRenderview(fluo::Renderview* view)
	{
		m_agent->setObject(view);
	}

	friend class fluo::MeasureAgent;

private:
	fluo::MeasureAgent* m_agent;
	RenderFrame* m_frame;

	//list ctrl
	wxButton* m_new_group;
	wxTextCtrl* m_group_text;
	wxButton* m_chg_group;
	wxButton* m_sel_group;
	wxButton* m_disptgl_group;
	RulerListCtrl *m_rulerlist;
	//tool bar
	wxToolBar *m_toolbar1;
	wxToolBar *m_toolbar2;
	wxToolBar *m_toolbar3;
	//options
	wxRadioButton *m_view_plane_rd;
	wxRadioButton *m_max_intensity_rd;
	wxRadioButton *m_acc_intensity_rd;
	wxCheckBox *m_use_transfer_chk;
	wxCheckBox *m_transient_chk;
	wxCheckBox *m_df_f_chk;
	wxToggleButton *m_auto_relax_btn;
	wxSpinCtrlDouble* m_relax_value_spin;
	wxComboBox *m_relax_data_cmb;
	//align
	wxCheckBox* m_align_center;
	wxButton* m_align_x;
	wxButton* m_align_y;
	wxButton* m_align_z;
	wxButton* m_align_nx;
	wxButton* m_align_ny;
	wxButton* m_align_nz;
	wxButton* m_align_xyz;
	wxButton* m_align_yxz;
	wxButton* m_align_zxy;
	wxButton* m_align_xzy;
	wxButton* m_align_yzx;
	wxButton* m_align_zyx;

private:
	void OnNewLocator(wxCommandEvent& event);
	void OnNewProbe(wxCommandEvent& event);
	void OnNewProtractor(wxCommandEvent& event);
	void OnNewRuler(wxCommandEvent& event);
	void OnNewRulerMP(wxCommandEvent& event);
	void OnEllipse(wxCommandEvent& event);
	void OnGrow(wxCommandEvent& event);
	void OnPencil(wxCommandEvent& event);
	void OnRulerMove(wxCommandEvent& event);
	void OnRulerEdit(wxCommandEvent& event);
	void OnRulerDel(wxCommandEvent& event);
	void OnRulerFlip(wxCommandEvent& event);
	void OnRulerAvg(wxCommandEvent& event);
	void OnLock(wxCommandEvent& event);
	void OnPrune(wxCommandEvent& event);
	void OnRelax(wxCommandEvent& event);
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
	void OnAutoRelax(wxCommandEvent& event);
	void OnRelaxValueSpin(wxSpinDoubleEvent& event);
	void OnRelaxValueText(wxCommandEvent& event);
	void OnRelaxData(wxCommandEvent& event);
	//ruler list
	void OnNewGroup(wxCommandEvent& event);
	void OnChgGroup(wxCommandEvent& event);
	void OnSelGroup(wxCommandEvent& event);
	void OnDispTglGroup(wxCommandEvent& event);
	//align
	void OnAlignCenter(wxCommandEvent& event);
	void OnAlignAxis(wxCommandEvent& event);
	void OnAlignRuler(wxCommandEvent& event);
	void OnAlignPca(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_MEASUREDLG_H_
