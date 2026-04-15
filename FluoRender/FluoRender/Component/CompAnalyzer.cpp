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
#include <CompAnalyzer.h>
#include <Global.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <AnnotData.h>
#include <VolumeGroup.h>
#include <DataManager.h>
#include <VolumeDefault.h>
#include <CurrentObjects.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <Ruler.h>
#include <RulerList.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>

using namespace flrd;

ComponentAnalyzer::ComponentAnalyzer()
	: Progress(),
	m_use_sel(false),
	m_analyzed(false),
	m_colocal(false),
	m_bn(0),
	m_slimit(5),
	m_compgroup(0)
{
	//m_compgroup = AddCompGroup(vd);
	//glbin_comp_def.Apply(this);
}

ComponentAnalyzer::~ComponentAnalyzer()
{
}

void ComponentAnalyzer::SetVolume(const std::shared_ptr<VolumeData>& vd)
{
	if (!vd)
		return;
	CompGroup* compgroup = FindCompGroup(vd);
	if (!compgroup)
		compgroup = AddCompGroup(vd);
	m_compgroup = compgroup;
}

std::shared_ptr<VolumeData> ComponentAnalyzer::GetVolume()
{
	if (m_compgroup)
		return m_compgroup->vd.lock();
	return nullptr;
}

void ComponentAnalyzer::SetCoVolumes(const std::vector<std::weak_ptr<VolumeData>>& list)
{
	m_vd_list = list;
}

void ComponentAnalyzer::AddCoVolume(const std::shared_ptr<VolumeData>& vd)
{
	m_vd_list.push_back(vd);
}

void ComponentAnalyzer::ClearCoVolumes()
{
	m_vd_list.clear();
}

std::optional<std::reference_wrapper<CelpList>> ComponentAnalyzer::GetCelpList()
{
	if (m_compgroup)
		return m_compgroup->celps;
	return std::nullopt;
}

std::optional<std::reference_wrapper<CellGraph>> ComponentAnalyzer::GetCellGraph()
{
	if (m_compgroup)
		return m_compgroup->graph;
	return std::nullopt;
}

int ComponentAnalyzer::GetCompGroupSize()
{
	return static_cast<int>(m_comp_groups.size());
}

std::optional<std::reference_wrapper<CompGroup>> ComponentAnalyzer::GetCompGroup(int i)
{
	if (i >= 0 && i < m_comp_groups.size())
		return m_comp_groups[i];
	return std::nullopt;
}

int ComponentAnalyzer::GetBrickNum()
{
	return m_bn;
}

int ComponentAnalyzer::GetColocalization(
	size_t bid,
	const fluo::Point& p,
	std::vector<unsigned int> &sumi,
	std::vector<double> &sumd)
{
	int num = 0;

	for (auto it = m_vd_list.begin(); it != m_vd_list.end(); ++it)
	{
		auto vd = it->lock();

		if (!vd)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		auto tex = vd->GetTexture();
		if (!tex)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		auto b = tex->get_brick(static_cast<unsigned int>(bid));
		if (!b)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		double value = b->get_data(p);
		sumi.push_back(value > 0.0 ? 1 : 0);
		sumd.push_back(value);
		num++;
	}

	return num;
}

