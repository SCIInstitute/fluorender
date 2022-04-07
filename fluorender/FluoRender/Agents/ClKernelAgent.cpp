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

#include <ClKernelAgent.hpp>
#include <ClKernelDlg.h>
#include <Global.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <KernelExecutor.h>

using namespace fluo;

ClKernelAgent::ClKernelAgent(ClKernelDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void ClKernelAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* ClKernelAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void ClKernelAgent::UpdateAllSettings()
{
	wxString exePath = glbin.getExecutablePath();
	dlg_.m_kernel_list->DeleteAllItems();
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
		dlg_.m_kernel_list->InsertItem(
			dlg_.m_kernel_list->GetItemCount(),
			list[i]);
}

void ClKernelAgent::RunKernel(int v)
{
	for (int i = 0; i < v; ++i)
		RunKernel();
}

void ClKernelAgent::RunKernel()
{
	dlg_.m_output_txt->SetValue("");

	Renderview* view = getObject();
	if (!view)
		return;
	flrd::KernelExecutor* executor = view->GetKernelExecutor();
	if (!executor)
		return;

	//currently, this is expected to be a convolution/filering kernel
	//get cl code
	wxString code = dlg_.m_kernel_edit_stc->GetText();

	//get volume currently selected
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd)
		return;
	bool dup = true;
	wxString vd_name = vd->getName();
	if (vd_name.Find("_CL") != wxNOT_FOUND)
		dup = false;

	executor->SetVolume(vd);
	executor->SetCode(code.ToStdString());
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

	std::string str;
	executor->GetMessage(str);
	(*dlg_.m_output_txt) << str;

	//add result for rendering
	if (dup)
	{
		VolumeData* vd_r = executor->GetResult(true);
		if (!vd_r)
			return;
			//m_frame->GetDataManager()->AddVolumeData(vd_r);
			view->addVolumeData(vd_r, 0);
			vd->setValue(gstDisplay, false);
			//m_frame->UpdateList();
			//m_frame->UpdateTree(vd_r->getName());
	}

	//m_view->Update(39);
}

void ClKernelAgent::copy_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				unsigned char rvalue = ((unsigned char*)data)[index];
				((unsigned char*)result)[index] = rvalue;
			}
}

void ClKernelAgent::box_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				double rvalue = 0;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							rvalue += 1.0 / (kx*ky*kz) * dvalue;
						}
				((unsigned char*)result)[index] = rvalue;
			}
}

void ClKernelAgent::gauss_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				double rvalue = 0;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							double s = 10.0;
							double r = (i - kx / 2)*(i - kx / 2) + (j - ky / 2)*(j - ky / 2) + (k - kz / 2)*(k - kz / 2);
							rvalue += exp(-r / s) / pow(3.1415*s, 1.5) * dvalue;
						}
				((unsigned char*)result)[index] = rvalue;
			}
}

void ClKernelAgent::median_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				unsigned char* rvalue = new unsigned char[kx*ky*kz];
				int id = 0;
				int c;
				unsigned char temp;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							rvalue[id] = dvalue;
							if (id > 0)
							{
								c = id;
								while (c > 0)
								{
									if (rvalue[c] < rvalue[c - 1])
									{
										temp = rvalue[c];
										rvalue[c] = rvalue[c - 1];
										rvalue[c - 1] = temp;
									}
									else break;
									c--;
								}
							}
							id++;
						}
				((unsigned char*)result)[index] = rvalue[kx*ky*kz / 2 - 1];
				delete[]rvalue;
			}
}

void ClKernelAgent::min_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				unsigned char rvalue = 255;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							rvalue = std::min(rvalue, dvalue);
						}
				((unsigned char*)result)[index] = rvalue;
			}
}

void ClKernelAgent::max_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				unsigned char rvalue = 0;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							rvalue = std::max(rvalue, dvalue);
						}
				((unsigned char*)result)[index] = rvalue;
			}
}

void ClKernelAgent::sobel_filter(void* data, void* result,
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

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				double rx = 0;
				double ry = 0;
				double rz = 0;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							rx += krnx[kx*ky*k + kx * j + i] * dvalue;
							ry += krny[kx*ky*k + kx * j + i] * dvalue;
							rz += krnz[kx*ky*k + kx * j + i] * dvalue;
						}
				double rvalue = sqrt(rx*rx + ry * ry + rz * rz);
				((unsigned char*)result)[index] = rvalue;
			}
}

void ClKernelAgent::morph_filter(void* data, void* result,
	int brick_x, int brick_y, int brick_z)
{
	int kx = 3;
	int ky = 3;
	int kz = 3;

	for (int bi = 0; bi < brick_x; ++bi)
		for (int bj = 0; bj < brick_y; ++bj)
			for (int bk = 0; bk < brick_z; ++bk)
			{
				unsigned int index = brick_x * brick_y*bk +
					brick_x * bj + bi;
				unsigned char rvalue = 0;
				for (int i = 0; i < kx; ++i)
					for (int j = 0; j < ky; ++j)
						for (int k = 0; k < kz; ++k)
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

							unsigned int kc = brick_x * brick_y*cz +
								brick_x * cy + cx;
							unsigned char dvalue = ((unsigned char*)data)[kc];
							rvalue = std::max(rvalue, dvalue);
						}
				unsigned int kc = brick_x * brick_y*bk +
					brick_x * bj + bi;
				unsigned char dvalue = ((unsigned char*)data)[kc];
				((unsigned char*)result)[index] = rvalue - dvalue;
			}
}

