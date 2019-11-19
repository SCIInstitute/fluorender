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
#include "DataManager.h"
#include "Diffusion.h"
#include "cl_code.h"

using namespace FL;

Diffusion::Diffusion(VolumeData* vd)
	: m_vd(vd)
{
}

Diffusion::~Diffusion()
{
}

bool Diffusion::CheckBricks()
{
	if (!m_vd || !m_vd->GetTexture())
		return false;
	vector<TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return false;
	return true;
}

void ComponentGenerator::GetMask(size_t brick_num, TextureBrick* b, void** val)
{
	if (!b)
		return;

	if (brick_num > 1)
	{
		int c = b->nlabel();
		int nb = b->nb(c);
		int nx = b->nx();
		int ny = b->ny();
		int nz = b->nz();
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz*(unsigned long long)nb;
		unsigned char* temp = new unsigned char[mem_size];
		unsigned char* tempp = temp;
		unsigned char* tp = (unsigned char*)(b->tex_data(c));
		unsigned char* tp2;
		for (unsigned int k = 0; k < nz; ++k)
		{
			tp2 = tp;
			for (unsigned int j = 0; j < ny; ++j)
			{
				memcpy(tempp, tp2, nx*nb);
				tempp += nx * nb;
				tp2 += b->sx()*nb;
			}
				tp += b->sx()*b->sy()*nb;
		}
		*val32 = (void*)temp;
	}
	else
	{
		Nrrd* nrrd_label = m_vd->GetLabel(false);
		if (!nrrd_label)
			return;
		*val = (void*)(nrrd_label->data);
	}
}

void ComponentGenerator::ReleaseMask(void* val, size_t brick_num, TextureBrick* b)
{
	if (!val32 || brick_num <= 1)
		return;

	unsigned char* tempp = (unsigned char*)val32;
	int c = b->nlabel();
	int nb = b->nb(c);
	int nx = b->nx();
	int ny = b->ny();
	int nz = b->nz();
	unsigned char* tp = (unsigned char*)(b->tex_data(c));
	unsigned char* tp2;
	for (unsigned int k = 0; k < nz; ++k)
	{
		tp2 = tp;
		for (unsigned int j = 0; j < ny; ++j)
		{
			memcpy(tp2, tempp, nx*nb);
			tempp += nx * nb;
			tp2 += b->sx()*nb;
		}
		tp += b->sx()*b->sy()*nb;
	}
	delete[] val32;
}
