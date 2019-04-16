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
#include "CompAnalyzer.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/filtered_graph.hpp>

using namespace FL;

ComponentAnalyzer::ComponentAnalyzer(VolumeData* vd)
	: m_analyzed(false),
	m_vd(vd),
	m_colocal(false),
	m_comp_list_dirty(true)
{
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
		Texture* tex = m_vd_list[i]->GetTexture();
		if (!tex)
		{
			sumi.push_back(0);
			sumd.push_back(0.0);
			continue;
		}
		TextureBrick* b = tex->get_brick(bid);
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
	if (!m_vd || !m_vd->GetTexture())
		return;
	vector<TextureBrick*> *bricks = m_vd->GetTexture()->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;

	size_t bn = bricks->size();
	//clear list and start calculating
	m_comp_list.clear();
	m_comp_list.min = std::numeric_limits<unsigned int>::max();
	m_comp_list.max = 0;
	m_comp_graph.clear();
	m_analyzed = false;

	if (sel)
		m_vd->GetVR()->return_mask();

	int bits;
	for (size_t bi = 0; bi < bn; ++bi)
	{
		void* data_data = 0;
		unsigned char* data_mask = 0;
		unsigned int* data_label = 0;
		int nx, ny, nz;
		TextureBrick* b = (*bricks)[bi];
		int c = 0;
		int nb = 1;
		if (bn > 1)
		{
			// get brick if ther are more than one brick
			nb = b->nb(0);
			bits = nb==1? nrrdTypeUChar: nrrdTypeUShort;
			nx = b->nx()-1;
			ny = b->ny()-1;
			nz = b->nz()-1;
			if (!nx || !ny || !nz) continue;

			//size
			unsigned long long mem_size = (unsigned long long)nx*
				(unsigned long long)ny*(unsigned long long)nz*nb;
			//data
			unsigned char* temp = new unsigned char[mem_size];
			unsigned char* tempp = temp;
			unsigned char* tp = (unsigned char*)(b->tex_data(0));
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
			data_data = (void*)temp;
			//mask
			if (sel)
			{
				c = b->nmask();
				nb = b->nb(c);
				mem_size = (unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz*nb;
				temp = new unsigned char[mem_size];
				tempp = temp;
				tp = (unsigned char*)(b->tex_data(c));
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
				data_mask = temp;
			}
			//label
			c = b->nlabel();
			nb = b->nb(c);
			mem_size = (unsigned long long)nx*
				(unsigned long long)ny*(unsigned long long)nz*nb;
			temp = new unsigned char[mem_size];
			tempp = temp;
			tp = (unsigned char*)(b->tex_data(c));
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
		}
		else
		{
			// get data if there is only one brick
			m_vd->GetResolution(nx, ny, nz);
			Nrrd* nrrd_data = m_vd->GetVolume(false);
			if (nrrd_data)
			{
				bits = nrrd_data->type;
				data_data = nrrd_data->data;
			}
			Nrrd* nrrd_mask = m_vd->GetMask(false);
			if (nrrd_mask)
				data_mask = (unsigned char*)(nrrd_mask->data);
			Nrrd* nrrd_label = m_vd->GetLabel(false);
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
		unsigned long long for_size = (unsigned long long)nx *
			(unsigned long long)ny * (unsigned long long)nz;
		unsigned long long index;
		CompListBrick comp_list_brick;
		CompListBrickIter iter;
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
			ext = GetExt(data_label, index, id, nx, ny, nz, i, j, k);

			//colocalization
			std::vector<unsigned int> sumi;
			std::vector<double> sumd;
			if (colocal) GetColocalization(brick_id, i, j, k, sumi, sumd);

			//find in list
			iter = comp_list_brick.find(id);
			if (iter == comp_list_brick.end())
			{
				//not found
				CompInfo* info = new CompInfo;
				info->id = id;
				info->alt_id = 0;//unused
				info->brick_id = brick_id;
				info->sumi = 1;
				info->sumd = value;
				info->ext_sumi = ext;
				info->ext_sumd = value * ext;
				info->mean = 0.0;
				info->var = 0.0;
				info->m2 = 0.0;
				delta = value - info->mean;
				info->mean += delta / info->sumi;
				info->m2 += delta * (value - info->mean);
				info->min = value;
				info->max = value;
				info->dist = 0.0;
				info->pos = FLIVR::Point(i+b->ox(), j+b->oy(), k+b->oz());
				if (colocal)
				{
					info->cosumi = sumi;
					info->cosumd = sumd;
				}
				comp_list_brick.insert(pair<unsigned int, pCompInfo>
					(id, pCompInfo(info)));
			}
			else
			{
				iter->second->pos = FLIVR::Point((iter->second->pos * iter->second->sumi +
					FLIVR::Point(i + b->ox(), j + b->oy(), k + b->oz())) / (iter->second->sumi + 1));
				//
				iter->second->sumi++;
				iter->second->sumd += value;
				iter->second->ext_sumi += ext;
				iter->second->ext_sumd += value * ext;
				//
				delta = value - iter->second->mean;
				iter->second->mean += delta / iter->second->sumi;
				iter->second->m2 += delta * (value - iter->second->mean);
				iter->second->min = value < iter->second->min ? value : iter->second->min;
				iter->second->max = value > iter->second->max ? value : iter->second->max;
				//
				if (colocal)
				{
					for (size_t i = 0; i < iter->second->cosumi.size(); ++i)
						iter->second->cosumi[i] += sumi[i];
					for (size_t i = 0; i < iter->second->cosumd.size(); ++i)
						iter->second->cosumd[i] += sumd[i];
				}
			}
		}

		unsigned int size_limit;
		for (iter = comp_list_brick.begin();
			iter != comp_list_brick.end(); ++iter)
		{
			if (bn > 1)
				size_limit = SIZE_LIMIT;
			else
				size_limit = 2;
			if (iter->second->sumi < size_limit)
				continue;
			iter->second->var = sqrt(iter->second->m2 / (iter->second->sumi));
			iter->second->mean *= scale;
			iter->second->min *= scale;
			iter->second->max *= scale;
			m_comp_list.min = iter->second->sumi <
				m_comp_list.min ? iter->second->sumi :
				m_comp_list.min;
			m_comp_list.max = iter->second->sumi >
				m_comp_list.max ? iter->second->sumi :
				m_comp_list.max;
			m_comp_list.insert(std::pair<unsigned long long, pCompInfo>
				(GetKey(iter->second->id, iter->second->brick_id), iter->second));
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

	m_comp_list_dirty = false;
	m_colocal = colocal && m_vd_list.size();
	if (!sel)
		m_analyzed = true;
}

void ComponentAnalyzer::MatchBricks(bool sel)
{
	if (!m_vd || !m_vd->GetTexture())
		return;
	Texture* tex = m_vd->GetTexture();
	vector<TextureBrick*> *bricks = tex->get_bricks();
	if (!bricks || bricks->size() <= 1)
		return;

	size_t bn = bricks->size();

	int bits;
	for (size_t bi = 0; bi < bn; ++bi)
	{
		void* data_data = 0;
		unsigned char* data_mask = 0;
		unsigned int* data_label = 0;
		int nx, ny, nz;
		int c = 0;
		int nb = 1;
		//get one brick
		TextureBrick* b = (*bricks)[bi];
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
		for (unsigned int j = 0; j < ny-1; ++j)
		for (unsigned int i = 0; i < nx-1; ++i)
		{
			index = nx*ny*(nz - 1) + j*nx + i;
			l1 = data_label[index];
			if (!l1) continue;
			index -= nx*ny;
			l2 = data_label[index];
			if (!l2) continue;
			//get brick ids
			b2 = b->get_id();
			b1 = tex->poszid(b2);
			if (b1 == b2) continue;
			//if l1 and l2 are different and already in the comp list
			//connect them
			auto i1 = m_comp_list.find(GetKey(l1, b1));
			auto i2 = m_comp_list.find(GetKey(l2, b2));
			if (i1 != m_comp_list.end() && i2 != m_comp_list.end())
				m_comp_graph.LinkComps(i1->second, i2->second);
		}
		//(x, ny-1, z)
		for (unsigned int k = 0; k < nz-1; ++k)
		for (unsigned int i = 0; i < nx-1; ++i)
		{
			index = nx*ny*k + nx*(ny-1) + i;
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
			auto i1 = m_comp_list.find(GetKey(l1, b1));
			auto i2 = m_comp_list.find(GetKey(l2, b2));
			if (i1 != m_comp_list.end() && i2 != m_comp_list.end())
				m_comp_graph.LinkComps(i1->second, i2->second);
		}
		//(nx-1, y, z)
		for (unsigned int k = 0; k < nz-1; ++k)
		for (unsigned int j = 0; j < ny-1; ++j)
		{
			index = nx*ny*k + nx*j + nx-1;
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
			auto i1 = m_comp_list.find(GetKey(l1, b1));
			auto i2 = m_comp_list.find(GetKey(l2, b2));
			if (i1 != m_comp_list.end() && i2 != m_comp_list.end())
				m_comp_graph.LinkComps(i1->second, i2->second);
		}

		if (data_data) delete[] (unsigned char*)data_data;
		if (data_mask) delete[] (unsigned char*)data_mask;
		if (data_label) delete[] (unsigned int*)data_label;

		m_sig_progress();
	}
}

void ComponentAnalyzer::UpdateMaxCompSize(bool colocal)
{
	unsigned int sumi;
	double sumd;
	unsigned int ext_sumi;
	double ext_sumd;
	double mean;
	double var;
	double min;
	double max;
	FLIVR::Point pos;
	std::vector<unsigned int> cosumi;
	std::vector<double> cosumd;

	m_comp_graph.ClearVisited();
	std::pair<CompVertexIter, CompVertexIter> vertices =
		boost::vertices(m_comp_graph);
	for (auto iter = vertices.first; iter != vertices.second; ++iter)
	{
		if (m_comp_graph[*iter].visited)
			continue;
		CompList list;
		pCompInfo info = m_comp_graph[*iter].compinfo.lock();
		if (m_comp_graph.GetLinkedComps(info, list, SIZE_LIMIT))
		{
			sumi = 0;
			sumd = 0.0;
			ext_sumi = 0;
			ext_sumd = 0.0;
			mean = 0.0;
			var = 0.0;
			min = std::numeric_limits<double>::max();
			max = 0.0;
			if (colocal)
			{
				cosumi.resize(m_vd_list.size(), 0);
				cosumd.resize(m_vd_list.size(), 0.0);
			}

			for (auto li = list.begin();
				li != list.end(); ++li)
			{
				//mean
				double temp = (mean * sumi + li->second->mean * li->second->sumi) /
					(sumi + li->second->sumi);
				//var
				var = ((var + mean * mean) * sumi +
					(li->second->var + li->second->mean * li->second->mean) * li->second->sumi) /
					(sumi + li->second->sumi) - temp * temp;
				//pos
				pos = FLIVR::Point((pos * sumi + li->second->pos * li->second->sumi) /
					(sumi + li->second->sumi));
				mean = temp;
				//others
				sumi += li->second->sumi;
				sumd += li->second->sumd;
				ext_sumi += li->second->ext_sumi;
				ext_sumd += li->second->ext_sumd;
				min = li->second->min < min ? li->second->min : min;
				max = li->second->max > max ? li->second->max : max;
				//colocalization
				if (colocal)
				{
					for (size_t i = 0; i<m_vd_list.size(); ++i)
					{
						cosumi[i] += li->second->cosumi[i];
						cosumd[i] += li->second->cosumd[i];
					}
				}
			}

			//update in comp list
			if (sumi > m_comp_list.max)
				m_comp_list.max = sumi;

			//update each
			for (auto li = list.begin();
				li != list.end(); ++li)
			{
				li->second->sumi = sumi;
				li->second->sumd = sumd;
				li->second->ext_sumi = ext_sumi;
				li->second->ext_sumd = ext_sumd;
				li->second->mean = mean;
				li->second->var = var;
				li->second->min = min;
				li->second->max = max;
				li->second->pos = pos;
				if (colocal)
				{
					li->second->cosumi = cosumi;
					li->second->cosumd = cosumd;
				}
			}
		}
	}
}

void ComponentAnalyzer::MakeColorConsistent()
{
	if (!m_vd || !m_vd->GetTexture())
		return;
	Texture* tex = m_vd->GetTexture();
	vector<TextureBrick*> *bricks = tex->get_bricks();
	if (!bricks || bricks->size() <= 1)
		return;

	m_comp_graph.ClearVisited();
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		if (m_comp_graph.Visited(i->second))
			continue;
		CompList list;
		if (m_comp_graph.GetLinkedComps(i->second, list, SIZE_LIMIT))
		{
			//make color consistent
			//get the id of the first item in the list
			//which is also the base color
			unsigned int base_id = list.begin()->second->id;
			//for other items
			for (auto iter = std::next(list.begin());
				iter != list.end(); ++iter)
			{
				unsigned int link_id = iter->second->id;
				if ((link_id - base_id) % 360)
				{
					//color is different
					//generate new_id
					ReplaceId(base_id, iter->second);
					//insert one with consistent key
					if (link_id != iter->second->id)
						m_comp_list.insert(std::pair<unsigned long long, pCompInfo>
						(GetKey(iter->second->id, iter->second->brick_id), iter->second));
				}
			}
		}
	}

	//remove comps with inconsistent keys
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end();)
	{
		if (i->first != GetKey(i->second->id, i->second->brick_id))
			i = m_comp_list.erase(i);
		else
			++i;
	}

	m_sig_progress();
}

size_t ComponentAnalyzer::GetListSize()
{
	return m_comp_list.size();
}

size_t ComponentAnalyzer::GetCompSize()
{
	size_t list_size = m_comp_list.size();
	size_t graph_size = boost::num_vertices(m_comp_graph);
	size_t cc_size = 0;
	if (graph_size)
	{
		m_comp_graph.ClearVisited();
		std::pair<CompVertexIter, CompVertexIter> vertices =
			boost::vertices(m_comp_graph);
		for (auto iter = vertices.first; iter != vertices.second; ++iter)
		{
			if (m_comp_graph[*iter].visited)
				continue;
			CompList list;
			pCompInfo info = m_comp_graph[*iter].compinfo.lock();
			m_comp_graph.GetLinkedComps(info, list, SIZE_LIMIT);
			cc_size++;
		}
	}

	return list_size - graph_size + cc_size;
}

void ComponentAnalyzer::OutputFormHeader(std::string &str)
{
	str = "";
	if (m_vd && m_vd->GetAllBrickNum() > 1)
		str = "BRICK_ID\t";

	str += "ID\tPosX\tPosY\tPosZ\tSumN\tSumI\tSurfaceN\tSurfaceI\tMean\tSigma\tMin\tMax\tDist";

	if (m_colocal)
	{
		for (size_t i = 0; i < m_vd_list.size(); ++i)
			str += "\t" + m_vd_list[i]->GetName() + "\t";
	}
	str += "\n";
}

void ComponentAnalyzer::OutputCompListStream(std::ostream &stream, int verbose, std::string comp_header)
{
	int bn = m_vd->GetAllBrickNum();

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

	m_comp_graph.ClearVisited();
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		if (comp_header != "")
		{
			if (i == m_comp_list.begin())
				stream << comp_header << "\t";
			else
				stream << "\t";
		}

		std::list<unsigned int> ids;
		std::list<unsigned int> brick_ids;
		bool added = false;

		if (bn > 1)
		{
			if (m_comp_graph.Visited(i->second))
				continue;

			CompList list;
			if (m_comp_graph.GetLinkedComps(i->second, list, SIZE_LIMIT))
			{
				for (auto iter = list.begin();
					iter != list.end(); ++iter)
				{
					ids.push_back(iter->second->id);
					brick_ids.push_back(iter->second->brick_id);
				}
				added = true;
			}
		}
		if (!added)
		{
			ids.push_back(i->second->id);
			brick_ids.push_back(i->second->brick_id);
		}

		if (bn > 1)
			stream << brick_ids.front() << "\t";
		stream << ids.front() << "\t";
		stream << i->second->pos.x() << "\t";
		stream << i->second->pos.y() << "\t";
		stream << i->second->pos.z() << "\t";
		stream << i->second->sumi << "\t";
		stream << i->second->sumd << "\t";
		stream << i->second->ext_sumi << "\t";
		stream << i->second->ext_sumd << "\t";
		stream << i->second->mean << "\t";
		stream << i->second->var << "\t";
		stream << i->second->min << "\t";
		stream << i->second->max << "\t";
		stream << i->second->dist;
		if (m_colocal)
		{
			stream << "\t";
			for (size_t ii = 0; ii<m_vd_list.size(); ++ii)
			{
				stream << i->second->cosumi[ii] << "\t" << i->second->cosumd[ii] << "\t";
			}
		}
		stream << "\n";
	}
}

