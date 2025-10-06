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
#include <CompGenerator.h>
#include <KernelProgram.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <CompKernelCode.h>
#include <EntryHist.h>
#include <EntryParams.h>
#include <Reshape.h>
#include <VolumeData.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <KernelFactory.h>
#include <VolumeRenderer.h>
#include <VolumeSelector.h>
#include <BaseConvVolMesh.h>
#include <CompSelector.h>
#include <TableHistParams.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>
#include <Plane.h>
#include <Count.h>
#include <compatibility.h>
#include <algorithm>
#ifdef _DEBUG
#include <Debug.h>
#endif

using namespace flrd;

ComponentGenerator::ComponentGenerator()
	:  Progress()
{
	//glbin_comp_def.Apply(this);
	prework = std::bind(
		&ComponentGenerator::StartTimer, this, std::placeholders::_1);
	postwork = std::bind(
		&ComponentGenerator::StopTimer, this, std::placeholders::_1);
}

ComponentGenerator::~ComponentGenerator()
{
}

bool ComponentGenerator::GetAutoCompGen()
{
	auto vd = m_vd.lock();
	if (!vd)
		return false;
	int get_comp_gen = glbin_automate_def.m_comp_gen;
	if (get_comp_gen == 0)
		return false;
	else if (get_comp_gen == 1)
		return true;
	else if (get_comp_gen == 2)
	{
		if (vd->GetAllBrickNum() > 1)
			return false;
	}
	return true;
}

void ComponentGenerator::SetVolumeData(const std::shared_ptr<VolumeData>& vd)
{
	m_vd = vd;
}

std::shared_ptr<VolumeData> ComponentGenerator::GetVolumeData()
{
	return m_vd.lock();
}

bool ComponentGenerator::CheckBricks()
{
	auto vd = m_vd.lock();
	if (!vd || !vd->GetTexture())
		return false;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

//high-level functions
void ComponentGenerator::Compute()
{
	if (m_use_ml)
		ApplyRecord();
	else
		GenerateComp();
}

void ComponentGenerator::GenerateComp(bool command)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	m_busy = true;
	//int clean_iter = m_clean_iter;
	//int clean_size = m_clean_size_vl;
	//if (!m_clean)
	//{
	//	clean_iter = 0;
	//	clean_size = 0;
	//}

	//get brick number
	int bn = vd->GetAllBrickNum();
	double scale = vd->GetScalarScale();

	//glbin_comp_generator.SetVolumeData(vd);
	m_titles.clear();
	m_values.clear();
	m_tps.clear();
	m_tps.push_back(std::chrono::high_resolution_clock::now());

	SetProgress(0, "Initializing component generation.");

	bool valid_mask = vd->IsValidMask();
	m_use_sel &= valid_mask;

	vd->AddEmptyMask(1, !m_use_sel);//select all if no mask, otherwise keep
	if (m_fixate && vd->GetLabel(false))
	{
		vd->LoadLabel2();
		SetIDBit(m_fix_size);
	}
	else
	{
		vd->AddEmptyLabel(0, !m_use_sel);
		ShuffleID();
	}

	SetProgress(10, "Generating components.");
	SetRange(10, 60);

	if (m_use_dist_field)
	{
		if (m_density)
			DistDensityField();
		else
			DistGrow();
	}
	else
	{
		if (m_density)
			DensityField();
		else
			Grow();
	}

	if (m_clean)
	{
		SetRange(60, 90);
		CleanNoise();
	}

	if (bn > 1)
	{
		SetRange(90, 100);
		FillBorders();
	}

	m_tps.push_back(std::chrono::high_resolution_clock::now());
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(
			m_tps.back() - m_tps.front());
	if (m_test_speed)
	{
		m_titles += L"Function\t";
		m_titles += L"Time\n";
		m_values += L"Total\t";
	}
	else
	{
		m_titles += L"Total time\n";
	}
	m_values += std::to_wstring(time_span.count());
	m_values += L" sec.\n";
	//SetOutput(m_titles, m_values);

	if (command && m_record_cmd)
		AddCmd("generate");

	vd->SetMlCompGenApplied(false);

	if (!valid_mask)
		glbin_comp_selector.All();

	SetRange(0, 100);
	SetProgress(0, "");
	m_busy = false;
}

void ComponentGenerator::Fixate(bool command)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;
	vd->PushLabel(true);

	if (command && m_record_cmd)
		AddCmd("fixate");
}

void ComponentGenerator::Clean(bool command)
{
	auto vd = m_vd.lock();
	if (!vd)
		return;

	m_busy = true;
	//int clean_iter = m_clean_iter;
	//int clean_size = m_clean_size_vl;
	//if (!m_clean)
	//{
	//	clean_iter = 0;
	//	clean_size = 0;
	//}

	//get brick number
	int bn = vd->GetAllBrickNum();

	//glbin_comp_generator.SetVolumeData(vd);
	//glbin_comp_def.Apply(&glbin_comp_generator);
	//glbin_comp_generator.SetUseMask(m_use_sel);

	SetProgress(0, "Initializing component generation.");

	vd->AddEmptyMask(1, !m_use_sel);

	if (bn > 1)
	{
		SetRange(0, 30);
		ClearBorders();
	}

	if (m_clean)
	{
		SetRange(30, 90);
		CleanNoise();
	}

	if (bn > 1)
	{
		SetRange(90, 100);
		FillBorders();
	}

	//m_view->RefreshGL(39);

	if (command && m_record_cmd)
		AddCmd("clean");

	SetRange(0, 100);
	SetProgress(0, "");
	m_busy = false;
}

void ComponentGenerator::ApplyRecord()
{
	//generate components using records
	auto vd = m_vd.lock();
	if (!vd)
		return;

	m_busy = true;
	//glbin_comp_generator.SetVolumeData(vd);
	SetProgress(0, "Initializing component generation.");

	vd->AddEmptyMask(1);
	if (!vd->GetMlCompGenApplied())
	{
		vd->AddEmptyLabel(0);
		ShuffleID();
	}

	SetProgress(10, "Generating components.");
	SetRange(10, 90);
	GenerateDB();

	int bn = vd->GetAllBrickNum();
	if (bn > 1)
	{
		SetRange(90, 100);
		FillBorders();
	}

	//update
	//m_view->RefreshGL(39);

	vd->SetMlCompGenApplied(true);

	SetRange(0, 100);
	SetProgress(0, "");
	m_busy = false;
}

