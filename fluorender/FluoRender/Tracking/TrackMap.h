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

namespace FL
{
//tags
#define TAG_CELL		1
#define TAG_VERT		2
#define TAG_INTRA_EDGE	3
#define TAG_INTER_EDGE	4
#define TAG_FRAM		5
#define TAG_LAST_OP		6
#define TAG_NUM			7

	typedef boost::signals2::signal<void(int)> SignalProg;
	class TrackMap;
	class TrackMapProcessor
	{
	public:
		TrackMapProcessor() :
		m_contact_thresh(0.5f), m_size_thresh(25.0f) {};
		~TrackMapProcessor() {};

		void ConnectSignalProgress(SignalProg::slot_type func);

		void SetContactThresh(float value);
		void SetSizeThresh(float value);

		void SetSizes(TrackMap& track_map,
			size_t nx, size_t ny, size_t nz);
		void SetBits(TrackMap& track_map,
			size_t bits);

		bool InitializeFrame(TrackMap& track_map,
			void *data, void *label, size_t frame);
		bool LinkMaps(TrackMap& track_map,
			size_t f1, size_t f2,
			void *data1, void *data2,
			void *label1, void *label2);
		bool ResolveGraph(TrackMap& track_map, size_t frame1, size_t frame2);
		bool MatchFrames(TrackMap& track_map, size_t frame1, size_t frame2, bool bl_check = true);
		bool UnmatchFrames(TrackMap& track_map, size_t frame1, size_t frame2);

		bool Export(TrackMap& track_map, std::string &filename);
		bool Import(TrackMap& track_map, std::string &filename);

		bool ResetVertexIDs(TrackMap& track_map);

		//get mapped cell
		bool GetMappedID(TrackMap& track_map,
			unsigned int id_in, unsigned int& id_out,
			size_t frame);
		bool GetMappedID(TrackMap& track_map,
			unsigned int id_in, unsigned int& id_out,
			size_t frame1, size_t frame2);
		//get mapped cells
		bool GetMappedCells(TrackMap& track_map,
			CellList &sel_list1, CellList &sel_list2,
			size_t frame1, size_t frame2);
		//edges (in a vector of drawable)
		unsigned int GetMappedEdges(TrackMap& track_map,
			CellList &sel_list1, CellList &sel_list2,
			std::vector<float> &verts,
			size_t frame1, size_t frame2);

		//modifications
		bool LinkCells(TrackMap& track_map,
			CellList &list1, CellList &list2,
			size_t frame1, size_t frame2, bool exclusive);

	private:
		float m_contact_thresh;
		float m_size_thresh;

		//processing
		bool CheckCellContact(TrackMap& track_map,
			pCell &cell, void *data, void *label,
			size_t ci, size_t cj, size_t ck);
		bool AddContact(IntraGraph& graph,
			pCell &cell1, pCell &cell2,
			float contact_value);
		bool LinkVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2,
			float overlap_value);
		bool IsolateVertex(InterGraph& graph,
			pVertex &vertex);
		bool ForceVertices(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2);
		bool EqualCells(pwCell &cell1, pwCell &cell2);
		bool FindCellBin(CellBin &bin, pwCell &cell);
		bool AddCellBin(std::vector<CellBin> &bins,
			pwCell &cell);
		bool AddCellBin(std::vector<CellBin> &bins,
			pwCell &cell1, pwCell &cell2);
		bool GreaterThanCellBin(pCell &cell1, CellBin &bin, pwCell &cell2);
		size_t GetBinsCellCount(std::vector<CellBin> &bins);
		bool MergeCells(VertexList& vertex_list, CellBin &bin,
			TrackMap& track_map, size_t frame);
		bool RelinkInterGraph(pVertex &vertex, pVertex &vertex0, size_t frame, InterGraph &graph);
		bool MatchVertex(pVertex &vertex, InterGraph &graph, bool bl_check = true);
		bool UnmatchVertex(pVertex &vertex, InterGraph &graph);
		unsigned int CheckBackLink(InterVert v0, InterVert v1, InterGraph &graph,
			unsigned int &bl_size_ui, float &bl_size_f);
		static bool edge_comp_size_ol(InterEdge edge1, InterEdge edge2, InterGraph& graph);
		static bool edge_comp_size_bl(InterEdge edge1, InterEdge edge2, InterGraph& graph);

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
			unsigned int size_ui, float size_f);
		bool AddInterEdge(InterGraph& graph,
			pVertex &vertex1, pVertex &vertex2,
			size_t f1, size_t f2,
			unsigned int size_ui, float size_f,
			float dist, unsigned int link);

		//update progress
		SignalProg m_sig_progress;
		boost::signals2::connection m_con_progress;
	};

	inline void TrackMapProcessor::ConnectSignalProgress(
		SignalProg::slot_type func)
	{
		if (m_con_progress.connected())
			m_con_progress.disconnect();
		m_con_progress = m_sig_progress.connect(func);
	}

	inline void TrackMapProcessor::SetContactThresh(float value)
	{
		m_contact_thresh = value;
	}

	inline void TrackMapProcessor::SetSizeThresh(float value)
	{
		m_size_thresh = value;
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
		unsigned int GetLastOp();
		void Clear();

	private:
		unsigned int m_last_op;//1: linking; 2: unlinking;
		//data information
		size_t m_frame_num;
		size_t m_size_x;
		size_t m_size_y;
		size_t m_size_z;
		size_t m_data_bits;
		float m_scale;

		//lists
		std::vector<CellList> m_cells_list;
		std::vector<VertexList> m_vertices_list;
		std::vector<IntraGraph> m_intra_graph_list;
		std::vector<InterGraph> m_inter_graph_list;

		friend class TrackMapProcessor;
	};

	inline size_t TrackMap::GetFrameNum()
	{
		return m_frame_num;
	}

	inline unsigned int TrackMap::GetLastOp()
	{
		return m_last_op;
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
		m_last_op = 0;
	}

}//namespace FL

#endif//FL_TrackMap_h