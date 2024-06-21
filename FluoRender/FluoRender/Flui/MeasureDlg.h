/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <PropPanel.h>
#include <wx/listctrl.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <Color.h>

#define IntCol 3
#define ColorCol 4
#define CenterCol 8
#define PointCol 10

class MeasureDlg;
namespace flrd
{
	class Ruler;
	class RulerList;
}
class RulerListCtrl : public wxListCtrl
{
	//enum
	//{
	//	ID_NameText = ID_MEASURE1,
	//	ID_CenterText,
	//	ID_ColorPicker,
	//	ID_ToggleDisp
	//};

public:
	RulerListCtrl(
		MeasureDlg* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style=wxLC_REPORT);
	~RulerListCtrl();

	void Append(bool enable, unsigned int id,
		wxString name, unsigned int group, int count,
		wxString intensity, wxString &color,
		int branches, double length, wxString &unit,
		double angle, wxString &center, bool time_dep,
		int time, wxString &extra, wxString &points);
	void AdjustSize();

	wxString GetText(long item, int col);
	void SetText(long item, int col, wxString &str);
	bool GetCurrSelection(std::set<int> &sel);
	void ClearSelection();
	void StartEdit(int type, bool use_color, const fluo::Color& color);
	void EndEdit();

	friend class MeasureDlg;

private:
	wxImageList *m_images;
	wxTextCtrl *m_name_text;
	wxTextCtrl *m_center_text;
	wxColourPickerCtrl *m_color_picker;

	long m_editing_item;
	wxString m_name;
	fluo::Point m_center;
	fluo::Color m_color;

private:
	void OnTextFocus(wxCommandEvent& event);
	void OnNameText(wxCommandEvent& event);
	void OnCenterText(wxCommandEvent& event);
	void OnColorChange(wxColourPickerEvent& event);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MeasureDlg : public PropPanel
{
public:
	enum
	{
		//context menu
		ID_ToggleDisp = 0
	};
	enum
	{
		//toolbar1
		ID_LocatorBtn = 0,
		ID_ProbeBtn,
		ID_RulerBtn,
		ID_ProtractorBtn,
		ID_EllipseBtn,
		ID_RulerMPBtn,
		ID_PencilBtn,
		ID_GrowBtn
	};
	enum
	{
		//toolbar2
		ID_RulerMoveBtn,
		ID_RulerMovePointBtn,
		ID_MagnetBtn,
		ID_RulerMovePencilBtn,
		ID_RulerFlipBtn,
		ID_RulerAvgBtn,
		ID_LockBtn,
		ID_RelaxBtn,
	};
	enum
	{
		//toolbar3
		ID_DeleteBtn,
		ID_DeleteAllBtn,
		ID_RulerDelBtn,
		ID_PruneBtn,
		ID_ProfileBtn,
		ID_DistanceBtn,
		ID_ProjectBtn,
		ID_ExportBtn,
	};
	//enum
	//{
	//	ID_ViewPlaneRd,
	//	ID_MaxIntensityRd,
	//	ID_AccIntensityRd,
	//	ID_UseTransferChk,
	//	ID_TransientChk,
	//	ID_DF_FChk,
	//	ID_AutoRelaxBtn,
	//	ID_RelaxValueSpin,
	//	ID_RelaxDataCmb,
	//	//ruler list
	//	ID_NewGroup,
	//	ID_GroupText,
	//	ID_ChgGroup,
	//	ID_SelGroup,
	//	ID_DispTglGroup,
	//	//interpolation/key
	//	ID_InterpCmb,
	//	ID_DeleteKeyBtn,
	//	ID_DeleteAllKeyBtn,
	//	//align
	//	ID_AlignCenter,
	//	ID_AlignX,
	//	ID_AlignY,
	//	ID_AlignZ,
	//	ID_AlignNX,
	//	ID_AlignNY,
	//	ID_AlignNZ,
	//	ID_AlignXYZ,
	//	ID_AlignYXZ,
	//	ID_AlignZXY,
	//	ID_AlignXZY,
	//	ID_AlignYZX,
	//	ID_AlignZYX,
	//};

	MeasureDlg(MainFrame* frame);
	~MeasureDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	void UpdateRulerList();
	void UpdateRulerListCur();
	void ToggleDisplay();
	void SetCurrentRuler();

	//toolbar1
	void Locator();
	void Probe();
	void Ruler();
	void Protractor();
	void Ellipse();
	void RulerMP();
	void Pencil();
	void Grow();

	//toolbar2
	void RulerMove();
	void RulerMovePoint();
	void Magnet();
	void RulerMovePencil();
	void RulerFlip();
	void RulerAvg();
	void Lock();
	void Relax();

	//toolbar3
	void DeleteSelection();
	void DeleteAll();
	void DeletePoint();
	void Prune();//remove branches with length equal to or smaller than len
	void Profile();
	void Distance();
	void Project();
	void Project(int idx);
	void Export();

	void AlignCenter(flrd::Ruler* ruler, flrd::RulerList* ruler_list);
	void SetProfile(int i);

	void SelectGroup(unsigned int group);
	void SetEdit() { m_edited = true; }

private:
	//list ctrl
	wxButton* m_new_group;
	wxTextCtrl* m_group_text;
	wxButton* m_chg_group;
	wxButton* m_sel_group;
	wxButton* m_disptgl_group;
	//interpolate/key
	wxComboBox* m_interp_cmb;
	wxButton* m_delete_key_btn;
	wxButton* m_delete_all_key_btn;
	//list
	RulerListCtrl *m_ruler_list;
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

	bool m_edited;

private:
	void OnToolbar1(wxCommandEvent& event);
	void OnToolbar2(wxCommandEvent& event);
	void OnToolbar3(wxCommandEvent& event);


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
	//interpolate/key
	void OnInterpCmb(wxCommandEvent& event);
	void OnDeleteKeyBtn(wxCommandEvent& event);
	void OnDeleteAllKeyBtn(wxCommandEvent& event);
	//align
	void OnAlignRuler(wxCommandEvent& event);
	void OnAlignPca(wxCommandEvent& event);

	//list events
	void OnKeyDown(wxKeyEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnMenuItem(wxCommandEvent& event);
	void OnSelection(wxListEvent& event);
	void OnEndSelection(wxListEvent& event);
	void OnScrollWin(wxScrollWinEvent& event);
	void OnScrollMouse(wxMouseEvent& event);
	void OnAct(wxListEvent& event);
};

#endif//_MEASUREDLG_H_
