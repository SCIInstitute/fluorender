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
#include <functional>
#include <algorithm>
#include <limits>

using namespace FL;

TrackMap::TrackMap() :
	m_last_op(0),
	m_frame_num(0),
	m_size_x(0),
	m_size_y(0),
	m_size_z(0),
	m_data_bits(8),
	m_scale(1.0f)
{
}

TrackMap::~TrackMap()
{
}

void TrackMapProcessor::SetSizes(TrackMap& track_map,
	size_t nx, size_t ny, size_t nz)
{
	track_map.m_size_x = nx;
	track_map.m_size_y = ny;
	track_map.m_size_z = nz;
}

void TrackMapProcessor::SetBits(TrackMap& track_map,
	size_t bits)
{
	track_map.m_data_bits = bits;
}

bool TrackMapProcessor::InitializeFrame(TrackMap& track_map,
	void* data, void* label, size_t frame)
{
	if (!data || !label)
		return false;

	//add one empty cell list to track_map
	track_map.m_cells_list.push_back(CellList());
	CellList &cell_list = track_map.m_cells_list.back();
	CellListIter iter;
	//in the meanwhile build the intra graph
	track_map.m_intra_graph_list.push_back(IntraGraph());

	size_t index;
	size_t i, j, k;
	size_t nx = track_map.m_size_x;
	size_t ny = track_map.m_size_y;
	size_t nz = track_map.m_size_z;
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

				if (track_map.m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (track_map.m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * track_map.m_scale / 65535.0f;

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
					CheckCellContact(track_map,
						iter->second, data, label,
						i, j, k);
				}
			}

	//build vertex list
	track_map.m_vertices_list.push_back(VertexList());
	VertexList &vertex_list = track_map.m_vertices_list.back();
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

	track_map.m_frame_num++;
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

bool TrackMapProcessor::CheckCellContact(TrackMap& track_map,
	pCell &cell, void *data, void *label,
	size_t ci, size_t cj, size_t ck)
{
	int ec = 0;//external count
	int cc = 0;//contact count
	size_t nx = track_map.m_size_x;
	size_t ny = track_map.m_size_y;
	size_t nz = track_map.m_size_z;
	size_t indexn;//neighbor index
	unsigned int idn;//neighbor id
	float valuen;//neighbor vlaue
	size_t index = nx*ny*ck + nx*cj + ci;
	unsigned int id = cell->Id();
	float value;
	size_t data_bits = track_map.m_data_bits;
	float scale = track_map.m_scale;
	if (data_bits == 8)
		value = ((unsigned char*)data)[index] / 255.0f;
	else if (data_bits == 16)
		value = ((unsigned short*)data)[index] * scale / 65535.0f;
	float contact_value;
	IntraGraph &intra_graph = track_map.m_intra_graph_list.back();
	CellList &cell_list = track_map.m_cells_list.back();
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
		cell->IncExternal(value);

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
	}
	else
	{
		graph[e.first].size_ui++;
		graph[e.first].size_f += contact_value;
	}

	return true;
}

bool TrackMapProcessor::LinkMaps(TrackMap& track_map,
	size_t f1, size_t f2, void *data1, void *data2,
	void *label1, void *label2)
{
	size_t frame_num = track_map.m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num ||
		f1 == f2 || !data1 || !data2 ||
		!label1 || !label2)
		return false;

	track_map.m_inter_graph_list.push_back(InterGraph());
	InterGraph &inter_graph = track_map.m_inter_graph_list.back();
	inter_graph.index = f1;

	size_t index;
	size_t i, j, k;
	size_t nx = track_map.m_size_x;
	size_t ny = track_map.m_size_y;
	size_t nz = track_map.m_size_z;
	float data_value1, data_value2;
	unsigned int label_value1, label_value2;
	VertexList &vertex_list1 = track_map.m_vertices_list.at(f1);
	VertexList &vertex_list2 = track_map.m_vertices_list.at(f2);
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

		if (track_map.m_data_bits == 8)
		{
			data_value1 = ((unsigned char*)data1)[index] / 255.0f;
			data_value2 = ((unsigned char*)data2)[index] / 255.0f;
		}
		else if (track_map.m_data_bits == 16)
		{
			data_value1 = ((unsigned short*)data1)[index] * track_map.m_scale / 65535.0f;
			data_value2 = ((unsigned short*)data2)[index] * track_map.m_scale / 65535.0f;
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
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = f2;
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
		graph[e.first].dist = float((p1 - p2).length());
		graph[e.first].link = 0;
	}
	else
	{
		graph[e.first].size_ui++;
		graph[e.first].size_f += overlap_value;
	}

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
			graph[edge.first].link = 0;
	}

	return true;
}

