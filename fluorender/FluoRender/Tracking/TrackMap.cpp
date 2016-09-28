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

#include "TrackMap.h"
#include "Stencil.h"
#include "Cluster/dbscan.h"
#include "Cluster/kmeans.h"
#include "Cluster/exmax.h"
#include <functional>
#include <algorithm>
#include <limits>

using namespace FL;

TrackMap::TrackMap() :
	m_counter(0),
	m_frame_num(0),
	m_size_x(0),
	m_size_y(0),
	m_size_z(0),
	m_data_bits(8),
	m_scale(1.0f),
	m_spc_x(1.0f),
	m_spc_y(1.0f),
	m_spc_z(1.0f)
{
}

TrackMap::~TrackMap()
{
}

void TrackMapProcessor::SetSizes(size_t nx, size_t ny, size_t nz)
{
	m_map.m_size_x = nx;
	m_map.m_size_y = ny;
	m_map.m_size_z = nz;
}

void TrackMapProcessor::SetBits(size_t bits)
{
	m_map.m_data_bits = bits;
}

void TrackMapProcessor::SetScale(float scale)
{
	m_map.m_scale = scale;
}

void TrackMapProcessor::SetSpacings(float spcx, float spcy, float spcz)
{
	m_map.m_spc_x = spcx;
	m_map.m_spc_y = spcy;
	m_map.m_spc_z = spcz;
}

void TrackMapProcessor::SetVolCacheSize(size_t size)
{
	m_vol_cache.set_max_size(size);
}

bool TrackMapProcessor::InitializeFrame(size_t frame)
{
	//get label and data from cache
	VolCache cache = m_vol_cache.get(frame);
	void* data = cache.data;
	void* label = cache.label;
	if (!data || !label)
		return false;

	//add one empty cell list to track_map
	m_map.m_cells_list.push_back(CellList());
	CellList &cell_list = m_map.m_cells_list.back();
	CellListIter iter;
	//in the meanwhile build the intra graph
	m_map.m_intra_graph_list.push_back(IntraGraph());

	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	float data_value;
	unsigned int label_value;

	//build cell list
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		label_value = ((unsigned int*)label)[index];

		if (!label_value)
			continue;

		if (m_map.m_data_bits == 8)
			data_value = ((unsigned char*)data)[index] / 255.0f;
		else if (m_map.m_data_bits == 16)
			data_value = ((unsigned short*)data)[index] * m_map.m_scale / 65535.0f;

		iter = cell_list.find(label_value);
		if (iter != cell_list.end())
		{
			iter->second->Inc(i, j, k, data_value);
		}
		else
		{
			Cell* cell = new Cell(label_value);
			cell->Inc(i, j, k, data_value);
			cell_list.insert(std::pair<unsigned int, pCell>
				(label_value, pCell(cell)));
		}
	}

	//build intra graph
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		label_value = ((unsigned int*)label)[index];
		if (!label_value)
			continue;

		iter = cell_list.find(label_value);
		if (iter != cell_list.end())
		{
			CheckCellContact(iter->second, data, label,
				i, j, k);
		}
	}

	//build vertex list
	m_map.m_vertices_list.push_back(VertexList());
	VertexList &vertex_list = m_map.m_vertices_list.back();
	for (CellList::iterator cell_iterator = cell_list.begin();
	cell_iterator != cell_list.end(); ++cell_iterator)
	{
		if (cell_iterator->second->GetSizeF() < m_size_thresh)
			continue;
		pVertex vertex(new Vertex(cell_iterator->second->Id()));
		vertex->SetCenter(cell_iterator->second->GetCenter());
		vertex->SetSizeUi(cell_iterator->second->GetSizeUi());
		vertex->SetSizeF(cell_iterator->second->GetSizeF());
		vertex->AddCell(cell_iterator->second);
		cell_iterator->second->AddVertex(vertex);
		vertex_list.insert(std::pair<unsigned int, pVertex>
			(vertex->Id(), vertex));
	}

	m_map.m_frame_num++;
	return true;
}

#define ADD_CONTACT \
	idn = ((unsigned int*)label)[indexn]; \
	if (!idn) \
		ec++; \
		else if (idn != id) \
	{ \
		ec++; \
		if (data_bits == 8) \
			valuen = ((unsigned char*)data)[indexn] / 255.0f; \
		else if (data_bits == 16) \
			valuen = ((unsigned short*)data)[indexn] * scale / 65535.0f; \
		contact_value = std::min(value, valuen); \
		if (contact_value > m_contact_thresh) \
		{ \
			cc++; \
			iter = cell_list.find(idn); \
			if (iter != cell_list.end()) \
				AddContact(intra_graph, cell, iter->second, contact_value); \
		} \
	}

bool TrackMapProcessor::CheckCellContact(
	pCell &cell, void *data, void *label,
	size_t ci, size_t cj, size_t ck)
{
	int ec = 0;//external count
	int cc = 0;//contact count
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	size_t indexn;//neighbor index
	unsigned int idn;//neighbor id
	float valuen;//neighbor vlaue
	size_t index = nx*ny*ck + nx*cj + ci;
	unsigned int id = cell->Id();
	float value;
	size_t data_bits = m_map.m_data_bits;
	float scale = m_map.m_scale;
	if (data_bits == 8)
		value = ((unsigned char*)data)[index] / 255.0f;
	else if (data_bits == 16)
		value = ((unsigned short*)data)[index] * scale / 65535.0f;
	float contact_value;
	IntraGraph &intra_graph = m_map.m_intra_graph_list.back();
	CellList &cell_list = m_map.m_cells_list.back();
	CellListIter iter;

	if (ci == 0)
		ec++;
	else
	{
		indexn = index - 1;
		ADD_CONTACT;
	}
	if (ci >= nx - 1)
		ec++;
	else
	{
		indexn = index + 1;
		ADD_CONTACT;
	}
	if (cj == 0)
		ec++;
	else
	{
		indexn = index - nx;
		ADD_CONTACT;
	}
	if (cj >= ny - 1)
		ec++;
	else
	{
		indexn = index + nx;
		ADD_CONTACT;
	}
	if (ck == 0)
		ec++;
	else
	{
		indexn = index - nx*ny;
		ADD_CONTACT;
	}
	if (ck >= nz - 1)
		ec++;
	else
	{
		indexn = index + nx*ny;
		ADD_CONTACT;
	}

	if (ec)
	{
		cell->IncExternal(value);

		//expand search range if it's external but not contacting
		//if (cc == 0)
		//	CheckCellDist(cell, label, ci, cj, ck);
	}

	return true;
}

bool TrackMapProcessor::CheckCellDist(
	pCell &cell, void *label, size_t ci, size_t cj, size_t ck)
{
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	float spcx = m_map.m_spc_x;
	float spcy = m_map.m_spc_y;
	float spcz = m_map.m_spc_z;
	size_t indexn;//neighbor index
	unsigned int idn;//neighbor id
	size_t index = nx*ny*ck + nx*cj + ci;
	unsigned int id = cell->Id();
	IntraGraph &intra_graph = m_map.m_intra_graph_list.back();
	CellList &cell_list = m_map.m_cells_list.back();
	CellListIter iter;
	float dist_v, dist_s;

	//search range
	int r = int(sqrt(m_size_thresh)/2.0+0.5);
	r = r < 1 ? 1 : r;
	for (int k = -r; k<=r; ++k)
	for (int j = -r; j<=r; ++j)
	for (int i = -r; i<=r; ++i)
	{
		if (ck + k < 0 || ck + k >= nz ||
			cj + j < 0 || cj + j >= ny ||
			ci + i < 0 || ci + i >= nx)
			continue;
		indexn = nx*ny*(ck + k) + nx*(cj + j) + ci + i;
		idn = ((unsigned int*)label)[indexn];
		if (!idn || idn == id)
			continue;
		iter = cell_list.find(idn);
		if (iter != cell_list.end())
		{
			dist_v = sqrt(i*i + j*j + k*k);
			dist_s = sqrt(i*i*spcx*spcx + j*j*spcy*spcy + k*k*spcz*spcz);
			AddNeighbor(intra_graph, cell, iter->second, dist_v, dist_s);
		}
	}
	return true;
}

bool TrackMapProcessor::AddContact(IntraGraph& graph,
	pCell &cell1, pCell &cell2, float contact_value)
{
	IntraVert v1 = cell1->GetIntraVert();
	IntraVert v2 = cell2->GetIntraVert();
	if (v1 == IntraGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = cell1->Id();
		graph[v1].cell = cell1;
		cell1->SetIntraVert(v1);
	}
	if (v2 == IntraGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = cell2->Id();
		graph[v2].cell = cell2;
		cell2->SetIntraVert(v2);
	}

	std::pair<IntraEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 1;
		graph[e.first].size_f = contact_value;
		graph[e.first].dist_v = 0.0f;
		graph[e.first].dist_s = 0.0f;
	}
	else
	{
		graph[e.first].size_ui++;
		graph[e.first].size_f += contact_value;
		graph[e.first].dist_v = 0.0f;
		graph[e.first].dist_s = 0.0f;
	}

	return true;
}

bool TrackMapProcessor::AddNeighbor(IntraGraph& graph,
	pCell &cell1, pCell &cell2,
	float dist_v, float dist_s)
{
	IntraVert v1 = cell1->GetIntraVert();
	IntraVert v2 = cell2->GetIntraVert();
	if (v1 == IntraGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = cell1->Id();
		graph[v1].cell = cell1;
		cell1->SetIntraVert(v1);
	}
	if (v2 == IntraGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = cell2->Id();
		graph[v2].cell = cell2;
		cell2->SetIntraVert(v2);
	}

	std::pair<IntraEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 0;
		graph[e.first].size_f = 0.0f;
		graph[e.first].dist_v = dist_v;
		graph[e.first].dist_s = dist_s;
	}
	else
	{
		//update the distance only if there is no
		//contact and current distance is shorter
		if (graph[e.first].size_ui == 0 &&
			dist_s < graph[e.first].dist_s)
		{
			graph[e.first].dist_v = dist_v;
			graph[e.first].dist_s = dist_s;
		}
	}

	return true;
}

bool TrackMapProcessor::LinkFrames(
	size_t f1, size_t f2)
{
	size_t frame_num = m_map.m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num || f1 == f2)
		return false;

	//get data and label
	VolCache cache = m_vol_cache.get(f1);
	void* data1 = cache.data;
	void* label1 = cache.label;
	if (!data1 || !label1)
		return false;
	cache = m_vol_cache.get(f2);
	void* data2 = cache.data;
	void* label2 = cache.label;
	if (!data2 || !label2)
		return false;

	m_map.m_inter_graph_list.push_back(InterGraph());
	InterGraph &inter_graph = m_map.m_inter_graph_list.back();
	inter_graph.index = f1;
	inter_graph.counter = 0;

	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	float data_value1, data_value2;
	unsigned int label_value1, label_value2;
	VertexList &vertex_list1 = m_map.m_vertices_list.at(f1);
	VertexList &vertex_list2 = m_map.m_vertices_list.at(f2);
	VertexListIter iter1, iter2;

	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		label_value1 = ((unsigned int*)label1)[index];
		label_value2 = ((unsigned int*)label2)[index];

		if (!label_value1 || !label_value2)
			continue;

		if (m_map.m_data_bits == 8)
		{
			data_value1 = ((unsigned char*)data1)[index] / 255.0f;
			data_value2 = ((unsigned char*)data2)[index] / 255.0f;
		}
		else if (m_map.m_data_bits == 16)
		{
			data_value1 = ((unsigned short*)data1)[index] * m_map.m_scale / 65535.0f;
			data_value2 = ((unsigned short*)data2)[index] * m_map.m_scale / 65535.0f;
		}

		iter1 = vertex_list1.find(label_value1);
		iter2 = vertex_list2.find(label_value2);

		if (iter1 == vertex_list1.end() ||
			iter2 == vertex_list2.end())
			continue;

		if (iter1->second->GetSizeF() < m_size_thresh ||
			iter2->second->GetSizeF() < m_size_thresh)
			continue;

		LinkVertices(inter_graph,
			iter1->second, iter2->second,
			f1, f2,
			std::min(data_value1, data_value2));
	}

	return true;
}

bool TrackMapProcessor::LinkVertices(InterGraph& graph,
	pVertex &vertex1, pVertex &vertex2,
	size_t f1, size_t f2, float overlap_value)
{
	InterVert v1 = vertex1->GetInterVert(graph);
	InterVert v2 = vertex2->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex1->Id();
		graph[v1].frame = f1;
		graph[v1].count = 1;
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = f2;
		graph[v2].count = 1;
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}

	std::pair<InterEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 1;
		graph[e.first].size_f = overlap_value;
		FLIVR::Point p1 = vertex1->GetCenter();
		FLIVR::Point p2 = vertex2->GetCenter();
		graph[e.first].dist_f = float((p1 - p2).length());
		graph[e.first].link = 1;
		graph[e.first].count = 1;
	}
	else
	{
		graph[e.first].size_ui++;
		graph[e.first].size_f += overlap_value;
	}

	return true;
}

