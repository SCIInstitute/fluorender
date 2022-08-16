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
#include "CompAnalyzer.h"
#include "VolumeData.hpp"
#include "Annotations.hpp"
#include "Global.hpp"
#include "VolumeFactory.hpp"
#include "Pca.h"
#include "TextureBrick.h"
#include "Texture.h"
#include "VolumeRenderer.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>

using namespace flrd;

ComponentAnalyzer::ComponentAnalyzer(fluo::VolumeData* vd)
	: m_analyzed(false),
	m_colocal(false),
	m_bn(0),
	m_slimit(5)
{
	m_compgroup = AddCompGroup(vd);
}

ComponentAnalyzer::~ComponentAnalyzer()
{
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

	for (size_t i=0; i<m_vd_list.size(); ++i)
	{
		if (!m_vd_list[i])
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		flvr::Texture* tex = m_vd_list[i]->GetTexture();
		if (!tex)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		flvr::TextureBrick* b = tex->get_brick(bid);
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

void ComponentAnalyzer::Analyze(bool sel, bool consistent, bool colocal)
{
	if (!m_compgroup)
		return;
	fluo::VolumeData* vd = m_compgroup->vd;
	if (!vd || !vd->GetTexture())
		return;
	double sx, sy, sz;
	vd->getValue(gstSpcX, sx);
	vd->getValue(gstSpcY, sy);
	vd->getValue(gstSpcZ, sz);
	std::vector<flvr::TextureBrick*> *bricks =
		vd->GetTexture()->get_bricks();
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

	vd->GetRenderer()->return_label();
	if (sel)
		vd->GetRenderer()->return_mask();

	int bits;
	for (size_t bi = 0; bi < bn; ++bi)
	{
		void* data_data = 0;
		unsigned char* data_mask = 0;
		unsigned int* data_label = 0;
		long nx, ny, nz;
		flvr::TextureBrick* b = (*bricks)[bi];
		int c = 0;
		int nb = 1;
		if (bn > 1)
		{
			if (sel && !b->is_mask_valid())
				continue;
			// get brick if ther are more than one brick
			nb = b->nb(0);
			bits = nb==1? nrrdTypeUChar: nrrdTypeUShort;
			nx = b->nx()-1;
			ny = b->ny()-1;
			nz = b->nz()-1;
			int cnt = (nx ? 1 : 0) + (ny ? 1 : 0) + (nz ? 1 : 0);
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
				for (unsigned int j = 0; j < ny; ++j)
				{
					memcpy(tempp, tp2, nx*nb);
					tempp += nx*nb;
					tp2 += b->sx()*nb;
				}
				tp += b->sx()*b->sy()*nb;
			}
			data_data = (void*)temp;
			//mask
			if (sel)
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
					for (unsigned int j = 0; j < ny; ++j)
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
				for (unsigned int j = 0; j < ny; ++j)
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
			vd->getValue(gstResX, nx);
			vd->getValue(gstResY, ny);
			vd->getValue(gstResZ, nz);
			Nrrd* nrrd_data = vd->GetData(false);
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
			if (!data_data || (sel && !data_mask) || !data_label)
				return;
		}

		unsigned int id = 0;
		unsigned int brick_id = b->get_id();
		double value;
		double scale;
		double delta;
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
			value = 0.0;
			if (sel)
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

			k = index / (nx*ny);
			j = index % (nx*ny);
			i = j % nx;
			j = j / nx;
			ext = GetExt(data_label, index, id, nx?nx:1, ny?ny:1, nz?nz:1, i, j, k);
			//ext = 0.0;

			//colocalization
			std::vector<unsigned int> sumi;
			std::vector<double> sumd;
			if (colocal) GetColocalization(brick_id, i, j, k, sumi, sumd);

			//find in list
			iter = comp_list_brick.find(id);
			if (iter == comp_list_brick.end())
			{
				//not found
				Cell* info = 0;
				if (colocal)
					info = new Cell(id, brick_id,
						1, value, ext,
						i + b->ox(), j + b->oy(), k + b->oz(),
						sx, sy, sz,
						sumi, sumd);
				else
					info = new Cell(id, brick_id,
						1, value, ext,
						i+b->ox(), j+b->oy(), k+b->oz(),
						sx, sy, sz);
				comp_list_brick.insert(std::pair<unsigned int, Celp>
					(id, Celp(info)));
			}
			else
			{
				if (colocal)
					iter->second->Inc(1, value, ext,
						i + b->ox(), j + b->oy(), k + b->oz(),
						sx, sy, sz,
						sumi, sumd);
				else
					iter->second->Inc(1, value, ext,
						i + b->ox(), j + b->oy(), k + b->oz(),
						sx, sy, sz);
			}
		}

		unsigned int size_limit;
		for (iter = comp_list_brick.begin();
			iter != comp_list_brick.end(); ++iter)
		{
			if (bn > 1)
				size_limit = m_slimit;
			else
				size_limit = 0;
			if (iter->second->GetSize() < size_limit)
				continue;
			//iter->second->var = sqrt(iter->second->m2 / (iter->second->sumi));
			//iter->second->mean *= scale;
			//iter->second->min *= scale;
			//iter->second->max *= scale;
			//iter->second->Calc();
			comps.min = iter->second->GetSizeD() <
				comps.min ? iter->second->GetSizeD() :
				comps.min;
			comps.max = iter->second->GetSizeD() >
				comps.max ? iter->second->GetSizeD() :
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

		m_sig_progress();
	}

	MatchBricks(sel);
	UpdateMaxCompSize(colocal);
	if (consistent)
		MakeColorConsistent();

	m_compgroup->dirty = false;
	m_colocal = colocal && m_vd_list.size();
	if (!sel)
		m_analyzed = true;
}

void ComponentAnalyzer::MatchBricks(bool sel)
{
	if (!m_compgroup)
		return;
	fluo::VolumeData* vd = m_compgroup->vd;
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
		for (unsigned int k = 0; k < nz; ++k)
		{
			tp2 = tp;
			for (unsigned int j = 0; j < ny; ++j)
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
			for (unsigned int j = 0; j < ny - 1; ++j)
			for (unsigned int i = 0; i < nx - 1; ++i)
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
			for (unsigned int k = 0; k < kz; ++k)
			for (unsigned int i = 0; i < nx - 1; ++i)
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
			for (unsigned int k = 0; k < kz; ++k)
			for (unsigned int j = 0; j < ny - 1; ++j)
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

		m_sig_progress();
	}
}

void ComponentAnalyzer::UpdateMaxCompSize(bool colocal)
{
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
	fluo::VolumeData* vd = m_compgroup->vd;
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

	m_sig_progress();
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
	fluo::VolumeData* vd = m_compgroup->vd;
	if (!vd || ! vd->GetTexture())
		return;

	//get label data
	Nrrd* nrrd_label = vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
	int ix = (int)(p.x() + 0.5);
	int iy = (int)(p.y() + 0.5);
	int iz = (int)(p.z() + 0.5);
	if (ix < 0 || ix >= nx ||
		iy < 0 || iy >= ny ||
		iz < 0 || iz >= nz)
		return;
	unsigned long long index = (unsigned long long)nx * ny * iz +
		(unsigned long long)nx * iy + ix;
	unsigned int id = data_label[index];
	if (!id)
		return;
	long bn;
	vd->getValue(gstBrickNum, bn);

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
	fluo::VolumeData* vd = m_compgroup->vd;
	if (!vd)
		return;

	str = "ID\t";
	if (vd && vd->GetAllBrickNum() > 1)
		str += "BRICK_ID\t";

	str += "PosX\tPosY\tPosZ\tSumN\tSumI\tPhysN\tPhysI\tSurfN\tSurfI\tMean\tSigma\tMin\tMax\tDist\tPcaL";

	if (m_colocal)
	{
		for (size_t i = 0; i < m_vd_list.size(); ++i)
			str += "\t" + std::string(
				m_vd_list[i]->getName()) + "\t";
	}
	str += "\n";
}

void ComponentAnalyzer::OutputCompListStream(std::ostream &stream, int verbose, std::string comp_header)
{
	if (!m_compgroup)
		return;
	fluo::VolumeData* vd = m_compgroup->vd;
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
	double maxscale;
	vd->getValue(gstMaxScale, maxscale);
	double intscale;
	vd->getValue(gstIntScale, intscale);
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
		stream << i->second->GetSizeD(intscale) << "\t";
		stream << size_scale * i->second->GetSizeUi() << "\t";
		stream << i->second->GetSizeD(size_scale * intscale) << "\t";
		stream << i->second->GetExtUi() << "\t";
		stream << i->second->GetExtD(intscale) << "\t";
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

void ComponentAnalyzer::OutputCompListStr(std::string &str, int verbose, std::string comp_header)
{
	std::ostringstream oss;
	OutputCompListStream(oss, verbose, comp_header);
	str = oss.str();
}

void ComponentAnalyzer::OutputCompListFile(std::string &filename, int verbose, std::string comp_header)
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

bool ComponentAnalyzer::GenAnnotations(fluo::Annotations *ann, bool consistent, int type)
{
	if (!m_compgroup)
		return false;
	fluo::VolumeData* vd = m_compgroup->vd;
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
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);

	//comp list
	CelpList &comps = m_compgroup->celps;
	//graph for linking multiple bricks
	CellGraph &graph = m_compgroup->graph;

	if (comps.empty() ||
		m_compgroup->dirty)
		Analyze(true, consistent);

	std::string sinfo;
	std::ostringstream oss;
	std::string str;

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

		oss.str("");
		oss << i->second->GetSizeUi() << "\t";
		oss << double(i->second->GetSizeUi())*spcx*spcy*spcz << "\t";
		oss << i->second->GetMean(scale);
		sinfo = oss.str();
		switch (type)
		{
		case 0://id
			str = std::to_string(i->second->Id());
			break;
		case 1://sn
			str = std::to_string(count);
		}
		fluo::Point p = i->second->GetCenter(1.0 / nx, 1.0 / ny, 1.0 / nz);
		ann->addText(p, str, sinfo);
		++count;
	}
	return true;
}

