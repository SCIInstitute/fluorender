/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include "Tracks.h"
#include <VertexArray.h>
#include <TextureRenderer.h>
#include <compatibility_utilities.h>

using namespace flrd;

int Tracks::m_num = 0;
Tracks::Tracks()
{
	//type = 8;//traces
	m_num++;
	m_name = "Traces " + std::to_string(m_num);
	m_cur_time = -1;
	m_prv_time = -1;
	m_ghost_num = 10;
	m_draw_tail = true;
	m_draw_lead = false;
	m_cell_size = 20;
	m_uncertain_low = 0;
	m_track_map = flrd::pTrackMap(new flrd::TrackMap());
}

Tracks::~Tracks()
{
}

void Tracks::SetCurTime(int time)
{
	m_cur_time = time;
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
}

int Tracks::GetCurTime()
{
	return m_cur_time;
}
void Tracks::SetPrvTime(int time)
{
	m_prv_time = time;
}

int Tracks::GetPrvTime()
{
	return m_prv_time;
}

void Tracks::SetGhostNum(int num)
{
	m_ghost_num = num;
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
}

int Tracks::GetGhostNum()
{
	return m_ghost_num;
}

void Tracks::SetDrawTail(bool draw)
{
	m_draw_tail = draw;
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
}

bool Tracks::GetDrawTail()
{
	return m_draw_tail;
}

void Tracks::SetDrawLead(bool draw)
{
	m_draw_lead = draw;
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
}

bool Tracks::GetDrawLead()
{
	return m_draw_lead;
}

//get information
void Tracks::GetLinkLists(size_t frame,
	flrd::VertexList &in_orphan_list,
	flrd::VertexList &out_orphan_list,
	flrd::VertexList &in_multi_list,
	flrd::VertexList &out_multi_list)
{
	if (in_orphan_list.size())
		in_orphan_list.clear();
	if (out_orphan_list.size())
		out_orphan_list.clear();
	if (in_multi_list.size())
		in_multi_list.clear();
	if (out_multi_list.size())
		out_multi_list.clear();

	flrd::TrackMapProcessor tm_processor(m_track_map);
	tm_processor.SetSizeThresh(m_cell_size);
	tm_processor.SetUncertainLow(m_uncertain_low);
	tm_processor.GetLinkLists(frame,
		in_orphan_list, out_orphan_list,
		in_multi_list, out_multi_list);
}

void Tracks::ClearCellList()
{
	m_cell_list.clear();
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
}

//cur_sel_list: ids from previous time point
//m_prv_time: previous time value
//m_id_map: ids of current time point that are linked to previous
//m_cur_time: current time value
//time values are check with frame ids in the frame list
void Tracks::UpdateCellList(flrd::CelpList &cur_sel_list)
{
	ClearCellList();
	flrd::CelpListIter cell_iter;

	//why does not the time change?
	//because I just want to find out the current selection
	if (m_prv_time == m_cur_time)
	{
		//copy cur_sel_list to m_cell_list
		for (cell_iter = cur_sel_list.begin();
			cell_iter != cur_sel_list.end();
			++cell_iter)
		{
			if (cell_iter->second->GetSizeUi() >
				(unsigned int)m_cell_size)
				m_cell_list.insert(std::pair<unsigned int, flrd::Celp>
				(cell_iter->second->Id(), cell_iter->second));
		}
		return;
	}

	//get mapped cells
	//cur_sel_list -> m_cell_list
	flrd::TrackMapProcessor tm_processor(m_track_map);
	tm_processor.GetMappedCells(
		cur_sel_list, m_cell_list,
		(unsigned int)m_prv_time,
		(unsigned int)m_cur_time);

	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
}

flrd::CelpList &Tracks::GetCellList()
{
	return m_cell_list;
}

bool Tracks::FindCell(unsigned int id)
{
	return m_cell_list.find(id) != m_cell_list.end();
}

//modifications
bool Tracks::AddCell(flrd::Celp &cell, size_t frame)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.AddCellDup(cell, frame);
}

bool Tracks::LinkCells(flrd::CelpList &list1, flrd::CelpList &list2,
	size_t frame1, size_t frame2, bool exclusive)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.LinkCells(list1, list2,
		frame1, frame2, exclusive);
}

bool Tracks::IsolateCells(flrd::CelpList &list, size_t frame)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.IsolateCells(list, frame);
}

bool Tracks::UnlinkCells(flrd::CelpList &list1, flrd::CelpList &list2,
	size_t frame1, size_t frame2)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.UnlinkCells(list1, list2, frame1, frame2);
}

bool Tracks::CombineCells(flrd::Celp &cell, flrd::CelpList &list,
	size_t frame)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.CombineCells(cell, list, frame);
}

bool Tracks::DivideCells(flrd::CelpList &list, size_t frame)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.DivideCells(list, frame);
}

