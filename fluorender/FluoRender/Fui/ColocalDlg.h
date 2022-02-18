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
#ifndef _COLOCALIZATIONDLG_H_
#define _COLOCALIZATIONDLG_H_

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>
#include <ColocalAgent.hpp>
#include <limits>
#include <chrono>

class VRenderFrame;
namespace fluo
{
	class Renderview;
	class VolumeData;
	class VolumeGroup;
}
class ColocalDlg : public wxPanel
{
public:
	enum
	{
		ID_ColocalizeBtn = ID_COLOCALIZE,
		ID_UseSelChk,
		ID_AutoUpdateBtn,
		//settings
		ID_ProductRdb,
		ID_MinValueRdb,
		ID_LogicalAndRdb,
		//formats
		ID_IntWeightBtn,
		ID_RatioBtn,
		ID_PhysicalBtn,
		ID_ColorMapBtn,

		//output
		ID_HistoryChk,
		ID_ClearHistBtn,
		ID_OutputGrid
	};

	ColocalDlg(VRenderFrame* frame);
	~ColocalDlg();

	void SetGroup(fluo::VolumeGroup* group)
	{
		m_agent->setObject(group);
	}

	//settings
	void GetSettings();

	//grid data ops
	void CopyData();
	void PasteData();

	friend class fluo::ColocalAgent;

private:
	fluo::ColocalAgent* m_agent;

	//speed test
	bool m_test_speed;
	std::vector<std::chrono::high_resolution_clock::time_point> m_tps;
	wxString m_titles, m_values;

	//interface
	//colocalization
	wxButton *m_colocalize_btn;
	wxCheckBox* m_use_sel_chk;
	wxToggleButton* m_auto_update_btn;
	//settings
	wxRadioButton* m_product_rdb;
	wxRadioButton* m_min_value_rdb;
	wxRadioButton* m_logical_and_rdb;
	//format
	wxToggleButton* m_int_weight_btn;
	wxToggleButton* m_ratio_btn;
	wxToggleButton* m_physical_btn;
	wxToggleButton* m_colormap_btn;

	//output
	wxCheckBox* m_history_chk;
	wxButton* m_clear_hist_btn;
	wxGrid *m_output_grid;

private:
	//speed test
	void StartTimer(std::string str);
	void StopTimer(std::string str);

	//calculate
	void OnColocalizenBtn(wxCommandEvent &event);
	void OnUseSelChk(wxCommandEvent &event);
	void OnAutoUpdate(wxCommandEvent &event);
	//settings
	void OnMethodRdb(wxCommandEvent &event);
	//format
	void OnIntWeightBtn(wxCommandEvent &event);
	void OnRatioBtn(wxCommandEvent &event);
	void OnPhysicalBtn(wxCommandEvent &event);
	void OnColorMapBtn(wxCommandEvent &event);

	//output
	void OnHistoryChk(wxCommandEvent& event);
	void OnClearHistBtn(wxCommandEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnSelectCell(wxGridEvent& event);
	void OnGridLabelClick(wxGridEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_COLOCALIZATIONDLG_H_
