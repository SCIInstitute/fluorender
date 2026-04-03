/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <Global.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <Cell.h>
#include <set>

using namespace flrd;

ComponentSelector::ComponentSelector():
	m_sel_all(false),
	m_id(0),
	m_id_empty(true),
	m_use_min(false),
	m_use_max(false),
	m_min_num(0),
	m_max_num(0)
{
	m_list = std::make_unique<CelpList>();
}

ComponentSelector::~ComponentSelector()
{
}

void ComponentSelector::SetId(const std::string& str)
{
	unsigned int id = 0;
	int brick_id = 0;
	bool id_empty = false;

	std::string::size_type sz;
	try
	{
		id = std::stoul(str, &sz);
	}
	catch (...)
	{
		id_empty = true;
	}
	std::string str2;
	if (sz < str.size())
	{
		brick_id = id;
		for (size_t i = sz; i < str.size() - 1; ++i)
		{
			if (std::isdigit(static_cast<unsigned char>(str[i])))
			{
				str2 = str.substr(i);
				try
				{
					id = std::stoul(str2);
				}
				catch (...)
				{
					id_empty = true;
				}
			}
		}
	}

	SetId(flrd::Cell::GetKey(id, brick_id), id_empty);
}

void ComponentSelector::SetList(const CelpList& list)
{
	m_list = std::make_unique<CelpList>(list);
}

CelpList& ComponentSelector::GetList()
{
	return *m_list;
}

void ComponentSelector::SelectFullComp()
{
	if (m_id_empty)
		CompFull();
	else
		Select(true);
}

void ComponentSelector::SelectCompsCanvas(const std::vector<unsigned long long>& ids, bool sel_all)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	flrd::CelpList cl;
	if (sel_all)
		glbin_comp_analyzer.GetAllCelp(cl);
	else
		glbin_comp_analyzer.GetCelpFromIds(cl, ids);
	view->SetCellList(cl);
	//view->RefreshGL(39);
}

void ComponentSelector::SetSelectedCompIds(const std::set<unsigned long long>& ids, int mode)
{
	m_sel_ids = ids;
	m_sel_mode = mode;
}