bool Tracks::ReplaceCellID(unsigned int old_id, unsigned int new_id, size_t frame)
{
	flrd::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.ReplaceCellID(old_id, new_id, frame);
}

bool Tracks::GetMappedRulers(flrd::RulerList &rulers)
{
	size_t frame_num = m_track_map->GetFrameNum();
	if (m_ghost_num <= 0 ||
		m_cur_time < 0 ||
		m_cur_time >= frame_num)
		return false;

	//estimate verts size
	size_t remain_num = frame_num - m_cur_time - 1;
	size_t ghost_lead, ghost_tail;
	ghost_lead = m_draw_lead ?
		(remain_num > m_ghost_num ?
			m_ghost_num : remain_num) : 0;
	ghost_tail = m_draw_tail ?
		(m_cur_time >= m_ghost_num ?
			m_ghost_num : m_cur_time) : 0;

	flrd::CelpList temp_sel_list1, temp_sel_list2;

	if (m_draw_lead)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
			i < m_cur_time + ghost_lead; ++i)
		{
			GetMappedRulers(
				temp_sel_list1, temp_sel_list2,
				rulers, i, i + 1);
			//swap
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	//clear ruler id??
	for (auto iter = rulers.begin();
		iter != rulers.end(); ++iter)
		(*iter)->Id(0);

	if (m_draw_tail)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
			i > m_cur_time - ghost_tail; --i)
		{
			GetMappedRulers(
				temp_sel_list1, temp_sel_list2,
				rulers, i, i - 1);
			//sawp
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	return true;
}

unsigned int Tracks::GetMappedEdges(
	flrd::CelpList & sel_list1, flrd::CelpList & sel_list2,
	std::vector<float>& verts,
	size_t frame1, size_t frame2,
	int shuffle)
{
	unsigned int result = 0;

	size_t frame_num = m_track_map->GetFrameNum();
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return result;

	flrd::CelpList &cell_list1 = m_track_map->GetCellList(frame1);
	flrd::InterGraph &inter_graph = m_track_map->GetInterGraph(
		frame1 > frame2 ? frame2 : frame1);
	flrd::CelpListIter sel_iter, cell_iter;
	flrd::Verp vertex1, vertex2;
	flrd::Celp cell;
	flrd::Vrtx v1, v2;
	std::pair<flrd::AdjIter, flrd::AdjIter> adj_verts;
	flrd::AdjIter inter_iter;
	flrd::CellBinIter pwcell_iter;
	fluo::Color c;
	std::pair<flrd::Edge, bool> inter_edge;

	for (sel_iter = sel_list1.begin();
		sel_iter != sel_list1.end();
		++sel_iter)
	{
		cell_iter = cell_list1.find(sel_iter->second->Id());
		if (cell_iter == cell_list1.end())
			continue;
		vertex1 = cell_iter->second->GetVertex().lock();
		if (!vertex1)
			continue;
		v1 = vertex1->GetInterVert(inter_graph);
		if (v1 == flrd::InterGraph::null_vertex())
			continue;
		adj_verts = boost::adjacent_vertices(v1, inter_graph);
		//for each adjacent vertex
		for (inter_iter = adj_verts.first;
			inter_iter != adj_verts.second;
			++inter_iter)
		{
			v2 = *inter_iter;
			//get edge
			inter_edge = boost::edge(v1, v2, inter_graph);
			if (!inter_edge.second)
				continue;
			else if (!inter_graph[inter_edge.first].link)
				continue;
			vertex2 = inter_graph[v2].vertex.lock();
			if (!vertex2)
				continue;
			//store all cells in sel_list2
			for (pwcell_iter = vertex2->GetCellsBegin();
				pwcell_iter != vertex2->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				sel_list2.insert(std::pair<unsigned int, flrd::Celp>
					(cell->Id(), cell));
				//save to verts
				c = fluo::Color(cell->Id(), shuffle);
				verts.push_back(vertex1->GetCenter().x());
				verts.push_back(vertex1->GetCenter().y());
				verts.push_back(vertex1->GetCenter().z());
				verts.push_back(c.r());
				verts.push_back(c.g());
				verts.push_back(c.b());
				verts.push_back(vertex2->GetCenter().x());
				verts.push_back(vertex2->GetCenter().y());
				verts.push_back(vertex2->GetCenter().z());
				verts.push_back(c.r());
				verts.push_back(c.g());
				verts.push_back(c.b());
				result += 2;
			}
		}
	}

	return result;
}

