/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef FL_VertexList_h
#define FL_VertexList_h

#include "Vertex.h"
#include <unordered_map>
#include <deque>
#include <iostream>

namespace flrd
{
	typedef std::unordered_map<unsigned int, Verp> VertexList;
	typedef std::unordered_map<unsigned int, Verp>::iterator VertexListIter;

	struct PathVert
	{
		Vrtx vert;
		bool edge_valid;
		float edge_value;
		float max_value;
		unsigned int link;
	};

	typedef std::set<Vrtx> VertVisitList;
	typedef std::deque<PathVert>::iterator PathIter;
	class Path
	{
	public:
		Path(InterGraph &graph)
			: m_graph(graph),
			m_max_size(0.0f),
			m_odd_size(0.0f),
			m_evn_size(0.0f) {}
		~Path() {}
		inline Path& operator=(const Path& path)
		{
			m_path = path.m_path;
			m_graph = path.m_graph;
			m_max_size = path.m_max_size;
			m_odd_size = path.m_odd_size;
			m_evn_size = path.m_evn_size;
			return *this;
		}
		inline PathIter begin()
		{ return m_path.begin(); }
		inline PathIter end()
		{ return m_path.end(); }
		inline void push_back(const PathVert& v)
		{ m_path.push_back(v); }
		inline size_t size()
		{ return m_path.size(); }
		inline PathVert& back()
		{ return m_path.back(); }
		inline PathVert& operator[](size_t n)
		{ return m_path[n]; }
		
		//overlap size
		inline float get_size(int odd);
		float get_max_size()
		{ return m_max_size; }
		float get_odd_size()
		{ return m_odd_size; }
		float get_evn_size()
		{ return m_evn_size; }

		//link count
		inline unsigned int get_count(int odd);
		unsigned int get_max_count()
		{ return m_max_count; }
		unsigned int get_odd_count()
		{ return m_odd_count; }
		unsigned int get_evn_count()
		{ return m_evn_count; }

		InterGraph &get_graph()
		{ return m_graph; }

		bool flip();
		bool unlink();

	private:
		std::deque<PathVert> m_path;
		InterGraph &m_graph;
		//overlap size
		float m_max_size;
		float m_odd_size;
		float m_evn_size;
		//link count
		unsigned int m_max_count;
		unsigned int m_odd_count;
		unsigned int m_evn_count;
	};
	typedef std::vector<Path> PathList;
	typedef std::vector<Path>::iterator PathListIter;

	inline float Path::get_size(int odd)
	{
		unsigned int counter = 0;
		float result = 0.0f;
		for (PathIter iter = m_path.begin();
			iter != m_path.end(); ++iter)
		{
			if (!iter->edge_valid)
				break;
			if (counter % 2 == odd)
				result += iter->edge_value;
			counter++;
		}
		m_max_size = std::max(m_max_size, result);
		if (odd == 0)
			m_evn_size = result;
		else
			m_odd_size = result;
		return result;
	}

	inline unsigned int Path::get_count(int odd)
	{
		unsigned int counter = 0;
		unsigned int result = 0;
		for (PathIter iter = m_path.begin();
			iter != m_path.end(); ++iter)
		{
			PathIter i1 = iter + 1;
			if (i1 == m_path.end())
				break;
			if (counter % 2 == odd)
			{
				std::pair<Edge, bool> edge =
					boost::edge(iter->vert, i1->vert, m_graph);
				if (edge.second)
					result += m_graph[edge.first].count;
			}
			counter++;
		}
		m_max_count = std::max(m_max_count, result);
		if (odd == 0)
			m_evn_count = result;
		else
			m_odd_count = result;
		return result;
	}

	inline bool Path::flip()
	{
		bool result = false;
		bool flag;
		bool link;
		unsigned int l;
		for (PathIter iter = m_path.begin();
			iter != m_path.end(); ++iter)
		{
			PathIter i1 = iter + 1;
			if (i1 == m_path.end())
				break;
			std::pair<Edge, bool> edge =
				boost::edge(iter->vert, i1->vert, m_graph);
			if (edge.second)
			{
				l = m_graph[edge.first].link;
				if (iter == m_path.begin())
					flag = l == 0 || l == 3;
				else
					flag = !flag;
				if (l == 2 || l == 3)
					continue;
				link = l == 1;
				if (link != flag)
				{
					m_graph[edge.first].link = flag ? 1 : 0;
					m_graph[edge.first].count++;
					m_graph[boost::source(edge.first, m_graph)].count++;
					m_graph[boost::target(edge.first, m_graph)].count++;
					result = true;
				}
			}
		}
		return result;
	}

	inline bool Path::unlink()
	{
		//return if it is unlinked
		PathIter iter = m_path.begin();
		PathIter i1 = iter + 1;
		if (i1 == m_path.end())
			return false;

		std::pair<Edge, bool> edge =
			boost::edge(iter->vert, i1->vert, m_graph);
		if (edge.second)
		{
			unsigned int l = m_graph[edge.first].link;
			if (l == 1)
			{
				m_graph[edge.first].link = 0;
				m_graph[edge.first].count++;
				m_graph[boost::source(edge.first, m_graph)].count++;
				m_graph[boost::target(edge.first, m_graph)].count++;

				return true;
			}
		}

		return false;
	}

	//output
	inline std::ostream& operator<<(std::ostream& os, Path& p)
	{
		InterGraph graph = p.get_graph();
		Verp vertex;
		for (auto iter = p.begin();
			iter != p.end(); ++iter)
		{
			//output vertex
			vertex = graph[iter->vert].vertex.lock();
			if (vertex)
			{
				os << "(Vertex: ";
				os << vertex->Id() << ", ";
				os << vertex->GetSizeD() << ", ";
				os << graph[iter->vert].count << ") ";
			}
			//output edge
			PathIter i1 = iter + 1;
			if (i1 != p.end())
			{
				std::pair<Edge, bool> edge =
					boost::edge(iter->vert, i1->vert, graph);
				if (edge.second)
				{
					os << "(Edge: ";
					os << graph[edge.first].size_d << ", ";
					os << graph[edge.first].dist << ", ";
					os << graph[edge.first].link << ", ";
					os << graph[edge.first].count << ") ";
				}
			}
		}
		os << "\n";
		return os;
	}

}//namespace flrd

#endif//FL_VertexList_h