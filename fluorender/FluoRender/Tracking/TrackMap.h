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
#ifndef FL_TrackMap_h
#define FL_TrackMap_h

#include "CellList.h"
#include "VertexList.h"
#include "VolCache.h"
#include <fstream>
#include <boost/signals2.hpp>
#include <deque>
#include <map>

namespace FL
{
//tags
#define TAG_CELL		1
#define TAG_VERT		2
#define TAG_INTRA_EDGE	3
#define TAG_INTER_EDGE	4
#define TAG_FRAM		5
#define TAG_FPCOUNT		6
#define TAG_NUM			7
#define TAG_VER219		8	//added to 2.19 for uncertainty
							//order: v1, v2, edge
#define TAG_VER220		9	//new values added in v2.20

	class TrackMap;
	class TrackMapProcessor
	{
	public:
		TrackMapProcessor(TrackMap &track_map) :
		m_contact_thresh(0.6f),
		m_size_thresh(25.0f),
		m_similar_thresh(0.2f),
		m_level_thresh(2),
		m_merge(false),
		m_split(false),
		m_map(track_map) {}
		~TrackMapProcessor();

		void SetContactThresh(float value);
		void SetSizeThresh(float value);
		void SetSimilarThresh(float value);
		void SetLevelThresh(int level);
		void SetUncertainLow(unsigned int value);
		void SetUncertainHigh(unsigned int value);
		void SetMerge(bool value);
		void SetSplit(bool value);

		void SetSizes(size_t nx, size_t ny, size_t nz);
		void SetBits(size_t bits);
		void SetScale(float scale);
		void SetSpacings(float spcx, float spcy, float spcz);

		void SetVolCacheSize(size_t size);

		//build cell list and intra graph
		bool InitializeFrame(size_t frame);
		//build inter graph
		bool LinkFrames(size_t f1, size_t f2);
		//group cells
		bool ResolveGraph(size_t frame1, size_t frame2);
		//find the maximum overlapping and set link flags on inter graph
		bool ProcessFrames(size_t frame1, size_t frame2);

		bool Export(std::string &filename);
		bool Import(std::string &filename);

		bool ResetVertexIDs();

		//get mapped cell
		bool GetMappedID(unsigned int id_in, unsigned int& id_out,
			size_t frame);
		bool GetMappedID(unsigned int id_in, unsigned int& id_out,
			size_t frame1, size_t frame2);
		//get mapped cells
		bool GetMappedCells(CellList &sel_list1, CellList &sel_list2,
			size_t frame1, size_t frame2);

		//modifications
		bool LinkCells(CellList &list1, CellList &list2,
			size_t frame1, size_t frame2, bool exclusive);
		bool LinkCells(pCell &cell1, pCell &cell2,
			size_t frame1, size_t frame2, bool exclusive);
		bool IsolateCells(CellList &list, size_t frame);
		bool UnlinkCells(CellList &list1, CellList &list2,
			size_t frame1, size_t frame2);
		//
		bool AddCell(pCell &cell, size_t frame, CellListIter &iter);
		bool AddCells(CellList &list, size_t frame);
		bool LinkAddedCells(CellList &list, size_t frame1, size_t frame2);
		bool CombineCells(pCell &cell, CellList &list, size_t frame);
		bool DivideCells(CellList &list, size_t frame);
		bool SegmentCells(void* data, void* label, CellList &list, size_t frame);
		bool ReplaceCellID(unsigned int old_id,
			unsigned int new_id, size_t frame);

		//information
		void GetLinkLists(size_t frame,
			FL::VertexList &in_orphan_list,
			FL::VertexList &out_orphan_list,
			FL::VertexList &in_multi_list,
			FL::VertexList &out_multi_list);
		void GetCellsByUncertainty(CellList &list_in, CellList &list_out,
			size_t frame);
		void GetCellUncertainty(CellList &list, size_t frame);
		void GetUncertainHist(UncertainHist &hist1, UncertainHist &hist2, size_t frame);
		void GetUncertainHist(UncertainHist &hist, VertexList &vertex_list, InterGraph &graph);
		void GetPaths(CellList &cell_list, PathList &path_list, size_t frame1, size_t frame2);