bool TrackMapProcessor::ForceVertices(InterGraph& graph,
	pVertex &vertex1, pVertex &vertex2)
{
	InterVert v1, v2;

	v1 = vertex1->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
		return false;
	v2 = vertex2->GetInterVert(graph);
	if (v2 == InterGraph::null_vertex())
		return false;

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
		graph[edge.first].dist = float((p1 - p2).length());
		graph[edge.first].link = 1;
	}
	else
	{
		graph[edge.first].size_ui = std::max(
			vertex1->GetSizeUi(), vertex2->GetSizeUi());
		graph[edge.first].size_f = std::max(
			vertex1->GetSizeF(), vertex2->GetSizeF());
		graph[edge.first].link = 1;
	}

	return true;
}

bool TrackMapProcessor::ResolveGraph(TrackMap& track_map, size_t frame1, size_t frame2)
{
	if (frame1 >= track_map.m_frame_num ||
		frame2 >= track_map.m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = track_map.m_vertices_list.at(frame1);
	VertexList &vertex_list2 = track_map.m_vertices_list.at(frame2);
	CellList &cell_list2 = track_map.m_cells_list.at(frame2);
	IntraGraph &intra_graph = track_map.m_intra_graph_list.at(frame2);
	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	VertexListIter iter;
	InterVert v1, v2;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	std::vector<pwCell> cells;
	CellBinIter pwcell_iter;
	pVertex vertex2;
	std::vector<CellBin> cell_bins;
	pCell cell2, cell2c;
	IntraVert c2, c2c;
	std::pair<IntraAdjIter, IntraAdjIter> adj_cells;
	IntraAdjIter intra_iter;
	bool added;
	std::pair<IntraEdge, bool> intra_edge;
	float osizef, c1sizef, c2sizef;
	size_t i;

	//check all vertices in the time frame
	for (iter = vertex_list1.begin();
	iter != vertex_list1.end(); ++iter)
	{
		cells.clear();
		cell_bins.clear();
		v1 = iter->second->GetInterVert(inter_graph);
		if (v1 == InterGraph::null_vertex())
			continue;
		adj_verts = boost::adjacent_vertices(v1, inter_graph);
		//for each adjacent vertex
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
		for (pwcell_iter = cells.begin();
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
					osizef = intra_graph[intra_edge.first].size_f;
					c1sizef = cell2->GetSizeF();
					cell2c = intra_graph[c2c].cell.lock();
					if (!cell2c)
						continue;
					c2sizef = cell2c->GetSizeF();
					if (osizef / c1sizef > m_contact_thresh ||
						osizef / c2sizef > m_contact_thresh)
					{
						//add both to bin list
						added = AddCellBin(cell_bins, *pwcell_iter, intra_graph[c2c].cell);
					}
				}
			}
			if (!added)//add to bin as well
				AddCellBin(cell_bins, *pwcell_iter);
		}

		//modify vertex list 2 if necessary
		for (i = 0; i < cell_bins.size(); ++i)
			MergeCells(vertex_list2, cell_bins[i], track_map, frame2);
	}

	return true;
}

bool TrackMapProcessor::MatchFrames(TrackMap& track_map,
	size_t frame1, size_t frame2, bool bl_check)
{
	if (frame1 >= track_map.m_frame_num ||
		frame2 >= track_map.m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = track_map.m_vertices_list.at(frame1);
	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	VertexListIter iter;

	for (iter = vertex_list1.begin();
	iter != vertex_list1.end(); ++iter)
		MatchVertex(iter->second, inter_graph, bl_check);

	track_map.m_last_op = 1;

	return true;
}

bool TrackMapProcessor::UnmatchFrames(TrackMap& track_map, size_t frame1, size_t frame2)
{
	if (frame1 >= track_map.m_frame_num ||
		frame2 >= track_map.m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = track_map.m_vertices_list.at(frame1);
	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	VertexListIter iter;
	InterVert v0;

	for (iter = vertex_list1.begin();
	iter != vertex_list1.end(); ++iter)
	{
		v0 = iter->second->GetInterVert(inter_graph);
		if (v0 == InterGraph::null_vertex())
		{
		}
		else
		{
			UnmatchVertex(iter->second, inter_graph);
		}
	}

	track_map.m_last_op = 2;

	return true;
}

bool TrackMapProcessor::MatchVertex(pVertex &vertex,
	InterGraph &graph, bool bl_check)
{
	if (!vertex)
		return false;
	InterVert v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	InterVert v1;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	std::vector<InterEdge> edges;
	std::pair<InterEdge, bool> edge;
	unsigned int bl_size_ui;
	float bl_size_f;
	bool linked = false;
	float edge_size, v0_size, v1_size;
	pVertex edge_vert;

	//set flag for link
	adj_verts = boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (InterAdjIter inter_iter = adj_verts.first;
	inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			if (graph[edge.first].link)
			{
				linked = true;
				break;
			}

			if (bl_check)
			{
				graph[edge.first].bl_num = CheckBackLink(
					v0, v1, graph, bl_size_ui, bl_size_f);
				if (graph[edge.first].bl_num)
				{
					graph[edge.first].bl_size_ui = bl_size_ui;
					graph[edge.first].bl_size_f = bl_size_f;
				}
			}

			edges.push_back(edge.first);
		}
	}

	if (!linked && edges.size())
	{
		//sort edges
		if (bl_check)
		{
			std::sort(edges.begin(), edges.end(),
				std::bind(edge_comp_size_bl, std::placeholders::_1,
				std::placeholders::_2, graph));
			graph[edges[0]].link = 1;
		}
		else
		{
			//std::sort(edges.begin(), edges.end(),
			//	std::bind(edge_comp_size_ol, std::placeholders::_1,
			//	std::placeholders::_2, graph));
			for (size_t i = 0; i < edges.size(); ++i)
			{
				edge_size = graph[edges[i]].size_f;
				edge_vert = graph[boost::source(edges[i], graph)].vertex.lock();
				if (!edge_vert) continue;
				v0_size = edge_vert->GetSizeF();
				edge_vert = graph[boost::target(edges[i], graph)].vertex.lock();
				if (!edge_vert) continue;
				v1_size = edge_vert->GetSizeF();
				if (/*edge_size * 10 > std::min(v0_size, v1_size) &&*/
					fabs(v0_size - v1_size) / (v0_size + v1_size) < 0.2f)
					graph[edges[i]].link = 1;
			}
		}
	}

	return true;
}

bool TrackMapProcessor::UnmatchVertex(pVertex &vertex, InterGraph &graph)
{
	if (!vertex)
		return false;
	InterVert v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	InterVert v1;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	std::vector<InterEdge> edges;
	std::pair<InterEdge, bool> edge;
	unsigned int bl_size_ui;
	float bl_size_f;

	adj_verts = boost::adjacent_vertices(v0, graph);
	for (InterAdjIter inter_iter = adj_verts.first;
	inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		edge = boost::edge(v0, v1, graph);
		if (edge.second &&
			graph[edge.first].link)
		{
			graph[edge.first].bl_num = CheckBackLink(
				v0, v1, graph, bl_size_ui, bl_size_f);
			if (graph[edge.first].bl_num)
			{
				graph[edge.first].bl_size_ui = bl_size_ui;
				graph[edge.first].bl_size_f = bl_size_f;
			}
			edges.push_back(edge.first);
		}
	}

	if (edges.size() > 1)
		//sort edges
		std::sort(edges.begin(), edges.end(),
			std::bind(edge_comp_size_bl, std::placeholders::_1,
			std::placeholders::_2, graph));

	for (size_t i = 1; i < edges.size(); ++i)
		graph[edges[i]].link = 0;

	return true;
}

bool TrackMapProcessor::edge_comp_size_ol(InterEdge edge1,
	InterEdge edge2, InterGraph& graph)
{
	return graph[edge1].size_f > graph[edge2].size_f;
}

bool TrackMapProcessor::edge_comp_size_bl(InterEdge edge1,
	InterEdge edge2, InterGraph& graph)
{
	if (graph[edge1].bl_num != graph[edge2].bl_num)
		return graph[edge1].bl_num < graph[edge2].bl_num;
	else
		return graph[edge1].size_f > graph[edge2].size_f;
}

unsigned int TrackMapProcessor::CheckBackLink(InterVert v0,
	InterVert v1, InterGraph &graph,
	unsigned int &bl_size_ui, float &bl_size_f)
{
	unsigned int result = 0;
	bl_size_ui = 0;
	bl_size_f = 0.0f;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterVert bl_vert;
	std::pair<InterEdge, bool> edge;

	adj_verts = boost::adjacent_vertices(v1, graph);
	for (InterAdjIter inter_iter = adj_verts.first;
	inter_iter != adj_verts.second; ++inter_iter)
	{
		bl_vert = *inter_iter;
		if (bl_vert == v0)
			continue;
		edge = boost::edge(bl_vert, v1, graph);
		if (edge.second &&
			graph[edge.first].link)
		{
			bl_size_ui = graph[edge.first].size_ui > bl_size_ui ?
				graph[edge.first].size_ui : bl_size_ui;
			bl_size_f = graph[edge.first].size_f > bl_size_f ?
				graph[edge.first].size_f : bl_size_f;
			result++;
		}
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

bool TrackMapProcessor::MergeCells(VertexList& vertex_list, CellBin &bin,
	TrackMap& track_map, size_t frame)
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
			if (vertex &&
				vertex->Id() == vertex0->Id())
				continue;
			if (vertex)
			{
				//relink inter graph
				if (frame > 0)
					RelinkInterGraph(vertex, vertex0, frame,
						track_map.m_inter_graph_list.at(frame-1));
				if (frame < track_map.m_frame_num-1)
					RelinkInterGraph(vertex, vertex0, frame,
						track_map.m_inter_graph_list.at(frame));

				//collect cells from vertex
				for (cell_iter = vertex->GetCellsBegin();
					cell_iter != vertex->GetCellsEnd();
					++cell_iter)
					cell_list.push_back(*cell_iter);

				//remove vertex from list
				vert_iter = vertex_list.find(vertex->Id());
				if (vert_iter != vertex_list.end())
					vertex_list.erase(vert_iter);
			}

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
			//vertex0->AddCell(cell, true);
			//cell->AddVertex(vertex0);
		}
	}

	return true;
}

bool TrackMapProcessor::RelinkInterGraph(pVertex &vertex, pVertex &vertex0, size_t frame, InterGraph &graph)
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
				graph[inter_vert0].vertex = vertex0;
				vertex0->SetInterVert(graph, inter_vert0);
			}
			e0 = boost::edge(*inter_iter,
				inter_vert0, graph);
			if (!e0.second)
			{
				e0 = boost::add_edge(*inter_iter,
					inter_vert0, graph);
				graph[e0.first].size_ui = graph[e.first].size_ui;
				graph[e0.first].size_f = graph[e.first].size_f;
				graph[e0.first].dist = graph[e.first].dist;
				graph[e0.first].link = graph[e.first].link;
			}
			else
			{
				graph[e0.first].size_ui += graph[e.first].size_ui;
				graph[e0.first].size_f += graph[e.first].size_f;
			}
			//delete the old edge
			edges_to_remove.push_back(e.first);
			//graph.remove_edge(e.first);
		}
		//remove edges
		for (edge_to_remove = edges_to_remove.begin();
		edge_to_remove != edges_to_remove.end();
			++edge_to_remove)
		{
			graph.remove_edge(*edge_to_remove);
		}
		//remove the vertex from inter graph
		//edges should be removed as well
		//boost::remove_vertex(inter_vert, graph);
	}

	return true;
}

bool TrackMapProcessor::Export(TrackMap & track_map, std::string &filename)
{
	if (track_map.m_frame_num == 0 ||
		track_map.m_frame_num != track_map.m_cells_list.size() ||
		track_map.m_frame_num != track_map.m_vertices_list.size() ||
		track_map.m_frame_num != track_map.m_inter_graph_list.size() + 1)
		return false;

	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	if (ofs.bad())
		return false;

	//header
	std::string header = "FluoRender links";
	ofs.write(header.c_str(), header.size());

	//last operation
	WriteTag(ofs, TAG_LAST_OP);
	WriteUint(ofs, track_map.m_last_op);

	//number of frames
	WriteTag(ofs, TAG_NUM);
	size_t num = track_map.m_frame_num;
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
		VertexList &vertex_list = track_map.m_vertices_list.at(i);
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
		IntraGraph &intra_graph = track_map.m_intra_graph_list.at(i);
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
		}
		//write inter edges
		if (i == 0)
			continue;
		InterGraph &inter_graph = track_map.m_inter_graph_list.at(i - 1);
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
			WriteFloat(ofs, inter_graph[*inter_iter].dist);
			WriteUint(ofs, inter_graph[*inter_iter].link);
		}
	}

	return true;
}

bool TrackMapProcessor::Import(TrackMap& track_map, std::string &filename)
{
	//clear everything
	track_map.Clear();

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
	if (ReadTag(ifs) == TAG_LAST_OP)
		track_map.m_last_op = ReadUint(ifs);
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
		track_map.m_vertices_list.push_back(VertexList());
		VertexList &vertex_list1 = track_map.m_vertices_list.back();
		//cell list
		track_map.m_cells_list.push_back(CellList());
		CellList &cell_list = track_map.m_cells_list.back();
		//vertex number
		vertex_num = ReadUint(ifs);
		//read each vertex
		for (size_t j = 0; j < vertex_num; ++j)
			ReadVertex(ifs, vertex_list1, cell_list);
		//intra graph
		track_map.m_intra_graph_list.push_back(IntraGraph());
		IntraGraph &intra_graph = track_map.m_intra_graph_list.back();
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
			if (edge_exist)
				AddIntraEdge(intra_graph, cell1, cell2,
					size_ui, size_f);
		}
		//inter graph
		if (i == 0)
			continue;
		//old vertex list
		VertexList &vertex_list0 = track_map.m_vertices_list.at(i - 1);
		track_map.m_inter_graph_list.push_back(InterGraph());
		InterGraph &inter_graph = track_map.m_inter_graph_list.back();
		inter_graph.index = i - 1;
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
			if (edge_exist)
				AddInterEdge(inter_graph, vertex1, vertex2,
					i-1, i, size_ui, size_f, dist, link);
		}
	}

	track_map.m_frame_num = num;
	return true;
}

bool TrackMapProcessor::ResetVertexIDs(TrackMap& track_map)
{
	for (size_t fi = 0; fi < track_map.m_frame_num; ++fi)
	{
		VertexList &vertex_list = track_map.m_vertices_list.at(fi);
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
					InterGraph &inter_graph = track_map.m_inter_graph_list.at(fi - 1);
					InterVert inter_vert = vertex->GetInterVert(inter_graph);
					if (inter_vert != InterGraph::null_vertex())
						inter_graph[inter_vert].id = max_id;
				}
				if (fi < track_map.m_frame_num - 1)
				{
					InterGraph &inter_graph = track_map.m_inter_graph_list.at(fi);
					InterVert inter_vert = vertex->GetInterVert(inter_graph);
					if (inter_vert != InterGraph::null_vertex())
						inter_graph[inter_vert].id = max_id;
				}
			}
		}
	}

	return true;
}

