/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "VRenderFrame.h"
#include "VRenderView.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <boost/chrono.hpp>

using namespace boost::chrono;

BEGIN_EVENT_TABLE(OclDlg, wxPanel)
	EVT_BUTTON(ID_BrowseBtn, OclDlg::OnBrowseBtn)
	EVT_BUTTON(ID_SaveBtn, OclDlg::OnSaveBtn)
	EVT_BUTTON(ID_SaveAsBtn, OclDlg::OnSaveAsBtn)
	EVT_BUTTON(ID_ExecuteBtn, OclDlg::OnExecuteBtn)
END_EVENT_TABLE()

OclDlg::OclDlg(wxWindow* frame,
wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(500, 600),
0, "OclDlg"),
m_frame(parent),
m_view(0)
{
	wxStaticText *st = 0;
	//
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Kernel file:",
		wxDefaultPosition, wxSize(70, 20));
	m_kernel_file_txt = new wxTextCtrl(this, ID_KernelFileTxt, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_browse_btn = new wxButton(this, ID_BrowseBtn, "Browse",
		wxDefaultPosition, wxSize(60, 23));
	m_save_btn = new wxButton(this, ID_SaveBtn, "Save",
		wxDefaultPosition, wxSize(60, 23));
	m_saveas_btn = new wxButton(this, ID_SaveAsBtn, "Save As",
		wxDefaultPosition, wxSize(60, 23));
	sizer_1->Add(5, 5);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_kernel_file_txt, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_1->Add(m_browse_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_save_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_saveas_btn, 0, wxALIGN_CENTER);

	//controls
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Controls:",
		wxDefaultPosition, wxSize(70, 20));
	m_execute_btn = new wxButton(this, ID_ExecuteBtn, "Run",
		wxDefaultPosition, wxSize(60, 23));
	sizer_2->Add(5, 5);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_execute_btn, 0, wxALIGN_CENTER);

	//output
	m_output_txt = new wxTextCtrl(this, ID_OutputTxt, "",
		wxDefaultPosition, wxSize(-1, 100), wxTE_READONLY|wxTE_MULTILINE);

	//
    m_LineNrID = 0;
    m_DividerID = 1;
    m_FoldingID = 2;
	m_kernel_edit_stc = new wxStyledTextCtrl(
		this, ID_KernelEditStc,
		wxDefaultPosition, wxDefaultSize);
    wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
    m_kernel_edit_stc->StyleSetFont (wxSTC_STYLE_DEFAULT, font);
    m_kernel_edit_stc->StyleSetForeground (wxSTC_STYLE_DEFAULT, *wxBLACK);
    m_kernel_edit_stc->StyleSetBackground (wxSTC_STYLE_DEFAULT, *wxWHITE);
    m_kernel_edit_stc->StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (wxT("DARK GREY")));
    m_kernel_edit_stc->StyleSetBackground (wxSTC_STYLE_LINENUMBER, *wxWHITE);
    m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour (wxT("DARK GREY")));
	m_kernel_edit_stc->SetLexer(wxSTC_LEX_CPP);
	m_kernel_edit_stc->SetMarginType(m_LineNrID, wxSTC_MARGIN_NUMBER);
	m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	m_kernel_edit_stc->SetMarginWidth(m_LineNrID, 50);
    m_kernel_edit_stc->StyleSetBackground(wxSTC_STYLE_LASTPREDEFINED + 1, wxColour(244, 220, 220));
    m_kernel_edit_stc->StyleSetForeground(wxSTC_STYLE_LASTPREDEFINED + 1, *wxBLACK);
    m_kernel_edit_stc->StyleSetSizeFractional(wxSTC_STYLE_LASTPREDEFINED + 1,
            (m_kernel_edit_stc->StyleGetSizeFractional(wxSTC_STYLE_DEFAULT)*4)/5);
    //// default fonts for all styles!
    //int Nr;
    //for (Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++) {
    //    wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
    //    m_kernel_edit_stc->StyleSetFont (Nr, font);
    //}
	m_kernel_edit_stc->SetMarginType (m_FoldingID, wxSTC_MARGIN_SYMBOL);
	m_kernel_edit_stc->SetMarginMask (m_FoldingID, wxSTC_MASK_FOLDERS);
	m_kernel_edit_stc->StyleSetBackground (m_FoldingID, *wxWHITE);
    // markers
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY,     wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_DOTDOTDOT, wxT("BLACK"), wxT("WHITE"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, wxT("BLACK"), wxT("WHITE"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY,     wxT("BLACK"), wxT("BLACK"));
    m_kernel_edit_stc->MarkerDefine (wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY,     wxT("BLACK"), wxT("BLACK"));

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_kernel_edit_stc, 1, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(m_output_txt, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

OclDlg::~OclDlg()
{
}

void OclDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv) return;
	m_view = vrv;
}

VRenderView* OclDlg::GetView()
{
	return m_view;
}

void OclDlg::OnBrowseBtn(wxCommandEvent& event)
{
	wxFileDialog *fopendlg = new wxFileDialog(
		this, "Choose an OpenCL kernel file", 
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
		this, "Choose the FluoRender link data file", 
		"", "", "*.fll", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg->ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg->GetPath();
		rval = m_kernel_edit_stc->SaveFile(filename);
		if (rval)
			m_kernel_file_txt->SetValue(filename);
	}

	if (fopendlg)
		delete fopendlg;
}

