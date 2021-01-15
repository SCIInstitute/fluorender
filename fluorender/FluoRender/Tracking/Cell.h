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
#ifndef FL_Cell_h
#define FL_Cell_h

#include <FLIVR/Point.h>
#include <FLIVR/BBox.h>
#include <FLIVR/Color.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <Distance/Pca.h>

namespace FL
{
	class Vertex;
	typedef boost::shared_ptr<Vertex> Verp;
	typedef boost::weak_ptr<Vertex> Verw;
	class Cell;
	typedef boost::shared_ptr<Cell> Celp;
	typedef boost::weak_ptr<Cell> Celw;

	struct IntraEdgeData
	{
		//contact size
		unsigned int size_ui;//voxel count
		float size_f;//voxel count weighted by intensity
		//distance
		float dist_v;//distance calculated from voxel grid
		float dist_s;//distance calculated from xyz spatial coordinates
	};

	struct IntraCellData
	{
		unsigned int id;
		Celw cell;
	};

	typedef boost::adjacency_list<boost::listS,
		boost::listS, boost::undirectedS,
		IntraCellData, IntraEdgeData> IntraGraph;
	typedef IntraGraph::vertex_descriptor IntraVert;
	typedef IntraGraph::edge_descriptor IntraEdge;
	typedef boost::graph_traits<IntraGraph>::adjacency_iterator IntraAdjIter;
	typedef boost::graph_traits<IntraGraph>::edge_iterator IntraEdgeIter;

	class Cell
	{
	public:
		Cell(unsigned int id) :
			m_id(id), m_brick_id(0),
			m_use_d(true), m_size_ui(0), m_size_d(0),
			m_ext_ui(0), m_ext_d(0),
			m_mean(0), m_int2(0), m_std(0),
			m_min(0), m_max(0),
			m_distp(0),
			m_count0(0), m_count1(0),
			m_intra_vert(IntraGraph::null_vertex())
		{}
		Cell(unsigned int id, unsigned int brick_id) :
			m_id(id), m_brick_id(brick_id),
			m_use_d(true), m_size_ui(0), m_size_d(0),
			m_ext_ui(0), m_ext_d(0),
			m_mean(0), m_int2(0), m_std(0),
			m_min(0), m_max(0),
			m_distp(0),
			m_count0(0), m_count1(0),
			m_intra_vert(IntraGraph::null_vertex())
		{}
		Cell(unsigned int id, unsigned int brick_id,
			unsigned int size_ui, double size_d,
			unsigned int ext_ui, double ext_d,
			unsigned int count0, unsigned int count1,
			FLIVR::Point &center, FLIVR::BBox &box):
			m_id(id), m_brick_id(brick_id),
			m_use_d(true), m_size_ui(size_ui), m_size_d(size_d),
			m_ext_ui(ext_ui), m_ext_d(ext_d),
			m_mean(0), m_int2(0), m_std(0),
			m_min(0), m_max(0),
			m_distp(0),
			m_count0(count0), m_count1(count1),
			m_center(center), m_box(box),
			m_intra_vert(IntraGraph::null_vertex())
		{}
		~Cell() {}

		void SetEId(unsigned long long eid)
		{
			m_id = eid << 32 >> 32;
			m_brick_id = eid >> 32;
		}
		unsigned int Id();
		unsigned int BrickId();
		unsigned long long GetEId();
		IntraVert GetIntraVert();
		void SetIntraVert(IntraVert intra_vert);
		void Set(Celp &celp);
		void Inc(size_t i, size_t j, size_t k, double value);
		void Inc(Celp &celp);
		void Inc();
		void IncExt(double value);
		void AddVertex(Verp &vertex);
		Verw GetVertex();
		unsigned int GetVertexId();

		//get
		//size
		double GetSize();
		unsigned int GetSizeUi();
		double GetSizeD();
		double GetExt();
		unsigned int GetExtUi();
		double GetExtD();
		//distribution
		double GetMean();
		double GetStd();
		double GetMin();
		double GetMax();
		//distance
		double GetDistp();
		//coords
		FLIVR::Point &GetCenter();
		FLIVR::BBox &GetBox();
		FLIVR::Point &GetProjp();
		//colocalization
		unsigned int GetCoSizeUi(size_t i);
		double GetCoSizeD(size_t i);
		//count
		unsigned int GetCount0();
		unsigned int GetCount1();

