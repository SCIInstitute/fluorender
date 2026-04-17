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
#include <CompEditor.h>
#include <Global.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <TrackGroup.h>
#include <DataManager.h>
#include <MovieMaker.h>
#include <Texture.h>
#include <VolumeRenderer.h>
#include <TrackMap.h>
#include <Cell.h>
#include <lbl_reader.h>
#include <msk_writer.h>
#include <VolCache4D.h>

using namespace flrd;

ComponentEditor::ComponentEditor() :
	m_id(0),
	m_id_empty(true)
{
	m_list = std::make_unique<CelpList>();
}

ComponentEditor::~ComponentEditor()
{

}

void ComponentEditor::SetList(const CelpList& list)
{
	m_list = std::make_unique<CelpList>(list);
}

CelpList& ComponentEditor::GetList()
{
	return *m_list;
}

std::string ComponentEditor::GetOutput()
{
	return m_output;
}

fluo::Color ComponentEditor::GetColor()
{
	fluo::Color c;
	if (m_id_empty)
		c = fluo::Color(1, 1, 1);
	else
	{
		if (m_id == 0)
			c = fluo::Color(0.094117674, 0.654901961, 0.709803922);
		else
		{
			int shuffle = 0;
			if (auto cur_vd = glbin_current.vol_data.lock())
				shuffle = cur_vd->GetShuffle();
			c = fluo::Color(m_id, shuffle);
		}
	}
	return c;
}

void ComponentEditor::Clean(int mode)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
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

	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	for (index = 0; index < for_size; ++index)
	{
		switch (mode)
		{
		case 0:
		default:
			if (!mask_ptr[index])
				label_ptr[index] = 0;
			break;
		case 1:
			if (mask_ptr[index])
				label_ptr[index] = 0;
			break;
		}
	}
	vd->GetVolumeRenderer().clear_tex_current();
}

void ComponentEditor::NewId(bool append, bool track)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	//trace group
	auto trkg = view->GetTrackGroup();
	if (!trkg)
	{
		view->CreateTrackGroup();
		trkg = view->GetTrackGroup();
	}

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	std::shared_ptr<fluo::RawData> raw_mask = nullptr;
	//get current mask
	raw_mask = vd->GetMask(true);
	if (!raw_mask)
	{
		vd->AddEmptyMask(0);
		raw_mask = vd->GetMask(false);
	}
	unsigned char* mask_ptr = raw_mask->DataAs<unsigned char>();
	if (!mask_ptr)
		return;

	//get current label
	auto raw_label = vd->GetLabel(false);
	if (!raw_label)
	{
		vd->AddEmptyLabel();
		raw_label = vd->GetLabel(false);
	}
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;

	auto res = vd->GetResolution();
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	unsigned long long index;

	//get ID of currently/previously masked region
	unsigned int id_vol = 0;
	if (m_id_empty)
	{
		for (index = 0; index < for_size; ++index)
		{
			if (mask_ptr[index] &&
				label_ptr[index])
			{
				id_vol = label_ptr[index];
				break;
			}
		}
	}

	//generate a unique ID
	unsigned int new_id = 0;
	unsigned int inc = 0;
	if (m_id_empty)
	{
		if (id_vol)
		{
			new_id = id_vol + 10;
			inc = 10;
		}
		else
		{
			new_id = 10;
			inc = 10;
		}
	}
	else
	{
		if (m_id)
		{
			new_id = m_id;
			inc = 253;
		}
		else
		{
			new_id = 0;
			inc = 0;
		}
	}
	unsigned int stop_id = new_id;
	if (inc)
	{
		while (vd->SearchLabel(new_id))
		{
			new_id += inc;
			if (new_id == stop_id)
			{
				m_output = "ID assignment failed. Type a different ID than " +
					std::to_string(stop_id);
				return;
			}
		}
	}

	//update label volume, set mask region to the new ID
	int i, j, k;
	Celp cell;
	if (new_id)
		cell = Celp(new Cell(new_id));
	for (i = 0; i < res.intx(); ++i) for (j = 0; j < res.inty(); ++j) for (k = 0; k < res.intz(); ++k)
	{
		index = res.get_size_xy()*k + res.intx() * j + i;
		if (mask_ptr[index])
		{
			if (append && label_ptr[index])
				continue;
			label_ptr[index] = new_id;
			if (new_id)
				cell->Inc(fluo::Point(i, j, k), 1.0f);
		}
	}

	//invalidate label mask in gpu
	vd->GetVolumeRenderer().clear_tex_current();

	//save label mask to disk
	int cur_time = view->m_tseq_cur_num;
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());

	if (new_id && track)
	{
		//trkg->AddCell(cell, m_cur_time);
		pTrackMap track_map = trkg->get().GetTrackMap();
		glbin_trackmap_proc.SetTrackMap(track_map);
		auto cache_queue = glbin_data_manager.GetCacheQueue(vd);
		if (cache_queue)
			cache_queue->set_max_size(4);
		//add
		cell->Calc();
		glbin_trackmap_proc.AddCellDup(cell, cur_time);
	}
}

