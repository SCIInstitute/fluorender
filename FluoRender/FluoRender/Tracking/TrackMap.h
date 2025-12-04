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
#ifndef FL_TrackMap_h
#define FL_TrackMap_h

#include <Cell.h>
#include <VertexList.h>
#include <Progress.h>
#include <fstream>
#include <memory>
#include <deque>
#include <map>

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
#define TAG_VER221		10	//new values added in v2.21

namespace flrd
{
	class TrackMap;
	typedef std::shared_ptr<TrackMap> pTrackMap;
	typedef std::weak_ptr<TrackMap> pwTrackMap;
	typedef std::function<void(const std::wstring&)> InfoOutFunc;

	class TrackMapProcessor : public Progress
	{
	public:
		TrackMapProcessor() :
		Progress(),
		m_contact_thresh(0.6f),
		m_size_thresh(25.0f),
		m_similar_thresh(0.2f),
		m_level_thresh(2),
		m_merge(false),
		m_split(false),
		m_max_iter(25),
		m_eps(1e-3f),
		m_filter(1),
		m_stencil_thresh(2){}
		~TrackMapProcessor();

		//automatic tracking
		void GenMap();
		void RefineMap(int t = -1, bool erase_v = true);

		void SetTrackMap(const pTrackMap& map);
		void SetContactThresh(double value);
		void SetSizeThresh(double value);
		void SetSimilarThresh(double value);
		void SetLevelThresh(int level);
		void SetUncertainLow(unsigned int value);
		void SetMerge(bool value);
		void SetSplit(bool value);
		void SetMaxIter(int value);
		void SetEps(double value);
		void SetFilterSize(size_t value);
		void SetStencilThresh(const fluo::Point &value);

		void SetSizes(const fluo::Vector& size);
		void SetBits(size_t bits);
		void SetScale(double scale);
		void SetSpacing(const fluo::Vector& spc);

		void SetClusterNum(int val);
		int GetClusterNum();

		//build cell list and intra graph
		bool InitializeFrame(size_t frame);
		//build inter graph
		bool LinkFrames(size_t f1, size_t f2);
		//group cells
		bool ResolveGraph(size_t frame1, size_t frame2);
		//find the maximum overlapping and set link flags on inter graph
		bool ProcessFrames(size_t frame1, size_t frame2, bool erase_v=true);

		//make id consistent
		bool MakeConsistent(size_t frame);//combine cells within vertex
		bool MakeConsistent(size_t frame1, size_t frame2);//make mapped cells same color

		//clear counters
		bool ClearCounters();

		bool Export(const std::wstring &filename);
		bool Import(const std::wstring &filename);

		bool ResetVertexIDs();

		//get
		Celp GetCell(size_t frame, unsigned int id);
		Verp GetVertex(Celp &celp);
		unsigned int GetUniCellID(size_t frame, unsigned int id);
		unsigned int GetNewCellID(size_t frame, unsigned int id, bool inc=false);
		unsigned int GetTrackedID(size_t frame1, size_t frame2, unsigned int id);

		//get mapped cell
		//bool GetMappedID(unsigned int id_in, unsigned int& id_out,
		//	size_t frame);
		//get mapped cells
		bool GetMappedCells(CelpList &sel_list1, CelpList &sel_list2,
			size_t frame1, size_t frame2);

		//modifications
		bool LinkCells(bool exclusive);
		bool LinkAllCells();
		bool LinkCells(Celp &celp1, Celp &celp2,
			size_t frame1, size_t frame2, bool exclusive);
		bool IsolateCells();
		bool UnlinkCells();
		//
		bool AddCellDup(Celp & celp, size_t frame);
		bool AddCell(Celp &celp, size_t frame, CelpListIter &iter);
		bool AddCells(CelpList &list, size_t frame);
		bool RemoveCells(CelpList &list, size_t frame);
		bool LinkAddedCells(CelpList &list, size_t frame1, size_t frame2);
		bool CombineCells(Celp &celp, CelpList &list, size_t frame);
		bool DivideCells();
		bool SegmentCells();
		bool ReplaceCellID(unsigned int old_id,
			unsigned int new_id, size_t frame);

		//relink cells after segmentation
		void RelinkCells(CelpList &in, CelpList& out, size_t frame);

