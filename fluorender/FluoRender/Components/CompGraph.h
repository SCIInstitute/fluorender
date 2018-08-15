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
#ifndef FL_CompGraph_h
#define FL_CompGraph_h

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <FLIVR/Point.h>
#include <FLIVR/Color.h>

namespace FL
{
	struct CompInfo;
	typedef boost::shared_ptr<CompInfo> pCompInfo;
	typedef boost::weak_ptr<CompInfo> pwCompInfo;

	typedef boost::unordered_map<unsigned int, pCompInfo> CompListBrick;
	typedef CompListBrick::iterator CompListBrickIter;

	class CompList : public std::map<unsigned long long, pCompInfo>
	{
	public:
		unsigned int min;
		unsigned int max;
	};
	typedef CompList::iterator CompListIter;

	struct CompEdgeData
	{
		//overlapping size
		unsigned int size_ui;
	};

	struct CompNodeData
	{
		unsigned int id;
		unsigned int brick_id;
		bool visited;
		pwCompInfo compinfo;
	};

	class CompGraph :
		public boost::adjacency_list<boost::listS,
		boost::listS, boost::undirectedS,
		CompNodeData, CompEdgeData>
	{
	public:
		bool Visited(pCompInfo &comp);
		void ClearVisited();
		void LinkComps(pCompInfo &comp1, pCompInfo &comp2);
		bool GetLinkedComps(pCompInfo& comp, CompList& list, unsigned int size_limit=0);
		bool GetLinkedComps(CompList& list_in, CompList& list, unsigned int size_limit = 0);
	private:
		unsigned long long GetKey(unsigned int id, unsigned int brick_id)
		{
			unsigned long long temp = brick_id;
			return (temp << 32) | id;
		}
	};
	typedef CompGraph::vertex_descriptor CompVert;
	typedef CompGraph::edge_descriptor CompEdge;
	typedef boost::graph_traits<CompGraph>::vertex_iterator CompVertexIter;
	typedef boost::graph_traits<CompGraph>::edge_iterator CompEdgeIter;
	typedef boost::graph_traits<CompGraph>::adjacency_iterator CompAdjIter;

	struct CompInfo
	{
		unsigned int id;
		unsigned int alt_id;//altered id to make color consistent
		unsigned int brick_id;
		unsigned int sumi;
		double sumd;
		unsigned int ext_sumi;
		double ext_sumd;
		double mean;
		double var;
		double m2;
		double min;
		double max;
		FLIVR::Point pos;
		std::vector<unsigned int> cosumi;
		std::vector<double> cosumd;
		CompVert v;

		CompInfo()
		{
			v = CompGraph::null_vertex();
		}
		CompInfo(unsigned int _id,
			unsigned int _bid)
			:id(_id), brick_id(_bid)
		{
			v = CompGraph::null_vertex();
		}

		bool operator<(const CompInfo &info2) const
		{
			return (brick_id == info2.brick_id)?
				(id < info2.id) : (brick_id < info2.brick_id);
		}
		bool operator>(const CompInfo &info2) const
		{
			return (brick_id == info2.brick_id) ?
				(id > info2.id) : (brick_id > info2.brick_id);
		}
		bool operator==(const CompInfo &info2) const
		{
			return (brick_id == info2.brick_id) &&
				(id == info2.id);
		}
	};

	inline bool CompGraph::Visited(pCompInfo &comp)
	{
		if (comp->v != CompGraph::null_vertex())
			return (*this)[comp->v].visited;
		else
			return false;
	}
	
	inline void CompGraph::LinkComps(pCompInfo &comp1, pCompInfo &comp2)
	{
		CompVert v1 = comp1->v;
		CompVert v2 = comp2->v;
		if (v1 == CompGraph::null_vertex())
		{
			v1 = boost::add_vertex(*this);
			comp1->v = v1;
			(*this)[v1].id = comp1->id;
			(*this)[v1].brick_id = comp1->brick_id;
			(*this)[v1].compinfo = comp1;
		}
		if (v2 == CompGraph::null_vertex())
		{
			v2 = boost::add_vertex(*this);
			comp2->v = v2;
			(*this)[v2].id = comp2->id;
			(*this)[v2].brick_id = comp2->brick_id;
			(*this)[v2].compinfo = comp2;
		}

		std::pair<CompEdge, bool> edge = boost::edge(v1, v2, *this);
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

}//namespace FL

#endif//FL_CompGraph_h