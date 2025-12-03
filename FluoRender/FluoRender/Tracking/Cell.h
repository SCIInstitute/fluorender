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
#ifndef FL_Cell_h
#define FL_Cell_h

#include <Point.h>
#include <BBox.h>
#include <Color.h>
#include <Pca.h>
#include <memory>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>

namespace flrd
{
	class Vertex;
	typedef std::shared_ptr<Vertex> Verp;
	typedef std::weak_ptr<Vertex> Verw;
	class Cell;
	typedef std::shared_ptr<Cell> Celp;
	typedef std::weak_ptr<Cell> Celw;

	class CelpList : public std::unordered_map<unsigned long long, Celp>
	{
	public:
		unsigned int min;
		unsigned int max;
		//spatial scale
		double sx;
		double sy;
		double sz;

		CelpList():
			min(0), max(0),
			sx(1), sy(1), sz(1)
		{}

		bool FindBrick(unsigned int id)
		{
			for (auto it = begin();
				it != end(); ++it)
			{
				if (id == (it->first >> 32))
					return true;
			}
			return false;
		}
	};
	typedef CelpList::iterator CelpListIter;

	struct CelEdgeNode
	{
		//contact size
		unsigned int size_ui;//voxel count
		double size_d;//voxel count weighted by intensity
		//distance
		double dist_v;//distance calculated from voxel grid
		double dist_s;//distance calculated from xyz spatial coordinates
	};

	struct CelNode
	{
		unsigned int id;
		unsigned int brick_id;
		bool visited;
		Celw cell;
	};

	//typedef boost::adjacency_list<boost::listS,
	//	boost::listS, boost::undirectedS,
	//	CelNode, CelEdgeNode> CellGraph;
	class CellGraph : public boost::adjacency_list<boost::listS,
		boost::listS, boost::undirectedS,
		CelNode, CelEdgeNode>
	{
	public:
		bool Visited(Celp &celp);
		void ClearVisited();
		void LinkComps(Celp &celp1, Celp &cell2);
		bool GetLinkedComps(Celp& celp, CelpList& list, unsigned int size_limit = 5);
		bool GetLinkedComps(CelpList& list_in, CelpList& list, unsigned int size_limit = 5);
	};
	typedef CellGraph::vertex_descriptor CelVrtx;
	typedef CellGraph::edge_descriptor CelEdge;
	typedef boost::graph_traits<CellGraph>::vertex_iterator CelVrtIter;
	typedef boost::graph_traits<CellGraph>::adjacency_iterator CelAdjIter;
	typedef boost::graph_traits<CellGraph>::edge_iterator CelEdgeIter;

	class Cell
	{
	public:
		Cell(unsigned int id) :
			m_id(id), m_brick_id(0),
			m_use_d(true), m_size_ui(0), m_size_d(0),
			m_ext_ui(0), m_ext_d(0),
			m_calc(false), m_mean(0), m_int2(0), m_std(0),
			m_min(0), m_max(0),
			m_distp(0),
			m_count0(0), m_count1(0),
			m_celvrtx(CellGraph::null_vertex())
		{}
		Cell(unsigned int id, unsigned int brick_id) :
			m_id(id), m_brick_id(brick_id),
			m_use_d(true), m_size_ui(0), m_size_d(0),
			m_ext_ui(0), m_ext_d(0),
			m_calc(false), m_mean(0), m_int2(0), m_std(0),
			m_min(0), m_max(0),
			m_distp(0),
			m_count0(0), m_count1(0),
			m_celvrtx(CellGraph::null_vertex())
		{}
		Cell(unsigned int id, unsigned int brick_id,
			unsigned int size_ui, double size_d, unsigned int ext_ui,
			const fluo::Point& center,
			const fluo::Vector& scale) :
			m_id(id), m_brick_id(brick_id),
			m_use_d(true), m_size_ui(size_ui), m_size_d(size_d),
			m_ext_ui(ext_ui), m_ext_d(size_d * ext_ui),
			m_calc(false), m_mean(0), m_int2(size_d * size_d), m_std(0),
			m_min(size_d), m_max(size_d),
			m_distp(0),
			m_count0(0), m_count1(0),
			m_celvrtx(CellGraph::null_vertex())
		{
			m_center = center;
			m_pos = m_center;
			m_box.extend(m_center);
			m_pca.AddPointScale(m_center, scale);
		}
		Cell(unsigned int id, unsigned int brick_id,
			unsigned int size_ui, double size_d, unsigned int ext_ui,
			const fluo::Point& center,
			const fluo::Vector& scale,
			std::vector<unsigned int> &sumi, std::vector<double> &sumd) :
			m_id(id), m_brick_id(brick_id),
			m_use_d(true), m_size_ui(size_ui), m_size_d(size_d),
			m_ext_ui(ext_ui), m_ext_d(size_d * ext_ui),
			m_calc(false), m_mean(0), m_int2(size_d * size_d), m_std(0),
			m_min(size_d), m_max(size_d),
			m_distp(0),
			m_count0(0), m_count1(0),
			m_celvrtx(CellGraph::null_vertex())
		{
			m_center = center;
			m_pos = m_center;
			m_box.extend(m_center);
			m_pca.AddPointScale(m_center, scale);
			m_size_ui_list = sumi;
			m_size_d_list = sumd;
		}
		~Cell() {}