void ComponentGenerator::ShuffleID()
{
	if (!CheckBricks())
		return;

	auto vd = m_vd.lock();
	if (!vd)
		return;
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_shuffle_id_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_sel)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	//clipping planes
	cl_float4 p[6];
	if (auto vr = vd->GetVR())
	{
		std::vector<fluo::Plane*> *planes = vr->get_planes();
		double abcd[4];
		for (size_t i = 0; i < 6; ++i)
		{
			(*planes)[i]->get(abcd);
			p[i] = { float(abcd[0]),
				float(abcd[1]),
				float(abcd[2]),
				float(abcd[3]) };
		}
	}

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		SetProgress(static_cast<int>(100 * count / brick_num),
			"Shuffling IDs.");
		count++;

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel)
		{
			if (!b->is_mask_valid())
				continue;
		}
		else
			b->valid_mask();

		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		//brick matrix
		fluo::BBox bbx = b->dbox();
		cl_float3 scl = {
			float(bbx.Max().x() - bbx.Min().x()),
			float(bbx.Max().y() - bbx.Min().y()),
			float(bbx.Max().z() - bbx.Min().z()) };
		cl_float3 trl ={
			float(bbx.Min().x()),
			float(bbx.Min().y()),
			float(bbx.Min().z())};
		kernel_prog->beginArgs(kernel_index);
		kernel_prog->setTex3D(CL_MEM_READ_ONLY, did);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenz));
		kernel_prog->setConst(sizeof(cl_float4), (void*)(p));
		kernel_prog->setConst(sizeof(cl_float4), (void*)(p+1));
		kernel_prog->setConst(sizeof(cl_float4), (void*)(p+2));
		kernel_prog->setConst(sizeof(cl_float4), (void*)(p+3));
		kernel_prog->setConst(sizeof(cl_float4), (void*)(p+4));
		kernel_prog->setConst(sizeof(cl_float4), (void*)(p+5));
		kernel_prog->setConst(sizeof(cl_float3), (void*)(&scl));
		kernel_prog->setConst(sizeof(cl_float3), (void*)(&trl));
		if (m_use_sel)
			kernel_prog->setTex3D(CL_MEM_READ_ONLY, mid);
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	glbin_kernel_factory.clear(kernel_prog);
}

void ComponentGenerator::SetIDBit(int psize)
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_set_bit_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0;
	//int kernel_index1;
	int kernel_index2;
	int kernel_index3;
	if (m_use_sel)
	{
		kernel_index0 = kernel_prog->createKernel("kernel_4");
		//kernel_index1 = kernel_prog->createKernel("kernel_5");
		kernel_index2 = kernel_prog->createKernel("kernel_6");
		kernel_index3 = kernel_prog->createKernel("kernel_7");
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel("kernel_0");
		//kernel_index1 = kernel_prog->createKernel("kernel_1");
		kernel_index2 = kernel_prog->createKernel("kernel_2");
		kernel_index3 = kernel_prog->createKernel("kernel_3");
	}

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		SetProgress(static_cast<int>(100 * count / brick_num),
			"Setting ID bits.");
		count++;

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel)
		{
			if (!b->is_mask_valid())
				continue;
		}
		else
			b->valid_mask();

		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		unsigned long long data_size =
			(unsigned long long)nx *
			(unsigned long long)ny *
			(unsigned long long)nz;
		unsigned long long label_size = data_size * 4;

		//set
		//kernel 0
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index0);
		auto arg_szbuf =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE, "arg_szbuf", label_size, nullptr);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenz));
		std::weak_ptr<flvr::Argument> arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog->setTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 2
		kernel_prog->beginArgs(kernel_index2);
		kernel_prog->bindArg(arg_szbuf);
		kernel_prog->bindArg(arg_label);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenz));
		if (m_use_sel)
			kernel_prog->bindArg(arg_mask);
		//kernel 3
		kernel_prog->beginArgs(kernel_index3);
		kernel_prog->bindArg(arg_szbuf);
		kernel_prog->bindArg(arg_label);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&psize));
		if (m_use_sel)
			kernel_prog->bindArg(arg_mask);

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	glbin_kernel_factory.clear(kernel_prog);
}

void ComponentGenerator::Grow()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	float scale = static_cast<float>(vd->GetScalarScale());
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_brainbow_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_sel)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t ticks = brick_num * m_iter;
	size_t count = 0;

	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		//auto iter
		int biter = m_iter;
		if (biter < 0)
			biter = std::max(std::max(nx, ny), nz);

		unsigned int rcnt = 0;
		unsigned int seed = biter > 10 ? biter : 11;
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		float scl_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float grad_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float tran = static_cast<float>(m_thresh * m_tfactor);
		int fixed = m_grow_fixed;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index0);
		kernel_prog->setTex3D(CL_MEM_READ_ONLY, did);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "", sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setConst(sizeof(float), (void*)(&tran));
		kernel_prog->setConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog->setConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog->setConst(sizeof(float), (void*)(&scale));
		kernel_prog->setConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog->setTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < biter; ++j)
		{
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;

			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		}

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();

	glbin_kernel_factory.clear(kernel_prog);
}

void ComponentGenerator::DensityField()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	float scale = static_cast<float>(vd->GetScalarScale());
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	//prog density
	flvr::KernelProgram* kernel_prog_dens = glbin_kernel_factory.program(str_cl_density_field_3d, bits, max_int);
	if (!kernel_prog_dens)
		return;
	int kernel_dens_index0 = kernel_prog_dens->createKernel("kernel_0");
	int kernel_dens_index1 = kernel_prog_dens->createKernel("kernel_1");
	int kernel_dens_index2 = kernel_prog_dens->createKernel("kernel_2");

	//prog grow
	flvr::KernelProgram* kernel_prog_grow = glbin_kernel_factory.program(str_cl_density_grow_3d, bits, max_int);
	if (!kernel_prog_grow)
		return;
	int kernel_grow_index0;
	if (m_use_sel)
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_1");
	else
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_0");

	//processing by brick
	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t ticks = (4 + m_iter) * brick_num;
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		//divide
		unsigned int gsx, gsy, gsz;//pixel number in group
		int ngx, ngy, ngz;//number of groups
		int dnx, dny, dnz;//adjusted n size
		int dnxy, ngxy;//precalculate
		gsx = m_density_stats_size >= nx ? nx : m_density_stats_size;
		gsy = m_density_stats_size >= ny ? ny : m_density_stats_size;
		gsz = m_density_stats_size >= nz ? nz : m_density_stats_size;
		ngx = nx / gsx + (nx % gsx ? 1 : 0);
		ngy = ny / gsy + (ny % gsy ? 1 : 0);
		ngz = nz / gsz + (nz % gsz ? 1 : 0);
		dnx = gsx * ngx;
		dny = gsy * ngy;
		dnz = gsz * ngz;
		dnxy = dnx * dny;
		ngxy = ngy * ngx;
		//sizes
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t global_size2[3] = { size_t(dnx), size_t(dny), size_t(dnz) };
		size_t local_size[3] = { 1, 1, 1 };

		//generate density field arg_densf
		//set
		//kernel 0
		kernel_prog_dens->beginArgs(kernel_dens_index0);
		auto arg_img =
			kernel_prog_dens->setTex3D(CL_MEM_READ_ONLY, did);
		auto arg_densf =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_densf", sizeof(unsigned char) * dnx * dny * dnz, nullptr);
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_dens->setConst(sizeof(int), (void*)(&m_density_window_size));
		kernel_prog_dens->setConst(sizeof(float), (void*)(&scale));
		//kernel 1
		kernel_prog_dens->beginArgs(kernel_dens_index1);
		kernel_prog_dens->bindArg(arg_densf);
		auto arg_gavg =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_gavg", sizeof(unsigned char) * ngx * ngy * ngz, nullptr);
		auto arg_gvar =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_gvar", sizeof(unsigned char) * ngx * ngy * ngz, nullptr);
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnx));
		//kernel 2
		kernel_prog_dens->beginArgs(kernel_dens_index2, 2);
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnx));

		//init
		kernel_prog_dens->executeKernel(kernel_dens_index0, 3, global_size2, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 densf(dnx, dny, 1);
//		kernel_prog_dens->readBuffer(arg_densf, densf.data);
//#endif
		//group avg and var
		global_size[0] = size_t(ngx); global_size[1] = size_t(ngy); global_size[2] = size_t(ngz);
		kernel_prog_dens->executeKernel(kernel_dens_index1, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 gvar(ngx, ngy, 1);
//		kernel_prog_dens->readBuffer(arg_gavg, gvar.data);
//		kernel_prog_dens->readBuffer(arg_gvar, gvar.data);
//#endif
		//compute avg
		global_size[0] = size_t(nx); global_size[1] = size_t(ny); global_size[2] = size_t(nz);
		kernel_prog_dens->beginArgs(kernel_dens_index2);
		auto arg_avg =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_avg", sizeof(unsigned char) * dnx * dny * dnz, nullptr);
		kernel_prog_dens->bindArg(arg_gavg);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//compute var
		kernel_prog_dens->beginArgs(kernel_dens_index2);
		auto arg_var =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_var", sizeof(unsigned char) * dnx * dny * dnz, nullptr);
		kernel_prog_dens->bindArg(arg_gvar);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;

		//release buffer
		kernel_prog_dens->releaseArg(arg_gavg);
		kernel_prog_dens->releaseArg(arg_gvar);

		//density grow
		unsigned int rcnt = 0;
		unsigned int seed = m_iter > 10 ? m_iter : 11;
		float scl_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float grad_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float tran = static_cast<float>(m_thresh * m_tfactor);
		float density = static_cast<float>(m_density_thresh);
		float varth = static_cast<float>(m_varth);
		int fixed = m_grow_fixed;

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog_grow->beginArgs(kernel_grow_index0);
		kernel_prog_grow->bindArg(arg_img);
		auto arg_label =
			kernel_prog_grow->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog_grow->bindArg(arg_densf);
		kernel_prog_grow->bindArg(arg_avg);
		kernel_prog_grow->bindArg(arg_var);
		kernel_prog_grow->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "", sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&tran));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&density));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&varth));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&scale));
		kernel_prog_grow->setConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog_grow->setTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < m_iter; ++j)
		{
			kernel_prog_grow->executeKernel(kernel_grow_index0, 3, global_size, local_size);

			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}

		//read back
		kernel_prog_grow->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog_grow->releaseAllArgs();
		kernel_prog_dens->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();

	glbin_kernel_factory.clear(kernel_prog_grow);
	glbin_kernel_factory.clear(kernel_prog_dens);
}

