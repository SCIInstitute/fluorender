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
#ifndef _BRUSHTOOLDLG_H_
#define _BRUSHTOOLDLG_H_

#include <PropPanel.h>
#include <wx/grid.h>
#include <wx/tglbtn.h>
#include <wx/clipbrd.h>

class wxSingleSlider;
struct GridData
{
	int voxel_sum = 0;
	double voxel_wsum = 0;
	double avg_int = 0;
	double size = 0;
	double wsize = 0;
};

class BrushToolDlg : public TabbedPanel
{
public:
	enum
	{
		//brushes
		ID_BrushUndo = 0,
		ID_BrushRedo,
		ID_BrushGrow,
		ID_BrushAppend,
		ID_BrushComp,
		ID_BrushMesh,
		ID_BrushSingle,
		ID_BrushDiffuse,
		ID_BrushSolid,
		ID_BrushUnsel,
		//second row
		ID_BrushClear,
		ID_BrushExtract,
		ID_BrushDelete,
		//mask
		ID_MaskCopy,
		ID_MaskCopyData,
		ID_MaskPaste,
		ID_MaskMerge,
		ID_MaskExclude,
		ID_MaskIntersect
	};
	enum
	{
		//axis align
		ID_AlignXYZ = 0,
		ID_AlignYXZ,
		ID_AlignZXY,
		ID_AlignXZY,
		ID_AlignYZX,
		ID_AlignZYX
	};

	BrushToolDlg(MainFrame* frame);
	~BrushToolDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//toolbar1
	void BrushUndo();
	void BrushRedo();
	void BrushGrow();
	void BrushAppend();
	void BrushComp();
	void BrushMesh();
	void BrushSingle();
	void BrushDiffuse();
	void BrushSolid();
	void BrushUnsel();
	//toolbar2
	void BrushClear();
	void BrushExtract();
	void BrushDelete();
	void MaskCopy();
	void MaskCopyData();
	void MaskPaste();
	void MaskMerge();
	void MaskExclude();
	void MaskIntersect();

private:
	//max volume value
	double m_max_value;
	//output
	bool m_hold_history;

	//paint tools
	//toolbar
	wxToolBar *m_toolbar;
	wxToolBar *m_toolbar2;

	//stop at boundary
	wxCheckBox* m_edge_detect_chk;
	//hidden removal
	wxCheckBox* m_hidden_removal_chk;
	//group selection
	wxCheckBox* m_select_group_chk;
	//brick acuracy
	wxCheckBox* m_accurate_bricks_chk;
	//selection strength
	//translate
	wxSingleSlider* m_brush_scl_translate_sldr;
	wxTextCtrl* m_brush_scl_translate_text;
	//gm falloff
	wxSingleSlider* m_brush_gm_falloff_sldr;
	wxTextCtrl* m_brush_gm_falloff_text;
	//2d influence
	wxSingleSlider* m_brush_2dinfl_sldr;
	wxTextCtrl* m_brush_2dinfl_text;
	//brush properties
	//size 1
	wxSingleSlider* m_brush_size1_sldr;
	wxTextCtrl *m_brush_size1_text;
	//size 2
	wxCheckBox* m_brush_size2_chk;
	wxSingleSlider* m_brush_size2_sldr;
	wxTextCtrl* m_brush_size2_text;
	//size relation
	wxRadioButton* m_brush_size_data_rb;
	wxRadioButton* m_brush_size_screen_rb;
	//growth
	wxSingleSlider* m_brush_iter_sldr;
	wxTextCtrl* m_brush_iter_text;
	//align
	wxCheckBox* m_align_center_chk;
	wxButton* m_align_xyz;
	wxButton* m_align_yxz;
	wxButton* m_align_zxy;
	wxButton* m_align_xzy;
	wxButton* m_align_yzx;
	wxButton* m_align_zyx;

	//output
	wxButton* m_update_btn;
	wxCheckBox* m_history_chk;
	wxButton* m_clear_hist_btn;
	wxGrid *m_output_grid;

private:
	//output
	void SetOutput(const GridData& data, const wxString& unit);
	void CopyData();
	void PasteData();

	wxWindow* CreateToolPage(wxWindow* parent);
	wxWindow* CreateListPage(wxWindow* parent);
	wxWindow* CreateAlignPage(wxWindow* parent);

private:
	//event handling
	//paint tools
	//brush commands
	void OnToolBar(wxCommandEvent& event);
	//selection adjustment
	//2d influence
	void OnBrush2dinflChange(wxScrollEvent& event);
	void OnBrush2dinflText(wxCommandEvent& event);
	//edge detect
	void OnBrushEdgeDetectChk(wxCommandEvent& event);
	//hidden removal
	void OnBrushHiddenRemovalChk(wxCommandEvent& event);
	//select group
	void OnBrushSelectGroupChk(wxCommandEvent& event);
	//brick accuracy
	void OnAccurateBricksCheck(wxCommandEvent& event);
	//translate
	void OnBrushSclTranslateChange(wxScrollEvent& event);
	void OnBrushSclTranslateText(wxCommandEvent& event);
	//gm falloff
	void OnBrushGmFalloffChange(wxScrollEvent& event);
	void OnBrushGmFalloffText(wxCommandEvent& event);
	//brush properties
	//brush size 1
	void OnBrushSize1Change(wxScrollEvent& event);
	void OnBrushSize1Text(wxCommandEvent& event);
	//brush size 2
	void OnBrushSize2Chk(wxCommandEvent& event);
	void OnBrushSize2Change(wxScrollEvent& event);
	void OnBrushSize2Text(wxCommandEvent& event);
	//brush iterations
	void OnBrushIterChange(wxScrollEvent& event);
	void OnBrushIterText(wxCommandEvent& event);
	//brush size relation
	void OnBrushSizeRelationCheck(wxCommandEvent& event);
	//align
	void OnAlignCenterCheck(wxCommandEvent& event);
	void OnAlignPca(wxCommandEvent& event);
	//output
	void OnUpdateBtn(wxCommandEvent& event);
	void OnHistoryChk(wxCommandEvent& event);
	void OnClearHistBtn(wxCommandEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnSelectCell(wxGridEvent& event);
	//resize
	void OnSize(wxSizeEvent& event);
};

#endif//_BRUSHTOOLDLG_H_