void ComponentAnalyzer::Analyze()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd || !vd->GetTexture())
		return;

	double prog = 0;
	SetProgress(static_cast<int>(prog), "Analyzing components.");

	if (!FindCompGroup(vd))
		m_compgroup = AddCompGroup(vd);

	m_compgroup->Clear();

	if (m_colocal)
	{
		ClearCoVolumes();
		auto view = glbin_current.render_view.lock();
		if (view)
		{
			for (int i = 0; i < view->GetDispVolumeNum(); ++i)
			{
				auto vdi = view->GetDispVolumeData(i);
				if (vdi != vd)
					AddCoVolume(vdi);
			}
		}
	}

	prog += 10;
	SetProgress(static_cast<int>(prog), "Analyzing components.");

	auto spc = vd->GetSpacing();
	auto bricks = vd->GetTexture()->get_bricks();
	int bits = vd->GetBits();
	if (bricks.empty())
		return;
	size_t bn = bricks.size();

	//comp list
	CelpList &comps = m_compgroup->celps;
	//clear list and start calculating
	comps.clear();
	comps.min = std::numeric_limits<unsigned int>::max();
	comps.max = 0;
	comps.scale = spc;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;
	graph.clear();
	m_analyzed = false;

	vd->GetVR()->return_label();
	bool use_sel = m_use_sel;//need to check if mask is valid
	if (use_sel)
		vd->GetVR()->return_mask();

	unsigned int size_limit;
	if (bn > 1)
		size_limit = m_slimit;
	else
		size_limit = 0;
	if (m_use_min)
		size_limit = std::max(m_min_num, size_limit);

	//progress range is 10 - 90
	size_t bi = 0;
	for (auto bbs : bricks)
	{
		void* data_ptr = 0;
		unsigned char* mask_ptr = 0;
		unsigned int* label_ptr = 0;
		fluo::Vector res;
		int nb = 1;
		if (bn > 1)
		{
			if (use_sel && !bbs->is_mask_valid())
				continue;
			// get brick if ther are more than one brick
			nb = bbs->nb(flvr::CompType::Data);
			auto res_b = bbs->get_size();
			res = res_b - fluo::Vector(1);
			auto stride = bbs->get_stride();
			int cnt = (res.intx()>0 ? 1 : 0) + (res.inty()>0 ? 1 : 0) + (res.intz()>0 ? 1 : 0);
			if (cnt < 2) continue;

			//size
			unsigned long long mem_size;
			mem_size = res.intz() ? ((unsigned long long)res.get_size_xyz()*nb) :
				((unsigned long long)res.get_size_xy()*nb);
			//data
			unsigned char* temp = new unsigned char[mem_size];
			unsigned char* tempp = temp;
			unsigned char* tp = bbs->get_raw_data(flvr::CompType::Data)->DataAs<unsigned char>();
			unsigned char* tp2;
			unsigned int kn = res.intz() ? res.intz() : 1;
			for (unsigned int k = 0; k < kn; ++k)
			{
				tp2 = tp;
				for (int j = 0; j < res.inty(); ++j)
				{
					memcpy(tempp, tp2, res.intx()*nb);
					tempp += res.intx()*nb;
					tp2 += stride.intx()*nb;
				}
				tp += stride.get_size_xy()*nb;
			}
			data_ptr = (void*)temp;
			//mask
			if (use_sel)
			{
				nb = bbs->nb(flvr::CompType::Mask);
				mem_size = res.intz() ? ((unsigned long long)res.get_size_xyz() * nb) :
					((unsigned long long)res.get_size_xy() * nb);
				temp = new unsigned char[mem_size];
				tempp = temp;
				tp = bbs->get_raw_data(flvr::CompType::Mask)->DataAs<unsigned char>();
				for (unsigned int k = 0; k < kn; ++k)
				{
					tp2 = tp;
					for (int j = 0; j < res.inty(); ++j)
					{
						memcpy(tempp, tp2, res.intx()*nb);
						tempp += res.intx()*nb;
						tp2 += stride.intx()*nb;
					}
					tp += stride.get_size_xy() * nb;
				}
				mask_ptr = temp;
			}
			//label
			nb = bbs->nb(flvr::CompType::Label);
			mem_size = res.intz() ? ((unsigned long long)res.get_size_xyz() * nb) :
				((unsigned long long)res.get_size_xy() * nb);
			temp = new unsigned char[mem_size];
			tempp = temp;
			tp = bbs->get_raw_data(flvr::CompType::Label)->DataAs<unsigned char>();
			for (unsigned int k = 0; k < kn; ++k)
			{
				tp2 = tp;
				for (int j = 0; j < res.inty(); ++j)
				{
					memcpy(tempp, tp2, res.intx()*nb);
					tempp += res.intx()*nb;
					tp2 += stride.intx() *nb;
				}
				tp += stride.get_size_xy() *nb;
			}
			label_ptr = (unsigned int*)temp;
		}
		else
		{
			// get data if there is only one brick
			res = vd->GetResolution();
			auto raw_data = vd->GetVolume(false);
			if (raw_data)
				data_ptr = raw_data->GetDataVoid();
			auto raw_mask = vd->GetMask(false);
			if (raw_mask)
				mask_ptr = raw_mask->DataAs<unsigned char>();
			auto raw_label = vd->GetLabel(false);
			if (raw_label)
				label_ptr = raw_label->DataAs<unsigned int>();
			if (!data_ptr || (use_sel && !mask_ptr) || !label_ptr)
				return;
		}

		prog += 80 * (0.2 + double(bi) / bn);
		SetProgress(static_cast<int>(prog), "Analyzing components.");

		unsigned int id = 0;
		unsigned int brick_id = bbs->get_id();
		double value;
		double scale;
		double ext;
		int i, j, k;
		//m_vd->GetResolution(nx, ny, nz);
		unsigned long long for_size = res.intz() ?
			((unsigned long long)res.get_size_xyz()) :
			((unsigned long long)res.get_size_xy());
		unsigned long long index;
		CelpList comp_list_brick;
		CelpListIter iter;
		for (index = 0; index < for_size; ++index)
		{
			if (index % 10000 == 0)
			{
				prog += 320000.0 * (bi + 1) / for_size / bn;
				SetProgress(static_cast<int>(prog),
					"Analyzing components.");
			}

			value = 0.0;
			if (use_sel)
			{
				if (mask_ptr && !mask_ptr[index])
					continue;
			}
			if (label_ptr && !label_ptr[index])
				continue;

			if (label_ptr)
				id = label_ptr[index];

			if (bits == 8)
			{
				value = ((unsigned char*)data_ptr)[index] / 255.0;
				scale = 255.0;
			}
			else if (bits == 16)
			{
				value = ((unsigned short*)data_ptr)[index] / 65535.0;
				scale = 65535.0;
			}

			if (value <= 0.0)
				continue;

			k = static_cast<int>(index / (res.get_size_xy()));
			j = static_cast<int>(index % (res.get_size_xy()));
			i = j % res.intx();
			j = j / res.intx();
			auto p = fluo::Point(i, j, k);
			auto res2 = fluo::Vector(
				res.intx() ? res.intx() : 1,
				res.inty() ? res.inty() : 1,
				res.intz() ? res.intz() : 1);
			ext = GetExt(label_ptr, index, id, res2, p);
			//ext = 0.0;

			//colocalization
			std::vector<unsigned int> sumi;
			std::vector<double> sumd;
			if (m_colocal) GetColocalization(brick_id, p, sumi, sumd);

			//find in list
			iter = comp_list_brick.find(id);
			if (iter == comp_list_brick.end())
			{
				//not found
				Cell* info = 0;
				if (m_colocal)
				{
					info = new Cell(id, brick_id,
						1, value, static_cast<unsigned int>(std::round(ext)),
						p + bbs->get_off_size(),
						spc,
						sumi, sumd);
				}
				else
					info = new Cell(id, brick_id,
						1, value, static_cast<unsigned int>(std::round(ext)),
						p + bbs->get_off_size(),
						spc);
					comp_list_brick.insert(std::pair<unsigned int, Celp>
						(id, Celp(info)));
			}
			else
			{
				if (m_colocal)
					iter->second->Inc(1, value, static_cast<unsigned int>(std::round(ext)),
						p + bbs->get_off_size(),
						spc,
						sumi, sumd);
				else
					iter->second->Inc(1, value, static_cast<unsigned int>(std::round(ext)),
						p + bbs->get_off_size(),
						spc);
			}
		}

		size_t count = 0;
		size_t ticks = comp_list_brick.size();
		for (iter = comp_list_brick.begin();
			iter != comp_list_brick.end(); ++iter)
		{
			if (count % 10000 == 0)
			{
				prog += 320000.0 * (bi + 1) / ticks / bn;
				SetProgress(static_cast<int>(prog),
					"Analyzing components.");
			}
			count++;

			if (iter->second->GetSizeUi() < size_limit)
				continue;
			//iter->second->var = sqrt(iter->second->m2 / (iter->second->sumi));
			//iter->second->mean *= scale;
			//iter->second->min *= scale;
			//iter->second->max *= scale;
			//iter->second->Calc();
			comps.min = static_cast<unsigned int>(std::round(iter->second->GetSizeD())) <
				comps.min ? static_cast<unsigned int>(std::round(iter->second->GetSizeD())) :
				comps.min;
			comps.max = static_cast<unsigned int>(std::round(iter->second->GetSizeD())) >
				comps.max ? static_cast<unsigned int>(std::round(iter->second->GetSizeD())) :
				comps.max;
			comps.insert(std::pair<unsigned long long, Celp>
				(iter->second->GetEId(), iter->second));
		}

		if (bn > 1)
		{
			if (data_ptr) delete[] (unsigned char*)data_ptr;
			if (mask_ptr) delete[] (unsigned char*)mask_ptr;
			if (label_ptr) delete[] (unsigned int*)label_ptr;
		}
		++bi;
	}

	prog = 90;
	SetProgress(static_cast<int>(prog), "Analyzing components.");

	MatchBricks(use_sel);
	UpdateMaxCompSize(m_colocal);
	if (m_consistent)
	{
		MakeColorConsistent();
		vd->GetVR()->clear_tex_label();
	}

	m_compgroup->dirty = false;
	m_colocal = m_colocal && m_vd_list.size();
	if (!use_sel)
		m_analyzed = true;

	SetRange(0, 100);
	SetProgress(0, "");
}

