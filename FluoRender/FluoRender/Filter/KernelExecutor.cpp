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
#include <KernelProgram.h>
#include <Global.h>
#include <VolumeDefault.h>
#include <VolKernel.h>
#include <DataManager.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <Ray.h>
#include <compatibility.h>
#include <filesystem>
#include <fstream>
#include <chrono>

KernelExecutor::KernelExecutor()
	: Progress(),
	m_duplicate(true),
	m_repeat(0),
	m_file_index(0)
{
}

KernelExecutor::~KernelExecutor()
{
}

void KernelExecutor::SetCode(const std::string &code)
{
	m_code = code;
}

void KernelExecutor::LoadCode(const std::wstring &filename)
{
	if (!std::filesystem::exists(filename))
	{
		m_message = L"Kernel file " +
			filename + L" doesn't exist.\n";
		return;
	}
#ifdef _WIN32
	std::ifstream input(filename);
#else
    std::ifstream input(ws2s(filename));
#endif
	if (!input)
	{
		m_message = L"Kernel file " +
			filename + L" reading failed.\n";
		return;
	}
	std::ostringstream ss;
	ss << input.rdbuf();
	m_code = ss.str();
	m_message = L"Kernel file " +
		filename + L" read.\n";
}

void KernelExecutor::SetVolume(const std::shared_ptr<VolumeData>& vd)
{
	m_vd = vd;
}

void KernelExecutor::SetDuplicate(bool dup)
{
	m_duplicate = dup;
}

std::shared_ptr<VolumeData> KernelExecutor::GetVolume()
{
	return m_vd.lock();
}

std::shared_ptr<VolumeData> KernelExecutor::GetResult(bool pop)
{
	if (!m_vd_r.empty())
	{
		auto vd = m_vd_r.back();
		if (pop)
			m_vd_r.pop_back();
		return vd;
	}
	return nullptr;
}

std::wstring KernelExecutor::GetInfo()
{
	return m_message;
}

bool KernelExecutor::Execute()
{
	if (m_code == "")
	{
		m_message = L"No OpenCL code to execute.\n";
		return false;
	}

#ifdef _DARWIN
	CGLContextObj ctx = CGLGetCurrentContext();
	if (ctx != flvr::KernelProgram::gl_context_)
		CGLSetCurrentContext(flvr::KernelProgram::gl_context_);
#endif

	//get volume currently selected
	auto vd = m_vd.lock();
	if (!vd)
	{
		m_message = L"No volume selected. Select a volume first.\n";
		return false;
	}
	flvr::Texture* tex =vd->GetTexture();
	if (!tex)
	{
		m_message = L"Volume corrupted.\n";
		return false;
	}

	int bits = vd->GetBits();
	int res_x, res_y, res_z;
	vd->GetResolution(res_x, res_y, res_z);
	int brick_size = vd->GetTexture()->get_build_max_tex_size();

	//get bricks
	fluo::Ray view_ray(fluo::Point(0.802, 0.267, 0.534), fluo::Vector(0.802, 0.267, 0.534));
	tex->set_sort_bricks();
	std::vector<flvr::TextureBrick*> *bricks = tex->get_sorted_bricks(view_ray);
	if (!bricks || bricks->size() == 0)
	{
		m_message = L"Volume empty.\n";
		return false;
	}

	m_message = L"";
	//execute for each brick
	std::vector<flvr::TextureBrick*> *bricks_r;
	auto vd_r = std::make_shared<VolumeData>();

	SetProgress(0, "Running OpenCL kernel.");

	if (m_duplicate)
	{
		//result
		double spc_x, spc_y, spc_z;
		vd->GetSpacings(spc_x, spc_y, spc_z);
		m_vd_r.push_back(vd_r);
		vd_r->AddEmptyData(bits,
			res_x, res_y, res_z,
			spc_x, spc_y, spc_z,
			brick_size);
		vd_r->SetSpcFromFile(true);
		vd_r->SetName(vd->GetName() + L"_CL");
		flvr::Texture* tex_r = vd_r->GetTexture();
		if (!tex_r)
			return false;

		tex_r->set_sort_bricks();
		bricks_r = tex_r->get_sorted_bricks(view_ray);
		if (!bricks_r || bricks_r->size() == 0)
			return false;

		glbin_vol_def.Copy(vd_r.get(), vd.get());
	}
	else
		vd_r = vd;

	bool kernel_exe = ExecuteKernel(vd.get(), vd_r.get());
	for (int i = 0; i < m_repeat; ++i)
	{
		int prg = static_cast<int>((i + 1) * 100.0 / m_repeat);
		SetProgress(0, "Running OpenCL kernel.");
		SetRange(static_cast<int>((i + 1) * 100.0 / m_repeat), static_cast<int>((i + 2) * 100.0 / m_repeat));
		kernel_exe &= ExecuteKernel(vd_r.get(), vd_r.get());
	}

	if (!kernel_exe)
	{
		m_vd_r.pop_back();
		SetProgress(0, "");
		return false;
	}

	if (!m_duplicate)
	{
		//clear gpu texture because the kernel updates the data in main memory after read back
		vd->GetVR()->clear_tex_current();
	}

	SetRange(0, 100);
	SetProgress(0, "");

	return true;
}

