/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

CelpList* ComponentAnalyzer::GetCelpList()
{
	if (m_compgroup)
		return &(m_compgroup->celps);
	return 0;
}

CellGraph* ComponentAnalyzer::GetCellGraph()
{
	if (m_compgroup)
		return &(m_compgroup->graph);
	return 0;
}

int ComponentAnalyzer::GetCompGroupSize()
{
	return static_cast<int>(m_comp_groups.size());
}

CompGroup* ComponentAnalyzer::GetCompGroup(int i)
{
	if (i >= 0 && i < m_comp_groups.size())
		return &(m_comp_groups[i]);
	return 0;
}

int ComponentAnalyzer::GetBrickNum()
{
	return m_bn;
}

int ComponentAnalyzer::GetColocalization(
	size_t bid,
	unsigned int bi,
	unsigned int bj,
	unsigned int bk,
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
		flvr::Texture* tex = vd->GetTexture();
		if (!tex)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		flvr::TextureBrick* b = tex->get_brick(static_cast<unsigned int>(bid));
		if (!b)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		double value = b->get_data(bi, bj, bk);
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

	double sx, sy, sz;
	vd->GetSpacings(sx, sy, sz);
	std::vector<flvr::TextureBrick*> *bricks = vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;
	size_t bn = bricks->size();

	//comp list
	CelpList &comps = m_compgroup->celps;
	//clear list and start calculating
	comps.clear();
	comps.min = std::numeric_limits<unsigned int>::max();
	comps.max = 0;
	comps.sx = sx;
	comps.sy = sy;
	comps.sz = sz;
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
	int bits;
	for (size_t bi = 0; bi < bn; ++bi)
	{
		void* data_data = 0;
		unsigned char* data_mask = 0;
		unsigned int* data_label = 0;
		int nx, ny, nz;
		flvr::TextureBrick* b = (*bricks)[bi];
		int c = 0;
		int nb = 1;
		if (bn > 1)
		{
			if (use_sel && !b->is_mask_valid())
				continue;
			// get brick if ther are more than one brick
			nb = b->nb(0);
			bits = nb==1? nrrdTypeUChar: nrrdTypeUShort;
			nx = b->nx()-1;
			ny = b->ny()-1;
			nz = b->nz()-1;
			int cnt = (nx>0 ? 1 : 0) + (ny>0 ? 1 : 0) + (nz>0 ? 1 : 0);
			if (cnt < 2) continue;

			//size
			unsigned long long mem_size;
			mem_size = nz ? ((unsigned long long)nx*
				(unsigned long long)ny*(unsigned long long)nz*nb) :
				((unsigned long long)nx*(unsigned long long)ny*nb);
			//data
			unsigned char* temp = new unsigned char[mem_size];
			unsigned char* tempp = temp;
			unsigned char* tp = (unsigned char*)(b->tex_data(0));
			unsigned char* tp2;
			unsigned int kn = nz ? nz : 1;
			for (unsigned int k = 0; k < kn; ++k)
			{
				tp2 = tp;
				for (int j = 0; j < ny; ++j)
				{
					memcpy(tempp, tp2, nx*nb);
					tempp += nx*nb;
					tp2 += b->sx()*nb;
				}
				tp += b->sx()*b->sy()*nb;
			}
			data_data = (void*)temp;
			//mask
			if (use_sel)
			{
				c = b->nmask();
				nb = b->nb(c);
				mem_size = nz ? ((unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz*nb) :
					((unsigned long long)nx*(unsigned long long)ny*nb);
				temp = new unsigned char[mem_size];
				tempp = temp;
				tp = (unsigned char*)(b->tex_data(c));
				for (unsigned int k = 0; k < kn; ++k)
				{
					tp2 = tp;
					for (int j = 0; j < ny; ++j)
					{
						memcpy(tempp, tp2, nx*nb);
						tempp += nx*nb;
						tp2 += b->sx()*nb;
					}
					tp += b->sx()*b->sy()*nb;
				}
				data_mask = temp;
			}
			//label
			c = b->nlabel();
			nb = b->nb(c);
			mem_size = nz ? ((unsigned long long)nx*
				(unsigned long long)ny*(unsigned long long)nz*nb) :
				((unsigned long long)nx*(unsigned long long)ny*nb);
			temp = new unsigned char[mem_size];
			tempp = temp;
			tp = (unsigned char*)(b->tex_data(c));
			for (unsigned int k = 0; k < kn; ++k)
			{
				tp2 = tp;
				for (int j = 0; j < ny; ++j)
				{
					memcpy(tempp, tp2, nx*nb);
					tempp += nx*nb;
					tp2 += b->sx()*nb;
				}
				tp += b->sx()*b->sy()*nb;
			}
			data_label = (unsigned int*)temp;
		}
		else
		{
			// get data if there is only one brick
			vd->GetResolution(nx, ny, nz);
			Nrrd* nrrd_data = vd->GetVolume(false);
			if (nrrd_data)
			{
				bits = nrrd_data->type;
				data_data = nrrd_data->data;
			}
			Nrrd* nrrd_mask = vd->GetMask(false);
			if (nrrd_mask)
				data_mask = (unsigned char*)(nrrd_mask->data);
			Nrrd* nrrd_label = vd->GetLabel(false);
			if (nrrd_label)
				data_label = (unsigned int*)(nrrd_label->data);
			if (!data_data || (use_sel && !data_mask) || !data_label)
				return;
		}

		prog += 80 * (0.2 + double(bi) / bn);
		SetProgress(static_cast<int>(prog), "Analyzing components.");

		unsigned int id = 0;
		unsigned int brick_id = b->get_id();
		double value;
		double scale;
		double ext;
		int i, j, k;
		//m_vd->GetResolution(nx, ny, nz);
		unsigned long long for_size = nz ? ((unsigned long long)nx *
			(unsigned long long)ny * (unsigned long long)nz) :
			((unsigned long long)nx * (unsigned long long)ny);
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
				if (data_mask && !data_mask[index])
					continue;
			}
			if (data_label && !data_label[index])
				continue;

			if (data_label)
				id = data_label[index];

			if (bits == nrrdTypeUChar)
			{
				value = ((unsigned char*)data_data)[index] / 255.0;
				scale = 255.0;
			}
			else if (bits == nrrdTypeUShort)
			{
				value = ((unsigned short*)data_data)[index] / 65535.0;
				scale = 65535.0;
			}

			if (value <= 0.0)
				continue;

			k = static_cast<int>(index / (nx*ny));
			j = static_cast<int>(index % (nx*ny));
			i = j % nx;
			j = j / nx;
			ext = GetExt(data_label, index, id, nx?nx:1, ny?ny:1, nz?nz:1, i, j, k);
			//ext = 0.0;

			//colocalization
			std::vector<unsigned int> sumi;
			std::vector<double> sumd;
			if (m_colocal) GetColocalization(brick_id, i, j, k, sumi, sumd);

			//find in list
			iter = comp_list_brick.find(id);
			if (iter == comp_list_brick.end())
			{
				//not found
				Cell* info = 0;
				if (m_colocal)
					info = new Cell(id, brick_id,
						1, value, static_cast<unsigned int>(std::round(ext)),
						i + b->ox(), j + b->oy(), k + b->oz(),
						sx, sy, sz,
						sumi, sumd);
				else
					info = new Cell(id, brick_id,
						1, value, static_cast<unsigned int>(std::round(ext)),
						i+b->ox(), j+b->oy(), k+b->oz(),
						sx, sy, sz);
				comp_list_brick.insert(std::pair<unsigned int, Celp>
					(id, Celp(info)));
			}
			else
			{
				if (m_colocal)
					iter->second->Inc(1, value, static_cast<unsigned int>(std::round(ext)),
						i + b->ox(), j + b->oy(), k + b->oz(),
						sx, sy, sz,
						sumi, sumd);
				else
					iter->second->Inc(1, value, static_cast<unsigned int>(std::round(ext)),
						i + b->ox(), j + b->oy(), k + b->oz(),
						sx, sy, sz);
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
			if (data_data) delete[] (unsigned char*)data_data;
			if (data_mask) delete[] (unsigned char*)data_mask;
			if (data_label) delete[] (unsigned int*)data_label;
		}
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
	flvr::Texture* tex = vd->GetTexture();
	std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks_id();
	if (!bricks || bricks->size() <= 1)
		return;
	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	int bits;
	size_t bn = bricks->size();
	for (size_t bi = 0; bi < bn; ++bi)
	{
		//get one brick
		flvr::TextureBrick* b = (*bricks)[bi];
		if (sel && !b->is_mask_valid())
			continue;
		void* data_data = 0;
		unsigned char* data_mask = 0;
		unsigned int* data_label = 0;
		int nx, ny, nz;
		int c = 0;
		int nb = 1;
		nb = b->nb(0);
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
		bits = nb * 8;
		//label
		c = b->nlabel();
		nb = b->nb(c);
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz*nb;
		unsigned char* temp = new unsigned char[mem_size];
		unsigned char* tempp = temp;
		unsigned char* tp = (unsigned char*)(b->tex_data(c));
		unsigned char* tp2;
		for (int k = 0; k < nz; ++k)
		{
			tp2 = tp;
			for (int j = 0; j < ny; ++j)
			{
				memcpy(tempp, tp2, nx*nb);
				tempp += nx*nb;
				tp2 += b->sx()*nb;
			}
			tp += b->sx()*b->sy()*nb;
		}
		data_label = (unsigned int*)temp;
		//check 3 positive faces
		unsigned int l1, l2, index, b1, b2;
		//(x, y, nz-1)
		if (nz > 1)
		{
			for (int j = 0; j < ny - 1; ++j)
			for (int i = 0; i < nx - 1; ++i)
			{
				index = nx * ny*(nz - 1) + j * nx + i;
				l1 = data_label[index];
				if (!l1) continue;
				index -= nx * ny;
				l2 = data_label[index];
				if (!l2) continue;
				//get brick ids
				b2 = b->get_id();
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
		int kz = nz > 1 ? nz - 1 : 1;
		if (ny > 1)
		{
			for (int k = 0; k < kz; ++k)
			for (int i = 0; i < nx - 1; ++i)
			{
				index = nx * ny*k + nx * (ny - 1) + i;
				l1 = data_label[index];
				if (!l1) continue;
				index -= nx;
				l2 = data_label[index];
				if (!l2) continue;
				//get brick ids
				b2 = b->get_id();
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
		if (nx > 1)
		{
			for (int k = 0; k < kz; ++k)
			for (int j = 0; j < ny - 1; ++j)
			{
				index = nx * ny*k + nx * j + nx - 1;
				l1 = data_label[index];
				if (!l1) continue;
				index -= 1;
				l2 = data_label[index];
				if (!l2) continue;
				//get brick ids
				b2 = b->get_id();
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
	flvr::Texture* tex = vd->GetTexture();
	std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
	if (!bricks || bricks->size() <= 1)
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

	flrd::CelpList* list = GetCelpList();
	if (!list || list->empty())
		return;

	for (auto it = list->begin();
		it != list->end(); ++it)
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
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	m_size = m_vox * spcx * spcy * spcz;
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
	Nrrd* nrrd_label = vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	int ix = static_cast<int>(std::round(p.x()));
	int iy = static_cast<int>(std::round(p.y()));
	int iz = static_cast<int>(std::round(p.z()));
	if (ix < 0 || ix >= nx ||
		iy < 0 || iy >= ny ||
		iz < 0 || iz >= nz)
		return;
	unsigned long long index = (unsigned long long)nx * ny * iz +
		(unsigned long long)nx * iy + ix;
	unsigned int id = data_label[index];
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

	double sx = comps.sx;
	double sy = comps.sy;
	double sz = comps.sz;
	double size_scale = sx * sy * sz;
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
		fluo::Point center = i->second->GetCenter(sx, sy, sz);
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
	int nx, int ny, int nz,
	int i, int j, int k)
{
	if (!data_label)
		return 0;
	bool surface_vox, contact_vox;
	unsigned long long indexn;
	//determine the numbers
	if (i == 0 || i == nx - 1 ||
		j == 0 || j == ny - 1 ||
		k == 0 || k == nz - 1)
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
		if (!contact_vox && i < nx - 1)
		{
			indexn = index + 1;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && j > 0)
		{
			indexn = index - nx;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && j < ny - 1)
		{
			indexn = index + nx;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && k > 0)
		{
			indexn = index - nx*ny;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && k < nz - 1)
		{
			indexn = index + nx*ny;
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
			indexn = index - nx;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//j+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + nx;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//k-1
		if (!surface_vox || !contact_vox)
		{
			indexn = index - nx*ny;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//k+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + nx*ny;
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

	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return false;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return false;
	int bits = nrrd_data->type;
	double scale;
	if (bits == nrrdTypeUChar)
		scale = 255.0;
	else if (bits == nrrdTypeUShort)
		scale = 65535.0;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);

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
		oss << double(i->second->GetSizeUi())*spcx*spcy*spcz << L"\t";
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
		fluo::Point p = i->second->GetCenter(1.0 / nx, 1.0 / ny, 1.0 / nz);
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

	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return false;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return false;
	int bits = 8;
	if (nrrd_data->type == nrrdTypeUChar)
		bits = 8;
	else if (nrrd_data->type == nrrdTypeUShort)
		bits = 16;
	void* data_data = nrrd_data->data;
	if (!data_data)
		return false;
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return false;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return false;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	int brick_size = vd->GetTexture()->get_build_max_tex_size();

	unsigned int count = 1;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
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
			nx, ny, nz,
			spcx, spcy, spcz,
			brick_size);
		vdn->SetSpcFromFile(true);
		vdn->SetName(vd->GetName() +
			L"_COMP" + std::to_wstring(count++) +
			L"_SIZE" + std::to_wstring(i->second->GetSizeUi()));

		//populate the volume
		//the actual data
		flvr::Texture* tex_vd = vdn->GetTexture();
		if (!tex_vd) continue;
		Nrrd* nrrd_vd = tex_vd->get_nrrd(0);
		if (!nrrd_vd) continue;
		void* data_vd = nrrd_vd->data;
		if (!data_vd) continue;

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
				flvr::TextureBrick* b_orig = tex->get_brick(iter->second->BrickId());
				flvr::TextureBrick* b_new = tex_vd->get_brick(iter->second->BrickId());
				int nx2 = b_orig->nx();
				int ny2 = b_orig->ny();
				int nz2 = b_orig->nz();
				int c = b_orig->nlabel();
				unsigned int* data_label_old = (unsigned int*)(b_orig->tex_data(c));
				unsigned char* data_data_old = (unsigned char*)(b_orig->tex_data(0));
				unsigned char* data_data_new = (unsigned char*)(b_new->tex_data(0));

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
						tp2 += b_orig->sx();
						if (bits == 8)
						{
							tp2_old += b_orig->sx();
							tp2_new += b_new->sx();
						}
						else
						{
							tp2_old += b_orig->sx()*2;
							tp2_new += b_new->sx()*2;
						}
					}
					tp += b_orig->sx()*b_orig->sy();
					if (bits == 8)
					{
						tp_old += b_orig->sx()*b_orig->sy();
						tp_new += b_new->sx()*b_new->sy();
					}
					else
					{
						tp_old += b_orig->sx()*b_orig->sy()*2;
						tp_new += b_new->sx()*b_new->sy()*2;
					}
				}
			}
		}
		else
		{
			for (index = 0; index < for_size; ++index)
			{
				value_label = data_label[index];
				if (value_label == i->second->Id())
				{
					if (bits == 8)
						((unsigned char*)data_vd)[index] = ((unsigned char*)data_data)[index];
					else
						((unsigned short*)data_vd)[index] = ((unsigned short*)data_data)[index];
				}
			}
		}

		//settings
		fluo::Color c;
		if (GetColor(i->second->Id(), i->second->BrickId(), vd.get(), c))
		{
			glbin_vol_def.Copy(vdn.get(), vd.get());
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

	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return false;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return false;
	int bits = 8;
	if (nrrd_data->type == nrrdTypeUChar)
		bits = 8;
	else if (nrrd_data->type == nrrdTypeUShort)
		bits = 16;
	void* data_data = nrrd_data->data;
	if (!data_data)
		return false;
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return false;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return false;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	int brick_size = vd->GetTexture()->get_build_max_tex_size();

	//red volume
	auto vd_r = std::make_shared<VolumeData>();
	vd_r->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(vd->GetName() + L"_CH_R");
	//green volume
	auto vd_g = std::make_shared<VolumeData>();
	vd_g->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(vd->GetName() + L"_CH_G");
	//blue volume
	auto vd_b = std::make_shared<VolumeData>();
	vd_b->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(vd->GetName() + L"_CH_B");

	//get new data
	//red volume
	flvr::Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return false;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return false;
	void* data_vd_r = nrrd_vd_r->data;
	if (!data_vd_r) return false;
	//green volume
	flvr::Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return false;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return false;
	void* data_vd_g = nrrd_vd_g->data;
	if (!data_vd_g) return false;
	//blue volume
	flvr::Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return false;
	Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
	if (!nrrd_vd_b) return false;
	void* data_vd_b = nrrd_vd_b->data;
	if (!data_vd_b) return false;

	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned int value_label;
	fluo::Color color;
	double max_value = vd->GetMaxValue();
	for (index = 0; index < for_size; ++index)
	{
		value_label = data_label[index];
		if (GetColor(value_label, tex->get_brick_id(index), vd.get(), color))
		{
			//assign colors
			double value;//0-255
			if (bits == 8)
				value = ((unsigned char*)data_data)[index];
			else
				value = ((unsigned short*)data_data)[index] * 255.0 / max_value;
			((unsigned char*)data_vd_r)[index] = (unsigned char)(color.r()*value);
			((unsigned char*)data_vd_g)[index] = (unsigned char)(color.g()*value);
			((unsigned char*)data_vd_b)[index] = (unsigned char)(color.b()*value);
		}
	}

	glbin_vol_def.Copy(vd_r.get(), vd.get());
	glbin_vol_def.Copy(vd_g.get(), vd.get());
	glbin_vol_def.Copy(vd_b.get(), vd.get());

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
	double sx, sy, sz;
	std::vector<fluo::Point> pos;
	pos.reserve(num);
	int num2 = 0;//actual number
	if (m_use_dist_allchan && gsize > 1)
	{
		for (size_t i = 0; i < gsize; ++i)
		{
			flrd::CellGraph& graph = m_comp_groups[i].graph;
			flrd::CelpList* list = &(m_comp_groups[i].celps);
			sx = list->sx;
			sy = list->sy;
			sz = list->sz;
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

				pos.push_back(it->second->GetCenter(sx, sy, sz));
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
		sx = list->sx;
		sy = list->sy;
		sz = list->sz;
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

			pos.push_back(it->second->GetCenter(sx, sy, sz));
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

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	Ruler ruler;
	ruler.SetRulerMode(RulerMode::Polyline);
	fluo::Point pt;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		pt = it->second->GetCenter(sx, sy, sz);
		ruler.AddPoint(pt);
	}
	ruler.SetFinished();

	rulerlist.push_back(&ruler);

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
	cl.sx = list->sx;
	cl.sy = list->sy;
	cl.sz = list->sz;

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
	cl.sx = list->sx;
	cl.sy = list->sy;
	cl.sz = list->sz;

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
	cl.sx = list->sx;
	cl.sy = list->sy;
	cl.sz = list->sz;

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
	VolumeData* vd,
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
	flvr::Texture* tex = vd->GetTexture();

	unsigned int brick_id = info->BrickId();
	unsigned int rep_id = info->Id();
	unsigned int new_id = base_id;

	//get brick label data
	flvr::TextureBrick* b = tex->get_brick(brick_id);
	int nx = b->nx();
	int ny = b->ny();
	int nz = b->nz();
	int c = b->nlabel();
	unsigned int* data_label = (unsigned int*)(b->tex_data(c));

	//get nonconflict id
	new_id = GetNonconflictId(new_id, nx, ny, nz, b, data_label);

	//assign new id
	unsigned int *tp, *tp2;
	unsigned int lv;
	tp = data_label;
	unsigned int kz = nz > 1 ? nz - 1 : 1;
	for (unsigned int k = 0; k < kz; ++k)
	{
		tp2 = tp;
		for (int j = 0; j < ny - 1; ++j)
		{
			for (int i = 0; i < nx - 1; ++i)
			{
				lv = tp2[i];
				if (lv == rep_id)
					tp2[i] = new_id;
			}
			tp2 += b->sx();
		}
		tp += b->sx()*b->sy();
	}

	//info->alt_id = new_id;
	info->SetId(new_id);
}

unsigned int ComponentAnalyzer::GetNonconflictId(
	unsigned int id,
	int nx, int ny, int nz,
	flvr::TextureBrick* b,
	unsigned int* data)
{
	unsigned int result = 0;
	unsigned int iid = id;
	unsigned int kz = nz > 1 ? nz - 1 : 1;
	unsigned int *tp, *tp2;
	unsigned int lv;
	do
	{
		bool found = false;
		tp = data;
		for (unsigned int k = 0; k < kz; ++k)
		{
			tp2 = tp;
			for (int j = 0; j < ny - 1; ++j)
			{
				for (int i = 0; i < nx - 1; ++i)
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
				tp2 += b->sx();
			}
			if (found) break;
			tp += b->sx()*b->sy();
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
		flrd::CellGraph* graph = glbin_comp_analyzer.GetCellGraph();
		graph->ClearVisited();
		flrd::CelpList links;
		if (graph->GetLinkedComps(it->second, links,
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

