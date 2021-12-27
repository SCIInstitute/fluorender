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
#include "OclDlg.h"
#include <VolumeData.hpp>
#include "VRenderFrame.h"
#include <compatibility.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/stdpaths.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(OclDlg, wxPanel)
	EVT_BUTTON(ID_BrowseBtn, OclDlg::OnBrowseBtn)
	EVT_BUTTON(ID_SaveBtn, OclDlg::OnSaveBtn)
	EVT_BUTTON(ID_SaveAsBtn, OclDlg::OnSaveAsBtn)
	EVT_BUTTON(ID_ExecuteBtn, OclDlg::OnExecuteBtn)
	EVT_BUTTON(ID_ExecuteNBtn, OclDlg::OnExecuteNBtn)
	EVT_COMMAND_SCROLL(ID_IterationsSldr, OclDlg::OnIterationsChange)
	EVT_TEXT(ID_IterationsTxt, OclDlg::OnIterationsEdit)
	EVT_LIST_ITEM_SELECTED(ID_KernelList, OclDlg::OnKernelListSelected)
END_EVENT_TABLE()

OclDlg::OclDlg(VRenderFrame* frame) :
wxPanel(frame, wxID_ANY,
wxDefaultPosition, wxSize(550, 600),
0, "OclDlg"),
m_frame(frame),
m_view(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;
	//
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Kernel file:",
		wxDefaultPosition, wxSize(70, 20));
	m_kernel_file_txt = new wxTextCtrl(this, ID_KernelFileTxt, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_browse_btn = new wxButton(this, ID_BrowseBtn, "Browse",
		wxDefaultPosition, wxSize(70, 23));
	m_save_btn = new wxButton(this, ID_SaveBtn, "Save",
		wxDefaultPosition, wxSize(70, 23));
	m_saveas_btn = new wxButton(this, ID_SaveAsBtn, "Save As",
		wxDefaultPosition, wxSize(70, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_kernel_file_txt, 1, wxEXPAND);
	sizer_1->Add(m_browse_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_saveas_btn, 0, wxALIGN_CENTER);

	//controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Controls:",
		wxDefaultPosition, wxSize(70, 20));
	m_execute_btn = new wxButton(this, ID_ExecuteBtn, "Run",
		wxDefaultPosition, wxSize(60, 23));
	m_execute_n_btn = new wxButton(this, ID_ExecuteNBtn, "Run N Times",
		wxDefaultPosition, wxSize(80, 23));
	m_iterations_sldr = new wxSlider(this, ID_IterationsSldr, 1, 1, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_iterations_txt = new wxTextCtrl(this, ID_IterationsTxt, "1",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_execute_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_execute_n_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_iterations_sldr, 1, wxEXPAND);
	sizer_2->Add(m_iterations_txt, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);

	//output
	m_output_txt = new wxTextCtrl(this, ID_OutputTxt, "",
		wxDefaultPosition, wxSize(-1, 100), wxTE_READONLY|wxTE_MULTILINE);

	//list
	m_kernel_list = new wxListCtrl(this, ID_KernelList,
		wxDefaultPosition, wxSize(-1, -1), wxLC_REPORT | wxLC_SINGLE_SEL);
	wxListItem itemCol;
	itemCol.SetText("Kernel Files");
	m_kernel_list->InsertColumn(0, itemCol);
	m_kernel_list->SetColumnWidth(0, 100);
	//stc
	m_LineNrID = 0;
	m_DividerID = 1;
	m_FoldingID = 2;
	m_kernel_edit_stc = new wxStyledTextCtrl(
		this, ID_KernelEditStc,
		wxDefaultPosition, wxDefaultSize);
	wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	m_kernel_edit_stc->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->SetLexer(wxSTC_LEX_CPP);
	m_kernel_edit_stc->SetMarginType(m_LineNrID, wxSTC_MARGIN_NUMBER);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	m_kernel_edit_stc->SetMarginWidth(m_LineNrID, 50);
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LASTPREDEFINED + 1, wxColour(244, 220, 220));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LASTPREDEFINED + 1, *wxBLACK);
	m_kernel_edit_stc->StyleSetSizeFractional(wxSTC_STYLE_LASTPREDEFINED + 1,
		(m_kernel_edit_stc->StyleGetSizeFractional(wxSTC_STYLE_DEFAULT) * 4) / 5);
	m_kernel_edit_stc->SetWrapMode(wxSTC_WRAP_WORD); // other choice is wxSCI_WRAP_NONE
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_STRING, wxColour(150, 0, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(165, 105, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(40, 0, 60));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_NUMBER, wxColour(0, 150, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_CHARACTER, wxColour(150, 0, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_WORD, wxColour(0, 0, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_WORD2, wxColour(0, 150, 0));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENT, wxColour(150, 150, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(150, 150, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(150, 150, 150));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, wxColour(0, 0, 200));
	m_kernel_edit_stc->StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour(0, 0, 200));
	m_kernel_edit_stc->StyleSetBold(wxSTC_C_WORD, true);
	m_kernel_edit_stc->StyleSetBold(wxSTC_C_WORD2, true);
	m_kernel_edit_stc->StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);
	// a sample list of keywords, I haven't included them all to keep it short...
	m_kernel_edit_stc->SetKeyWords(0, wxT("return for while break continue __kernel kernel_main __global"));
	m_kernel_edit_stc->SetKeyWords(1, wxT("const int float void char double unsigned int4 float4 signed"));
	m_kernel_edit_stc->SetMarginType(m_FoldingID, wxSTC_MARGIN_SYMBOL);
	m_kernel_edit_stc->SetMarginMask(m_FoldingID, wxSTC_MASK_FOLDERS);
	m_kernel_edit_stc->StyleSetBackground(m_FoldingID, *wxWHITE);
	// markers
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("WHITE"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("WHITE"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));
	m_kernel_edit_stc->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY, wxT("BLACK"), wxT("BLACK"));

	//sizer
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	sizer_3->Add(m_kernel_list, 0, wxEXPAND);
	wxStaticText * separator = new wxStaticText(this, 0, "", wxDefaultPosition, wxSize(5, -1));
	sizer_3->Add(separator, 0, wxEXPAND);
	sizer_3->Add(m_kernel_edit_stc, 1, wxEXPAND);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 4, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_output_txt, 1, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