void ComponentGenerator::DistGrow()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	float scale = static_cast<float>(vd->GetScalarScale());
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	//prog dist
	flvr::KernelProgram* kernel_prog_dist = glbin_kernel_factory.program(str_cl_dist_field_2d, bits, max_int);
	if (!kernel_prog_dist)
		return;
	int kernel_dist_index0;
	int kernel_dist_index1;
	if (m_use_sel)
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_3");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_5");
	}
	else
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_0");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_2");
	}

	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_dist_grow_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0;
	if (m_use_sel)
		kernel_index0 = kernel_prog->createKernel("kernel_1");
	else
		kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t ticks = (1 + m_max_dist + m_iter) * brick_num;
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		float dist_thresh = static_cast<float>(m_dist_thresh);
		//generate distance field arg_distf
		unsigned char ini = 1;
		//set
		//kernel 0
		kernel_prog_dist->beginArgs(kernel_dist_index0);
		auto arg_img =
			kernel_prog_dist->setTex3D(CL_MEM_READ_ONLY, did);
		auto arg_distf =
			kernel_prog_dist->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_distf", sizeof(unsigned char) * nx * ny * nz, nullptr);
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setConst(sizeof(int), (void*)(&m_dist_filter_size));
		kernel_prog_dist->setConst(sizeof(float), (void*)(&dist_thresh));
		kernel_prog_dist->setConst(sizeof(float), (void*)(&scale));
		kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&ini));
		std::weak_ptr<flvr::Argument> arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog_dist->setTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog_dist->beginArgs(kernel_dist_index1);
		kernel_prog_dist->bindArg(arg_distf);
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&ini));
		if (m_use_sel)
		{
			kernel_prog_dist->beginArgs(kernel_dist_index1, 7);
			kernel_prog_dist->bindArg(arg_mask);
		}
		//init
		kernel_prog_dist->executeKernel(kernel_dist_index0, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		unsigned char nn, re;
		for (int j = 0; j < m_max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog_dist->beginArgs(kernel_dist_index1, 5);
			kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&nn));
			kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&re));
			kernel_prog_dist->executeKernel(kernel_dist_index1, 3, global_size, local_size);
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}

		//grow
		unsigned int rcnt = 0;
		unsigned int seed = m_iter > 10 ? m_iter : 11;
		float scl_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float grad_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float distscl = 5.0f / m_max_dist;
		float tran = static_cast<float>(m_thresh * m_tfactor);
		float dist_strength = static_cast<float>(m_dist_strength);
		int fixed = m_grow_fixed;
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index0);
		kernel_prog->bindArg(arg_img);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->bindArg(arg_distf);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "", sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setConst(sizeof(float), (void*)(&tran));
		kernel_prog->setConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog->setConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog->setConst(sizeof(float), (void*)(&scale));
		kernel_prog->setConst(sizeof(float), (void*)(&distscl));
		kernel_prog->setConst(sizeof(float), (void*)(&dist_strength));
		kernel_prog->setConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog->bindArg(arg_mask);

		//execute
		for (int j = 0; j < m_iter; ++j)
		{
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();
		kernel_prog_dist->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();

	glbin_kernel_factory.clear(kernel_prog);
	glbin_kernel_factory.clear(kernel_prog_dist);
}