		void SetEId(unsigned long long eid)
		{
			m_id = eid << 32 >> 32;
			m_brick_id = eid >> 32;
		}
		void SetId(unsigned int id)
		{
			m_id = id;
		}
		unsigned int Id();
		unsigned int BrickId();
		unsigned long long GetEId();
		static unsigned long long GetKey(unsigned int id, unsigned int bid);
		CelVrtx GetCelVrtx();
		void SetCelVrtx(CelVrtx intra_vert);
		void Set(Celp &celp);
		void Inc(const fluo::Point& p, double value);
		void Inc(Celp &celp);
		void Inc();
		void IncExt(double value);
		void Inc(unsigned int size_ui, double size_d, unsigned int ext_ui,
			const fluo::Point& p,
			const fluo::Vector& scale);
		void Inc(unsigned int size_ui, double size_d, unsigned int ext_ui,
			const fluo::Point& p,
			const fluo::Vector& scale,
			std::vector<unsigned int> &sumi, std::vector<double> &sumd);
		void Calc();//calculate mean etc
		void AddVertex(Verp &vertex);
		Verw GetVertex();
		unsigned int GetVertexId();
		Pca &GetPca();

		//get
		//size
		double GetSize(double si = 1);
		unsigned int GetSizeUi();
		double GetSizeD(double si=1);
		double GetExt(double si = 1);
		unsigned int GetExtUi();
		double GetExtD(double si=1);
		//distribution
		double GetMean(double si=1);
		double GetStd(double si=1);
		double GetMin(double si=1);
		double GetMax(double si=1);
		//distance
		double GetDistp();
		//coords
		void SetCalc(bool bval = true) { m_calc = bval; }
		fluo::Point &GetCenter();
		fluo::Point GetCenter(const fluo::Vector& scale);
		fluo::BBox &GetBox();
		fluo::BBox GetBox(const fluo::Vector& scale);
		fluo::Point &GetProjp();
		//flvr::Point GetProjp(double sx, double sy, double sz);
		//colocalization
		unsigned int GetCoSizeUi(size_t i);
		double GetCoSizeD(size_t i);
		//count
		unsigned int GetCount0();
		unsigned int GetCount1();

		//set
		void Copy(Cell &cell, bool copy_id = false);
		void Copy(Celp &celp, bool copy_id = false);
		void SetBrickId(unsigned int id);
		void SetUseD(bool val);
		void SetSizeUi(unsigned int size);
		void SetSizeD(double size);
		void SetExtUi(unsigned int size);
		void SetExtD(double size);
		void SetCount0(unsigned int val);
		void SetCount1(unsigned int val);
		void SetCenter(const fluo::Point &center);
		void SetBox(const fluo::BBox &box);
		void SetProjp(fluo::Point &p);
		void SetDistp(double dist);

	private:
		unsigned int m_id;
		unsigned int m_brick_id;
		//size
		bool m_use_d;
		unsigned int m_size_ui;
		double m_size_d;
		//external size
		unsigned int m_ext_ui;
		double m_ext_d;
		//distribution
		bool m_calc;//if mean etc are calculated
		double m_mean;//averaged int
		double m_int2;//intensity squared accu
		double m_std;//standard deviation
		double m_min;
		double m_max;
		//distance
		double m_distp;//distance to a point
		//coords
		fluo::Point m_center;
		fluo::Point m_pos;//point position accumulation
		fluo::BBox m_box;
		fluo::Point m_prjp;//projected point
		//colocalization
		std::vector<unsigned int> m_size_ui_list;
		std::vector<double> m_size_d_list;
		//shape
		Pca m_pca;
		//uncertainty
		unsigned int m_count0;
		unsigned int m_count1;
		//vertex (parent group of cells)
		CelVrtx m_celvrtx;
		Verw m_vertex;//parent