bool TrackMapProcessor::LinkOrphans(InterGraph& graph, pVertex &vertex)
{
	if (!vertex)
		return false;

	FLIVR::Point pos = vertex->GetCenter();
	float size = vertex->GetSizeF();

	pVertex vertex1;
	size_t valence1;
	FLIVR::Point pos1;
	double d_min = -1;
	double d;
	pVertex v1_min;

	//find closest
	VertexListIter iter;
	VertexList &vertex_list = m_map.m_vertices_list.at(m_frame2);
	for (iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
	{
		vertex1 = iter->second;
		//if orphan too
		
		if (GetValence(vertex1, graph, valence1) && valence1)
			continue;
		//pos
		pos1 = vertex1->GetCenter();
		d = (pos - pos1).length();
		if (d_min < 0)
		{
			d_min = d;
			v1_min = vertex1;
		}
		else
		{
			if (d < d_min)
			{
				d_min = d;
				v1_min = vertex1;
			}
		}
	}

	if (!v1_min)
		return false;
	//should have similar size
	if (!similar_vertex_size(vertex, v1_min))
		return false;
	//should be very close
	FLIVR::BBox box = vertex->GetBox();
	FLIVR::BBox box1 = v1_min->GetBox();
	box.extend(0.1, 0.1, 0.1);
	box1.extend(0.1, 0.1, 0.1);
	if (!box.intersect(box1))
		return false;

	//link vertices
	InterVert v1 = vertex->GetInterVert(graph);
	InterVert v2 = v1_min->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex->Id();
		graph[v1].frame = m_frame1;
		graph[v1].count = 1;
		graph[v1].vertex = vertex;
		vertex->SetInterVert(graph, v1);
	}
	else
		graph[v1].count++;
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = v1_min->Id();
		graph[v2].frame = m_frame2;
		graph[v2].count = 1;
		graph[v2].vertex = v1_min;
		v1_min->SetInterVert(graph, v2);
	}
	else
		graph[v2].count++;

	std::pair<InterEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 0;
		graph[e.first].size_f = 0.0f;
		graph[e.first].dist_f = d;
		graph[e.first].link = 1;
		graph[e.first].count = 1;
	}
	else
		graph[e.first].count++;

	return true;
}

bool TrackMapProcessor::IsolateVertex(InterGraph& graph, pVertex &vertex)
{
	InterVert v1, v2;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	std::pair<InterEdge, bool> edge;

	v1 = vertex->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
		return false;
	//reset
	graph[v1].count = 0;
	adj_verts = boost::adjacent_vertices(v1, graph);
	//for each adjacent vertex
	for (inter_iter = adj_verts.first;
	inter_iter != adj_verts.second; ++inter_iter)
	{
		v2 = *inter_iter;
		if (v2 == InterGraph::null_vertex())
			continue;
		edge = boost::edge(v1, v2, graph);
		if (edge.second &&
			graph[edge.first].link)
		{
			graph[edge.first].link = 0;
			//reset
			graph[edge.first].count = 0;
		}
	}

	return true;
}

bool TrackMapProcessor::ForceVertices(InterGraph& graph,
	pVertex &vertex1, pVertex &vertex2,
	size_t f1, size_t f2)
{
	InterVert v1 = vertex1->GetInterVert(graph);
	InterVert v2 = vertex2->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex1->Id();
		graph[v1].frame = f1;
		//reset
		graph[v1].count = 0;
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = f2;
		//reset
		graph[v2].count = 0;
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}

	std::pair<InterEdge, bool> edge;
	edge = boost::edge(v1, v2, graph);
	if (!edge.second)
	{
		edge = boost::add_edge(v1, v2, graph);
		graph[edge.first].size_ui = std::max(
			vertex1->GetSizeUi(), vertex2->GetSizeUi());
		graph[edge.first].size_f = std::max(
			vertex1->GetSizeF(), vertex2->GetSizeF());
		FLIVR::Point p1 = vertex1->GetCenter();
		FLIVR::Point p2 = vertex2->GetCenter();
		graph[edge.first].dist_f = float((p1 - p2).length());
		graph[edge.first].link = 2;
		//reset
		graph[edge.first].count = 0;
	}
	else
	{
		graph[edge.first].size_ui = std::max(
			vertex1->GetSizeUi(), vertex2->GetSizeUi());
		graph[edge.first].size_f = std::max(
			vertex1->GetSizeF(), vertex2->GetSizeF());
		graph[edge.first].link = 2;
		//reset
		graph[edge.first].count = 0;
	}

	return true;
}

bool TrackMapProcessor::UnlinkVertices(InterGraph& graph,
	pVertex &vertex1, pVertex &vertex2)
{
	InterVert v1, v2;

	v1 = vertex1->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
		return false;
	//reset
	graph[v1].count = 0;
	v2 = vertex2->GetInterVert(graph);
	if (v2 == InterGraph::null_vertex())
		return false;
	//reset
	graph[v2].count = 0;

	std::pair<InterEdge, bool> edge;
	edge = boost::edge(v1, v2, graph);
	if (edge.second)
	{
		graph[edge.first].link = 0;
		//reset
		graph[edge.first].count = 0;
	}

	return true;
}

bool TrackMapProcessor::ResolveGraph(size_t frame1, size_t frame2)
{
	if (frame1 >= m_map.m_frame_num ||
		frame2 >= m_map.m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = m_map.m_vertices_list.at(frame1);
	VertexList &vertex_list2 = m_map.m_vertices_list.at(frame2);
	IntraGraph &intra_graph = m_map.m_intra_graph_list.at(frame2);
	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	VertexListIter iter;
	InterVert v1, v2;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	std::vector<pwCell> cells;
	std::vector<CellBin> cell_bins;
	CellBinIter pwcell_iter;
	pVertex vertex2;

	//check all vertices in the time frame
	for (iter = vertex_list1.begin();
	iter != vertex_list1.end(); ++iter)
	{
		v1 = iter->second->GetInterVert(inter_graph);
		if (v1 == InterGraph::null_vertex())
			continue;
		adj_verts = boost::adjacent_vertices(v1, inter_graph);
		//for each adjacent vertex
		//add cells to cells the list
		cells.clear();
		for (inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
		{
			v2 = *inter_iter;
			vertex2 = inter_graph[v2].vertex.lock();
			if (!vertex2)
				continue;
			//store all cells in the list temporarily
			for (pwcell_iter = vertex2->GetCellsBegin();
			pwcell_iter != vertex2->GetCellsEnd(); ++pwcell_iter)
				cells.push_back(*pwcell_iter);
		}
		//if a cell in the list has contacts that are also in the list,
		//try to group them
		cell_bins.clear();
		if (GroupCells(cells, cell_bins, intra_graph,
			&TrackMapProcessor::merge_cell_size))
		{
			//modify vertex list 2 if necessary
			for (size_t i = 0; i < cell_bins.size(); ++i)
				MergeCells(vertex_list2, cell_bins[i], frame2);
		}
	}

	return true;
}

bool TrackMapProcessor::ProcessFrames(size_t frame1, size_t frame2)
{
	if (frame1 >= m_map.m_frame_num ||
		frame2 >= m_map.m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = m_map.m_vertices_list.at(frame1);
	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	m_frame1 = frame1;
	m_frame2 = frame2;

	VertexListIter iter;

	for (iter = vertex_list1.begin();
		iter != vertex_list1.end();)
	{
		ProcessVertex(iter->second, inter_graph,
			m_merge, m_split);
		//see if it is removed
		////debug
		//pVertex vert = iter->second;
		if (iter->second->GetRemovedFromGraph())
			iter = vertex_list1.erase(iter);
		else
			++iter;
	}

	inter_graph.counter++;

	return true;
}

//vertex matching routines
//find out current valence of a vertex
bool TrackMapProcessor::GetValence(pVertex &vertex, InterGraph &graph,
	size_t &valence)
{
	valence = 0;
	if (!vertex)
		return false;
	InterVert v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	InterVert v1;
	std::pair<InterEdge, bool> edge;
	unsigned int link;

	//set flag for link
	std::pair<InterAdjIter, InterAdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (InterAdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			link = graph[edge.first].link;
			if (link == 1 || link == 2)
				valence++;
		}
	}

	return true;
}

bool TrackMapProcessor::GetValence(pVertex &vertex, InterGraph &graph,
	size_t &valence, std::vector<InterEdge> &edges)
{
	valence = 0;
	if (!vertex)
		return false;
	InterVert v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	InterVert v1;
	std::pair<InterEdge, bool> edge;
	unsigned int link;

	//set flag for link
	std::pair<InterAdjIter, InterAdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (InterAdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			link = graph[edge.first].link;
			if (link == 1 || link == 2)
				valence++;
			edges.push_back(edge.first);
		}
	}

	return true;
}

bool TrackMapProcessor::GetValence(pVertex &vertex, InterGraph &graph,
	size_t &valence, std::vector<InterEdge> &all_edges,
	std::vector<InterEdge> &linked_edges)
{
	valence = 0;
	if (!vertex)
		return false;
	InterVert v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	InterVert v1;
	std::pair<InterEdge, bool> edge;
	unsigned int link;

	//set flag for link
	std::pair<InterAdjIter, InterAdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (InterAdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		////debug
		//pVertex vert1 = graph[v1].vertex.lock();
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			link = graph[edge.first].link;
			if (link == 1 || link == 2)
			{
				valence++;
				linked_edges.push_back(edge.first);
			}
			all_edges.push_back(edge.first);
		}
	}

	return true;
}

//simple match of the max overlap
bool TrackMapProcessor::LinkEdgeSize(InterGraph &graph, pVertex &vertex,
	std::vector<InterEdge> &edges, bool calc_sim)
{
	size_t edge_size = edges.size();
	if (!edge_size)
		return false;

	if (edge_size > 1)
		//sort edges
		std::sort(edges.begin(), edges.end(),
			std::bind(comp_edge_size, std::placeholders::_1,
				std::placeholders::_2, graph));

	//link edges by size
	bool result = false;
	//if 0 hasn't been linked/unlinked many times
	//try later ones
	if (graph.counter > 1 && calc_sim)
	{
		unsigned int count_prv = graph[edges[0]].count;
		unsigned int count_cur;
		for (size_t i = 1; i < edge_size; ++i)
		{
			count_cur = graph[edges[i]].count;
			if (count_cur <= count_prv)
			{
				link_edge(edges[i], graph);
				result = true;
				break;
			}
			count_prv = count_cur;
		}
	}
	else//otherwise, it may be useful (uncertain)
	{
		link_edge(edges[0], graph);
		for (size_t i = 1; i < edge_size; ++i)
		{
			if (similar_edge_size(edges[0], edges[i], graph))
				link_edge(edges[i], graph);
		}
		result = true;
	}

	return result;
}

//unlink edge by size similarity
bool TrackMapProcessor::UnlinkEdgeSize(InterGraph &graph, pVertex &vertex,
	std::vector<InterEdge> &edges, bool calc_sim)
{
	if (edges.size() < 2)
		return false;

	//sort edges
	std::sort(edges.begin(), edges.end(),
		std::bind(comp_edge_size, std::placeholders::_1,
			std::placeholders::_2, graph));
	//suppose we have more than 2 edges, find where to cut
	//if 0 hasn't been linked/unlinked many times
	if (calc_sim)
	{
		for (size_t ei = 1; ei < edges.size(); ++ei)
		{
			if (!similar_edge_size(edges[0], edges[ei], graph))
			{
				//if unsimilar, keep the first one
				for (size_t i = ei; i < edges.size(); ++i)
					unlink_edge(edges[i], graph);
				return true;
			}
		}
	}
	//otherwise, it's uncertain
	return false;
}

//unlink edge by extended alternating path
bool TrackMapProcessor::UnlinkAlterPath(InterGraph &graph, pVertex &vertex,
	bool calc_sim)
{
	//expand the search range with alternating paths
	m_level_thresh = calc_sim ? 2 : 3;

	PathList paths;
	if (!GetAlterPath(graph, vertex, paths))
		return false;
	if (paths.size() < 2)
		return false;

	//use max matching
	bool result =
		UnlinkAlterPathMaxMatch(graph, vertex, paths, calc_sim);
	if (!result)
		result = UnlinkAlterPathConn(graph, vertex, paths);

	return result;
}

bool TrackMapProcessor::GetAlterPath(InterGraph &graph, pVertex &vertex,
	PathList &paths)
{
	//get all potential alternating paths
	bool got_list;
	VertVisitList visited;
	while (true)
	{
		Path alt_path(graph);
		got_list = get_alter_path(
			graph, vertex, alt_path,
			visited, 0);
		if (got_list)
			paths.push_back(alt_path);
		else
			break;
	}

	if (paths.size())
		return true;
	else
		return false;
}

bool TrackMapProcessor::UnlinkAlterPathMaxMatch(InterGraph &graph, pVertex &vertex,
	PathList &paths, bool calc_sim)
{
	float max_weight = 0;
	max_weight = get_path_max(graph, paths, 0, 0);

	//order paths
	std::sort(paths.begin(), paths.end(),
		TrackMapProcessor::comp_path_mm);
	
	//similar to unlink edge size
	bool unlinked = false;
	for (size_t pi = 1; pi < paths.size(); ++pi)
	{
		if (!calc_sim || (calc_sim &&
			!similar_path_mm(paths[0], paths[pi])))
		{
			//if unsimilar, keep the first one
			for (size_t i = pi; i < paths.size(); ++i)
				unlinked |= paths[i].unlink();
			return unlinked;
		}
	}

	return false;
}

