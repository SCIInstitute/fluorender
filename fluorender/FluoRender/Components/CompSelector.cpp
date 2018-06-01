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
#include "CompAnalyzer.h"
#include "DataManager.h"

using namespace FL;

ComponentSelector::ComponentSelector(VolumeData* vd)
	: m_vd(vd),
	m_analyzer(0),
	m_sel_all(false),
	m_id(0),
	m_brick_id(-1),
	m_use_min(false),
	m_use_max(false),
	m_min_num(0),
	m_max_num(0)
{

}

ComponentSelector::~ComponentSelector()
{
}

void ComponentSelector::SetAnalyzer(ComponentAnalyzer* analyzer)
{
	m_analyzer = analyzer;
}

ComponentAnalyzer* ComponentSelector::GetAnalyzer()
{
	return m_analyzer;
}

void ComponentSelector::CompFull()
{
	//get current mask
	if (!m_vd)
		return;

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

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
	unsigned int brick_id;
	CompList sel_labels;
	CompListIter label_iter;
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = (unsigned long long)nx*(unsigned long long)ny*(unsigned long long)k +
			(unsigned long long)nx*(unsigned long long)j + (unsigned long long)i;
		if (data_mask[index] &&
			data_label[index])
		{
			label_value = data_label[index];
			brick_id = tex->get_brick_id(index);
			label_iter = sel_labels.find(GetKey(label_value, brick_id));
			if (label_iter == sel_labels.end())
			{
				CompInfo* info = new CompInfo;
				info->id = label_value;
				info->brick_id = brick_id;
				if (!m_analyzer || !m_analyzer->GetAnalyzed())
					info->sumi = 1;
				sel_labels.insert(std::pair<unsigned long long, pCompInfo>
					(GetKey(label_value, brick_id), pCompInfo(info)));
			}
			else if (!m_analyzer || !m_analyzer->GetAnalyzed())
				label_iter->second->sumi++;
		}
	}

	CompList list_out;
	CompList* comp_list = GetListFromAnalyzer(sel_labels, list_out);

	//reselect
	unsigned int size;
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		if (data_label[index])
		{
			label_value = data_label[index];
			brick_id = tex->get_brick_id(index);
			label_iter = comp_list->find(GetKey(label_value, brick_id));
			if (label_iter != comp_list->end())
			{
				if (m_use_min || m_use_max)
				{
					size = label_iter->second->sumi;
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

void ComponentSelector::Append(bool all, bool rmask)
{
	//get current mask
	if (!m_vd)
		return;

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

	Nrrd* nrrd_mask = m_vd->GetMask(rmask);
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
	unsigned long long index;
	unsigned int label_value;
	unsigned int brick_id;
	CompListIter label_iter;
	unsigned int size;
	if (all)
	{
		int i, j, k;
		CompList sel_labels;
		for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
		for (k = 0; k < nz; ++k)
		{
			index = (unsigned long long)nx*(unsigned long long)ny*(unsigned long long)k +
				(unsigned long long)nx*(unsigned long long)j + (unsigned long long)i;
			if (data_label[index])
			{
				label_value = data_label[index];
				brick_id = tex->get_brick_id(index);
				label_iter = sel_labels.find(GetKey(label_value, brick_id));
				if (label_iter == sel_labels.end())
				{
					CompInfo* info = new CompInfo;
					info->id = label_value;
					info->brick_id = brick_id;
					if (!m_analyzer || !m_analyzer->GetAnalyzed())
						info->sumi = 1;
					sel_labels.insert(std::pair<unsigned long long, pCompInfo>
						(GetKey(label_value, brick_id), pCompInfo(info)));
				}
				else if (!m_analyzer || !m_analyzer->GetAnalyzed())
					label_iter->second->sumi++;
			}
		}

		CompList list_out;
		CompList* comp_list = GetListFromAnalyzer(sel_labels, list_out);

		//reselect
		for (index = 0; index < for_size; ++index)
		{
			if (data_label[index])
			{
				label_value = data_label[index];
				brick_id = tex->get_brick_id(index);
				label_iter = comp_list->find(GetKey(label_value, brick_id));
				if (label_iter != comp_list->end())
				{
					if (m_use_min || m_use_max)
					{
						size = label_iter->second->sumi;
						if (CompareSize(size))
							data_mask[index] = 255;
						else
							data_mask[index] = 0;
					}
					else
						data_mask[index] = 255;
				}
				else if (!m_use_min && m_use_max)
					//analyzer filters small comps, make sure they are also selected here
					data_mask[index] = 255;
				else
					data_mask[index] = 0;
			}
		}
	}
	else
	{
		bool simple_select = false;
		if (m_analyzer)
		{
			CompList* analyzer_list = m_analyzer->GetCompList();
			auto iter = analyzer_list->find(GetKey(m_id, m_brick_id));
			if (iter == analyzer_list->end())
				simple_select = true;
			else
			{
				m_analyzer->GetCompGraph()->ClearVisited();
				CompList comp_list;
				if (m_analyzer->GetCompGraph()->
					GetLinkedComps(iter->second, comp_list))
				{
					for (index = 0; index < for_size; ++index)
					{
						if (data_label[index])
						{
							label_value = data_label[index];
							brick_id = tex->get_brick_id(index);
							label_iter = comp_list.find(GetKey(label_value, brick_id));
							if (label_iter != comp_list.end())
							{
								if (m_use_min || m_use_max)
								{
									size = label_iter->second->sumi;
									if (CompareSize(size))
										data_mask[index] = 255;
									else
										data_mask[index] = 0;
								}
								else
									data_mask[index] = 255;
							}
						}
					}
				}
				else
					simple_select = true;
			}
		}

		if (simple_select)
		{
			unsigned long long acc_size = 0;
			if (m_brick_id >= 0)
			{
				for (index = 0; index < for_size; ++index)
				{
					brick_id = tex->get_brick_id(index);
					if (data_label[index] == m_id &&
						brick_id == m_brick_id)
						acc_size++;
				}
				if (((m_use_min || m_use_max) &&
					CompareSize((unsigned int)(acc_size))) ||
					(!m_use_min && !m_use_max))
				{
					for (index = 0; index < for_size; ++index)
					{
						brick_id = tex->get_brick_id(index);
						if (data_label[index] == m_id &&
							brick_id == m_brick_id)
							data_mask[index] = 255;
					}
				}
			}
			else
			{
				for (index = 0; index < for_size; ++index)
				{
					if (data_label[index] == m_id)
						acc_size++;
				}
				if (((m_use_min || m_use_max) &&
					CompareSize((unsigned int)(acc_size))) ||
					(!m_use_min && !m_use_max))
				{
					for (index = 0; index < for_size; ++index)
					{
						if (data_label[index] == m_id)
							data_mask[index] = 255;
					}
				}
			}
		}
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

void ComponentSelector::Exclusive()
{
	Clear(false);
	Append(false, false);
}

void ComponentSelector::All()
{
	//get current mask
	if (!m_vd)
		return;

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

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

void ComponentSelector::Clear(bool invalidate)
{
	//get current mask
	if (!m_vd)
		return;

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

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
	if (invalidate)
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

void ComponentSelector::SelectList(CellList& list)
{
	if (!m_vd)
		return;

	if (Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

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
	for (index = 0;
		index < for_size; ++index)
	{
		if (list.find(data_label[index]) != list.end())
			data_mask[index] = 255;
		else
			data_mask[index] = 0;
	}

	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_pool();
}

inline CompList* ComponentSelector::GetListFromAnalyzer(CompList &list_in, CompList &list_out)
{
	if (m_analyzer && m_analyzer->GetAnalyzed())
	{
		//assign graph node identifier for sel_labels
		CompList* analyzer_list = m_analyzer->GetCompList();
		for (auto iter = list_in.begin(); iter != list_in.end(); )
		{
			auto iter2 = analyzer_list->find(iter->first);
			if (iter2 == analyzer_list->end())
			{
				//remove
				iter = list_in.erase(iter);
				continue;
			}
			iter->second->v = iter2->second->v;
			iter->second->sumi = iter2->second->sumi;
			++iter;
		}
		if (m_analyzer->GetCompGraph()->
			GetLinkedComps(list_in, list_out))
			return &list_out;
		else
			return &list_in;
	}
	else
		return &list_in;
	return 0;
}