		//track list
		void SetListIn(CelpList& list);
		void SetListOut(CelpList& list);
		CelpList& GetListIn();
		CelpList& GetListOut();
		//information
		void AnalyzeLink();
		void GetLinkLists(size_t frame,
			flrd::VertexList &in_orphan_list,
			flrd::VertexList &out_orphan_list,
			flrd::VertexList &in_multi_list,
			flrd::VertexList &out_multi_list);
		void AnalyzeUncertainty();
		void GetCellsByUncertainty(bool filter_in_list);
		void GetCellUncertainty();
		void GetUncertainHist();
		void GetUncertainHist(UncertainHist& hist, VertexList &vertex_list, InterGraph &graph);
		void AnalyzePath();
		void GetPaths(PathList &path_list, size_t frame1, size_t frame2);

		//tracking by matching user input
		//mode: 0-compare adj; 1-compare start;
		//sim: 0-dot product; 1-diff squared
		bool TrackStencils(size_t frame1, size_t frame2,
			fluo::Vector &extt, fluo::Vector &exta,
			int mode, size_t start, int sim);

		//conversion
		void ConvertRulers();
		void ConvertConsistent();

		//info out
		void RegisterInfoOutFunc(const InfoOutFunc& f);
		void UnregisterInfoOutFunc();

	private:
		//the trackmap
		pTrackMap m_map;
		float m_contact_thresh;
		float m_size_thresh;
		float m_similar_thresh;
		int m_level_thresh;
		int m_max_iter;
		float m_eps;
		size_t m_filter;
		bool m_merge;
		bool m_split;
		fluo::Point m_stencil_thresh;
		//uncertainty filter
		unsigned int m_uncertain_low;
		//cur & neighbor frames for orphan searching
		size_t m_frame1;
		size_t m_frame2;
		bool m_major_converge;//majority of the links have converged
		int m_cluster_num;//for splitting cells

		CelpList m_list_in;
		CelpList m_list_out;

	private:
		//modification
		bool CheckCellContact(Celp &celp, void *data, void *label,
			size_t ci, size_t cj, size_t ck);
		bool AddContact(CellGraph& graph,
			Celp &celp1, Celp &celp2,
			float contact_value);
		bool CheckCellDist(Celp &celp, void *label,
			size_t ci, size_t cj, size_t ck);
		bool AddNeighbor(CellGraph& graph,
			Celp &celp1, Celp &celp2,
			float dist_v, float dist_s);
		bool LinkVertices(InterGraph& graph,
			Verp &vertex1, Verp &vertex2,
			size_t f1, size_t f2,
			float overlap_value);
		bool IsolateVertex(InterGraph& graph,
			Verp &vertex);
		bool ForceVertices(InterGraph& graph,
			Verp &vertex1, Verp &vertex2,
			size_t f1, size_t f2);
		bool UnlinkVertices(InterGraph& graph,
			Verp &vertex1, Verp &vertex2);
		bool MergeCells(VertexList& vertex_list, CellBin &bin, size_t frame);
		bool RelinkInterGraph(Verp &vertex, Verp &vertex0, size_t frame, InterGraph &graph, bool reset);
		bool RemoveVertex(InterGraph& graph, Verp &vertex);
		
		//determine if cells on intragraph can be merged
		typedef bool(TrackMapProcessor::*f_merge_cell)(CelEdge&, Celp&, Celp&, CellGraph&);
		bool GroupCells(std::vector<Celw> &celws, std::vector<CellBin> &cell_bins,
			CellGraph &intra_graph, f_merge_cell merge_cell);
		bool EqualCells(Celw &celw1, Celw &celw2);
		bool FindCellBin(CellBin &bin, Celw &celw);
		bool AddCellBin(std::vector<CellBin> &bins,
			Celw &celw);
		bool AddCellBin(std::vector<CellBin> &bins,
			Celw &celw1, Celw &celw2);
		bool GreaterThanCellBin(Celp &celp1, CellBin &bin, Celw &celw2);
		size_t GetBinsCellCount(std::vector<CellBin> &bins);

