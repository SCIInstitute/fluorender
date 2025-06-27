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
#include <VolumeRoi.h>
#include <KernelProgram.h>
#include <Global.h>
#include <VolumeRenderer.h>
#include <TextureBrick.h>
#include <Texture.h>
#include <VolKernel.h>
#include <DataManager.h>
#include <Ruler.h>
#ifdef _DEBUG
#include <Debug.h>
#endif

using namespace flrd;

const char* str_cl_volume_roi_ellipse = \
"#define DWL unsigned char\n" \
"#define VSCL 255\n" \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float eval_ellipse(float2 coord, float3 ectr, float4 eaxis)\n" \
"{\n" \
"	float2 v = coord;\n" \
"	v.x *= ectr.z;\n" \
"	v -= ectr.xy; \n" \
"	float2 l = (float2)(length(eaxis.xy), length(eaxis.zw));\n" \
"	float2 p = (float2)(dot(v, eaxis.xy / l.x), dot(v, eaxis.zw / l.y));\n" \
"	p = p / l;\n" \
"	return p.x * p.x + p.y * p.y;\n" \
"}\n" \
"//count\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	uint ngx,\n" \
"	uint ngy,\n" \
"	uint ngz,\n" \
"	uint gsxy,\n" \
"	uint gsx,\n" \
"	float4 spaces,\n" \
"	float4 mat0,\n" \
"	float4 mat1,\n" \
"	float4 mat2,\n" \
"	float4 mat3,\n" \
"	float3 ectr,\n" \
"	float4 eaxis,\n" \
"	__global uint* count,\n" \
"	__global float* wcount)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*ngx, gid.y*ngy, gid.z*ngz);\n" \
"	int3 ub = (int3)(lb.x + ngx, lb.y + ngy, lb.z + ngz);\n" \
"	int4 ijk = (int4)(0, 0, 0, 1);\n" \
"	uint lsum = 0;\n" \
"	float lwsum = 0.0f;\n" \
"	float val;\n" \
"	float4 coord;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		coord = (convert_float4(ijk) + (float4)(0.5f, 0.5f, 0.5f, 0.0f)) * spaces;\n" \
"		coord = (float4)(dot(coord, mat0), dot(coord, mat1), dot(coord, mat2), dot(coord, mat3));\n" \
"		coord /= coord.w;\n" \
"		if (eval_ellipse(coord.xy, ectr, eaxis) > 1.0f)\n" \
"			continue;\n" \
"		val = read_imagef(data, samp, ijk).x;\n" \
"		if (val > 0.0f)\n" \
"		{\n" \
"			lsum++;\n" \
"			lwsum += val;\n" \
"		}\n" \
"	}\n" \
"	uint index = gsxy * gid.z + gsx * gid.y + gid.x;\n" \
"	atomic_xchg(count+index, lsum);\n" \
"	atomic_xchg(wcount+index, lwsum);\n" \
"}\n" \
;

VolumeRoi::VolumeRoi(VolumeData* vd):
	m_vd(vd),
	m_use_mask(false),
	m_ruler(0),
	m_aspect(1)
{}

VolumeRoi::~VolumeRoi()
{}

bool VolumeRoi::CheckBricks()
{
	if (!m_vd)
		return false;
	if (m_use_mask && !m_vd->GetMask(false))
		return false;
	if (!m_vd->GetTexture())
		return false;
	int brick_num = m_vd->GetTexture()->get_brick_num();
	if (!brick_num)
		return false;
	return true;
}

bool VolumeRoi::GetInfo(
	flvr::TextureBrick* b,
	long& bits, long& nx, long& ny, long& nz)
{
	bits = b->nb(0) * 8;
	nx = b->nx();
	ny = b->ny();
	nz = b->nz();
	return true;
}