bool TrackMapProcessor::UnlinkAlterPathSize(InterGraph &graph, pVertex &vertex,
	PathList &paths)
{
	//order paths
	std::sort(paths.begin(), paths.end(),
		TrackMapProcessor::comp_path_size);
	//similar to unlink edge size
	for (size_t pi = 1; pi < paths.size(); ++pi)
	{
		if (!similar_path_size(paths[0], paths[pi]))
		{
			for (size_t i = 0; i < paths.size(); ++i)
			{
				if (i < pi)
				{
					if (paths[i].get_max_size() ==
						paths[i].get_odd_size())
						paths[i].flip();
				}
				else
					paths[i].flip();
			}
			return true;
		}
	}

	return false;
}

bool TrackMapProcessor::UnlinkAlterPathCount(InterGraph &graph, pVertex &vertex,
	PathList &paths)
{
	//order paths
	if (graph.counter % 2)
		std::sort(paths.begin(), paths.end(),
			TrackMapProcessor::comp_path_count);
	else
		std::sort(paths.begin(), paths.end(),
			TrackMapProcessor::comp_path_count_rev);
	//similar to unlink edge size
	for (size_t pi = 1; pi < paths.size(); ++pi)
	{
		if (!similar_path_count(paths[0], paths[pi]))
		{
			for (size_t i = 0; i < paths.size(); ++i)
				paths[i].flip();
			return true;
		}
	}

	return false;
}

bool TrackMapProcessor::UnlinkAlterPathConn(InterGraph &graph, pVertex &vertex,
	PathList &paths)
{
	if (paths.size() < 2)
		return false;

	std::vector<size_t> plist;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	std::pair<InterEdge, bool> edge;
	InterVert v1;
	InterVert v0 = paths[0][0].vert;
	size_t links = 0;

	for (size_t pi = 0; pi < paths.size(); ++pi)
	{
		if (!paths[pi][0].edge_valid ||
			paths[pi][0].link != 1)
			continue;
		else
			links++;

		adj_verts = boost::adjacent_vertices(
			paths[pi][1].vert, graph);
		for (InterAdjIter inter_iter = adj_verts.first;
			inter_iter != adj_verts.second; ++inter_iter)
		{
			v1 = *inter_iter;
			if (v1 == v0)
				continue;
			edge = boost::edge(paths[pi][1].vert, v1, graph);
			if (edge.second &&
				(graph[edge.first].link == 1 ||
					graph[edge.first].link == 2))
			{
				plist.push_back(pi);
				break;
			}
		}
	}

	if (plist.size() < links)
	{
		bool unlinked = false;
		for (size_t pi = 0; pi < plist.size(); ++pi)
			unlinked |= paths[pi].unlink();
		return unlinked;
	}

	return false;
}

//unlink edge by vertex size, use after merge fails
bool TrackMapProcessor::UnlinkVertexSize(InterGraph &graph, pVertex &vertex,
	std::vector<InterEdge> &edges)
{
	if (edges.size() < 2)
		return false;

	bool result = false;

	//find vertex that is not similar
	pVertex vert1;
	for (size_t ei = 0; ei < edges.size(); ++ei)
	{
		vert1 = graph[boost::target(edges[ei], graph)].vertex.lock();
		if (vertex == vert1)
			vert1 = graph[boost::source(edges[ei], graph)].vertex.lock();
		if (!vert1) continue;

		if (!similar_vertex_size(vertex, vert1))
		{
			unlink_edge(edges[ei], graph);
			result = true;
		}
	}

	return result;
}

//reduce valence by segmentation
bool TrackMapProcessor::MergeEdges(InterGraph &graph, pVertex &vertex,
	std::vector<InterEdge> &edges)
{
	if (edges.size() < 2)
		return false;

	pVertex v1;
	CellBinIter pwcell_iter;
	CellList cells;
	pCell cell;
	CellBin cell_bin;

	//first, check if any out cells are touching
	for (size_t i = 0; i < edges.size(); ++i)
	{
		v1 = graph[boost::target(edges[i], graph)].vertex.lock();
		if (v1 == vertex)
			v1 = graph[boost::source(edges[i], graph)].vertex.lock();
		if (!v1) continue;

		//store all cells in the list temporarily
		for (pwcell_iter = v1->GetCellsBegin();
			pwcell_iter != v1->GetCellsEnd(); ++pwcell_iter)
		{
			cell = pwcell_iter->lock();
			if (cell)
			cells.insert(std::pair<unsigned int, pCell>
				(cell->Id(), cell));
			cell_bin.push_back(*pwcell_iter);
		}
	}
	//if a cell in the list has contacts that are also in the list,
	//try to group them
	if (!v1) return false;
	size_t frame = v1->GetFrame(graph);
	if (ClusterCellsMerge(cells, frame))
	{
		//modify vertex list 2 if necessary
		VertexList &vertex_list = m_map.m_vertices_list.at(frame);
		MergeCells(vertex_list, cell_bin, frame);
		return true;
	}

	return false;
}

bool TrackMapProcessor::SplitVertex(InterGraph &graph, pVertex &vertex,
	std::vector<InterEdge> &edges)
{
	if (!vertex || edges.size() < 2)
		return false;

	if (vertex->GetSplit())
		return false;
	vertex->SetSplit();
	
	CellBinIter pwcell_iter;
	CellList cells;
	pCell cell;

	//store all cells in the list temporarily
	for (pwcell_iter = vertex->GetCellsBegin();
		pwcell_iter != vertex->GetCellsEnd(); ++pwcell_iter)
	{
		cell = pwcell_iter->lock();
		if (cell)
			cells.insert(std::pair<unsigned int, pCell>
				(cell->Id(), cell));
	}

	size_t frame = vertex->GetFrame(graph);
	CellList outlist;
	if (ClusterCellsSplit(cells, frame, edges.size(), outlist))
	{
		//remove input vertex
		RemoveVertex(graph, vertex);
		AddCells(outlist, frame);
		//remove vertex from another intergraph
		size_t gindex = graph.index == frame ? frame - 1 : frame;
		if (gindex < m_map.m_frame_num - 1)
		{
			InterGraph &graph2 = m_map.m_inter_graph_list.at(gindex);
			RemoveVertex(graph2, vertex);
		}

		LinkAddedCells(outlist, frame, frame - 1);
		LinkAddedCells(outlist, frame, frame + 1);

		return true;
	}

	return false;
}

bool TrackMapProcessor::ProcessVertex(pVertex &vertex, InterGraph &graph,
	bool hint_merge, bool hint_split)
{
	bool result = false;

	//get count
	unsigned int count = 0;
	InterVert inter_vert = vertex->GetInterVert(graph);
	if (inter_vert != InterGraph::null_vertex())
		count = graph[inter_vert].count;
	bool calc_sim = get_random(count, graph);

	//get valence
	size_t valence;
	std::vector<InterEdge> all_edges;
	std::vector<InterEdge> linked_edges;
	GetValence(vertex, graph, valence, all_edges, linked_edges);
	if (valence == 0)
	{
		result = LinkEdgeSize(graph, vertex, all_edges, calc_sim);
		//if (!result)	//find and link neighboring orphans
		//	result = LinkOrphans(graph, vertex);
	}
	else if (valence > 1)
	{
		result = UnlinkEdgeSize(graph, vertex, linked_edges, calc_sim);
		if (!result)	//unlink edge by extended alternating path
			result = UnlinkAlterPath(graph, vertex, calc_sim);
		if (!result && hint_merge)	//merge edges if possible (DBSCAN)
			result = MergeEdges(graph, vertex, linked_edges);
		if (!result)	//unlink edge based on vertex size
			result = UnlinkVertexSize(graph, vertex, linked_edges);
		if (!result && hint_split)	//split vertex if possible (EM)
			result = SplitVertex(graph, vertex, linked_edges);
	}

	return result;
}

bool TrackMapProcessor::comp_edge_size(InterEdge &edge1,
	InterEdge &edge2, InterGraph& graph)
{
	return graph[edge1].size_f > graph[edge2].size_f;
}

bool TrackMapProcessor::comp_path_size(Path &path1, Path &path2)
{
	float p1 = path1.get_size(0);
	float p1_odd = path1.get_size(1);
	float p2 = path2.get_size(0);
	float p2_odd = path2.get_size(1);
	return path1.get_max_size() > path2.get_max_size();
}

bool TrackMapProcessor::comp_path_mm(Path &path1, Path &path2)
{
	if (path1.size() && path2.size())
		return path1[0].max_value > path2[0].max_value;
	else
		return false;
}

bool TrackMapProcessor::similar_edge_size(InterEdge &edge1,
	InterEdge &edge2, InterGraph& graph)
{
	float s1 = graph[edge1].size_f;
	float s2 = graph[edge2].size_f;
	//float s1 = graph[edge1].size_ui;
	//float s2 = graph[edge2].size_ui;
	float d;
	if (s1 > 0.0f || s2 > 0.0f)
	{
		d = fabs(s1 - s2) / std::max(s1, s2);
		return d < m_similar_thresh;
	}
	else
	{
		s1 = graph[edge1].dist_f;
		s2 = graph[edge2].dist_f;
		if (s1 > 0.0f || s2 > 0.0f)
		{
			d = fabs(s1 - s2) / std::max(s1, s2);
			return d < m_similar_thresh;
		}
		else
			return true;
	}
}

bool TrackMapProcessor::similar_path_size(Path &path1, Path &path2)
{
	float p1 = path1.get_max_size();
	float p2 = path2.get_max_size();
	float d;
	//p1 compares to p2
	if (p1 > 0.0f || p2 > 0.0f)
	{
		d = fabs(p1 - p2) / std::max(p1, p2);
		if (d < m_similar_thresh)
			return true;
	}
	return false;
}

bool TrackMapProcessor::similar_path_mm(Path &path1, Path &path2)
{
	if (path1.size() && path2.size())
	{
		float p1 = path1[0].max_value;
		float p2 = path2[0].max_value;
		float d;
		//p1 compares to p2
		if (p1 > 0.0f || p2 > 0.0f)
		{
			d = fabs(p1 - p2) / std::max(p1, p2);
			if (d < m_similar_thresh)
				return true;
		}
	}
	return false;
}

bool TrackMapProcessor::comp_path_count(Path &path1, Path &path2)
{
	unsigned int p1 = path1.get_count(0);
	unsigned int p1_odd = path1.get_count(1);
	unsigned int p2 = path2.get_count(0);
	unsigned int p2_odd = path2.get_count(1);
	return path1.get_max_count() < path2.get_max_count();
}

bool TrackMapProcessor::comp_path_count_rev(Path &path1, Path &path2)
{
	unsigned int p1 = path1.get_count(0);
	unsigned int p1_odd = path1.get_count(1);
	unsigned int p2 = path2.get_count(0);
	unsigned int p2_odd = path2.get_count(1);
	return path1.get_max_count() > path2.get_max_count();
}

bool TrackMapProcessor::similar_path_count(Path &path1, Path &path2)
{
	unsigned int p1 = path1.get_max_count();
	unsigned int p2 = path2.get_max_count();
	float d;
	//p1 compares to p2
	if (p1 > 0 || p2 > 0)
	{
		d = fabs(p1 - p2) / std::max(p1, p2);
		if (d < m_similar_thresh)
			return true;
	}
	return false;
}

bool TrackMapProcessor::similar_vertex_size(pVertex& v1, pVertex& v2)
{
	float s1 = v1->GetSizeF();
	float s2 = v2->GetSizeF();
	if (fabs(s1 - s2) / std::max(s1, s2) < m_similar_thresh)
		return true;
	else
		return false;
}

void TrackMapProcessor::link_edge(InterEdge edge, InterGraph &graph, unsigned int value)
{
	unsigned int link = graph[edge].link;
	if (link == 0 || link == 1)
	{
		if (link != value)
		{
			graph[edge].link = value;
			graph[edge].count++;
			graph[boost::source(edge, graph)].count++;
			graph[boost::target(edge, graph)].count++;
		}
	}
}

void TrackMapProcessor::unlink_edge(InterEdge edge, InterGraph &graph, unsigned int value)
{
	unsigned int link = graph[edge].link;
	if (link == 0 || link == 1)
	{
		if (link != value)
		{
			graph[edge].link = value;
			graph[edge].count++;
			graph[boost::source(edge, graph)].count++;
			graph[boost::target(edge, graph)].count++;
		}
	}
}

bool TrackMapProcessor::get_alter_path(InterGraph &graph, pVertex &vertex,
	Path &alt_path, VertVisitList &visited, int curl)
{
	if (!vertex)
		return false;
	InterVert v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	visited.insert(v0);
	PathVert pv0;
	pv0.vert = v0;
	pv0.edge_valid = false;
	pv0.edge_value = 0;
	pv0.max_value = 0;
	pv0.link = 0;
	alt_path.push_back(pv0);
	graph[v0].max_value = 0;
	graph[v0].max_valid = false;

	if (curl >= m_level_thresh)
		return true;

	InterVert v1;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	std::pair<InterEdge, bool> edge;
	pVertex vertex1;

	adj_verts = boost::adjacent_vertices(v0, graph);
	for (InterAdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			vertex1 = graph[v1].vertex.lock();
			if (visited.find(v1) ==
				visited.end())
			{
				//set back
				alt_path.back().edge_valid = true;
				alt_path.back().edge_value = graph[edge.first].size_f;
				alt_path.back().link = graph[edge.first].link;

				return get_alter_path(graph, vertex1,
					alt_path, visited, curl + 1);
			}
		}
	}

	if (alt_path.size() > 1)
		return true;
	else
		return false;
}