		//replaces all previous match/unmatch funcs
		bool ProcessVertex(Verp &vertex, InterGraph &graph,
			unsigned int seg_count_min);
		//vertex matching routines
		//find out current valence of a vertex
		bool GetValence(Verp &vertex, InterGraph &graph,
			size_t &valence);
		//edges include linked and unlinked
		bool GetValence(Verp &vertex, InterGraph &graph,
			size_t &valence, std::vector<Edge> &edges);
		//edges include linked only
		bool GetValence(Verp &vertex, InterGraph &graph,
			size_t &valence, std::vector<Edge> &all_edges,
			std::vector<Edge> &linked_edges);
		//get uncertain edges for segmentation
		bool GetUncertainEdges(Verp &vertex, InterGraph &graph,
			unsigned int min_count,
			std::vector<Edge> &uncertain_edges);
		//detailed match functions
		//link edge of the max overlap
		bool LinkEdgeSize(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges, bool calc_sim);
		//search for neighboring orphans for linking
		bool LinkOrphans(InterGraph& graph, Verp &vertex);
		//unlink edge by size similarity
		bool UnlinkEdgeSize(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges, bool calc_sim);
		//unlink edge by count
		bool UnlinkEdgeCount(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges);
		//unlink last edge
		bool UnlinkEdgeLast(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges);
		//unlink edge by extended alternating path
		bool UnlinkAlterPath(InterGraph &graph, Verp &vertex,
			bool calc_sim);
		//fix multi-link by segmentation
		bool UnlinkSegment(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &linked_edges, bool calc_sim,
			bool segment, unsigned int seg_count_min);
		bool GetAlterPath(InterGraph &graph, Verp &vertex,
			PathList &paths);
		bool UnlinkAlterPathMaxMatch(InterGraph &graph, Verp &vertex,
			PathList &paths, bool calc_sim);
		bool UnlinkAlterPathSize(InterGraph &graph, Verp &vertex,
			PathList &paths);
		bool UnlinkAlterPathConn(InterGraph &graph, Verp &vertex,
			PathList &paths);
		bool UnlinkAlterPathCount(InterGraph &graph, Verp &vertex,
			PathList &paths);
		//check if any out vertex can be combined
		bool MergeEdges(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges);
		//unlink edge by vertex size, use after merge fails
		bool UnlinkVertexSize(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges);
		//check if the vertex can split
		bool SplitVertex(InterGraph &graph, Verp &vertex,
			std::vector<Edge> &edges);
		bool ClusterCellsMerge(CelpList &list, size_t frame);
		bool ClusterCellsSplit(CelpList &list, size_t frame,
			size_t clnum, CelpList &listout);

		//helper functions
		bool get_alter_path(InterGraph &graph, Verp &vertex,
			Path &alt_path, VertVisitList &visited, int curl);
		float get_path_max(InterGraph &graph, PathList &paths,
			size_t curl, Vrtx v0);
		bool unlink_alt_path(InterGraph &graph, PathList &paths);
		bool merge_cell_size(CelEdge &edge, Celp &celp1, Celp &celp2, CellGraph& graph);
		static bool comp_edge_size(Edge &edge1, Edge &edge2, InterGraph& graph);
		static bool comp_edge_count(Edge &edge1, Edge &edge2, InterGraph& graph);
		bool similar_edge_size(Edge &edge1, Edge &edge2, InterGraph& graph);
		static bool comp_path_size(Path &path1, Path &path2);
		static bool comp_path_mm(Path &path1, Path &path2);
		bool similar_path_size(Path &path1, Path &path2);
		bool similar_path_mm(Path &path1, Path &path2);
		static bool comp_path_count(Path &path1, Path &path2);
		static bool comp_path_count_rev(Path &path1, Path &path2);
		bool similar_path_count(Path &path1, Path &path2);
		bool similar_vertex_size(Verp& v1, Verp& v2);

		void link_edge(Edge edge, InterGraph &graph, unsigned int value = 1);
		void unlink_edge(Edge edge, InterGraph &graph, unsigned int value = 0);

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
		void WriteDouble(std::ofstream& ofs, const double value);
		void WritePoint(std::ofstream& ofs, const fluo::Point &point);
		void WriteCell(std::ofstream& ofs, const Celp &celp);
		void WriteVertex(std::ofstream& ofs, const Verp &vertex);
		//import
		bool ReadBool(std::ifstream& ifs);
		unsigned char ReadTag(std::ifstream& ifs);
		unsigned int ReadUint(std::ifstream& ifs);
		float ReadFloat(std::ifstream& ifs);
		double ReadDouble(std::ifstream& ifs);
		fluo::Point ReadPoint(std::ifstream& ifs);
		Celp ReadCell(std::ifstream& ifs, CelpList& list);
		void ReadVertex(std::ifstream& ifs, VertexList& vertex_list, CelpList& list);
		bool AddIntraEdge(CellGraph& graph,
			Celp &celp1, Celp &celp2,
			unsigned int size_ui, double size_d,
			double dist_v, double dist_s);
		bool AddInterEdge(InterGraph& graph,
			Verp &vertex1, Verp &vertex2,
			size_t f1, size_t f2,
			unsigned int size_ui, double size_d,
			double dist, unsigned int link,
			unsigned int v1_count, unsigned int v2_count,
			unsigned int edge_count);