		//tracking by matching user input
		bool TrackStencils(size_t frame1, size_t frame2);

		//connect and disconnect functions for cache queue
		typedef boost::function<void (VolCache&)> func_cache;
		void RegisterCacheQueueFuncs(const func_cache &fnew, const func_cache &fdel);
		void UnregisterCacheQueueFuncs();

	private:
		float m_contact_thresh;
		float m_size_thresh;
		float m_similar_thresh;
		int m_level_thresh;
		bool m_merge;
		bool m_split;
		//uncertainty filter
		unsigned int m_uncertain_low;
		unsigned int m_uncertain_high;
		//the trackmap
		TrackMap &m_map;
		//volume data cache
		CacheQueue m_vol_cache;
		boost::signals2::connection m_new_conn;
		boost::signals2::connection m_del_conn;
		//cur & neighbor frames for orphan searching
		size_t m_frame1;
		size_t m_frame2;
		bool m_major_converge;//majority of the links have converged

	private:
		//modification
		bool CheckCellContact(pCell &cell, void *data, void *label,
			size_t ci, size_t cj, size_t ck);
		bool AddContact(IntraGraph& graph,
			pCell &cell1, pCell &cell2,
			float contact_value);
		bool CheckCellDist(pCell &cell, void *label,
			size_t ci, size_t cj, size_t ck);
		bool AddNeighbor(IntraGraph& graph,
			pCell &cell1, pCell &cell2,
			float dist_v, float dist_s);
		bool LinkVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2,
			float overlap_value);
		bool IsolateVertex(InterGraph& graph,
			pVertex &vertex);
		bool ForceVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2);
		bool UnlinkVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2);
		bool MergeCells(VertexList& vertex_list, CellBin &bin, size_t frame);
		bool RelinkInterGraph(pVertex &vertex, pVertex &vertex0, size_t frame, InterGraph &graph, bool reset);
		bool RemoveVertex(InterGraph& graph, pVertex &vertex);
		
		//determine if cells on intragraph can be merged
		typedef bool(TrackMapProcessor::*f_merge_cell)(IntraEdge&, pCell&, pCell&, IntraGraph&);
		bool GroupCells(std::vector<pwCell> &cells, std::vector<CellBin> &cell_bins,
			IntraGraph &intra_graph, f_merge_cell merge_cell);
		bool EqualCells(pwCell &cell1, pwCell &cell2);
		bool FindCellBin(CellBin &bin, pwCell &cell);
		bool AddCellBin(std::vector<CellBin> &bins,
			pwCell &cell);
		bool AddCellBin(std::vector<CellBin> &bins,
			pwCell &cell1, pwCell &cell2);
		bool GreaterThanCellBin(pCell &cell1, CellBin &bin, pwCell &cell2);
		size_t GetBinsCellCount(std::vector<CellBin> &bins);

		//replaces all previous match/unmatch funcs
		bool ProcessVertex(pVertex &vertex, InterGraph &graph,
			unsigned int seg_count_min);
		//vertex matching routines
		//find out current valence of a vertex
		bool GetValence(pVertex &vertex, InterGraph &graph,
			size_t &valence);
		//edges include linked and unlinked
		bool GetValence(pVertex &vertex, InterGraph &graph,
			size_t &valence, std::vector<InterEdge> &edges);
		//edges include linked only
		bool GetValence(pVertex &vertex, InterGraph &graph,
			size_t &valence, std::vector<InterEdge> &all_edges,
			std::vector<InterEdge> &linked_edges);
		//get uncertain edges for segmentation
		bool GetUncertainEdges(pVertex &vertex, InterGraph &graph,
			unsigned int min_count,
			std::vector<InterEdge> &uncertain_edges);
		//detailed match functions
		//link edge of the max overlap
		bool LinkEdgeSize(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges, bool calc_sim);
		//search for neighboring orphans for linking
		bool LinkOrphans(InterGraph& graph, pVertex &vertex);
		//unlink edge by size similarity
		bool UnlinkEdgeSize(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges, bool calc_sim);
		//unlink edge by extended alternating path
		bool UnlinkAlterPath(InterGraph &graph, pVertex &vertex,
			bool calc_sim);
		//fix multi-link by segmentation
		bool UnlinkSegment(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &linked_edges, bool calc_sim,
			bool segment, unsigned int seg_count_min);
		bool GetAlterPath(InterGraph &graph, pVertex &vertex,
			PathList &paths);
		bool UnlinkAlterPathMaxMatch(InterGraph &graph, pVertex &vertex,
			PathList &paths, bool calc_sim);
		bool UnlinkAlterPathSize(InterGraph &graph, pVertex &vertex,
			PathList &paths);
		bool UnlinkAlterPathConn(InterGraph &graph, pVertex &vertex,
			PathList &paths);
		bool UnlinkAlterPathCount(InterGraph &graph, pVertex &vertex,
			PathList &paths);
		//check if any out vertex can be combined
		bool MergeEdges(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		//unlink edge by vertex size, use after merge fails
		bool UnlinkVertexSize(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		//check if the vertex can split
		bool SplitVertex(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		bool ClusterCellsMerge(CellList &list, size_t frame);
		bool ClusterCellsSplit(CellList &list, size_t frame,
			size_t clnum, CellList &listout);

		//helper functions
		bool get_alter_path(InterGraph &graph, pVertex &vertex,
			Path &alt_path, VertVisitList &visited, int curl);
		float get_path_max(InterGraph &graph, PathList &paths,
			size_t curl, InterVert v0);
		bool unlink_alt_path(InterGraph &graph, PathList &paths);
		bool merge_cell_size(IntraEdge &edge, pCell &cell1, pCell &cell2, IntraGraph& graph);
		static bool comp_edge_size(InterEdge &edge1, InterEdge &edge2, InterGraph& graph);
		bool similar_edge_size(InterEdge &edge1, InterEdge &edge2, InterGraph& graph);
		static bool comp_path_size(Path &path1, Path &path2);
		static bool comp_path_mm(Path &path1, Path &path2);
		bool similar_path_size(Path &path1, Path &path2);
		bool similar_path_mm(Path &path1, Path &path2);
		static bool comp_path_count(Path &path1, Path &path2);
		static bool comp_path_count_rev(Path &path1, Path &path2);
		bool similar_path_count(Path &path1, Path &path2);
		bool similar_vertex_size(pVertex& v1, pVertex& v2);

		void link_edge(InterEdge edge, InterGraph &graph, unsigned int value = 1);
		void unlink_edge(InterEdge edge, InterGraph &graph, unsigned int value = 0);

		//random number
		bool get_random(size_t count, InterGraph &graph);
		//get if segmentation is computed
		bool get_segment(VertexList &vertex_list, InterGraph &inter_graph, unsigned int &count_thresh);
		bool get_major_converge(InterGraph &inter_graph, size_t vertex_frame, UncertainBin &major_bin);
		bool similar_count(unsigned int count1, unsigned int count2);

		//export
		void WriteBool(std::ofstream& ofs, const bool value);
		void WriteTag(std::ofstream& ofs, const unsigned char tag);
		void WriteUint(std::ofstream& ofs, const unsigned int value);
		void WriteFloat(std::ofstream& ofs, const float value);
		void WritePoint(std::ofstream& ofs, const FLIVR::Point &point);
		void WriteCell(std::ofstream& ofs, const pCell &cell);
		void WriteVertex(std::ofstream& ofs, const pVertex &vertex);
		//import
		bool ReadBool(std::ifstream& ifs);
		unsigned char ReadTag(std::ifstream& ifs);
		unsigned int ReadUint(std::ifstream& ifs);
		float ReadFloat(std::ifstream& ifs);
		FLIVR::Point ReadPoint(std::ifstream& ifs);
		pCell ReadCell(std::ifstream& ifs, CellList& cell_list);
		void ReadVertex(std::ifstream& ifs, VertexList& vertex_list, CellList& cell_list);
		bool AddIntraEdge(IntraGraph& graph,
			pCell &cell1, pCell &cell2,
			unsigned int size_ui, float size_f,
			float dist_v, float dist_s);
		bool AddInterEdge(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2,
			unsigned int size_ui, float size_f,
			float dist, unsigned int link,
			unsigned int v1_count, unsigned int v2_count,
			unsigned int edge_count);

	};

	inline TrackMapProcessor::~TrackMapProcessor()
	{
		//delete cache queue
		//order is important
		m_vol_cache.CacheQueue::~CacheQueue();
		UnregisterCacheQueueFuncs();
	}

	inline void TrackMapProcessor::RegisterCacheQueueFuncs(
		const func_cache &fnew, const func_cache &fdel)
	{
		m_new_conn = m_vol_cache.m_new_cache.connect(fnew);
		m_del_conn = m_vol_cache.m_del_cache.connect(fdel);
	}

	inline void TrackMapProcessor::UnregisterCacheQueueFuncs()
	{
		m_new_conn.disconnect();
		m_del_conn.disconnect();
	}

	inline void TrackMapProcessor::SetContactThresh(float value)
	{
		m_contact_thresh = value;
	}

	inline void TrackMapProcessor::SetSizeThresh(float value)
	{
		m_size_thresh = value;
	}

	inline void TrackMapProcessor::SetSimilarThresh(float value)
	{
		m_similar_thresh = value;
	}

	inline void TrackMapProcessor::SetLevelThresh(int level)
	{
		m_level_thresh = level;
	}

	inline void TrackMapProcessor::SetUncertainLow(unsigned int value)
	{
		m_uncertain_low = value;
	}

	inline void TrackMapProcessor::SetUncertainHigh(unsigned int value)
	{
		m_uncertain_high = value;
	}

	inline void TrackMapProcessor::SetMerge(bool value)
	{
		m_merge = value;
	}

	inline void TrackMapProcessor::SetSplit(bool value)
	{
		m_split = value;
	}

	inline void TrackMapProcessor::WriteBool(std::ofstream& ofs, const bool value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(bool));
	}

	inline void TrackMapProcessor::WriteTag(std::ofstream& ofs, const unsigned char tag)
	{
		ofs.write(reinterpret_cast<const char*>(&tag), sizeof(unsigned char));
	}

	inline void TrackMapProcessor::WriteUint(std::ofstream& ofs, const unsigned int value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(unsigned int));
	}

	inline void TrackMapProcessor::WriteFloat(std::ofstream& ofs, const float value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(float));
	}

	inline void TrackMapProcessor::WritePoint(std::ofstream& ofs, const FLIVR::Point &point)
	{
		double x = point.x();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
		x = point.y();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
		x = point.z();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
	}

	inline void TrackMapProcessor::WriteCell(std::ofstream& ofs, const pCell &cell)
	{
		WriteTag(ofs, TAG_CELL);
		WriteUint(ofs, cell->Id());
		WriteUint(ofs, cell->GetSizeUi());
		WriteFloat(ofs, cell->GetSizeF());
		WriteUint(ofs, cell->GetExternalUi());
		WriteFloat(ofs, cell->GetExternalF());
		WritePoint(ofs, cell->GetCenter());
		WriteTag(ofs, TAG_VER220);
		WritePoint(ofs, cell->GetBox().min());
		WritePoint(ofs, cell->GetBox().max());
	}

	inline bool TrackMapProcessor::ReadBool(std::ifstream& ifs)
	{
		bool value;
		ifs.read(reinterpret_cast<char*>(&value), sizeof(bool));
		return value;
	}

	inline unsigned char TrackMapProcessor::ReadTag(std::ifstream& ifs)
	{
		unsigned char tag;
		ifs.read(reinterpret_cast<char*>(&tag), sizeof(unsigned char));
		return tag;
	}

	inline unsigned int TrackMapProcessor::ReadUint(std::ifstream& ifs)
	{
		unsigned int value;
		ifs.read(reinterpret_cast<char*>(&value), sizeof(unsigned int));
		return value;
	}

	inline float TrackMapProcessor::ReadFloat(std::ifstream& ifs)
	{
		float value;
		ifs.read(reinterpret_cast<char*>(&value), sizeof(float));
		return value;
	}

	inline FLIVR::Point TrackMapProcessor::ReadPoint(std::ifstream& ifs)
	{
		double x, y, z;
		ifs.read(reinterpret_cast<char*>(&x), sizeof(double));
		ifs.read(reinterpret_cast<char*>(&y), sizeof(double));
		ifs.read(reinterpret_cast<char*>(&z), sizeof(double));
		return FLIVR::Point(x, y, z);
	}

	inline pCell TrackMapProcessor::ReadCell(std::ifstream& ifs, CellList& cell_list)
	{
		pCell cell;
		if (ReadTag(ifs) != TAG_CELL)
			return cell;
		unsigned int id = ReadUint(ifs);
		if (cell_list.find(id) != cell_list.end())
			return cell;
		cell = pCell(new Cell(id));
		cell->SetSizeUi(ReadUint(ifs));
		cell->SetSizeF(ReadFloat(ifs));
		cell->SetExternalUi(ReadUint(ifs));
		cell->SetExternalF(ReadFloat(ifs));
		FLIVR::Point p = ReadPoint(ifs);
		cell->SetCenter(p);
		FLIVR::BBox box;
		FLIVR::Point p1;
		if (ReadTag(ifs) == TAG_VER220)
		{
			p = ReadPoint(ifs);
			p1 = ReadPoint(ifs);
			box = FLIVR::BBox(p, p1);
			cell->SetBox(box);
		}
		else
			ifs.unget();
		cell_list.insert(std::pair<unsigned int, pCell>
			(id, cell));
		return cell;
	}

	//random
	inline bool TrackMapProcessor::get_random(size_t count, InterGraph &graph)
	{
		int c = graph.counter;
		if (c < 4)
			return true;
		int r = c / 2 + rand() % c;
		return count < r;
	}

	class TrackMap
	{
	public:
		TrackMap();
		~TrackMap();

		size_t GetFrameNum();
		CellList &GetCellList(size_t frame);
		VertexList &GetVertexList(size_t frame);
		IntraGraph &GetIntraGraph(size_t frame);
		InterGraph &GetInterGraph(size_t frame);
		bool ExtendFrameNum(size_t frame);
		void Clear();

	private:
		unsigned int m_counter;//counter for frame processing
		//data information
		size_t m_frame_num;
		size_t m_size_x;
		size_t m_size_y;
		size_t m_size_z;
		size_t m_data_bits;
		float m_scale;
		float m_spc_x;
		float m_spc_y;
		float m_spc_z;

		//lists
		std::deque<CellList> m_cells_list;
		std::deque<VertexList> m_vertices_list;
		std::deque<IntraGraph> m_intra_graph_list;
		std::deque<InterGraph> m_inter_graph_list;

		friend class TrackMapProcessor;
	};

	inline size_t TrackMap::GetFrameNum()
	{
		return m_frame_num;
	}

	inline CellList &TrackMap::GetCellList(size_t frame)
	{
		return m_cells_list.at(frame);
	}

	inline VertexList &TrackMap::GetVertexList(size_t frame)
	{
		return m_vertices_list.at(frame);
	}

	inline IntraGraph &TrackMap::GetIntraGraph(size_t frame)
	{
		return m_intra_graph_list.at(frame);
	}

	inline InterGraph &TrackMap::GetInterGraph(size_t frame)
	{
		return m_inter_graph_list.at(frame);
	}

	inline bool TrackMap::ExtendFrameNum(size_t frame)
	{
		size_t sframe = m_frame_num;
		for (size_t i = sframe; i <= frame; ++i)
		{
			m_cells_list.push_back(CellList());
			m_vertices_list.push_back(VertexList());
			m_intra_graph_list.push_back(IntraGraph());
			if (m_inter_graph_list.size() < frame)
			{
				m_inter_graph_list.push_back(InterGraph());
				m_inter_graph_list.back().index = m_frame_num;
			}
			m_frame_num++;
		}
		return frame < m_frame_num;
	}

	inline void TrackMap::Clear()
	{
		m_cells_list.clear();
		m_vertices_list.clear();
		m_intra_graph_list.clear();
		m_inter_graph_list.clear();
		m_frame_num = 0;
		m_size_x = m_size_y = m_size_z = 0;
		m_data_bits = 8;
		m_scale = 1.0f;
		m_counter = 0;
	}

}//namespace FL

#endif//FL_TrackMap_h
