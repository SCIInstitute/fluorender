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

	class Path : public std::deque<InterVert>
	{
	public:
		inline float get_size(InterGraph &graph, int odd);
	};
	typedef std::deque<InterVert>::iterator PathIter;
	typedef std::vector<Path> PathList;
	typedef std::vector<Path>::iterator PathListIter;

	inline float Path::get_size(InterGraph &graph, int odd)
	{
		unsigned int counter = 0;
		float result = 0.0f;
		for (PathIter iter = this->begin();
			iter != this->end(); ++iter)
		{
			PathIter i1 = iter + 1;
			if (i1 == this->end())
				break;
			if (counter % 2 == odd)
			{
				std::pair<InterEdge, bool> edge =
					boost::edge(*iter, *i1, graph);
				if (edge.second)
					result += graph[edge.first].size_f;
			}
			counter++;
		}
		return result;
	}
}//namespace FL

#endif//FL_VertexList_h