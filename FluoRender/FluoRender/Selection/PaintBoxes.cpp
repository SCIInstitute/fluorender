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

#include <PaintBoxes.h>
#include <Global.h>
#include <Point.h>
#include <KernelProgram.h>
#include <Kernel.h>
#include <TextureBrick.h>

using namespace flrd;

constexpr const char* str_cl_paint_boxes = R"CLKER(
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image2d_t paint,
	__global float* boxes,
	__global unsigned int* hits,
	unsigned int nx,
	unsigned int ny,
	unsigned int nbb,
	float4 imat0,
	float4 imat1,
	float4 imat2,
	float4 imat3)
{
	unsigned int i = (unsigned int)(get_global_id(0));
	unsigned int j = (unsigned int)(get_global_id(1));
	float v = read_imagef(paint, samp, (int2)(i, j)).x;
	if (v < 0.45f)
		return;
	float x = (float)(i) * 2.0f / (float)(nx) - 1.0f;
	float y = (float)(j) * 2.0f / (float)(ny) - 1.0f;
	float4 mp1 = (float4)(x, y, 0.0f, 1.0f);
	float4 mp2 = (float4)(x, y, 1.0f, 1.0f);
	mp1 = (float4)(dot(mp1, imat0), dot(mp1, imat1), dot(mp1, imat2), dot(mp1, imat3));
	mp1 /= mp1.w;
	mp2 = (float4)(dot(mp2, imat0), dot(mp2, imat1), dot(mp2, imat2), dot(mp2, imat3));
	mp2 /= mp2.w;
	float3 p0 = (float3)(mp1.x, mp1.y, mp1.z);
	float3 dir = (float3)(mp1.x - mp2.x, mp1.y - mp2.y, mp1.z - mp2.z);
	for (int ibb = 0; ibb < nbb; ++ibb)
	{
		float3 bbmin = (float3)(boxes[ibb*6], boxes[ibb*6+1], boxes[ibb*6+2]);
		float3 bbmax = (float3)(boxes[ibb*6+3], boxes[ibb*6+4], boxes[ibb*6+5]);
		float3 t1 = (bbmin - p0) / dir;
		float3 t2 = (bbmax - p0) / dir;
		float3 tn = fmin(t1, t2);
		float3 tf = fmax(t1, t2);
		float tnn = max(tn.x, max(tn.y, tn.z));
		float tff = min(tf.x, min(tf.y, tf.z));
		if (tnn <= tff)
			atomic_inc(hits+ibb);
	}
}
)CLKER";

void PaintBoxes::Compute()
{
	if (m_mouse_pos)
	{
		BrickRayInt();
		return;
	}

	if (m_view_only)
	{
		BrickViewInt();
		return;
	}

	std::vector<BrickBox> bbs;
	if (!GetBrickBoxes(bbs))
		return;

	//allocate memory and initialize
	size_t num = bbs.size();
	float* boxes = new float[num * 6];
	unsigned int* hits = new unsigned int[num];
	for (size_t i = 0; i < num; ++i)
	{
		BrickBox bb = bbs[i];
		boxes[i * 6 + 0] = static_cast<float>(bb.bbox.Min().x());
		boxes[i * 6 + 1] = static_cast<float>(bb.bbox.Min().y());
		boxes[i * 6 + 2] = static_cast<float>(bb.bbox.Min().z());
		boxes[i * 6 + 3] = static_cast<float>(bb.bbox.Max().x());
		boxes[i * 6 + 4] = static_cast<float>(bb.bbox.Max().y());
		boxes[i * 6 + 5] = static_cast<float>(bb.bbox.Max().z());
		hits[i] = 0;
	}

	//create program and kernels
	flvr::KernelProgram* kernel_prog = glbin_kernel_factory.kernel(str_cl_paint_boxes, 8, 255.0f);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_0");

	size_t global_size[2] = { (size_t)m_ptx, (size_t)m_pty };
	size_t local_size[2] = { 1, 1 };

	//set
	cl_float4 imat0 = { float(m_imat.get_mat_val(0, 0)),
		float(m_imat.get_mat_val(1, 0)),
		float(m_imat.get_mat_val(2, 0)),
		float(m_imat.get_mat_val(3, 0)) };
	cl_float4 imat1 = { float(m_imat.get_mat_val(0, 1)),
		float(m_imat.get_mat_val(1, 1)),
		float(m_imat.get_mat_val(2, 1)),
		float(m_imat.get_mat_val(3, 1)) };
	cl_float4 imat2 = { float(m_imat.get_mat_val(0, 2)),
		float(m_imat.get_mat_val(1, 2)),
		float(m_imat.get_mat_val(2, 2)),
		float(m_imat.get_mat_val(3, 2)) };
	cl_float4 imat3 = { float(m_imat.get_mat_val(0, 3)),
		float(m_imat.get_mat_val(1, 3)),
		float(m_imat.get_mat_val(2, 3)),
		float(m_imat.get_mat_val(3, 3)) };
	kernel_prog->setKernelArgBegin(kernel_index);
	kernel_prog->setKernelArgTex2D(CL_MEM_READ_ONLY, m_paint_tex);
	auto arg_boxes =
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*num*6, boxes);
	auto arg_hits =
		kernel_prog->setKernelArgBuf(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int)*num, hits);
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&m_ptx));
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&m_pty));
	kernel_prog->setKernelArgConst(sizeof(unsigned int), (void*)(&num));
	kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&imat0));
	kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&imat1));
	kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&imat2));
	kernel_prog->setKernelArgConst(sizeof(cl_float4), (void*)(&imat3));

	//execute
	kernel_prog->executeKernel(kernel_index, 2, global_size, local_size);
	//read back
	kernel_prog->readBuffer(arg_hits, hits);

	//assign paint mask flag in bricks
	for (int i = 0; i < num; ++i)
	{
		if (hits[i])
		{
			bbs[i].brick->act_mask();
			bbs[i].brick->valid_mask();
		}
	}

	//release buffer
	kernel_prog->releaseAllArgs();
	//free memory
	delete[] boxes;
	delete[] hits;
}