		//output info
		void WriteInfo(const std::wstring& str)
		{
			if (m_info_out)
				m_info_out(str);
		}
		InfoOutFunc m_info_out;
	};

	inline TrackMapProcessor::~TrackMapProcessor()
	{
	}

	inline void TrackMapProcessor::SetContactThresh(double value)
	{
		m_contact_thresh = static_cast<float>(value);
	}

	inline void TrackMapProcessor::SetSizeThresh(double value)
	{
		m_size_thresh = static_cast<float>(value);
	}

	inline void TrackMapProcessor::SetSimilarThresh(double value)
	{
		m_similar_thresh = static_cast<float>(value);
	}

	inline void TrackMapProcessor::SetLevelThresh(int level)
	{
		m_level_thresh = level;
	}

	inline void TrackMapProcessor::SetMerge(bool value)
	{
		m_merge = value;
	}

	inline void TrackMapProcessor::SetSplit(bool value)
	{
		m_split = value;
	}

	inline void TrackMapProcessor::SetMaxIter(int value)
	{
		m_max_iter = value;
	}

	inline void TrackMapProcessor::SetEps(double value)
	{
		m_eps = static_cast<float>(value);
	}

	inline void TrackMapProcessor::SetFilterSize(size_t value)
	{
		m_filter = value;
	}

	inline void TrackMapProcessor::SetStencilThresh(const fluo::Point &value)
	{
		m_stencil_thresh = value;
	}

	inline void TrackMapProcessor::RegisterInfoOutFunc(const InfoOutFunc& f)
	{
		m_info_out = f;
	}

	inline void TrackMapProcessor::UnregisterInfoOutFunc()
	{
		m_info_out = nullptr;
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

	inline void TrackMapProcessor::WriteDouble(std::ofstream& ofs, const double value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(double));
	}

	inline void TrackMapProcessor::WritePoint(std::ofstream& ofs, const fluo::Point &point)
	{
		double x = point.x();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
		x = point.y();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
		x = point.z();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
	}