		friend class CellGraph;
	};

	inline unsigned int Cell::Id()
	{
		return m_id;
	}

	inline unsigned int Cell::BrickId()
	{
		return m_brick_id;
	}

	inline unsigned long long Cell::GetEId()
	{
		unsigned long long temp = m_brick_id;
		return (temp << 32) | m_id;
	}

	inline CelVrtx Cell::GetCelVrtx()
	{
		return m_celvrtx;
	}

	inline void Cell::SetCelVrtx(CelVrtx intra_vert)
	{
		m_celvrtx = intra_vert;
	}

	inline void Cell::Set(Celp &celp)
	{
		m_center = celp->GetCenter();
		m_size_ui = celp->GetSizeUi();
		m_size_d = celp->GetSizeD();
		m_box = celp->GetBox();
		m_calc = true;
	}

	inline void Cell::Inc(const fluo::Point& p, double value)
	{
		m_size_ui++;
		m_size_d += value;

		m_int2 += value * value;
		m_min = std::min(m_min, value);
		m_max = std::max(m_max, value);

		m_pos += p;
		m_box.extend(p);

		m_calc = false;
	}

	inline void Cell::Inc(Celp &celp)
	{
		m_size_ui += celp->m_size_ui;
		m_size_d += celp->m_size_d;
		m_ext_ui += celp->m_ext_ui;
		m_ext_d += celp->m_ext_d;

		m_int2 += celp->m_int2;
		m_min = std::min(m_min, celp->m_min);
		m_max = std::max(m_max, celp->m_max);

		m_pos += celp->m_pos;
		m_box.extend(celp->m_box);
		m_pca.Add(celp->m_pca);

		m_size_ui_list.insert(m_size_ui_list.end(),
			celp->m_size_ui_list.begin(), celp->m_size_ui_list.end());
		m_size_d_list.insert(m_size_d_list.end(),
			celp->m_size_d_list.begin(), celp->m_size_d_list.end());

		m_calc = false;
	}

	inline void Cell::Inc()
	{
		m_size_ui++;
		m_calc = false;
	}

	inline void Cell::IncExt(double value)
	{
		m_ext_ui++;
		m_ext_d += value;
		m_calc = false;
	}

	inline void Cell::Inc(unsigned int size_ui, double size_d, unsigned int ext_ui,
		const fluo::Point& p,
		const fluo::Vector& scale)
	{
		m_size_ui += size_ui;
		m_size_d += size_d;
		m_ext_ui += ext_ui;
		m_ext_d += ext_ui * size_d;

		m_int2 += size_d * size_d;
		m_min = std::min(m_min, size_d);
		m_max = std::max(m_max, size_d);

		m_pos += p;
		m_box.extend(p);
		m_pca.AddPointScale(p, scale);

		m_calc = false;
	}

	inline void Cell::Inc(unsigned int size_ui, double size_d, unsigned int ext_ui,
		const fluo::Point& p,
		const fluo::Vector& scale,
		std::vector<unsigned int> &sumi, std::vector<double> &sumd)
	{
		m_size_ui += size_ui;
		m_size_d += size_d;
		m_ext_ui += ext_ui;
		m_ext_d += ext_ui * size_d;

		m_int2 += size_d * size_d;
		m_min = std::min(m_min, size_d);
		m_max = std::max(m_max, size_d);

		m_pos += p;
		m_box.extend(p);
		m_pca.AddPointScale(p, scale);

		for (size_t i = 0; i < m_size_ui_list.size(); ++i)
			m_size_ui_list[i] += sumi[i];
		for (size_t i = 0; i < m_size_d_list.size(); ++i)
			m_size_d_list[i] += sumd[i];

		m_calc = false;
	}

	inline void Cell::Calc()
	{
		if (m_calc)
			return;

		m_mean = m_size_d / m_size_ui;
		m_std = std::sqrt((m_int2 + m_size_ui * m_mean * m_mean -
			2 * m_mean * m_size_d) / m_size_ui);
		m_center = m_pos / m_size_ui;

		m_calc = true;
	}