void ComponentAnalyzer::OutputCompListStr(std::string &str, int verbose, std::string comp_header)
{
	ostringstream oss;
	OutputCompListStream(oss, verbose, comp_header);
	str = oss.str();
}

void ComponentAnalyzer::OutputCompListFile(std::string &filename, int verbose, std::string comp_header)
{
	ofstream ofs;
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

bool ComponentAnalyzer::GenAnnotations(Annotations &ann, bool consistent)
{
	if (!m_vd)
		return false;

	Texture* tex = m_vd->GetTexture();
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
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);

	if (m_comp_list.empty() ||
		m_comp_list_dirty)
		Analyze(true, consistent);

	std::string sinfo;
	ostringstream oss;

	int bn = m_vd->GetAllBrickNum();
	m_comp_graph.ClearVisited();
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		if (bn > 1)
		{
			if (m_comp_graph.Visited(i->second))
				continue;
			CompList list;
			m_comp_graph.GetLinkedComps(i->second, list, SIZE_LIMIT);
		}

		oss.str("");
		oss << i->second->sumi << "\t";
		oss << double(i->second->sumi)*spcx*spcy*spcz << "\t";
		oss << i->second->mean;
		sinfo = oss.str();
		ann.AddText(std::to_string(i->second->id),
			FLIVR::Point(i->second->pos.x()/nx,
			i->second->pos.y()/ny, i->second->pos.z()/nz),
			sinfo);
	}
	return true;
}

