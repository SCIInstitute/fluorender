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
#ifndef _OCLDLG_H_
#define _OCLDLG_H_

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <wx/listctrl.h>
#include <string>

class MainFrame;
class RenderCanvas;
class wxSingleSlider;
class OclDlg : public wxPanel
{
public:
	enum
	{
		ID_KernelFileTxt = ID_OCL,
		ID_BrowseBtn,
		ID_SaveBtn,
		ID_SaveAsBtn,
		ID_ExecuteBtn,
		ID_ExecuteNBtn,
		ID_IterationsSldr,
		ID_IterationsTxt,
		ID_KernelList,
		ID_KernelEditStc,
		ID_OutputTxt
	};

	OclDlg(MainFrame* frame);
	~OclDlg();

	void GetSettings(RenderCanvas* view);
	RenderCanvas* GetRenderCanvas();

private:
	MainFrame* m_frame;
	RenderCanvas* m_view;

	//ui
	wxTextCtrl* m_kernel_file_txt;
	wxButton* m_browse_btn;
	wxButton* m_save_btn;
	wxButton* m_saveas_btn;
	wxButton* m_execute_btn;
	wxButton* m_execute_n_btn;
	wxSingleSlider* m_iterations_sldr;
	wxTextCtrl* m_iterations_txt;
	wxTextCtrl* m_output_txt;

	//list
	wxListCtrl* m_kernel_list;

	//stc
	wxStyledTextCtrl* m_kernel_edit_stc;

	int m_LineNrID;
	int m_DividerID;
	int m_FoldingID;

private:
	void AddKernelsToList();
	void Execute();

	void copy_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void box_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void gauss_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void median_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void min_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void max_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void sobel_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);
	void morph_filter(void* data, void* result,
		int brick_x, int brick_y, int brick_z);

	void OnBrowseBtn(wxCommandEvent& event);
	void OnSaveBtn(wxCommandEvent& event);
	void OnSaveAsBtn(wxCommandEvent& event);
	void OnExecuteBtn(wxCommandEvent& event);
	void OnExecuteNBtn(wxCommandEvent& event);
	void OnIterationsChange(wxScrollEvent &event);
	void OnIterationsEdit(wxCommandEvent &event);
	void OnKernelListSelected(wxListEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif
