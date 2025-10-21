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

#include <TrackMap.h>
#include <Global.h>
#include <MainSettings.h>
#include <RenderView.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <TrackGroup.h>
#include <DataManager.h>
#include <Stencil.h>
#include <dbscan.h>
#include <kmeans.h>
#include <exmax.h>
#include <StencilCompare.h>
#include <Clusterizer.h>
#include <base_vol_reader.h>
#include <lbl_reader.h>
#include <msk_writer.h>
#include <Texture.h>
#include <VolumeRenderer.h>
#include <VertexArray.h>
#include <Ruler.h>
#include <VolCache4D.h>
#include <functional>
#include <algorithm>
#include <limits>
#include <boost/qvm/vec_access.hpp>

using namespace flrd;

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

//auto tracking
void TrackMapProcessor::GenMap()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	//get trace group
	view->CreateTrackGroup();
	TrackGroup* trkg = view->GetTrackGroup();
	if (!trkg)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	auto reader = vd->GetReader();
	if (!reader)
		return;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return;

	std::string info_str;
	int frames = reader->GetTimeNum();
	size_t ticks = (glbin_settings.m_track_iter + 2) * frames;
	size_t count = 0;
	//start progress
	info_str = "Generating track map.\n";
	WriteInfo(s2ws(info_str));
	SetProgress(0, info_str);

	//get and set parameters
	flrd::pTrackMap track_map = trkg->GetTrackMap();
	SetTrackMap(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	SetBits(vd->GetBits());
	SetScale(vd->GetScalarScale());
	SetSizes(resx, resy, resz);
	SetSpacings(spcx, spcy, spcz);
	SetSizeThresh(glbin_settings.m_component_size);
	SetContactThresh(glbin_settings.m_contact_factor);
	SetSimilarThresh(glbin_settings.m_similarity);
	//register file reading and deleteing functions
	cache_queue->SetHandleFlags(
		flvr::CQCallback::HDL_DATA |
		flvr::CQCallback::HDL_LABEL |
		flvr::CQCallback::SAV_LABEL |
		flvr::CQCallback::TIME_COND0);
	cache_queue->set_max_size(4);
	//merge/split
	SetMerge(glbin_settings.m_try_merge);
	SetSplit(glbin_settings.m_try_split);

	//start timing
	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	//initialization
	for (int i = 0; i < frames; ++i)
	{
		InitializeFrame(i);
		WriteInfo(L"Time point " + std::to_wstring(i) + L" initialized.\n");
		SetProgress(static_cast<int>(100.0 * count / ticks), info_str);
		count++;

		if (i < 1)
			continue;

		//link maps 1 and 2
		LinkFrames(i - 1, i);
		WriteInfo(L"Time point " + std::to_wstring(i) + L" linked.\n");

		//check contacts and merge cells
		ResolveGraph(i - 1, i);
		ResolveGraph(i, i - 1);
		WriteInfo(L"Time point " + std::to_wstring(i) + L" merged.\n");

		if (i < 2)
			continue;

		//further process
		ProcessFrames(i - 2, i - 1);
		ProcessFrames(i - 1, i - 2);
		WriteInfo(L"Time point " + std::to_wstring(i) + L" processed.\n");
	}
	//last frame
	ProcessFrames(frames - 2, frames - 1);
	ProcessFrames(frames - 1, frames - 2);
	WriteInfo(L"Time point " + std::to_wstring(frames - 1) + L" processed.\n");
	SetProgress(static_cast<int>(100.0 * count / ticks), info_str);

	//iterations
	for (size_t iteri = 0; iteri < glbin_settings.m_track_iter; ++iteri)
	{
		for (int i = 2; i <= frames; ++i)
		{
			//further process
			ProcessFrames(i - 2, i - 1);
			ProcessFrames(i - 1, i - 2);
			WriteInfo(L"Time point " + std::to_wstring(i - 1) + L" processed.\n");
			SetProgress(static_cast<int>(100.0 * count / ticks), info_str);
			count++;
		}
	}

	//consistent colors
	if (glbin_settings.m_consistent_color)
	{
		WriteInfo(L"Set colors for frame 0\n");
		MakeConsistent(0);
		//remaining frames
		for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
		{
			WriteInfo(L"Set colors for frame " + std::to_wstring(fi) + L"\n");
			SetProgress(static_cast<int>(100.0 * count / ticks), info_str);
			count++;

			MakeConsistent(fi - 1, fi);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
	WriteInfo(L"Wall clock time: " + std::to_wstring(time_span.count()) + L"s.\n");
	SetProgress(0, "");
	SetRange(0, 100);

	view->GetTraces(false);
}

void TrackMapProcessor::RefineMap(int t, bool erase_v)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	//get trace group
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return;
	TrackGroup* trkg = view->GetTrackGroup();
	if (!trkg)
		return;
	if (t < 0)
		WriteInfo(L"Refining track map for all time points.\n");
	else
		WriteInfo(L"Refining track map at time point " + std::to_wstring(t) + L"\n");

	//start progress
	bool clear_counters = false;
	flrd::pTrackMap track_map = trkg->GetTrackMap();
	int start_frame, end_frame;
	if (t < 0)
	{
		start_frame = 0;
		end_frame = static_cast<int>(track_map->GetFrameNum()) - 1;
		clear_counters = true;
	}
	else
		start_frame = end_frame = t;

	//get and set parameters
	SetTrackMap(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	SetBits(vd->GetBits());
	SetScale(vd->GetScalarScale());
	SetSizes(resx, resy, resz);
	SetSpacings(spcx, spcy, spcz);
	SetSizeThresh(glbin_settings.m_component_size);
	SetContactThresh(glbin_settings.m_contact_factor);
	SetSimilarThresh(glbin_settings.m_similarity);
	//register file reading and deleteing functions
	cache_queue->SetHandleFlags(
		flvr::CQCallback::HDL_DATA |
		flvr::CQCallback::HDL_LABEL |
		flvr::CQCallback::SAV_LABEL |
		flvr::CQCallback::TIME_COND0);
	cache_queue->set_max_size(4);
	//merge/split
	SetMerge(glbin_settings.m_try_merge);
	SetSplit(glbin_settings.m_try_split);

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	//not sure if counters need to be cleared for all refinement
	//if (clear_counters)
	//	ClearCounters();
	//iterations
	for (size_t iteri = 0; iteri < glbin_settings.m_track_iter; ++iteri)
	{
		for (int i = start_frame - 1; i <= end_frame; ++i)
		{
			//further process
			ProcessFrames(i, i + 1, erase_v);
			ProcessFrames(i + 1, i, erase_v);
			WriteInfo(L"Time point " + std::to_wstring(i + 1) + L" processed.\n");
		}
	}

	//consistent colors
	if (glbin_settings.m_consistent_color)
	{
		if (t < 0)
		{
			WriteInfo(L"Set colors for frame 0\n");
			MakeConsistent(0);
			//remaining frames
			for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
			{
				WriteInfo(L"Set colors for frame " + std::to_wstring(fi) + L"\n");
				MakeConsistent(fi - 1, fi);
			}
		}
		else
		{
			MakeConsistent(t - 1, t);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
	WriteInfo(L"Wall clock time: " + std::to_wstring(time_span.count()) + L"s.\n");

	view->GetTraces(false);
}

void TrackMapProcessor::SetUncertainLow(unsigned int value)
{
	m_uncertain_low = value;
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (trkg)
		trkg->SetUncertainLow(value);
}

void TrackMapProcessor::SetTrackMap(const pTrackMap& map)
{
	m_map = map;
}

void TrackMapProcessor::SetSizes(size_t nx, size_t ny, size_t nz)
{
	m_map->m_size_x = nx;
	m_map->m_size_y = ny;
	m_map->m_size_z = nz;
}

void TrackMapProcessor::SetBits(size_t bits)
{
	m_map->m_data_bits = bits;
}

void TrackMapProcessor::SetScale(double scale)
{
	m_map->m_scale = static_cast<float>(scale);
}

void TrackMapProcessor::SetSpacings(double spcx, double spcy, double spcz)
{
	m_map->m_spc_x = static_cast<float>(spcx);
	m_map->m_spc_y = static_cast<float>(spcy);
	m_map->m_spc_z = static_cast<float>(spcz);
}

void TrackMapProcessor::SetClusterNum(int val)
{
	m_cluster_num = val;
}

int TrackMapProcessor::GetClusterNum()
{
	return m_cluster_num;
}

bool TrackMapProcessor::InitializeFrame(size_t frame)
{
	//get label and data from cache
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	flvr::VolCache4D* cache = cache_queue->get(frame);
	if (!cache)
		return false;
	void* data = cache->GetRawData();
	void* label = cache->GetRawLabel();
	if (!data || !label)
		return false;

	//add one empty cell list to track_map
	m_map->m_celp_list.push_back(CelpList());
	CelpList &cell_list = m_map->m_celp_list.back();
	CelpListIter iter;
	//in the meanwhile build the intra graph
	m_map->m_intra_graph_list.push_back(CellGraph());

	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
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

		if (m_map->m_data_bits == 8)
			data_value = ((unsigned char*)data)[index] / 255.0f;
		else if (m_map->m_data_bits == 16)
			data_value = ((unsigned short*)data)[index] * m_map->m_scale / 65535.0f;

		iter = cell_list.find(label_value);
		if (iter != cell_list.end())
		{
			iter->second->Inc(i, j, k, data_value);
		}
		else
		{
			Cell* cell = new Cell(label_value);
			cell->Inc(i, j, k, data_value);
			cell_list.insert(std::pair<unsigned int, Celp>
				(label_value, Celp(cell)));
		}
	}

	//prune list
	for (auto celli = cell_list.begin();
		celli != cell_list.end(); )
	{
		if (celli->second->GetSize() < m_size_thresh)
			celli = cell_list.erase(celli);
		else
			++celli;
	}
	//prune data
	size_t tsize = nx * ny * nz;
	for (index = 0; index < tsize; ++index)
	{
		label_value = ((unsigned int*)label)[index];
		if (label_value &&
			cell_list.find(label_value) ==
			cell_list.end())
			((unsigned int*)label)[index] = 0;
	}
	//label modified, save before delete
	cache_queue->set_modified(frame);

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
	m_map->m_vertices_list.push_back(VertexList());
	VertexList &vertex_list = m_map->m_vertices_list.back();
	for (iter = cell_list.begin();
	iter != cell_list.end(); ++iter)
	{
		if (iter->second->GetSize() < m_size_thresh)
			continue;
		Verp vertex(new Vertex(iter->second->Id()));
		vertex->SetCenter(iter->second->GetCenter());
		vertex->SetSizeUi(iter->second->GetSizeUi());
		vertex->SetSizeD(iter->second->GetSizeD());
		vertex->AddCell(iter->second);
		iter->second->AddVertex(vertex);
		vertex_list.insert(std::pair<unsigned int, Verp>
			(vertex->Id(), vertex));
	}

	m_map->m_frame_num++;
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
				AddContact(intra_graph, celp, iter->second, contact_value); \
		} \
	}

bool TrackMapProcessor::CheckCellContact(
	Celp &celp, void *data, void *label,
	size_t ci, size_t cj, size_t ck)
{
	int ec = 0;//external count
	int cc = 0;//contact count
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	size_t indexn;//neighbor index
	unsigned int idn;//neighbor id
	float valuen;//neighbor vlaue
	size_t index = nx*ny*ck + nx*cj + ci;
	unsigned int id = celp->Id();
	float value;
	size_t data_bits = m_map->m_data_bits;
	float scale = m_map->m_scale;
	if (data_bits == 8)
		value = ((unsigned char*)data)[index] / 255.0f;
	else if (data_bits == 16)
		value = ((unsigned short*)data)[index] * scale / 65535.0f;
	float contact_value;
	CellGraph &intra_graph = m_map->m_intra_graph_list.back();
	CelpList &cell_list = m_map->m_celp_list.back();
	CelpListIter iter;

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
		celp->IncExt(value);

		//expand search range if it's external but not contacting
		//if (cc == 0)
		//	CheckCellDist(cell, label, ci, cj, ck);
	}

	return true;
}

bool TrackMapProcessor::CheckCellDist(
	Celp &celp, void *label, size_t ci, size_t cj, size_t ck)
{
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	float spcx = m_map->m_spc_x;
	float spcy = m_map->m_spc_y;
	float spcz = m_map->m_spc_z;
	size_t indexn;//neighbor index
	unsigned int idn;//neighbor id
	unsigned int id = celp->Id();
	CellGraph &intra_graph = m_map->m_intra_graph_list.back();
	CelpList &cell_list = m_map->m_celp_list.back();
	CelpListIter iter;
	float dist_v, dist_s;

	//search range
	int r = int(sqrt(m_size_thresh)/2.0+0.5);
	r = r < 1 ? 1 : r;
	for (int k = -r; k<=r; ++k)
	for (int j = -r; j<=r; ++j)
	for (int i = -r; i<=r; ++i)
	{
		if ((int)ck + k < 0 || ck + k >= nz ||
			(int)cj + j < 0 || cj + j >= ny ||
			(int)ci + i < 0 || ci + i >= nx)
			continue;
		indexn = nx*ny*(ck + k) + nx*(cj + j) + ci + i;
		idn = ((unsigned int*)label)[indexn];
		if (!idn || idn == id)
			continue;
		iter = cell_list.find(idn);
		if (iter != cell_list.end())
		{
			dist_v = static_cast<float>(sqrt(i*i + j*j + k*k));
			dist_s = sqrt(i*i*spcx*spcx + j*j*spcy*spcy + k*k*spcz*spcz);
			AddNeighbor(intra_graph, celp, iter->second, dist_v, dist_s);
		}
	}
	return true;
}

bool TrackMapProcessor::AddContact(CellGraph& graph,
	Celp &celp1, Celp &celp2, float contact_value)
{
	CelVrtx v1 = celp1->GetCelVrtx();
	CelVrtx v2 = celp2->GetCelVrtx();
	if (v1 == CellGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = celp1->Id();
		graph[v1].cell = celp1;
		celp1->SetCelVrtx(v1);
	}
	if (v2 == CellGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = celp2->Id();
		graph[v2].cell = celp2;
		celp2->SetCelVrtx(v2);
	}

	std::pair<CelEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 1;
		graph[e.first].size_d = contact_value;
		graph[e.first].dist_v = 0.0f;
		graph[e.first].dist_s = 0.0f;
	}
	else
	{
		graph[e.first].size_ui++;
		graph[e.first].size_d += contact_value;
		graph[e.first].dist_v = 0.0f;
		graph[e.first].dist_s = 0.0f;
	}

	return true;
}

bool TrackMapProcessor::AddNeighbor(CellGraph& graph,
	Celp &celp1, Celp &celp2,
	float dist_v, float dist_s)
{
	CelVrtx v1 = celp1->GetCelVrtx();
	CelVrtx v2 = celp2->GetCelVrtx();
	if (v1 == CellGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = celp1->Id();
		graph[v1].cell = celp1;
		celp1->SetCelVrtx(v1);
	}
	if (v2 == CellGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = celp2->Id();
		graph[v2].cell = celp2;
		celp2->SetCelVrtx(v2);
	}

	std::pair<CelEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 0;
		graph[e.first].size_d = 0.0f;
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
	size_t frame_num = m_map->m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num || f1 == f2)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	//get data and label
	flvr::VolCache4D* cache = cache_queue->get(f1);
	if (!cache)
		return false;
	void* data1 = cache->GetRawData();
	void* label1 = cache->GetRawLabel();
	if (!data1 || !label1)
		return false;
	cache = cache_queue->get(f2);
	if (!cache)
		return false;
	void* data2 = cache->GetRawData();
	void* label2 = cache->GetRawLabel();
	if (!data2 || !label2)
		return false;
	cache_queue->protect(f1);

	m_map->m_inter_graph_list.push_back(InterGraph());
	InterGraph &inter_graph = m_map->m_inter_graph_list.back();
	inter_graph.index = f1;
	inter_graph.counter = 0;

	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	float data_value1, data_value2;
	unsigned int label_value1, label_value2;
	Verp v1, v2;
	Celp cl1, cl2;

	for (i = 0; i < nx; ++i)
	for (j = 0; j < ny; ++j)
	for (k = 0; k < nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		label_value1 = ((unsigned int*)label1)[index];
		label_value2 = ((unsigned int*)label2)[index];

		if (!label_value1 || !label_value2)
			continue;

		if (m_map->m_data_bits == 8)
		{
			data_value1 = ((unsigned char*)data1)[index] / 255.0f;
			data_value2 = ((unsigned char*)data2)[index] / 255.0f;
		}
		else if (m_map->m_data_bits == 16)
		{
			data_value1 = ((unsigned short*)data1)[index] * m_map->m_scale / 65535.0f;
			data_value2 = ((unsigned short*)data2)[index] * m_map->m_scale / 65535.0f;
		}

		cl1 = GetCell(f1, label_value1);
		cl2 = GetCell(f2, label_value2);
		v1 = GetVertex(cl1);
		v2 = GetVertex(cl2);
		if (!v1 || !v2)
			continue;

		if (v1->GetSizeUi() < m_size_thresh ||
			v2->GetSizeUi() < m_size_thresh)
			continue;

		LinkVertices(inter_graph,
			v1, v2, f1, f2,
			std::min(data_value1, data_value2));
	}

	cache_queue->unprotect(f1);

	return true;
}

bool TrackMapProcessor::LinkVertices(InterGraph& graph,
	Verp &vertex1, Verp &vertex2,
	size_t f1, size_t f2, float overlap_value)
{
	Vrtx v1 = vertex1->GetInterVert(graph);
	Vrtx v2 = vertex2->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex1->Id();
		graph[v1].frame = static_cast<unsigned int>(f1);
		graph[v1].count = 1;
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = static_cast<unsigned int>(f2);
		graph[v2].count = 1;
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}

	std::pair<Edge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 1;
		graph[e.first].size_d = overlap_value;
		fluo::Point p1 = vertex1->GetCenter();
		fluo::Point p2 = vertex2->GetCenter();
		graph[e.first].dist = float((p1 - p2).length());
		graph[e.first].link = 1;
		graph[e.first].count = 1;
	}
	else
	{
		graph[e.first].size_ui++;
		graph[e.first].size_d += overlap_value;
	}

	return true;
}

bool TrackMapProcessor::LinkOrphans(InterGraph& graph, Verp &vertex)
{
	if (!vertex)
		return false;

	fluo::Point pos = vertex->GetCenter();

	Verp vertex1;
	size_t valence1;
	fluo::Point pos1;
	double d_min = -1;
	double d;
	Verp v1_min;

	//find closest
	VertexListIter iter;
	VertexList &vertex_list = m_map->m_vertices_list.at(m_frame2);
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
	fluo::BBox box = vertex->GetBox();
	fluo::BBox box1 = v1_min->GetBox();
	box.extend_mul(1.0);
	box1.extend_mul(1.0);
	if (!box.intersect(box1))
		return false;

	//link vertices
	Vrtx v1 = vertex->GetInterVert(graph);
	Vrtx v2 = v1_min->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex->Id();
		graph[v1].frame = static_cast<unsigned int>(m_frame1);
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
		graph[v2].frame = static_cast<unsigned int>(m_frame2);
		graph[v2].count = 1;
		graph[v2].vertex = v1_min;
		v1_min->SetInterVert(graph, v2);
	}
	else
		graph[v2].count++;

	std::pair<Edge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = 0;
		graph[e.first].size_d = 0.0f;
		graph[e.first].dist = d;
		graph[e.first].link = 1;
		graph[e.first].count = 1;
	}
	else
		graph[e.first].count++;

	return true;
}

bool TrackMapProcessor::IsolateVertex(InterGraph& graph, Verp &vertex)
{
	Vrtx v1, v2;
	std::pair<AdjIter, AdjIter> adj_verts;
	AdjIter inter_iter;
	std::pair<Edge, bool> edge;

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
	Verp &vertex1, Verp &vertex2,
	size_t f1, size_t f2)
{
	Vrtx v1 = vertex1->GetInterVert(graph);
	Vrtx v2 = vertex2->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex1->Id();
		graph[v1].frame = static_cast<unsigned int>(f1);
		//reset
		graph[v1].count = 0;
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = static_cast<unsigned int>(f2);
		//reset
		graph[v2].count = 0;
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}

	std::pair<Edge, bool> edge;
	edge = boost::edge(v1, v2, graph);
	if (!edge.second)
	{
		edge = boost::add_edge(v1, v2, graph);
		graph[edge.first].size_ui = std::max(
			vertex1->GetSizeUi(), vertex2->GetSizeUi());
		graph[edge.first].size_d = std::max(
			vertex1->GetSizeD(), vertex2->GetSizeD());
		fluo::Point p1 = vertex1->GetCenter();
		fluo::Point p2 = vertex2->GetCenter();
		graph[edge.first].dist = float((p1 - p2).length());
		graph[edge.first].link = 2;
		//reset
		graph[edge.first].count = 0;
	}
	else
	{
		graph[edge.first].size_ui = std::max(
			vertex1->GetSizeUi(), vertex2->GetSizeUi());
		graph[edge.first].size_d = std::max(
			vertex1->GetSizeD(), vertex2->GetSizeD());
		graph[edge.first].link = 2;
		//reset
		graph[edge.first].count = 0;
	}

	return true;
}

bool TrackMapProcessor::UnlinkVertices(InterGraph& graph,
	Verp &vertex1, Verp &vertex2)
{
	Vrtx v1, v2;

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

	std::pair<Edge, bool> edge;
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
	if (frame1 >= m_map->m_frame_num ||
		frame2 >= m_map->m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list1 = m_map->m_vertices_list.at(frame1);
	VertexList &vertex_list2 = m_map->m_vertices_list.at(frame2);
	CellGraph &intra_graph = m_map->m_intra_graph_list.at(frame2);
	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	VertexListIter iter;
	Vrtx v1, v2;
	std::pair<AdjIter, AdjIter> adj_verts;
	AdjIter inter_iter;
	std::vector<Celw> cells;
	std::vector<CellBin> cell_bins;
	CellBinIter pwcell_iter;
	Verp vertex2;

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

bool TrackMapProcessor::ProcessFrames(size_t frame1, size_t frame2, bool erase_v)
{
	if (frame1 >= m_map->m_frame_num ||
		frame2 >= m_map->m_frame_num ||
		frame1 == frame2)
		return false;

	VertexList &vertex_list = m_map->m_vertices_list.at(frame1);
	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	m_frame1 = frame1;
	m_frame2 = frame2;

	//compute segmentation
	unsigned int count_min = 0;
	m_major_converge = false;
	if (m_merge || m_split)
		m_major_converge = get_segment(vertex_list, inter_graph, count_min);

	VertexListIter iter;

	for (iter = vertex_list.begin();
		iter != vertex_list.end();)
	{
		ProcessVertex(iter->second, inter_graph, count_min);
		//see if it is removed
		////debug
		//Verp vert = iter->second;
		if (erase_v && iter->second->GetRemovedFromGraph())
			iter = vertex_list.erase(iter);
		else
			++iter;
	}

	inter_graph.counter++;

	return true;
}

//make id consistent
bool TrackMapProcessor::MakeConsistent(size_t f)
{
	if (f >= m_map->m_frame_num)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	//get label
	flvr::VolCache4D* cache = cache_queue->get(f);
	if (!cache)
		return false;
	unsigned int* label = (unsigned int*)(cache->GetRawLabel());
	if (!label)
		return false;
	//size
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	unsigned long long size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	CellIDMap id_map;
	//scan label for cell ids
	unsigned int lv;
	for (index = 0; index < size; ++index)
	{
		lv = label[index];
		if (!lv)
			continue;
		//find in id map
		auto it = id_map.find(lv);
		if (it != id_map.end())
		{
			//already processed
			label[index] = it->second;
		}
		else
		{
			unsigned int id0 = GetUniCellID(f, lv);
			if (id0 != lv)
			{
				unsigned int newid = GetNewCellID(f, id0, true);//inc because its the same frame
				if (!newid)
					continue;
				ReplaceCellID(lv, newid, f);
				label[index] = newid;
				id_map.insert(std::pair<unsigned int,
					unsigned int>(lv, newid));
			}
		}
	}

	cache_queue->set_modified(f);
	return true;
}

bool TrackMapProcessor::MakeConsistent(size_t f1, size_t f2)
{
	if (f1 >= m_map->m_frame_num ||
		f2 >= m_map->m_frame_num ||
		f1 == f2)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	//get label
	flvr::VolCache4D* cache = cache_queue->get(f2);
	if (!cache)
		return false;
	unsigned int* label = (unsigned int*)(cache->GetRawLabel());
	if (!label)
		return false;
	//size
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	unsigned long long size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	//InterGraph &inter_graph = m_map->m_inter_graph_list.at(
	//	f1 > f2 ? f2 : f1);
	CellIDMap id_map;
	//scan label for cell ids
	unsigned int lv;
	for (index = 0; index < size; ++index)
	{
		lv = label[index];
		if (!lv)
			continue;
		//find in id map
		auto it = id_map.find(lv);
		if (it != id_map.end() &&
			lv != it->second)
		{
			//already processed
			label[index] = it->second;
		}
		else
		{
			unsigned int id0 = GetTrackedID(f2, f1, lv);//track back
			if (id0 == lv)
			{
				id_map.insert(std::pair<unsigned int,
					unsigned int>(lv, id0));
			}
			else
			{
				unsigned int newid = GetNewCellID(f2, id0);
				if (!newid)
					continue;
				if (newid != lv)
				{
					ReplaceCellID(lv, newid, f2);
					label[index] = newid;
				}
				id_map.insert(std::pair<unsigned int,
					unsigned int>(lv, newid));
			}
		}
	}

	cache_queue->set_modified(f2);

	return true;
}

//clear counters
bool TrackMapProcessor::ClearCounters()
{
	size_t listsize = m_map->m_inter_graph_list.size();
	for (size_t i = 0; i < listsize; ++i)
	{
		InterGraph &graph = m_map->m_inter_graph_list.at(i);
		graph.counter = 0;

		for (auto iv : boost::make_iterator_range(vertices(graph)))
		{
			graph[iv].count = 0;
		}
		for (auto ie : boost::make_iterator_range(edges(graph)))
		{
			graph[ie].count = 0;
		}
	}
	return true;
}

//vertex matching routines
//find out current valence of a vertex
bool TrackMapProcessor::GetValence(Verp &vertex, InterGraph &graph,
	size_t &valence)
{
	valence = 0;
	if (!vertex)
		return false;
	Vrtx v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	Vrtx v1;
	std::pair<Edge, bool> edge;
	unsigned int link;

	//set flag for link
	std::pair<AdjIter, AdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (AdjIter inter_iter = adj_verts.first;
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

bool TrackMapProcessor::GetValence(Verp &vertex, InterGraph &graph,
	size_t &valence, std::vector<Edge> &edges)
{
	valence = 0;
	if (!vertex)
		return false;
	Vrtx v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	Vrtx v1;
	std::pair<Edge, bool> edge;
	unsigned int link;

	//set flag for link
	std::pair<AdjIter, AdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (AdjIter inter_iter = adj_verts.first;
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

bool TrackMapProcessor::GetValence(Verp &vertex, InterGraph &graph,
	size_t &valence, std::vector<Edge> &all_edges,
	std::vector<Edge> &linked_edges)
{
	valence = 0;
	if (!vertex)
		return false;
	Vrtx v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	Vrtx v1;
	std::pair<Edge, bool> edge;
	unsigned int link;

	//set flag for link
	std::pair<AdjIter, AdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (AdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		////debug
		//Verp vert1 = graph[v1].vertex.lock();
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

//get uncertain edges for segmentation
bool TrackMapProcessor::GetUncertainEdges(Verp &vertex, InterGraph &graph,
	unsigned int min_count, std::vector<Edge> &uncertain_edges)
{
	if (!vertex)
		return false;
	Vrtx v0 = vertex->GetInterVert(graph);
	if (v0 == InterGraph::null_vertex())
		return false;

	Vrtx v1;
	std::pair<Edge, bool> edge;
	unsigned int link;
	unsigned int count;

	std::pair<AdjIter, AdjIter> adj_verts =
		boost::adjacent_vertices(v0, graph);
	//for each adjacent vertex
	for (AdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		////debug
		//Verp vert1 = graph[v1].vertex.lock();
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			link = graph[edge.first].link;
			count = graph[edge.first].count;
			if (link == 1 || link == 2 || count >= min_count)
				uncertain_edges.push_back(edge.first);
		}
	}

	return true;
}

//simple match of the max overlap
bool TrackMapProcessor::LinkEdgeSize(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges, bool calc_sim)
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
	if (!result)//otherwise, it may be useful (uncertain)
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
bool TrackMapProcessor::UnlinkEdgeSize(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges, bool calc_sim)
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

//unlink edge by count
bool TrackMapProcessor::UnlinkEdgeCount(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges)
{
	if (edges.size() < 2)
		return false;

	//sort edges
	std::sort(edges.begin(), edges.end(),
		std::bind(comp_edge_count, std::placeholders::_1,
			std::placeholders::_2, graph));
	//suppose we have more than 2 edges, find where to cut
	//if 0 hasn't been linked/unlinked many times
	for (size_t i = 1; i < edges.size(); ++i)
	{
		if (graph[edges[i]].count < graph[edges[0]].count)
			unlink_edge(edges[i], graph);
		return true;
	}
	//otherwise, it's uncertain
	return false;
}

//unlink last edge
bool TrackMapProcessor::UnlinkEdgeLast(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges)
{
	if (edges.size() < 2)
		return false;
	size_t lasti = edges.size() - 1;
	unlink_edge(edges[lasti], graph);
	return true;
}

//unlink edge by extended alternating path
bool TrackMapProcessor::UnlinkAlterPath(InterGraph &graph, Verp &vertex,
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

bool TrackMapProcessor::GetAlterPath(InterGraph &graph, Verp &vertex,
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

bool TrackMapProcessor::UnlinkAlterPathMaxMatch(InterGraph &graph, Verp &vertex,
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

bool TrackMapProcessor::UnlinkAlterPathSize(InterGraph &graph, Verp &vertex,
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

bool TrackMapProcessor::UnlinkAlterPathCount(InterGraph &graph, Verp &vertex,
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

bool TrackMapProcessor::UnlinkAlterPathConn(InterGraph &graph, Verp &vertex,
	PathList &paths)
{
	if (paths.size() < 2)
		return false;

	std::vector<size_t> plist;
	std::pair<AdjIter, AdjIter> adj_verts;
	std::pair<Edge, bool> edge;
	Vrtx v1;
	Vrtx v0 = paths[0][0].vert;
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
		for (AdjIter inter_iter = adj_verts.first;
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
bool TrackMapProcessor::UnlinkVertexSize(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges)
{
	if (edges.size() < 2)
		return false;

	bool result = false;

	//find vertex that is not similar
	Verp vert1;
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

//fix multi-link by segmentation
bool TrackMapProcessor::UnlinkSegment(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &linked_edges, bool calc_sim, bool segment,
	unsigned int seg_count_min)
{
	bool result = false;
	if (!m_merge && !m_split)
		return result;
	//not converge and uncertainty not separate
	if (!m_major_converge && !segment)
		return result;

	//majority converged but uncertainty not separate
	if (m_major_converge && !segment)
	{
		if (calc_sim)
		{
			//merge edges if possible (DBSCAN)
			if (m_merge)
				result = MergeEdges(graph, vertex, linked_edges);
			//split vertex if possible (EM)
			if (!result && m_split)
				result = SplitVertex(graph, vertex, linked_edges);
		}
	}
	else//majority converged or uncertanty separate
	{
		std::vector<Edge> uncertain_edges;
		GetUncertainEdges(vertex, graph, seg_count_min, uncertain_edges);
		//merge edges if possible (DBSCAN)
		if (m_merge)
			result = MergeEdges(graph, vertex, uncertain_edges);
		//split vertex if possible (EM)
		if (!result && m_split)
			result = SplitVertex(graph, vertex, uncertain_edges);
	}

	return result;
}

//reduce valence by segmentation
bool TrackMapProcessor::MergeEdges(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges)
{
	if (edges.size() < 2)
		return false;

	Verp v1;
	CelpList list;
	Celp celp;
	CellBin cell_bin;

	//first, check if any out cells are touching
	for (size_t i = 0; i < edges.size(); ++i)
	{
		v1 = graph[boost::target(edges[i], graph)].vertex.lock();
		if (v1 == vertex)
			v1 = graph[boost::source(edges[i], graph)].vertex.lock();
		if (!v1) continue;

		//store all cells in the list temporarily
		for (auto iter = v1->GetCellsBegin();
			iter != v1->GetCellsEnd(); ++iter)
		{
			celp = iter->lock();
			if (celp)
			list.insert(std::pair<unsigned int, Celp>
				(celp->Id(), celp));
			cell_bin.push_back(*iter);
		}
	}
	//if a cell in the list has contacts that are also in the list,
	//try to group them
	if (!v1) return false;
	size_t frame = v1->GetFrame(graph);
	if (ClusterCellsMerge(list, frame))
	{
		//modify vertex list 2 if necessary
		VertexList &vertex_list = m_map->m_vertices_list.at(frame);
		MergeCells(vertex_list, cell_bin, frame);
		return true;
	}

	return false;
}

bool TrackMapProcessor::SplitVertex(InterGraph &graph, Verp &vertex,
	std::vector<Edge> &edges)
{
	if (!vertex || edges.size() < 2)
		return false;

	if (vertex->GetSplit())
		return false;
	vertex->SetSplit();
	
	CelpList list;
	Celp celp;

	//store all cells in the list temporarily
	for (auto iter = vertex->GetCellsBegin();
		iter != vertex->GetCellsEnd(); ++iter)
	{
		celp = iter->lock();
		if (celp)
			list.insert(std::pair<unsigned int, Celp>
				(celp->Id(), celp));
	}

	size_t frame = vertex->GetFrame(graph);
	CelpList outlist;
	if (ClusterCellsSplit(list, frame, edges.size(), outlist))
	{
		//remove input vertex
		RemoveVertex(graph, vertex);
		AddCells(outlist, frame);
		//remove vertex from another intergraph
		size_t gindex = graph.index == frame ? frame - 1 : frame;
		if (gindex < m_map->m_frame_num - 1)
		{
			InterGraph &graph2 = m_map->m_inter_graph_list.at(gindex);
			RemoveVertex(graph2, vertex);
		}

		LinkAddedCells(outlist, frame, frame - 1);
		LinkAddedCells(outlist, frame, frame + 1);

		return true;
	}

	return false;
}

bool TrackMapProcessor::ProcessVertex(Verp &vertex, InterGraph &graph,
	unsigned int seg_count_min)
{
	bool result = false;

	//get count
	unsigned int count = 0;
	Vrtx inter_vert = vertex->GetInterVert(graph);
	if (inter_vert != InterGraph::null_vertex())
		count = graph[inter_vert].count;
	//compute similarity
	bool calc_sim = get_random(count, graph);

	//get valence
	size_t valence;
	std::vector<Edge> all_edges;
	std::vector<Edge> linked_edges;
	GetValence(vertex, graph, valence, all_edges, linked_edges);
	if (valence == 0)
	{
		result = LinkEdgeSize(graph, vertex, all_edges, calc_sim);
		if (!result)	//find and link neighboring orphans
			result = LinkOrphans(graph, vertex);
	}
	else if (valence > 1)
	{
		//do not run unlink repeatedly if it has been
		//executed so many times
		if (calc_sim || !m_major_converge || !seg_count_min)
		{
			result = UnlinkEdgeSize(graph, vertex, linked_edges, calc_sim);
			//unlink edge by extended alternating path
			if (!result)
				result = UnlinkAlterPath(graph, vertex, calc_sim);
		}
		if (!result && !calc_sim && !m_major_converge && !seg_count_min)
			result = UnlinkEdgeCount(graph, vertex, linked_edges);

		//segmentation
		if (!result)
			result = UnlinkSegment(graph, vertex, linked_edges,
				calc_sim, seg_count_min < count, seg_count_min);
	}

	return result;
}

bool TrackMapProcessor::comp_edge_size(Edge &edge1,
	Edge &edge2, InterGraph& graph)
{
	return graph[edge1].size_d > graph[edge2].size_d;
}

bool TrackMapProcessor::comp_edge_count(Edge &edge1,
	Edge &edge2, InterGraph& graph)
{
	return graph[edge1].count > graph[edge2].count;
}

bool TrackMapProcessor::comp_path_size(Path &path1, Path &path2)
{
	path1.get_size(0);
	path1.get_size(1);
	path2.get_size(0);
	path2.get_size(1);
	return path1.get_max_size() > path2.get_max_size();
}

bool TrackMapProcessor::comp_path_mm(Path &path1, Path &path2)
{
	if (path1.size() && path2.size())
		return path1[0].max_value > path2[0].max_value;
	else
		return false;
}

bool TrackMapProcessor::similar_edge_size(Edge &edge1,
	Edge &edge2, InterGraph& graph)
{
	double s1 = graph[edge1].size_d;
	double s2 = graph[edge2].size_d;
	//float s1 = graph[edge1].size_ui;
	//float s2 = graph[edge2].size_ui;
	double d;
	if (s1 > 0.0 || s2 > 0.0)
	{
		d = fabs(s1 - s2) / std::max(s1, s2);
		return d < m_similar_thresh;
	}
	else
	{
		s1 = graph[edge1].dist;
		s2 = graph[edge2].dist;
		if (s1 > 0.0 || s2 > 0.0)
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
	path1.get_count(0);
	path1.get_count(1);
	path2.get_count(0);
	path2.get_count(1);
	return path1.get_max_count() < path2.get_max_count();
}

bool TrackMapProcessor::comp_path_count_rev(Path &path1, Path &path2)
{
	path1.get_count(0);
	path1.get_count(1);
	path2.get_count(0);
	path2.get_count(1);
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
		d = static_cast<float>(fabs((double)p1 - (double)p2) / std::max(p1, p2));
		if (d < m_similar_thresh)
			return true;
	}
	return false;
}

bool TrackMapProcessor::similar_vertex_size(Verp& v1, Verp& v2)
{
	double s1 = v1->GetSizeD();
	double s2 = v2->GetSizeD();
	if (fabs(s1 - s2) / std::max(s1, s2) < m_similar_thresh)
		return true;
	else
		return false;
}

void TrackMapProcessor::link_edge(Edge edge, InterGraph &graph, unsigned int value)
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

void TrackMapProcessor::unlink_edge(Edge edge, InterGraph &graph, unsigned int value)
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

bool TrackMapProcessor::get_alter_path(InterGraph &graph, Verp &vertex,
	Path &alt_path, VertVisitList &visited, int curl)
{
	if (!vertex)
		return false;
	Vrtx v0 = vertex->GetInterVert(graph);
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

	Vrtx v1;
	std::pair<AdjIter, AdjIter> adj_verts;
	std::pair<Edge, bool> edge;
	Verp vertex1;

	adj_verts = boost::adjacent_vertices(v0, graph);
	for (AdjIter inter_iter = adj_verts.first;
		inter_iter != adj_verts.second; ++inter_iter)
	{
		v1 = *inter_iter;
		edge = boost::edge(v0, v1, graph);
		if (edge.second)
		{
			vertex1 = graph[v1].vertex.lock();
			if (!vertex1)
				continue;
			if (visited.find(v1) ==
				visited.end())
			{
				//set back
				alt_path.back().edge_valid = true;
				alt_path.back().edge_value = static_cast<float>(graph[edge.first].size_d);
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
	size_t curl, Vrtx v0)
{
	float result = 0.0f;
	float value;
	Vrtx vert;
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
			std::pair<Edge, bool> edge =
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

bool TrackMapProcessor::merge_cell_size(CelEdge &edge,
	Celp &celp1, Celp &celp2, CellGraph& graph)
{
	double osizef, c1sizef, c2sizef;
	osizef = graph[edge].size_d;
	c1sizef = celp1->GetSize();
	c2sizef = celp2->GetSize();
	return osizef / c1sizef > m_contact_thresh ||
			osizef / c2sizef > m_contact_thresh;
}

//get if segmentation is computed
bool TrackMapProcessor::get_segment(VertexList &vertex_list,
	InterGraph &inter_graph, unsigned int &count_thresh)
{
	count_thresh = 0;
	if (inter_graph.counter < 6)
		return false;

	UncertainHist hist;
	GetUncertainHist(hist, vertex_list, inter_graph);
	if (hist.empty())
		return false;
	//search for first peak
	UncertainHistIter idx_max;
	unsigned int count_max = 0;
	for (UncertainHistIter iter = hist.begin();
		iter != hist.end(); ++iter)
	{
		if (iter->second.count > count_max)
		{
			count_max = iter->second.count;
			idx_max = iter;
		}
	}
	if (count_max == 0)
		return false;

	UncertainBin major_bin = idx_max->second;

	UncertainHistIter idx_max2, idx_min;
	unsigned int count_max2, count_min;
	while (true)
	{
		//search for second peak
		count_max2 = 0;
		if (idx_max == hist.end())
			return false;
		for (UncertainHistIter iter = std::next(idx_max);
			iter != hist.end(); ++iter)
		{
			UncertainHistIter prv_iter = std::prev(iter);
			if (prv_iter == hist.end())
				return false;
			if (iter->second.count <= prv_iter->second.count)
				continue;
			if (iter->second.count > count_max2)
			{
				count_max2 = iter->second.count;
				idx_max2 = iter;
			}
		}
		if (count_max2 == 0)
			return false;

		//find valley
		count_min = count_max;
		for (UncertainHistIter iter = std::next(idx_max);
			iter != idx_max2; ++iter)
		{
			if (iter->second.count < count_min)
			{
				count_min = iter->second.count;
				idx_min = iter;
			}
		}

		//count_min should be much smaller than count_max2
		if (!similar_count(count_max2, count_min))
		{
			count_thresh = idx_min->second.level;
			break;
		}
		else
			idx_max = idx_max2;
	}

	return get_major_converge(inter_graph, m_frame1, major_bin);
}

bool TrackMapProcessor::get_major_converge(InterGraph &inter_graph, size_t vertex_frame, UncertainBin &major_bin)
{
	bool result = false;
	if (vertex_frame == inter_graph.index)
	{
		if (major_bin == inter_graph.major_bin[0])
			result = true;
		inter_graph.major_bin[0] = major_bin;
	}
	else
	{
		if (major_bin == inter_graph.major_bin[1])
			result = true;
		inter_graph.major_bin[1] = major_bin;
	}

	return result;
}

bool TrackMapProcessor::similar_count(unsigned int count1, unsigned int count2)
{
	if (count1 > 0 || count2 > 0)
	{
		double d = double(abs(int(count1 - count2))) /
			std::max(count1, count2);
		if (d < 0.65)
			return true;
	}
	return false;
}

//determine if cells on intragraph can be merged
bool TrackMapProcessor::GroupCells(std::vector<Celw> &cells,
	std::vector<CellBin> &cell_bins, CellGraph &intra_graph,
	f_merge_cell merge_cell)
{
	Celp celp2, celp2c;
	CelVrtx c2, c2c;
	std::pair<CelAdjIter, CelAdjIter> adj_cells;
	CelAdjIter intra_iter;
	bool added;
	std::pair<CelEdge, bool> intra_edge;
	bool result = false;

	for (auto pwcell_iter = cells.begin();
		pwcell_iter != cells.end(); ++pwcell_iter)
	{
		celp2 = pwcell_iter->lock();
		if (!celp2)
			continue;
		added = false;
		c2 = celp2->GetCelVrtx();
		if (c2 == CellGraph::null_vertex())
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
				celp2c = intra_graph[c2c].cell.lock();
				if (!celp2c)
					continue;
				//meausre for merging
				if ((this->*merge_cell)(intra_edge.first,
					celp2, celp2c, intra_graph))
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

bool TrackMapProcessor::EqualCells(Celw &cell1, Celw &cell2)
{
	return !cell1.owner_before(cell2) && !cell2.owner_before(cell1);
}

bool TrackMapProcessor::FindCellBin(CellBin &bin, Celw &cell)
{
	for (size_t i = 0; i < bin.size(); ++i)
		if (EqualCells(bin[i], cell))
			return true;
	return false;
}

bool TrackMapProcessor::AddCellBin(std::vector<CellBin> &bins, Celw &cell)
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

bool TrackMapProcessor::AddCellBin(std::vector<CellBin> &bins, Celw &cell1, Celw &cell2)
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
			Celp c2 = cell2.lock();
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
			Celp c1 = cell1.lock();
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

bool TrackMapProcessor::GreaterThanCellBin(Celp &cell1, CellBin &bin, Celw &cell2)
{
	Celp bin_cell;
	for (size_t i = 0; i < bin.size(); ++i)
	{
		if (EqualCells(bin[i], cell2))
			continue;
		bin_cell = bin[i].lock();
		if (!bin_cell)
			continue;
		if (cell1->GetSize() < bin_cell->GetSize()*3.0f)
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
	Celp celp0, celp;
	Verp vertex0, vertex;
	VertexListIter vert_iter;
	CellBin cell_list;
	CellBinIter cell_iter;

	for (size_t i = 0; i < bin.size(); ++i)
	{
		celp = bin[i].lock();
		if (!celp)
			continue;
		if (!vertex0)
		{
			celp0 = celp;
			vertex0 = celp0->GetVertex().lock();
		}
		else
		{
			vertex = celp->GetVertex().lock();
			if (!vertex ||
				vertex->Id() == vertex0->Id())
				continue;

			//relink inter graph
			if (frame > 0)
			{
				InterGraph &graph = m_map->m_inter_graph_list.at(frame - 1);
				RelinkInterGraph(vertex, vertex0, frame, graph, false);
			}
			if (frame < m_map->m_frame_num - 1)
			{
				InterGraph &graph = m_map->m_inter_graph_list.at(frame);
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
				celp = (*cell_iter).lock();
				if (celp)
				{
					vertex0->AddCell(celp, true);
					celp->AddVertex(vertex0);
				}
			}
		}
	}

	return true;
}

bool TrackMapProcessor::RelinkInterGraph(Verp &vertex, Verp &vertex0, size_t frame, InterGraph &graph, bool reset)
{
	Vrtx inter_vert, inter_vert0;
	std::pair<AdjIter, AdjIter> adj_verts;
	AdjIter inter_iter;
	std::pair<Edge, bool> e, e0;
	std::vector<Edge> edges_to_remove;
	std::vector<Edge>::iterator edge_to_remove;

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
				graph[inter_vert0].frame = static_cast<unsigned int>(frame);
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
				graph[e0.first].size_d = graph[e.first].size_d;
				graph[e0.first].dist = graph[e.first].dist;
				graph[e0.first].link = graph[e.first].link;
				graph[e0.first].count = graph[e.first].count;
			}
			else
			{
				graph[e0.first].size_ui += graph[e.first].size_ui;
				graph[e0.first].size_d += graph[e.first].size_d;
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

bool TrackMapProcessor::RemoveVertex(InterGraph& graph, Verp &vertex)
{
	Vrtx inter_vert = vertex->GetInterVert(graph);
	if (inter_vert != InterGraph::null_vertex())
	{
		std::pair<AdjIter, AdjIter> adj_verts;
		std::vector<Edge> edges_to_remove;
		std::vector<Edge>::iterator edge_to_remove;
		AdjIter inter_iter;
		adj_verts = boost::adjacent_vertices(inter_vert, graph);
		std::pair<Edge, bool> e;
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

bool TrackMapProcessor::Export(const std::wstring &filename)
{
	if (m_map->m_frame_num == 0 ||
		m_map->m_frame_num != m_map->m_celp_list.size() ||
		m_map->m_frame_num != m_map->m_vertices_list.size() ||
		m_map->m_frame_num != m_map->m_inter_graph_list.size() + 1)
		return false;

#ifdef _WIN32
	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
#else
    std::ofstream ofs(ws2s(filename), std::ios::out | std::ios::binary);
#endif
	if (ofs.bad())
		return false;

	//header
	std::string header = "FluoRender links";
	ofs.write(header.c_str(), header.size());

	//last operation
	WriteTag(ofs, TAG_FPCOUNT);
	WriteUint(ofs, m_map->m_counter);

	//number of frames
	WriteTag(ofs, TAG_NUM);
	size_t num = m_map->m_frame_num;
	WriteUint(ofs, static_cast<unsigned int>(num));

	VertexListIter iter;
	Verp vertex;
	CelEdgeIter intra_iter;
	std::pair<CelEdgeIter, CelEdgeIter> intra_pair;
	CelVrtx intra_vert;
	EdgeIter inter_iter;
	std::pair<EdgeIter, EdgeIter> inter_pair;
	Vrtx inter_vert0, inter_vert1;
	size_t edge_num;
	//write each frame
	for (size_t i = 0; i < num; ++i)
	{
		WriteTag(ofs, TAG_FRAM);
		//frame id
		WriteUint(ofs, static_cast<unsigned int>(i));

		//vertex list
		VertexList &vertex_list = m_map->m_vertices_list.at(i);
		//vertex number
		WriteUint(ofs, static_cast<unsigned int>(vertex_list.size()));
		//write each vertex
		for (iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
		{
			vertex = iter->second;
			WriteVertex(ofs, vertex);
		}
		//write intra edges
		CellGraph &intra_graph = m_map->m_intra_graph_list.at(i);
		intra_pair = edges(intra_graph);
		edge_num = 0;
		for (intra_iter = intra_pair.first;
		intra_iter != intra_pair.second;
			++intra_iter)
			edge_num++;
		//intra edge num
		WriteUint(ofs, static_cast<unsigned int>(edge_num));
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
			WriteDouble(ofs, intra_graph[*intra_iter].size_d);
			//distance
			WriteTag(ofs, TAG_VER220);
			WriteDouble(ofs, intra_graph[*intra_iter].dist_v);
			WriteDouble(ofs, intra_graph[*intra_iter].dist_s);
		}
		//write inter edges
		if (i == 0)
			continue;
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(i - 1);
		//write index and counter
		WriteTag(ofs, TAG_VER220);
		//index
		WriteUint(ofs, static_cast<unsigned int>(inter_graph.index));
		//counter
		WriteUint(ofs, static_cast<unsigned int>(inter_graph.counter));
		//get edge number
		inter_pair = boost::edges(inter_graph);
		edge_num = 0;
		for (inter_iter = inter_pair.first;
		inter_iter != inter_pair.second;
			++inter_iter)
			edge_num++;
		//inter edge number
		WriteUint(ofs, static_cast<unsigned int>(edge_num));
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
			WriteDouble(ofs, inter_graph[*inter_iter].size_d);
			WriteDouble(ofs, inter_graph[*inter_iter].dist);
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

bool TrackMapProcessor::Import(const std::wstring &filename)
{
	//clear everything
	m_map->Clear();

#ifdef _WIN32
	std::ifstream ifs(filename, std::ios::in | std::ios::binary);
#else
    std::ifstream ifs(ws2s(filename), std::ios::in | std::ios::binary);
#endif
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
		m_map->m_counter = ReadUint(ifs);
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
	Celp celp1, celp2;
	CelpListIter cell_iter;
	unsigned int size_ui;
	double size_f;
	double dist_v, dist_s;
	Verp vertex1, vertex2;
	VertexListIter vertex_iter;
	double dist;
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
		m_map->m_vertices_list.push_back(VertexList());
		VertexList &vertex_list1 = m_map->m_vertices_list.back();
		//cell list
		m_map->m_celp_list.push_back(CelpList());
		CelpList &cell_list = m_map->m_celp_list.back();
		//vertex number
		vertex_num = ReadUint(ifs);
		//read each vertex
		for (size_t j = 0; j < vertex_num; ++j)
			ReadVertex(ifs, vertex_list1, cell_list);
		//intra graph
		m_map->m_intra_graph_list.push_back(CellGraph());
		CellGraph &intra_graph = m_map->m_intra_graph_list.back();
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
				celp1 = cell_iter->second;
			//second cell
			id2 = ReadUint(ifs);
			cell_iter = cell_list.find(id2);
			if (cell_iter == cell_list.end())
				edge_exist = false;
			else
				celp2 = cell_iter->second;
			//add edge
			size_ui = ReadUint(ifs);
			size_f = ReadDouble(ifs);
			if (ReadTag(ifs) == TAG_VER220)
			{
				dist_v = ReadDouble(ifs);
				dist_s = ReadDouble(ifs);
			}
			else
			{
				ifs.unget();
				dist_v = dist_s = 0.0f;
			}
			if (edge_exist)
				AddIntraEdge(intra_graph, celp1, celp2,
					size_ui, size_f, dist_v, dist_s);
		}
		//inter graph
		if (i == 0)
			continue;
		//old vertex list
		VertexList &vertex_list0 = m_map->m_vertices_list.at(i - 1);
		m_map->m_inter_graph_list.push_back(InterGraph());
		InterGraph &inter_graph = m_map->m_inter_graph_list.back();
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
			size_f = ReadDouble(ifs);
			dist = ReadDouble(ifs);
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

	m_map->m_frame_num = num;
	return true;
}

bool TrackMapProcessor::ResetVertexIDs()
{
	for (size_t fi = 0; fi < m_map->m_frame_num; ++fi)
	{
		VertexList &vertex_list = m_map->m_vertices_list.at(fi);
		for (VertexListIter vertex_iter = vertex_list.begin();
		vertex_iter != vertex_list.end(); ++vertex_iter)
		{
			Verp vertex = vertex_iter->second;
			if (vertex->GetCellNum() <= 1)
				continue;
			unsigned int max_id = 0;
			float max_size = 0.0f;
			for (CellBinIter cell_iter = vertex->GetCellsBegin();
			cell_iter != vertex->GetCellsEnd(); ++cell_iter)
			{
				Celp celp = cell_iter->lock();
				if (celp)
				{
					if (celp->GetSize() > max_size)
					{
						max_size = static_cast<float>(celp->GetSize());
						max_id = celp->Id();
					}
				}
			}
			if (max_id)
			{
				vertex->Id(max_id);
				if (fi > 0)
				{
					InterGraph &inter_graph = m_map->m_inter_graph_list.at(fi - 1);
					Vrtx inter_vert = vertex->GetInterVert(inter_graph);
					if (inter_vert != InterGraph::null_vertex())
						inter_graph[inter_vert].id = max_id;
				}
				if (fi < m_map->m_frame_num - 1)
				{
					InterGraph &inter_graph = m_map->m_inter_graph_list.at(fi);
					Vrtx inter_vert = vertex->GetInterVert(inter_graph);
					if (inter_vert != InterGraph::null_vertex())
						inter_graph[inter_vert].id = max_id;
				}
			}
		}
	}

	return true;
}

void TrackMapProcessor::WriteVertex(std::ofstream& ofs, const Verp &vertex)
{
	WriteTag(ofs, TAG_VERT);
	WriteUint(ofs, vertex->Id());
	WriteUint(ofs, vertex->GetSizeUi());
	WriteDouble(ofs, vertex->GetSizeD());
	WritePoint(ofs, vertex->GetCenter());
	//cell number
	WriteUint(ofs, static_cast<unsigned int>(vertex->GetCellNum()));

	//cells
	Celp celp;
	for (auto iter = vertex->GetCellsBegin();
	iter != vertex->GetCellsEnd(); ++iter)
	{
		celp = iter->lock();
		if (!celp)
			continue;
		WriteCell(ofs, celp);
	}
}

void TrackMapProcessor::ReadVertex(std::ifstream& ifs,
	VertexList& vertex_list, CelpList& cell_list)
{
	if (ReadTag(ifs) != TAG_VERT)
		return;

	unsigned int id = ReadUint(ifs);
	if (vertex_list.find(id) != vertex_list.end())
		return;

	Verp vertex;
	vertex = Verp(new Vertex(id));
	vertex->SetSizeUi(ReadUint(ifs));
	vertex->SetSizeD(ReadDouble(ifs));
	fluo::Point p = ReadPoint(ifs);
	vertex->SetCenter(p);

	//cell number
	unsigned int num = ReadUint(ifs);
	//cells
	Celp celp;
	for (unsigned int i = 0; i < num; ++i)
	{
		celp = ReadCell(ifs, cell_list);
		vertex->AddCell(celp);
		celp->AddVertex(vertex);
	}

	vertex_list.insert(std::pair<unsigned int, Verp>
		(id, vertex));
}

bool TrackMapProcessor::AddIntraEdge(CellGraph& graph,
	Celp &celp1, Celp &celp2,
	unsigned int size_ui, double size_d,
	double dist_v, double dist_s)
{
	CelVrtx v1 = celp1->GetCelVrtx();
	CelVrtx v2 = celp2->GetCelVrtx();
	if (v1 == CellGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = celp1->Id();
		graph[v1].cell = celp1;
		celp1->SetCelVrtx(v1);
	}
	if (v2 == CellGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = celp2->Id();
		graph[v2].cell = celp2;
		celp2->SetCelVrtx(v2);
	}

	std::pair<CelEdge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = size_ui;
		graph[e.first].size_d = size_d;
		graph[e.first].dist_v = dist_v;
		graph[e.first].dist_s = dist_s;
	}
	else
		return false;

	return true;
}

bool TrackMapProcessor::AddInterEdge(InterGraph& graph,
	Verp &vertex1, Verp &vertex2,
	size_t f1, size_t f2,
	unsigned int size_ui, double size_d,
	double dist, unsigned int link,
	unsigned int v1_count,
	unsigned int v2_count,
	unsigned int edge_count)
{
	Vrtx v1 = vertex1->GetInterVert(graph);
	Vrtx v2 = vertex2->GetInterVert(graph);
	if (v1 == InterGraph::null_vertex())
	{
		v1 = boost::add_vertex(graph);
		graph[v1].id = vertex1->Id();
		graph[v1].frame = static_cast<unsigned int>(f1);
		graph[v1].vertex = vertex1;
		vertex1->SetInterVert(graph, v1);
	}
	graph[v1].count = v1_count;
	if (v2 == InterGraph::null_vertex())
	{
		v2 = boost::add_vertex(graph);
		graph[v2].id = vertex2->Id();
		graph[v2].frame = static_cast<unsigned int>(f2);
		graph[v2].vertex = vertex2;
		vertex2->SetInterVert(graph, v2);
	}
	graph[v2].count = v2_count;

	std::pair<Edge, bool> e = boost::edge(v1, v2, graph);
	if (!e.second)
	{
		e = boost::add_edge(v1, v2, graph);
		graph[e.first].size_ui = size_ui;
		graph[e.first].size_d = size_d;
		graph[e.first].dist = dist;
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

unsigned int TrackMapProcessor::GetTrackedID(
	size_t frame1, size_t frame2, unsigned int id)
{
	unsigned int rid = 0;
	size_t frame_num = m_map->m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);

	Celp celp = GetCell(frame1, id);
	if (!celp)
		return rid;
	Verp vert = GetVertex(celp);
	if (!vert)
		return rid;

	Vrtx v1 = vert->GetInterVert(inter_graph);
	if (v1 == InterGraph::null_vertex())
		return rid;

	std::pair<AdjIter, AdjIter> adj_verts =
		boost::adjacent_vertices(v1, inter_graph);
	Vrtx v2;
	Verp vert2;
	Celp celp2;
	std::pair<Edge, bool> edge;
	for (auto it = adj_verts.first;
		it != adj_verts.second; ++it)
	{
		v2 = *it;
		if (v2 == InterGraph::null_vertex() ||
			v2 == v1)
			continue;
		edge = boost::edge(v1, v2, inter_graph);
		if (!edge.second ||
			!inter_graph[edge.first].link)
			continue;
		vert2 = inter_graph[v2].vertex.lock();
		if (!vert2)
			continue;
		celp2 = (*vert2->GetCellsBegin()).lock();
		if (!celp2)
			continue;
		rid = celp2->Id();
		break;
	}

	return rid;
}

bool TrackMapProcessor::GetMappedCells(
	CelpList &sel_list1, CelpList &sel_list2,
	size_t frame1, size_t frame2)
{
	size_t frame_num = m_map->m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	CelpList &cell_list1 = m_map->m_celp_list.at(frame1);
	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	CelpListIter sel_iter, cell_iter;
	Verp vertex1, vertex2;
	Celp celp;
	Vrtx v1, v2;
	std::pair<AdjIter, AdjIter> adj_verts;
	AdjIter inter_iter;
	CellBinIter pwcell_iter;
	std::pair<Edge, bool> inter_edge;

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
				celp = pwcell_iter->lock();
				if (!celp)
					continue;
				sel_list2.insert(std::pair<unsigned int, Celp>
					(celp->Id(), celp));
			}
		}
	}

	return true;
}

//modifications
bool TrackMapProcessor::LinkCells(bool exclusive)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	size_t frame1 = view->m_tseq_cur_num;
	size_t frame2 = view->m_tseq_prv_num;

	//check validity
	if ((frame2 != frame1 + 1 &&
		frame2 != frame1 - 1) ||
		!m_map->ExtendFrameNum(
		std::max(frame1, frame2)))
		return false;

	VertexList vlist1, vlist2;
	CelpListIter citer1, citer2;

	CelpList &cell_list1 = m_map->m_celp_list.at(frame1);
	CelpList &cell_list2 = m_map->m_celp_list.at(frame2);
	CelpListIter cell;
	unsigned int cell_id;

	for (citer1 = m_list_in.begin();
	citer1 != m_list_in.end(); ++citer1)
	{
		cell_id = citer1->second->Id();
		cell = cell_list1.find(cell_id);
		if (cell == cell_list1.end())
			AddCell(citer1->second, frame1, cell);
		else
			cell->second->Set(citer1->second);
		Verp vert1 = cell->second->GetVertex().lock();
		if (vert1)
		{
			vert1->Update();
			vlist1.insert(std::pair<unsigned int, Verp>
				(vert1->Id(), vert1));
		}
	}
	for (citer2 = m_list_out.begin();
	citer2 != m_list_out.end(); ++citer2)
	{
		cell_id = citer2->second->Id();
		cell = cell_list2.find(cell_id);
		if (cell == cell_list2.end())
			AddCell(citer2->second, frame2, cell);
		else
			cell->second->Set(citer2->second);
		Verp vert2 = cell->second->GetVertex().lock();
		if (vert2)
		{
			vert2->Update();
			vlist2.insert(std::pair<unsigned int, Verp>
				(vert2->Id(), vert2));
		}
	}

	if (vlist1.size() == 0 ||
		vlist2.size() == 0)
		return false;

	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
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

bool TrackMapProcessor::LinkAllCells()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return false;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;

	SetTrackMap(trkg->GetTrackMap());
	cache_queue->SetHandleFlags(
		flvr::CQCallback::HDL_DATA |
		flvr::CQCallback::HDL_LABEL |
		flvr::CQCallback::SAV_LABEL |
		flvr::CQCallback::TIME_COND0);
	cache_queue->set_max_size(3);
	flrd::CelpList in = glbin_clusterizer.GetInCells();
	flrd::CelpList out = glbin_clusterizer.GetOutCells();
	RelinkCells(in, out, view->m_tseq_cur_num);
	view->GetTraces(false);

	return true;
}

bool TrackMapProcessor::LinkCells(Celp &celp1, Celp &celp2,
	size_t frame1, size_t frame2, bool exclusive)
{
	//check validity
	if ((frame2 != frame1 + 1 &&
		frame2 != frame1 - 1) ||
		!m_map->ExtendFrameNum(
			std::max(frame1, frame2)))
		return false;

	VertexList vlist1, vlist2;

	CelpList &cell_list1 = m_map->m_celp_list.at(frame1);
	CelpList &cell_list2 = m_map->m_celp_list.at(frame2);
	CelpListIter cell;
	unsigned int cell_id;

	cell_id = celp1->Id();
	cell = cell_list1.find(cell_id);
	if (cell == cell_list1.end())
		AddCell(celp1, frame1, cell);
	Verp vert1 = cell->second->GetVertex().lock();

	cell_id = celp2->Id();
	cell = cell_list2.find(cell_id);
	if (cell == cell_list2.end())
		AddCell(celp2, frame2, cell);
	Verp vert2 = cell->second->GetVertex().lock();

	if (!vert1 || !vert2)
		return false;

	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
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

bool TrackMapProcessor::IsolateCells()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	size_t frame = view->m_tseq_cur_num;

	//check validity
	size_t frame_num = m_map->m_frame_num;
	if (frame >= frame_num)
		return false;

	VertexList vlist;
	CelpListIter citer;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	CelpListIter cell;

	for (citer = m_list_in.begin();
	citer != m_list_in.end(); ++citer)
	{
		cell = cell_list.find(citer->second->Id());
		if (cell == cell_list.end())
			continue;
		Verp vert = cell->second->GetVertex().lock();
		if (vert)
			vlist.insert(std::pair<unsigned int, Verp>
				(vert->Id(), vert));
	}

	if (vlist.size() == 0)
		return false;

	if (frame > 0)
	{
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame - 1);
		VertexListIter viter;
		for (viter = vlist.begin();
		viter != vlist.end(); ++viter)
			IsolateVertex(inter_graph, viter->second);
	}
	if (frame < frame_num - 1)
	{
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame);
		VertexListIter viter;
		for (viter = vlist.begin();
		viter != vlist.end(); ++viter)
			IsolateVertex(inter_graph, viter->second);
	}

	view->GetTraces(false);

	return true;
}

bool TrackMapProcessor::UnlinkCells()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	size_t frame1 = view->m_tseq_cur_num;
	size_t frame2 = view->m_tseq_prv_num;

	//check validity
	size_t frame_num = m_map->m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		(frame2 != frame1 + 1 &&
			frame2 != frame1 - 1))
		return false;

	VertexList vlist1, vlist2;
	CelpListIter citer1, citer2;

	CelpList &cell_list1 = m_map->m_celp_list.at(frame1);
	CelpList &cell_list2 = m_map->m_celp_list.at(frame2);
	CelpListIter cell;

	for (citer1 = m_list_in.begin();
	citer1 != m_list_in.end(); ++citer1)
	{
		cell = cell_list1.find(citer1->second->Id());
		if (cell == cell_list1.end())
			continue;
		Verp vert1 = cell->second->GetVertex().lock();
		if (vert1)
			vlist1.insert(std::pair<unsigned int, Verp>
				(vert1->Id(), vert1));
	}
	for (citer2 = m_list_out.begin();
	citer2 != m_list_out.end(); ++citer2)
	{
		cell = cell_list2.find(citer2->second->Id());
		if (cell == cell_list2.end())
			continue;
		Verp vert2 = cell->second->GetVertex().lock();
		if (vert2)
			vlist2.insert(std::pair<unsigned int, Verp>
				(vert2->Id(), vert2));
	}

	if (vlist1.size() == 0 ||
		vlist2.size() == 0)
		return false;

	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
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

bool TrackMapProcessor::AddCellDup(Celp & celp, size_t frame)
{
	Celp new_cell = Celp(new Cell(celp->Id()));
	new_cell->Set(celp);
	CelpListIter iter;
	bool result = AddCell(new_cell, frame, iter);
	//link new cell
	CelpList list;
	list.insert(std::pair<unsigned int, Celp>
		(celp->Id(), celp));
	LinkAddedCells(list, frame, frame - 1);
	LinkAddedCells(list, frame, frame + 1);
	return result;
}

bool TrackMapProcessor::AddCell(
	Celp &celp, size_t frame, CelpListIter &iter)
{
	//check validity
	if (!m_map->ExtendFrameNum(frame))
		return false;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	VertexList &vert_list = m_map->m_vertices_list.at(frame);

	if (cell_list.find(celp->Id()) != cell_list.end() ||
		vert_list.find(celp->Id()) != vert_list.end())
		return true;

	Verp vertex(new Vertex(celp->Id()));
	vertex->SetCenter(celp->GetCenter());
	vertex->SetSizeUi(celp->GetSizeUi());
	vertex->SetSizeD(celp->GetSizeD());
	vertex->AddCell(celp);
	celp->AddVertex(vertex);
	vert_list.insert(std::pair<unsigned int, Verp>
		(vertex->Id(), vertex));
	std::pair<CelpListIter, bool> result = cell_list.insert(
		std::pair<unsigned int, Celp>(celp->Id(), celp));
	iter = result.first;
	return true;
}

bool TrackMapProcessor::AddCells(CelpList &list, size_t frame)
{
	//check validity
	if (!m_map->ExtendFrameNum(frame))
		return false;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	VertexList &vert_list = m_map->m_vertices_list.at(frame);

	for (auto iter = list.begin();
		iter != list.end(); ++iter)
	{
		Celp celp = iter->second;
		if (cell_list.find(celp->Id()) != cell_list.end() ||
			vert_list.find(celp->Id()) != vert_list.end())
			continue;

		Verp vertex(new Vertex(celp->Id()));
		vertex->SetCenter(celp->GetCenter());
		vertex->SetSizeUi(celp->GetSizeUi());
		vertex->SetSizeD(celp->GetSizeD());
		vertex->AddCell(celp);
		celp->AddVertex(vertex);
		vert_list.insert(std::pair<unsigned int, Verp>
			(vertex->Id(), vertex));
		cell_list.insert(std::pair<unsigned int, Celp>
			(celp->Id(), celp));
	}

	return true;
}

bool TrackMapProcessor::RemoveCells(CelpList &list, size_t frame)
{
	//check validity
	if (!m_map->ExtendFrameNum(frame))
		return false;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	VertexList &vert_list = m_map->m_vertices_list.at(frame);

	for (auto iter = list.begin();
		iter != list.end(); ++iter)
	{
		Celp celp = iter->second;
		auto iter_old = cell_list.find(celp->Id());
		if (iter_old == cell_list.end())
			continue;

		Celp old_celp = iter_old->second;
		cell_list.erase(iter_old);
		//vertex
		Verp vertex = old_celp->GetVertex().lock();
		if (vertex)
		{
			vertex->RemoveCell(old_celp);
			VertexListIter iter_vt = vert_list.find(vertex->Id());
			if (iter_vt != vert_list.end())
				vert_list.erase(iter_vt);
		}
	}

	return true;
}

bool TrackMapProcessor::LinkAddedCells(CelpList &list, size_t f1, size_t f2)
{
	size_t frame_num = m_map->m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num || f1 == f2)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	cache_queue->SetHandleFlags(
		flvr::CQCallback::HDL_DATA |
		flvr::CQCallback::HDL_LABEL |
		flvr::CQCallback::SAV_LABEL |
		flvr::CQCallback::TIME_COND0);
	cache_queue->set_max_size(3);

	//get data and label
	flvr::VolCache4D* cache = cache_queue->get(f1);
	if (!cache)
		return false;
	void* data1 = cache->GetRawData();
	void* label1 = cache->GetRawLabel();
	if (!data1 || !label1)
		return false;
	cache = cache_queue->get(f2);
	void* data2 = cache->GetRawData();
	void* label2 = cache->GetRawLabel();
	if (!data2 || !label2)
		return false;
	cache_queue->protect(f1);

	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
		f1 > f2 ? f2 : f1);
	Verp v1, v2;
	Celp cl1, cl2;

	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	//size_t nz = m_map->m_size_z;
	size_t minx, miny, minz;
	size_t maxx, maxy, maxz;
	float data_value1, data_value2;
	unsigned int label_value1, label_value2;

	for (auto cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		Celp celp = cliter->second;
		unsigned int cid = celp->Id();

		minx = size_t(celp->GetBox().Min().x() + 0.5);
		miny = size_t(celp->GetBox().Min().y() + 0.5);
		minz = size_t(celp->GetBox().Min().z() + 0.5);
		maxx = size_t(celp->GetBox().Max().x() + 0.5);
		maxy = size_t(celp->GetBox().Max().y() + 0.5);
		maxz = size_t(celp->GetBox().Max().z() + 0.5);
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

			if (m_map->m_data_bits == 8)
			{
				data_value1 = ((unsigned char*)data1)[index] / 255.0f;
				data_value2 = ((unsigned char*)data2)[index] / 255.0f;
			}
			else if (m_map->m_data_bits == 16)
			{
				data_value1 = ((unsigned short*)data1)[index] * m_map->m_scale / 65535.0f;
				data_value2 = ((unsigned short*)data2)[index] * m_map->m_scale / 65535.0f;
			}

			
			cl1 = GetCell(f1, label_value1);
			cl2 = GetCell(f2, label_value2);
			v1 = GetVertex(cl1);
			v2 = GetVertex(cl2);
			if (!v1 || !v2)
				continue;

			if (v1->GetSizeUi() < m_size_thresh ||
				v2->GetSizeUi() < m_size_thresh)
				continue;

			LinkVertices(inter_graph,
				v1, v2, f1, f2,
				std::min(data_value1, data_value2));
		}
	}

	cache_queue->unprotect(f1);

	//reset counter
	inter_graph.counter = 0;

	return true;
}

bool TrackMapProcessor::CombineCells(
	Celp &celp, CelpList &list, size_t frame)
{
	//check validity
	if (!m_map->ExtendFrameNum(frame))
		return false;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	VertexList &vert_list = m_map->m_vertices_list.at(frame);

	//find the largest cell
	auto cell_iter = cell_list.find(celp->Id());
	if (cell_iter == cell_list.end())
		return false;
	Celp celp0 = cell_iter->second;
	Verp vertex0 = celp0->GetVertex().lock();

	//add each cell to cell0
	for (cell_iter = list.begin();
		cell_iter != list.end(); ++cell_iter)
	{
		auto iter = cell_list.find(cell_iter->second->Id());
		if (iter == cell_list.end() ||
			iter->second->Id() == celp0->Id())
			continue;
		Celp celp1 = iter->second;
		Verp vertex1 = celp1->GetVertex().lock();
		celp0->Inc(celp1);
		//relink vertex
		if (vertex0 && vertex1)
		{
			//remove cell1 from vertex1
			vertex1->RemoveCell(celp1);
			if (vertex1->GetCellNum() == 0)
			{
				//relink inter graph
				if (frame > 0)
				{
					InterGraph &graph = m_map->m_inter_graph_list.at(frame - 1);
					RelinkInterGraph(vertex1, vertex0, frame, graph, true);
				}
				if (frame < m_map->m_frame_num - 1)
				{
					InterGraph &graph = m_map->m_inter_graph_list.at(frame);
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

bool TrackMapProcessor::DivideCells()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return false;
	SetTrackMap(trkg->GetTrackMap());
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	size_t frame = view->m_tseq_cur_num;

	//check validity
	if (!m_map->ExtendFrameNum(frame))
		return false;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	VertexList &vert_list = m_map->m_vertices_list.at(frame);

	//temporary vertex list
	VertexList vlist;
	VertexListIter vert_iter;
	for (auto cell_iter = m_list_in.begin();
		cell_iter != m_list_in.end(); ++cell_iter)
	{
		auto iter = cell_list.find(cell_iter->second->Id());
		if (iter == cell_list.end())
			continue;
		Celp celp = iter->second;
		Verp vertex = celp->GetVertex().lock();
		if (celp && vertex)
		{
			vert_iter = vlist.find(vertex->Id());
			if (vert_iter == vlist.end())
			{
				Verp v = Verp(new Vertex(vertex->Id()));
				vlist.insert(std::pair<unsigned int, Verp>(
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
		Verp vertex = vert_iter->second;
		if (vertex->GetCellNum() <= 1)
			continue;

		unsigned int max_size = 0;
		unsigned int max_id = 0;
		for (CellBinIter iter = vertex->GetCellsBegin();
		iter != vertex->GetCellsEnd(); ++iter)
		{
			Celp celp = iter->lock();
			if (!celp)
				continue;
			if (celp->GetSizeUi() > max_size)
			{
				max_size = celp->GetSizeUi();
				max_id = celp->Id();
			}
		}

		for (CellBinIter iter = vertex->GetCellsBegin();
		iter != vertex->GetCellsEnd(); ++iter)
		{
			Celp celp = iter->lock();
			if (!celp)
				continue;
			unsigned int id = celp->Id();
			if (id == max_id)
				continue;
			auto cell_iter = cell_list.find(id);
			if (cell_iter == cell_list.end())
				continue;
			celp = cell_iter->second;
			Verp v = celp->GetVertex().lock();
			if (v)
				v->RemoveCell(celp);
			//new vertex
			VertexListIter viter = vert_list.find(id);
			if (viter == vert_list.end())
			{
				Verp v0 = Verp(new Vertex(id));
				v0->AddCell(celp, true);
				celp->AddVertex(v0);
				vert_list.insert(std::pair<unsigned int, Verp>(
					id, v0));
			}
			else
			{
				Verp v0 = viter->second;
				v0->AddCell(celp, true);
				celp->AddVertex(v0);
			}
		}
	}

	return true;
}

bool TrackMapProcessor::ClusterCellsMerge(CelpList &list, size_t frame)
{
	size_t frame_num = m_map->m_frame_num;
	if (frame >= frame_num)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	//get data and label
	flvr::VolCache4D* cache = cache_queue->get(frame);
	if (!cache)
		return false;
	void* data = cache->GetRawData();
	void* label = cache->GetRawLabel();
	if (!data || !label)
		return false;

	//needs a way to choose processor
	ClusterDbscan cs_processor;
	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	//size_t nz = m_map->m_size_z;
	size_t minx, miny, minz;
	size_t maxx, maxy, maxz;
	unsigned int label_value;
	unsigned int id = 0;
	float data_value;

	//add cluster points
	for (auto cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		Celp celp = cliter->second;
		unsigned int cid = celp->Id();
		if (!id) id = cid;

		minx = size_t(celp->GetBox().Min().x() + 0.5);
		miny = size_t(celp->GetBox().Min().y() + 0.5);
		minz = size_t(celp->GetBox().Min().z() + 0.5);
		maxx = size_t(celp->GetBox().Max().x() + 0.5);
		maxy = size_t(celp->GetBox().Max().y() + 0.5);
		maxz = size_t(celp->GetBox().Max().z() + 0.5);
		for (i = minx; i <= maxx; ++i)
		for (j = miny; j <= maxy; ++j)
		for (k = minz; k <= maxz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value = ((unsigned int*)label)[index];
			if (label_value == cid)
			{
				if (m_map->m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (m_map->m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * m_map->m_scale / 65535.0f;
				EmVec pnt = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k) };
				cs_processor.AddClusterPoint(
					pnt, data_value);
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

bool TrackMapProcessor::ClusterCellsSplit(CelpList &list, size_t frame,
	size_t clnum, CelpList &listout)
{
	size_t frame_num = m_map->m_frame_num;
	if (frame >= frame_num)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	//get data and label
	flvr::VolCache4D* cache = cache_queue->get(frame);
	if (!cache)
		return false;
	void* data = cache->GetRawData();
	void* label = cache->GetRawLabel();
	if (!data || !label)
		return false;

	//needs a way to choose processor
	ClusterKmeans cs_proc_km;
	ClusterExmax cs_proc_em;
	cs_proc_km.SetSpacings(m_map->m_spc_x, m_map->m_spc_y, m_map->m_spc_z);
	cs_proc_em.SetSpacings(m_map->m_spc_x, m_map->m_spc_y, m_map->m_spc_z);

	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	size_t minx, miny, minz;
	size_t maxx, maxy, maxz;
	unsigned int label_value;
	unsigned int id = 0;
	float data_value;

	//add cluster points
	for (auto cliter = list.begin();
		cliter != list.end(); ++cliter)
	{
		Celp celp = cliter->second;
		unsigned int cid = celp->Id();
		if (!id) id = cid;

		minx = size_t(celp->GetBox().Min().x() + 0.5);
		miny = size_t(celp->GetBox().Min().y() + 0.5);
		minz = size_t(celp->GetBox().Min().z() + 0.5);
		maxx = size_t(celp->GetBox().Max().x() + 0.5);
		maxy = size_t(celp->GetBox().Max().y() + 0.5);
		maxz = size_t(celp->GetBox().Max().z() + 0.5);
		for (i = minx; i <= maxx; ++i)
		for (j = miny; j <= maxy; ++j)
		for (k = minz; k <= maxz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value = ((unsigned int*)label)[index];
			if (label_value == cid)
			{
				if (m_map->m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (m_map->m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * m_map->m_scale / 65535.0f;
				EmVec pnt = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k) };
				cs_proc_km.AddClusterPoint(
					pnt, data_value);
			}
		}
	}

	//bool result = false;
	//for (size_t clnumi = clnum; clnumi > 1; --clnumi)
	//{
	//	cs_processor.SetClnum(clnum);
	//	if (cs_processor.Execute())
	//	{
	//		result = true;
	//		break;
	//	}
	//}
	cs_proc_km.SetClnum(static_cast<unsigned int>(clnum));
	cs_proc_km.Execute();
	cs_proc_km.AddIDsToData();
	cs_proc_em.SetClnum(static_cast<unsigned int>(clnum));
	cs_proc_em.SetProbTol(0.9f);
	cs_proc_em.SetData(cs_proc_km.GetData());
	cs_proc_em.SetUseInitCluster(true);
	bool result = cs_proc_em.Execute();

	if (result)
	{
		cs_proc_em.GenerateNewIDs(id, label,
			nx, ny, nz, true);
		listout = cs_proc_em.GetCellList();
		//generate output cell list
/*		Cluster &points = cs_proc_em.GetData();
		//std::vector<unsigned int> &ids = cs_proc_em.GetNewIDs();
		pClusterPoint point;
		unsigned int id2;
		CellListIter citer;
		for (ClusterIter piter = points.begin();
			piter != points.end(); ++piter)
		{
			point = *piter;
			i = size_t(boost::qvm::A0(point->centeri) + 0.5);
			j = size_t(boost::qvm::A1(point->centeri) + 0.5);
			k = size_t(boost::qvm::A2(point->centeri) + 0.5);
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
				listout.insert(std::pair<unsigned int, Celp>
					(id2, Celp(cell)));
			}
		}*/

		//label modified, save before delete
		cache_queue->set_modified(frame);

		return true;
	}
	else
		return false;
}

bool TrackMapProcessor::SegmentCells()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return false;
	SetTrackMap(trkg->GetTrackMap());
	auto view = glbin_current.render_view.lock();
	if (!view)
		return false;
	size_t frame = view->m_tseq_cur_num;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return false;
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	SetBits(vd->GetBits());
	SetScale(vd->GetScalarScale());
	SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	cache_queue->SetHandleFlags(
		flvr::CQCallback::HDL_DATA |
		flvr::CQCallback::HDL_LABEL |
		flvr::CQCallback::SAV_LABEL |
		flvr::CQCallback::TIME_COND0);
	cache_queue->set_max_size(3);

	if (m_cluster_num < 2)
		return false;

	//get label and data from cache
	flvr::VolCache4D* cache = cache_queue->get(frame);
	if (cache)
		return false;
	void* data = cache->GetRawData();
	void* label = cache->GetRawLabel();
	if (!data || !label)
		return false;

	ClusterKmeans cs_proc_km;
	//ClusterExmax cs_proc_em;
	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	unsigned int label_value;
	unsigned int id = 0;
	float data_value = 0;

	//add cluster points
	for (auto cliter = m_list_in.begin();
		cliter != m_list_in.end(); ++cliter)
	{
		Celp celp = cliter->second;
		unsigned int cid = celp->Id();
		if (!id) id = cid;

		for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
		for (k = 0; k < nz; ++k)
		{
			index = nx*ny*k + nx*j + i;
			label_value = ((unsigned int*)label)[index];
			if (label_value == cid)
			{
				if (m_map->m_data_bits == 8)
					data_value = ((unsigned char*)data)[index] / 255.0f;
				else if (m_map->m_data_bits == 16)
					data_value = ((unsigned short*)data)[index] * m_map->m_scale / 65535.0f;
				EmVec pnt = { static_cast<double>(i), static_cast<double>(j), static_cast<double>(k) };
				cs_proc_km.AddClusterPoint(
					pnt, data_value);
			}
		}
	}

	//int clnum = 0;
	//GetCellUncertainty(list, frame);
	//for (flrd::CellListIter iter = list.begin();
	//	iter != list.end(); ++iter)
	//{
	//	int size1 = iter->second->GetSizeUi();
	//	int size2 = iter->second->GetExternalUi();
	//	clnum += std::max(size1, size2);
	//}

	//run clustering
	cs_proc_km.SetClnum(m_cluster_num);
	cs_proc_km.Execute();
	cs_proc_km.AddIDsToData();
	//cs_proc_em.SetData(cs_proc_km.GetData());
	cs_proc_km.GenerateNewIDs(id, label, nx, ny, nz, true);
	//label modified, save before delete
	cache_queue->set_modified(frame);

	CelpList out_cells = cs_proc_km.GetCellList();
	//modify map
	RemoveCells(m_list_in, frame);
	AddCells(out_cells, frame);
	LinkAddedCells(out_cells, frame, frame - 1);
	LinkAddedCells(out_cells, frame, frame + 1);

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();

	RefineMap(static_cast<int>(frame));

	return true;
}

void TrackMapProcessor::RelinkCells(CelpList &in, CelpList& out, size_t frame)
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return;
	flvr::VolCache4D* cache = cache_queue->get(frame);
	bool result = false;
	result |= RemoveCells(in, frame);
	result |= AddCells(out, frame);
	result |= LinkAddedCells(out, frame, frame - 1);
	result |= LinkAddedCells(out, frame, frame + 1);
	if (result)
		cache_queue->set_modified(frame);
}

bool TrackMapProcessor::ReplaceCellID(
	unsigned int old_id, unsigned int new_id, size_t frame)
{
	if (frame >= m_map->m_frame_num)
		return false;

	CelpList &cell_list = m_map->m_celp_list.at(frame);
	auto iter = cell_list.find(old_id);
	if (iter == cell_list.end())
		return false;

	Celp old_celp = iter->second;
	Celp new_celp = Celp(new Cell(new_id));
	new_celp->SetCenter(old_celp->GetCenter());
	new_celp->SetSizeUi(old_celp->GetSizeUi());
	new_celp->SetSizeD(old_celp->GetSizeD());
	new_celp->SetExtUi(old_celp->GetExtUi());
	new_celp->SetExtD(old_celp->GetExtD());
	new_celp->SetCelVrtx(old_celp->GetCelVrtx());
	cell_list.erase(iter);
	cell_list.insert(std::pair<unsigned int, Celp>
		(new_id, new_celp));

	//vertex
	Verp vertex = old_celp->GetVertex().lock();
	if (vertex)
	{
		vertex->RemoveCell(old_celp);
		vertex->AddCell(new_celp);
		new_celp->AddVertex(vertex);
	}

	//intra graph
	CellGraph &graph = m_map->m_intra_graph_list.at(frame);
	CelVrtx intra_vert = new_celp->GetCelVrtx();
	if (intra_vert != CellGraph::null_vertex())
	{
		graph[intra_vert].cell = new_celp;
		graph[intra_vert].id = new_id;
	}

	return true;
}

//track list
void TrackMapProcessor::SetListIn(CelpList& list)
{
	m_list_in = list;
}

void TrackMapProcessor::SetListOut(CelpList& list)
{
	m_list_out = list;
}

CelpList& TrackMapProcessor::GetListIn()
{
	return m_list_in;
}

CelpList& TrackMapProcessor::GetListOut()
{
	return m_list_out;
}

void TrackMapProcessor::AnalyzeLink()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	SetTrackMap(trkg->GetTrackMap());
	size_t frames = m_map->GetFrameNum();
	if (frames == 0)
		WriteInfo(L"ERROR! Generate a track map first.\n");
	else
		WriteInfo(L"Time point number: " + std::to_wstring(frames) + L"\n");

	WriteInfo(L"Time\tIn Orphan\tOut Orphan\tIn Multi\tOut Multi\n");
	flrd::VertexList in_orphan_list;
	flrd::VertexList out_orphan_list;
	flrd::VertexList in_multi_list;
	flrd::VertexList out_multi_list;
	for (size_t fi = 0; fi < frames; ++fi)
	{
		GetLinkLists(fi,
			in_orphan_list, out_orphan_list,
			in_multi_list, out_multi_list);
		WriteInfo(std::to_wstring(fi) + L"\t" +
			std::to_wstring(in_orphan_list.size()) + L"\t" +
			std::to_wstring(out_orphan_list.size()) + L"\t" +
			std::to_wstring(in_multi_list.size()) + L"\t" +
			std::to_wstring(out_multi_list.size()) + L"\n");
	}
}

void TrackMapProcessor::GetLinkLists(
	size_t frame,
	flrd::VertexList &in_orphan_list,
	flrd::VertexList &out_orphan_list,
	flrd::VertexList &in_multi_list,
	flrd::VertexList &out_multi_list)
{
	if (frame >= m_map->m_frame_num)
		return;

	VertexList &vertex_list = m_map->m_vertices_list.at(frame);

	Vrtx v0, v1;
	std::pair<AdjIter, AdjIter> adj_verts;
	std::pair<Edge, bool> edge;
	int edge_count;

	//in lists
	if (frame > 0)
	{
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame - 1);
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
				in_orphan_list.insert(std::pair<unsigned int, Verp>(
					iter->second->Id(), iter->second));
				continue;
			}
			adj_verts = boost::adjacent_vertices(v0, inter_graph);
			edge_count = 0;
			//for each adjacent vertex
			for (AdjIter inter_iter = adj_verts.first;
			inter_iter != adj_verts.second; ++inter_iter)
			{
				v1 = *inter_iter;
				edge = boost::edge(v0, v1, inter_graph);
				if (edge.second && inter_graph[edge.first].link)
					edge_count++;
			}
			if (edge_count == 0)
				in_orphan_list.insert(std::pair<unsigned int, Verp>(
					iter->second->Id(), iter->second));
			else if (edge_count > 1)
				in_multi_list.insert(std::pair<unsigned int, Verp>(
					iter->second->Id(), iter->second));
		}
	}

	//out lists
	if (frame < m_map->m_frame_num - 1)
	{
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame);
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
				out_orphan_list.insert(std::pair<unsigned int, Verp>(
					iter->second->Id(), iter->second));
				continue;
			}
			adj_verts = boost::adjacent_vertices(v0, inter_graph);
			edge_count = 0;
			//for each adjacent vertex
			for (AdjIter inter_iter = adj_verts.first;
			inter_iter != adj_verts.second; ++inter_iter)
			{
				v1 = *inter_iter;
				edge = boost::edge(v0, v1, inter_graph);
				if (edge.second && inter_graph[edge.first].link)
					edge_count++;
			}
			if (edge_count == 0)
				out_orphan_list.insert(std::pair<unsigned int, Verp>(
					iter->second->Id(), iter->second));
			else if (edge_count > 1)
				out_multi_list.insert(std::pair<unsigned int, Verp>(
					iter->second->Id(), iter->second));
		}
	}
}

void TrackMapProcessor::AnalyzeUncertainty()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	glbin_trackmap_proc.SetTrackMap(trkg->GetTrackMap());

	bool sel = !m_list_in.empty();

	if (sel)
		GetCellUncertainty();
	else
		GetUncertainHist();
}

void TrackMapProcessor::GetCellsByUncertainty(bool filter_in_list)
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	if (!trkg->GetTrackMap()->GetFrameNum())
		return;
	SetTrackMap(trkg->GetTrackMap());
	size_t frame = view->m_tseq_cur_num;
	if (frame >= m_map->m_frame_num)
		return;

	m_list_out.clear();

	unsigned int count;
	Verp vertex;
	Celp celp;
	CellBinIter pwcell_iter;
	CelpListIter cell_iter;
	//VertexList &vertex_list = m_map->m_vertices_list.at(frame);
	if (frame > 0)
	{
		InterGraph &inter_graph = 
			m_map->m_inter_graph_list.at(frame-1);
		for (auto ie : boost::make_iterator_range(edges(inter_graph)))
		{
			count = inter_graph[ie].count;
			if (count >= m_uncertain_low)
			{
				auto v0 = boost::source(ie, inter_graph);
				if (inter_graph[v0].frame == frame)
				{
					vertex = inter_graph[v0].vertex.lock();
					if (vertex)
					{
						for (pwcell_iter = vertex->GetCellsBegin();
							pwcell_iter != vertex->GetCellsEnd();
							++pwcell_iter)
						{
							celp = pwcell_iter->lock();
							if (!celp)
								continue;
							if (filter_in_list)
							{
								cell_iter = m_list_in.find(celp->Id());
								if (cell_iter == m_list_in.end())
									continue;
							}
							m_list_out.insert(std::pair<unsigned int, Celp>
								(celp->Id(), celp));
						}
					}
				}
				v0 = boost::target(ie, inter_graph);
				if (inter_graph[v0].frame == frame)
				{
					vertex = inter_graph[v0].vertex.lock();
					if (vertex)
					{
						for (pwcell_iter = vertex->GetCellsBegin();
							pwcell_iter != vertex->GetCellsEnd();
							++pwcell_iter)
						{
							celp = pwcell_iter->lock();
							if (!celp)
								continue;
							if (filter_in_list)
							{
								cell_iter = m_list_in.find(celp->Id());
								if (cell_iter == m_list_in.end())
									continue;
							}
							m_list_out.insert(std::pair<unsigned int, Celp>
								(celp->Id(), celp));
						}
					}
				}
			}
		}
	}
	if (frame < m_map->m_frame_num - 1)
	{
		InterGraph &inter_graph =
			m_map->m_inter_graph_list.at(frame);
		for (auto ie : boost::make_iterator_range(edges(inter_graph)))
		{
			count = inter_graph[ie].count;
			if (count >= m_uncertain_low)
			{
				auto v0 = boost::source(ie, inter_graph);
				if (inter_graph[v0].frame == frame)
				{
					vertex = inter_graph[v0].vertex.lock();
					if (vertex)
					{
						for (pwcell_iter = vertex->GetCellsBegin();
							pwcell_iter != vertex->GetCellsEnd();
							++pwcell_iter)
						{
							celp = pwcell_iter->lock();
							if (!celp)
								continue;
							if (filter_in_list)
							{
								cell_iter = m_list_in.find(celp->Id());
								if (cell_iter == m_list_in.end())
									continue;
							}
							m_list_out.insert(std::pair<unsigned int, Celp>
								(celp->Id(), celp));
						}
					}
				}
				v0 = boost::target(ie, inter_graph);
				if (inter_graph[v0].frame == frame)
				{
					vertex = inter_graph[v0].vertex.lock();
					if (vertex)
					{
						for (pwcell_iter = vertex->GetCellsBegin();
							pwcell_iter != vertex->GetCellsEnd();
							++pwcell_iter)
						{
							celp = pwcell_iter->lock();
							if (!celp)
								continue;
							if (filter_in_list)
							{
								cell_iter = m_list_in.find(celp->Id());
								if (cell_iter == m_list_in.end())
									continue;
							}
							m_list_out.insert(std::pair<unsigned int, Celp>
								(celp->Id(), celp));
						}
					}
				}
			}
		}
	}

	view->GetTraces(false);
}

void TrackMapProcessor::GetCellUncertainty()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	size_t frame = view->m_tseq_cur_num;
	if (frame >= m_map->m_frame_num)
		return;

	VertexList &vertex_list = m_map->m_vertices_list.at(frame);

	Vrtx v0;
	Verp vertex;
	Celp celp;
	CellBinIter pwcell_iter;
	CelpListIter cell_iter;

	//in lists
	if (frame > 0)
	{
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame - 1);
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
				celp = pwcell_iter->lock();
				if (!celp)
					continue;
				cell_iter = m_list_in.find(celp->Id());
				if (cell_iter != m_list_in.end())
				{
					v0 = vertex->GetInterVert(inter_graph);
					if (v0 == InterGraph::null_vertex())
						continue;
					cell_iter->second->SetCount0(
						inter_graph[v0].count);
				}
			}
		}
	}
	else
	{
		//clear size
		for (cell_iter = m_list_in.begin();
			cell_iter != m_list_in.end(); ++cell_iter)
		{
			cell_iter->second->SetCount0(0);
		}
	}

	//out lists
	if (frame < m_map->m_frame_num - 1)
	{
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame);
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
				celp = pwcell_iter->lock();
				if (!celp)
					continue;
				cell_iter = m_list_in.find(celp->Id());
				if (cell_iter != m_list_in.end())
				{
					v0 = vertex->GetInterVert(inter_graph);
					if (v0 == InterGraph::null_vertex())
						continue;
					cell_iter->second->SetCount1(
						inter_graph[v0].count);
				}
			}
		}
	}
	else
	{
		//clear size
		for (cell_iter = m_list_in.begin();
		cell_iter != m_list_in.end(); ++cell_iter)
		{
			cell_iter->second->SetCount1(0);
		}
	}

	//header
	WriteInfo(L"ID\tIn\tOut\n");
	for (auto iter = m_list_in.begin();
		iter != m_list_in.end(); ++iter)
	{
		std::wstring sid = std::to_wstring(iter->second->Id());
		WriteInfo(sid + L"\t" +
			std::to_wstring(iter->second->GetCount0()) + L"\t" +
			std::to_wstring(iter->second->GetCount1()) + L"\n");
	}
}

void TrackMapProcessor::GetUncertainHist()
{
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;
	size_t frame = view->m_tseq_cur_num;
	if (frame >= m_map->m_frame_num)
		return;

	VertexList &vertex_list = m_map->m_vertices_list.at(frame);

	//in lists
	if (frame > 0)
	{
		//header
		WriteInfo(L"In\n");
		WriteInfo(L"Level\tFrequency\n");
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame - 1);
		UncertainHist hist;
		GetUncertainHist(hist, vertex_list, inter_graph);
	}

	WriteInfo(L"\n");

	//out lists
	if (frame < m_map->m_frame_num - 1)
	{
		//header
		WriteInfo(L"Out\n");
		WriteInfo(L"Level\tFrequency\n");
		InterGraph &inter_graph = m_map->m_inter_graph_list.at(frame);
		UncertainHist hist;
		GetUncertainHist(hist, vertex_list, inter_graph);
	}
}

void TrackMapProcessor::GetUncertainHist(UncertainHist& hist,
	VertexList &vertex_list, InterGraph &inter_graph)
{
	unsigned int count;
	Vrtx v0;
	Verp vertex;

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
		auto uhist_iter = hist.find(count);
		if (uhist_iter == hist.end())
		{
			UncertainBin bin;
			bin.level = count;
			bin.count = 1;
			hist.insert(std::pair<unsigned int, UncertainBin>(
				count, bin));
		}
		else
			uhist_iter->second.count++;
	}

	//fill in zeros
	unsigned int index = 0;
	for (UncertainHistIter iter = hist.begin();
		iter != hist.end(); ++iter)
	{
		if (iter->second.level > index)
		{
			for (unsigned int i = index;
				i < iter->second.level; ++i)
			{
				UncertainBin bin;
				bin.level = i;
				bin.count = 0;
				hist.insert(std::pair<unsigned int,
					UncertainBin>(i, bin));
			}
		}
		index = iter->second.level + 1;
	}

	count = 0;
	for (auto iter = hist.begin();
		iter != hist.end(); ++iter)
	{
		while (iter->second.level > count)
		{
			WriteInfo(std::to_wstring(count++) + L"\t0\n");
		}
		WriteInfo(std::to_wstring(iter->second.level) + L"\t" +
			std::to_wstring(iter->second.count) + L"\n");
		count++;
	}
}

void TrackMapProcessor::AnalyzePath()
{
	auto view = glbin_current.render_view.lock();
	size_t frame = view->m_tseq_cur_num;
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	SetTrackMap(trkg->GetTrackMap());
	if (m_list_in.empty())
		return;

	std::wstringstream ss;

	if (frame > 0)
	{
		WriteInfo(L"Paths of T" + std::to_wstring(frame) + L" to T" + std::to_wstring(frame - 1) + L":\n");
		flrd::PathList paths_prv;
		GetPaths(paths_prv, frame, frame - 1);
		for (size_t i = 0; i < paths_prv.size(); ++i)
			ss << paths_prv[i];
		WriteInfo(ss.str());
	}

	ss.str(L"");
	if (frame < m_map->GetFrameNum() - 1)
	{
		WriteInfo(L"Paths of T" + std::to_wstring(frame) + L" to T" + std::to_wstring(frame + 1) + L":\n");
		flrd::PathList paths_nxt;
		GetPaths(paths_nxt, frame, frame + 1);
		for (size_t i = 0; i < paths_nxt.size(); ++i)
			ss << paths_nxt[i];
		WriteInfo(ss.str());
	}
}

void TrackMapProcessor::GetPaths(PathList &path_list, size_t frame1, size_t frame2)
{
	size_t frame_num = m_map->m_frame_num;
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return;

	CelpList &cell_list1 = m_map->m_celp_list.at(frame1);
	InterGraph &inter_graph = m_map->m_inter_graph_list.at(
		frame1 > frame2 ? frame2 : frame1);
	VertexList vertex_list;
	CelpListIter cell_iter;
	Verp vertex1;

	for (auto sel_iter = m_list_in.begin();
		sel_iter != m_list_in.end();
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
			vertex_list.insert(std::pair<unsigned int, Verp>
			(vertex1->Id(), vertex1));
	}

	m_level_thresh = 2;
	for (auto iter = vertex_list.begin();
		iter != vertex_list.end(); ++iter)
	{
		GetAlterPath(inter_graph, iter->second, path_list);
	}
}

bool TrackMapProcessor::TrackStencils(size_t f1, size_t f2,
	fluo::Vector &extt, fluo::Vector &exta,
	int mode, size_t start, int sim)
{
	//check validity
	if (!m_map->ExtendFrameNum(std::max(f1, f2)))
		return false;

	size_t frame_num = m_map->m_frame_num;
	if (f1 >= frame_num || f2 >= frame_num || f1 == f2)
		return false;
	if (start >= frame_num)
		return false;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return false;
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (!cache_queue)
		return false;
	//get data and label
	size_t f0 = mode == 1?start:f1;
	cache_queue->set_max_size(2);
	flvr::VolCache4D* cache = cache_queue->get(f0);
	if (!cache)
		return false;
	void* data1 = cache->GetRawData();
	void* label1 = cache->GetRawLabel();
	if (!data1 || !label1)
		return false;
	cache = cache_queue->get(f2);
	if (!cache)
		return false;
	void* data2 = cache->GetRawData();
	void* label2 = cache->GetRawLabel();
	if (!data2 || !label2)
		return false;
	cache_queue->protect(f0);

	//precision level
	int plevel = std::max(1, int(-std::log10(m_eps)));

	size_t index;
	size_t i, j, k;
	size_t nx = m_map->m_size_x;
	size_t ny = m_map->m_size_y;
	size_t nz = m_map->m_size_z;
	unsigned int label_value;

	//clear label2
	unsigned long long clear_size = (unsigned long long)nx *
		ny * nz * sizeof(unsigned int);
	std::memset(label2, 0, clear_size);

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
			iter->second.extend(fluo::Point(
				static_cast<double>(i),
				static_cast<double>(j),
				static_cast<double>(k)));
		}
		else
		{
			Stencil stencil;
			stencil.data = data1;
			stencil.label = label1;
			stencil.id = label_value;
			stencil.nx = nx;
			stencil.ny = ny;
			stencil.nz = nz;
			stencil.bits = m_map->m_data_bits;
			stencil.scale = m_map->m_scale;
			stencil.fsize = m_filter;
			stencil.box.extend(fluo::Point(
				static_cast<double>(i),
				static_cast<double>(j),
				static_cast<double>(k)));
			stencil_list.insert(std::pair<unsigned int, Stencil>
				(label_value, stencil));
		}
	}

	//find matching stencil in frame2
	fluo::Point center;
	Stencil s1, s2;
	s2.data = data2;
	s2.label = label2;
	s2.nx = nx;
	s2.ny = ny;
	s2.nz = nz;
	s2.bits = m_map->m_data_bits;
	s2.scale = m_map->m_scale;
	s2.fsize = m_filter;
	for (iter = stencil_list.begin(); iter != stencil_list.end(); ++iter)
	{
		s1 = iter->second;
		//filter out small
		fluo::Point s1size = s1.box.size();
		if (s1size.x() < m_stencil_thresh.x() &&
			s1size.y() < m_stencil_thresh.y() &&
			s1size.z() < m_stencil_thresh.z())
			continue;
		//get offset
		fluo::Vector off;
		if (mode == 1 &&
			f1 != f0)
		{
			//start frame
			Celp temp = GetCell(f0, s1.id);
			if (temp)
				off = fluo::Vector(temp->GetCenter());
			//prev frame
			temp = GetCell(f1, s1.id);
			if (temp)
				off = fluo::Vector(temp->GetCenter()) - off;
		}
		//if (match_stencils(s1, s2, ext, off, center, prob, m_max_iter, m_eps, sim))
		s2.box = s1.box;
		StencilCompare compare(
			&s1, &s2, extt, exta, off, fluo::Vector(),
			m_max_iter, plevel, sim, false);
		if (compare.Compare())
		{
			//label stencil 2
			compare.Lookup();

			//add s1 to track map
			CelpListIter iter;
			Celp celp1(new Cell(s1.id));
			celp1->SetCenter(s1.center());
			celp1->SetBox(s1.box);
			celp1->SetCalc();
			AddCell(celp1, f1, iter);
			//add s2 id to track map
			Celp celp2(new Cell(s2.id));
			celp2->SetCenter(s2.center());
			celp2->SetBox(s2.box);
			celp2->SetCalc();
			AddCell(celp2, f2, iter);
			//connect cells
			LinkCells(celp1, celp2, f1, f2, false);
		}
	}

	cache_queue->unprotect(f0);
	if (mode == 0)
		cache_queue->clear(f0);
	else
		cache_queue->clear(f2);
	return true;
}

//conversion
void TrackMapProcessor::ConvertRulers()
{
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	flrd::RulerList* ruler_list = glbin_current.GetRulerList();
	if (!ruler_list)
		return;

	//get rulers
	flrd::RulerList rulers;
	trkg->GetMappedRulers(rulers);
	for (auto iter = rulers.begin();
		iter != rulers.end(); ++iter)
	{
		(*iter)->Scale(spcx, spcy, spcz);
		ruler_list->push_back(*iter);
	}
	glbin_vertex_array_manager.set_dirty(flvr::VAType::VA_Rulers);
}

void TrackMapProcessor::ConvertConsistent()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	TrackGroup* trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	WriteInfo(L"Generating consistent IDs in");

	flrd::pTrackMap track_map = trkg->GetTrackMap();
	SetTrackMap(track_map);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	SetBits(vd->GetBits());
	SetScale(vd->GetScalarScale());
	SetSizes(resx, resy, resz);
	SetSpacings(spcx, spcy, spcz);
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(vd.get());
	if (cache_queue)
	{
		cache_queue->SetHandleFlags(
			flvr::CQCallback::HDL_DATA |
			flvr::CQCallback::HDL_LABEL |
			flvr::CQCallback::SAV_LABEL |
			flvr::CQCallback::TIME_COND0);
		cache_queue->set_max_size(2);
	}

	WriteInfo(L"Frame 0\n");
	MakeConsistent(0);

	//remaining frames
	for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
	{
		WriteInfo(L"Frame " + std::to_wstring(fi) + L"\n");
		MakeConsistent(fi - 1, fi);
	}
}