	inline void Cell::AddVertex(Verp &vertex)
	{
		m_vertex = vertex;
	}

	inline Verw Cell::GetVertex()
	{
		return m_vertex;
	}

	inline double Cell::GetSize(double si)
	{
		if (m_use_d)
			return GetSizeD(si);
		else
			return GetSizeUi();
	}

	inline unsigned int Cell::GetSizeUi()
	{
		return m_size_ui;
	}

	inline double Cell::GetSizeD(double si)
	{
		return m_size_d * si;
	}

	inline double Cell::GetExt(double si)
	{
		if (m_use_d)
			return GetExtD(si);
		else
			return GetExtUi();
	}

	inline unsigned int Cell::GetExtUi()
	{
		return m_ext_ui;
	}

	inline double Cell::GetExtD(double si)
	{
		return m_ext_d * si;
	}

	inline double Cell::GetMean(double si)
	{
		if (!m_calc)
			Calc();

		return m_mean * si;
	}

	inline double Cell::GetStd(double si)
	{
		if (!m_calc)
			Calc();

		return m_std * si;
	}

	inline double Cell::GetMin(double si)
	{
		return m_min * si;
	}

	inline double Cell::GetMax(double si)
	{
		return m_max * si;
	}

	inline double Cell::GetDistp()
	{
		return m_distp;
	}

	inline fluo::Point &Cell::GetCenter()
	{
		if (!m_calc)
			Calc();

		return m_center;
	}

	inline fluo::Point Cell::GetCenter(const fluo::Vector& scale)
	{
		if (!m_calc)
			Calc();

		auto p = m_center;
		p.scale(scale);
		return p;
	}

	inline fluo::BBox &Cell::GetBox()
	{
		return m_box;
	}

	inline fluo::BBox Cell::GetBox(const fluo::Vector& scale)
	{
		fluo::BBox box(m_box);
		box.scale(scale);
		return box;
	}

	inline fluo::Point &Cell::GetProjp()
	{
		return m_prjp;
	}

	inline unsigned int Cell::GetCoSizeUi(size_t i)
	{
		if (i < m_size_ui_list.size())
			return m_size_ui_list[i];
		return 0;
	}

	inline double Cell::GetCoSizeD(size_t i)
	{
		if (i < m_size_d_list.size())
			return m_size_d_list[i];
		return 0;
	}

	//count
	inline unsigned int Cell::GetCount0()
	{
		return m_count0;
	}

	inline unsigned int Cell::GetCount1()
	{
		return m_count1;
	}

	inline void Cell::Copy(Cell &cell, bool copy_id)
	{
		if (copy_id)
		{
			m_id = cell.m_id;
			m_brick_id = cell.m_brick_id;
		}

		m_size_ui = cell.m_size_ui;
		m_size_d = cell.m_size_d;
		m_ext_ui = cell.m_ext_ui;
		m_ext_d = cell.m_ext_d;

		m_calc = cell.m_calc;
		m_int2 = cell.m_int2;
		m_min = cell.m_min;
		m_max = cell.m_max;

		m_pos = cell.m_pos;
		m_center = cell.m_center;
		m_box = cell.m_box;
		m_pca.Add(cell.m_pca);
	}

	inline void Cell::Copy(Celp &celp, bool copy_id)
	{
		if (copy_id)
		{
			m_id = celp->m_id;
			m_brick_id = celp->m_brick_id;
		}

		m_size_ui = celp->m_size_ui;
		m_size_d = celp->m_size_d;
		m_ext_ui = celp->m_ext_ui;
		m_ext_d = celp->m_ext_d;

		m_calc = celp->m_calc;
		m_int2 = celp->m_int2;
		m_min = celp->m_min;
		m_max = celp->m_max;

		m_pos = celp->m_pos;
		m_center = celp->m_center;
		m_box = celp->m_box;
		m_pca.Add(celp->m_pca);
	}

	inline void Cell::SetBrickId(unsigned int id)
	{
		m_brick_id = id;
	}

	inline void Cell::SetCenter(const fluo::Point &center)
	{
		m_center = center;
	}

	inline void Cell::SetBox(const fluo::BBox &box)
	{
		m_box = box;
	}

	inline void Cell::SetProjp(fluo::Point &p)
	{
		m_prjp = p;
	}