bool KernelExecutor::ExecuteKernel(VolumeData* vd, VolumeData* vd_r)
{
	bool kernel_exe = true;

	flvr::VolumeRenderer* vr = vd->GetVR();
	if (!vr)
	{
		m_message = L"Volume corrupted.\n";
		return false;
	}
	flvr::Texture* tex =vd->GetTexture();
	if (!tex)
	{
		m_message = L"Volume corrupted.\n";
		return false;
	}
	std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
	if (!bricks)
		return false;

	flvr::Texture* tex_r = vd_r->GetTexture();
	if (!tex_r)
		return false;
	std::vector<flvr::TextureBrick*> *bricks_r = tex_r->get_bricks();
	if (!bricks_r)
		return false;
	void* result = vd_r->GetVolume(false)->data;

	size_t brick_num = bricks->size();
	int bits = vd->GetBits();
	int chars = bits / 8;
	int res_x, res_y, res_z;
	vd->GetResolution(res_x, res_y, res_z);
	float max_int = static_cast<float>(vd->GetMaxValue());

	flvr::TextureBrick *b, *b_r;
	for (size_t i = 0; i<brick_num; ++i)
	{
		SetProgress(static_cast<int>(100.0 * i / brick_num), "Running OpenCL kernel.");

		b = (*bricks)[i];
		b_r = (*bricks_r)[i];
		GLint data_id = vr->load_brick(b);
		flvr::KernelProgram* kernel =
			glbin_vol_kernel_factory.
			kernel(m_code, bits, max_int);
		if (kernel)
		{
			m_message += L"OpenCL kernel created.\n";
			if (brick_num == 1)
				kernel_exe = ExecuteKernelBrick(kernel, data_id, result, res_x, res_y, res_z, chars);
			else
			{
				int brick_x = b->nx();
				int brick_y = b->ny();
				int brick_z = b->nz();
				void* bresult = (void*)(new unsigned char[brick_x*brick_y*brick_z*chars]);
				kernel_exe = ExecuteKernelBrick(kernel, data_id, bresult, brick_x, brick_y, brick_z, chars);
				if (!kernel_exe)
					break;
				//copy data back
				unsigned char* ptr_br = (unsigned char*)bresult;
				unsigned char* ptr_z;
				ptr_z = (unsigned char*)(b_r->tex_data(0));
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
			m_message += L"Fail to create OpenCL kernel.\n";
			kernel_exe = false;
			break;
		}
	}

	//clear gpu texture because the kernel updates the data in main memory after read back
	if (vd == vd_r)
		vd->GetVR()->clear_tex_current();

	return kernel_exe;
}

bool KernelExecutor::ExecuteKernelBrick(flvr::KernelProgram* kernel,
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
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	kernel->executeKernel(kernel_index, 3, global_size, local_size);
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	std::wstring stime = std::to_wstring(time_span.count());
	m_message += L"OpenCL time on ";
	m_message += s2ws(kernel->get_device_name());
	m_message += L": ";
	m_message += stime;
	m_message += L" sec.\n";
	kernel->readBuffer(result_size, result, result);

	//release buffer
	kernel->releaseMemObject(kernel_index, 0, 0, data_id);
	kernel->releaseMemObject(kernel_index, 1, result_size, 0);
	return true;
}