void ComponentAnalyzer::MatchBricks(bool sel)
{
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd || !vd->GetTexture())
		return;
	auto tex = vd->GetTexture();
	auto bricks = tex->get_bricks_id();
	if (bricks.size() <= 1)
		return;
	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	int bits;
	size_t bn = bricks.size();
	for (auto bbs : bricks)
	{
		//get one brick
		if (sel && !bbs->is_mask_valid())
			continue;
		void* data_data = 0;
		unsigned char* data_mask = 0;
		unsigned int* data_label = 0;
		int nb = 1;
		auto res = bbs->get_size();
		auto stride = bbs->get_stride();
		nb = bbs->nb(flvr::CompType::Data);
		bits = nb * 8;
		//label
		nb = bbs->nb(flvr::CompType::Label);
		unsigned long long mem_size = (unsigned long long)res.get_size_xyz()*nb;
		unsigned char* temp = new unsigned char[mem_size];
		unsigned char* tempp = temp;
		unsigned char* tp = bbs->get_raw_data(flvr::CompType::Label)->DataAs<unsigned char>();
		unsigned char* tp2;
		for (int k = 0; k < res.intz(); ++k)
		{
			tp2 = tp;
			for (int j = 0; j < res.inty(); ++j)
			{
				memcpy(tempp, tp2, res.intx()*nb);
				tempp += res.intx()*nb;
				tp2 += stride.intx()*nb;
			}
			tp += stride.get_size_xy()*nb;
		}
		data_label = (unsigned int*)temp;
		//check 3 positive faces
		unsigned int l1, l2, index, b1, b2;
		//(x, y, nz-1)
		if (res.intz() > 1)
		{
			for (int j = 0; j < res.inty() - 1; ++j)
			for (int i = 0; i < res.intx() - 1; ++i)
			{
				index = res.get_size_xy()*(res.intz() - 1) + j * res.intx() + i;
				l1 = data_label[index];
				if (!l1) continue;
				index -= res.get_size_xy();
				l2 = data_label[index];
				if (!l2) continue;
				//get brick ids
				b2 = bbs->get_id();
				b1 = tex->poszid(b2);
				if (b1 == b2) continue;
				//if l1 and l2 are different and already in the comp list
				//connect them
				auto i1 = comps.find(Cell::GetKey(l1, b1));
				auto i2 = comps.find(Cell::GetKey(l2, b2));
				if (i1 != comps.end() && i2 != comps.end())
					graph.LinkComps(i1->second, i2->second);
			}
		}
		//(x, ny-1, z)
		int kz = res.intz() > 1 ? res.intz() - 1 : 1;
		if (res.inty() > 1)
		{
			for (int k = 0; k < kz; ++k)
			for (int i = 0; i < res.intx() - 1; ++i)
			{
				index = res.get_size_xy()*k + res.intx() * (res.inty() - 1) + i;
				l1 = data_label[index];
				if (!l1) continue;
				index -= res.intx();
				l2 = data_label[index];
				if (!l2) continue;
				//get brick ids
				b2 = bbs->get_id();
				b1 = tex->posyid(b2);
				if (b1 == b2) continue;
				//if l1 and l2 are different and already in the comp list
				//connect them
				auto i1 = comps.find(Cell::GetKey(l1, b1));
				auto i2 = comps.find(Cell::GetKey(l2, b2));
				if (i1 != comps.end() && i2 != comps.end())
					graph.LinkComps(i1->second, i2->second);
			}
		}
		//(nx-1, y, z)
		if (res.intx() > 1)
		{
			for (int k = 0; k < kz; ++k)
			for (int j = 0; j < res.inty() - 1; ++j)
			{
				index = res.get_size_xy()*k + res.intx() * j + res.intx() - 1;
				l1 = data_label[index];
				if (!l1) continue;
				index -= 1;
				l2 = data_label[index];
				if (!l2) continue;
				//get brick ids
				b2 = bbs->get_id();
				b1 = tex->posxid(b2);
				if (b1 == b2) continue;
				//if l1 and l2 are different and already in the comp list
				//connect them
				auto i1 = comps.find(Cell::GetKey(l1, b1));
				auto i2 = comps.find(Cell::GetKey(l2, b2));
				if (i1 != comps.end() && i2 != comps.end())
					graph.LinkComps(i1->second, i2->second);
			}
		}

		if (data_data) delete[] (unsigned char*)data_data;
		if (data_mask) delete[] (unsigned char*)data_mask;
		if (data_label) delete[] (unsigned int*)data_label;

		//m_sig_progress();
	}
}

void ComponentAnalyzer::UpdateMaxCompSize(bool colocal)
{
	if (!m_compgroup)
		return;
	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	graph.ClearVisited();
	std::pair<CelVrtIter, CelVrtIter> vertices =
		boost::vertices(graph);
	for (auto iter = vertices.first; iter != vertices.second; ++iter)
	{
		if (graph[*iter].visited)
			continue;
		CelpList list;
		Celp info = graph[*iter].cell.lock();
		if (graph.GetLinkedComps(info, list, m_slimit))
		{
			Cell temp(0, 0);//temporary cell for accumulation
			for (auto li = list.begin();
				li != list.end(); ++li)
				temp.Inc(li->second);

			//update in comp list
			comps.max = std::max(temp.GetSizeUi(), comps.max);

			//update each
			for (auto li = list.begin();
				li != list.end(); ++li)
				li->second->Copy(temp);
		}
	}
}

void ComponentAnalyzer::MakeColorConsistent()
{
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd || !vd->GetTexture())
		return;
	auto tex = vd->GetTexture();
	auto bricks = tex->get_bricks();
	if (bricks.size() <= 1)
		return;
	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	graph.ClearVisited();
	for (auto i = comps.begin();
		i != comps.end(); ++i)
	{
		if (graph.Visited(i->second))
			continue;
		CelpList list;
		if (graph.GetLinkedComps(i->second, list, m_slimit))
		{
			//make color consistent
			//get the id of the first item in the list
			//which is also the base color
			unsigned int base_id = list.begin()->second->Id();
			//for other items
			for (auto iter = std::next(list.begin());
				iter != list.end(); ++iter)
			{
				unsigned int link_id = iter->second->Id();
				unsigned int diff = base_id > link_id ? base_id - link_id : link_id - base_id;
				if (diff % 253)
				{
					//color is different
					//generate new_id
					ReplaceId(base_id, iter->second);
					//insert one with consistent key
					if (link_id != iter->second->Id())
						comps.insert(std::pair<unsigned long long, Celp>
						(iter->second->GetEId(), iter->second));
				}
			}
		}
	}

	//remove comps with inconsistent keys
	for (auto i = comps.begin();
		i != comps.end();)
	{
		if (i->first != i->second->GetEId())
			i = comps.erase(i);
		else
			++i;
	}

	//m_sig_progress();
}

void ComponentAnalyzer::Count()
{
	m_count = 0;
	m_vox = 0;
	m_size = 0;

	auto list = GetCelpList();
	if (!list || list->get().empty())
		return;

	for (auto it = list->get().begin();
		it != list->get().end(); ++it)
	{
		unsigned int sumi = it->second->GetSizeUi();
		if (sumi > m_min_num &&
			(!m_use_max ||
				(m_use_max && sumi < m_max_num)))
		{
			++m_count;
			m_vox += sumi;
		}
	}
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd)
		return;
	auto spc = vd->GetSpacing();
	m_size = m_vox * spc.x() * spc.y() * spc.z();
}