bool ComponentAnalyzer::GenMultiChannels(std::list<VolumeData*>& channs, int color_type, bool consistent)
{
	if (!m_vd)
		return false;

	if (m_comp_list.empty() ||
		m_comp_list_dirty)
		Analyze(true, consistent);

	Texture* tex = m_vd->GetTexture();
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
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);
	int brick_size = m_vd->GetTexture()->get_build_max_tex_size();

	unsigned int count = 1;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned int value_label;

	int bn = m_vd->GetAllBrickNum();
	m_comp_graph.ClearVisited();
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		if (bn > 1)
		{
			if (m_comp_graph.Visited(i->second))
				continue;
		}

		VolumeData* vd = new VolumeData();
		vd->AddEmptyData(bits,
			nx, ny, nz,
			spcx, spcy, spcz,
			brick_size);
		vd->SetSpcFromFile(true);
		vd->SetName(m_vd->GetName() +
			wxString::Format("_COMP%d_SIZE%d", count++, i->second->sumi));

		//populate the volume
		//the actual data
		Texture* tex_vd = vd->GetTexture();
		if (!tex_vd) continue;
		Nrrd* nrrd_vd = tex_vd->get_nrrd(0);
		if (!nrrd_vd) continue;
		void* data_vd = nrrd_vd->data;
		if (!data_vd) continue;

		if (bn > 1)
		{
			CompList list;
			if (!m_comp_graph.GetLinkedComps(i->second, list, SIZE_LIMIT))
			{
				list.insert(std::pair<unsigned long long, pCompInfo>
					(GetKey(i->second->id, i->second->brick_id), i->second));
			}
			//for comps in list
			for (auto iter = list.begin();
				iter != list.end(); ++iter)
			{
				TextureBrick* b_orig = tex->get_brick(iter->second->brick_id);
				TextureBrick* b_new = tex_vd->get_brick(iter->second->brick_id);
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
							if (lv == (iter->second->alt_id?
								iter->second->alt_id:
								iter->second->id))
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
				if (value_label == i->second->id)
				{
					if (bits == 8)
						((unsigned char*)data_vd)[index] = ((unsigned char*)data_data)[index];
					else
						((unsigned short*)data_vd)[index] = ((unsigned short*)data_data)[index];
				}
			}
		}

		//settings
		FLIVR::Color c;
		if (GetColor(i->second->id, i->second->brick_id, m_vd, color_type, c))
		{
			vd->SetColor(c);
			vd->SetEnableAlpha(m_vd->GetEnableAlpha());
			vd->SetShading(m_vd->GetShading());
			vd->SetShadow(false);
			//other settings
			vd->Set3DGamma(m_vd->Get3DGamma());
			vd->SetBoundary(m_vd->GetBoundary());
			vd->SetOffset(m_vd->GetOffset());
			vd->SetLeftThresh(m_vd->GetLeftThresh());
			vd->SetRightThresh(m_vd->GetRightThresh());
			vd->SetAlpha(m_vd->GetAlpha());
			vd->SetSampleRate(m_vd->GetSampleRate());
			vd->SetMaterial(amb, diff, spec, shine);

			channs.push_back(vd);
		}
		else
			delete vd;
	}
	return true;
}