		//set
		void SetBrickId(unsigned int id);
		void SetUseD(bool val);
		void SetSizeUi(unsigned int size);
		void SetSizeD(double size);
		void SetExtUi(unsigned int size);
		void SetExtD(double size);
		void SetCount0(unsigned int val);
		void SetCount1(unsigned int val);
		void SetCenter(const FLIVR::Point &center);
		void SetBox(const FLIVR::BBox &box);

	private:
		unsigned int m_id;
		unsigned int m_brick_id;
		//size
		bool m_use_d;
		unsigned int m_size_ui;
		double m_size_d;
		//external size
		unsigned int m_ext_ui;
		float m_ext_d;
		//distribution
		double m_mean;
		double m_int2;//intensity squared
		double m_std;//standard deviation
		double m_min;
		double m_max;
		//distance
		double m_distp;//distance to a point
		//coords
		FLIVR::Point m_center;
		FLIVR::BBox m_box;
		FLIVR::Point m_prjp;//projected point
		//colocalization
		std::vector<unsigned int> m_size_ui_list;
		std::vector<double> m_size_d_list;
		//shape
		Pca m_pca;
		//uncertainty
		unsigned int m_count0;
		unsigned int m_count1;
		//vertex (parent group of cells)
		IntraVert m_intra_vert;
		Verw m_vertex;//parent
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

	inline IntraVert Cell::GetIntraVert()
	{
		return m_intra_vert;
	}

	inline void Cell::SetIntraVert(IntraVert intra_vert)
	{
		m_intra_vert = intra_vert;
	}

	inline void Cell::Set(Celp &celp)
	{
		m_center = celp->GetCenter();
		m_size_ui = celp->GetSizeUi();
		m_size_d = celp->GetSizeD();
		m_box = celp->GetBox();
	}

	inline void Cell::Inc(size_t i, size_t j, size_t k, double value)
	{
		m_center = FLIVR::Point(
			(m_center*m_size_ui + FLIVR::Point(double(i),
			double(j), double(k))) / (m_size_ui + 1));
		m_size_ui++;
		m_size_d += value;
		FLIVR::Point p(i, j, k);
		m_box.extend(p);
	}

	inline void Cell::Inc(Celp &celp)
	{
		m_center = FLIVR::Point(
			(m_center * m_size_ui + celp->GetCenter() *
			celp->GetSizeUi()) / (m_size_ui +
			celp->GetSizeUi()));
		m_size_ui += celp->GetSizeUi();
		m_size_d += celp->GetSizeD();
		m_box.extend(celp->GetBox());
	}

	inline void Cell::Inc()
	{
		m_size_ui++;
	}

	inline void Cell::IncExt(double value)
	{
		m_ext_ui++;
		m_ext_d += value;
	}

	inline void Cell::AddVertex(Verp &vertex)
	{
		m_vertex = vertex;
	}

	inline Verw Cell::GetVertex()
	{
		return m_vertex;
	}

	inline double Cell::GetSize()
	{
		if (m_use_d)
			return GetSizeD();
		else
			return GetSizeUi();
	}

	inline unsigned int Cell::GetSizeUi()
	{
		return m_size_ui;
	}

	inline double Cell::GetSizeD()
	{
		return m_size_d;
	}

	inline double Cell::GetExt()
	{
		if (m_use_d)
			return GetExtD();
		else
			return GetExtUi();
	}

	inline unsigned int Cell::GetExtUi()
	{
		return m_ext_ui;
	}

	inline double Cell::GetExtD()
	{
		return m_ext_d;
	}

	inline double Cell::GetMean()
	{
		return m_mean;
	}

	inline double Cell::GetStd()
	{
		return m_std;
	}

	inline double Cell::GetMin()
	{
		return m_min;
	}

	inline double Cell::GetMax()
	{
		return m_max;
	}

	inline double Cell::GetDistp()
	{
		return m_distp;
	}

	inline FLIVR::Point &Cell::GetCenter()
	{
		return m_center;
	}

	inline FLIVR::BBox &Cell::GetBox()
	{
		return m_box;
	}

	inline FLIVR::Point &Cell::GetProjp()
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

	inline void Cell::SetBrickId(unsigned int id)
	{
		m_brick_id = id;
	}

	inline void Cell::SetCenter(const FLIVR::Point &center)
	{
		m_center = center;
	}

	inline void Cell::SetBox(const FLIVR::BBox &box)
	{
		m_box = box;
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

}//namespace FL

#endif//FL_Cell_h