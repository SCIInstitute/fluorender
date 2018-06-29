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
#include "DataManager.h"
#include "CompGenerator.h"
#include "cl_code.h"
#include <algorithm>

using namespace FL;

ComponentGenerator::ComponentGenerator(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false),
	m_init(false)
{
}

ComponentGenerator::~ComponentGenerator()
{
}

void ComponentGenerator::OrderID_3D()
{
	if (!m_vd)
		return;
	m_vd->AddEmptyLabel(1);
	m_sig_progress();
}

#define GET_VOLDATA \
	if (!m_vd) \
		return; \
	int nx, ny, nz; \
	m_vd->GetResolution(nx, ny, nz); \
	Nrrd* nrrd_data = 0; \
	if (m_use_mask) \
		nrrd_data = m_vd->GetMask(true); \
	if (!nrrd_data) \
		nrrd_data = m_vd->GetVolume(false); \
	if (!nrrd_data) \
		return; \
	Nrrd* nrrd_label = m_vd->GetLabel(false); \
	if (!nrrd_data) \
		return; \
	unsigned char* val8 = 0; \
	unsigned short* val16 = 0; \
	int bits; \
	if (nrrd_data->type == nrrdTypeUChar) \
	{ \
		bits = 8; \
		val8 = (unsigned char*)(nrrd_data->data); \
	} \
	else if (nrrd_data->type == nrrdTypeUShort) \
	{ \
		bits = 16; \
		val16 = (unsigned short*)(nrrd_data->data); \
	} \
	unsigned int* val32 = (unsigned int*)(nrrd_label->data);

#define CHECK_BRICKS \
	if (!m_vd || !m_vd->GetTexture()) \
		return; \
	vector<TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks(); \
	if (!bricks || bricks->size() == 0) \
		return;

#define GET_VOLDATA_STREAM \
	int nx, ny, nz; \
	unsigned char* val8 = 0; \
	unsigned short* val16 = 0; \
	int bits = 8; \
	unsigned int* val32 = 0; \
	TextureBrick* b = 0; \
	int c = 0; \
	int nb = 1; \
	if (bricks->size() > 1) \
	{ \
		b = (*bricks)[i]; \
		c = m_use_mask ? b->nmask() : 0; \
		nb = b->nb(c); \
		nx = b->nx(); \
		ny = b->ny(); \
		nz = b->nz(); \
		bits = nb * 8; \
		unsigned long long mem_size = (unsigned long long)nx* \
		(unsigned long long)ny*(unsigned long long)nz*(unsigned long long)nb; \
		unsigned char* temp = new unsigned char[mem_size]; \
		unsigned char* tempp = temp; \
		unsigned char* tp = (unsigned char*)(b->tex_data(c)); \
		unsigned char* tp2; \
		for (unsigned int k = 0; k < nz; ++k) \
		{ \
			tp2 = tp; \
			for (unsigned int j = 0; j < ny; ++j) \
			{ \
				memcpy(tempp, tp2, nx*nb); \
				tempp += nx*nb; \
				tp2 += b->sx()*nb; \
			} \
			tp += b->sx()*b->sy()*nb; \
		} \
		if (bits == 8) \
		val8 = temp; \
		else if (bits == 16) \
		val16 = (unsigned short*)temp; \
		c = b->nlabel(); \
		nb = b->nb(c); \
		mem_size = (unsigned long long)nx* \
		(unsigned long long)ny*(unsigned long long)nz*(unsigned long long)nb; \
		temp = new unsigned char[mem_size]; \
		tempp = temp; \
		tp = (unsigned char*)(b->tex_data(c)); \
		for (unsigned int k = 0; k < nz; ++k) \
		{ \
			tp2 = tp; \
			for (unsigned int j = 0; j < ny; ++j) \
			{ \
				memcpy(tempp, tp2, nx*nb); \
				tempp += nx*nb; \
				tp2 += b->sx()*nb; \
			} \
			tp += b->sx()*b->sy()*nb; \
		} \
		val32 = (unsigned int*)temp; \
	} \
	else \
	{ \
		m_vd->GetResolution(nx, ny, nz); \
		Nrrd* nrrd_data = 0; \
		if (m_use_mask) \
			nrrd_data = m_vd->GetMask(false); \
		if (!nrrd_data) \
			nrrd_data = m_vd->GetVolume(false); \
		if (!nrrd_data) \
			return; \
		Nrrd* nrrd_label = m_vd->GetLabel(false); \
		if (!nrrd_data) \
			return; \
		if (nrrd_data->type == nrrdTypeUChar) \
		{ \
			bits = 8; \
			val8 = (unsigned char*)(nrrd_data->data); \
		} \
		else if (nrrd_data->type == nrrdTypeUShort) \
		{ \
			bits = 16; \
			val16 = (unsigned short*)(nrrd_data->data); \
		} \
		val32 = (unsigned int*)(nrrd_label->data); \
	}

#define RELEASE_DATA_STREAM \
	if (bricks->size() > 1) \
	{ \
		unsigned char* tempp = (unsigned char*)val32; \
		unsigned char* tp = (unsigned char*)(b->tex_data(c)); \
		unsigned char* tp2; \
		for (unsigned int k = 0; k < nz; ++k) \
		{ \
			tp2 = tp; \
			for (unsigned int j = 0; j < ny; ++j) \
			{ \
				memcpy(tp2, tempp, nx*nb); \
				tempp += nx*nb; \
				tp2 += b->sx()*nb; \
			} \
			tp += b->sx()*b->sy()*nb; \
		} \
		if (val8) delete[] val8; \
		if (val16) delete[] val16; \
		if (val32) delete[] val32; \
	}