void ComponentGenerator::DistDensityField()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	float scale = static_cast<float>(vd->GetScalarScale());
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	//prog dist
	flvr::KernelProgram* kernel_prog_dist = glbin_kernel_factory.program(str_cl_dist_field_2d, bits, max_int);
	if (!kernel_prog_dist)
		return;
	int kernel_dist_index0;
	int kernel_dist_index1;
	if (m_use_sel)
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_3");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_5");
	}
	else
	{
		kernel_dist_index0 = kernel_prog_dist->createKernel("kernel_0");
		kernel_dist_index1 = kernel_prog_dist->createKernel("kernel_2");
	}
	//prog density
	flvr::KernelProgram* kernel_prog_dens = glbin_kernel_factory.program(str_cl_distdens_field_3d, bits, max_int);
	if (!kernel_prog_dens)
		return;
	int kernel_dens_index0 = kernel_prog_dens->createKernel("kernel_0");
	int kernel_dens_index1 = kernel_prog_dens->createKernel("kernel_1");
	int kernel_dens_index2 = kernel_prog_dens->createKernel("kernel_2");

	//prog grow
	flvr::KernelProgram* kernel_prog_grow = glbin_kernel_factory.program(str_cl_density_grow_3d, bits, max_int);
	if (!kernel_prog_grow)
		return;
	int kernel_grow_index0;
	if (m_use_sel)
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_1");
	else
		kernel_grow_index0 = kernel_prog_grow->createKernel("kernel_0");

	//processing by brick
	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t ticks = (5 + m_max_dist + m_iter) * brick_num;
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		//divide
		unsigned int gsx, gsy, gsz;//pixel number in group
		int ngx, ngy, ngz;//number of groups
		int dnx, dny, dnz;//adjusted n size
		int nxy, dnxy, ngxy;//precalculate
		gsx = m_density_stats_size >= nx ? nx : m_density_stats_size;
		gsy = m_density_stats_size >= ny ? ny : m_density_stats_size;
		gsz = m_density_stats_size >= nz ? nz : m_density_stats_size;
		ngx = nx / gsx + (nx % gsx ? 1 : 0);
		ngy = ny / gsy + (ny % gsy ? 1 : 0);
		ngz = nz / gsz + (nz % gsz ? 1 : 0);
		dnx = gsx * ngx;
		dny = gsy * ngy;
		dnz = gsz * ngz;
		nxy = nx * ny;
		dnxy = dnx * dny;
		ngxy = ngy * ngx;
		//sizes
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t global_size2[3] = { size_t(dnx), size_t(dny), size_t(dnz) };
		size_t local_size[3] = { 1, 1, 1 };

		//generate distance field arg_distf
		unsigned char ini = 1;
		float dist_thresh = static_cast<float>(m_dist_thresh);
		//set
		//kernel 0
		kernel_prog_dist->beginArgs(kernel_dist_index0);
		auto arg_img =
			kernel_prog_dist->setTex3D(CL_MEM_READ_ONLY, did);
		auto arg_distf =
			kernel_prog_dist->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_distf", sizeof(unsigned char) * nx * ny * nz, nullptr);
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setConst(sizeof(int), (void*)(&m_dist_filter_size));
		kernel_prog_dist->setConst(sizeof(float), (void*)(&dist_thresh));
		kernel_prog_dist->setConst(sizeof(float), (void*)(&scale));
		kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&ini));
		std::weak_ptr<flvr::Argument> arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog_dist->setTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog_dist->beginArgs(kernel_dist_index1);
		kernel_prog_dist->bindArg(arg_distf);
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dist->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&ini));
		if (m_use_sel)
		{
			kernel_prog_dist->beginArgs(kernel_dist_index1, 7);
			kernel_prog_dist->bindArg(arg_mask);
		}
		//init
		kernel_prog_dist->executeKernel(kernel_dist_index0, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		unsigned char nn, re;
		for (int j = 0; j < m_max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog_dist->beginArgs(kernel_dist_index1, 5);
			kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&nn));
			kernel_prog_dist->setConst(sizeof(unsigned char), (void*)(&re));
			kernel_prog_dist->executeKernel(kernel_dist_index1, 3, global_size, local_size);
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 distf(nx, ny, 1);
//		kernel_prog_dist->readBuffer(arg_distf, distf.data);
//#endif

		//generate density field arg_densf
		//set
		//kernel 0
		float distscl = 5.0f / m_max_dist;
		float dist_strength = static_cast<float>(m_dist_strength);
		kernel_prog_dens->beginArgs(kernel_dens_index0);
		kernel_prog_dens->bindArg(arg_img);
		kernel_prog_dens->bindArg(arg_distf);
		auto arg_densf =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_densf", sizeof(unsigned char) * dnx * dny * dnz, nullptr);
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_dens->setConst(sizeof(int), (void*)(&m_density_window_size));
		kernel_prog_dens->setConst(sizeof(float), (void*)(&scale));
		kernel_prog_dens->setConst(sizeof(float), (void*)(&distscl));
		kernel_prog_dens->setConst(sizeof(float), (void*)(&dist_strength));
		//kernel 1
		kernel_prog_dens->beginArgs(kernel_dens_index1);
		kernel_prog_dens->bindArg(arg_densf);
		auto arg_gavg =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_gavg", sizeof(unsigned char) * ngx * ngy * ngz, nullptr);
		auto arg_gvar =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_gvar", sizeof(unsigned char) * ngx * ngy * ngz, nullptr);
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnx));
		//kernel 2
		kernel_prog_dens->beginArgs(kernel_dens_index2, 2);
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&gsz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngx));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&ngz));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_dens->setConst(sizeof(unsigned int), (void*)(&dnx));

		//init
		kernel_prog_dens->executeKernel(kernel_dens_index0, 3, global_size2, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 densf(dnx, dny, 1);
//		kernel_prog_dens->readBuffer(arg_densf, densf.data);
//#endif
		//group avg and var
		global_size[0] = size_t(ngx); global_size[1] = size_t(ngy); global_size[2] = size_t(ngz);
		kernel_prog_dens->executeKernel(kernel_dens_index1, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//compute avg
		global_size[0] = size_t(nx); global_size[1] = size_t(ny); global_size[2] = size_t(nz);
		kernel_prog_dens->beginArgs(kernel_dens_index2);
		auto arg_avg =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_avg", sizeof(unsigned char) * dnx * dny * dnz, nullptr);
		kernel_prog_dens->bindArg(arg_gavg);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//compute var
		kernel_prog_dens->beginArgs(kernel_dens_index2);
		auto arg_var =
			kernel_prog_dens->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, "arg_var", sizeof(unsigned char) * dnx * dny * dnz, nullptr);
		kernel_prog_dens->bindArg(arg_gvar);
		kernel_prog_dens->executeKernel(kernel_dens_index2, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;

		//release buffer
		kernel_prog_dens->releaseArg(arg_gavg);
		kernel_prog_dens->releaseArg(arg_gvar);
		kernel_prog_dens->releaseArg(arg_distf);

		//distance + density grow
		unsigned int rcnt = 0;
		unsigned int seed = m_iter > 10 ? m_iter : 11;
		float scl_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float grad_ff = m_diff ? static_cast<float>(m_falloff) : 0.0f;
		float tran = static_cast<float>(m_thresh * m_tfactor);
		int fixed = m_grow_fixed;
		float density = static_cast<float>(m_density_thresh);
		float varth = static_cast<float>(m_varth);

		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog_grow->beginArgs(kernel_grow_index0);
		kernel_prog_grow->bindArg(arg_img);
		auto arg_label =
			kernel_prog_grow->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog_grow->bindArg(arg_densf);
		kernel_prog_grow->bindArg(arg_avg);
		kernel_prog_grow->bindArg(arg_var);
		kernel_prog_grow->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "", sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&dnxy));
		kernel_prog_grow->setConst(sizeof(unsigned int), (void*)(&dnx));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&tran));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&scl_ff));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&grad_ff));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&density));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&varth));
		kernel_prog_grow->setConst(sizeof(float), (void*)(&scale));
		kernel_prog_grow->setConst(sizeof(int), (void*)(&fixed));
		if (m_use_sel)
			kernel_prog_grow->setTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		for (int j = 0; j < m_iter; ++j)
		{
			kernel_prog_grow->executeKernel(kernel_grow_index0, 3, global_size, local_size);
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}

		//read back
		kernel_prog_grow->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog_grow->releaseAllArgs();
		kernel_prog_dist->releaseAllArgs();
		kernel_prog_dens->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddEntry();

	glbin_kernel_factory.clear(kernel_prog_grow);
	glbin_kernel_factory.clear(kernel_prog_dist);
	glbin_kernel_factory.clear(kernel_prog_dens);
}

