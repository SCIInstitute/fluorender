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
#ifndef FL_Vertex_h
#define FL_Vertex_h

#include "Cell.h"
#include <vector>

namespace FL
{
	struct UncertainBin
	{
		unsigned int level;
		unsigned int count;

		UncertainBin() :level(0), count(0) {}
	};
	inline bool operator==(const UncertainBin& bin1, const UncertainBin& bin2)
	{ return bin1.level == bin2.level && bin1.count == bin2.count; }
	typedef std::map<unsigned int, UncertainBin> UncertainHist;
	typedef std::map<unsigned int, UncertainBin>::iterator UncertainHistIter;

	struct InterEdgeData
	{
		unsigned int size_ui;
		float size_f;
		float dist_f;
		unsigned int link;//0: not linked; 1: linked; 2: force linked; 3: force unlinked
		//count for changes
		unsigned int count;
	};

	struct InterVertexData
	{
		unsigned int id;
		unsigned int frame;
		//count for changes
		unsigned int count;
		pwVertex vertex;
		//for matching
		bool max_valid;
		float max_value;
	};

	class InterGraph :
		public boost::adjacency_list<boost::listS,
		boost::listS, boost::undirectedS,
		InterVertexData, InterEdgeData>
	{
	public:
		size_t index;
		size_t counter;
		UncertainBin major_bin[2];
	};
	typedef InterGraph::vertex_descriptor InterVert;
	typedef InterGraph::edge_descriptor InterEdge;
	typedef boost::graph_traits<InterGraph>::adjacency_iterator InterAdjIter;
	typedef boost::graph_traits<InterGraph>::edge_iterator InterEdgeIter;

	typedef std::vector<Celw> CellBin;
	typedef CellBin::iterator CellBinIter;

	typedef std::unordered_map<unsigned int, unsigned int> CellIDMap;
	typedef std::unordered_map<unsigned int, unsigned int>::iterator CellIDMapIter;

	class Vertex
	{
	public:
		Vertex(unsigned int id) :
			m_id(id), m_size_ui(0),
			m_size_f(0.0f), m_split(false)
		{}
		~Vertex() {};

		unsigned int Id();
		void Id(unsigned int);
		InterVert GetInterVert(InterGraph& graph);
		void SetInterVert(InterGraph& graph, InterVert inter_vert);
		bool GetRemovedFromGraph();
		size_t GetFrame(InterGraph& graph);
		void SetSplit(bool split = true);
		bool GetSplit();

		void SetCenter(FLIVR::Point &center);
		void SetSizeUi(unsigned int size_ui);
		void SetSizeF(float size_f);
		void Update();

		FLIVR::Point &GetCenter();
		unsigned int GetSizeUi();
		float GetSizeF();
		//bbox computed from cells
		FLIVR::BBox GetBox();

		//cells
		size_t GetCellNum();
		Celp GetCell(size_t idx);
		int FindCell(Celp &celp);
		void AddCell(Celp &celp, bool inc=false);
		void RemoveCell(Celp &celp);
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
		bool m_split;//true if em has been run already
	};

	inline unsigned int Cell::GetVertexId()
	{
		pVertex vertex = m_vertex.lock();
		if (vertex)
			return vertex->Id();
		else
			return 0;
	}

	inline unsigned int Vertex::Id()
	{
		return m_id;
	}

	inline void Vertex::Id(unsigned int id)
	{
		m_id = id;
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

	inline bool Vertex::GetRemovedFromGraph()
	{
		for (auto iter = m_inter_verts.begin();
			iter != m_inter_verts.end(); ++iter)
			if (iter->second != InterGraph::null_vertex())
				return false;
		return true;
	}

	inline size_t Vertex::GetFrame(InterGraph& graph)
	{
		unsigned int key = graph.index;
		InterVertListIter iter = m_inter_verts.find(key);
		if (iter != m_inter_verts.end())
			return graph[iter->second].frame;
		else
			return (size_t)-1;
	}

	inline bool Vertex::GetSplit()
	{
		return m_split;
	}

	inline void Vertex::SetSplit(bool split)
	{
		m_split = split;
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

	inline void Vertex::Update()
	{
		m_center = FLIVR::Point();
		m_size_ui = 0;
		m_size_f = 0.0f;

		if (m_cells.size() == 0)
			return;

		for (CellBinIter iter = m_cells.begin();
		iter != m_cells.end(); ++iter)
		{
			Celp celp = iter->lock();
			if (!celp)
				continue;
			m_center += celp->GetCenter();
			m_size_ui += celp->GetSizeUi();
			m_size_f += celp->GetSizeD();
		}
		m_center /= m_cells.size();
	}

	inline size_t Vertex::GetCellNum()
	{
		return m_cells.size();
	}

	inline Celp Vertex::GetCell(size_t idx)
	{
		if (idx >= m_cells.size())
			return nullptr;
		else
			return m_cells[idx].lock();
	}

	inline int Vertex::FindCell(Celp &celp)
	{
		for (size_t i = 0; i < m_cells.size(); ++i)
		{
			Celp celp0 = m_cells[i].lock();
			if (celp0 && celp0->Id() == celp->Id())
				return i;
		}
		return -1;
	}

	inline void Vertex::AddCell(Celp &celp, bool inc)
	{
		if (FindCell(celp) >= 0)
			return;

		if (inc)
		{
			m_size_ui += celp->GetSizeUi();
			m_size_f += celp->GetSizeD();
		}
		m_cells.push_back(Celw(celp));

		m_split = false;
	}

	inline void Vertex::RemoveCell(Celp &celp)
	{
		for (CellBinIter iter = m_cells.begin();
			iter != m_cells.end(); ++iter)
		{
			Celp celp0 = iter->lock();
			if (celp0 && celp0->Id() == celp->Id())
			{
				m_cells.erase(iter);
				m_split = false;
				return;
			}
		}
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

	inline FLIVR::BBox Vertex::GetBox()
	{
		FLIVR::BBox box;
		for (CellBinIter iter = m_cells.begin();
			iter != m_cells.end(); ++iter)
		{
			Celp celp = iter->lock();
			box.extend(celp->GetBox());
		}
		return box;
	}

}//namespace FL

#endif//FL_Vertex_h