float TrackMapProcessor::get_path_max(InterGraph &graph, PathList &paths,
	size_t curl, InterVert v0)
{
	float result = 0.0f;
	float value;
	InterVert vert;
	VertVisitList visited;

	//go through all paths
	for (size_t pi = 0; pi < paths.size(); ++pi)
	{
		//continue if path doesn't have the level
		if (curl >= paths[pi].size())
			continue;
		vert = paths[pi][curl].vert;
		//input vert is not linked
		if (v0 == vert)
			continue;

		//if already calculated, use the stored value
		if (vert != InterGraph::null_vertex() &&
			visited.find(vert) == visited.end() &&
			graph[vert].max_valid)
		{
			result += graph[vert].max_value;
			visited.insert(vert);
			continue;
		}

		//if not calculated, calculate max value recursively
		value = paths[pi][curl].edge_value;
		if (paths[pi][curl].edge_valid)
			value += get_path_max(graph, paths, curl + 1,
				paths[pi][curl+1].vert);
		paths[pi][curl].max_value = value;

		//accumulate all possible values
		if (visited.find(vert) ==
			visited.end())
		{
			result += value;
			graph[vert].max_value = value;
			visited.insert(vert);
		}
		else if (value > graph[vert].max_value)
		{
			result -= graph[vert].max_value;
			result += value;
			graph[vert].max_value = value;
		}
	}

	//set flag for validity
	for (size_t pi = 0; pi < paths.size(); ++pi)
	{
		//continue if path doesn't have the level
		if (curl >= paths[pi].size())
			continue;
		vert = paths[pi][curl].vert;
		if (visited.find(vert) != visited.end())
			graph[vert].max_valid = true;
	}

	return result;
}

bool TrackMapProcessor::unlink_alt_path(InterGraph &graph, PathList &paths)
{
	for (size_t pi = 0; pi < paths.size(); ++pi)
	{
		Path path = paths[pi];
		bool link = false;
		unsigned int l;
		for (PathIter iter = path.begin();
			iter != path.end(); ++iter)
		{
			PathIter i1 = iter + 1;
			if (i1 == path.end())
				break;
			std::pair<InterEdge, bool> edge =
				boost::edge(iter->vert, i1->vert, graph);
			if (!edge.second)
				break;
			l = graph[edge.first].link;

			//link
			if (!link &&
				iter->edge_valid &&
				iter->max_value ==
				graph[iter->vert].max_value)
			{
				if (l == 2 || l == 3)
				{
					link = true;
					continue;
				}

				if (l != 1)
				{
					graph[edge.first].link = 1;
					graph[edge.first].count++;
					graph[boost::source(edge.first, graph)].count++;
					graph[boost::target(edge.first, graph)].count++;
				}

				link = true;
			}
			else//unlink
			{
				if (l == 2 || l == 3)
				{
					link = false;
					continue;
				}

				if (l != 0)
				{
					graph[edge.first].link = 0;
					graph[edge.first].count++;
					graph[boost::source(edge.first, graph)].count++;
					graph[boost::target(edge.first, graph)].count++;
				}

				link = false;
			}
		}
	}

	return true;
}

bool TrackMapProcessor::merge_cell_size(IntraEdge &edge,
	pCell &cell1, pCell &cell2, IntraGraph& graph)
{
	float osizef, c1sizef, c2sizef;
	osizef = graph[edge].size_f;
	c1sizef = cell1->GetSizeF();
	c2sizef = cell2->GetSizeF();
	return osizef / c1sizef > m_contact_thresh ||
			osizef / c2sizef > m_contact_thresh;
}

//determine if cells on intragraph can be merged
bool TrackMapProcessor::GroupCells(std::vector<pwCell> &cells,
	std::vector<CellBin> &cell_bins, IntraGraph &intra_graph,
	f_merge_cell merge_cell)
{
	pCell cell2, cell2c;
	IntraVert c2, c2c;
	std::pair<IntraAdjIter, IntraAdjIter> adj_cells;
	IntraAdjIter intra_iter;
	bool added;
	std::pair<IntraEdge, bool> intra_edge;
	bool result = false;

	for (auto pwcell_iter = cells.begin();
		pwcell_iter != cells.end(); ++pwcell_iter)
	{
		cell2 = pwcell_iter->lock();
		if (!cell2)
			continue;
		added = false;
		c2 = cell2->GetIntraVert();
		if (c2 == IntraGraph::null_vertex())
		{
			AddCellBin(cell_bins, *pwcell_iter);
			continue;
		}
		adj_cells = boost::adjacent_vertices(c2, intra_graph);
		//for each cell in contact
		for (intra_iter = adj_cells.first;
			intra_iter != adj_cells.second; ++intra_iter)
		{
			c2c = *intra_iter;
			if (FindCellBin(cells, intra_graph[c2c].cell))
			{
				intra_edge = boost::edge(c2, c2c, intra_graph);
				if (!intra_edge.second)
					continue;
				//continue if no contact
				if (intra_graph[intra_edge.first].size_ui == 0)
					continue;
				cell2c = intra_graph[c2c].cell.lock();
				if (!cell2c)
					continue;
				//meausre for merging
				if ((this->*merge_cell)(intra_edge.first,
					cell2, cell2c, intra_graph))
				{
					//add both to bin list
					added = AddCellBin(cell_bins,
						*pwcell_iter, intra_graph[c2c].cell);
					result |= added;
				}
			}
		}
		if (!added)//add to bin as well
			AddCellBin(cell_bins, *pwcell_iter);
	}

	return result;
}

bool TrackMapProcessor::EqualCells(pwCell &cell1, pwCell &cell2)
{
	return !cell1.owner_before(cell2) && !cell2.owner_before(cell1);
}

bool TrackMapProcessor::FindCellBin(CellBin &bin, pwCell &cell)
{
	for (size_t i = 0; i < bin.size(); ++i)
		if (EqualCells(bin[i], cell))
			return true;
	return false;
}

bool TrackMapProcessor::AddCellBin(std::vector<CellBin> &bins, pwCell &cell)
{
	bool found_cell;
	for (size_t i = 0; i < bins.size(); ++i)
	{
		found_cell = FindCellBin(bins[i], cell);
		if (found_cell)
			return false;
	}
	CellBin bin;
	bin.push_back(cell);
	bins.push_back(bin);
	return true;
}

bool TrackMapProcessor::AddCellBin(std::vector<CellBin> &bins, pwCell &cell1, pwCell &cell2)
{
	bool found_cell1, found_cell2;
	for (size_t i = 0; i < bins.size(); ++i)
	{
		found_cell1 = FindCellBin(bins[i], cell1);
		found_cell2 = FindCellBin(bins[i], cell2);
		if (found_cell1 && found_cell2)
			return true;
		else if (found_cell1 && !found_cell2)
		{
			pCell c2 = cell2.lock();
			if (!c2) continue;
			if (GreaterThanCellBin(c2, bins.at(i), cell1))
			{
				//adding large to small, check
				bins[i].push_back(cell2);
				return true;
			}
			else
				return false;
		}
		else if (!found_cell1 && found_cell2)
		{
			pCell c1 = cell1.lock();
			if (!c1) continue;
			if (GreaterThanCellBin(c1, bins.at(i), cell2))
			{
				bins[i].push_back(cell1);
				return true;
			}
			else
				return false;
		}
	}
	CellBin bin;
	bin.push_back(cell1);
	bin.push_back(cell2);
	bins.push_back(bin);
	return true;
}

bool TrackMapProcessor::GreaterThanCellBin(pCell &cell1, CellBin &bin, pwCell &cell2)
{
	pCell bin_cell;
	for (size_t i = 0; i < bin.size(); ++i)
	{
		if (EqualCells(bin[i], cell2))
			continue;
		bin_cell = bin[i].lock();
		if (!bin_cell)
			continue;
		if (cell1->GetSizeF() < bin_cell->GetSizeF()*3.0f)
			return false;
	}
	return true;
}

size_t TrackMapProcessor::GetBinsCellCount(std::vector<CellBin> &bins)
{
	size_t count = 0;
	for (size_t i = 0; i < bins.size(); ++i)
		count += bins[i].size();
	return count;
}

bool TrackMapProcessor::MergeCells(
	VertexList& vertex_list, CellBin &bin, size_t frame)
{
	if (bin.size() <= 1)
		return false;

	//the keeper
	pCell cell0, cell;
	pVertex vertex0, vertex;
	VertexListIter vert_iter;
	CellBin cell_list;
	CellBinIter cell_iter;

	for (size_t i = 0; i < bin.size(); ++i)
	{
		cell = bin[i].lock();
		if (!cell)
			continue;
		if (!vertex0)
		{
			cell0 = cell;
			vertex0 = cell0->GetVertex().lock();
		}
		else
		{
			vertex = cell->GetVertex().lock();
			if (!vertex ||
				vertex->Id() == vertex0->Id())
				continue;

			//relink inter graph
			if (frame > 0)
			{
				InterGraph &graph = m_map.m_inter_graph_list.at(frame - 1);
				RelinkInterGraph(vertex, vertex0, frame, graph, false);
			}
			if (frame < m_map.m_frame_num - 1)
			{
				InterGraph &graph = m_map.m_inter_graph_list.at(frame);
				RelinkInterGraph(vertex, vertex0, frame, graph, false);
			}

			//collect cells from vertex
			for (cell_iter = vertex->GetCellsBegin();
				cell_iter != vertex->GetCellsEnd();
				++cell_iter)
				cell_list.push_back(*cell_iter);

			//remove vertex from list
			vert_iter = vertex_list.find(vertex->Id());
			if (vert_iter != vertex_list.end())
				vertex_list.erase(vert_iter);

			//add cell to vertex0
			for (cell_iter = cell_list.begin();
				cell_iter != cell_list.end();
				++cell_iter)
			{
				cell = (*cell_iter).lock();
				if (cell)
				{
					vertex0->AddCell(cell, true);
					cell->AddVertex(vertex0);
				}
			}
		}
	}

	return true;
}