void ComponentGenerator::CleanNoise()
{
	if (m_clean_iter <= 0)
		return;
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_cleanup_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0;
	int kernel_index1;
	int kernel_index2;
	if (m_use_sel)
	{
		kernel_index0 = kernel_prog->createKernel("kernel_3");
		kernel_index1 = kernel_prog->createKernel("kernel_4");
		kernel_index2 = kernel_prog->createKernel("kernel_5");
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel("kernel_0");
		kernel_index1 = kernel_prog->createKernel("kernel_1");
		kernel_index2 = kernel_prog->createKernel("kernel_2");
	}

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t ticks = brick_num * m_clean_iter;
	size_t count = 0;

	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//bit length
		unsigned int lenx = 0;
		unsigned int r = std::max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			lenx++;
		}
		unsigned int lenz = 0;
		r = nz;
		while (r > 0)
		{
			r /= 2;
			lenz++;
		}

		unsigned long long data_size =
			(unsigned long long)nx *
			(unsigned long long)ny *
			(unsigned long long)nz;
		unsigned long long label_size = data_size * 4;

		//set
		//kernel 0
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index0);
		auto arg_szbuf =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE, "arg_szbuf", label_size, nullptr);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenz));
		std::weak_ptr<flvr::Argument> arg_mask;
		if (m_use_sel)
			arg_mask = kernel_prog->setTex3D(CL_MEM_READ_ONLY, mid);
		//kernel 1
		kernel_prog->beginArgs(kernel_index1);
		kernel_prog->bindArg(arg_szbuf);
		kernel_prog->bindArg(arg_label);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&lenz));
		if (m_use_sel)
			kernel_prog->bindArg(arg_mask);
		//kernel 2
		unsigned int size_lm = m_clean_size_vl;
		kernel_prog->beginArgs(kernel_index2);
		kernel_prog->setTex3D(CL_MEM_READ_ONLY, did);
		kernel_prog->bindArg(arg_szbuf);
		kernel_prog->bindArg(arg_label);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&size_lm));
		if (m_use_sel)
			kernel_prog->bindArg(arg_mask);

		//execute
		for (int j = 0; j < m_clean_iter; ++j)
		{
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);

			SetProgress(static_cast<int>(100 * count / ticks),
				"Cleaning components.");
			count++;
		}

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	if (glbin.get_cg_table_enable())
		AddCleanEntry();

	glbin_kernel_factory.clear(kernel_prog);
}

void ComponentGenerator::ClearBorders()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_clear_borders_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_sel)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		if (m_use_sel)
			kernel_prog->setTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / brick_num),
			"Clearing borders.");
		count++;

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	glbin_kernel_factory.clear(kernel_prog);
}

void ComponentGenerator::FillBorders()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_fill_borders_3d, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index;
	if (m_use_sel)
		kernel_index = kernel_prog->createKernel("kernel_1");
	else
		kernel_index = kernel_prog->createKernel("kernel_0");

	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks_id();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		if (m_use_sel && !b->is_mask_valid())
			continue;
		int bits = b->nb(0) * 8;
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_sel)
			mid = vd->GetVR()->load_brick_mask(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		float tol = static_cast<float>(m_fill_border);
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index);
		kernel_prog->setTex3D(CL_MEM_READ_ONLY, did);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setConst(sizeof(float), (void*)(&tol));
		if (m_use_sel)
			kernel_prog->setTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);

		SetProgress(static_cast<int>(100 * count / brick_num),
			"Filling borders.");
		count++;

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();

		if (postwork)
			postwork(__FUNCTION__);
	}

	glbin_kernel_factory.clear(kernel_prog);
}

void ComponentGenerator::AddEntry()
{
	//parameters
	flrd::EntryParams& ep = glbin.get_cg_entry();
	ep.setParam("iter", m_iter);
	ep.setParam("thresh", m_thresh);
	ep.setParam("use_dist_field", m_use_dist_field);
	ep.setParam("dist_strength", m_dist_strength);
	ep.setParam("dist_filter_size", m_dist_filter_size);
	ep.setParam("max_dist", m_max_dist);
	ep.setParam("dist_thresh", m_dist_thresh);
	ep.setParam("diff", m_diff);
	ep.setParam("falloff", m_falloff);
	ep.setParam("density", m_density);
	ep.setParam("density_thresh", m_density_thresh);
	ep.setParam("varth", m_varth);
	ep.setParam("density_window_size", m_density_window_size);
	ep.setParam("density_stats_size", m_density_stats_size);
	ep.setParam("grow_fixed", m_grow_fixed);
}

void ComponentGenerator::AddCleanEntry()
{
	flrd::EntryParams& ep = glbin.get_cg_entry();
	ep.setParam("cleanb", 1.0f);
	ep.setParam("clean_iter", m_clean_iter);
	ep.setParam("clean_size_vl", m_clean_size_vl);
}