	inline void Cell::SetDistp(double dist)
	{
		m_distp = dist;
	}

	inline void Cell::SetUseD(bool val)
	{
		m_use_d = val;
	}

	inline void Cell::SetSizeUi(unsigned int size)
	{
		m_size_ui = size;
	}

	inline void Cell::SetSizeD(double size)
	{
		m_size_d = size;
	}

	inline void Cell::SetExtUi(unsigned int size)
	{
		m_ext_ui = size;
	}

	inline void Cell::SetExtD(double size)
	{
		m_ext_d = size;
	}

	inline void Cell::SetCount0(unsigned int val)
	{
		m_count0 = val;
	}

	inline void Cell::SetCount1(unsigned int val)
	{
		m_count1 = val;
	}

	inline unsigned long long Cell::GetKey(unsigned int id, unsigned int bid)
	{
		unsigned long long temp = bid;
		return (temp << 32) | id;
	}

	inline bool CellGraph::Visited(Celp &celp)
	{
		if (celp->m_celvrtx != CellGraph::null_vertex())
			return (*this)[celp->m_celvrtx].visited;
		else
			return false;
	}

	inline void CellGraph::ClearVisited()
	{
		std::pair<CelVrtIter, CelVrtIter> verts =
			boost::vertices(*this);
		for (auto iter = verts.first; iter != verts.second; ++iter)
			(*this)[*iter].visited = false;
	}

	inline void CellGraph::LinkComps(Celp &celp1, Celp &celp2)
	{
		CelVrtx v1 = celp1->m_celvrtx;
		CelVrtx v2 = celp2->m_celvrtx;
		if (v1 == CellGraph::null_vertex())
		{
			v1 = boost::add_vertex(*this);
			celp1->m_celvrtx = v1;
			(*this)[v1].id = celp1->m_id;
			(*this)[v1].brick_id = celp1->m_brick_id;
			(*this)[v1].cell = celp1;
		}
		if (v2 == CellGraph::null_vertex())
		{
			v2 = boost::add_vertex(*this);
			celp2->m_celvrtx = v2;
			(*this)[v2].id = celp2->m_id;
			(*this)[v2].brick_id = celp2->m_brick_id;
			(*this)[v2].cell = celp2;
		}

		std::pair<CelEdge, bool> edge = boost::edge(v1, v2, *this);
		if (!edge.second)
		{
			edge = boost::add_edge(v1, v2, *this);
			(*this)[edge.first].size_ui = 1;
		}
		else
		{
			(*this)[edge.first].size_ui++;
		}
	}

	inline bool CellGraph::GetLinkedComps(Celp& celp, CelpList& list, unsigned int size_limit)
	{
		if (!celp)
			return false;

		auto v1 = celp->m_celvrtx;

		if (v1 == CellGraph::null_vertex())
			return false;

		if ((*this)[v1].visited)
			return false;
		else
		{
			list.insert(std::pair<unsigned long long, Celp>
				(celp->GetEId(), celp));
			(*this)[v1].visited = true;
		}

		std::pair<CelAdjIter, CelAdjIter> adj_verts =
			boost::adjacent_vertices(v1, *this);
		for (auto iter = adj_verts.first; iter != adj_verts.second; ++iter)
		{
			auto v2 = *iter;
			//check connecting size
			std::pair<CelEdge, bool> edge = boost::edge(v1, v2, *this);
			if (edge.second)
			{
				if ((*this)[edge.first].size_ui < size_limit)
					continue;
			}
			//
			unsigned int id = (*this)[v2].id;
			unsigned int brick_id = (*this)[v2].brick_id;
			auto l1 = list.find(Cell::GetKey(id, brick_id));
			if (l1 == list.end())
			{
				Celp info = (*this)[v2].cell.lock();
				GetLinkedComps(info, list, size_limit);
			}
		}

		return true;
	}

	inline bool CellGraph::GetLinkedComps(CelpList& list_in, CelpList& list, unsigned int size_limit)
	{
		ClearVisited();

		for (auto comp_iter = list_in.begin();
			comp_iter != list_in.end(); ++comp_iter)
		{
			auto comp = comp_iter->second;
			if (!GetLinkedComps(comp, list, size_limit))
			{
				list.insert(std::pair<unsigned long long, Celp>
					(comp_iter->first, comp));
			}
		}

		return !list.empty();
	}
}//namespace flrd

#endif//FL_Cell_h