bool TrackMapProcessor::RelinkInterGraph(pVertex &vertex, pVertex &vertex0, size_t frame, InterGraph &graph, bool reset)
{
	InterVert inter_vert, inter_vert0;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	std::pair<InterEdge, bool> e, e0;
	std::vector<InterEdge> edges_to_remove;
	std::vector<InterEdge>::iterator edge_to_remove;

	inter_vert = vertex->GetInterVert(graph);
	if (inter_vert != InterGraph::null_vertex())
	{
		adj_verts = boost::adjacent_vertices(inter_vert, graph);
		edges_to_remove.clear();
		//for each adjacent vertex
		for (inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
		{
			//get edge
			e = boost::edge(*inter_iter, inter_vert, graph);
			if (!e.second)
				continue;
			//add an edge between vertex0 and inter_iter
			inter_vert0 = vertex0->GetInterVert(graph);
			if (inter_vert0 == InterGraph::null_vertex())
			{
				inter_vert0 = boost::add_vertex(graph);
				graph[inter_vert0].id = vertex0->Id();
				graph[inter_vert0].frame = frame;
				graph[inter_vert0].count = 0;
				graph[inter_vert0].vertex = vertex0;
				vertex0->SetInterVert(graph, inter_vert0);
			}
			if (reset)
				graph[inter_vert0].count = 0;
			else
				graph[inter_vert0].count += graph[inter_vert].count;
			e0 = boost::edge(*inter_iter,
				inter_vert0, graph);
			if (!e0.second)
			{
				e0 = boost::add_edge(*inter_iter,
					inter_vert0, graph);
				graph[e0.first].size_ui = graph[e.first].size_ui;
				graph[e0.first].size_f = graph[e.first].size_f;
				graph[e0.first].dist_f = graph[e.first].dist_f;
				graph[e0.first].link = graph[e.first].link;
				graph[e0.first].count = graph[e.first].count;
			}
			else
			{
				graph[e0.first].size_ui += graph[e.first].size_ui;
				graph[e0.first].size_f += graph[e.first].size_f;
				graph[e0.first].count += graph[e.first].count;
			}
			//delete the old edge
			edges_to_remove.push_back(e.first);
		}
		//remove edges
		for (edge_to_remove = edges_to_remove.begin();
		edge_to_remove != edges_to_remove.end();
			++edge_to_remove)
		{
			graph.remove_edge(*edge_to_remove);
		}
		//remove the vertex from inter graph
		//edges are NOT removed!
		boost::remove_vertex(inter_vert, graph);
	}

	return true;
}

bool TrackMapProcessor::RemoveVertex(InterGraph& graph, pVertex &vertex)
{
	InterVert inter_vert = vertex->GetInterVert(graph);
	if (inter_vert != InterGraph::null_vertex())
	{
		std::pair<InterAdjIter, InterAdjIter> adj_verts;
		std::vector<InterEdge> edges_to_remove;
		std::vector<InterEdge>::iterator edge_to_remove;
		InterAdjIter inter_iter;
		adj_verts = boost::adjacent_vertices(inter_vert, graph);
		std::pair<InterEdge, bool> e;
		for (inter_iter = adj_verts.first;
			inter_iter != adj_verts.second; ++inter_iter)
		{
			//get edge
			e = boost::edge(*inter_iter, inter_vert, graph);
			if (!e.second)
				continue;
			//delete the old edge
			edges_to_remove.push_back(e.first);
		}
		//remove edges
		for (edge_to_remove = edges_to_remove.begin();
			edge_to_remove != edges_to_remove.end();
			++edge_to_remove)
			graph.remove_edge(*edge_to_remove);
		//remove the vertex from inter graph
		//edges are NOT removed!
		boost::remove_vertex(inter_vert, graph);
		vertex->SetInterVert(graph, 0);
		//it needs to be actually removed from the list later
		return true;
	}
	return false;
}

bool TrackMapProcessor::Export(std::string &filename)
{
	if (m_map.m_frame_num == 0 ||
		m_map.m_frame_num != m_map.m_cells_list.size() ||
		m_map.m_frame_num != m_map.m_vertices_list.size() ||
		m_map.m_frame_num != m_map.m_inter_graph_list.size() + 1)
		return false;

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	if (ofs.bad())
		return false;

	//header
	std::string header = "FluoRender links";
	ofs.write(header.c_str(), header.size());

	//last operation
	WriteTag(ofs, TAG_FPCOUNT);
	WriteUint(ofs, m_map.m_counter);

	//number of frames
	WriteTag(ofs, TAG_NUM);
	size_t num = m_map.m_frame_num;
	WriteUint(ofs, num);

	VertexListIter iter;
	pVertex vertex;
	IntraEdgeIter intra_iter;
	std::pair<IntraEdgeIter, IntraEdgeIter> intra_pair;
	IntraVert intra_vert;
	InterEdgeIter inter_iter;
	std::pair<InterEdgeIter, InterEdgeIter> inter_pair;
	InterVert inter_vert0, inter_vert1;
	size_t edge_num;
	//write each frame
	for (size_t i = 0; i < num; ++i)
	{
		WriteTag(ofs, TAG_FRAM);
		//frame id
		WriteUint(ofs, i);

		//vertex list
		VertexList &vertex_list = m_map.m_vertices_list.at(i);
		//vertex number
		WriteUint(ofs, vertex_list.size());
		//write each vertex
		for (iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
		{
			vertex = iter->second;
			WriteVertex(ofs, vertex);
		}
		//write intra edges
		IntraGraph &intra_graph = m_map.m_intra_graph_list.at(i);
		intra_pair = edges(intra_graph);
		edge_num = 0;
		for (intra_iter = intra_pair.first;
		intra_iter != intra_pair.second;
			++intra_iter)
			edge_num++;
		//intra edge num
		WriteUint(ofs, edge_num);
		//write each intra edge
		for (intra_iter = intra_pair.first;
		intra_iter != intra_pair.second;
			++intra_iter)
		{
			WriteTag(ofs, TAG_INTRA_EDGE);
			//first cell
			intra_vert = boost::source(*intra_iter, intra_graph);
			WriteUint(ofs, intra_graph[intra_vert].id);
			//second cell
			intra_vert = boost::target(*intra_iter, intra_graph);
			WriteUint(ofs, intra_graph[intra_vert].id);
			//size
			WriteUint(ofs, intra_graph[*intra_iter].size_ui);
			WriteFloat(ofs, intra_graph[*intra_iter].size_f);
			//distance
			WriteTag(ofs, TAG_VER220);
			WriteFloat(ofs, intra_graph[*intra_iter].dist_v);
			WriteFloat(ofs, intra_graph[*intra_iter].dist_s);
		}
		//write inter edges
		if (i == 0)
			continue;
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(i - 1);
		//write index and counter
		WriteTag(ofs, TAG_VER220);
		//index
		WriteUint(ofs, inter_graph.index);
		//counter
		WriteUint(ofs, inter_graph.counter);
		//get edge number
		inter_pair = boost::edges(inter_graph);
		edge_num = 0;
		for (inter_iter = inter_pair.first;
		inter_iter != inter_pair.second;
			++inter_iter)
			edge_num++;
		//inter edge number
		WriteUint(ofs, edge_num);
		//write each inter edge
		for (inter_iter = inter_pair.first;
		inter_iter != inter_pair.second;
			++inter_iter)
		{
			WriteTag(ofs, TAG_INTER_EDGE);
			inter_vert0 = boost::source(*inter_iter, inter_graph);
			inter_vert1 = boost::target(*inter_iter, inter_graph);
			if (inter_graph[inter_vert0].frame <
				inter_graph[inter_vert1].frame)
			{
				//first vertex
				WriteUint(ofs, inter_graph[inter_vert0].id);
				//second vertex
				WriteUint(ofs, inter_graph[inter_vert1].id);
			}
			else
			{
				//first vertex
				WriteUint(ofs, inter_graph[inter_vert1].id);
				//second vertex
				WriteUint(ofs, inter_graph[inter_vert0].id);
			}
			//size
			WriteUint(ofs, inter_graph[*inter_iter].size_ui);
			WriteFloat(ofs, inter_graph[*inter_iter].size_f);
			WriteFloat(ofs, inter_graph[*inter_iter].dist_f);
			WriteUint(ofs, inter_graph[*inter_iter].link);
			//uncertainty
			WriteTag(ofs, TAG_VER219);
			if (inter_graph[inter_vert0].frame <
				inter_graph[inter_vert1].frame)
			{
				//first vertex
				WriteUint(ofs, inter_graph[inter_vert0].count);
				//second vertex
				WriteUint(ofs, inter_graph[inter_vert1].count);
			}
			else
			{
				//first vertex
				WriteUint(ofs, inter_graph[inter_vert1].count);
				//second vertex
				WriteUint(ofs, inter_graph[inter_vert0].count);
			}
			WriteUint(ofs, inter_graph[*inter_iter].count);
		}
	}

	return true;
}

bool TrackMapProcessor::Import(std::string &filename)
{
	//clear everything
	m_map.Clear();

	std::ifstream ifs(filename, std::ios::in | std::ios::binary);
	if (ifs.bad())
		return false;

	//header
	char cheader[17];
	ifs.read(cheader, 16);
	cheader[16] = 0;
	std::string header = cheader;
	if (header != "FluoRender links")
		return false;

	//last operation
	if (ReadTag(ifs) == TAG_FPCOUNT)
		m_map.m_counter = ReadUint(ifs);
	else
		ifs.unget();

	//number of frames
	size_t num;
	if (ReadTag(ifs) == TAG_NUM)
		num = ReadUint(ifs);
	else
	{
		ifs.unget();
		num = ReadUint(ifs);
	}


	size_t vertex_num;
	size_t edge_num;
	unsigned id1, id2;
	pCell cell1, cell2;
	CellListIter cell_iter;
	unsigned int size_ui;
	float size_f;
	float dist_v, dist_s;
	pVertex vertex1, vertex2;
	VertexListIter vertex_iter;
	float dist;
	unsigned int link;
	bool edge_exist;
	//read each frame
	for (size_t i = 0; i < num; ++i)
	{
		if (ReadTag(ifs) != TAG_FRAM)
			return false;
		//frame id
		ReadUint(ifs);

		//vertex list
		m_map.m_vertices_list.push_back(VertexList());
		VertexList &vertex_list1 = m_map.m_vertices_list.back();
		//cell list
		m_map.m_cells_list.push_back(CellList());
		CellList &cell_list = m_map.m_cells_list.back();
		//vertex number
		vertex_num = ReadUint(ifs);
		//read each vertex
		for (size_t j = 0; j < vertex_num; ++j)
			ReadVertex(ifs, vertex_list1, cell_list);
		//intra graph
		m_map.m_intra_graph_list.push_back(IntraGraph());
		IntraGraph &intra_graph = m_map.m_intra_graph_list.back();
		//intra edge num
		edge_num = ReadUint(ifs);
		//read each intra edge
		for (size_t j = 0; j < edge_num; ++j)
		{
			edge_exist = true;
			if (ReadTag(ifs) != TAG_INTRA_EDGE)
				return false;
			//first cell
			id1 = ReadUint(ifs);
			cell_iter = cell_list.find(id1);
			if (cell_iter == cell_list.end())
				edge_exist = false;
			else
				cell1 = cell_iter->second;
			//second cell
			id2 = ReadUint(ifs);
			cell_iter = cell_list.find(id2);
			if (cell_iter == cell_list.end())
				edge_exist = false;
			else
				cell2 = cell_iter->second;
			//add edge
			size_ui = ReadUint(ifs);
			size_f = ReadFloat(ifs);
			if (ReadTag(ifs) == TAG_VER220)
			{
				dist_v = ReadFloat(ifs);
				dist_s = ReadFloat(ifs);
			}
			else
			{
				ifs.unget();
				dist_v = dist_s = 0.0f;
			}
			if (edge_exist)
				AddIntraEdge(intra_graph, cell1, cell2,
					size_ui, size_f, dist_v, dist_s);
		}
		//inter graph
		if (i == 0)
			continue;
		//old vertex list
		VertexList &vertex_list0 = m_map.m_vertices_list.at(i - 1);
		m_map.m_inter_graph_list.push_back(InterGraph());
		InterGraph &inter_graph = m_map.m_inter_graph_list.back();
		//read index and counter
		if (ReadTag(ifs) == TAG_VER220)
		{
			//index
			inter_graph.index = ReadUint(ifs);
			//counter
			inter_graph.counter = ReadUint(ifs);
		}
		else
		{
			ifs.unget();
			inter_graph.index = i - 1;
			inter_graph.counter = 0;
		}
		//inter edge num
		edge_num = ReadUint(ifs);
		//read each inter edge
		for (size_t j = 0; j < edge_num; ++j)
		{
			edge_exist = true;
			if (ReadTag(ifs) != TAG_INTER_EDGE)
				return false;
			//first vertex
			id1 = ReadUint(ifs);
			vertex_iter = vertex_list0.find(id1);
			if (vertex_iter == vertex_list0.end())
				edge_exist = false;
			else
				vertex1 = vertex_iter->second;
			//second vertex
			id2 = ReadUint(ifs);
			vertex_iter = vertex_list1.find(id2);
			if (vertex_iter == vertex_list1.end())
				edge_exist = false;
			else
				vertex2 = vertex_iter->second;
			//add edge
			size_ui = ReadUint(ifs);
			size_f = ReadFloat(ifs);
			dist = ReadFloat(ifs);
			link = ReadUint(ifs);
			unsigned int v1_count = 0;
			unsigned int v2_count = 0;
			unsigned int edge_count = 0;
			if (ReadTag(ifs) == TAG_VER219)
			{
				v1_count = ReadUint(ifs);
				v2_count = ReadUint(ifs);
				edge_count = ReadUint(ifs);
			}
			else
				ifs.unget();
			if (edge_exist)
				AddInterEdge(inter_graph, vertex1, vertex2,
					i-1, i, size_ui, size_f, dist, link,
					v1_count, v2_count, edge_count);
		}
	}

	m_map.m_frame_num = num;
	return true;
}

bool TrackMapProcessor::ResetVertexIDs()
{
	for (size_t fi = 0; fi < m_map.m_frame_num; ++fi)
	{
		VertexList &vertex_list = m_map.m_vertices_list.at(fi);
		for (VertexListIter vertex_iter = vertex_list.begin();
		vertex_iter != vertex_list.end(); ++vertex_iter)
		{
			pVertex vertex = vertex_iter->second;
			if (vertex->GetCellNum() <= 1)
				continue;
			unsigned int max_id = 0;
			float max_size = 0.0f;
			for (CellBinIter cell_iter = vertex->GetCellsBegin();
			cell_iter != vertex->GetCellsEnd(); ++cell_iter)
			{
				pCell cell = cell_iter->lock();
				if (cell)
				{
					if (cell->GetSizeF() > max_size)
					{
						max_size = cell->GetSizeF();
						max_id = cell->Id();
					}
				}
			}
			if (max_id)
			{
				vertex->Id(max_id);
				if (fi > 0)
				{
					InterGraph &inter_graph = m_map.m_inter_graph_list.at(fi - 1);
					InterVert inter_vert = vertex->GetInterVert(inter_graph);
					if (inter_vert != InterGraph::null_vertex())
						inter_graph[inter_vert].id = max_id;
				}
				if (fi < m_map.m_frame_num - 1)
				{
					InterGraph &inter_graph = m_map.m_inter_graph_list.at(fi);
					InterVert inter_vert = vertex->GetInterVert(inter_graph);
					if (inter_vert != InterGraph::null_vertex())
						inter_graph[inter_vert].id = max_id;
				}
			}
		}
	}

	return true;
}

void TrackMapProcessor::WriteVertex(std::ofstream& ofs, const pVertex &vertex)
{
	WriteTag(ofs, TAG_VERT);
	WriteUint(ofs, vertex->Id());
	WriteUint(ofs, vertex->GetSizeUi());
	WriteUint(ofs, vertex->GetSizeF());
	WritePoint(ofs, vertex->GetCenter());
	//cell number
	WriteUint(ofs, vertex->GetCellNum());

	//cells
	CellBinIter iter;
	pCell cell;
	for (iter = vertex->GetCellsBegin();
	iter != vertex->GetCellsEnd(); ++iter)
	{
		cell = iter->lock();
		if (!cell)
			continue;
		WriteCell(ofs, cell);
	}
}

void TrackMapProcessor::ReadVertex(std::ifstream& ifs,
	VertexList& vertex_list, CellList& cell_list)
{
	if (ReadTag(ifs) != TAG_VERT)
		return;

	unsigned int id = ReadUint(ifs);
	if (vertex_list.find(id) != vertex_list.end())
		return;

	pVertex vertex;
	vertex = pVertex(new Vertex(id));
	vertex->SetSizeUi(ReadUint(ifs));
	vertex->SetSizeF(ReadFloat(ifs));
	FLIVR::Point p = ReadPoint(ifs);
	vertex->SetCenter(p);

	//cell number
	unsigned int num = ReadUint(ifs);
	//cells
	pCell cell;
	for (unsigned int i = 0; i < num; ++i)
	{
		cell = ReadCell(ifs, cell_list);
		vertex->AddCell(cell);
		cell->AddVertex(vertex);
	}

	vertex_list.insert(std::pair<unsigned int, pVertex>
		(id, vertex));
}

bool TrackMapProcessor::AddIntraEdge(IntraGraph& graph,
	pCell &cell1, pCell &cell2,
	unsigned int size_ui, float size_f,
	float dist_v, float dist_s)
{
	IntraVert v1 = cell1->GetIntraVert();
	IntraVert v2 = cell2->GetIntraVert();
	if (v1 == IntraGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = cell1->Id();
		graph[v1].cell = cell1;
		cell1->SetIntraVert(v1);
	}
	if (v2 == IntraGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = cell2->Id();
		graph[v2].cell = cell2;
		cell2->SetIntraVert(v2);
	}

	std::pair<IntraEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = size_ui;
		graph[e.first].size_f = size_f;
		graph[e.first].dist_v = dist_v;
		graph[e.first].dist_s = dist_s;
	}
	else
		return false;

	return true;
}

bool TrackMapProcessor::AddInterEdge(InterGraph& graph,
	pVertex &vertex1, pVertex &vertex2,
	size_t f1, size_t f2,
	unsigned int size_ui, float size_f,
	float dist_f, unsigned int link,
	unsigned int v1_count,
	unsigned int v2_count,
	unsigned int edge_count)
{
	InterVert v1 = vertex1->GetInterVert(graph);
	InterVert v2 = vertex2->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex1->Id();
		graph[v1].frame = f1;
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	graph[v1].count = v1_count;
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = f2;
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}
	graph[v2].count = v2_count;

	std::pair<InterEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = size_ui;
		graph[e.first].size_f = size_f;
		graph[e.first].dist_f = dist_f;
		graph[e.first].link = link;
		graph[e.first].count = edge_count;
	}
	else
	{
		graph[e.first].count = edge_count;
		return false;
	}

	return true;
}