OclDlg::~OclDlg()
{
}

void OclDlg::GetSettings(VRenderGLView* view)
{
	if (!view) return;
	m_view = view;

	AddKernelsToList();
}

VRenderGLView* OclDlg::GetView()
{
	return m_view;
}

void OclDlg::OnBrowseBtn(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose an OpenCL kernel file", 
		"", "", "OpenCL kernel file|*.cl;*.txt", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		m_kernel_edit_stc->LoadFile(filename);
		m_kernel_edit_stc->EmptyUndoBuffer();
		m_kernel_file_txt->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void OclDlg::OnSaveBtn(wxCommandEvent& event)
{
	wxString filename = m_kernel_file_txt->GetValue();
	if (filename == "")
	{
		wxCommandEvent e;
		OnSaveAsBtn(e);
	}
	else
		m_kernel_edit_stc->SaveFile(filename);
}

void OclDlg::OnSaveAsBtn(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		m_frame, "Choose an OpenCL kernel file", 
		"", "", "OpenCL kernel file|*.cl;*.txt", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		rval = m_kernel_edit_stc->SaveFile(filename);
		if (rval) {
			m_kernel_file_txt->SetValue(filename);
			filename = wxFileNameFromPath(filename);
			wxString exePath = wxStandardPaths::Get().GetExecutablePath();
			exePath = wxPathOnly(exePath);
			wxString temp = exePath + GETSLASH() + "CL_code" +
				GETSLASH() + filename;
			m_kernel_edit_stc->SaveFile(temp);
			filename = filename.BeforeFirst('.');
			m_kernel_list->InsertItem(m_kernel_list->GetItemCount(), filename);
		}
	}

	if (fopendlg)
		delete fopendlg;
}

void OclDlg::OnExecuteBtn(wxCommandEvent& event)
{
	Execute();
}

void OclDlg::OnExecuteNBtn(wxCommandEvent& event)
{
	wxString str = m_iterations_txt->GetValue();
	unsigned long ival;
	str.ToULong(&ival);

	for (int i=0; i<ival; ++i)
		Execute();
}

void OclDlg::OnIterationsChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	if (str != m_iterations_txt->GetValue())
		m_iterations_txt->SetValue(str);
}

void OclDlg::OnIterationsEdit(wxCommandEvent &event)
{
	wxString str = m_iterations_txt->GetValue();
	unsigned long ival;
	str.ToULong(&ival);
	m_iterations_sldr->SetValue(ival);
}

void OclDlg::AddKernelsToList()
{
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	exePath = wxPathOnly(exePath);
	m_kernel_list->DeleteAllItems();
	wxString loc = exePath + GETSLASH() + "CL_code" +
		GETSLASH() + "*.cl";
	wxLogNull logNo;
	wxArrayString list;
	wxString file = wxFindFirstFile(loc);
	while (!file.empty())
	{
		file = wxFileNameFromPath(file);
		file = file.BeforeLast('.');
		list.Add(file);
		file = wxFindNextFile();
	}
	list.Sort();
	for (size_t i = 0; i < list.GetCount(); ++i)
		m_kernel_list->InsertItem(
			m_kernel_list->GetItemCount(),
			list[i]);
}