void ComponentGenerator::GenerateDB()
{
	if (!CheckBricks())
		return;
	auto vd = m_vd.lock();
	if (!vd)
		return;

	flrd::TableHistParams& table = glbin.get_cg_table();
	//check table size
	size_t rec = table.getRecSize();
	unsigned int bin = EntryHist::m_bins;
	size_t par = flrd::Reshape::get_param_size("comp_gen");
	if (!(rec && bin && par))
		return;

	//iterration maximum from db
	int iter = static_cast<int>(table.getParamIter());
	int max_dist = static_cast<int>(table.getParamMxdist());//max iteration for distance field
	bool cleanb = table.getParamCleanb() > 0.0f;
	int clean_iter = static_cast<int>(table.getParamCleanIter());

	//histogram window size
	int whistxy = 20;//histogram size
	int whistz = 3;
	float hsize = table.getHistPopl();
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	float w;
	if (nz < 5)
	{
		w = std::sqrt(hsize);
		whistxy = static_cast<int>(std::ceil(w));
		whistz = 1;
	}
	else
	{
		w = static_cast<float>(std::pow((double)hsize / (nx / nz) / (ny / nz), double(1) / 3));
		whistz = static_cast<int>(std::ceil(w));
		whistxy = static_cast<int>(std::ceil(w * nx / nz));
	}
	//intensity scale
	float sscale = static_cast<float>(vd->GetScalarScale());
	long bits = vd->GetBits();
	float max_int = static_cast<float>(vd->GetMaxValue());

	//prog
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.program(str_cl_comp_gen_db, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0 = kernel_prog->createKernel("kernel_0");//generate lut
	int kernel_index1 = kernel_prog->createKernel("kernel_1");//init dist field
	int kernel_index2 = kernel_prog->createKernel("kernel_2");//generate dist field
	int kernel_index3 = kernel_prog->createKernel("kernel_3");//mix den and dist fields
	int kernel_index4 = kernel_prog->createKernel("kernel_4");//generate statistics
	int kernel_index5 = kernel_prog->createKernel("kernel_5");//grow
	int kernel_index6 = kernel_prog->createKernel("kernel_6");//count and store size
	int kernel_index7 = kernel_prog->createKernel("kernel_7");//set size to same comp
	int kernel_index8 = kernel_prog->createKernel("kernel_8");//size based grow

	//processing by brick
	size_t brick_num = vd->GetTexture()->get_brick_num();
	size_t ticks = (4 + max_dist + iter + cleanb ? clean_iter : 0) * brick_num;
	size_t count = 0;
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	for (size_t i = 0; i < brick_num; ++i)
	{
		if (prework)
			prework("");

		flvr::TextureBrick* b = (*bricks)[i];
		int bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
		GLint did = vd->GetVR()->load_brick(b);
		GLint lid = vd->GetVR()->load_brick_label(b);

		//precalculate
		unsigned int nxy, nxyz;
		nxy = nx * ny;
		nxyz = nxy * nz;
		//sizes
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//compute local histogram and generate lut
		kernel_prog->beginArgs(kernel_index0);
		auto arg_img =
			kernel_prog->setTex3D(CL_MEM_READ_ONLY, did);
		//rechist
		size_t fsize = bin * rec;
		float* rechist = new float[fsize]();
		table.getRecInput(rechist);
		//debug
//#ifdef _DEBUG
//		DBMIFLOAT32 histmi2;
//		histmi2.nx = bin; histmi2.ny = rec; histmi2.nc = 1; histmi2.nt = bin * 4; histmi2.data = rechist;
//#endif
		auto arg_rechist =
			kernel_prog->setBufIfNew(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "", sizeof(float) * fsize, (void*)(rechist));
		fsize = nx * ny * nz;
		cl_uchar* lut = new cl_uchar[fsize]();
		auto arg_lut =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "", sizeof(cl_uchar) * fsize, (void*)(lut));
		//local histogram
		kernel_prog->setLocal(sizeof(float)*bin);
		cl_int3 cl_histxyz = { cl_int(whistxy), cl_int(whistxy), cl_int(whistz) };
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_histxyz));
		cl_int3 cl_nxyz = { cl_int(nx), cl_int(ny), cl_int(nz) };
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		float minv = 0;
		float maxv = 1;
		if (bits > 8) maxv = float(1.0 / vd->GetScalarScale());
		kernel_prog->setConst(sizeof(float), (void*)(&minv));
		kernel_prog->setConst(sizeof(float), (void*)(&maxv));
		cl_uchar cl_bin = (cl_uchar)(bin);
		kernel_prog->setConst(sizeof(cl_uchar), (void*)(&cl_bin));
		cl_uchar cl_rec = (cl_uchar)(rec);
		kernel_prog->setConst(sizeof(cl_uchar), (void*)(&cl_rec));

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;

//#ifdef _DEBUG
//		//read back
//		kernel_prog->readBuffer(arg_lut, lut);
//		DBMIUINT8 mi;
//		mi.nx = nx; mi.ny = ny; mi.nc = 1; mi.nt = nx; mi.data = lut;
//#endif
		//release
		kernel_prog->releaseArg(arg_rechist);
		delete[] rechist;
		delete[] lut;

		//generate distance field arg_distf
		unsigned char ini = 1;
		//set
		//kernel 1
		kernel_prog->beginArgs(kernel_index1);
		kernel_prog->bindArg(arg_img);
		kernel_prog->bindArg(arg_lut);
		//params
		fsize = par * rec;
		float* params = new float[fsize]();
		table.getRecOutput(params);
		auto arg_params =
			kernel_prog->setBufIfNew(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			"", sizeof(float) * fsize, (void*)(params));
		auto arg_distf =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
			"arg_distf", sizeof(unsigned char) * nxyz, nullptr);
		kernel_prog->setConst(sizeof(float), (void*)(&sscale));
		kernel_prog->setConst(sizeof(unsigned char), (void*)(&ini));
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&par));
		//kernel 2
		kernel_prog->beginArgs(kernel_index2);
		kernel_prog->bindArg(arg_distf);
		kernel_prog->setConst(sizeof(unsigned char), (void*)(&ini));
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
		//init
		kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;

		unsigned char nn, re;
		for (int j = 0; j < max_dist; ++j)
		{
			nn = j == 0 ? 0 : j + ini;
			re = j + ini + 1;
			kernel_prog->beginArgs(kernel_index2, 4);
			kernel_prog->setConst(sizeof(unsigned char), (void*)(&nn));
			kernel_prog->setConst(sizeof(unsigned char), (void*)(&re));
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}

		//generate density field arg_densf
		//set
		//kernel 3
		kernel_prog->beginArgs(kernel_index3);
		kernel_prog->bindArg(arg_img);
		kernel_prog->bindArg(arg_distf);
		auto arg_densf =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				"arg_densf", sizeof(unsigned char) * nxyz, nullptr);
		kernel_prog->bindArg(arg_lut);
		kernel_prog->bindArg(arg_params);
		kernel_prog->setConst(sizeof(float), (void*)(&sscale));
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&par));
		//kernel 4
		kernel_prog->beginArgs(kernel_index4);
		kernel_prog->bindArg(arg_densf);
		auto arg_avg =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				"arg_avg", sizeof(unsigned char) * nxyz, nullptr);
		auto arg_var =
			kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY,
				"arg_var", sizeof(unsigned char) * nxyz, nullptr);
		kernel_prog->bindArg(arg_lut);
		kernel_prog->bindArg(arg_params);
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&par));

		//mix fields
		kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//gen avg and var
		kernel_prog->executeKernel(kernel_index4, 3, global_size, local_size);
		SetProgress(static_cast<int>(100 * count / ticks),
			"Generating components.");
		count++;
		//#ifdef _DEBUG