void ComponentAnalyzer::ClearCompGroup()
{
	if (!m_compgroup)
		return;
	m_compgroup->Clear();
	m_compgroup->vd.reset();
}

size_t ComponentAnalyzer::GetListSize()
{
	if (!m_compgroup)
		return 0;
	return m_compgroup->celps.size();
}

size_t ComponentAnalyzer::GetCompSize()
{
	if (!m_compgroup)
		return 0;
	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	size_t list_size = comps.size();
	size_t graph_size = boost::num_vertices(graph);
	size_t cc_size = 0;
	if (graph_size)
	{
		graph.ClearVisited();
		std::pair<CelVrtIter, CelVrtIter> vertices =
			boost::vertices(graph);
		for (auto iter = vertices.first; iter != vertices.second; ++iter)
		{
			if (graph[*iter].visited)
				continue;
			CelpList list;
			Celp info = graph[*iter].cell.lock();
			graph.GetLinkedComps(info, list, m_slimit);
			cc_size++;
		}
	}

	return list_size - graph_size + cc_size;
}

void ComponentAnalyzer::GetCompsPoint(fluo::Point& p, std::set<unsigned long long> &ids)
{
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd || ! vd->GetTexture())
		return;

	//get label data
	auto raw_label = vd->GetLabel(true);
	if (!raw_label)
		return;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return;
	auto res = vd->GetResolution();
	int ix = static_cast<int>(std::round(p.x()));
	int iy = static_cast<int>(std::round(p.y()));
	int iz = static_cast<int>(std::round(p.z()));
	if (ix < 0 || ix >= res.intx() ||
		iy < 0 || iy >= res.inty() ||
		iz < 0 || iz >= res.intz())
		return;
	unsigned long long index = (unsigned long long)res.get_size_xy() * iz +
		(unsigned long long)res.intx() * iy + ix;
	unsigned int id = label_ptr[index];
	if (!id)
		return;
	int bn = vd->GetBrickNum();

	unsigned long long ull;
	if (bn > 1)
	{
		unsigned int brick_id = vd->GetTexture()->get_brick_id(index);
		ull = Cell::GetKey(id, brick_id);
	}
	else
		ull = id;
	ids.insert(ull);

	//id, brick_id
	CellGraph &graph = m_compgroup->graph;
	size_t graph_size = boost::num_vertices(graph);
	if (graph_size)
	{
		graph.ClearVisited();
		std::pair<CelVrtIter, CelVrtIter> vertices =
			boost::vertices(graph);
		for (auto iter = vertices.first; iter != vertices.second; ++iter)
		{
			if (graph[*iter].visited)
				continue;
			CelpList list;
			Celp info = graph[*iter].cell.lock();
			if (info &&
				ids.find(info->GetEId()) != ids.end())
			{
				graph.GetLinkedComps(info, list, m_slimit);
				for (auto it2 = list.begin();
					it2 != list.end(); ++it2)
					ids.insert(it2->second->GetEId());
			}
		}
	}
}

void ComponentAnalyzer::OutputFormHeader(std::string &str)
{
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd)
		return;

	str = "ID\t";
	if (vd && vd->GetAllBrickNum() > 1)
		str += "BRICK_ID\t";

	str += "PosX\tPosY\tPosZ\tSumN\tSumI\tPhysN\tPhysI\tSurfN\tSurfI\tMean\tSigma\tMin\tMax\tDist\tPcaL";

	if (m_colocal)
	{
		for (auto it = m_vd_list.begin(); it != m_vd_list.end(); ++it)
		{
			auto vd = it->lock();
			if (!vd)
				continue;
			str += "\t" + ws2s(vd->GetName()) + "\t";
		}
	}
	str += "\n";
}

void ComponentAnalyzer::OutputCompListStream(std::ostream &stream, int verbose, const std::string& comp_header)
{
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd)
		return;
	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	m_bn = vd->GetAllBrickNum();

	if (verbose == 1)
	{
		stream << "Statistics on the selection:\n";
		stream << "A total of " <<
			GetCompSize() <<
			" component(s) selected\n";
		std::string header;
		OutputFormHeader(header);
		stream << header;
	}

	auto scale = comps.scale;
	double size_scale = scale.x() * scale.y() * scale.z();
	double maxscale = vd->GetMaxScale();
	double scalarscale = vd->GetScalarScale();
	fluo::Vector lens;

	graph.ClearVisited();
	for (auto i = comps.begin();
		i != comps.end(); ++i)
	{
		if (comp_header != "")
			stream << comp_header << "\t";

		std::list<unsigned int> ids;
		std::list<unsigned int> brick_ids;
		bool added = false;

		if (m_bn > 1)
		{
			if (graph.Visited(i->second))
				continue;

			CelpList list;
			if (graph.GetLinkedComps(i->second, list, m_slimit))
			{
				for (auto iter = list.begin();
					iter != list.end(); ++iter)
				{
					ids.push_back(iter->second->Id());
					brick_ids.push_back(iter->second->BrickId());
				}
				added = true;
			}
		}
		if (!added)
		{
			ids.push_back(i->second->Id());
			brick_ids.push_back(i->second->BrickId());
		}

		//pca
		i->second->GetPca().Compute();
		lens = i->second->GetPca().GetLengths();

		stream << ids.front() << "\t";
		if (m_bn > 1)
			stream << brick_ids.front() << "\t";
		fluo::Point center = i->second->GetCenter(scale);
		stream << center.x() << "\t";
		stream << center.y() << "\t";
		stream << center.z() << "\t";
		stream << i->second->GetSizeUi() << "\t";
		stream << i->second->GetSizeD(scalarscale) << "\t";
		stream << size_scale * i->second->GetSizeUi() << "\t";
		stream << i->second->GetSizeD(size_scale * scalarscale) << "\t";
		stream << i->second->GetExtUi() << "\t";
		stream << i->second->GetExtD(scalarscale) << "\t";
		stream << i->second->GetMean(maxscale) << "\t";
		stream << i->second->GetStd(maxscale) << "\t";
		stream << i->second->GetMin(maxscale) << "\t";
		stream << i->second->GetMax(maxscale) << "\t";
		stream << i->second->GetDistp() << "\t";
		stream << lens.x();
		if (m_colocal)
		{
			stream << "\t";
			for (size_t ii = 0; ii<m_vd_list.size(); ++ii)
				stream << i->second->GetCoSizeUi(ii) << "\t" << i->second->GetCoSizeD(ii) << "\t";
		}
		stream << "\n";
	}
}

void ComponentAnalyzer::OutputCompListStr(std::string &str, int verbose, const std::string& comp_header)
{
	std::ostringstream oss;
	OutputCompListStream(oss, verbose, comp_header);
	str = oss.str();
}

