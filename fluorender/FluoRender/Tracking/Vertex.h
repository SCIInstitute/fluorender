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
#ifndef FL_Vertex_h
#define FL_Vertex_h

#include "Cell.h"
#include <vector>

namespace FL
{
	struct InterEdgeData
	{
		unsigned int size_ui;
		float size_f;
		float dist;
		unsigned int link;
		//unsaved, for calculation
		unsigned int bl_num;
		unsigned int bl_size_ui;
		float bl_size_f;
	};

	struct InterVertexData
	{
		unsigned int id;
		pwVertex vertex;
	};

	//typedef boost::adjacency_list<boost::vecS,
	//	boost::vecS, boost::undirectedS,
	//	InterVertexData, InterEdgeData> InterGraph;
	class InterGraph :
		public boost::adjacency_list<boost::vecS,
		boost::vecS, boost::undirectedS,
		InterVertexData, InterEdgeData>
	{
	public:
		size_t index;
	};
	typedef InterGraph::vertex_descriptor InterVert;
	typedef InterGraph::edge_descriptor InterEdge;
	typedef boost::graph_traits<InterGraph>::adjacency_iterator InterAdjIter;
	typedef boost::graph_traits<InterGraph>::edge_iterator InterEdgeIter;

	typedef std::vector<pwCell> CellBin;
	typedef CellBin::iterator CellBinIter;

	class Vertex
	{
	public:
		Vertex(unsigned int id) :
			m_id(id), m_size_ui(0), m_size_f(0.0f)
		{}
		~Vertex() {};

		unsigned int Id();
		InterVert GetInterVert(InterGraph& graph);
		void SetInterVert(InterGraph& graph, InterVert inter_vert);

		void SetCenter(FLIVR::Point &center);
		void SetSizeUi(unsigned int size_ui);
		void SetSizeF(float size_f);

		FLIVR::Point &GetCenter();
		unsigned int GetSizeUi();
		float GetSizeF();

		//cells
		size_t GetCellNum();
		void AddCell(pCell &cell, bool inc=false);
		CellBinIter GetCellsBegin();
		CellBinIter GetCellsEnd();

	private:
		unsigned int m_id;
		FLIVR::Point m_center;
		unsigned int m_size_ui;
		float m_size_f;
		typedef boost::unordered_map<unsigned int, InterVert> InterVertList;
		typedef boost::unordered_map<unsigned int, InterVert>::iterator InterVertListIter;
		InterVertList m_inter_verts;
		CellBin m_cells;//children
	};

	inline unsigned int Vertex::Id()
	{
		return m_id;
	}

	inline InterVert Vertex::GetInterVert(InterGraph& graph)
	{
		unsigned int key = graph.index;
		InterVertListIter iter = m_inter_verts.find(key);
		if (iter != m_inter_verts.end())
			return iter->second;
		else
			return InterGraph::null_vertex();
	}

	inline void Vertex::SetInterVert(InterGraph& graph,
		InterVert inter_vert)
	{
		unsigned int key = graph.index;
		InterVertListIter iter = m_inter_verts.find(key);
		if (iter != m_inter_verts.end())
			iter->second = inter_vert;
		else
			m_inter_verts.insert(
				std::pair<unsigned int, InterVert>(key, inter_vert));
	}

	inline void Vertex::SetCenter(FLIVR::Point &center)
	{
		m_center = center;
	}

	inline void Vertex::SetSizeUi(unsigned int size_ui)
	{
		m_size_ui = size_ui;
	}

	inline void Vertex::SetSizeF(float size_f)
	{
		m_size_f = size_f;
	}

	inline size_t Vertex::GetCellNum()
	{
		return m_cells.size();
	}

	inline void Vertex::AddCell(pCell &cell, bool inc)
	{
		if (inc)
		{
			m_size_ui += cell->GetSizeUi();
			m_size_f += cell->GetSizeF();
		}
		m_cells.push_back(pwCell(cell));
	}

	inline CellBinIter Vertex::GetCellsBegin()
	{
		return m_cells.begin();
	}

	inline CellBinIter Vertex::GetCellsEnd()
	{
		return m_cells.end();
	}

	inline FLIVR::Point &Vertex::GetCenter()
	{
		return m_center;
	}

	inline unsigned int Vertex::GetSizeUi()
	{
		return m_size_ui;
	}

	inline float Vertex::GetSizeF()
	{
		return m_size_f;
	}

}//namespace FL

#endif//FL_Vertex_h