void ComponentSelector::CompFull()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_>0)
		tex->push_mask();

	//get current mask
	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
		return;
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;
	//get current label
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;
	auto bricks = tex->get_bricks();
	if (bricks.empty())
		return;
	size_t bn = bricks.size();

	//get selected IDs
	int i, j, k;
	unsigned long long index;
	unsigned int label_value;
	unsigned int brick_id;
	CelpList sel_labels;
	CelpListIter label_iter;
	auto res = vd->GetResolution();

	for (auto bbs : bricks)
	{
		if (!bbs->is_mask_valid())
			continue;

		brick_id = bbs->get_id();
		auto res_b = bbs->get_size();
		auto off_size = bbs->get_off_size();
		for (i = 0; i < res_b.intx(); ++i)
		for (j = 0; j < res_b.inty(); ++j)
		for (k = 0; k < res_b.intz(); ++k)
		{
			index = (unsigned long long)res.get_size_xy()*(unsigned long long)(k + off_size.intz()) +
				(unsigned long long)res.intx()*(unsigned long long)(j + off_size.inty()) + (unsigned long long)(i + off_size.intx());
			if (mask_ptr[index] &&
				label_ptr[index])
			{
				label_value = label_ptr[index];
				label_iter = sel_labels.find(Cell::GetKey(label_value, brick_id));
				if (label_iter == sel_labels.end())
				{
					Cell* info = new Cell(label_value, brick_id);
					if (!glbin_comp_analyzer.GetAnalyzed())
						info->SetSizeUi(1);
					sel_labels.insert(std::pair<unsigned long long, Celp>
						(info->GetEId(), Celp(info)));
				}
				else if (!glbin_comp_analyzer.GetAnalyzed())
					label_iter->second->Inc();
			}
		}
	}

	CelpList list_out;
	CelpList* comp_list = GetListFromAnalyzer(sel_labels, list_out);

	//reselect
	unsigned int size;
	for (auto bbs : bricks)
	{
		brick_id = bbs->get_id();
		if (!bbs->is_mask_valid() &&
			!comp_list->FindBrick(brick_id))
			continue;
		
		auto res_b = bbs->get_size();
		auto off_size = bbs->get_off_size();
		for (i = 0; i < res_b.intx(); ++i)
		for (j = 0; j < res_b.inty(); ++j)
		for (k = 0; k < res_b.intz(); ++k)
		{
			index = (unsigned long long)res.get_size_xy()*(unsigned long long)(k + off_size.intz()) +
				(unsigned long long)res.intx()*(unsigned long long)(j + off_size.inty()) + (unsigned long long)(i + off_size.intx());
			if (label_ptr[index])
			{
				label_value = label_ptr[index];
				label_iter = comp_list->find(Cell::GetKey(label_value, brick_id));
				if (label_iter != comp_list->end())
				{
					if (m_use_min || m_use_max)
					{
						size = label_iter->second->GetSizeUi();
						if (CompareSize(size))
							SelectMask(mask_ptr, index, 255, tex);
						else
							mask_ptr[index] = 0;
					}
					else
						SelectMask(mask_ptr, index, 255, tex);
				}
				else
					mask_ptr[index] = 0;
			}
		}
		bbs->valid_mask();
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::Select(bool all, bool rmask)
{
	//get current mask
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_>0)
		tex->push_mask();

	auto raw_mask = vd->GetMask(rmask);
	if (!raw_mask)
	{
		vd->AddEmptyMask(0);
		raw_mask = vd->GetMask(false);
	}
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;
	//get current label
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;

	int bn = vd->GetBrickNum();
	unsigned int idsel, bidsel;
	if (bn > 1)
	{
		idsel = m_id << 32 >> 32;
		bidsel = m_id >> 32;
	}
	else
	{
		idsel = static_cast<unsigned int>(m_id);
		bidsel = 0;
	}

	//select append
	auto res = vd->GetResolution();
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	unsigned long long index;
	unsigned int label_value;
	CelpListIter label_iter;
	unsigned int bid;
	unsigned int size;
	if (all)
	{
		CelpList sel_labels;
		CelpList* comp_list = 0;
		if (glbin_comp_analyzer.GetAnalyzed())
			comp_list = glbin_comp_analyzer.GetCelpList();
		else
		{
			for (index = 0; index < for_size; ++index)
			{
				if (label_ptr[index])
				{
					label_value = label_ptr[index];
					bid = tex->get_brick_id(index);
					label_iter = sel_labels.find(Cell::GetKey(label_value, bid));
					if (label_iter == sel_labels.end())
					{
						Cell* info = new Cell(label_value, bid);
						info->SetSizeUi(1);
						sel_labels.insert(std::pair<unsigned long long, Celp>
							(info->GetEId(), Celp(info)));
					}
					else
						label_iter->second->Inc();
				}
			}
			comp_list = &sel_labels;
		}

		//reselect
		for (index = 0; index < for_size; ++index)
		{
			if (label_ptr[index])
			{
				label_value = label_ptr[index];
				bid = tex->get_brick_id(index);
				label_iter = comp_list->find(Cell::GetKey(label_value, bid));
				if (label_iter != comp_list->end())
				{
					if (m_use_min || m_use_max)
					{
						size = label_iter->second->GetSizeUi();
						if (CompareSize(size))
							//mask_ptr[index] = 255;
							SelectMask(mask_ptr, index, 255, tex);
						else
							mask_ptr[index] = 0;
					}
					else
						SelectMask(mask_ptr, index, 255, tex);
				}
				else if (!m_use_min && m_use_max)
					//analyzer filters small comps, make sure they are also selected here
					SelectMask(mask_ptr, index, 255, tex);
				else
					mask_ptr[index] = 0;
			}
		}
	}
	else
	{
		bool simple_select = true;
		simple_select = false;
		CelpList* analyzer_list = glbin_comp_analyzer.GetCelpList();
		auto iter = analyzer_list->find(m_id);
		if (iter == analyzer_list->end())
			simple_select = true;
		else
		{
			glbin_comp_analyzer.GetCellGraph()->ClearVisited();
			CelpList comp_list;
			if (glbin_comp_analyzer.GetCellGraph()->
				GetLinkedComps(iter->second, comp_list))
			{
				for (index = 0; index < for_size; ++index)
				{
					if (label_ptr[index])
					{
						label_value = label_ptr[index];
						bid = tex->get_brick_id(index);
						label_iter = comp_list.find(Cell::GetKey(label_value, bid));
						if (label_iter != comp_list.end())
						{
							if (m_use_min || m_use_max)
							{
								size = label_iter->second->GetSizeUi();
								if (CompareSize(size))
									SelectMask(mask_ptr, index, 255, tex);
								else
									mask_ptr[index] = 0;
							}
							else
								SelectMask(mask_ptr, index, 255, tex);
						}
					}
				}
			}
			else
				simple_select = true;
		}

		if (simple_select)
		{
			unsigned long long acc_size = 0;
			if (bn > 1)
			{
				for (index = 0; index < for_size; ++index)
				{
					bid = tex->get_brick_id(index);
					if (label_ptr[index] == idsel &&
						bid == bidsel)
						acc_size++;
				}
				if (((m_use_min || m_use_max) &&
					CompareSize((unsigned int)(acc_size))) ||
					(!m_use_min && !m_use_max))
				{
					for (index = 0; index < for_size; ++index)
					{
						bid = tex->get_brick_id(index);
						if (label_ptr[index] == idsel &&
							bid == bidsel)
							SelectMask(mask_ptr, index, 255, tex);
					}
				}
			}
			else
			{
				for (index = 0; index < for_size; ++index)
				{
					if (label_ptr[index] == idsel)
						acc_size++;
				}
				if (((m_use_min || m_use_max) &&
					CompareSize((unsigned int)(acc_size))) ||
					(!m_use_min && !m_use_max))
				{
					for (index = 0; index < for_size; ++index)
					{
						if (label_ptr[index] == idsel)
							SelectMask(mask_ptr, index, 255, tex);
					}
				}
			}
		}
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask(false);
}

void ComponentSelector::Exclusive()
{
	Clear(false);
	Select(false, false);
}

void ComponentSelector::All()
{
	//get current mask
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	if (flvr::Texture::mask_undo_num_>0 &&
		vd->GetTexture())
		vd->GetTexture()->push_mask();

	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
	{
		vd->AddEmptyMask(0);
		raw_mask = vd->GetMask(false);
	}
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;

	//select append
	auto res = vd->GetResolution();
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	std::memset(mask_ptr, 255, for_size);
	vd->GetTexture()->valid_all_mask();
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::Clear(bool invalidate)
{
	//get current mask
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	if (flvr::Texture::mask_undo_num_>0 &&
		vd->GetTexture())
		vd->GetTexture()->push_mask();

	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
		return;
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;

	//select append
	auto res = vd->GetResolution();
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	std::memset(mask_ptr, 0, for_size);
	//invalidate label mask in gpu
	if (invalidate)
		vd->GetVR()->clear_tex_mask();
	vd->GetTexture()->invalid_all_mask();
}

void ComponentSelector::Delete()
{
	//get current mask
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
		return;
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;
	//get current label
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;
	//select append
	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	for (index = 0; index < for_size; ++index)
	{
		if (m_id == label_ptr[index])
			SelectMask(mask_ptr, index, 255, tex);
		else
			mask_ptr[index] = 0;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::DeleteList()
{
	std::set<unsigned long long> ids;
	for (auto it : *m_list)
		ids.insert(it.first);

	bool clear_all = ids.empty();

	//get current mask
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
		return;
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;
	//get current label
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;
	//select append
	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	unsigned long long key;
	int bn = vd->GetBrickNum();
	for (index = 0; index < for_size; ++index)
	{
		if (clear_all)
			mask_ptr[index] = 0;
		else
		{
			if (bn > 1)
				key = flrd::Cell::GetKey(
					label_ptr[index],
					vd->GetTexture()->
					get_brick_id(index));
			else
				key = label_ptr[index];
			if (find(ids.begin(), ids.end(), key)
				!= ids.end())
				SelectMask(mask_ptr, index, 255, tex);
			else
				mask_ptr[index] = 0;
		}
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask();
	if (clear_all)
		tex->invalid_all_mask();
}

void ComponentSelector::SelectList()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_ > 0)
		tex->push_mask();

	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
	{
		vd->AddEmptyMask(0);
		raw_mask = vd->GetMask(false);
	}
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;
	//get current label
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;
	auto bricks = tex->get_bricks();
	if (bricks.empty())
		return;
	size_t bn = bricks.size();

	//select append
	unsigned int brick_id;
	unsigned long long index;
	unsigned long long key;
	int i, j, k;
	auto res = vd->GetResolution();
	for (auto bbs : bricks)
	{
		brick_id = bbs->get_id();
		auto res_b = bbs->get_size();
		auto off_size = bbs->get_off_size();
		for (i = 0; i < res_b.intx(); ++i)
		for (j = 0; j < res_b.inty(); ++j)
		for (k = 0; k < res_b.intz(); ++k)
		{
			index = (unsigned long long)res.get_size_xy() * (unsigned long long)(k + off_size.intz()) +
				(unsigned long long)res.intx() * (unsigned long long)(j + off_size.inty()) + (unsigned long long)(i + off_size.intx());
			key = brick_id;
			key = (key << 32) | label_ptr[index];
			if (m_list->find(key) != m_list->end())
				SelectMask(mask_ptr, index, 255, tex);
			else
				mask_ptr[index] = 0;
		}
	}

	//unsigned long long for_size = (unsigned long long)nx *
	//	(unsigned long long)ny * (unsigned long long)nz;
	//unsigned long long index;
	//for (index = 0;
	//	index < for_size; ++index)
	//{
	//	if (list.find(label_ptr[index]) != list.end())
	//		mask_ptr[index] = 255;
	//	else
	//		mask_ptr[index] = 0;
	//}

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::EraseList()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_ > 0)
		tex->push_mask();

	auto raw_mask = vd->GetMask(true);
	if (!raw_mask)
	{
		vd->AddEmptyMask(0);
		raw_mask = vd->GetMask(false);
	}
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;
	//get current label
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;
	auto bricks = tex->get_bricks();
	if (bricks.empty())
		return;
	size_t bn = bricks.size();

	//select append
	unsigned int brick_id;
	unsigned long long index;
	unsigned long long key;
	int i, j, k;
	auto res = vd->GetResolution();
	for (auto bbs : bricks)
	{
		brick_id = bbs->get_id();
		auto res_b = bbs->get_size();
		auto off_size = bbs->get_off_size();
		for (i = 0; i < res_b.intx(); ++i)
		for (j = 0; j < res_b.inty(); ++j)
		for (k = 0; k < res_b.intz(); ++k)
		{
			index = (unsigned long long)res.get_size_xy() * (unsigned long long)(k + off_size.intz()) +
				(unsigned long long)res.intx() * (unsigned long long)(j + off_size.inty()) + (unsigned long long)(i + off_size.intx());
			key = brick_id;
			key = (key << 32) | label_ptr[index];
			if (m_list->find(key) != m_list->end())
				SelectMask(mask_ptr, index, 0, tex);
			else
				mask_ptr[index] = 0;
		}
	}

	//unsigned long long for_size = (unsigned long long)nx *
	//	(unsigned long long)ny * (unsigned long long)nz;
	//unsigned long long index;
	//for (index = 0;
	//	index < for_size; ++index)
	//{
	//	if (list.find(label_ptr[index]) != list.end())
	//		mask_ptr[index] = 255;
	//	else
	//		mask_ptr[index] = 0;
	//}

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_mask();
}

inline CelpList* ComponentSelector::GetListFromAnalyzer(CelpList &list_in, CelpList &list_out)
{
	if (glbin_comp_analyzer.GetAnalyzed())
	{
		//assign graph node identifier for sel_labels
		CelpList* analyzer_list = glbin_comp_analyzer.GetCelpList();
		for (auto iter = list_in.begin(); iter != list_in.end(); )
		{
			auto iter2 = analyzer_list->find(iter->first);
			if (iter2 == analyzer_list->end())
			{
				//remove
				iter = list_in.erase(iter);
				continue;
			}
			iter->second->SetCelVrtx(iter2->second->GetCelVrtx());
			iter->second->SetSizeUi(iter2->second->GetSizeUi());
			++iter;
		}
		if (glbin_comp_analyzer.GetCellGraph()->
			GetLinkedComps(list_in, list_out,
				glbin_comp_analyzer.GetSizeLimit()))
			return &list_out;
		else
			return &list_in;
	}
	else
		return &list_in;
	return 0;
}

void ComponentSelector::SelectMask(unsigned char* mask,
	unsigned long long idx, unsigned char v, flvr::Texture* tex)
{
	mask[idx] = v;
	if (tex)
	{
		auto b = tex->get_brick(
			tex->get_brick_id(idx));
		if (b)
			b->valid_mask();
	}
}