bool ComponentAnalyzer::GenMultiChannels(std::list<fluo::VolumeData*>& channs, int color_type, bool consistent)
{
	if (!m_compgroup)
		return false;
	fluo::VolumeData* vd = m_compgroup->vd;
	if (!vd)
		return false;
	//comp list
	CelpList &comps = m_compgroup->celps;

	if (comps.empty() ||
		m_compgroup->dirty)
		Analyze(true, consistent);

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
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
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
	std::string name;
	for (auto i = comps.begin();
		i != comps.end(); ++i)
	{
		if (bn > 1)
		{
			if (graph.Visited(i->second))
				continue;
		}

		fluo::VolumeData* vdn = glbin_volf->build(vd);
		vdn->AddEmptyData(bits,
			nx, ny, nz,
			spcx, spcy, spcz,
			brick_size);
		vdn->setValue(gstSpcFromFile, true);
		name = vd->getName();
		name += "_COMP" + std::to_string(count++);
		name += "_SIZE" + std::to_string(i->second->GetSizeUi());
		vdn->setName(name);

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
				for (unsigned int k = 0; k < nz2 - 1; ++k)
				{
					tp2 = tp;
					tp2_old = tp_old;
					tp2_new = tp_new;
					for (unsigned int j = 0; j < ny2 - 1; ++j)
					{
						for (unsigned int i = 0; i < nx2 - 1; ++i)
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
		if (GetColor(i->second->Id(), i->second->BrickId(), vd, color_type, c))
		{
			vdn->setValue(gstColor, c);
			channs.push_back(vdn);
		}
		else
			glbin_volf->remove(vdn);
	}
	return true;
}

bool ComponentAnalyzer::GenRgbChannels(std::list<fluo::VolumeData*> &channs, int color_type, bool consistent)
{
	if (!m_compgroup)
		return false;
	fluo::VolumeData* vd = m_compgroup->vd;
	if (!vd)
		return false;
	//comp list
	CelpList &comps = m_compgroup->celps;

	if (comps.empty() ||
		m_compgroup->dirty)
		Analyze(true, consistent);

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
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
	int brick_size = vd->GetTexture()->get_build_max_tex_size();

	//red volume
	fluo::VolumeData* vd_r = glbin_volf->build(vd);
	vd_r->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_r->setValue(gstSpcFromFile, true);
	vd_r->setName(std::string(vd->getName()) + "_CH_R");
	//green volume
	fluo::VolumeData* vd_g = glbin_volf->build(vd);
	vd_g->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_g->setValue(gstSpcFromFile, true);
	vd_g->setName(std::string(vd->getName()) + "_CH_G");
	//blue volume
	fluo::VolumeData* vd_b = glbin_volf->build(vd);
	vd_b->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_b->setValue(gstSpcFromFile, true);
	vd_b->setName(std::string(vd->getName()) + "_CH_B");

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
	double max_value;
	vd->getValue(gstMaxInt, max_value);
	for (index = 0; index < for_size; ++index)
	{
		value_label = data_label[index];
		if (GetColor(value_label, tex->get_brick_id(index), vd, color_type, color))
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

	fluo::Color red(1.0, 0.0, 0.0);
	fluo::Color green(0.0, 1.0, 0.0);
	fluo::Color blue(0.0, 0.0, 1.0);
	vd_r->setValue(gstColor, red);
	vd_g->setValue(gstColor, green);
	vd_b->setValue(gstColor, blue);

	channs.push_back(vd_r);
	channs.push_back(vd_g);
	channs.push_back(vd_b);

	return true;
}

bool ComponentAnalyzer::GetColor(
	unsigned int id, int brick_id,
	fluo::VolumeData* vd, int color_type,
	fluo::Color &color)
{
	if (!id)
		return false;
	//comp list
	CelpList &comps = m_compgroup->celps;

	switch (color_type)
	{
	case 1:
	default:
		{
			int shuffle = vd->GetShuffle();
			color = fluo::Color(id, shuffle);
		}
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
				for (unsigned int i=0; i<bn; ++i)
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
	fluo::VolumeData* vd = m_compgroup->vd;
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
		for (unsigned int j = 0; j < ny - 1; ++j)
		{
			for (unsigned int i = 0; i < nx - 1; ++i)
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
			for (unsigned int j = 0; j < ny - 1; ++j)
			{
				for (unsigned int i = 0; i < nx - 1; ++i)
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