bool TrackMapProcessor::GetMappedID(
	unsigned int id_in, unsigned int& id_out,
	size_t frame)
{
	size_t frame_num = m_map.m_frame_num;
	if (frame >= frame_num)
		return false;

	CellList &cell_list = m_map.m_cells_list.at(frame);

	CellListIter cell_iter;
	pVertex vertex;
	cell_iter = cell_list.find(id_in);
	if (cell_iter == cell_list.end())
		return false;
	vertex = cell_iter->second->GetVertex().lock();
	if (!vertex)
		return false;

	pCell cell = (*vertex->GetCellsBegin()).lock();
	if (!cell)
		return false;

	id_out = cell->Id();
	return true;
}

bool TrackMapProcessor::GetMappedID(
	unsigned int id_in, unsigned int& id_out,
	size_t frame1, size_t frame2)
{
	bool result = false;
	size_t frame_num = m_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	CellList &cell_list1 = m_map.m_cells_list.at(frame1);
	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	CellListIter cell_iter;
	pVertex vertex1, vertex2;
	pCell cell;
	InterVert v1, v2;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	CellBinIter pwcell_iter;
	std::pair<InterEdge, bool> inter_edge;
	float in_size;
	float out_size, min_diff;

	cell_iter = cell_list1.find(id_in);
	if (cell_iter == cell_list1.end())
		return false;
	vertex1 = cell_iter->second->GetVertex().lock();
	if (!vertex1)
		return false;
	in_size = vertex1->GetSizeF();
	v1 = vertex1->GetInterVert(inter_graph);
	if (v1 == InterGraph::null_vertex())
		return false;

	min_diff = std::numeric_limits<float>::max();
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

		//find closest size
		out_size = vertex2->GetSizeF();
		if (fabs(out_size - in_size) < min_diff)
		{
			cell = (*vertex2->GetCellsBegin()).lock();
			if (!cell)
				continue;
			id_out = cell->Id();
			min_diff = fabs(out_size - in_size);
			result = true;
		}
	}

	return result;
}

bool TrackMapProcessor::GetMappedCells(
	CellList &sel_list1, CellList &sel_list2,
	size_t frame1, size_t frame2)
{
	size_t frame_num = m_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	CellList &cell_list1 = m_map.m_cells_list.at(frame1);
	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	CellListIter sel_iter, cell_iter;
	pVertex vertex1, vertex2;
	pCell cell;
	InterVert v1, v2;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	CellBinIter pwcell_iter;
	std::pair<InterEdge, bool> inter_edge;

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
		if (v1 == InterGraph::null_vertex())
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
				sel_list2.insert(std::pair<unsigned int, pCell>
					(cell->Id(), cell));
			}
		}
	}

	return true;
}

//modifications
bool TrackMapProcessor::LinkCells(
	CellList &list1, CellList &list2,
	size_t frame1, size_t frame2,
	bool exclusive)
{
	//check validity
	if ((frame2 != frame1 + 1 &&
		frame2 != frame1 - 1) ||
		!m_map.ExtendFrameNum(
		std::max(frame1, frame2)))
		return false;

	VertexList vlist1, vlist2;
	CellListIter citer1, citer2;

	CellList &cell_list1 = m_map.m_cells_list.at(frame1);
	CellList &cell_list2 = m_map.m_cells_list.at(frame2);
	CellListIter cell;
	unsigned int cell_id;

	for (citer1 = list1.begin();
	citer1 != list1.end(); ++citer1)
	{
		cell_id = citer1->second->Id();
		cell = cell_list1.find(cell_id);
		if (cell == cell_list1.end())
			AddCell(citer1->second, frame1, cell);
		else
			cell->second->Set(citer1->second);
		pVertex vert1 = cell->second->GetVertex().lock();
		if (vert1)
		{
			vert1->Update();
			vlist1.insert(std::pair<unsigned int, pVertex>
				(vert1->Id(), vert1));
		}
	}
	for (citer2 = list2.begin();
	citer2 != list2.end(); ++citer2)
	{
		cell_id = citer2->second->Id();
		cell = cell_list2.find(cell_id);
		if (cell == cell_list2.end())
			AddCell(citer2->second, frame2, cell);
		else
			cell->second->Set(citer2->second);
		pVertex vert2 = cell->second->GetVertex().lock();
		if (vert2)
		{
			vert2->Update();
			vlist2.insert(std::pair<unsigned int, pVertex>
				(vert2->Id(), vert2));
		}
	}

	if (vlist1.size() == 0 ||
		vlist2.size() == 0)
		return false;

	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	
	VertexListIter viter1, viter2;

	if (exclusive)
	{
		for (viter1 = vlist1.begin();
		viter1 != vlist1.end(); ++viter1)
			IsolateVertex(inter_graph, viter1->second);
		for (viter2 = vlist2.begin();
		viter2 != vlist2.end(); ++viter2)
			IsolateVertex(inter_graph, viter2->second);
	}

	for (viter1 = vlist1.begin();
	viter1 != vlist1.end(); ++viter1)
	for (viter2 = vlist2.begin();
	viter2 != vlist2.end(); ++viter2)
		ForceVertices(inter_graph,
			viter1->second, viter2->second,
			frame1, frame2);

	return true;
}

bool TrackMapProcessor::LinkCells(pCell &cell1, pCell &cell2,
	size_t frame1, size_t frame2, bool exclusive)
{
	//check validity
	if ((frame2 != frame1 + 1 &&
		frame2 != frame1 - 1) ||
		!m_map.ExtendFrameNum(
			std::max(frame1, frame2)))
		return false;

	VertexList vlist1, vlist2;

	CellList &cell_list1 = m_map.m_cells_list.at(frame1);
	CellList &cell_list2 = m_map.m_cells_list.at(frame2);
	CellListIter cell;
	unsigned int cell_id;

	cell_id = cell1->Id();
	cell = cell_list1.find(cell_id);
	if (cell == cell_list1.end())
		AddCell(cell1, frame1, cell);
	pVertex vert1 = cell->second->GetVertex().lock();

	cell_id = cell2->Id();
	cell = cell_list2.find(cell_id);
	if (cell == cell_list2.end())
		AddCell(cell2, frame2, cell);
	pVertex vert2 = cell->second->GetVertex().lock();

	if (!vert1 || !vert2)
		return false;

	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	if (exclusive)
	{
		IsolateVertex(inter_graph, vert1);
		IsolateVertex(inter_graph, vert2);
	}

	ForceVertices(inter_graph,
		vert1, vert2,
		frame1, frame2);

	return true;
}

bool TrackMapProcessor::IsolateCells(
	CellList &list, size_t frame)
{
	//check validity
	size_t frame_num = m_map.m_frame_num;
	if (frame >= frame_num)
		return false;

	VertexList vlist;
	CellListIter citer;

	CellList &cell_list = m_map.m_cells_list.at(frame);
	CellListIter cell;

	for (citer = list.begin();
	citer != list.end(); ++citer)
	{
		cell = cell_list.find(citer->second->Id());
		if (cell == cell_list.end())
			continue;
		pVertex vert = cell->second->GetVertex().lock();
		if (vert)
			vlist.insert(std::pair<unsigned int, pVertex>
				(vert->Id(), vert));
	}

	if (vlist.size() == 0)
		return false;

	if (frame > 0)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame - 1);
		VertexListIter viter;
		for (viter = vlist.begin();
		viter != vlist.end(); ++viter)
			IsolateVertex(inter_graph, viter->second);
	}
	if (frame < frame_num - 1)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame);
		VertexListIter viter;
		for (viter = vlist.begin();
		viter != vlist.end(); ++viter)
			IsolateVertex(inter_graph, viter->second);
	}

	return true;
}

bool TrackMapProcessor::UnlinkCells(
	CellList &list1, CellList &list2,
	size_t frame1, size_t frame2)
{
	//check validity
	size_t frame_num = m_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		(frame2 != frame1 + 1 &&
			frame2 != frame1 - 1))
		return false;

	VertexList vlist1, vlist2;
	CellListIter citer1, citer2;

	CellList &cell_list1 = m_map.m_cells_list.at(frame1);
	CellList &cell_list2 = m_map.m_cells_list.at(frame2);
	CellListIter cell;

	for (citer1 = list1.begin();
	citer1 != list1.end(); ++citer1)
	{
		cell = cell_list1.find(citer1->second->Id());
		if (cell == cell_list1.end())
			continue;
		pVertex vert1 = cell->second->GetVertex().lock();
		if (vert1)
			vlist1.insert(std::pair<unsigned int, pVertex>
				(vert1->Id(), vert1));
	}
	for (citer2 = list2.begin();
	citer2 != list2.end(); ++citer2)
	{
		cell = cell_list2.find(citer2->second->Id());
		if (cell == cell_list2.end())
			continue;
		pVertex vert2 = cell->second->GetVertex().lock();
		if (vert2)
			vlist2.insert(std::pair<unsigned int, pVertex>
				(vert2->Id(), vert2));
	}

	if (vlist1.size() == 0 ||
		vlist2.size() == 0)
		return false;

	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	VertexListIter viter1, viter2;

	for (viter1 = vlist1.begin();
	viter1 != vlist1.end(); ++viter1)
		for (viter2 = vlist2.begin();
	viter2 != vlist2.end(); ++viter2)
			UnlinkVertices(inter_graph,
				viter1->second, viter2->second);

	return true;
}

bool TrackMapProcessor::AddCell(
	pCell &cell, size_t frame, CellListIter &iter)
{
	//check validity
	if (!m_map.ExtendFrameNum(frame))
		return false;

	CellList &cell_list = m_map.m_cells_list.at(frame);
	VertexList &vert_list = m_map.m_vertices_list.at(frame);

	if (cell_list.find(cell->Id()) != cell_list.end() ||
		vert_list.find(cell->Id()) != vert_list.end())
		return true;

	pVertex vertex(new Vertex(cell->Id()));
	vertex->SetCenter(cell->GetCenter());
	vertex->SetSizeUi(cell->GetSizeUi());
	vertex->SetSizeF(cell->GetSizeF());
	vertex->AddCell(cell);
	cell->AddVertex(vertex);
	vert_list.insert(std::pair<unsigned int, pVertex>
		(vertex->Id(), vertex));
	std::pair<CellListIter, bool> result = cell_list.insert(
		std::pair<unsigned int, pCell>(cell->Id(), cell));
	iter = result.first;
	return true;
}

bool TrackMapProcessor::AddCells(CellList &list, size_t frame)
{
	//check validity
	if (!m_map.ExtendFrameNum(frame))
		return false;

	CellList &cell_list = m_map.m_cells_list.at(frame);
	VertexList &vert_list = m_map.m_vertices_list.at(frame);

	for (CellListIter iter = list.begin();
		iter != list.end(); ++iter)
	{
		pCell cell = iter->second;
		if (cell_list.find(cell->Id()) != cell_list.end() ||
			vert_list.find(cell->Id()) != vert_list.end())
			continue;

		pVertex vertex(new Vertex(cell->Id()));
		vertex->SetCenter(cell->GetCenter());
		vertex->SetSizeUi(cell->GetSizeUi());
		vertex->SetSizeF(cell->GetSizeF());
		vertex->AddCell(cell);
		cell->AddVertex(vertex);
		vert_list.insert(std::pair<unsigned int, pVertex>
			(vertex->Id(), vertex));
		cell_list.insert(std::pair<unsigned int, pCell>
			(cell->Id(), cell));
	}

	return true;
}

