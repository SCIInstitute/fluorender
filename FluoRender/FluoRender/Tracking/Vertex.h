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
#ifndef FL_Vertex_h
#define FL_Vertex_h

#include <Cell.h>
#include <vector>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>

namespace flrd
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

	struct EdgeNode
	{
		unsigned int size_ui;
		double size_d;
		double dist;
		unsigned int link;//0: not linked; 1: linked; 2: force linked; 3: force unlinked
		//count for changes
		unsigned int count;
	};

	struct VertNode
	{
		unsigned int id;
		unsigned int frame;
		//count for changes
		unsigned int count;
		Verw vertex;
		//for matching
		bool max_valid;
		float max_value;
	};

	class InterGraph :
		public boost::adjacency_list<boost::listS,
		boost::listS, boost::undirectedS,
		VertNode, EdgeNode>
	{
	public:
		size_t index;
		size_t counter;
		UncertainBin major_bin[2];
	};
	typedef InterGraph::vertex_descriptor Vrtx;
	typedef InterGraph::edge_descriptor Edge;
	typedef boost::graph_traits<InterGraph>::adjacency_iterator AdjIter;
	typedef boost::graph_traits<InterGraph>::edge_iterator EdgeIter;

	typedef std::vector<Celw> CellBin;
	typedef CellBin::iterator CellBinIter;

	typedef std::unordered_map<unsigned int, unsigned int> CellIDMap;
	typedef std::unordered_map<unsigned int, unsigned int>::iterator CellIDMapIter;

	class Vertex
	{
	public:
		Vertex(unsigned int id) :
			m_id(id), m_size_ui(0),
			m_size_d(0), m_split(false)
		{}
		~Vertex() {};

		unsigned int Id();
		void Id(unsigned int);
		Vrtx GetInterVert(InterGraph& graph);
		void SetInterVert(InterGraph& graph, Vrtx inter_vert);
		bool GetRemovedFromGraph();
		size_t GetFrame(InterGraph& graph);
		void SetSplit(bool split = true);
		bool GetSplit();

		void SetCenter(fluo::Point &center);
		void SetSizeUi(unsigned int size);
		void SetSizeD(double size);
		void Update();

		fluo::Point &GetCenter();
		unsigned int GetSizeUi();
		double GetSizeD();
		//bbox computed from cells
		fluo::BBox GetBox();

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
		fluo::Point m_center;
		unsigned int m_size_ui;
		double m_size_d;
		typedef std::unordered_map<unsigned int, Vrtx> InterVertList;
		typedef std::unordered_map<unsigned int, Vrtx>::iterator InterVertListIter;
		InterVertList m_inter_verts;
		CellBin m_cells;//children
		bool m_split;//true if em has been run already
	};


}//namespace flrd

#endif//FL_Vertex_h