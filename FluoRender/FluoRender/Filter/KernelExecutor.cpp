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
#include <KernelExecutor.h>
#include <Global.h>
#include <KernelProgram.h>
#include <VolKernel.h>
#include <boost/chrono.hpp>
#include <filesystem>

using namespace boost::chrono;

KernelExecutor::KernelExecutor()
	: Progress(),
	m_vd(0),
	m_duplicate(true)
{
}

KernelExecutor::~KernelExecutor()
{
}

void KernelExecutor::SetCode(const std::string &code)
{
	m_code = code;
}

void KernelExecutor::LoadCode(const std::string &filename)
{
	if (!std::filesystem::exists(filename))
	{
		m_message = "Kernel file " +
			filename + " doesn't exist.\n";
		return;
	}
	std::ifstream input(filename);
	if (!input)
	{
		m_message = "Kernel file " +
			filename + " reading failed.\n";
		return;
	}
	std::ostringstream ss;
	ss << input.rdbuf();
	m_code = ss.str();
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

VolumeData* KernelExecutor::GetResult(bool pop)
{
	VolumeData* vd = 0;
	if (!m_vd_r.empty())
	{
		vd = m_vd_r.back();
		if (pop)
			m_vd_r.pop_back();
	}
	return vd;
}

std::string KernelExecutor::GetMessage()
{
	return m_message;
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
	if (ctx != flvr::KernelProgram::gl_context_)
		CGLSetCurrentContext(flvr::KernelProgram::gl_context_);
#endif

	//get volume currently selected
	if (!m_vd)
	{
		m_message = "No volume selected. Select a volume first.\n";
		return false;
	}
	flvr::VolumeRenderer* vr = m_vd->GetVR();
	if (!vr)
	{
		m_message = "Volume corrupted.\n";
		return false;
	}
	flvr::Texture* tex =m_vd->GetTexture();
	if (!tex)
	{
		m_message = "Volume corrupted.\n";
		return false;
	}

	int bits = m_vd->GetBits();
	int chars = bits / 8;
	int res_x, res_y, res_z;
	m_vd->GetResolution(res_x, res_y, res_z);
	int brick_size = m_vd->GetTexture()->get_build_max_tex_size();

	//get bricks
	fluo::Ray view_ray(fluo::Point(0.802, 0.267, 0.534), fluo::Vector(0.802, 0.267, 0.534));
	tex->set_sort_bricks();
	std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray);
	if (!bricks || bricks->size() == 0)
	{
		m_message = "Volume empty.\n";
		return false;
	}

	m_message = "";
	//execute for each brick
	flvr::TextureBrick *b, *b_r;
	std::vector<flvr::TextureBrick*> *bricks_r;
	void *result;

	SetProgress(0, "Running OpenCL kernel.");

	if (m_duplicate)
	{
		//result
		double spc_x, spc_y, spc_z;
		m_vd->GetSpacings(spc_x, spc_y, spc_z);
		VolumeData* vd = new VolumeData();
		m_vd_r.push_back(vd);
		vd->AddEmptyData(bits,
			res_x, res_y, res_z,
			spc_x, spc_y, spc_z,
			brick_size);
		vd->SetSpcFromFile(true);
		vd->SetName(m_vd->GetName() + "_CL");
		flvr::Texture* tex_r = vd->GetTexture();
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

		glbin_vol_def.Copy(vd, m_vd);
	}
	else
		result = tex->get_nrrd(0)->data;

	bool kernel_exe = true;
	size_t brick_num = bricks->size();

	for (size_t i = 0; i<brick_num; ++i)
	{
		SetProgress(100.0 * i / brick_num, "Running OpenCL kernel.");

		b = (*bricks)[i];
		if (m_duplicate) b_r = (*bricks_r)[i];
		GLint data_id = vr->load_brick(b);
		flvr::KernelProgram* kernel =
			glbin_vol_kernel_factory.
			kernel(m_code, bits);
		if (kernel)
		{
			m_message += "OpenCL kernel created.\n";
			if (brick_num == 1)
				kernel_exe = ExecuteKernel(kernel, data_id, result, res_x, res_y, res_z, chars);
			else
			{
				int brick_x = b->nx();
				int brick_y = b->ny();
				int brick_z = b->nz();
				void* bresult = (void*)(new unsigned char[brick_x*brick_y*brick_z*chars]);
				kernel_exe = ExecuteKernel(kernel, data_id, bresult, brick_x, brick_y, brick_z, chars);
				if (!kernel_exe)
					break;
				//copy data back
				unsigned char* ptr_br = (unsigned char*)bresult;
				unsigned char* ptr_z;
				if (m_duplicate)
					ptr_z = (unsigned char*)(b_r->tex_data(0));
				else
					ptr_z = (unsigned char*)(b->tex_data(0));
				unsigned char* ptr_y;
				for (int bk = 0; bk < brick_z; ++bk)
				{
					ptr_y = ptr_z;
					for (int bj = 0; bj < brick_y; ++bj)
					{
						memcpy(ptr_y, ptr_br, brick_x*chars);
						ptr_y += res_x*chars;
						ptr_br += brick_x*chars;
					}
					ptr_z += res_x*res_y*chars;
				}
				delete[] bresult;
			}
		}
		else
		{
			m_message += "Fail to create OpenCL kernel.\n";
			kernel_exe = false;
			break;
		}
	}

	if (!kernel_exe)
	{
		if (m_duplicate && !m_vd_r.empty())
			delete m_vd_r.back();
		m_vd_r.pop_back();
		SetProgress(0, "");
		return false;
	}

	if (!m_duplicate)
		m_vd->GetVR()->clear_tex_current();

	SetProgress(0, "");

	return true;
}

bool KernelExecutor::ExecuteKernel(flvr::KernelProgram* kernel,
	unsigned int data_id, void* result,
	size_t brick_x, size_t brick_y,
	size_t brick_z, int chars)
{
	if (!kernel)
		return false;

	int kernel_index = kernel->createKernel("kernel_main");

	//textures
	size_t result_size = brick_x*brick_y*brick_z*chars;
	kernel->setKernelArgBegin(kernel_index);
	kernel->setKernelArgTex3D(CL_MEM_READ_ONLY, data_id);
	kernel->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, result_size, result);
	kernel->setKernelArgConst(sizeof(unsigned int), (void*)(&brick_x));
	kernel->setKernelArgConst(sizeof(unsigned int), (void*)(&brick_y));
	kernel->setKernelArgConst(sizeof(unsigned int), (void*)(&brick_z));
	//execute
	size_t global_size[3] = { brick_x, brick_y, brick_z };
	size_t local_size[3] = { 1, 1, 1 };
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	kernel->executeKernel(kernel_index, 3, global_size, local_size);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	std::string stime = std::to_string(time_span.count());
	m_message += "OpenCL time on ";
	m_message += kernel->get_device_name().c_str();
	m_message += ": ";
	m_message += stime;
	m_message += " sec.\n";
	kernel->readBuffer(result_size, result, result);

	//release buffer
	kernel->releaseMemObject(kernel_index, 0, 0, data_id);
	kernel->releaseMemObject(kernel_index, 1, result_size, 0);
	return true;
}
