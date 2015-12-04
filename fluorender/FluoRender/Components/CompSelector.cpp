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
#include "CompSelector.h"
#include "DataManager.h"

using namespace FL;

ComponentSelector::ComponentSelector(VolumeData* vd)
	: m_vd(vd),
	m_sel_all(false),
	m_id(0),
	m_use_min(false),
	m_use_max(false),
	m_min_num(0),
	m_max_num(0)
{

}

ComponentSelector::~ComponentSelector()
{
}

void ComponentSelector::CompFull()
{
	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//get selected IDs
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	m_vd->GetResolution(nx, ny, nz);
	unsigned int label_value;
	CellList sel_labels;
	CellListIter label_iter;
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_mask[index] &&
			data_label[index])
		{
			label_value = data_label[index];
			label_iter = sel_labels.find(label_value);
			if (label_iter == sel_labels.end())
			{
				FL::pCell cell(new FL::Cell(label_value));
				cell->Inc(i, j, k, 1.0f);
				sel_labels.insert(pair<unsigned int, FL::pCell>
					(label_value, cell));
			}
			else
				label_iter->second->Inc(i, j, k, 1.0f);
		}
	}

	//reselect
	unsigned int size;
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_label[index])
		{
			label_value = data_label[index];
			label_iter = sel_labels.find(label_value);
			if (label_iter != sel_labels.end())
			{
				if (m_use_min || m_use_max)
				{
					size = label_iter->second->GetSizeUi();
					if (CompareSize(size))
						data_mask[index] = 255;
					else
						data_mask[index] = 0;
				}
				else
					data_mask[index] = 255;
			}
			else
				data_mask[index] = 0;
		}
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::Append(bool all)
{
	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
	{
		m_vd->AddEmptyMask(0);
		nrrd_mask = m_vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx*
		(unsigned long long)ny * (unsigned long long)nz;
	if (all)
	{
		int i, j, k;
		unsigned long long index;
		unsigned int label_value;
		CellList sel_labels;
		CellListIter label_iter;
		for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
		for (k = 0; k < nz; ++k)
		{
			index = (unsigned long long)nx*(unsigned long long)ny*(unsigned long long)k +
				(unsigned long long)nx*(unsigned long long)j + (unsigned long long)i;
			if (data_label[index])
			{
				label_value = data_label[index];
				label_iter = sel_labels.find(label_value);
				if (label_iter == sel_labels.end())
				{
					FL::pCell cell(new FL::Cell(label_value));
					cell->Inc(i, j, k, 1.0f);
					sel_labels.insert(pair<unsigned int, FL::pCell>
						(label_value, cell));
				}
				else
					label_iter->second->Inc(i, j, k, 1.0f);
			}
		}
		unsigned int size;
		for (index = 0;
		index < for_size; ++index)
		{
			if (data_label[index])
			{
				label_value = data_label[index];
				label_iter = sel_labels.find(label_value);
				if (label_iter != sel_labels.end())
				{
					if (m_use_min || m_use_max)
					{
						size = label_iter->second->GetSizeUi();
						if (CompareSize(size))
							data_mask[index] = 255;
						else
							data_mask[index] = 0;
					}
					else
						data_mask[index] = 255;
				}
				else
					data_mask[index] = 0;
			}
		}
	}
	else
	{
		unsigned long long acc_size = 0;
		for (unsigned long long index = 0;
		index < for_size; ++index)
		{
			if (data_label[index] == m_id)
				acc_size++;
		}
		if (((m_use_min || m_use_max) &&
			CompareSize((unsigned int)(acc_size))) ||
			(!m_use_min && !m_use_max))
		{
			for (unsigned long long index = 0;
			index < for_size; ++index)
			{
				if (data_label[index] == m_id)
					data_mask[index] = 255;
			}
		}
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::Exclusive()
{
	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
	{
		m_vd->AddEmptyMask(0);
		nrrd_mask = m_vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned long long acc_size = 0;
	for (index = 0;
	index < for_size; ++index)
	{
		if (data_label[index] == m_id)
			acc_size++;
		data_mask[index] = 0;
	}
	if (((m_use_min || m_use_max) &&
		CompareSize((unsigned int)(acc_size))) ||
		(!m_use_min && !m_use_max))
	{
		for (index = 0;
		index < for_size; ++index)
		{
			if (data_label[index] == m_id)
				data_mask[index] = 255;
		}
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::All()
{
	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
	{
		m_vd->AddEmptyMask(0);
		nrrd_mask = m_vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	memset(data_mask, 255, for_size);
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::Clear()
{
	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	memset(data_mask, 0, for_size);
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::Delete()
{
	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (m_id == data_label[index])
			data_mask[index] = 255;
		else
			data_mask[index] = 0;
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::Delete(std::vector<unsigned int> &ids)
{
	bool clear_all = ids.empty();

	//get current mask
	if (!m_vd)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (clear_all)
			data_mask[index] = 0;
		else if (find(ids.begin(), ids.end(), data_label[index])
			!= ids.end())
			data_mask[index] = 255;
		else
			data_mask[index] = 0;
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}
