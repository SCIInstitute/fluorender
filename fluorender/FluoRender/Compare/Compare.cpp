/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "Compare.h"
#include <Scenegraph/VolumeData.h>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/TextureBrick.h>
#include <FLIVR/Texture.h>
#include "cl_code.h"
#include <algorithm>

using namespace FL;

ChannelCompare::ChannelCompare(VolumeData* vd)
	: m_vd(vd),
	m_use_mask(false),
	m_init(false)
{
}

ChannelCompare::~ChannelCompare()
{
}

#define GET_VOLDATA \
	if (!m_vd) \
		return; \
	long nx, ny, nz; \
	m_vd->getValue("res x", nx); \
	m_vd->getValue("res y", ny); \
	m_vd->getValue("res z", nz); \
	Nrrd* nrrd_data = 0; \
	if (m_use_mask) \
		nrrd_data = m_vd->GetMask(true); \
	if (!nrrd_data) \
		nrrd_data = m_vd->GetData(false); \
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
	vector<FLIVR::TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks(); \
	if (!bricks || bricks->size() == 0) \
		return;

#define GET_VOLDATA_STREAM \
	long nx, ny, nz; \
	unsigned char* val8 = 0; \
	unsigned short* val16 = 0; \
	int bits = 8; \
	unsigned int* val32 = 0; \
	FLIVR::TextureBrick* b = 0; \
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
		m_vd->getValue("res x", nx); \
		m_vd->getValue("res y", ny); \
		m_vd->getValue("res z", nz); \
		Nrrd* nrrd_data = 0; \
		if (m_use_mask) \
			nrrd_data = m_vd->GetMask(false); \
		if (!nrrd_data) \
			nrrd_data = m_vd->GetData(false); \
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

