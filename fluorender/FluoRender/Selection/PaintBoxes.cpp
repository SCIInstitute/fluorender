/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#include <Selection/PaintBoxes.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/Point.h>

using namespace FL;

const char* str_cl_paint_boxes = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image2d_t paint,\n" \
"	__global float* boxes,\n" \
"	__global unsigned int* hits,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nbb,\n" \
"	float4 imat0,\n" \
"	float4 imat1,\n" \
"	float4 imat2,\n" \
"	float4 imat3)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	float v = read_imagef(paint, samp, (int2)(i, j)).x;\n" \
"	if (v < 0.45)\n" \
"		return;\n" \
"	float x = (float)(i) * 2.0 / (float)(nx) - 1.0;\n" \
"	float y = (float)(j) * 2.0 / (float)(ny) - 1.0;\n" \
"	float4 mp1 = (float4)(x, y, 0.0, 1.0);\n" \
"	float4 mp2 = (float4)(x, y, 1.0, 1.0);\n" \
"	mp1 = (float4)(dot(mp1, imat0), dot(mp1, imat1), dot(mp1, imat2), dot(mp1, imat3));\n" \
"	mp1 /= mp1.w;\n" \
"	mp2 = (float4)(dot(mp2, imat0), dot(mp2, imat1), dot(mp2, imat2), dot(mp2, imat3));\n" \
"	mp2 /= mp2.w;\n" \
"	float3 p0 = (float3)(mp1.x, mp1.y, mp1.z);\n" \
"	float3 dir = (float3)(mp1.x - mp2.x, mp1.y - mp2.y, mp1.z - mp2.z);\n" \
"	for (int ibb = 0; ibb < nbb; ++ibb)\n" \
"	{\n" \
"		float3 bbmin = (float3)(boxes[ibb*6], boxes[ibb*6+1], boxes[ibb*6+2]);\n" \
"		float3 bbmax = (float3)(boxes[ibb*6+3], boxes[ibb*6+4], boxes[ibb*6+5]);\n" \
"		float3 t1 = (bbmin - p0) / dir;\n" \
"		float3 t2 = (bbmax - p0) / dir;\n" \
"		float3 tn = fmin(t1, t2);\n" \
"		float3 tf = fmax(t1, t2);\n" \
"		float tnn = max(tn.x, max(tn.y, tn.z));\n" \
"		float tff = min(tf.x, min(tf.y, tf.z));\n" \
"		if (tnn <= tff)\n" \
"			atomic_inc(hits+ibb);\n" \
"	}\n" \
"}\n" \
;