bool PaintBoxes::GetBrickBoxes(std::vector<BrickBox> &bbs)
{
	if (!m_bricks)
		return false;

	for (int i = 0; i < m_bricks->size(); ++i)
	{
		flvr::TextureBrick* b = (*m_bricks)[i];
		fluo::BBox bbox = b->bbox();
		if (test_against_view(bbox))
		{
			BrickBox brick_box;
			brick_box.bbox = bbox;
			brick_box.brick = b;
			bbs.push_back(brick_box);
		}
	}
	return !bbs.empty();
}

void PaintBoxes::BrickViewInt()
{
	if (!m_bricks)
		return;

	for (int i = 0; i < m_bricks->size(); ++i)
	{
		flvr::TextureBrick* b = (*m_bricks)[i];
		fluo::BBox bbox = b->bbox();
		if (test_against_view(bbox))
		{
			b->act_mask();
			b->valid_mask();
		}
	}
}

void PaintBoxes::BrickRayInt()
{
	if (!m_bricks)
		return;

	//get ray
	fluo::Point mp1(
		(double)m_mx * 2 / m_ptx - 1,
		1 - (double)m_my * 2 / m_pty, 0);
	fluo::Point mp2 = mp1;
	mp2.z(1);
	mp1 = m_imat.transform(mp1);
	mp2 = m_imat.transform(mp2);
	fluo::Vector dir(mp1 - mp2);
	dir.normalize();

	//test ray-box
	fluo::Point hit;
	for (int i = 0; i < m_bricks->size(); ++i)
	{
		flvr::TextureBrick* b = (*m_bricks)[i];
		fluo::BBox bbox = b->bbox();
		if (bbox.intersect(mp1, dir, hit))
		{
			b->act_mask();
			b->valid_mask();
		}
	}
}

bool PaintBoxes::test_against_view(const fluo::BBox &bbox)
{
	if (m_persp)
	{
		const fluo::Point p0_cam(0.0, 0.0, 0.0);
		fluo::Point p0, p0_obj;
		m_pr.unproject(p0_cam, p0);
		m_mv.unproject(p0, p0_obj);
		if (bbox.inside(p0_obj))
			return true;
	}

	bool overx = true;
	bool overy = true;
	bool overz = true;
	bool underx = true;
	bool undery = true;
	bool underz = true;
	for (int i = 0; i < 8; i++)
	{
		const fluo::Point pold(
			(i & 1) ? bbox.Min().x() : bbox.Max().x(),
			(i & 2) ? bbox.Min().y() : bbox.Max().y(),
			(i & 4) ? bbox.Min().z() : bbox.Max().z());
		const fluo::Point p = m_pr.project(m_mv.project(pold));
		overx = overx && (p.x() > 1.0);
		overy = overy && (p.y() > 1.0);
		overz = overz && (p.z() > 1.0);
		underx = underx && (p.x() < -1.0);
		undery = undery && (p.y() < -1.0);
		underz = underz && (p.z() < -1.0);
	}

	return !(overx || overy || overz || underx || undery || underz);
}