void OclDlg::Execute()
{
	m_output_txt->SetValue("");

	if (!m_view)
		return;
	KernelExecutor* executor = m_view->GetKernelExecutor();
	if (!executor)
		return;

	//currently, this is expected to be a convolution/filering kernel
	//get cl code
	wxString code = m_kernel_edit_stc->GetText();

	//get volume currently selected
	fluo::VolumeData* vd = m_view->m_cur_vol;
	if (!vd)
		return;
	bool dup = true;
	wxString vd_name = vd->getName();
	if (vd_name.Find("_CL") != wxNOT_FOUND)
		dup = false;

	executor->SetVolume(vd);
	executor->SetCode(code);
	executor->SetDuplicate(dup);
	executor->Execute();

	/*	Texture* tex = vd->GetTexture();
	void* result = executor->GetResult()->GetTexture()->get_nrrd(0)->data;
	int res_x, res_y, res_z;
	vd->GetResolution(res_x, res_y, res_z);
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	min_filter(tex->get_nrrd(0)->data, result,
	res_x, res_y, res_z);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2-t1);
	(*m_output_txt) << "CPU time: " << time_span.count() << " sec.\n";*/

	wxString str;
	executor->GetMessage(str);
	(*m_output_txt) << str;

	//add result for rendering
	if (dup)
	{
		fluo::VolumeData* vd_r = executor->GetResult(true);
		if (!vd_r)
			return;
		if (m_frame)
		{
			m_frame->GetDataManager()->AddVolumeData(vd_r);
			m_view->AddVolumeData(vd_r);
			vd->setValue(gstDisplay, false);
			m_frame->UpdateList();
			m_frame->UpdateTree(vd_r->getName());
		}
	}

	m_view->RefreshGL(39);
}

void OclDlg::OnKernelListSelected(wxListEvent& event)
{
	long item = m_kernel_list->GetNextItem(-1,
		wxLIST_NEXT_ALL,
		wxLIST_STATE_SELECTED);

	if (item != -1)
	{
		wxString file = m_kernel_list->GetItemText(item);
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		file = exePath + GETSLASH() + "CL_code" +
			GETSLASH() + file + ".cl";
		m_kernel_edit_stc->LoadFile(file);
		m_kernel_edit_stc->EmptyUndoBuffer();
		m_kernel_file_txt->SetValue(file);
	}
}

void OclDlg::copy_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	for (int bi=0; bi<brick_x; ++bi)
	for (int bj=0; bj<brick_y; ++bj)
	for (int bk=0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		unsigned char rvalue = ((unsigned char*)data)[index];
		((unsigned char*)result)[index] = rvalue;
	}
}

void OclDlg::box_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi=0; bi<brick_x; ++bi)
	for (int bj=0; bj<brick_y; ++bj)
	for (int bk=0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		double rvalue = 0;
		for (int i=0; i<kx; ++i)
		for (int j=0; j<ky; ++j)
		for (int k=0; k<kz; ++k)
		{
			int cx = bi+i-kx/2;
			int cy = bj+j-ky/2;
			int cz = bk+k-kz/2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x-1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y-1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z-1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			rvalue += 1.0/(kx*ky*kz) * dvalue;
		}
		((unsigned char*)result)[index] = rvalue;
	}
}

void OclDlg::gauss_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi=0; bi<brick_x; ++bi)
	for (int bj=0; bj<brick_y; ++bj)
	for (int bk=0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		double rvalue = 0;
		for (int i=0; i<kx; ++i)
		for (int j=0; j<ky; ++j)
		for (int k=0; k<kz; ++k)
		{
			int cx = bi+i-kx/2;
			int cy = bj+j-ky/2;
			int cz = bk+k-kz/2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x-1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y-1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z-1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			double s = 10.0;
			double r = (i-kx/2)*(i-kx/2)+(j-ky/2)*(j-ky/2)+(k-kz/2)*(k-kz/2);
			rvalue += exp(-r/s)/pow(3.1415*s, 1.5) * dvalue;
		}
		((unsigned char*)result)[index] = rvalue;
	}
}

void OclDlg::median_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi=0; bi<brick_x; ++bi)
	for (int bj=0; bj<brick_y; ++bj)
	for (int bk=0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		unsigned char* rvalue = new unsigned char[kx*ky*kz];
		int id = 0;
		int c;
		unsigned char temp;
		for (int i=0; i<kx; ++i)
		for (int j=0; j<ky; ++j)
		for (int k=0; k<kz; ++k)
		{
			int cx = bi+i-kx/2;
			int cy = bj+j-ky/2;
			int cz = bk+k-kz/2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x-1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y-1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z-1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			rvalue[id] = dvalue;
			if (id > 0)
			{
				c = id;
				while (c > 0)
				{
					if (rvalue[c] < rvalue[c-1])
					{
						temp = rvalue[c];
						rvalue[c] = rvalue[c-1];
						rvalue[c-1] = temp;
					}
					else break;
					c--;
				}
			}
			id++;
		}
		((unsigned char*)result)[index] = rvalue[kx*ky*kz/2-1];
		delete []rvalue;
	}
}

