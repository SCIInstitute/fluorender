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
#ifndef _MEASUREDLG_H_
#define _MEASUREDLG_H_

#include <PropPanel.h>
#include <Color.h>
#include <Point.h>
#include <wx/listctrl.h>
#include <wx/clrpicker.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>

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
public:
	RulerListCtrl(
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style=wxLC_REPORT);
	~RulerListCtrl();

	void SelectItemSilently(int i)
	{
		if (i < 0 || i >= GetItemCount())
			return;
		m_silent_select = true;
		SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		m_silent_select = false;
	}

	void Append(bool enable,
		unsigned int id,
		bool time_dep,
		const wxString &unit,
		const wxString& name,
		unsigned int group,
		int count,
		const wxString& intensity,
		const wxString& color,
		int branches,
		double length,
		double angle,
		const wxString &center,
		int time,
		const wxString &points,
		const wxString &voxels);
	void AdjustSize();

	wxString GetText(long item, int col);
	void SetText(long item, int col, const wxString &str);
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

	long m_editing_item = -1;
	wxString m_name;
	fluo::Point m_center;
	fluo::Color m_color;
	bool m_color_set = false;
	bool m_silent_select = false;

private:
	void OnTextFocus(wxMouseEvent& event);
	void OnNameText(wxCommandEvent& event);
	void OnCenterText(wxCommandEvent& event);
	void OnColorChange(wxColourPickerEvent& event);

private:
	fluo::Point GetPointFromString(const wxString& str);
	fluo::Color GetColorFromWxColor(const wxColor& c);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class MeasureDlg : public TabbedPanel
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
		ID_RulerLocator = 0,
		ID_RulerProbe,
		ID_RulerLine,
		ID_RulerAngle,
		ID_RulerEllipse,
		ID_RulerPolyline,
		ID_RulerPencil,
		ID_RulerGrow,
		//toolbar2
		ID_RulerMoveBtn,
		ID_RulerMovePointBtn,
		ID_MagnetBtn,
		ID_RulerMovePencilBtn,
		ID_RulerFlipBtn,
		ID_RulerAvgBtn,
		ID_LockBtn,
		ID_RelaxBtn,
		//toolbar3
		ID_DeleteBtn,
		ID_DeleteAllBtn,
		ID_RulerDelBtn,
		ID_PruneBtn,
		ID_ProfileBtn,
		ID_DistanceBtn,
		ID_ProjectBtn,
		ID_ExportBtn
	};
	enum
	{
		ID_ViewPlaneRd = 0,
		ID_MaxIntensityRd,
		ID_AccIntensityRd
	};
	enum
	{
		ID_AlignX = 0,
		ID_AlignY,
		ID_AlignZ,
		ID_AlignNX,
		ID_AlignNY,
		ID_AlignNZ,
	};
	enum
	{
		//align
		ID_AlignXYZ = 0,
		ID_AlignYXZ,
		ID_AlignZXY,
		ID_AlignXZY,
		ID_AlignYZX,
		ID_AlignZYX,
	};

	MeasureDlg(MainFrame* frame);
	~MeasureDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});
	void UpdateRulerList();
	void UpdateRulerListCur();
	void ToggleDisplay();
	void SetCurrentRuler();
	void UpdateProfile();
	void UpdateGroupSel();

	//toolbar1
	void Locator();
	void Probe();
	void RulerLine();
	void Protractor();
	void Ellipse();
	void RulerPolyline();
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
	void Export();

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
	//display options
	wxCheckBox* m_disp_point_chk;
	wxCheckBox* m_disp_line_chk;
	wxCheckBox* m_disp_name_chk;
	wxCheckBox* m_disp_all_chk;
	//relax
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
	wxWindow* CreateToolPage(wxWindow* parent);
	wxWindow* CreateListPage(wxWindow* parent);
	wxWindow* CreateAlignPage(wxWindow* parent);

private:
	void OnToolbar(wxCommandEvent& event);

	void OnIntensityMethodCheck(wxCommandEvent& event);
	void OnTransientCheck(wxCommandEvent& event);
	void OnUseTransferCheck(wxCommandEvent& event);
	void OnDispPointCheck(wxCommandEvent& event);
	void OnDispLineCheck(wxCommandEvent& event);
	void OnDispNameCheck(wxCommandEvent& event);
	void OnDispAllCheck(wxCommandEvent& event);
	void OnRelaxData(wxCommandEvent& event);
	void OnRelaxValueSpin(wxSpinDoubleEvent& event);
	void OnRelaxValueText(wxCommandEvent& event);

	//ruler list
	void OnGroupText(wxCommandEvent& event);
	void OnNewGroup(wxCommandEvent& event);
	void OnChgGroup(wxCommandEvent& event);
	void OnSelGroup(wxCommandEvent& event);
	void OnDispTglGroup(wxCommandEvent& event);

	//interpolate/key
	void OnInterpCmb(wxCommandEvent& event);
	void OnDeleteKeyBtn(wxCommandEvent& event);
	void OnDeleteAllKeyBtn(wxCommandEvent& event);

	//align
	void OnAlignCenterChk(wxCommandEvent& event);
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
	//edit
	//void OnNameChange(wxCommandEvent& event);
	//void OnCenterChange(wxCommandEvent& event);
	//void OnColorChange(wxColourPickerEvent& event);
};

#endif//_MEASUREDLG_H_
