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

	struct UncertainBin
	{
		unsigned int level;
		unsigned int count;
	};
	typedef std::map<unsigned int, UncertainBin> UncertainHist;
	class TrackMap;
	class TrackMapProcessor
	{
	public:
		TrackMapProcessor(TrackMap &track_map) :
		m_map(track_map),
		m_contact_thresh(0.7f),
		m_size_thresh(25.0f),
		m_level_thresh(7),
		m_similar_thresh(0.2f) {}
		~TrackMapProcessor() {}

		void SetContactThresh(float value);
		void SetSizeThresh(float value);
		void SetLevelThresh(int level);
		void SetUncertainLow(unsigned int value);
		void SetUncertainHigh(unsigned int value);

		void SetSizes(size_t nx, size_t ny, size_t nz);
		void SetBits(size_t bits);
		void SetScale(float scale);
		void SetSpacings(float spcx, float spcy, float spcz);

		//build cell list and intra graph
		bool InitializeFrame(void *data, void *label, size_t frame);
		//build inter graph
		bool LinkMaps(size_t f1, size_t f2,
			void *data1, void *data2,
			void *label1, void *label2);
		//group cells
		bool ResolveGraph(size_t frame1, size_t frame2);
		//find the maximum overlapping and set link flags on inter graph
		bool ProcessFrames(size_t frame1, size_t frame2);
		//for multiple links, remove link flags
		bool UnmatchFrames(size_t frame1, size_t frame2);
		//re-segment frame
		bool ResegmentFrame(size_t frame);
		//for orphans, search neighbors to add link flags
		bool ExMatchFrames(size_t frame1, size_t frame2);

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
		bool IsolateCells(CellList &list, size_t frame);
		bool UnlinkCells(CellList &list1, CellList &list2,
			size_t frame1, size_t frame2);
		//
		bool AddCell(pCell &cell, size_t frame, CellListIter &iter);
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

	private:
		float m_contact_thresh;
		float m_size_thresh;
		int m_level_thresh;
		float m_similar_thresh;
		//uncertainty filter
		unsigned int m_uncertain_low;
		unsigned int m_uncertain_high;
		//the trackmap
		TrackMap &m_map;

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
		bool LinkOrphans(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2,
			float dist_value);
		bool IsolateVertex(InterGraph& graph,
			pVertex &vertex);
		bool ForceVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2);
		bool UnlinkVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2);
		bool EqualCells(pwCell &cell1, pwCell &cell2);
		bool FindCellBin(CellBin &bin, pwCell &cell);
		bool AddCellBin(std::vector<CellBin> &bins,
			pwCell &cell);
		bool AddCellBin(std::vector<CellBin> &bins,
			pwCell &cell1, pwCell &cell2);
		bool GreaterThanCellBin(pCell &cell1, CellBin &bin, pwCell &cell2);
		size_t GetBinsCellCount(std::vector<CellBin> &bins);
		bool MergeCells(VertexList& vertex_list, CellBin &bin, size_t frame);
		bool RelinkInterGraph(pVertex &vertex, pVertex &vertex0, size_t frame, InterGraph &graph, bool reset);
		
		//replaces all previous match/unmatch funcs
		bool ProcessVertex(pVertex &vertex, InterGraph &graph);
		//vertex matching routines
		//find out current valence of a vertex
		bool GetValence(pVertex &vertex, InterGraph &graph,
			size_t &valence);
		//edges include linked and unlinked
		bool GetValence(pVertex &vertex, InterGraph &graph,
			size_t &valence, std::vector<InterEdge> &edges);
		//edges include linked only
		bool GetLinkedEdges(pVertex &vertex, InterGraph &graph,
			std::vector<InterEdge> &edges);
		//detailed match functions
		//link edge of the max overlap
		bool LinkEdgeMax(InterGraph &graph, std::vector<InterEdge> &edges);
		//unlink edge by size similarity
		bool UnlinkEdgeSize(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		//unlink edge by extended alternating path
		bool UnlinkAlterPath(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		//reduce valence by segmentation
		bool MatchVertexMerge(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		bool MatchVertexCluster(InterGraph &graph, pVertex &vertex,
			std::vector<InterEdge> &edges);
		
		//helper functions
		bool get_alter_path(InterGraph &graph, pVertex &vertex,
			Path &alt_path, VertexList &visited, int curl);
		static bool comp_edge_size(InterEdge &edge1, InterEdge &edge2, InterGraph& graph);
		bool similar_edge_size(InterEdge edge1, InterEdge edge2, InterGraph& graph);
		static bool comp_path_size(Path &path1, Path &path2, InterGraph& graph);
		void link_edge(InterEdge edge, InterGraph &graph, unsigned int value = 1);
		void unlink_edge(InterEdge edge, InterGraph &graph, unsigned int value = 0);

		//bool MatchVertex(pVertex &vertex, InterGraph &graph, bool bl_check = true);
		bool UnmatchVertex(pVertex &vertex, InterGraph &graph);
		bool ExMatchVertex(pVertex &vertex, InterGraph &graph, size_t frame1, size_t frame2);
		bool MatchVertexList(pVertex &vertex, VertexList &list2,
			InterGraph &graph, size_t frame1, size_t frame2);
		void FindOrphans(pVertex &vertex, InterGraph &inter_graph,
			VertexList &orphan_list, VertexList &visited_list, int level);
		unsigned int CheckBackLink(InterVert v0, InterVert v1, InterGraph &graph,
			unsigned int &bl_size_ui, float &bl_size_f);

		//export
		void WriteBool(std::ofstream& ofs, bool value);
		void WriteTag(std::ofstream& ofs, unsigned char tag);
		void WriteUint(std::ofstream& ofs, unsigned int value);
		void WriteFloat(std::ofstream& ofs, float value);
		void WritePoint(std::ofstream& ofs, FLIVR::Point &point);
		void WriteCell(std::ofstream& ofs, pCell &cell);
		void WriteVertex(std::ofstream& ofs, pVertex &vertex);
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

	inline void TrackMapProcessor::SetContactThresh(float value)
	{
		m_contact_thresh = value;
	}

	inline void TrackMapProcessor::SetSizeThresh(float value)
	{
		m_size_thresh = value;
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

	inline void TrackMapProcessor::WriteBool(std::ofstream& ofs, bool value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(bool));
	}

	inline void TrackMapProcessor::WriteTag(std::ofstream& ofs, unsigned char tag)
	{
		ofs.write(reinterpret_cast<const char*>(&tag), sizeof(unsigned char));
	}

	inline void TrackMapProcessor::WriteUint(std::ofstream& ofs, unsigned int value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(unsigned int));
	}

	inline void TrackMapProcessor::WriteFloat(std::ofstream& ofs, float value)
	{
		ofs.write(reinterpret_cast<const char*>(&value), sizeof(float));
	}

	inline void TrackMapProcessor::WritePoint(std::ofstream& ofs, FLIVR::Point &point)
	{
		double x = point.x();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
		x = point.y();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
		x = point.z();
		ofs.write(reinterpret_cast<const char*>(&x), sizeof(double));
	}

	inline void TrackMapProcessor::WriteCell(std::ofstream& ofs, pCell &cell)
	{
		WriteTag(ofs, TAG_CELL);
		WriteUint(ofs, cell->Id());
		WriteUint(ofs, cell->GetSizeUi());
		WriteFloat(ofs, cell->GetSizeF());
		WriteUint(ofs, cell->GetExternalUi());
		WriteFloat(ofs, cell->GetExternalF());
		WritePoint(ofs, cell->GetCenter());
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
		cell_list.insert(std::pair<unsigned int, pCell>
			(id, cell));
		return cell;
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