bool ComponentAnalyzer::GenRgbChannels(std::list<VolumeData*> &channs, int color_type, bool consistent)
{
	if (!m_vd)
		return false;

	if (m_comp_list.empty() ||
		m_comp_list_dirty)
		Analyze(true, consistent);

	Texture* tex = m_vd->GetTexture();
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
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);
	int brick_size = m_vd->GetTexture()->get_build_max_tex_size();

	//red volume
	VolumeData* vd_r = new VolumeData();
	vd_r->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(m_vd->GetName() +
		wxString::Format("_CH_R"));
	//green volume
	VolumeData* vd_g = new VolumeData();
	vd_g->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_vd->GetName() +
		wxString::Format("_CH_G"));
	//blue volume
	VolumeData* vd_b = new VolumeData();
	vd_b->AddEmptyData(8,
		nx, ny, nz,
		spcx, spcy, spcz,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_vd->GetName() +
		wxString::Format("_CH_B"));

	//get new data
	//red volume
	Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return false;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return false;
	void* data_vd_r = nrrd_vd_r->data;
	if (!data_vd_r) return false;
	//green volume
	Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return false;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return false;
	void* data_vd_g = nrrd_vd_g->data;
	if (!data_vd_g) return false;
	//blue volume
	Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return false;
	Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
	if (!nrrd_vd_b) return false;
	void* data_vd_b = nrrd_vd_b->data;
	if (!data_vd_b) return false;

	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned int value_label;
	Color color;
	double max_value = m_vd->GetMaxValue();
	for (index = 0; index < for_size; ++index)
	{
		value_label = data_label[index];
		if (GetColor(value_label, tex->get_brick_id(index), m_vd, color_type, color))
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

	FLIVR::Color red = Color(1.0, 0.0, 0.0);
	FLIVR::Color green = Color(0.0, 1.0, 0.0);
	FLIVR::Color blue = Color(0.0, 0.0, 1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);

	bool bval = m_vd->GetEnableAlpha();
	vd_r->SetEnableAlpha(bval);
	vd_g->SetEnableAlpha(bval);
	vd_b->SetEnableAlpha(bval);
	bval = m_vd->GetShading();
	vd_r->SetShading(bval);
	vd_g->SetShading(bval);
	vd_b->SetShading(bval);
	vd_r->SetShadow(false);
	vd_g->SetShadow(false);
	vd_b->SetShadow(false);
	//other settings
	double dval = m_vd->Get3DGamma();
	vd_r->Set3DGamma(dval);
	vd_g->Set3DGamma(dval);
	vd_b->Set3DGamma(dval);
	dval = m_vd->GetBoundary();
	vd_r->SetBoundary(dval);
	vd_g->SetBoundary(dval);
	vd_b->SetBoundary(dval);
	dval = m_vd->GetOffset();
	vd_r->SetOffset(dval);
	vd_g->SetOffset(dval);
	vd_b->SetOffset(dval);
	dval = m_vd->GetLeftThresh();
	vd_r->SetLeftThresh(dval);
	vd_g->SetLeftThresh(dval);
	vd_b->SetLeftThresh(dval);
	dval = m_vd->GetRightThresh();
	vd_r->SetRightThresh(dval);
	vd_g->SetRightThresh(dval);
	vd_b->SetRightThresh(dval);
	dval = m_vd->GetAlpha();
	vd_r->SetAlpha(dval);
	vd_g->SetAlpha(dval);
	vd_b->SetAlpha(dval);
	dval = m_vd->GetSampleRate();
	vd_r->SetSampleRate(dval);
	vd_g->SetSampleRate(dval);
	vd_b->SetSampleRate(dval);
	vd_r->SetMaterial(amb, diff, spec, shine);
	vd_g->SetMaterial(amb, diff, spec, shine);
	vd_b->SetMaterial(amb, diff, spec, shine);

	channs.push_back(vd_r);
	channs.push_back(vd_g);
	channs.push_back(vd_b);

	return true;
}

