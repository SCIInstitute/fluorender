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
#include "KernelExecutor.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <boost/chrono.hpp>

using namespace boost::chrono;

KernelExecutor::KernelExecutor()
	: m_vd(0),
	m_vd_r(0),
	m_duplicate(true)
{
}

KernelExecutor::~KernelExecutor()
{
}

void KernelExecutor::SetCode(wxString &code)
{
	m_code = code;
}

void KernelExecutor::LoadCode(wxString &filename)
{
	if (!wxFileExists(filename))
	{
		m_message = "Kernel file " +
			filename + " doesn't exist.\n";
		return;
	}
	wxFileInputStream input(filename);
	wxTextInputStream cl_file(input);
	if (!input.IsOk())
	{
		m_message = "Kernel file " +
			filename + " reading failed.\n";
		return;
	}
	m_code = "";
	while (!input.Eof())
	{
		m_code += cl_file.ReadLine();
		m_code += "\n";
	}
	m_message = "Kernel file " +
		filename + " read.\n";
}

void KernelExecutor::SetVolume(VolumeData *vd)
{
	m_vd = vd;
}

void KernelExecutor::SetDuplicate(bool dup)
{
	m_duplicate = dup;
}

VolumeData* KernelExecutor::GetVolume()
{
	return m_vd;
}

VolumeData* KernelExecutor::GetResult()
{
	return m_vd_r;
}

void KernelExecutor::DeleteResult()
{
	if (m_vd_r)
		delete m_vd_r;
	m_vd_r = 0;
}

bool KernelExecutor::GetMessage(wxString &msg)
{
	if (m_message == "")
		return false;
	else
	{
		msg = m_message;
		return true;
	}
}