bool TrackMapProcessor::LinkAddedCells(CellList &list, size_t f1, size_t f2)
{
	size_t frame_num = m_map.m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num || f1 == f2)
		return false;

	//get data and label
	VolCache cache = m_vol_cache.get(f1);
	void* data1 = cache.data;
	void* label1 = cache.label;
	if (!data1 || !label1)
		return false;
	cache = m_vol_cache.get(f2);
	void* data2 = cache.data;
	void* label2 = cache.label;
	if (!data2 || !label2)
		return false;

	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		f1 > f2 ? f2 : f1);
	VertexList &vertex_list1 = m_map.m_vertices_list.at(f1);
	VertexList &vertex_list2 = m_map.m_vertices_list.at(f2);
	VertexListIter iter1, iter2;

	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	size_t minx, miny, minz;
	size_t maxx, maxy, maxz;
	float data_value1, data_value2;
	unsigned int label_value1, label_value2;

	for (CellListIter cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		pCell cell = cliter->second;
		unsigned int cid = cell->Id();

		minx = size_t(cell->GetBox().min().x() + 0.5);
		miny = size_t(cell->GetBox().min().y() + 0.5);
		minz = size_t(cell->GetBox().min().z() + 0.5);
		maxx = size_t(cell->GetBox().max().x() + 0.5);
		maxy = size_t(cell->GetBox().max().y() + 0.5);
		maxz = size_t(cell->GetBox().max().z() + 0.5);
		for (i = minx; i <= maxx; ++i)
		for (j = miny; j <= maxy; ++j)
		for (k = minz; k <= maxz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value1 = ((unsigned int*)label1)[index];
			label_value2 = ((unsigned int*)label2)[index];

			if (label_value1 != cid ||
				!label_value2)
				continue;

			if (m_map.m_data_bits == 8)
			{
				data_value1 = ((unsigned char*)data1)[index] / 255.0f;
				data_value2 = ((unsigned char*)data2)[index] / 255.0f;
			}
			else if (m_map.m_data_bits == 16)
			{
				data_value1 = ((unsigned short*)data1)[index] * m_map.m_scale / 65535.0f;
				data_value2 = ((unsigned short*)data2)[index] * m_map.m_scale / 65535.0f;
			}

			iter1 = vertex_list1.find(label_value1);
			iter2 = vertex_list2.find(label_value2);

			if (iter1 == vertex_list1.end() ||
				iter2 == vertex_list2.end())
				continue;

			if (iter1->second->GetSizeF() < m_size_thresh ||
				iter2->second->GetSizeF() < m_size_thresh)
				continue;

			LinkVertices(inter_graph,
				iter1->second, iter2->second,
				f1, f2,
				std::min(data_value1, data_value2));
		}
	}

	return true;
}

bool TrackMapProcessor::CombineCells(
	pCell &cell, CellList &list, size_t frame)
{
	//check validity
	if (!m_map.ExtendFrameNum(frame))
		return false;

	CellList &cell_list = m_map.m_cells_list.at(frame);
	VertexList &vert_list = m_map.m_vertices_list.at(frame);

	//find the largest cell
	CellListIter cell_iter = cell_list.find(cell->Id());
	if (cell_iter == cell_list.end())
		return false;
	pCell cell0 = cell_iter->second;
	pVertex vertex0 = cell0->GetVertex().lock();

	//add each cell to cell0
	for (cell_iter = list.begin();
	cell_iter != list.end(); ++cell_iter)
	{
		CellListIter iter = cell_list.find(cell_iter->second->Id());
		if (iter == cell_list.end() ||
			iter->second->Id() == cell0->Id())
			continue;
		pCell cell1 = iter->second;
		pVertex vertex1 = cell1->GetVertex().lock();
		cell0->Inc(cell1);
		//relink vertex
		if (vertex0 && vertex1)
		{
			//remove cell1 from vertex1
			vertex1->RemoveCell(cell1);
			if (vertex1->GetCellNum() == 0)
			{
				//relink inter graph
				if (frame > 0)
				{
					InterGraph &graph = m_map.m_inter_graph_list.at(frame - 1);
					RelinkInterGraph(vertex1, vertex0, frame, graph, true);
				}
				if (frame < m_map.m_frame_num - 1)
				{
					InterGraph &graph = m_map.m_inter_graph_list.at(frame);
					RelinkInterGraph(vertex1, vertex0, frame, graph, true);
				}

				//erase from list
				vert_list.erase(vertex1->Id());
			}
		}
		//remove from list
		cell_list.erase(iter);
	}

	//update information
	if (vertex0)
		vertex0->Update();

	return true;
}

bool TrackMapProcessor::DivideCells(
	CellList &list, size_t frame)
{
	//check validity
	if (!m_map.ExtendFrameNum(frame))
		return false;

	CellList &cell_list = m_map.m_cells_list.at(frame);
	VertexList &vert_list = m_map.m_vertices_list.at(frame);

	//temporary vertex list
	VertexList vlist;
	VertexListIter vert_iter;
	CellListIter cell_iter;
	for (cell_iter = list.begin();
	cell_iter != list.end(); ++cell_iter)
	{
		CellListIter iter = cell_list.find(cell_iter->second->Id());
		if (iter == cell_list.end())
			continue;
		pCell cell = iter->second;
		pVertex vertex = cell->GetVertex().lock();
		if (cell && vertex)
		{
			vert_iter = vlist.find(vertex->Id());
			if (vert_iter == vlist.end())
			{
				pVertex v = pVertex(new Vertex(vertex->Id()));
				vlist.insert(std::pair<unsigned int, pVertex>(
					v->Id(), v));
				v->AddCell(cell_iter->second);
			}
			else
			{
				vert_iter->second->AddCell(cell_iter->second);
			}
		}
	}

	for (vert_iter = vlist.begin();
	vert_iter != vlist.end(); ++vert_iter)
	{
		pVertex vertex = vert_iter->second;
		if (vertex->GetCellNum() <= 1)
			continue;

		unsigned int max_size = 0;
		unsigned int max_id = 0;
		for (CellBinIter iter = vertex->GetCellsBegin();
		iter != vertex->GetCellsEnd(); ++iter)
		{
			pCell cell = iter->lock();
			if (!cell)
				continue;
			if (cell->GetSizeUi() > max_size)
			{
				max_size = cell->GetSizeUi();
				max_id = cell->Id();
			}
		}

		for (CellBinIter iter = vertex->GetCellsBegin();
		iter != vertex->GetCellsEnd(); ++iter)
		{
			pCell cell = iter->lock();
			if (!cell)
				continue;
			unsigned int id = cell->Id();
			if (id == max_id)
				continue;
			cell_iter = cell_list.find(id);
			if (cell_iter == cell_list.end())
				continue;
			cell = cell_iter->second;
			pVertex v = cell->GetVertex().lock();
			if (v)
				v->RemoveCell(cell);
			//new vertex
			VertexListIter viter = vert_list.find(id);
			if (viter == vert_list.end())
			{
				pVertex v0 = pVertex(new Vertex(id));
				v0->AddCell(cell, true);
				cell->AddVertex(v0);
				vert_list.insert(std::pair<unsigned int, pVertex>(
					id, v0));
			}
			else
			{
				pVertex v0 = viter->second;
				v0->AddCell(cell, true);
				cell->AddVertex(v0);
			}
		}
	}

	return true;
}