void OclDlg::OnExecuteBtn(wxCommandEvent& event)
{
	m_output_txt->SetValue("");

	if (!m_view)
		return;

	//currently, this is expected to be a convolution/filering kernel
	//get cl code
	wxString code = m_kernel_edit_stc->GetText();

	//get volume currently selected
	VolumeData* vd = m_view->m_glview->m_cur_vol;
	if (!vd)
		return;
	bool dup = true;
	wxString vd_name = vd->GetName();
	if (vd_name.Find("_CL") != wxNOT_FOUND)
		dup = false;
	VolumeRenderer* vr = vd->GetVR();
	if (!vr)
		return;
	Texture* tex = vd->GetTexture();
	if (!tex)
		return;

	//get bricks
	Ray view_ray(Point(0,0,0), Vector(1,0,0));
	tex->set_sort_bricks();
	vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray);
	if (!bricks || bricks->size() == 0)
		return;

	int res_x, res_y, res_z;
	vd->GetResolution(res_x, res_y, res_z);

	//execute for each brick
	TextureBrick *b, *b_r;
	vector<TextureBrick*> *bricks_r;
	VolumeData* vd_r;
	void *result;

	if (dup)
	{
		//result
		double spc_x, spc_y, spc_z;
		vd->GetSpacings(spc_x, spc_y, spc_z);
		vd_r = new VolumeData();
		vd_r->AddEmptyData(8,
			res_x, res_y, res_z,
			spc_x, spc_y, spc_z);
		vd_r->SetSpcFromFile(true);
		wxString name = vd->GetName();
		vd_r->SetName(name+"_CL");
		Texture* tex_r = vd_r->GetTexture();
		if (!tex_r)
			return;
		Nrrd* nrrd_r = tex_r->get_nrrd(0);
		if (!nrrd_r)
			return;
		result = nrrd_r->data;
		if (!result)
			return;

		tex_r->set_sort_bricks();
		bricks_r = tex_r->get_sorted_bricks(view_ray);
		if (!bricks_r || bricks_r->size() == 0)
			return;
	}
	else
		result = tex->get_nrrd(0)->data;

	for (unsigned int i=0; i<bricks->size(); ++i)
	{
		b = (*bricks)[i];
		if (dup) b_r = (*bricks_r)[i];
		GLint data_id = vr->load_brick_cl(0, bricks, i);
		KernelProgram* kernel = VolumeRenderer::vol_kernel_factory_.kernel(code.ToStdString());
		if (kernel)
		{
			(*m_output_txt) << "OpenCL kernel created.\n";
			if (bricks->size()==1)
				ExecuteKernel(kernel, data_id, result, res_x, res_y, res_z);
			else
			{
				int brick_x = b->nx();
				int brick_y = b->ny();
				int brick_z = b->nz();
				unsigned char* bresult = new unsigned char[brick_x*brick_y*brick_z];
				ExecuteKernel(kernel, data_id, bresult, brick_x, brick_y, brick_z);
				//copy data back
				unsigned char* ptr_br = bresult;
				unsigned char* ptr_z;
				if (dup)
					ptr_z = (unsigned char*)(b_r->tex_data(0));
				else
					ptr_z = (unsigned char*)(b->tex_data(0));
				unsigned char* ptr_y;
				for (int bk=0; bk<brick_z; ++bk)
				{
					ptr_y = ptr_z;
					for (int bj=0; bj<brick_y; ++bj)
					{
						memcpy(ptr_y, ptr_br, brick_x);
						ptr_y += res_x;
						ptr_br += brick_x;
					}
					ptr_z += res_x*res_y;
				}
				delete []bresult;
			}
		}
		else
			(*m_output_txt) << "Fail to create OpenCL kernel.\n";
		//this is a problem needs to be solved
		VolumeRenderer::vol_kernel_factory_.clean();
	}

	//high_resolution_clock::time_point t1 = high_resolution_clock::now();
	//max_filter(tex->get_nrrd(0)->data, result,
	//	res_x, res_y, res_z);
	//high_resolution_clock::time_point t2 = high_resolution_clock::now();
	//duration<double> time_span = duration_cast<duration<double>>(t2-t1);
	//(*m_output_txt) << "CPU time: " << time_span.count() << " sec.\n";

	//add result for rendering
	if (dup)
	{
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
		{
			vr_frame->GetDataManager()->AddVolumeData(vd_r);
			m_view->AddVolumeData(vd_r);
			vd->SetDisp(false);
			vr_frame->UpdateList();
			vr_frame->UpdateTree(vd_r->GetName());
		}
	}
	else
	{
		vd->GetVR()->clear_tex_pool();
	}
	m_view->RefreshGL();
}

int OclDlg::ExecuteKernel(KernelProgram* kernel, GLuint data_id, void* result,
		size_t brick_x, size_t brick_y, size_t brick_z)
{
	if (!kernel)
		return 0;

	if (!kernel->valid())
	{
		string name = "main";
		if (kernel->create(name))
			(*m_output_txt) << "Kernel program compiled successfully.\n";
		else
		{
			(*m_output_txt) << "Kernel program failed to compile.\n";
			(*m_output_txt) << kernel->getInfo() << "\n";
		}
	}
	//textures
	kernel->setKernelArgTex3D(0, CL_MEM_READ_ONLY, data_id);
	size_t result_size = brick_x*brick_y*brick_z*sizeof(unsigned char);
	kernel->setKernelArgBuf(1, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, result_size, result);
	kernel->setKernelArgConst(2, sizeof(unsigned int), (void*)(&brick_x));
	kernel->setKernelArgConst(3, sizeof(unsigned int), (void*)(&brick_y));
	kernel->setKernelArgConst(4, sizeof(unsigned int), (void*)(&brick_z));
	//execute
	size_t global_size[3] = {brick_x, brick_y, brick_z};
	size_t local_size[3] = {1, 1, 1};
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	kernel->execute(3, global_size, local_size);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2-t1);
	wxString stime = wxString::Format("%.4f", time_span.count());
	(*m_output_txt) << "OpenCL time: " << stime << " sec.\n";
	kernel->readBuffer(1, result);

	return 0;
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