bool KernelExecutor::Execute()
{
	if (m_code == "")
	{
		m_message = "No OpenCL code to execute.\n";
		return false;
	}

#ifdef _DARWIN
	CGLContextObj ctx = CGLGetCurrentContext();
	if (ctx != KernelProgram::gl_context_)
		CGLSetCurrentContext(KernelProgram::gl_context_);
#endif

	//get volume currently selected
	if (!m_vd)
	{
		m_message = "No volume selected. Select a volume first.\n";
		return false;
	}
	VolumeRenderer* vr = m_vd->GetVR();
	if (!vr)
	{
		m_message = "Volume corrupted.\n";
		return false;
	}
	Texture* tex =m_vd->GetTexture();
	if (!tex)
	{
		m_message = "Volume corrupted.\n";
		return false;
	}

	int res_x, res_y, res_z;
	m_vd->GetResolution(res_x, res_y, res_z);

	//get bricks
	Ray view_ray(Point(0.802, 0.267, 0.534), Vector(0.802, 0.267, 0.534));
	tex->set_sort_bricks();
	vector<TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray);
	if (!bricks || bricks->size() == 0)
	{
		m_message = "Volume empty.\n";
		return false;
	}

	m_message = "";
	//execute for each brick
	TextureBrick *b, *b_r;
	vector<TextureBrick*> *bricks_r;
	void *result;

	if (m_duplicate)
	{
		//result
		double spc_x, spc_y, spc_z;
		m_vd->GetSpacings(spc_x, spc_y, spc_z);
		m_vd_r = new VolumeData();
		m_vd_r->AddEmptyData(8,
			res_x, res_y, res_z,
			spc_x, spc_y, spc_z);
		m_vd_r->SetSpcFromFile(true);
		wxString name = m_vd->GetName();
		m_vd_r->SetName(name + "_CL");
		Texture* tex_r = m_vd_r->GetTexture();
		if (!tex_r)
			return false;
		Nrrd* nrrd_r = tex_r->get_nrrd(0);
		if (!nrrd_r)
			return false;
		result = nrrd_r->data;
		if (!result)
			return false;

		tex_r->set_sort_bricks();
		bricks_r = tex_r->get_sorted_bricks(view_ray);
		if (!bricks_r || bricks_r->size() == 0)
			return false;

		if (m_vd)
		{
			//clipping planes
			vector<Plane*> *planes = m_vd->GetVR() ? m_vd->GetVR()->get_planes() : 0;
			if (planes && m_vd_r->GetVR())
				m_vd_r->GetVR()->set_planes(planes);
			//transfer function
			m_vd_r->Set3DGamma(m_vd->Get3DGamma());
			m_vd_r->SetBoundary(m_vd->GetBoundary());
			m_vd_r->SetOffset(m_vd->GetOffset());
			m_vd_r->SetLeftThresh(m_vd->GetLeftThresh());
			m_vd_r->SetRightThresh(m_vd->GetRightThresh());
			FLIVR::Color col = m_vd->GetColor();
			m_vd_r->SetColor(col);
			m_vd_r->SetAlpha(m_vd->GetAlpha());
			//shading
			m_vd_r->SetShading(m_vd->GetShading());
			double amb, diff, spec, shine;
			m_vd->GetMaterial(amb, diff, spec, shine);
			m_vd_r->SetMaterial(amb, diff, spec, shine);
			//shadow
			m_vd_r->SetShadow(m_vd->GetShadow());
			double shadow;
			m_vd->GetShadowParams(shadow);
			m_vd_r->SetShadowParams(shadow);
			//sample rate
			m_vd_r->SetSampleRate(m_vd->GetSampleRate());
			//2d adjusts
			col = m_vd->GetGamma();
			m_vd_r->SetGamma(col);
			col = m_vd->GetBrightness();
			m_vd_r->SetBrightness(col);
			col = m_vd->GetHdr();
			m_vd_r->SetHdr(col);
			m_vd_r->SetSyncR(m_vd->GetSyncR());
			m_vd_r->SetSyncG(m_vd->GetSyncG());
			m_vd_r->SetSyncB(m_vd->GetSyncB());
		}
	}
	else
		result = tex->get_nrrd(0)->data;

	bool kernel_exe = true;
	for (unsigned int i = 0; i<bricks->size(); ++i)
	{
		b = (*bricks)[i];
		if (m_duplicate) b_r = (*bricks_r)[i];
		GLint data_id = vr->load_brick(0, 0, bricks, i);
		KernelProgram* kernel = VolumeRenderer::vol_kernel_factory_.kernel(m_code.ToStdString());
		if (kernel)
		{
			m_message += "OpenCL kernel created.\n";
			if (bricks->size() == 1)
				kernel_exe = ExecuteKernel(kernel, data_id, result, res_x, res_y, res_z);
			else
			{
				int brick_x = b->nx();
				int brick_y = b->ny();
				int brick_z = b->nz();
				unsigned char* bresult = new unsigned char[brick_x*brick_y*brick_z];
				kernel_exe = ExecuteKernel(kernel, data_id, bresult, brick_x, brick_y, brick_z);
				if (!kernel_exe)
					break;
				//copy data back
				unsigned char* ptr_br = bresult;
				unsigned char* ptr_z;
				if (m_duplicate)
					ptr_z = (unsigned char*)(b_r->tex_data(0));
				else
					ptr_z = (unsigned char*)(b->tex_data(0));
				unsigned char* ptr_y;
				for (int bk = 0; bk<brick_z; ++bk)
				{
					ptr_y = ptr_z;
					for (int bj = 0; bj<brick_y; ++bj)
					{
						memcpy(ptr_y, ptr_br, brick_x);
						ptr_y += res_x;
						ptr_br += brick_x;
					}
					ptr_z += res_x*res_y;
				}
				delete[]bresult;
			}
		}
		else
		{
			m_message += "Fail to create OpenCL kernel.\n";
			kernel_exe = false;
			break;
		}
		//this is a problem needs to be solved
		VolumeRenderer::vol_kernel_factory_.clean();
	}

	if (!kernel_exe)
	{
		if (m_duplicate && m_vd_r)
			delete m_vd_r;
		m_vd_r = 0;
		return false;
	}

	//update
	if (!m_duplicate)
		m_vd->GetVR()->clear_tex_pool();

	return true;
}

bool KernelExecutor::ExecuteKernel(KernelProgram* kernel,
	GLuint data_id, void* result,
	size_t brick_x, size_t brick_y,
	size_t brick_z)
{
	if (!kernel)
		return false;

	if (!kernel->valid())
	{
		string name = "kernel_main";
		if (kernel->create(name))
			m_message += "Kernel program compiled successfully on " +
			kernel->get_device_name() + ".\n";
		else
		{
			m_message += "Kernel program failed to compile on " +
				kernel->get_device_name() + ".\n";
			m_message += kernel->getInfo() + "\n";
			return false;
		}
	}
	//textures
	kernel->setKernelArgTex3D(0, CL_MEM_READ_ONLY, data_id);
	size_t result_size = brick_x*brick_y*brick_z*sizeof(unsigned char);
	kernel->setKernelArgBuf(1, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, result_size, result);
	kernel->setKernelArgConst(2, sizeof(unsigned int), (void*)(&brick_x));
	kernel->setKernelArgConst(3, sizeof(unsigned int), (void*)(&brick_y));
	kernel->setKernelArgConst(4, sizeof(unsigned int), (void*)(&brick_z));
	//execute
	size_t global_size[3] = { brick_x, brick_y, brick_z };
	size_t local_size[3] = { 1, 1, 1 };
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	kernel->execute(3, global_size, local_size);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	wxString stime = wxString::Format("%.4f", time_span.count());
	m_message += "OpenCL time on " +
		kernel->get_device_name() +
		": " + stime + " sec.\n";
	kernel->readBuffer(1, result);

	return true;
}