void TrackMapProcessor::WriteVertex(std::ofstream& ofs, pVertex &vertex)
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
	pCell &cell1, pCell &cell2, unsigned int size_ui, float size_f)
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
	}
	else
		return false;

	return true;
}

bool TrackMapProcessor::AddInterEdge(InterGraph& graph,
	pVertex &vertex1, pVertex &vertex2,
	size_t f1, size_t f2,
	unsigned int size_ui, float size_f,
	float dist, unsigned int link)
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
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = f2;
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}

	std::pair<InterEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = size_ui;
		graph[e.first].size_f = size_f;
		graph[e.first].dist = dist;
		graph[e.first].link = link;
	}
	else
		return false;

	return true;
}

bool TrackMapProcessor::GetMappedID(TrackMap& track_map,
	unsigned int id_in, unsigned int& id_out,
	size_t frame)
{
	size_t frame_num = track_map.m_frame_num;
	if (frame >= frame_num)
		return false;

	VertexList &vertex_list = track_map.m_vertices_list.at(frame);
	CellList &cell_list = track_map.m_cells_list.at(frame);

	CellListIter cell_iter;
	pVertex vertex;
	cell_iter = cell_list.find(id_in);
	if (cell_iter == cell_list.end())
		return false;
	vertex = cell_iter->second->GetVertex().lock();
	if (!vertex)
		return false;

	id_out = vertex->Id();
	return true;
}

bool TrackMapProcessor::GetMappedID(TrackMap& track_map,
	unsigned int id_in, unsigned int& id_out,
	size_t frame1, size_t frame2)
{
	bool result = false;
	size_t frame_num = track_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	CellList &cell_list1 = track_map.m_cells_list.at(frame1);
	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
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
			id_out = vertex2->Id();
			min_diff = fabs(out_size - in_size);
			result = true;
		}
	}

	return result;
}

bool TrackMapProcessor::GetMappedCells(TrackMap& track_map,
	CellList &sel_list1, CellList &sel_list2,
	size_t frame1, size_t frame2)
{
	size_t frame_num = track_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = track_map.m_vertices_list.at(frame1);
	VertexList &vertex_list2 = track_map.m_vertices_list.at(frame2);
	CellList &cell_list1 = track_map.m_cells_list.at(frame1);
	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
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

unsigned int TrackMapProcessor::GetMappedEdges(TrackMap & track_map,
	CellList & sel_list1, CellList & sel_list2,
	std::vector<float>& verts,
	size_t frame1, size_t frame2)
{
	unsigned int result = 0;

	size_t frame_num = track_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return result;

	VertexList &vertex_list1 = track_map.m_vertices_list.at(frame1);
	VertexList &vertex_list2 = track_map.m_vertices_list.at(frame2);
	CellList &cell_list1 = track_map.m_cells_list.at(frame1);
	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	CellListIter sel_iter, cell_iter;
	pVertex vertex1, vertex2;
	pCell cell;
	InterVert v1, v2;
	std::pair<InterAdjIter, InterAdjIter> adj_verts;
	InterAdjIter inter_iter;
	CellBinIter pwcell_iter;
	FLIVR::Color c;
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
				//save to verts
				c = FLIVR::HSVColor(cell->Id() % 360, 1.0, 0.9);
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

//modifications
bool TrackMapProcessor::LinkCells(TrackMap& track_map,
	CellList &list1, CellList &list2,
	size_t frame1, size_t frame2,
	bool exclusive)
{
	//check validity
	size_t frame_num = track_map.m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num || 
		(frame2 != frame1 + 1 &&
		frame2 != frame1 - 1))
		return false;

	VertexList vlist1, vlist2;
	CellListIter citer1, citer2;

	CellList &cell_list1 = track_map.m_cells_list.at(frame1);
	CellList &cell_list2 = track_map.m_cells_list.at(frame2);
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

	InterGraph &inter_graph = track_map.m_inter_graph_list.at(
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
			viter1->second, viter2->second);

	return true;
}