void ComponentAnalyzer::OutputCompListFile(const std::wstring &filename, int verbose, const std::string& comp_header)
{
	std::ofstream ofs;
	ofs.open(filename, std::ofstream::out);
	OutputCompListStream(ofs, verbose, comp_header);
	ofs.close();
}

unsigned int ComponentAnalyzer::GetExt(unsigned int* data_label,
	unsigned long long index,
	unsigned int id,
	const fluo::Vector& size,
	const fluo::Point& p)
{
	if (!data_label)
		return 0;
	bool surface_vox, contact_vox;
	unsigned long long indexn;
	//determine the numbers
	size_t i = p.intx();
	size_t j = p.inty();
	size_t k = p.intz();
	if (i == 0 || i == size.intx() - 1 ||
		j == 0 || j == size.inty() - 1 ||
		k == 0 || k == size.intz() - 1)
	{
		//border voxel
		surface_vox = true;
		//determine contact
		contact_vox = false;
		if (i > 0)
		{
			indexn = index - 1;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && i < size.intx() - 1)
		{
			indexn = index + 1;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && j > 0)
		{
			indexn = index - size.intx();
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && j < size.inty() - 1)
		{
			indexn = index + size.intx();
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && k > 0)
		{
			indexn = index - size.get_size_xy();
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && k < size.intz() - 1)
		{
			indexn = index + size.get_size_xy();
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
	}
	else
	{
		surface_vox = false;
		contact_vox = false;
		//i-1
		indexn = index - 1;
		if (data_label[indexn] == 0)
			surface_vox = true;
		if (data_label[indexn] &&
			data_label[indexn] != id)
			surface_vox = contact_vox = true;
		//i+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + 1;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//j-1
		if (!surface_vox || !contact_vox)
		{
			indexn = index - size.intx();
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//j+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + size.intx();
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//k-1
		if (!surface_vox || !contact_vox)
		{
			indexn = index - size.get_size_xy();
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//k+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + size.get_size_xy();
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
	}

	return surface_vox ? 1 : 0;
}

bool ComponentAnalyzer::OutputAnnotData()
{
	if (!m_compgroup)
		return false;
	auto vd = m_compgroup->vd.lock();
	if (!vd)
		return false;

	auto raw_data = vd->GetVolume(false);
	int bits = raw_data->GetBitsPerElement();
	double scale = std::pow(2.0, bits);
	auto spc = vd->GetSpacing();
	auto res = vd->GetResolution();

	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	if (comps.empty() ||
		m_compgroup->dirty)
		Analyze();

	std::wostringstream oss;
	std::wstring sinfo;
	std::wstring str;

	auto ann = std::make_shared<AnnotData>();

	int bn = vd->GetAllBrickNum();
	graph.ClearVisited();
	int count = 1;
	for (auto i = comps.begin();
		i != comps.end(); ++i)
	{
		if (bn > 1)
		{
			if (graph.Visited(i->second))
				continue;
			CelpList list;
			graph.GetLinkedComps(i->second, list, m_slimit);
		}

		oss.str(L"");
		oss << i->second->GetSizeUi() << L"\t";
		oss << double(i->second->GetSizeUi())*spc.x()*spc.y()*spc.z() << L"\t";
		oss << i->second->GetMean(scale);
		sinfo = oss.str();
		switch (m_annot_type)
		{
		case 1://id
			str = std::to_wstring(i->second->Id());
			break;
		case 2://sn
			str = std::to_wstring(count);
		}
		fluo::Point p = i->second->GetCenter(fluo::Vector(1.0) / spc);
		ann->AddText(str, p, sinfo);
		++count;
	}

	ann->SetVolume(vd);
	ann->SetTransform(vd->GetTexture()->transform());
	glbin_data_manager.AddAnnotData(ann);
	auto view = glbin_current.render_view.lock();
	if (view)
		view->AddAnnotData(ann);

	return true;
}

bool ComponentAnalyzer::OutputChannels()
{
	bool result = false;
	std::vector<std::shared_ptr<VolumeData>> channs;
	switch (m_channel_type)
	{
	case 1:
		result = OutputMultiChannels(channs);
		break;
	case 2:
		result = OutputRgbChannels(channs);
		break;
	}

	std::wstring group_name = L"";
	std::shared_ptr<VolumeGroup> group;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	
	for (auto it = channs.begin(); it != channs.end(); ++it)
	{
		if (*it)
		{
			glbin_data_manager.AddVolumeData(*it);
			if (it == channs.begin())
			{
				group_name = view->AddGroup(L"");
				group = view->GetGroup(group_name);
			}
			view->AddVolumeData(*it, group_name);
		}
	}
	if (group)
	{
		//group->SetSyncRAll(true);
		//group->SetSyncGAll(true);
		//group->SetSyncBAll(true);
		auto vd = glbin_current.vol_data.lock();
		if (vd)
		{
			fluo::Color col = vd->GetGammaColor();
			group->SetGammaAll(col);
			col = vd->GetBrightness();
			group->SetBrightnessAll(col);
			col = vd->GetHdr();
			group->SetHdrAll(col);
		}
	}
	//glbin_current.SetVolumeData(view->m_cur_vol);

	return result;
}

bool ComponentAnalyzer::OutputMultiChannels(std::vector<std::shared_ptr<VolumeData>> &channs)
{
	if (!m_compgroup)
		return false;
	auto vd = m_compgroup->vd.lock();
	if (!vd)
		return false;
	//comp list
	CelpList &comps = m_compgroup->celps;

	if (comps.empty() ||
		m_compgroup->dirty)
		Analyze();

	auto tex = vd->GetTexture();
	if (!tex)
		return false;
	auto raw_data = vd->GetVolume(false);
	if (!raw_data)
		return false;
	int bits = raw_data->GetBitsPerElement();
	void* data_ptr = raw_data->GetDataVoid();
	if (!data_ptr)
		return false;
	//get label
	auto raw_label = vd->GetLabel(false);
	if (!raw_label)
		return false;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return false;
	auto spc = vd->GetSpacing();
	auto res = vd->GetResolution();
	int brick_size = vd->GetTexture()->get_build_max_tex_size();

	unsigned int count = 1;
	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	unsigned long long index;
	unsigned int value_label;

	int bn = vd->GetAllBrickNum();

	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;
	graph.ClearVisited();
	for (auto i = comps.begin();
		i != comps.end(); ++i)
	{
		if (bn > 1)
		{
			if (graph.Visited(i->second))
				continue;
		}

		auto vdn = std::make_shared<VolumeData>();
		vdn->AddEmptyData(bits,
			res,
			spc,
			brick_size);
		vdn->SetSpcFromFile(true);
		vdn->SetName(vd->GetName() +
			L"_COMP" + std::to_wstring(count++) +
			L"_SIZE" + std::to_wstring(i->second->GetSizeUi()));

		//populate the volume
		//the actual data
		auto tex_vd = vdn->GetTexture();
		if (!tex_vd)
			continue;
		auto raw_vdn = vdn->GetVolume(false);
		if (!raw_vdn)
			continue;
		void* vdn_ptr = raw_vdn->GetDataVoid();
		if (!vdn_ptr)
			continue;

		if (bn > 1)
		{
			CelpList list;
			if (!graph.GetLinkedComps(i->second, list, m_slimit))
			{
				list.insert(std::pair<unsigned long long, Celp>
					(i->second->GetEId(), i->second));
			}
			//for comps in list
			for (auto iter = list.begin();
				iter != list.end(); ++iter)
			{
				auto b_orig = tex->get_brick(iter->second->BrickId());
				auto b_new = tex_vd->get_brick(iter->second->BrickId());
				auto res_bo = b_orig->get_size();
				auto res_bn = b_new->get_size();
				auto stride_bo = b_orig->get_stride();
				auto stride_bn = b_new->get_stride();
				int nx2 = res_bo.intx();
				int ny2 = res_bo.inty();
				int nz2 = res_bo.intz();
				unsigned int* data_label_old = b_orig->get_raw_data(flvr::CompType::Label)->DataAs<unsigned int>();
				unsigned char* data_data_old = b_orig->get_raw_data(flvr::CompType::Data)->DataAs<unsigned char>();
				unsigned char* data_data_new = b_new->get_raw_data(flvr::CompType::Data)->DataAs<unsigned char>();

				unsigned int *tp, *tp2;
				unsigned char *tp_old, *tp2_old;
				unsigned char *tp_new, *tp2_new;
				unsigned int lv;
				tp = data_label_old;
				tp_old = data_data_old;
				tp_new = data_data_new;
				for (int k = 0; k < nz2 - 1; ++k)
				{
					tp2 = tp;
					tp2_old = tp_old;
					tp2_new = tp_new;
					for (int j = 0; j < ny2 - 1; ++j)
					{
						for (int i = 0; i < nx2 - 1; ++i)
						{
							lv = tp2[i];
							if (lv == iter->second->Id())
							{
								//assign values
								if (bits == 8)
									tp2_new[i] = tp2_old[i];
								else
									((unsigned short*)tp2_new)[i] = ((unsigned short*)tp2_old)[i];
							}
						}
						tp2 += stride_bo.intx();
						if (bits == 8)
						{
							tp2_old += stride_bo.intx();
							tp2_new += stride_bn.intx();
						}
						else
						{
							tp2_old += stride_bo.intx()*2;
							tp2_new += stride_bn.intx()*2;
						}
					}
					tp += stride_bo.get_size_xy();
					if (bits == 8)
					{
						tp_old += stride_bo.get_size_xy();
						tp_new += stride_bn.get_size_xy();
					}
					else
					{
						tp_old += stride_bo.get_size_xy()*2;
						tp_new += stride_bn.get_size_xy()*2;
					}
				}
			}
		}
		else
		{
			for (index = 0; index < for_size; ++index)
			{
				value_label = label_ptr[index];
				if (value_label == i->second->Id())
				{
					if (bits == 8)
						((unsigned char*)vdn_ptr)[index] = ((unsigned char*)data_ptr)[index];
					else
						((unsigned short*)vdn_ptr)[index] = ((unsigned short*)data_ptr)[index];
				}
			}
		}

		//settings
		fluo::Color c;
		if (GetColor(i->second->Id(), i->second->BrickId(), vd, c))
		{
			glbin_vol_def.Copy(vdn, vd);
			vdn->SetColor(c);
			channs.push_back(vdn);
		}
	}
	return true;
}

bool ComponentAnalyzer::OutputRgbChannels(std::vector<std::shared_ptr<VolumeData>> &channs)
{
	if (!m_compgroup)
		return false;
	auto vd = m_compgroup->vd.lock();
	if (!vd)
		return false;
	//comp list
	CelpList &comps = m_compgroup->celps;

	if (comps.empty() ||
		m_compgroup->dirty)
		Analyze();

	auto tex = vd->GetTexture();
	if (!tex)
		return false;
	auto raw_data = vd->GetVolume(false);
	if (!raw_data)
		return false;
	int bits = raw_data->GetBitsPerElement();
	void* data_ptr = raw_data->GetDataVoid();
	if (!data_ptr)
		return false;
	//get label
	auto raw_label = vd->GetLabel(false);
	if (!raw_label)
		return false;
	unsigned int* label_ptr = raw_label->DataAs<unsigned int>();
	if (!label_ptr)
		return false;
	auto spc = vd->GetSpacing();
	auto res = vd->GetResolution();
	int brick_size = vd->GetTexture()->get_build_max_tex_size();

	//red volume
	auto vd_r = std::make_shared<VolumeData>();
	vd_r->AddEmptyData(8,
		res,
		spc,
		brick_size);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(vd->GetName() + L"_CH_R");
	//green volume
	auto vd_g = std::make_shared<VolumeData>();
	vd_g->AddEmptyData(8,
		res,
		spc,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(vd->GetName() + L"_CH_G");
	//blue volume
	auto vd_b = std::make_shared<VolumeData>();
	vd_b->AddEmptyData(8,
		res,
		spc,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(vd->GetName() + L"_CH_B");

	//get new data
	//red volume
	auto raw_vd_r = vd_r->GetVolume(false);
	if (!raw_vd_r)
		return false;
	void* data_vd_r_ptr = raw_vd_r->GetDataVoid();
	if (!data_vd_r_ptr)
		return false;
	//green volume
	auto raw_vd_g = vd_g->GetVolume(false);
	if (!raw_vd_g)
		return false;
	void* data_vd_g_ptr = raw_vd_g->GetDataVoid();
	if (!data_vd_g_ptr)
		return false;
	//blue volume
	auto raw_vd_b = vd_b->GetVolume(false);
	if (!raw_vd_b)
		return false;
	void* data_vd_b_ptr = raw_vd_b->GetDataVoid();
	if (!data_vd_b_ptr)
		return false;

	unsigned long long for_size = (unsigned long long)res.get_size_xyz();
	unsigned long long index;
	unsigned int value_label;
	fluo::Color color;
	double max_value = vd->GetMaxValue();
	for (index = 0; index < for_size; ++index)
	{
		value_label = label_ptr[index];
		if (GetColor(value_label, tex->get_brick_id(index), vd, color))
		{
			//assign colors
			double value;//0-255
			if (bits == 8)
				value = ((unsigned char*)data_ptr)[index];
			else
				value = ((unsigned short*)data_ptr)[index] * 255.0 / max_value;
			((unsigned char*)data_vd_r_ptr)[index] = (unsigned char)(color.r()*value);
			((unsigned char*)data_vd_g_ptr)[index] = (unsigned char)(color.g()*value);
			((unsigned char*)data_vd_b_ptr)[index] = (unsigned char)(color.b()*value);
		}
	}

	glbin_vol_def.Copy(vd_r, vd);
	glbin_vol_def.Copy(vd_g, vd);
	glbin_vol_def.Copy(vd_b, vd);

	fluo::Color red(1.0, 0.0, 0.0);
	fluo::Color green(0.0, 1.0, 0.0);
	fluo::Color blue(0.0, 0.0, 1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);

	channs.push_back(vd_r);
	channs.push_back(vd_g);
	channs.push_back(vd_b);

	return true;
}

void ComponentAnalyzer::OutputDistance(std::ostream& stream)
{
	size_t num = GetDistMatSize();
	if (!num)
		return;

	size_t gsize = m_comp_groups.size();

	//result
	std::string str;
	std::vector<std::vector<double>> rm;//result matrix
	std::vector<std::string> nl;//name list
	std::vector<int> gn;//group number
	rm.reserve(num);
	nl.reserve(num);
	if (gsize > 1)
		gn.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	//compute
	fluo::Vector scale;
	std::vector<fluo::Point> pos;
	pos.reserve(num);
	int num2 = 0;//actual number
	if (m_use_dist_allchan && gsize > 1)
	{
		for (size_t i = 0; i < gsize; ++i)
		{
			flrd::CellGraph& graph = m_comp_groups[i].graph;
			flrd::CelpList* list = &(m_comp_groups[i].celps);
			scale = list->scale;
			if (m_bn > 1)
				graph.ClearVisited();

			for (auto it = list->begin();
				it != list->end(); ++it)
			{
				if (m_bn > 1)
				{
					if (graph.Visited(it->second))
						continue;
					flrd::CelpList links;
					graph.GetLinkedComps(it->second, links, m_slimit);
				}

				pos.push_back(it->second->GetCenter(scale));
				str = std::to_string(i + 1);
				str += ":";
				str += std::to_string(it->second->Id());
				nl.push_back(str);
				gn.push_back(static_cast<int>(i));
				num2++;
			}
		}
	}
	else
	{
		if (!m_compgroup)
			return;
		flrd::CellGraph& graph = m_comp_groups[0].graph;
		flrd::CelpList* list = &(m_compgroup->celps);
		scale = list->scale;
		if (m_bn > 1)
			graph.ClearVisited();

		for (auto it = list->begin();
			it != list->end(); ++it)
		{
			if (m_bn > 1)
			{
				if (graph.Visited(it->second))
					continue;
				flrd::CelpList links;
				graph.GetLinkedComps(it->second, links, m_slimit);
			}

			pos.push_back(it->second->GetCenter(scale));
			str = std::to_string(it->second->Id());
			nl.push_back(str);
			num2++;
		}
	}
	double dist = 0;
	for (int i = 0; i < num2; ++i)
	{
		for (int j = i; j < num2; ++j)
		{
			dist = (pos[i] - pos[j]).length();
			rm[i][j] = dist;
			rm[j][i] = dist;
		}
	}

	bool bdist = m_use_dist_neighbor &&
		m_dist_neighbor_num > 0 &&
		m_dist_neighbor_num < num2 - 1;

	std::vector<double> in_group;//distances with in a group
	std::vector<double> out_group;//distance between groups
	in_group.reserve(num2 * num2 / 2);
	out_group.reserve(num2 * num2 / 2);
	std::vector<std::vector<int>> im;//index matrix
	if (bdist)
	{
		//sort with indices
		im.reserve(num2);
		for (size_t i = 0; i < num2; ++i)
		{
			im.push_back(std::vector<int>());
			im[i].reserve(num2);
			for (size_t j = 0; j < num2; ++j)
				im[i].push_back(static_cast<int>(j));
		}
		//copy rm
		std::vector<std::vector<double>> rm2 = rm;
		//sort
		for (size_t i = 0; i < num2; ++i)
		{
			std::sort(im[i].begin(), im[i].end(),
				[&](int ii, int jj) {return rm2[i][ii] < rm2[i][jj]; });
		}
		//fill rm
		for (size_t i = 0; i < num2; ++i)
			for (size_t j = 0; j < num2; ++j)
			{
				rm[i][j] = rm2[i][im[i][j]];
				if (gsize > 1 && j > 0 &&
					j <= m_dist_neighbor_num)
				{
					if (gn[i] == gn[im[i][j]])
						in_group.push_back(rm[i][j]);
					else
						out_group.push_back(rm[i][j]);
				}
			}
	}
	else
	{
		if (gsize > 1)
		{
			for (int i = 0; i < num2; ++i)
				for (int j = i + 1; j < num2; ++j)
				{
					if (gn[i] == gn[j])
						in_group.push_back(rm[i][j]);
					else
						out_group.push_back(rm[i][j]);
				}
		}
	}

	size_t dnum = bdist ? (m_dist_neighbor_num + 1) : num2;
	for (size_t i = 0; i < num2; ++i)
	{
		stream << nl[i] << "\t";
		for (size_t j = bdist ? 1 : 0; j < dnum; ++j)
		{
			stream << rm[i][j];
			if (j < dnum - 1)
				stream << "\t";
		}
		stream << "\n";
	}
	//index matrix
	if (bdist)
	{
		stream << "\n";
		for (size_t i = 0; i < num2; ++i)
		{
			stream << nl[i] << "\t";
			for (size_t j = bdist ? 1 : 0; j < dnum; ++j)
			{
				stream << im[i][j] + 1;
				if (j < dnum - 1)
					stream << "\t";
			}
			stream << "\n";
		}
	}
	//output in_group and out_group distances
	if (gsize > 1)
	{
		stream << "\nIn group Distances\t";
		for (size_t i = 0; i < in_group.size(); ++i)
		{
			stream << in_group[i];
			if (i < in_group.size() - 1)
				stream << "\t";
		}
		stream << "\n";
		stream << "Out group Distances\t";
		for (size_t i = 0; i < out_group.size(); ++i)
		{
			stream << out_group[i];
			if (i < out_group.size() - 1)
				stream << "\t";
		}
		stream << "\n";
	}
}

size_t ComponentAnalyzer::GetDistMatSize()
{
	size_t gsize = m_comp_groups.size();
	if (m_use_dist_allchan && gsize > 1)
	{
		size_t matsize = 0;
		for (int i = 0; i < gsize; ++i)
		{
			matsize += m_comp_groups[i].celps.size();
		}
		return matsize;
	}
	else
	{
		if (!m_compgroup)
			return 0;
		return m_compgroup->celps.size();
	}
}

bool ComponentAnalyzer::GetRulerListFromCelp(RulerList& rulerlist)
{
	if (!m_compgroup)
		return false;

	CelpList* list = &(m_compgroup->celps);
	if (!list || list->size() < 3)
		return false;

	auto scale = list->scale;
	auto ruler = std::make_shared<Ruler>();
	ruler->SetRulerMode(RulerMode::Polyline);
	fluo::Point pt;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		pt = it->second->GetCenter(scale);
		ruler->AddPoint(pt);
	}
	ruler->SetFinished();

	rulerlist.Add(ruler);

	return true;
}

void ComponentAnalyzer::SetSelectedIds(const std::vector<unsigned int>& ids,
	const std::vector<unsigned int>& bids)
{
	m_sel_ids = ids;
	m_sel_bids = bids;
}

bool ComponentAnalyzer::GetSelectedCelp(CelpList& cl, bool links)
{
	if (!m_compgroup)
		return false;
	CelpList* list = &(m_compgroup->celps);
	if (!list || list->empty())
		return false;

	cl.min = list->min;
	cl.max = list->max;
	cl.scale = list->scale;

	int bn = glbin_comp_analyzer.GetBrickNum();
	for (size_t i = 0; i < m_sel_ids.size(); ++i)
	{
		unsigned long long key = m_sel_ids[i];
		unsigned int bid = 0;
		if (bn > 1)
		{
			key = m_sel_bids[i];
			key = (key << 32) | m_sel_ids[i];
			bid = m_sel_bids[i];
		}
		auto it = list->find(key);
		if (it != list->end())
			FindCelps(cl, it, links);
	}

	if (cl.empty())
		return false;
	return true;
}

bool ComponentAnalyzer::GetAllCelp(CelpList& cl, bool links)
{
	if (!m_compgroup)
		return false;
	CelpList* list = &(m_compgroup->celps);
	if (!list || list->empty())
		return false;

	cl.min = list->min;
	cl.max = list->max;
	cl.scale = list->scale;

	for (auto it = list->begin(); it != list->end(); ++it)
		FindCelps(cl, it, links);

	if (cl.empty())
		return false;
	return true;
}

bool ComponentAnalyzer::GetCelpFromIds(CelpList& cl, const std::vector<unsigned long long>& ids, bool links)
{
	if (!m_compgroup)
		return false;
	CelpList* list = &(m_compgroup->celps);
	if (!list || list->empty())
		return false;

	cl.min = list->min;
	cl.max = list->max;
	cl.scale = list->scale;

	for (size_t i = 0; i < ids.size(); ++i)
	{
		unsigned long long key = ids[i];
		auto it = list->find(key);
		if (it != list->end())
			FindCelps(cl, it, links);
	}

	if (cl.empty())
		return false;
	return true;
}

bool ComponentAnalyzer::GetColor(
	unsigned int id,
	int brick_id,
	const std::shared_ptr<VolumeData>& vd,
	fluo::Color &color)
{
	if (!m_compgroup)
		return false;
	if (!id)
		return false;
	//comp list
	CelpList &comps = m_compgroup->celps;

	switch (m_color_type)
	{
	case 1:
	default:
		color = fluo::Color(id, vd->GetShuffle());
		return true;
	case 2:
		if (vd)
		{
			unsigned int size = 0;
			CelpListIter iter;
			//search in comp list
			if (brick_id < 0)
			{
				int bn = vd->GetAllBrickNum();
				for (int i=0; i<bn; ++i)
				{
					iter = comps.find(Cell::GetKey(id, i));
					if (iter != comps.end())
						break;
				}
			}
			else
			{
				iter = comps.find(Cell::GetKey(id, brick_id));
			}
			if (iter == comps.end())
				return false;
			size = iter->second->GetSizeUi();
			double value;
			if (comps.min == comps.max)
				value = 1.0;
			else
				value = double(size - comps.min) /
				double(comps.max - comps.min);
			color = vd->GetColorFromColormap(value);
			return true;
		}
		break;
	}
	return false;
}

//replace id to make color consistent
void ComponentAnalyzer::ReplaceId(unsigned int base_id, Celp &info)
{
	if (!m_compgroup)
		return;
	auto vd = m_compgroup->vd.lock();
	if (!vd || !vd->GetTexture())
		return;
	auto tex = vd->GetTexture();

	unsigned int brick_id = info->BrickId();
	unsigned int rep_id = info->Id();
	unsigned int new_id = base_id;

	//get brick label data
	auto b = tex->get_brick(brick_id);
	auto res = b->get_size();
	auto stride = b->get_stride();
	unsigned int* data_label = b->get_raw_data(flvr::CompType::Label)->DataAs<unsigned int>();

	//get nonconflict id
	new_id = GetNonconflictId(new_id, res, b, data_label);

	//assign new id
	unsigned int *tp, *tp2;
	unsigned int lv;
	tp = data_label;
	unsigned int kz = res.intz() > 1 ? res.intz() - 1 : 1;
	for (unsigned int k = 0; k < kz; ++k)
	{
		tp2 = tp;
		for (int j = 0; j < res.inty() - 1; ++j)
		{
			for (int i = 0; i < res.intx() - 1; ++i)
			{
				lv = tp2[i];
				if (lv == rep_id)
					tp2[i] = new_id;
			}
			tp2 += stride.intx();
		}
		tp += stride.get_size_xy();
	}

	//info->alt_id = new_id;
	info->SetId(new_id);
}

unsigned int ComponentAnalyzer::GetNonconflictId(
	unsigned int id,
	const fluo::Vector& size,
	const std::shared_ptr<flvr::TextureBrick>& b,
	unsigned int* data)
{
	unsigned int result = 0;
	unsigned int iid = id;
	unsigned int kz = size.intz() > 1 ? size.intz() - 1 : 1;
	unsigned int *tp, *tp2;
	unsigned int lv;
	auto stride = b->get_stride();
	do
	{
		bool found = false;
		tp = data;
		for (unsigned int k = 0; k < kz; ++k)
		{
			tp2 = tp;
			for (int j = 0; j < size.inty() - 1; ++j)
			{
				for (int i = 0; i < size.intx() - 1; ++i)
				{
					lv = tp2[i];
					if (lv == iid)
					{
						found = true;
						iid += 253;
						break;
					}
				}
				if (found) break;
				tp2 += stride.intx();
			}
			if (found) break;
			tp += stride.get_size_xy();
		}

		if (!found)
		{
			result = iid;
			break;
		}
	} while (id != iid);

	return result;
}

CompGroup* ComponentAnalyzer::FindCompGroup(const std::shared_ptr<VolumeData>& vd)
{
	for (size_t i = 0; i < m_comp_groups.size(); ++i)
	{
		if (m_comp_groups[i].vd.lock() == vd)
			return &(m_comp_groups[i]);
	}
	return 0;
}

CompGroup* ComponentAnalyzer::AddCompGroup(const std::shared_ptr<VolumeData>& vd)
{
	if (!vd)
		return 0;
	m_comp_groups.push_back(CompGroup());
	CompGroup *compgroup = &(m_comp_groups.back());
	compgroup->vd = vd;
	return compgroup;
}

void ComponentAnalyzer::FindCelps(CelpList& list,
	CelpListIter& it, bool links)
{
	list.insert(std::pair<unsigned long long, flrd::Celp>
		(it->second->GetEId(), it->second));

	if (links)
	{
		auto graph = glbin_comp_analyzer.GetCellGraph();
		graph->get().ClearVisited();
		flrd::CelpList links;
		if (graph->get().GetLinkedComps(it->second, links,
			glbin_comp_analyzer.GetSizeLimit()))
		{
			for (auto it2 = links.begin();
				it2 != links.end(); ++it2)
			{
				list.insert(std::pair<unsigned long long, flrd::Celp>
					(it2->second->GetEId(), it2->second));
			}
		}
	}
}