bool TrackMapProcessor::ClusterCellsMerge(CellList &list, size_t frame)
{
	size_t frame_num = m_map.m_frame_num;
	if (frame >= frame_num)
		return false;

	//get data and label
	VolCache cache = m_vol_cache.get(frame);
	void* data = cache.data;
	void* label = cache.label;
	if (!data || !label)
		return false;

	//needs a way to choose processor
	ClusterDbscan cs_processor;
	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	size_t minx, miny, minz;
	size_t maxx, maxy, maxz;
	unsigned int label_value;
	unsigned int id = 0;
	float data_value;

	//add cluster points
	for (CellListIter cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		pCell cell = cliter->second;
		unsigned int cid = cell->Id();
		if (!id) id = cid;

		minx = size_t(cell->GetBox().min().x() + 0.5);
		miny = size_t(cell->GetBox().min().y() + 0.5);
		minz = size_t(cell->GetBox().min().z() + 0.5);
		maxx = size_t(cell->GetBox().max().x() + 0.5);
		maxy = size_t(cell->GetBox().max().y() + 0.5);
		maxz = size_t(cell->GetBox().max().z() + 0.5);
		for (i = minx; i <= maxx; ++i)
		for (j = miny; j <= maxy; ++j)
		for (k = minz; k <= maxz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value = ((unsigned int*)label)[index];
			if (label_value == cid)
			{
				if (m_map.m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (m_map.m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * m_map.m_scale / 65535.0f;
				cs_processor.AddClusterPoint(
					FLIVR::Point(i, j, k), data_value);
			}
		}
	}

	//use dbscan to check cluster
	//set values to be conservative
	unsigned int size = (unsigned int)m_size_thresh;
	cs_processor.SetSize(size);
	cs_processor.Execute();
	if (cs_processor.GetCluterNum() == 1)
		return true;
	else
		return false;
}

bool TrackMapProcessor::ClusterCellsSplit(CellList &list, size_t frame,
	size_t clnum, CellList &listout)
{
	size_t frame_num = m_map.m_frame_num;
	if (frame >= frame_num)
		return false;

	//get data and label
	VolCache cache = m_vol_cache.get(frame);
	void* data = cache.data;
	void* label = cache.label;
	if (!data || !label)
		return false;

	//needs a way to choose processor
	ClusterExmax cs_processor;
	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	size_t minx, miny, minz;
	size_t maxx, maxy, maxz;
	unsigned int label_value;
	unsigned int id = 0;
	float data_value;

	//add cluster points
	for (CellListIter cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		pCell cell = cliter->second;
		unsigned int cid = cell->Id();
		if (!id) id = cid;

		minx = size_t(cell->GetBox().min().x() + 0.5);
		miny = size_t(cell->GetBox().min().y() + 0.5);
		minz = size_t(cell->GetBox().min().z() + 0.5);
		maxx = size_t(cell->GetBox().max().x() + 0.5);
		maxy = size_t(cell->GetBox().max().y() + 0.5);
		maxz = size_t(cell->GetBox().max().z() + 0.5);
		for (i = minx; i <= maxx; ++i)
		for (j = miny; j <= maxy; ++j)
		for (k = minz; k <= maxz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value = ((unsigned int*)label)[index];
			if (label_value == cid)
			{
				if (m_map.m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (m_map.m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * m_map.m_scale / 65535.0f;
				cs_processor.AddClusterPoint(
					FLIVR::Point(i, j, k), data_value);
			}
		}
	}

	bool result = false;
	for (size_t clnumi = clnum; clnumi > 1; --clnumi)
	{
		cs_processor.SetClnum(clnum);
		if (cs_processor.Execute())
		{
			result = true;
			break;
		}
	}

	if (result)
	{
		cs_processor.GenerateNewIDs(id, label,
			nx, ny, nz);
		//generate output cell list
		Cluster &points = cs_processor.GetData();
		std::vector<unsigned int> &ids = cs_processor.GetNewIDs();
		pClusterPoint point;
		unsigned int id2;
		CellListIter citer;
		for (ClusterIter piter = points.begin();
			piter != points.end(); ++piter)
		{
			point = *piter;
			i = size_t(point->center.x() + 0.5);
			j = size_t(point->center.y() + 0.5);
			k = size_t(point->center.z() + 0.5);
			index = nx*ny*k + nx*j + i;
			id2 = ((unsigned int*)label)[index];
			citer = listout.find(id2);
			if (citer != listout.end())
			{
				citer->second->Inc(i, j, k, point->intensity);
			}
			else
			{
				Cell* cell = new Cell(id2);
				cell->Inc(i, j, k, point->intensity);
				listout.insert(std::pair<unsigned int, pCell>
					(id2, pCell(cell)));
			}
		}

		//label modified, save before delete
		m_vol_cache.set_modified(frame);

		return true;
	}
	else
		return false;
}

bool TrackMapProcessor::SegmentCells(
	void* data, void* label,
	CellList &list, size_t frame)
{
	//ClusterDbscan cs_processor;
	//unsigned int size = (unsigned int)m_size_thresh;
	//cs_processor.SetSize(size);
	//ClusterKmeans cs_processor;
	ClusterExmax cs_processor;

	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	unsigned int label_value;
	unsigned int id = 0;
	float data_value;

	//add cluster points
	for (CellListIter cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		pCell cell = cliter->second;
		unsigned int cid = cell->Id();
		if (!id) id = cid;

		for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
		for (k = 0; k < nz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value = ((unsigned int*)label)[index];
			if (label_value == cid)
			{
				if (m_map.m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (m_map.m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * m_map.m_scale / 65535.0f;
				cs_processor.AddClusterPoint(
					FLIVR::Point(i, j, k), data_value);
			}
		}
	}

	//run clustering
	cs_processor.Execute();
	cs_processor.GenerateNewIDs(id, label, nx, ny, nz);
	//cs_processor.GenerateNewColors2(label, nx, ny, nz);

	return true;
}

bool TrackMapProcessor::ReplaceCellID(
	unsigned int old_id, unsigned int new_id, size_t frame)
{
	if (frame >= m_map.m_frame_num)
		return false;

	CellList &cell_list = m_map.m_cells_list.at(frame);
	CellListIter iter = cell_list.find(old_id);
	if (iter == cell_list.end())
		return false;

	pCell old_cell = iter->second;
	pCell new_cell = pCell(new Cell(new_id));
	new_cell->SetCenter(old_cell->GetCenter());
	new_cell->SetSizeUi(old_cell->GetSizeUi());
	new_cell->SetSizeF(old_cell->GetSizeF());
	new_cell->SetExternalUi(old_cell->GetExternalUi());
	new_cell->SetExternalF(old_cell->GetExternalF());
	new_cell->SetIntraVert(old_cell->GetIntraVert());
	cell_list.erase(iter);
	cell_list.insert(std::pair<unsigned int, pCell>
		(new_id, new_cell));

	//vertex
	pVertex vertex = old_cell->GetVertex().lock();
	if (vertex)
	{
		vertex->RemoveCell(old_cell);
		vertex->AddCell(new_cell);
		new_cell->AddVertex(vertex);
	}

	//intra graph
	IntraGraph &graph = m_map.m_intra_graph_list.at(frame);
	IntraVert intra_vert = new_cell->GetIntraVert();
	if (intra_vert != IntraGraph::null_vertex())
	{
		graph[intra_vert].cell = new_cell;
		graph[intra_vert].id = new_id;
	}

	return true;
}

void TrackMapProcessor::GetLinkLists(
	size_t frame,
	FL::VertexList &in_orphan_list,
	FL::VertexList &out_orphan_list,
	FL::VertexList &in_multi_list,
	FL::VertexList &out_multi_list)
{
	if (frame >= m_map.m_frame_num)
		return;

	VertexList &vertex_list = m_map.m_vertices_list.at(frame);

	InterVert v0, v1;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	std::pair<InterEdge, bool> edge;
	int edge_count;

	//in lists
	if (frame > 0)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame - 1);
		for (VertexListIter iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
		{
			if (!iter->second)
				continue;
			if (iter->second->GetSizeUi() < m_size_thresh)
				continue;
			v0 = iter->second->GetInterVert(inter_graph);
			if (v0 == InterGraph::null_vertex())
			{
				in_orphan_list.insert(std::pair<unsigned int, pVertex>(
					iter->second->Id(), iter->second));
				continue;
			}
			adj_verts = boost::adjacent_vertices(v0, inter_graph);
			edge_count = 0;
			//for each adjacent vertex
			for (InterAdjIter inter_iter = adj_verts.first;
			inter_iter != adj_verts.second; ++inter_iter)
			{
				v1 = *inter_iter;
				edge = boost::edge(v0, v1, inter_graph);
				if (edge.second && inter_graph[edge.first].link)
					edge_count++;
			}
			if (edge_count == 0)
				in_orphan_list.insert(std::pair<unsigned int, pVertex>(
					iter->second->Id(), iter->second));
			else if (edge_count > 1)
				in_multi_list.insert(std::pair<unsigned int, pVertex>(
					iter->second->Id(), iter->second));
		}
	}

	//out lists
	if (frame < m_map.m_frame_num - 1)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame);
		for (VertexListIter iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
		{
			if (!iter->second)
				continue;
			if (iter->second->GetSizeUi() < m_size_thresh)
				continue;
			v0 = iter->second->GetInterVert(inter_graph);
			if (v0 == InterGraph::null_vertex())
			{
				out_orphan_list.insert(std::pair<unsigned int, pVertex>(
					iter->second->Id(), iter->second));
				continue;
			}
			adj_verts = boost::adjacent_vertices(v0, inter_graph);
			edge_count = 0;
			//for each adjacent vertex
			for (InterAdjIter inter_iter = adj_verts.first;
			inter_iter != adj_verts.second; ++inter_iter)
			{
				v1 = *inter_iter;
				edge = boost::edge(v0, v1, inter_graph);
				if (edge.second && inter_graph[edge.first].link)
					edge_count++;
			}
			if (edge_count == 0)
				out_orphan_list.insert(std::pair<unsigned int, pVertex>(
					iter->second->Id(), iter->second));
			else if (edge_count > 1)
				out_multi_list.insert(std::pair<unsigned int, pVertex>(
					iter->second->Id(), iter->second));
		}
	}
}

void TrackMapProcessor::GetCellsByUncertainty(
	CellList &list_in, CellList &list_out,
	size_t frame)
{
	if (frame >= m_map.m_frame_num)
		return;

	VertexList &vertex_list = m_map.m_vertices_list.at(frame);
	InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame);
	bool filter = !(list_in.empty());

	InterVert v0;
	unsigned int count;
	pVertex vertex;
	pCell cell;
	CellBinIter pwcell_iter;
	CellListIter cell_iter;
	for (VertexListIter iter = vertex_list.begin();
	iter != vertex_list.end(); ++iter)
	{
		if (!iter->second)
			continue;
		if (iter->second->GetSizeUi() < m_size_thresh)
			continue;
		v0 = iter->second->GetInterVert(inter_graph);
		if (v0 == InterGraph::null_vertex())
			continue;
		count = inter_graph[v0].count;
		if (count >= m_uncertain_low &&
			count <= m_uncertain_high)
		{
			vertex = inter_graph[v0].vertex.lock();
			if (!vertex)
				continue;
			for (pwcell_iter = vertex->GetCellsBegin();
				pwcell_iter != vertex->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				if (filter)
				{
					cell_iter = list_in.find(cell->Id());
					if (cell_iter == list_in.end())
						continue;
				}
				list_out.insert(std::pair<unsigned int, pCell>
					(cell->Id(), cell));
			}
		}
	}
}

void TrackMapProcessor::GetCellUncertainty(
	CellList &list, size_t frame)
{
	if (frame >= m_map.m_frame_num)
		return;

	VertexList &vertex_list = m_map.m_vertices_list.at(frame);

	InterVert v0;
	pVertex vertex;
	pCell cell;
	CellBinIter pwcell_iter;
	CellListIter cell_iter;

	//in lists
	if (frame > 0)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame - 1);
		for (VertexListIter iter = vertex_list.begin();
			iter != vertex_list.end(); ++iter)
		{
			if (!iter->second)
				continue;
			vertex = iter->second;
			for (pwcell_iter = vertex->GetCellsBegin();
				pwcell_iter != vertex->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				cell_iter = list.find(cell->Id());
				if (cell_iter != list.end())
				{
					v0 = vertex->GetInterVert(inter_graph);
					if (v0 == InterGraph::null_vertex())
						continue;
					cell_iter->second->SetSizeUi(
						inter_graph[v0].count);
				}
			}
		}
	}
	else
	{
		//clear size
		for (cell_iter = list.begin();
			cell_iter != list.end(); ++cell_iter)
		{
			cell_iter->second->SetSizeUi(0);
		}
	}

	//out lists
	if (frame < m_map.m_frame_num - 1)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame);
		for (VertexListIter iter = vertex_list.begin();
			iter != vertex_list.end(); ++iter)
		{
			if (!iter->second)
				continue;
			vertex = iter->second;
			for (pwcell_iter = vertex->GetCellsBegin();
			pwcell_iter != vertex->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				cell_iter = list.find(cell->Id());
				if (cell_iter != list.end())
				{
					v0 = vertex->GetInterVert(inter_graph);
					if (v0 == InterGraph::null_vertex())
						continue;
					cell_iter->second->SetExternalUi(
						inter_graph[v0].count);
				}
			}
		}
	}
	else
	{
		//clear size
		for (cell_iter = list.begin();
		cell_iter != list.end(); ++cell_iter)
		{
			cell_iter->second->SetExternalUi(0);
		}
	}
}

void TrackMapProcessor::GetUncertainHist(
	UncertainHist &hist1, UncertainHist &hist2, size_t frame)
{
	if (frame >= m_map.m_frame_num)
		return;

	VertexList &vertex_list = m_map.m_vertices_list.at(frame);

	unsigned int count;
	InterVert v0;
	pVertex vertex;

	//in lists
	if (frame > 0)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame - 1);
		for (VertexListIter iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
		{
			if (!iter->second)
				continue;
			vertex = iter->second;
			v0 = vertex->GetInterVert(inter_graph);
			if (v0 == InterGraph::null_vertex())
				continue;
			count = inter_graph[v0].count;
			auto uhist_iter = hist1.find(count);
			if (uhist_iter == hist1.end())
			{
				UncertainBin bin;
				bin.level = count;
				bin.count = 1;
				hist1.insert(std::pair<unsigned int, UncertainBin>(
					count, bin));
			}
			else
				uhist_iter->second.count++;
		}
	}

	//out lists
	if (frame < m_map.m_frame_num - 1)
	{
		InterGraph &inter_graph = m_map.m_inter_graph_list.at(frame);
		for (VertexListIter iter = vertex_list.begin();
			iter != vertex_list.end(); ++iter)
		{
			if (!iter->second)
				continue;
			vertex = iter->second;
			v0 = vertex->GetInterVert(inter_graph);
			if (v0 == InterGraph::null_vertex())
				continue;
			count = inter_graph[v0].count;
			auto uhist_iter = hist2.find(count);
			if (uhist_iter == hist2.end())
			{
				UncertainBin bin;
				bin.level = count;
				bin.count = 1;
				hist2.insert(std::pair<unsigned int, UncertainBin>(
					count, bin));
			}
			else
				uhist_iter->second.count++;
		}
	}
}

void TrackMapProcessor::GetPaths(CellList &cell_list, PathList &path_list, size_t frame1, size_t frame2)
{
	size_t frame_num = m_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return;

	CellList &cell_list1 = m_map.m_cells_list.at(frame1);
	InterGraph &inter_graph = m_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	VertexList vertex_list;
	CellListIter cell_iter;
	pVertex vertex1;

	for (auto sel_iter = cell_list.begin();
		sel_iter != cell_list.end();
		++sel_iter)
	{
		cell_iter = cell_list1.find(sel_iter->second->Id());
		if (cell_iter == cell_list1.end())
			continue;
		vertex1 = cell_iter->second->GetVertex().lock();
		if (!vertex1)
			continue;
		if (vertex_list.find(vertex1->Id()) ==
			vertex_list.end())
			vertex_list.insert(std::pair<unsigned int, pVertex>
			(vertex1->Id(), vertex1));
	}

	m_level_thresh = 2;
	for (auto iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
	{
		GetAlterPath(inter_graph, iter->second, path_list);
	}
}

bool TrackMapProcessor::TrackStencils(size_t f1, size_t f2)
{
	//check validity
	if (!m_map.ExtendFrameNum(std::max(f1, f2)))
		return false;

	size_t frame_num = m_map.m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num || f1 == f2)
		return false;

	//get data and label
	m_vol_cache.set_max_size(2);
	VolCache cache = m_vol_cache.get(f1);
	void* data1 = cache.data;
	void* label1 = cache.label;
	if (!data1 || !label1)
		return false;
	cache = m_vol_cache.get(f2);
	void* data2 = cache.data;
	void* label2 = cache.label;
	if (!data2 || !label2)
		return false;

	size_t index;
	size_t i, j, k;
	size_t nx = m_map.m_size_x;
	size_t ny = m_map.m_size_y;
	size_t nz = m_map.m_size_z;
	unsigned int label_value;

	//get all stencils from frame1
	StencilList stencil_list;
	StencilListIter iter;
	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		label_value = ((unsigned int*)label1)[index];

		if (!label_value)
			continue;

		iter = stencil_list.find(label_value);
		if (iter != stencil_list.end())
		{
			iter->second.extend(i, j, k);
		}
		else
		{
			Stencil stencil;
			stencil.data = data1;
			stencil.id = label_value;
			stencil.nx = nx;
			stencil.ny = ny;
			stencil.nz = nz;
			stencil.bits = m_map.m_data_bits;
			stencil.scale = m_map.m_scale;
			stencil.box.extend(FLIVR::Point(i, j, k));
			stencil_list.insert(std::pair<unsigned int, Stencil>
				(label_value, stencil));
		}
	}

	//find matching stencil in frame2
	FLIVR::Point center;
	FLIVR::Vector ext(1.5, 1.5, 0.5);
	float prob;
	Stencil s1, s2;
	s2.data = data2;
	s2.nx = nx;
	s2.ny = ny;
	s2.nz = nz;
	s2.bits = m_map.m_data_bits;
	s2.scale = m_map.m_scale;
	for (iter = stencil_list.begin(); iter != stencil_list.end(); ++iter)
	{
		s1 = iter->second;
		if (match_stencils(s1, s2, ext, center, prob))
		{
			//if (prob > 0.5f)
			//	continue;

			//label stencil 2
			label_stencil(s1, s2, label1, label2);

			//add s1 to track map
			CellListIter iter;
			pCell cell1(new Cell(s1.id));
			cell1->SetCenter(s1.box.center());
			cell1->SetBox(s1.box);
			AddCell(cell1, f1, iter);
			//add s2 id to track map
			pCell cell2(new Cell(s2.id));
			cell2->SetCenter(s2.box.center());
			cell2->SetBox(s2.box);
			AddCell(cell2, f2, iter);
			//connect cells
			LinkCells(cell1, cell2, f1, f2, false);
		}
	}

	return true;
}