void PaintBoxes::Compute()
{
	vector<BrickBox> bbs;
	if (!GetBrickBoxes(bbs))
		return;

	//allocate memory and initialize
	int num = bbs.size();
	float* boxes = new float[num * 6];
	unsigned int* hits = new unsigned int[num];
	for (int i = 0; i < num; ++i)
	{
		BrickBox bb = bbs[i];
		boxes[i * 6 + 0] = bb.bbox.min().x();
		boxes[i * 6 + 1] = bb.bbox.min().y();
		boxes[i * 6 + 2] = bb.bbox.min().z();
		boxes[i * 6 + 3] = bb.bbox.max().x();
		boxes[i * 6 + 4] = bb.bbox.max().y();
		boxes[i * 6 + 5] = bb.bbox.max().z();
		hits[i] = 0;
	}

	//create program and kernels
	FLIVR::KernelProgram* kernel_prog = FLIVR::VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_paint_boxes);
	if (!kernel_prog)
		return;
	int kernel_index = kernel_prog->createKernel("kernel_0");

	size_t global_size[2] = { (size_t)m_ptx, (size_t)m_pty };
	size_t local_size[2] = { 1, 1 };

	//set
	kernel_prog->setKernelArgTex2D(kernel_index, 0,
		CL_MEM_READ_ONLY, m_paint_tex);
	FLIVR::Argument arg_boxes = kernel_prog->setKernelArgBuf(kernel_index, 1,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float)*num*6, boxes);
	FLIVR::Argument arg_hits = kernel_prog->setKernelArgBuf(kernel_index, 2,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(unsigned int)*num, hits);
	kernel_prog->setKernelArgConst(kernel_index, 3,
		sizeof(unsigned int), (void*)(&m_ptx));
	kernel_prog->setKernelArgConst(kernel_index, 4,
		sizeof(unsigned int), (void*)(&m_pty));
	kernel_prog->setKernelArgConst(kernel_index, 5,
		sizeof(unsigned int), (void*)(&num));
	cl_float4 imat0 = { float(m_imat.get_mat_val(0, 0)),
		float(m_imat.get_mat_val(1, 0)),
		float(m_imat.get_mat_val(2, 0)),
		float(m_imat.get_mat_val(3, 0)) };
	kernel_prog->setKernelArgConst(kernel_index, 6,
		sizeof(cl_float4), (void*)(&imat0));
	cl_float4 imat1 = { float(m_imat.get_mat_val(0, 1)),
		float(m_imat.get_mat_val(1, 1)),
		float(m_imat.get_mat_val(2, 1)),
		float(m_imat.get_mat_val(3, 1)) };
	kernel_prog->setKernelArgConst(kernel_index, 7,
		sizeof(cl_float4), (void*)(&imat1));
	cl_float4 imat2 = { float(m_imat.get_mat_val(0, 2)),
		float(m_imat.get_mat_val(1, 2)),
		float(m_imat.get_mat_val(2, 2)),
		float(m_imat.get_mat_val(3, 2)) };
	kernel_prog->setKernelArgConst(kernel_index, 8,
		sizeof(cl_float4), (void*)(&imat2));
	cl_float4 imat3 = { float(m_imat.get_mat_val(0, 3)),
		float(m_imat.get_mat_val(1, 3)),
		float(m_imat.get_mat_val(2, 3)),
		float(m_imat.get_mat_val(3, 3)) };
	kernel_prog->setKernelArgConst(kernel_index, 9,
		sizeof(cl_float4), (void*)(&imat3));

	//execute
	kernel_prog->executeKernel(kernel_index, 2, global_size, local_size);
	//read back
	kernel_prog->readBuffer(sizeof(unsigned int)*num, hits, hits);

	//assign paint mask flag in bricks
	for (int i = 0; i < num; ++i)
	{
		if (hits[i])
			bbs[i].brick->set_paint_mask(true);
	}

	//release buffer
	kernel_prog->releaseAll();
	//kernel_prog->releaseMemObject(arg_boxes);
	//kernel_prog->releaseMemObject(arg_hits);
	//free memory
	delete[] boxes;
	delete[] hits;
}

bool PaintBoxes::GetBrickBoxes(vector<BrickBox> &bbs)
{
	if (!m_bricks)
		return false;

	for (int i = 0; i < m_bricks->size(); ++i)
	{
		FLIVR::TextureBrick* b = (*m_bricks)[i];
		FLIVR::BBox bbox = b->bbox();
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

bool PaintBoxes::test_against_view(const FLIVR::BBox &bbox)
{
	if (m_persp)
	{
		const FLIVR::Point p0_cam(0.0, 0.0, 0.0);
		FLIVR::Point p0, p0_obj;
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
		const FLIVR::Point pold((i & 1) ? bbox.min().x() : bbox.max().x(),
			(i & 2) ? bbox.min().y() : bbox.max().y(),
			(i & 4) ? bbox.min().z() : bbox.max().z());
		const FLIVR::Point p = m_pr.project(m_mv.project(pold));
		overx = overx && (p.x() > 1.0);
		overy = overy && (p.y() > 1.0);
		overz = overz && (p.z() > 1.0);
		underx = underx && (p.x() < -1.0);
		undery = undery && (p.y() < -1.0);
		underz = underz && (p.z() < -1.0);
	}

	return !(overx || overy || overz || underx || undery || underz);
}