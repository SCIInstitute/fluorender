//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#include "CompGraph.h"

using namespace FL;

void CompGraph::ClearVisited()
{
	std::pair<CompVertexIter, CompVertexIter> vertices =
		boost::vertices(*this);
	for (auto iter = vertices.first; iter != vertices.second; ++iter)
		(*this)[*iter].visited = false;
}

bool CompGraph::GetLinkedComps(CompInfo& comp, CompUList& list)
{
	if (comp.v == CompGraph::null_vertex())
		return false;

	bool added = false;
	std::pair<CompAdjIter, CompAdjIter> adj_verts =
		boost::adjacent_vertices(comp.v, *this);
	for (auto iter = adj_verts.first; iter != adj_verts.second; ++iter)
	{
		auto v1 = *iter;
		unsigned int id = (*this)[v1].id;
		unsigned int brick_id = (*this)[v1].brick_id;
		auto l1 = list.find(GetKey(id, brick_id));
		if (l1 == list.end())
		{
			CompInfo info = *(*this)[v1].compinfo;
			list.insert(std::pair<unsigned long long, CompInfo>
				(GetKey(id, brick_id), info));
			(*this)[v1].visited = true;
			added = true;
			GetLinkedComps(info, list);
		}
	}

	return added;
}