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
#ifndef FL_Cell_h
#define FL_Cell_h

#include <Point.h>
#include <Color.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>

namespace FL
{
	class Vertex;
	typedef boost::shared_ptr<Vertex> pVertex;
	typedef boost::weak_ptr<Vertex> pwVertex;
	class Cell;
	typedef boost::shared_ptr<Cell> pCell;
	typedef boost::weak_ptr<Cell> pwCell;

	struct IntraEdgeData
	{
		unsigned int size_ui;
		float size_f;
	};

	struct IntraCellData
	{
		unsigned int id;
		pwCell cell;
	};

	typedef boost::adjacency_list<boost::vecS,
		boost::vecS, boost::undirectedS,
		IntraCellData, IntraEdgeData> IntraGraph;
	typedef IntraGraph::vertex_descriptor IntraVert;
	typedef IntraGraph::edge_descriptor IntraEdge;
	typedef boost::graph_traits<IntraGraph>::adjacency_iterator IntraAdjIter;
	typedef boost::graph_traits<IntraGraph>::edge_iterator IntraEdgeIter;

	class Cell
	{
	public:
		Cell(unsigned int id) :
			m_id(id), m_size_ui(0), m_size_f(0.0f),
			m_external_ui(0), m_external_f(0.0f),
			m_intra_vert(IntraGraph::null_vertex())
		{}
		~Cell() {}

		unsigned int Id();
		IntraVert GetIntraVert();
		void SetIntraVert(IntraVert intra_vert);
		void Inc(size_t i, size_t j, size_t k, float value);
		void Inc();
		void IncExternal(float value);
		void AddVertex(pVertex &vertex);
		pwVertex GetVertex();

		//get
		FLIVR::Point &GetCenter();
		unsigned int GetSizeUi();
		float GetSizeF();
		unsigned int GetExternalUi();
		float GetExternalF();
		//set
		void SetCenter(FLIVR::Point &center);
		void SetSizeUi(unsigned int size_ui);
		void SetSizeF(float size_f);
		void SetExternalUi(unsigned int external_ui);
		void SetExternalF(float external_f);

	private:
		unsigned int m_id;
		FLIVR::Point m_center;
		//size
		unsigned int m_size_ui;
		float m_size_f;
		//external size
		unsigned int m_external_ui;
		float m_external_f;
		IntraVert m_intra_vert;
		pwVertex m_vertex;//parent
	};

	inline unsigned int Cell::Id()
	{
		return m_id;
	}

	inline IntraVert Cell::GetIntraVert()
	{
		return m_intra_vert;
	}

	inline void Cell::SetIntraVert(IntraVert intra_vert)
	{
		m_intra_vert = intra_vert;
	}

	inline void Cell::Inc(size_t i, size_t j, size_t k, float value)
	{
		m_center = FLIVR::Point(
			(m_center*m_size_ui + FLIVR::Point(double(i),
			double(j), double(k))) / (m_size_ui + 1));
		m_size_ui++;
		m_size_f += value;
	}

	inline void Cell::Inc()
	{
		m_size_ui++;
	}

	inline void Cell::IncExternal(float value)
	{
		m_external_ui++;
		m_external_f += value;
	}

	inline void Cell::AddVertex(pVertex &vertex)
	{
		m_vertex = vertex;
	}

	inline pwVertex Cell::GetVertex()
	{
		return m_vertex;
	}

	inline FLIVR::Point &Cell::GetCenter()
	{
		return m_center;
	}

	inline unsigned int Cell::GetSizeUi()
	{
		return m_size_ui;
	}

	inline float Cell::GetSizeF()
	{
		return m_size_f;
	}

	inline unsigned int Cell::GetExternalUi()
	{
		return m_external_ui;
	}

	inline float Cell::GetExternalF()
	{
		return m_external_f;
	}

	inline void Cell::SetCenter(FLIVR::Point &center)
	{
		m_center = center;
	}

	inline void Cell::SetSizeUi(unsigned int size_ui)
	{
		m_size_ui = size_ui;
	}

	inline void Cell::SetSizeF(float size_f)
	{
		m_size_f = size_f;
	}

	inline void Cell::SetExternalUi(unsigned int external_ui)
	{
		m_external_ui = external_ui;
	}

	inline void Cell::SetExternalF(float external_f)
	{
		m_external_f = external_f;
	}

}//namespace FL

#endif//FL_Cell_h