bool Tracks::GetMappedRulers(
	flrd::CelpList& sel_list1, flrd::CelpList &sel_list2,
	flrd::RulerList& rulers,
	size_t frame1, size_t frame2)
{
	size_t frame_num = m_track_map->GetFrameNum();
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	flrd::CelpList &cell_list1 = m_track_map->GetCellList(frame1);
	flrd::InterGraph &inter_graph = m_track_map->GetInterGraph(
		frame1 > frame2 ? frame2 : frame1);
	flrd::CelpListIter sel_iter, cell_iter;
	flrd::Verp vertex1, vertex2;
	flrd::Celp cell;
	flrd::Vrtx v1, v2;
	std::pair<flrd::AdjIter, flrd::AdjIter> adj_verts;
	flrd::AdjIter inter_iter;
	flrd::CellBinIter pwcell_iter;
	fluo::Color c;
	std::pair<flrd::Edge, bool> inter_edge;
	flrd::RulerListIter ruler_iter;

	for (sel_iter = sel_list1.begin();
		sel_iter != sel_list1.end();
		++sel_iter)
	{
		cell_iter = cell_list1.find(sel_iter->second->Id());
		if (cell_iter == cell_list1.end())
			continue;
		vertex1 = cell_iter->second->GetVertex().lock();
		if (!vertex1)
			continue;
		v1 = vertex1->GetInterVert(inter_graph);
		if (v1 == flrd::InterGraph::null_vertex())
			continue;
		adj_verts = boost::adjacent_vertices(v1, inter_graph);
		//for each adjacent vertex
		for (inter_iter = adj_verts.first;
			inter_iter != adj_verts.second;
			++inter_iter)
		{
			v2 = *inter_iter;
			//get edge
			inter_edge = boost::edge(v1, v2, inter_graph);
			if (!inter_edge.second)
				continue;
			else if (!inter_graph[inter_edge.first].link)
				continue;
			vertex2 = inter_graph[v2].vertex.lock();
			if (!vertex2)
				continue;
			//store all cells in sel_list2
			for (pwcell_iter = vertex2->GetCellsBegin();
				pwcell_iter != vertex2->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				sel_list2.insert(std::pair<unsigned int, flrd::Celp>
					(cell->Id(), cell));
				//save to rulers
				ruler_iter = FindRulerFromList(vertex1->Id(), rulers);
				if (ruler_iter == rulers.end())
				{
					flrd::Ruler* ruler = new flrd::Ruler();
					ruler->SetRulerType(1);//multi-point
					ruler->AddPoint(vertex1->GetCenter());
					ruler->AddPoint(vertex2->GetCenter());
					ruler->SetTimeDep(false);
					ruler->Id(vertex2->Id());
					rulers.push_back(ruler);
				}
				else
				{
					flrd::Ruler* ruler = *ruler_iter;
					ruler->AddPoint(vertex2->GetCenter());
					ruler->Id(vertex2->Id());
				}
			}
		}
	}

	return true;
}

flrd::RulerListIter Tracks::FindRulerFromList(unsigned int id, flrd::RulerList &list)
{
	auto iter = list.begin();
	while (iter != list.end())
	{
		if ((*iter)->Id() == id)
			return iter;
		++iter;
	}
	return iter;
}

void Tracks::Clear()
{
	m_track_map->Clear();
}

bool Tracks::LoadData(const std::wstring &filename)
{
	m_data_path = filename;
	flrd::TrackMapProcessor tm_processor(m_track_map);
	std::string str = ws2s(m_data_path);
	return tm_processor.Import(str);
}

bool Tracks::SaveData(const std::wstring &filename)
{
	m_data_path = filename;
	flrd::TrackMapProcessor tm_processor(m_track_map);
	std::string str = ws2s(m_data_path);
	return tm_processor.Export(str);
}

unsigned int Tracks::Draw(std::vector<float> &verts, int shuffle)
{
	unsigned int result = 0;
	size_t frame_num = m_track_map->GetFrameNum();
	if (m_ghost_num <= 0 ||
		m_cur_time < 0 ||
		m_cur_time >= frame_num ||
		m_cell_list.empty())
		return result;

	//estimate verts size
	size_t remain_num = frame_num - m_cur_time - 1;
	size_t ghost_lead, ghost_tail;
	ghost_lead = m_draw_lead ?
		(remain_num > m_ghost_num ?
			m_ghost_num : remain_num) : 0;
	ghost_tail = m_draw_tail ?
		(m_cur_time >= m_ghost_num ?
			m_ghost_num : m_cur_time) : 0;
	verts.reserve((ghost_lead + ghost_tail) *
		m_cell_list.size() * 3 * 6 * 3);//1.5 branches each

	flrd::CelpList temp_sel_list1, temp_sel_list2;

	if (m_draw_lead)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
			i < m_cur_time + ghost_lead; ++i)
		{
			result += GetMappedEdges(
				temp_sel_list1, temp_sel_list2,
				verts, i, i + 1, shuffle);
			//swap
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	if (m_draw_tail)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
			i > m_cur_time - ghost_tail; --i)
		{
			result += GetMappedEdges(
				temp_sel_list1, temp_sel_list2,
				verts, i, i - 1, shuffle);
			//sawp
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	return result;
}