void ComponentGenerator::OrderID_2D()
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_order_id_2d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = nz;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, val32);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, 0);
		kernel_prog->releaseMemObject(kernel_index, 1, sizeof(unsigned int)*nx*ny*nz, 0);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::ShuffleID_3D()
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_shuffle_id_3d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		//bit length
		unsigned int lenx = 0;
		unsigned int r = Max(nx, ny);
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

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = size_t(nx);
		image_desc.image_height = size_t(ny);
		image_desc.image_depth = size_t(nz);
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, val32);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index, 6,
			sizeof(unsigned int), (void*)(&lenz));
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, 0);
		kernel_prog->releaseMemObject(kernel_index, 1, sizeof(unsigned int)*nx*ny*nz, 0);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::ShuffleID_2D()
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_shuffle_id_2d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		unsigned int len = 0;
		unsigned int r = Max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			len++;
		}

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = nz;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, val32);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(unsigned int), (void*)(&len));
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, 0);
		kernel_prog->releaseMemObject(kernel_index, 1, sizeof(unsigned int)*nx*ny*nz, 0);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::ClearBorders3D()
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_clear_borders_3d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		kernel_prog->setKernelArgBuf(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, val32);
		kernel_prog->setKernelArgConst(kernel_index, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&nz));
		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, sizeof(unsigned int)*nx*ny*nz, 0);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::ClearBorders2D()
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_clear_borders_2d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned int* cur_page_label = val32;
		size_t global_size[2] = { size_t(nx), size_t(ny) };
		size_t local_size[2] = { 1, 1 };

		cl_mem label_buffer = kernel_prog->setKernelArgBuf(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog->setKernelArgConst(kernel_index, 1,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&ny));

		for (size_t i = 0; i < nz; ++i)
		{
			//update
			if (i)
				kernel_prog->writeBuffer(label_buffer, cur_page_label);
			//execute
			kernel_prog->executeKernel(kernel_index, 2, global_size, local_size);
			//read back
			kernel_prog->readBuffer(label_buffer, cur_page_label);
			//advance
			cur_page_label += nx*ny;
		}

		//release buffer
		kernel_prog->releaseMemObject(label_buffer);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::FillBorder3D(float tol)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_fill_borders_3d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = nz;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, val32);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(float), (void*)(&tol));

		//execute
		kernel_prog->executeKernel(kernel_index, 3, global_size, local_size);
		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(kernel_index, 0, 0, 0);
		kernel_prog->releaseMemObject(kernel_index, 1, sizeof(unsigned int)*nx*ny*nz, 0);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::FillBorder2D(float tol)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_fill_borders_2d);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned char* cur_page_data8 = bits == 8 ? val8 : 0;
		unsigned short* cur_page_data16 = bits == 16 ? val16 : 0;
		unsigned int* cur_page_label = val32;
		size_t global_size[2] = { size_t(nx), size_t(ny) };
		size_t local_size[2] = { 1, 1 };

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = 0;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		cl_mem data_buffer = kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16));
		cl_mem label_buffer = kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index, 4,
			sizeof(float), (void*)(&tol));

		const size_t origin[] = { 0, 0, 0 };
		const size_t region[] = { size_t(nx), size_t(ny), 1 };
		void *img_data = bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16);
		for (size_t i = 0; i < nz; ++i)
		{
			if (i)
			{
				kernel_prog->writeImage(origin, region, data_buffer, img_data);
				kernel_prog->writeBuffer(label_buffer, cur_page_label);
			}

			//execute
			kernel_prog->executeKernel(kernel_index, 2, global_size, local_size);
			//read back
			kernel_prog->readBuffer(label_buffer, cur_page_label);

			if (bits == 8)
				cur_page_data8 += nx*ny;
			else if (bits == 16)
				cur_page_data16 += nx*ny;
			cur_page_label += nx*ny;
		}

		//release buffer
		kernel_prog->releaseMemObject(data_buffer);
		kernel_prog->releaseMemObject(label_buffer);

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::Grow3D(bool diffuse, int iter, float tran, float falloff,
	float density, int clean_iter, int clean_size)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_brainbow_3d);
	if (!kernel_prog)
		return;
	int kernel_index0 = -1;
	int kernel_index1 = -1;
	int kernel_index2 = -1;
	int kernel_index3 = -1;
	string name0 = "kernel_0";
	string name1 = "kernel_1";
	string name2 = "kernel_2";
	string name3 = "kernel_3";
	if (kernel_prog->valid())
	{
		kernel_index0 = kernel_prog->findKernel(name0);
		if (clean_iter)
		{
			kernel_index1 = kernel_prog->findKernel(name1);
			kernel_index2 = kernel_prog->findKernel(name2);
			kernel_index3 = kernel_prog->findKernel(name3);
		}
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel(name0);
		if (clean_iter)
		{
			kernel_index1 = kernel_prog->createKernel(name1);
			kernel_index2 = kernel_prog->createKernel(name2);
			kernel_index3 = kernel_prog->createKernel(name3);
		}
	}

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		float scl_ff = diffuse ? falloff : 0.0f;
		float grad_ff = diffuse ? falloff : 0.0f;

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = nz;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		kernel_prog->setKernelArgImage(kernel_index0, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->setKernelArgBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(val32));
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(kernel_index0, 5,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(kernel_index0, 6,
			sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(kernel_index0, 7,
			sizeof(float), (void*)(&tran));
		kernel_prog->setKernelArgConst(kernel_index0, 8,
			sizeof(float), (void*)(&scl_ff));
		kernel_prog->setKernelArgConst(kernel_index0, 9,
			sizeof(float), (void*)(&grad_ff));
		kernel_prog->setKernelArgConst(kernel_index0, 10,
			sizeof(float), (void*)(&density));

		unsigned int* mask32 = 0;

		if (clean_iter)
		{
			//bit length
			unsigned int lenx = 0;
			unsigned int r = Max(nx, ny);
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
			mask32 = new unsigned int[data_size];
			memset(mask32, 0, label_size);

			//set
			//kernel 1
			kernel_prog->setKernelArgBuf(kernel_index1, 0,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				label_size, mask32);
			kernel_prog->setKernelArgBuf(kernel_index1, 1,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(unsigned int)*nx*ny*nz, val32);
			kernel_prog->setKernelArgConst(kernel_index1, 2,
				sizeof(unsigned int), (void*)(&nx));
			kernel_prog->setKernelArgConst(kernel_index1, 3,
				sizeof(unsigned int), (void*)(&ny));
			kernel_prog->setKernelArgConst(kernel_index1, 4,
				sizeof(unsigned int), (void*)(&nz));
			kernel_prog->setKernelArgConst(kernel_index1, 5,
				sizeof(unsigned int), (void*)(&lenx));
			kernel_prog->setKernelArgConst(kernel_index1, 6,
				sizeof(unsigned int), (void*)(&lenz));
			//kernel 2
			kernel_prog->setKernelArgBuf(kernel_index2, 0,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				label_size, mask32);
			kernel_prog->setKernelArgBuf(kernel_index2, 1,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(unsigned int)*nx*ny*nz, val32);
			kernel_prog->setKernelArgConst(kernel_index2, 2,
				sizeof(unsigned int), (void*)(&nx));
			kernel_prog->setKernelArgConst(kernel_index2, 3,
				sizeof(unsigned int), (void*)(&ny));
			kernel_prog->setKernelArgConst(kernel_index2, 4,
				sizeof(unsigned int), (void*)(&nz));
			kernel_prog->setKernelArgConst(kernel_index2, 5,
				sizeof(unsigned int), (void*)(&lenx));
			kernel_prog->setKernelArgConst(kernel_index2, 6,
				sizeof(unsigned int), (void*)(&lenz));
			//kernel 3
			kernel_prog->setKernelArgImage(kernel_index3, 0,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				image_format, image_desc,
				bits == 8 ? (void*)(val8) : (void*)(val16));
			kernel_prog->setKernelArgBuf(kernel_index3, 1,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				label_size, mask32);
			kernel_prog->setKernelArgBuf(kernel_index3, 2,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(unsigned int)*nx*ny*nz, val32);
			kernel_prog->setKernelArgConst(kernel_index3, 3,
				sizeof(unsigned int), (void*)(&nx));
			kernel_prog->setKernelArgConst(kernel_index3, 4,
				sizeof(unsigned int), (void*)(&ny));
			kernel_prog->setKernelArgConst(kernel_index3, 5,
				sizeof(unsigned int), (void*)(&nz));
			kernel_prog->setKernelArgConst(kernel_index3, 6,
				sizeof(unsigned int), (void*)(&clean_size));
		}

		//execute
		for (int j = 0; j < iter; ++j)
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
		if (clean_iter)
		{
			for (int j = 0; j < clean_iter; ++j)
			{
				kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);
			}
		}

		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(0,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->releaseMemObject(sizeof(unsigned int)*nx*ny*nz,
			(void*)(val32));
		kernel_prog->releaseMemObject(sizeof(unsigned int),
			(void*)(&rcnt));
		if (clean_iter)
		{
			kernel_prog->releaseMemObject(sizeof(unsigned int)*nx*ny*nz,
				(void*)(mask32));
			delete[] mask32;
		}

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::Grow3DSized(
	bool diffuse, int iter, float tran, float falloff,
	int size_lm, float density, int clean_iter, int clean_size)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_brainbow_3d_sized);
	if (!kernel_prog)
		return;
	int kernel_index0 = -1;
	int kernel_index1 = -1;
	int kernel_index2 = -1;
	int kernel_index3 = -1;
	string name0 = "kernel_0";
	string name1 = "kernel_1";
	string name2 = "kernel_2";
	string name3 = "kernel_3";
	if (kernel_prog->valid())
	{
		kernel_index0 = kernel_prog->findKernel(name0);
		kernel_index1 = kernel_prog->findKernel(name1);
		kernel_index2 = kernel_prog->findKernel(name2);
		if (clean_iter)
			kernel_index3 = kernel_prog->findKernel(name3);
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel(name0);
		kernel_index1 = kernel_prog->createKernel(name1);
		kernel_index2 = kernel_prog->createKernel(name2);
		if (clean_iter)
			kernel_index3 = kernel_prog->createKernel(name3);
	}

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned int* mask32 = new unsigned int[nx*ny*nz];
		memset(mask32, 0, sizeof(unsigned int)*nx*ny*nz);

		unsigned int rcnt = 0;
		unsigned int seed = iter > 10 ? iter : 11;
		size_t global_size[3] = { size_t(nx), size_t(ny), size_t(nz) };
		size_t local_size[3] = { 1, 1, 1 };
		float scl_ff = diffuse ? falloff : 0.0f;
		float grad_ff = diffuse ? falloff : 0.0f;
		//bit length
		unsigned int lenx = 0;
		unsigned int r = Max(nx, ny);
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

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE3D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = nz;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;

		//set
		//kernel 0
		kernel_prog->setKernelArgBuf(kernel_index0, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(mask32));
		kernel_prog->setKernelArgBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(val32));
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index0, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index0, 6,
			sizeof(unsigned int), (void*)(&lenz));
		//kernel 1
		kernel_prog->setKernelArgBuf(kernel_index1, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(mask32));
		kernel_prog->setKernelArgBuf(kernel_index1, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(val32));
		kernel_prog->setKernelArgConst(kernel_index1, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index1, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index1, 4,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgConst(kernel_index1, 5,
			sizeof(unsigned int), (void*)(&lenx));
		kernel_prog->setKernelArgConst(kernel_index1, 6,
			sizeof(unsigned int), (void*)(&lenz));
		//kernel 2
		kernel_prog->setKernelArgImage(kernel_index2, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->setKernelArgBuf(kernel_index2, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(mask32));
		kernel_prog->setKernelArgBuf(kernel_index2, 2,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny*nz, (void*)(val32));
		kernel_prog->setKernelArgConst(kernel_index2, 3,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index2, 4,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index2, 5,
			sizeof(unsigned int), (void*)(&nz));
		kernel_prog->setKernelArgBuf(kernel_index2, 6,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(kernel_index2, 7,
			sizeof(unsigned int), (void*)(&seed));
		kernel_prog->setKernelArgConst(kernel_index2, 8,
			sizeof(float), (void*)(&tran));
		kernel_prog->setKernelArgConst(kernel_index2, 9,
			sizeof(float), (void*)(&scl_ff));
		kernel_prog->setKernelArgConst(kernel_index2, 10,
			sizeof(float), (void*)(&grad_ff));
		kernel_prog->setKernelArgConst(kernel_index2, 11,
			sizeof(unsigned int), (void*)(&size_lm));
		kernel_prog->setKernelArgConst(kernel_index2, 12,
			sizeof(float), (void*)(&density));

		//execute
		for (int j = 0; j < iter; ++j)
		{
			kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
			kernel_prog->executeKernel(kernel_index2, 3, global_size, local_size);
		}

		//kernel 3
		if (clean_iter)
		{
			kernel_prog->setKernelArgImage(kernel_index3, 0,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				image_format, image_desc,
				bits == 8 ? (void*)(val8) : (void*)(val16));
			kernel_prog->setKernelArgBuf(kernel_index3, 1,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(unsigned int)*nx*ny*nz, mask32);
			kernel_prog->setKernelArgBuf(kernel_index3, 2,
				CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
				sizeof(unsigned int)*nx*ny*nz, val32);
			kernel_prog->setKernelArgConst(kernel_index3, 3,
				sizeof(unsigned int), (void*)(&nx));
			kernel_prog->setKernelArgConst(kernel_index3, 4,
				sizeof(unsigned int), (void*)(&ny));
			kernel_prog->setKernelArgConst(kernel_index3, 5,
				sizeof(unsigned int), (void*)(&nz));
			kernel_prog->setKernelArgConst(kernel_index3, 6,
				sizeof(unsigned int), (void*)(&clean_size));

			//execute
			for (int j = 0; j < clean_iter; ++j)
			{
				kernel_prog->executeKernel(kernel_index0, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index1, 3, global_size, local_size);
				kernel_prog->executeKernel(kernel_index3, 3, global_size, local_size);
			}
		}

		//read back
		kernel_prog->readBuffer(sizeof(unsigned int)*nx*ny*nz, val32, val32);

		//release buffer
		kernel_prog->releaseMemObject(0,
			bits == 8 ? (void*)(val8) : (void*)(val16));
		kernel_prog->releaseMemObject(sizeof(unsigned int)*nx*ny*nz,
			(void*)(val32));
		kernel_prog->releaseMemObject(sizeof(unsigned int)*nx*ny*nz,
			(void*)(mask32));
		kernel_prog->releaseMemObject(sizeof(unsigned int),
			(void*)(&rcnt));
		delete[] mask32;

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::InitialGrow(bool param_tr, int iter,
	float tran, float tran2, float scl_ff, float scl_ff2,
	float grad_ff, float grad_ff2, float var_ff, float var_ff2,
	float ang_ff, float ang_ff2)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_slice_brainbow);
	if (!kernel_prog)
		return;
	int kernel_index = -1;
	string name = "kernel_0";
	if (kernel_prog->valid())
		kernel_index = kernel_prog->findKernel(name);
	else
		kernel_index = kernel_prog->createKernel(name);

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned char* cur_page_data8 = bits == 8 ? val8 : 0;
		unsigned short* cur_page_data16 = bits == 16 ? val16 : 0;
		unsigned int* cur_page_label = val32;
		unsigned int rcnt = 0;
		unsigned int seed = iter>10 ? iter : 11;
		size_t global_size[2] = { size_t(nx), size_t(ny) };
		size_t local_size[2] = { 1, 1 };
		//trnsition params
		float cur_tran, cur_scl_ff, cur_grad_ff, cur_var_ff, cur_ang_ff;

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = 0;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		cl_mem data_buffer = kernel_prog->setKernelArgImage(kernel_index, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16));
		cl_mem label_buffer = kernel_prog->setKernelArgBuf(kernel_index, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog->setKernelArgConst(kernel_index, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgBuf(kernel_index, 4,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(kernel_index, 5,
			sizeof(unsigned int), (void*)(&seed));
		if (!param_tr)
		{
			kernel_prog->setKernelArgConst(kernel_index, 6,
				sizeof(float), (void*)(&tran));
			kernel_prog->setKernelArgConst(kernel_index, 7,
				sizeof(float), (void*)(&scl_ff));
			kernel_prog->setKernelArgConst(kernel_index, 8,
				sizeof(float), (void*)(&grad_ff));
			kernel_prog->setKernelArgConst(kernel_index, 9,
				sizeof(float), (void*)(&var_ff));
			kernel_prog->setKernelArgConst(kernel_index, 10,
				sizeof(float), (void*)(&ang_ff));
		}

		const size_t origin[] = { 0, 0, 0 };
		const size_t region[] = { size_t(nx), size_t(ny), 1 };
		size_t count = 0;
		void *img_data = bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16);
		for (size_t i = 0; i<nz; ++i)
		{
			if (i)
			{
				kernel_prog->writeImage(origin, region, data_buffer, img_data);
				kernel_prog->writeBuffer(label_buffer, cur_page_label);
			}

			for (int j = 0; j < iter; ++j)
			{
				if (param_tr)
				{
					cur_tran = tran + (float)j*(tran2 - tran) / (float)(iter - 1);
					cur_scl_ff = scl_ff + (float)j*(scl_ff2 - scl_ff) / (float)(iter - 1);
					cur_grad_ff = grad_ff + (float)j*(grad_ff2 - grad_ff) / (float)(iter - 1);
					cur_var_ff = var_ff + (float)j*(var_ff2 - var_ff) / (float)(iter - 1);
					cur_ang_ff = ang_ff + (float)j*(ang_ff2 - ang_ff) / (float)(iter - 1);
					kernel_prog->setKernelArgConst(kernel_index, 6,
						sizeof(float), (void*)(&cur_tran));
					kernel_prog->setKernelArgConst(kernel_index, 7,
						sizeof(float), (void*)(&cur_scl_ff));
					kernel_prog->setKernelArgConst(kernel_index, 8,
						sizeof(float), (void*)(&cur_grad_ff));
					kernel_prog->setKernelArgConst(kernel_index, 9,
						sizeof(float), (void*)(&cur_var_ff));
					kernel_prog->setKernelArgConst(kernel_index, 10,
						sizeof(float), (void*)(&cur_ang_ff));
				}

				kernel_prog->executeKernel(kernel_index, 2, global_size, local_size);

				count++;
				if (count == nz - 1)
				{
					count = 0;
				}
			}

			kernel_prog->readBuffer(label_buffer, cur_page_label);

			if (bits == 8)
				cur_page_data8 += nx*ny;
			else if (bits == 16)
				cur_page_data16 += nx*ny;
			cur_page_label += nx*ny;
		}

		kernel_prog->releaseMemObject(data_buffer);
		kernel_prog->releaseMemObject(label_buffer);
		kernel_prog->releaseMemObject(sizeof(unsigned int),
			(void*)(&rcnt));

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::SizedGrow(bool param_tr, int iter,
	unsigned int size_lm, unsigned int size_lm2, float tran, float tran2,
	float scl_ff, float scl_ff2, float grad_ff, float grad_ff2,
	float var_ff, float var_ff2, float ang_ff, float ang_ff2)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_grow_size);
	if (!kernel_prog)
		return;
	int kernel_index0 = -1;
	int kernel_index1 = -1;
	int kernel_index2 = -1;
	string name0 = "kernel_0";
	string name1 = "kernel_1";
	string name2 = "kernel_2";
	if (kernel_prog->valid())
	{
		kernel_index0 = kernel_prog->findKernel(name0);
		kernel_index1 = kernel_prog->findKernel(name1);
		kernel_index2 = kernel_prog->findKernel(name2);
	}
	else
	{
		kernel_index0 = kernel_prog->createKernel(name0);
		kernel_index1 = kernel_prog->createKernel(name1);
		kernel_index2 = kernel_prog->createKernel(name2);
	}

	if (m_use_mask)
		m_vd->GetVR()->return_mask();

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned int* mask32 = new unsigned int[nx*ny];
		memset(mask32, 0, sizeof(unsigned int)*nx*ny);

		unsigned char* cur_page_data8 = bits == 8 ? val8 : 0;
		unsigned short* cur_page_data16 = bits == 16 ? val16 : 0;
		unsigned int* cur_page_label = val32;
		unsigned int rcnt = 0;
		unsigned int seed = iter>10 ? iter : 11;
		size_t global_size[2] = { size_t(nx), size_t(ny) };
		size_t local_size[2] = { 1, 1 };
		unsigned int len = 0;
		unsigned int r = Max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			len++;
		}

		//trnsition params
		unsigned int cur_size_lm;
		float cur_tran, cur_scl_ff, cur_grad_ff, cur_var_ff, cur_ang_ff;

		//data
		cl_image_format image_format;
		image_format.image_channel_order = CL_R;
		if (bits == 8)
			image_format.image_channel_data_type = CL_UNORM_INT8;
		else if (bits == 16)
			image_format.image_channel_data_type = CL_UNORM_INT16;
		cl_image_desc image_desc;
		image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
		image_desc.image_width = nx;
		image_desc.image_height = ny;
		image_desc.image_depth = 0;
		image_desc.image_array_size = 0;
		image_desc.image_row_pitch = 0;
		image_desc.image_slice_pitch = 0;
		image_desc.num_mip_levels = 0;
		image_desc.num_samples = 0;
		image_desc.buffer = 0;
		//set
		//kernel 0
		cl_mem mask_buffer = kernel_prog->setKernelArgBuf(kernel_index0, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, mask32);
		cl_mem label_buffer = kernel_prog->setKernelArgBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&len));
		//kernel 1
		mask_buffer = kernel_prog->setKernelArgBuf(kernel_index1, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, mask32);
		label_buffer = kernel_prog->setKernelArgBuf(kernel_index1, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog->setKernelArgConst(kernel_index1, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index1, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgConst(kernel_index1, 4,
			sizeof(unsigned int), (void*)(&len));
		//kernel 2
		cl_mem data_buffer = kernel_prog->setKernelArgImage(kernel_index2, 0,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			image_format, image_desc,
			bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16));
		mask_buffer = kernel_prog->setKernelArgBuf(kernel_index2, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, mask32);
		label_buffer = kernel_prog->setKernelArgBuf(kernel_index2, 2,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog->setKernelArgConst(kernel_index2, 3,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog->setKernelArgConst(kernel_index2, 4,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog->setKernelArgBuf(kernel_index2, 5,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int), (void*)(&rcnt));
		kernel_prog->setKernelArgConst(kernel_index2, 6,
			sizeof(unsigned int), (void*)(&seed));
		if (!param_tr)
		{
			kernel_prog->setKernelArgConst(kernel_index2, 7,
				sizeof(float), (void*)(&tran));
			kernel_prog->setKernelArgConst(kernel_index2, 8,
				sizeof(float), (void*)(&scl_ff));
			kernel_prog->setKernelArgConst(kernel_index2, 9,
				sizeof(float), (void*)(&grad_ff));
			kernel_prog->setKernelArgConst(kernel_index2, 10,
				sizeof(float), (void*)(&var_ff));
			kernel_prog->setKernelArgConst(kernel_index2, 11,
				sizeof(float), (void*)(&ang_ff));
			kernel_prog->setKernelArgConst(kernel_index2, 12,
				sizeof(float), (void*)(&size_lm));
		}

		const size_t origin[] = { 0, 0, 0 };
		const size_t region[] = { size_t(nx), size_t(ny), 1 };
		size_t count = 0;
		void *img_data = bits == 8 ? (void*)(cur_page_data8) : (void*)(cur_page_data16);
		for (size_t i = 0; i<nz; ++i)
		{
			if (i)
			{
				kernel_prog->writeImage(origin, region, data_buffer, img_data);
				kernel_prog->writeBuffer(label_buffer, cur_page_label);
			}

			for (int j = 0; j<iter; ++j)
			{
				kernel_prog->writeBuffer(mask_buffer, mask32);
				kernel_prog->executeKernel(kernel_index0, 2, global_size, local_size);
				kernel_prog->executeKernel(kernel_index1, 2, global_size, local_size);

				if (param_tr)
				{
					cur_size_lm = size_lm + (unsigned int)((float)j*(size_lm2 - size_lm) / (float)(iter - 1) + 0.5f);
					cur_tran = tran + (float)j*(tran2 - tran) / (float)(iter - 1);
					cur_scl_ff = scl_ff + (float)j*(scl_ff2 - scl_ff) / (float)(iter - 1);
					cur_grad_ff = grad_ff + (float)j*(grad_ff2 - grad_ff) / (float)(iter - 1);
					cur_var_ff = var_ff + (float)j*(var_ff2 - var_ff) / (float)(iter - 1);
					cur_ang_ff = ang_ff + (float)j*(ang_ff2 - ang_ff) / (float)(iter - 1);
					kernel_prog->setKernelArgConst(kernel_index2, 7,
						sizeof(float), (void*)(&cur_tran));
					kernel_prog->setKernelArgConst(kernel_index2, 8,
						sizeof(float), (void*)(&cur_scl_ff));
					kernel_prog->setKernelArgConst(kernel_index2, 9,
						sizeof(float), (void*)(&cur_grad_ff));
					kernel_prog->setKernelArgConst(kernel_index2, 10,
						sizeof(float), (void*)(&cur_var_ff));
					kernel_prog->setKernelArgConst(kernel_index2, 11,
						sizeof(float), (void*)(&cur_ang_ff));
					kernel_prog->setKernelArgConst(kernel_index2, 12,
						sizeof(float), (void*)(&cur_size_lm));
				}
				kernel_prog->executeKernel(kernel_index2, 2, global_size, local_size);

				count++;
				if (count == nz - 1)
				{
					count = 0;
				}
			}

			kernel_prog->readBuffer(label_buffer, cur_page_label);

			if (bits == 8)
				cur_page_data8 += nx*ny;
			else if (bits == 16)
				cur_page_data16 += nx*ny;
			cur_page_label += nx*ny;
		}

		kernel_prog->releaseMemObject(data_buffer);
		kernel_prog->releaseMemObject(label_buffer);
		kernel_prog->releaseMemObject(mask_buffer);
		kernel_prog->releaseMemObject(sizeof(unsigned int),
			(void*)(&rcnt));
		delete[] mask32;

		m_sig_progress();

		RELEASE_DATA_STREAM
	}
}

void ComponentGenerator::Cleanup(int iter, unsigned int size_lm)
{
	CHECK_BRICKS

	//create program and kernels
	KernelProgram* kernel_prog1 = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_grow_size);
	if (!kernel_prog1)
		return;
	int kernel_index0 = -1;
	int kernel_index1 = -1;
	string name0 = "kernel_0";
	string name1 = "kernel_1";
	if (kernel_prog1->valid())
	{
		kernel_index0 = kernel_prog1->findKernel(name0);
		kernel_index1 = kernel_prog1->findKernel(name1);
	}
	else
	{
		kernel_index0 = kernel_prog1->createKernel(name0);
		kernel_index1 = kernel_prog1->createKernel(name1);
	}
	KernelProgram* kernel_prog2 = VolumeRenderer::
		vol_kernel_factory_.kernel(str_cl_clean_up);
	if (!kernel_prog2)
		return;
	int kernel_index2 = -1;
	string name2 = "kernel_0";
	if (kernel_prog2->valid())
		kernel_index2 = kernel_prog2->findKernel(name2);
	else
		kernel_index2 = kernel_prog2->createKernel(name2);
	kernel_index2 = kernel_prog1->addKernel(
		kernel_prog2, kernel_index2);

	for (size_t i = 0; i < bricks->size(); ++i)
	{
		GET_VOLDATA_STREAM

		unsigned int* mask32 = new unsigned int[nx*ny];
		memset(mask32, 0, sizeof(unsigned int)*nx*ny);

		unsigned int* cur_page_label = val32;
		size_t global_size[2] = { size_t(nx), size_t(ny) };
		size_t local_size[2] = { 1, 1 };
		unsigned int len = 0;
		unsigned int r = Max(nx, ny);
		while (r > 0)
		{
			r /= 2;
			len++;
		}

		//set
		//kernel 0
		cl_mem mask_buffer = kernel_prog1->setKernelArgBuf(kernel_index0, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, mask32);
		cl_mem label_buffer = kernel_prog1->setKernelArgBuf(kernel_index0, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog1->setKernelArgConst(kernel_index0, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog1->setKernelArgConst(kernel_index0, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog1->setKernelArgConst(kernel_index0, 4,
			sizeof(unsigned int), (void*)(&len));
		//kernel 1
		mask_buffer = kernel_prog1->setKernelArgBuf(kernel_index1, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, mask32);
		label_buffer = kernel_prog1->setKernelArgBuf(kernel_index1, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog1->setKernelArgConst(kernel_index1, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog1->setKernelArgConst(kernel_index1, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog1->setKernelArgConst(kernel_index1, 4,
			sizeof(unsigned int), (void*)(&len));
		//kernel 2
		mask_buffer = kernel_prog1->setKernelArgBuf(kernel_index2, 0,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, mask32);
		label_buffer = kernel_prog1->setKernelArgBuf(kernel_index2, 1,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(unsigned int)*nx*ny, cur_page_label);
		kernel_prog1->setKernelArgConst(kernel_index2, 2,
			sizeof(unsigned int), (void*)(&nx));
		kernel_prog1->setKernelArgConst(kernel_index2, 3,
			sizeof(unsigned int), (void*)(&ny));
		kernel_prog1->setKernelArgConst(kernel_index2, 4,
			sizeof(unsigned int), (void*)(&size_lm));

		size_t count = 0;
		for (size_t i = 0; i < nz; ++i)
		{
			if (i)
				kernel_prog1->writeBuffer(label_buffer, cur_page_label);

			for (int j = 0; j < iter; ++j)
			{
				kernel_prog1->writeBuffer(mask_buffer, mask32);
				kernel_prog1->executeKernel(kernel_index0, 2, global_size, local_size);
				kernel_prog1->executeKernel(kernel_index1, 2, global_size, local_size);
				kernel_prog1->executeKernel(kernel_index2, 2, global_size, local_size);

				count++;
				if (count == nz - 1)
				{
					count = 0;
				}
			}

			kernel_prog1->readBuffer(label_buffer, cur_page_label);

			cur_page_label += nx*ny;
		}

		kernel_prog1->releaseMemObject(label_buffer);
		kernel_prog1->releaseMemObject(mask_buffer);
		delete[]mask32;

		m_sig_progress();

		RELEASE_DATA_STREAM
	}

	kernel_prog1->removeExternalKernels();
}

void ComponentGenerator::MatchSlices_CPU(bool backwards, unsigned int size_thresh,
	float size_ratio, float dist_thresh, float angle_thresh)
{
	GET_VOLDATA

	if (nz < 2) return;
	float sscale = m_vd->GetScalarScale();

	CellList cell_list1, cell_list2;
	unsigned int* page1 = val32;
	if (backwards) page1 = val32 + nx*ny*(nz - 1);
	unsigned int* page2 = val32 + nx*ny;
	if (backwards) page2 = val32 + nx*ny*(nz - 2);
	void* page1_data = bits==8?((void*)val8):((void*)val16);
	if (backwards) page1_data = bits == 8 ? ((void*)(val8 + nx*ny*(nz - 1))) : ((void*)(val16 + nx*ny*(nz - 1)));
	void* page2_data = bits==8?((void*)(val8 + nx*ny)):((void*)(val16 + nx*ny));
	if (backwards) page2_data = bits == 8 ? ((void*)(val8 + nx*ny*(nz - 2))) : ((void*)(val16 + nx*ny*(nz - 2)));
	unsigned int index;
	unsigned int label_value1, label_value2;
	float data_value1, data_value2;
	CellListIter iter;
	Cell *cell1, *cell2;
	ReplaceList rep_list;
	ReplaceListIter rep_iter;
	float size1, size2, size_ol;
	float vx1, vy1, vx2, vy2, d1, d2;
	//
	for (size_t i = backwards?nz:0;
		backwards?(i>1):(i<nz - 1);
		backwards?(--i):(++i))
	{
		InitializeCellList(page1, page1_data, bits, sscale, nx, ny, &cell_list1);
		InitializeCellList(page2, page2_data, bits, sscale, nx, ny, &cell_list2);
		//calculate overlap
		for (size_t ii = 0; ii<nx; ++ii)
		for (size_t jj = 0; jj<ny; ++jj)
		{
			index = nx*jj + ii;
			label_value1 = page1[index];
			label_value2 = page2[index];
			if (bits == 8)
			{
				data_value1 = ((unsigned char*)page1_data)[index] / 255.0f;
				data_value2 = ((unsigned char*)page2_data)[index] / 255.0f;
			}
			else if (bits == 16)
			{
				data_value1 = ((unsigned short*)page1_data)[index] / 65535.0f * sscale;
				data_value2 = ((unsigned short*)page2_data)[index] / 65535.0f * sscale;
			}
			if (label_value1 && label_value2)
			{
				iter = cell_list1.find(label_value1);
				if (iter != cell_list1.end())
				{
					cell1 = iter->second;
					iter = cell_list2.find(label_value2);
					if (iter == cell_list2.end())
						continue;
					cell2 = iter->second;
					bool found_edge = false;
					for (size_t kk = 0; kk<cell1->edges.size(); ++kk)
					{
						if (cell1->edges[kk]->cell2 == cell2)
						{
							cell1->edges[kk]->x = (cell1->edges[kk]->x *
								cell1->edges[kk]->sizei + ii) /
								(cell1->edges[kk]->sizei + 1);
							cell1->edges[kk]->y = (cell1->edges[kk]->y *
								cell1->edges[kk]->sizei + jj) /
								(cell1->edges[kk]->sizei + 1);
							cell1->edges[kk]->sizei++;
							cell1->edges[kk]->size +=
								min(data_value1, data_value2);
							found_edge = true;
						}
					}
					if (!found_edge)
					{
						Edge *edge = new Edge();
						edge->cell1 = cell1;
						edge->cell2 = cell2;
						edge->x = ii;
						edge->y = jj;
						edge->sizei = 1;
						edge->size = min(data_value1, data_value2);
						cell1->edges.push_back(edge);
					}
				}
			}
		}
		//build replacing list
		for (iter = cell_list1.begin(); iter != cell_list1.end(); ++iter)
		{
			cell1 = iter->second;
			if (cell1->edges.empty())
				continue;
			if (cell1->size <= size_thresh)
				continue;
			//sort
			if (cell1->edges.size() > 1)
				std::sort(cell1->edges.begin(),
					cell1->edges.end(), sort_ol);
			cell2 = cell1->edges[0]->cell2;
			if (cell2->size <= size_thresh)
				continue;
			size1 = cell1->size;
			size2 = cell2->size;
			size_ol = cell1->edges[0]->size;
			if (size_ol / size1 < size_ratio &&
				size_ol / size2 < size_ratio)
				continue;
			vx1 = cell1->x - cell1->edges[0]->x;
			vy1 = cell1->y - cell1->edges[0]->y;
			vx2 = cell2->x - cell1->edges[0]->x;
			vy2 = cell2->y - cell1->edges[0]->y;
			d1 = sqrt(vx1 * vx1 + vy1 * vy1);
			vx1 /= d1; vy1 /= d1;
			d2 = sqrt(vx2 * vx2 + vy2 * vy2);
			vx2 /= d2; vy2 /= d2;
			if (d1 > dist_thresh && d2 > dist_thresh &&
				fabs(vx1 * vx2 + vy1 * vy2) < angle_thresh)
				continue;
			rep_iter = rep_list.find(cell2->id);
			if (rep_iter != rep_list.end())
			{
				if (size_ol > rep_iter->second->size)
					rep_list.erase(rep_iter);
			}
			rep_list.insert(pair<unsigned int, Edge*>
				(cell2->id, cell1->edges[0]));
		}
		//replace
		for (size_t ii = 0; ii<nx; ++ii)
			for (size_t jj = 0; jj<ny; ++jj)
			{
				index = nx*jj + ii;
				label_value2 = page2[index];
				rep_iter = rep_list.find(label_value2);
				if (rep_iter != rep_list.end())
					page2[index] = rep_iter->second->cell1->id;
			}
		ClearCellList(&cell_list1);
		ClearCellList(&cell_list2);
		rep_list.clear();
		if (backwards)
		{
			page1 -= nx*ny;
			page2 -= nx*ny;
			if (bits == 8)
			{
				page1_data = (void*)(((unsigned char*)page1_data) - nx*ny);
				page2_data = (void*)(((unsigned char*)page2_data) - nx*ny);
			}
			else
			{
				page1_data = (void*)(((unsigned short*)page1_data) - nx*ny);
				page2_data = (void*)(((unsigned short*)page2_data) - nx*ny);
			}
		}
		else
		{
			page1 += nx*ny;
			page2 += nx*ny;
			if (bits == 8)
			{
				page1_data = (void*)(((unsigned char*)page1_data) + nx*ny);
				page2_data = (void*)(((unsigned char*)page2_data) + nx*ny);
			}
			else
			{
				page1_data = (void*)(((unsigned short*)page1_data) + nx*ny);
				page2_data = (void*)(((unsigned short*)page2_data) + nx*ny);
			}
		}
	}

	m_sig_progress();
}