//		//read back
//		DBMIUINT8 densf(nx, ny, 1);
//		kernel_prog->readBuffer(arg_densf, densf.data);
//		kernel_prog->readBuffer(arg_avg, densf.data);
//		kernel_prog->readBuffer(arg_var, densf.data);
//		//kernel_prog->readBuffer(arg_distf, densf.data);
//#endif

		//release buffer
		kernel_prog->releaseArg(arg_distf);

		//grow
		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		//set
		size_t region[3] = { (size_t)nx, (size_t)ny, (size_t)nz };
		kernel_prog->beginArgs(kernel_index5);
		float iterf = 0;
		kernel_prog->setConst(sizeof(float), (void*)(&iterf));
		kernel_prog->bindArg(arg_img);
		auto arg_label =
			kernel_prog->copyTex3DToBuf(CL_MEM_READ_WRITE, lid, "arg_label", sizeof(unsigned int) * nx * ny * nz, region);
		kernel_prog->bindArg(arg_densf);
		kernel_prog->bindArg(arg_avg);
		kernel_prog->bindArg(arg_var);
		kernel_prog->setBufIfNew(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, "", sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->bindArg(arg_lut);
		kernel_prog->bindArg(arg_params);
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setConst(sizeof(float), (void*)(&sscale));
		kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
		kernel_prog->setConst(sizeof(unsigned int), (void*)(&par));

		//execute
		for (int j = 0; j < iter; ++j)
		{
			if (j)
			{
				iterf = static_cast<float>(j);
				kernel_prog->beginArgs(kernel_index5);
				kernel_prog->setConst(sizeof(float), (void*)(&iterf));
			}
			kernel_prog->executeKernel(kernel_index5, 3, global_size, local_size);
			SetProgress(static_cast<int>(100 * count / ticks),
				"Generating components.");
			count++;
		}

		//clean up
		if (cleanb)
		{
			//temp
			unsigned int size_lm = 5;

			//bit length
			unsigned int lenx = 0;
			unsigned int r = std::max(nx, ny);
			while (r > 0)
			{
				r /= 2;
				lenx++;
			}
			unsigned int lenz = 0;
			r = nz;
			while (r > 0)
			{
				r /= 2;
				lenz++;
			}
			cl_int3 cl_lenxyz = { cl_int(lenx), cl_int(lenx), cl_int(lenz) };

			unsigned long long data_size =
				(unsigned long long)nx *
				(unsigned long long)ny *
				(unsigned long long)nz;
			unsigned long long label_size = data_size * 4;

			//set
			//kernel 6
			kernel_prog->beginArgs(kernel_index6);
			auto arg_szbuf = kernel_prog->setBufIfNew(CL_MEM_READ_WRITE, "arg_szbuf", label_size, nullptr);
			kernel_prog->bindArg(arg_label);
			kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
			kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_lenxyz));
			//kernel 7
			kernel_prog->beginArgs(kernel_index7);
			kernel_prog->bindArg(arg_szbuf);
			kernel_prog->bindArg(arg_label);
			kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
			kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_lenxyz));
			//kernel 8
			kernel_prog->beginArgs(kernel_index8);
			kernel_prog->bindArg(arg_img);
			kernel_prog->bindArg(arg_szbuf);
			kernel_prog->bindArg(arg_label);
			kernel_prog->bindArg(arg_lut);
			kernel_prog->bindArg(arg_params);
			kernel_prog->setConst(sizeof(cl_int3), (void*)(&cl_nxyz));
			kernel_prog->setConst(sizeof(unsigned int), (void*)(&nxy));
			kernel_prog->setConst(sizeof(unsigned int), (void*)(&par));

			//execute
			for (int j = 0; j < clean_iter; ++j)
			{
				kernel_prog->executeKernel(kernel_index6, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index7, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index8, 3, global_size, local_size);
				SetProgress(static_cast<int>(100 * count / ticks),
					"Generating components.");
				count++;
			}
		}

		//read back
		kernel_prog->copyBufToTex3D(arg_label, lid,
			sizeof(unsigned int)*nx*ny*nz, region);

		//release buffer
		kernel_prog->releaseAllArgs();
		delete[] params;

		if (postwork)
			postwork(__FUNCTION__);
	}

	glbin_kernel_factory.clear(kernel_prog);
}