void ComponentEditor::ReplaceId()
{
	if (m_id_empty)
		return;
	if (!m_id)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	int cur_time = view->m_tseq_cur_num;
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

	unsigned int old_id;
	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	for (index = 0; index < for_size; ++index)
	{
		old_id = label_ptr[index];
		if (!mask_ptr[index] ||
			!old_id ||
			old_id == m_id)
			continue;

		label_ptr[index] = m_id;
	}
	//invalidate label mask in gpu
	vd->GetVolumeRenderer().clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());
}

void ComponentEditor::ReplaceList()
{
	if (m_id_empty)
		return;
	if (!m_id)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	//trace group
	glbin_trackmap_proc.SetTrackMap(trkg->get().GetTrackMap());
	int cur_time = view->m_tseq_cur_num;

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

	//replace ID
	CelpListIter cell_iter;
	std::unordered_map<unsigned int, unsigned int> list_rep;
	std::unordered_map<unsigned int, unsigned int>::iterator list_rep_iter;
	unsigned int old_id, new_id;
	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	for (index = 0; index < for_size; ++index)
	{
		old_id = label_ptr[index];
		if (!mask_ptr[index] ||
			!old_id ||
			old_id == m_id)
			continue;

		list_rep_iter = list_rep.find(old_id);
		if (list_rep_iter != list_rep.end())
		{
			label_ptr[index] = list_rep_iter->second;
			continue;
		}

		cell_iter = m_list->find(old_id);
		if (cell_iter != m_list->end())
		{
			new_id = m_id;
			while (vd->SearchLabel(new_id))
				new_id += 253;
			//add cell to list_rep
			list_rep.insert(std::pair<unsigned int, unsigned int>
				(old_id, new_id));
			glbin_trackmap_proc.ReplaceCellID(old_id, new_id,
					cur_time);
			label_ptr[index] = new_id;
		}
	}
	//invalidate label mask in gpu
	vd->GetVolumeRenderer().clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());
}

void ComponentEditor::CombineId()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	int cur_time = view->m_tseq_cur_num;
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

	//combine IDs
	unsigned int id = 0;
	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	for (index = 0; index < for_size; ++index)
	{
		if (!mask_ptr[index] ||
			!label_ptr[index])
			continue;
		if (!id)
			id = label_ptr[index];
		if (label_ptr[index] != id)
			label_ptr[index] = id;
	}
	//invalidate label mask in gpu
	vd->GetVolumeRenderer().clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());
}

void ComponentEditor::CombineList()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	if (m_list->size() <= 1)
		return;//nothing to combine
	//trace group
	auto trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	glbin_trackmap_proc.SetTrackMap(trkg->get().GetTrackMap());
	int cur_time = view->m_tseq_cur_num;

	//find the largest cell in the list
	flrd::Celp cell;
	flrd::CelpListIter cell_iter;
	for (cell_iter = m_list->begin();
		cell_iter != m_list->end(); ++cell_iter)
	{
		if (cell)
		{
			if (cell_iter->second->GetSizeUi() >
				cell->GetSizeUi())
				cell = cell_iter->second;
		}
		else
			cell = cell_iter->second;
	}
	if (!cell)
		return;

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

	//combine IDs
	auto res = vd->GetResolution();
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	for (index = 0; index < for_size; ++index)
	{
		if (!mask_ptr[index] ||
			!label_ptr[index])
			continue;
		cell_iter = m_list->find(label_ptr[index]);
		if (cell_iter != m_list->end())
			label_ptr[index] = cell->Id();
	}
	//invalidate label mask in gpu
	vd->GetVolumeRenderer().clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());

	//modify graphs
	glbin_trackmap_proc.CombineCells(cell, *m_list,
		cur_time);
}