void OclDlg::min_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi=0; bi<brick_x; ++bi)
	for (int bj=0; bj<brick_y; ++bj)
	for (int bk=0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		unsigned char rvalue = 255;
		for (int i=0; i<kx; ++i)
		for (int j=0; j<ky; ++j)
		for (int k=0; k<kz; ++k)
		{
			int cx = bi+i-kx/2;
			int cy = bj+j-ky/2;
			int cz = bk+k-kz/2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x-1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y-1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z-1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			rvalue = min(rvalue, dvalue);
		}
		((unsigned char*)result)[index] = rvalue;
	}
}

void OclDlg::max_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi=0; bi<brick_x; ++bi)
	for (int bj=0; bj<brick_y; ++bj)
	for (int bk=0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		unsigned char rvalue = 0;
		for (int i=0; i<kx; ++i)
		for (int j=0; j<ky; ++j)
		for (int k=0; k<kz; ++k)
		{
			int cx = bi+i-kx/2;
			int cy = bj+j-ky/2;
			int cz = bk+k-kz/2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x-1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y-1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z-1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			rvalue = max(rvalue, dvalue);
		}
		((unsigned char*)result)[index] = rvalue;
	}
}

void OclDlg::sobel_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	int krnx[] = 
	{ 1, 0, -1,
		2, 0, -2,
		1, 0, -1,
		2, 0, -2,
		4, 0, -4,
		2, 0, -2,
		1, 0, -1,
		2, 0, -2,
		1, 0, -1 };
	int krny[] =
	{ 1, 2, 1,
		0, 0, 0,
		-1, -2, -1,
		2, 4, 2,
		0, 0, 0,
		-2, -4, -2,
		1, 2, 1,
		0, 0, 0,
		-1, -2, -1 };
	int krnz[] =
	{ 1, 2, 1,
		2, 4, 2,
		1, 2, 1,
		0, 0, 0,
		0, 0, 0,
		0, 0, 0,
		-1, -2, -1,
		-2, -4, -2,
		-1, -2, -1 };

	for (int bi = 0; bi<brick_x; ++bi)
	for (int bj = 0; bj<brick_y; ++bj)
	for (int bk = 0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		double rx = 0;
		double ry = 0;
		double rz = 0;
		for (int i = 0; i<kx; ++i)
		for (int j = 0; j<ky; ++j)
		for (int k = 0; k<kz; ++k)
		{
			int cx = bi + i - kx / 2;
			int cy = bj + j - ky / 2;
			int cz = bk + k - kz / 2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x - 1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y - 1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z - 1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			rx += krnx[kx*ky*k + kx*j + i] * dvalue;
			ry += krny[kx*ky*k + kx*j + i] * dvalue;
			rz += krnz[kx*ky*k + kx*j + i] * dvalue;
		}
		double rvalue = sqrt(rx*rx + ry*ry + rz*rz);
		((unsigned char*)result)[index] = rvalue;
	}
}

void OclDlg::morph_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi<brick_x; ++bi)
	for (int bj = 0; bj<brick_y; ++bj)
	for (int bk = 0; bk<brick_z; ++bk)
	{
		unsigned int index = brick_x*brick_y*bk +
			brick_x*bj + bi;
		unsigned char rvalue = 0;
		for (int i = 0; i<kx; ++i)
		for (int j = 0; j<ky; ++j)
		for (int k = 0; k<kz; ++k)
		{
			int cx = bi + i - kx / 2;
			int cy = bj + j - ky / 2;
			int cz = bk + k - kz / 2;
			if (cx < 0) cx = 0;
			if (cx >= brick_x) cx = brick_x - 1;
			if (cy < 0) cy = 0;
			if (cy >= brick_y) cy = brick_y - 1;
			if (cz < 0) cz = 0;
			if (cz >= brick_z) cz = brick_z - 1;

			unsigned int kc = brick_x*brick_y*cz +
				brick_x*cy + cx;
			unsigned char dvalue = ((unsigned char*)data)[kc];
			rvalue = max(rvalue, dvalue);
		}
		unsigned int kc = brick_x*brick_y*bk +
			brick_x*bj + bi;
		unsigned char dvalue = ((unsigned char*)data)[kc];
		((unsigned char*)result)[index] = rvalue - dvalue;
	}
}