//command
void ComponentGenerator::LoadCmd(const std::wstring& filename)
{
	//m_cmd_file_text->ChangeValue(filename);
	std::shared_ptr<BaseTreeFile> fconfig =
		glbin_tree_file_factory.createTreeFile(filename, gstCompCommandFile);
	if (!fconfig)
		return;

	if (fconfig->LoadFile(filename))
		return;

	m_command.clear();
	int cmd_count = 0;
	std::string str;
	std::string cmd_str = "/cmd" + std::to_string(cmd_count);
	while (fconfig->Exists(cmd_str))
	{
		flrd::CompCmdParams params;
		fconfig->SetPath(cmd_str);
		fconfig->Read("type", &str, std::string(""));
		if (str == "generate" ||
			str == "clean" ||
			str == "fixate")
			params.push_back(str);
		else
			continue;
		long lval;
		if (fconfig->Read("iter", &lval))
		{
			params.push_back("iter"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("use_dist_field", &lval))
		{
			params.push_back("use_dist_field"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("dist_filter_size", &lval))
		{
			params.push_back("dist_filter_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("max_dist", &lval))
		{
			params.push_back("max_dist"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("diff", &lval))
		{
			params.push_back("diff"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("density", &lval))
		{
			params.push_back("density"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("density_window_size", &lval))
		{
			params.push_back("density_window_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("density_stats_size", &lval))
		{
			params.push_back("density_stats_size"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("cleanb", &lval))
		{
			params.push_back("cleanb"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("clean_iter", &lval))
		{
			params.push_back("clean_iter"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("clean_size_vl", &lval))
		{
			params.push_back("clean_size_vl"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("grow_fixed", &lval))
		{
			params.push_back("grow_fixed"); params.push_back(std::to_string(lval));
		}
		if (fconfig->Read("fix_size", &lval))
		{
			params.push_back("fix_size"); params.push_back(std::to_string(lval));
		}
		double dval;
		if (fconfig->Read("thresh", &dval))
		{
			params.push_back("thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig->Read("dist_strength", &dval))
		{
			params.push_back("dist_strength"); params.push_back(std::to_string(dval));
		}
		if (fconfig->Read("dist_thresh", &dval))
		{
			params.push_back("dist_thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig->Read("falloff", &dval))
		{
			params.push_back("falloff"); params.push_back(std::to_string(dval));
		}
		if (fconfig->Read("density_thresh", &dval))
		{
			params.push_back("density_thresh"); params.push_back(std::to_string(dval));
		}
		if (fconfig->Read("varth", &dval))
		{
			params.push_back("varth"); params.push_back(std::to_string(dval));
		}

		m_command.push_back(params);
		cmd_count++;
		cmd_str = "/cmd" + std::to_string(cmd_count);
	}
	//record
	//int ival = m_command.size();
	//m_cmd_count_text->ChangeValue(Format("%d", ival));
}

void ComponentGenerator::SaveCmd(const std::wstring& filename)
{
	if (m_command.empty())
	{
		AddCmd("generate");
	}

	std::shared_ptr<BaseTreeFile> fconfig =
		glbin_tree_file_factory.createTreeFile(
			glbin_settings.m_config_file_type, gstCompCommandFile);
	if (!fconfig)
		return;

	int cmd_count = 0;
	std::string str, str2;

	for (auto it = m_command.begin();
		it != m_command.end(); ++it)
	{
		if (it->empty())
			continue;
		if ((*it)[0] == "generate" ||
			(*it)[0] == "clean" ||
			(*it)[0] == "fixate")
		{
			str = "/cmd" + std::to_string(cmd_count++);
			fconfig->SetPath(str);
			str = (*it)[0];
			fconfig->Write("type", str);
		}
		for (auto it2 = it->begin();
			it2 != it->end(); ++it2)
		{
			if (*it2 == "iter" ||
				*it2 == "use_dist_field" ||
				*it2 == "dist_filter_size" ||
				*it2 == "max_dist" ||
				*it2 == "diff" ||
				*it2 == "density" ||
				*it2 == "density_window_size" ||
				*it2 == "density_stats_size" ||
				*it2 == "cleanb" ||
				*it2 == "clean_iter" ||
				*it2 == "clean_size_vl" ||
				*it2 == "fix_size" ||
				*it2 == "grow_fixed")
			{
				str = (*it2);
				++it2;
				str2 = (*it2);
				fconfig->Write(str, STOI(str2));
			}
			else if (*it2 == "thresh" ||
				*it2 == "dist_strength" ||
				*it2 == "dist_thresh" ||
				*it2 == "falloff" ||
				*it2 == "density_thresh" ||
				*it2 == "varth")
			{
				str = (*it2);
				++it2;
				str2 = (*it2);
				fconfig->Write(str, STOD(str2));
			}
		}
	}

	fconfig->SaveFile(filename);
	//m_cmd_file_text->ChangeValue(filename);
}

void ComponentGenerator::AddCmd(const std::string& type)
{
	if (!m_command.empty())
	{
		flrd::CompCmdParams& params = m_command.back();
		if (!params.empty())
		{
			if ((params[0] == "generate" ||
				params[0] == "fixate") &&
				params[0] == type)
			{
				//replace
				m_command.pop_back();
			}
			//else do nothing
		}
	}
	//add
	flrd::CompCmdParams params;
	if (type == "generate")
	{
		params.push_back("generate");
		params.push_back("iter"); params.push_back(std::to_string(m_iter));
		params.push_back("thresh"); params.push_back(std::to_string(m_thresh));
		params.push_back("use_dist_field"); params.push_back(std::to_string(m_use_dist_field));
		params.push_back("dist_strength"); params.push_back(std::to_string(m_dist_strength));
		params.push_back("dist_filter_size"); params.push_back(std::to_string(m_dist_filter_size));
		params.push_back("max_dist"); params.push_back(std::to_string(m_max_dist));
		params.push_back("dist_thresh"); params.push_back(std::to_string(m_dist_thresh));
		params.push_back("diff"); params.push_back(std::to_string(m_diff));
		params.push_back("falloff"); params.push_back(std::to_string(m_falloff));
		params.push_back("density"); params.push_back(std::to_string(m_density));
		params.push_back("density_thresh"); params.push_back(std::to_string(m_density_thresh));
		params.push_back("varth"); params.push_back(std::to_string(m_varth));
		params.push_back("density_window_size"); params.push_back(std::to_string(m_density_window_size));
		params.push_back("density_stats_size"); params.push_back(std::to_string(m_density_stats_size));
		params.push_back("cleanb"); params.push_back(std::to_string(m_clean));
		params.push_back("clean_iter"); params.push_back(std::to_string(m_clean_iter));
		params.push_back("clean_size_vl"); params.push_back(std::to_string(m_clean_size_vl));
		params.push_back("grow_fixed"); params.push_back(std::to_string(m_grow_fixed));
	}
	else if (type == "clean")
	{
		params.push_back("clean");
		params.push_back("clean_iter"); params.push_back(std::to_string(m_clean_iter));
		params.push_back("clean_size_vl"); params.push_back(std::to_string(m_clean_size_vl));
	}
	else if (type == "fixate")
	{
		params.push_back("fixate");
		params.push_back("fix_size"); params.push_back(std::to_string(m_fix_size));
	}
	m_command.push_back(params);

	//record
	//int ival = m_command.size();
	//m_cmd_count_text->ChangeValue(Format("%d", ival));
}

void ComponentGenerator::ResetCmd()
{
	m_command.clear();
	//m_record_cmd_btn->SetValue(false);
	m_record_cmd = false;
	//record
	//int ival = m_command.size();
	//m_cmd_count_text->ChangeValue(Format("%d", ival));
}

void ComponentGenerator::PlayCmd(double tfactor)
{
	//disable first
	m_fixate = false;
	//glbin_comp_def.m_auto_update = false;
	//m_auto_update_btn->SetValue(false);

	if (m_command.empty())
	{
		//the threshold factor is used to lower the threshold value for semi auto segmentation
		m_tfactor = tfactor;
		GenerateComp(false);
		m_tfactor = 1.0;
		return;
	}

	for (auto it = m_command.begin();
		it != m_command.end(); ++it)
	{
		if (it->empty())
			continue;
		if ((*it)[0] == "generate")
		{
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "iter")
					m_iter = STOI(*(++it2));
				else if (*it2 == "thresh")
					m_thresh = STOD(*(++it2));
				else if (*it2 == "use_dist_field")
					m_use_dist_field = STOI(*(++it2));
				else if (*it2 == "dist_strength")
					m_dist_strength = STOD(*(++it2));
				else if (*it2 == "dist_filter_size")
					m_dist_filter_size = STOI(*(++it2));
				else if (*it2 == "max_dist")
					m_max_dist = STOI(*(++it2));
				else if (*it2 == "dist_thresh")
					m_dist_thresh = STOD(*(++it2));
				else if (*it2 == "diff")
					m_diff = STOI(*(++it2));
				else if (*it2 == "falloff")
					m_falloff = STOD(*(++it2));
				else if (*it2 == "density")
					m_density = STOI(*(++it2));
				else if (*it2 == "density_thresh")
					m_density_thresh = STOD(*(++it2));
				else if (*it2 == "varth")
					m_varth = STOD(*(++it2));
				else if (*it2 == "density_window_size")
					m_density_window_size = STOI(*(++it2));
				else if (*it2 == "density_stats_size")
					m_density_stats_size = STOI(*(++it2));
				else if (*it2 == "cleanb")
					m_clean = STOI(*(++it2));
				else if (*it2 == "clean_iter")
					m_clean_iter = STOI(*(++it2));
				else if (*it2 == "clean_size_vl")
					m_clean_size_vl = STOI(*(++it2));
				else if (*it2 == "grow_fixed")
					m_grow_fixed = STOI(*(++it2));
			}
			GenerateComp(false);
		}
		else if ((*it)[0] == "clean")
		{
			m_clean = true;
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "clean_iter")
					m_clean_iter = STOI(*(++it2));
				else if (*it2 == "clean_size_vl")
					m_clean_size_vl = STOI(*(++it2));
			}
			Clean(false);
		}
		else if ((*it)[0] == "fixate")
		{
			m_fixate = true;
			for (auto it2 = it->begin();
				it2 != it->end(); ++it2)
			{
				if (*it2 == "fix_size")
					m_fix_size = STOI(*(++it2));
			}
			//GenerateComp();
			Fixate(false);
			//return;
		}
	}
	//Update();
}

void ComponentGenerator::SetRecordCmd(bool val)
{
	m_record_cmd = val;
}

bool ComponentGenerator::GetRecordCmd()
{
	return m_record_cmd;
}

size_t ComponentGenerator::GetCmdNum()
{
	return m_command.size();
}

bool ComponentGenerator::GetAutoThreshold()
{
	auto vd = m_vd.lock();
	if (!vd)
		return false;
	if (vd->IsAutoThresholdValid())
		return false;
	double threshold = vd->GetAutoThreshold();
	if (threshold != m_thresh)
	{
		m_thresh = threshold;
		glbin_vol_selector.SetBrushSclTranslate(m_thresh);
		glbin_conv_vol_mesh->SetIsoValue(m_thresh);
		return true;
	}
	return false;
}

void ComponentGenerator::StartTimer(const std::string& str)
{
	if (m_test_speed)
	{
		m_tps.push_back(std::chrono::high_resolution_clock::now());
	}
}

void ComponentGenerator::StopTimer(const std::string& str)
{
	if (m_test_speed)
	{
		auto t0 = m_tps.back();
		m_tps.push_back(std::chrono::high_resolution_clock::now());
		std::chrono::duration<double> time_span =
			std::chrono::duration_cast<std::chrono::duration<double>>(
				m_tps.back() - t0);

		m_values += s2ws(str) + L"\t";
		m_values += std::to_wstring(time_span.count());
		m_values += L" sec.\n";
	}
}