void VolumeRoi::Run()
{
	if (!m_ruler ||
		m_ruler->GetRulerMode() != RulerMode::Ellipse)//only ellipse ruler is supported
		return;
	if (!CheckBricks())
		return;
	long bits = m_vd->GetBits();
	float max_int = static_cast<float>(m_vd->GetMaxValue());

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_vol_kernel_factory.kernel(str_cl_volume_roi_ellipse, bits, max_int);
	if (!kernel_prog)
		return;
	int kernel_index0;
	kernel_index0 = kernel_prog->createKernel("kernel_0");

	size_t brick_num = m_vd->GetTexture()->get_brick_num();
	std::vector<flvr::TextureBrick*>* bricks = m_vd->GetTexture()->get_bricks();

	//init
	m_sum = 0;
	m_wsum = 0.0;
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	cl_float4 spaces = { cl_float(spcx), cl_float(spcy), cl_float(spcz), cl_float(1) };
	cl_float4 tf0 = {
		float(m_tf.get_mat_val(0, 0)),
		float(m_tf.get_mat_val(1, 0)),
		float(m_tf.get_mat_val(2, 0)),
		float(m_tf.get_mat_val(3, 0)) };
	cl_float4 tf1 = {
		float(m_tf.get_mat_val(0, 1)),
		float(m_tf.get_mat_val(1, 1)),
		float(m_tf.get_mat_val(2, 1)),
		float(m_tf.get_mat_val(3, 1)) };
	cl_float4 tf2 = {
		float(m_tf.get_mat_val(0, 2)),
		float(m_tf.get_mat_val(1, 2)),
		float(m_tf.get_mat_val(2, 2)),
		float(m_tf.get_mat_val(3, 2)) };
	cl_float4 tf3 = {
		float(m_tf.get_mat_val(0, 3)),
		float(m_tf.get_mat_val(1, 3)),
		float(m_tf.get_mat_val(2, 3)),
		float(m_tf.get_mat_val(3, 3)) };
	fluo::Point p0 = m_ruler->GetPoint(0);
	fluo::Point p1 = m_ruler->GetPoint(1);
	fluo::Point p2 = m_ruler->GetPoint(2);
	fluo::Point p3 = m_ruler->GetPoint(3);
	p0 = m_tf.transform(p0); p0.z(0);
	p1 = m_tf.transform(p1); p1.z(0);
	p2 = m_tf.transform(p2); p2.z(0);
	p3 = m_tf.transform(p3); p3.z(0);
	//adjust for aspect
	p0.x(p0.x() * m_aspect);
	p1.x(p1.x() * m_aspect);
	p2.x(p2.x() * m_aspect);
	p3.x(p3.x() * m_aspect);
	fluo::Point pc((p0 + p1) / 2);
	cl_float3 ectr = {
		cl_float(pc.x()),
		cl_float(pc.y()),
		cl_float(m_aspect)};
	cl_float4 eaxis = {
		cl_float((p2 - pc).x()),
		cl_float((p2 - pc).y()),
		cl_float((p0 - pc).x()),
		cl_float((p0 - pc).y())};

	//go through bricks
	for (size_t i = 0; i < brick_num; ++i)
	{
		flvr::TextureBrick* b = (*bricks)[i];
		long nx, ny, nz;
		if (!GetInfo(b, bits, nx, ny, nz))
			continue;
		//get tex ids
		GLint tid = m_vd->GetVR()->load_brick(b);
		GLint mid = 0;
		if (m_use_mask)
			mid = m_vd->GetVR()->load_brick_mask(b);

		//compute workload
		flvr::GroupSize gsize;
		kernel_prog->get_group_size(kernel_index0, nx, ny, nz, gsize);
		size_t local_size[3] = { 1, 1, 1 };
		size_t global_size[3] = {
			size_t(gsize.gsx), size_t(gsize.gsy), size_t(gsize.gsz) };

		//mean
		unsigned int* sum = new unsigned int[gsize.gsxyz];
		float* wsum = new float[gsize.gsxyz];
		kernel_prog->setKernelArgBegin(kernel_index0);
		kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, tid);
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngx));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.ngz));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsxy));
		kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&gsize.gsx));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&spaces));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf0));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf1));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf2));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&tf3));
		kernel_prog->setKernelArgConst(sizeof(cl_float3), (void*)(&ectr));
		kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&eaxis));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int) * (gsize.gsxyz), (void*)(sum));
		kernel_prog->setKernelArgBuf(CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float) * (gsize.gsxyz), (void*)(wsum));
		//if (m_use_mask)
		//	kernel_prog->setKernelArgTex3D(CL_MEM_READ_ONLY, mid);

		//execute
		kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int) * (gsize.gsxyz), sum, sum);
		kernel_prog->readBuffer(sizeof(float) * (gsize.gsxyz), wsum, wsum);

		//debug
//#ifdef _DEBUG
//		DBMIFLOAT32 mi;
//		mi.nx = gsize.gsx; mi.ny = gsize.gsy; mi.nc = 1; mi.nt = mi.nx * mi.nc * 4;
//		mi.data = wsum;
//#endif
		//sum
		for (size_t ii = 0; ii < gsize.gsxyz; ++ii)
		{
			m_sum += sum[ii];
			m_wsum += wsum[ii];
		}
		delete[] sum;
		delete[] wsum;
	}

	kernel_prog->releaseAll();
}

double VolumeRoi::GetResult()
{
	if (!m_vd)
		return 0;
	if (m_sum == 0)
		return 0;
	double result = m_wsum;
	int bits = m_vd->GetBits();
	switch (bits)
	{
	case 8:
		result *= 255;
		break;
	case 16:
		result *= 65535;
		break;
	}
	result /= m_sum;

	return result;
}
