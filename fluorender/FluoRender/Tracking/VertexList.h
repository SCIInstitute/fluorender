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
#ifndef FL_VertexList_h
#define FL_VertexList_h

#include "Vertex.h"
#include <boost/unordered_map.hpp>
#include <deque>

namespace FL
{
	typedef boost::unordered_map<unsigned int, pVertex> VertexList;
	typedef boost::unordered_map<unsigned int, pVertex>::iterator VertexListIter;
	typedef std::set<InterVert> VertVisitList;

	typedef std::deque<InterVert>::iterator PathIter;
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
		inline void push_back(const InterVert& v)
		{ m_path.push_back(v); }
		inline size_t size()
		{ return m_path.size(); }
		inline float get_size(int odd);
		float get_max_size()
		{ return m_max_size; }
		float get_odd_size()
		{ return m_odd_size; }
		float get_evn_size()
		{ return m_evn_size; }

		void flip();

	private:
		std::deque<InterVert> m_path;
		InterGraph &m_graph;
		float m_max_size;
		float m_odd_size;
		float m_evn_size;
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
			PathIter i1 = iter + 1;
			if (i1 == m_path.end())
				break;
			if (counter % 2 == odd)
			{
				std::pair<InterEdge, bool> edge =
					boost::edge(*iter, *i1, m_graph);
				if (edge.second)
					result += m_graph[edge.first].size_f;
			}
			counter++;
		}
		m_max_size = std::max(m_max_size, result);
		if (odd == 0)
			m_evn_size = result;
		else
			m_odd_size = result;
		return result;
	}

	inline void Path::flip()
	{
		bool flag;
		bool link;
		unsigned int l;
		for (PathIter iter = m_path.begin();
			iter != m_path.end(); ++iter)
		{
			PathIter i1 = iter + 1;
			if (i1 == m_path.end())
				break;
			std::pair<InterEdge, bool> edge =
				boost::edge(*iter, *i1, m_graph);
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
				}
			}
		}
	}

}//namespace FL

#endif//FL_VertexList_h