bool ComponentAnalyzer::GetColor(
	unsigned int id, int brick_id,
	VolumeData* vd, int color_type,
	FLIVR::Color &color)
{
	if (!id)
		return false;

	switch (color_type)
	{
	case 1:
	default:
		color = FLIVR::Color(HSVColor(id % 360, 1.0, 1.0));
		return true;
	case 2:
		if (vd)
		{
			unsigned int size = 0;
			CompListIter iter;
			//search in comp list
			if (brick_id < 0)
			{
				int bn = vd->GetAllBrickNum();
				for (unsigned int i=0; i<bn; ++i)
				{
					iter = m_comp_list.find(GetKey(id, i));
					if (iter != m_comp_list.end())
						break;
				}
			}
			else
			{
				iter = m_comp_list.find(GetKey(id, brick_id));
			}
			if (iter == m_comp_list.end())
				return false;
			size = iter->second->sumi;
			double value;
			if (m_comp_list.min == m_comp_list.max)
				value = 1.0;
			else
				value = double(size - m_comp_list.min) /
				double(m_comp_list.max - m_comp_list.min);
			color = vd->GetColorFromColormap(value);
			return true;
		}
		break;
	}
	return false;
}

//replace id to make color consistent
void ComponentAnalyzer::ReplaceId(unsigned int base_id, pCompInfo &info)
{
	if (!m_vd || !m_vd->GetTexture())
		return;
	Texture* tex = m_vd->GetTexture();

	unsigned int brick_id = info->brick_id;
	unsigned int rep_id = info->id;
	unsigned int new_id = base_id;

	//get brick label data
	TextureBrick* b = tex->get_brick(brick_id);
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
	for (unsigned int k = 0; k < nz - 1; ++k)
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

	info->alt_id = new_id;
	info->id = new_id;
}

unsigned int ComponentAnalyzer::GetNonconflictId(
	unsigned int id,
	int nx, int ny, int nz,
	TextureBrick* b,
	unsigned int* data)
{
	unsigned int result = 0;
	unsigned int iid = id;

	unsigned int *tp, *tp2;
	unsigned int lv;
	do
	{
		bool found = false;
		tp = data;
		for (unsigned int k = 0; k < nz - 1; ++k)
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
						iid += 360;
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