	inline void TrackMapProcessor::WriteCell(std::ofstream& ofs, const Celp &celp)
	{
		WriteTag(ofs, TAG_CELL);
		WriteUint(ofs, celp->Id());
		WriteTag(ofs, TAG_VER221);
		WriteUint(ofs, celp->BrickId());
		WriteUint(ofs, celp->GetSizeUi());
		WriteDouble(ofs, celp->GetSizeD());
		WriteUint(ofs, celp->GetExtUi());
		WriteDouble(ofs, celp->GetExtD());
		WritePoint(ofs, celp->GetCenter());
		WriteTag(ofs, TAG_VER220);
		WritePoint(ofs, celp->GetBox().Min());
		WritePoint(ofs, celp->GetBox().Max());
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

	inline double TrackMapProcessor::ReadDouble(std::ifstream& ifs)
	{
		double value;
		ifs.read(reinterpret_cast<char*>(&value), sizeof(double));
		return value;
	}

	inline fluo::Point TrackMapProcessor::ReadPoint(std::ifstream& ifs)
	{
		double x, y, z;
		ifs.read(reinterpret_cast<char*>(&x), sizeof(double));
		ifs.read(reinterpret_cast<char*>(&y), sizeof(double));
		ifs.read(reinterpret_cast<char*>(&z), sizeof(double));
		return fluo::Point(x, y, z);
	}

	inline Celp TrackMapProcessor::ReadCell(std::ifstream& ifs, CelpList& list)
	{
		Celp celp;
		if (ReadTag(ifs) != TAG_CELL)
			return celp;
		unsigned int id = ReadUint(ifs);
		if (list.find(id) != list.end())
			return celp;
		celp = Celp(new Cell(id));
		if (ReadTag(ifs) == TAG_VER221)
		{
			unsigned int brick_id = ReadUint(ifs);
			celp->SetBrickId(brick_id);
		}
		else
			ifs.unget();
		celp->SetSizeUi(ReadUint(ifs));
		celp->SetSizeD(ReadDouble(ifs));
		celp->SetExtUi(ReadUint(ifs));
		celp->SetExtD(ReadDouble(ifs));
		fluo::Point p = ReadPoint(ifs);
		celp->SetCenter(p);
		fluo::BBox box;
		fluo::Point p1;
		if (ReadTag(ifs) == TAG_VER220)
		{
			p = ReadPoint(ifs);
			p1 = ReadPoint(ifs);
			box = fluo::BBox(p, p1);
			celp->SetBox(box);
		}
		else
			ifs.unget();
		list.insert(std::pair<unsigned int, Celp>
			(id, celp));
		return celp;
	}

	//random
	inline bool TrackMapProcessor::get_random(size_t count, InterGraph &graph)
	{
		size_t c = graph.counter;
		if (c < 4)
			return true;
		size_t r = c / 2 + std::rand() % c;
		return count < r;

		//if (rand() % c < 10)
		//	return true;
		//if (count < 4)
		//	return true;
		//if (rand() % count < 10)
		//	return true;
		//return false;
	}

	class TrackMap
	{
	public:
		TrackMap();
		~TrackMap();

		size_t GetFrameNum();
		CelpList &GetCellList(size_t frame);
		VertexList &GetVertexList(size_t frame);
		CellGraph &GetIntraGraph(size_t frame);
		InterGraph &GetInterGraph(size_t frame);
		bool ExtendFrameNum(size_t frame);
		void Clear();

	private:
		unsigned int m_counter;//counter for frame processing
		//data information
		size_t m_frame_num;
		fluo::Vector m_size;
		size_t m_data_bits;
		float m_scale;
		fluo::Vector m_spacing;

		//lists
		std::deque<CelpList> m_celp_list;
		std::deque<VertexList> m_vertices_list;
		std::deque<CellGraph> m_intra_graph_list;
		std::deque<InterGraph> m_inter_graph_list;

		friend class TrackMapProcessor;
	};

	inline size_t TrackMap::GetFrameNum()
	{
		return m_frame_num;
	}

	inline CelpList &TrackMap::GetCellList(size_t frame)
	{
		return m_celp_list.at(frame);
	}

	inline VertexList &TrackMap::GetVertexList(size_t frame)
	{
		return m_vertices_list.at(frame);
	}

	inline CellGraph &TrackMap::GetIntraGraph(size_t frame)
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
			m_celp_list.push_back(CelpList());
			m_vertices_list.push_back(VertexList());
			m_intra_graph_list.push_back(CellGraph());
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
		m_celp_list.clear();
		m_vertices_list.clear();
		m_intra_graph_list.clear();
		m_inter_graph_list.clear();
		m_frame_num = 0;
		m_size = fluo::Vector(0);
		m_data_bits = 8;
		m_scale = 1.0f;
		m_counter = 0;
	}

	//get
	inline Celp TrackMapProcessor::GetCell(size_t frame, unsigned int id)
	{
		if (frame >= m_map->m_frame_num)
			return nullptr;

		CelpList &list = m_map->m_celp_list.at(frame);
		auto it = list.find(id);
		if (it == list.end())
			return nullptr;
		else
			return it->second;
	}

	inline Verp TrackMapProcessor::GetVertex(Celp &celp)
	{
		if (!celp)
			return nullptr;

		Verw pvert = celp->GetVertex();
		return pvert.lock();
	}

	inline unsigned int TrackMapProcessor::GetUniCellID(size_t frame, unsigned int id)
	{
		unsigned int rid = 0;
		Celp celp = GetCell(frame, id);
		if (!celp)
			return rid;
		Verp vert = GetVertex(celp);
		if (!vert ||
			vert->FindCell(celp) < 0)
			return rid;
		Celp cell0 = vert->GetCell(0);
		rid = cell0->Id();
		return rid;
	}

	//get new id
	inline unsigned int TrackMapProcessor::GetNewCellID(size_t frame, unsigned int id, bool inc)
	{
		bool wrap = false;
		//cell list
		CelpList &list = m_map->m_celp_list.at(frame);
		unsigned int newid = id+(inc?253:0);
		newid = newid < id ? (wrap = true, (id % 253)) : newid;
		while (list.find(newid) != list.end())
		{
			newid += 253;
			if (newid < newid - 253)
			{
				if (wrap)
					break;
				else
				{
					newid = id % 253;
					wrap = true;
				}
			}
		}
		return newid;
	}

}//namespace flrd

#endif//FL_TrackMap_h
