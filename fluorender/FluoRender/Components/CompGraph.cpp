//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

bool CompGraph::GetLinkedComps(pCompInfo& comp, CompList& list, unsigned int size_limit)
{
	if (!comp)
		return false;

	auto v1 = comp->v;

	if (v1 == CompGraph::null_vertex())
		return false;

	if ((*this)[v1].visited)
		return false;
	else
	{
		list.insert(std::pair<unsigned long long, pCompInfo>
			(GetKey(comp->id, comp->brick_id), comp));
		(*this)[v1].visited = true;
	}

	std::pair<CompAdjIter, CompAdjIter> adj_verts =
		boost::adjacent_vertices(v1, *this);
	for (auto iter = adj_verts.first; iter != adj_verts.second; ++iter)
	{
		auto v2 = *iter;
		//check connecting size
		std::pair<CompEdge, bool> edge = boost::edge(v1, v2, *this);
		if (edge.second)
		{
			if ((*this)[edge.first].size_ui < size_limit)
				continue;
		}
		//
		unsigned int id = (*this)[v2].id;
		unsigned int brick_id = (*this)[v2].brick_id;
		auto l1 = list.find(GetKey(id, brick_id));
		if (l1 == list.end())
		{
			pCompInfo info = (*this)[v2].compinfo.lock();
			GetLinkedComps(info, list, size_limit);
		}
	}

	return true;
}

bool CompGraph::GetLinkedComps(CompList& list_in, CompList& list, unsigned int size_limit)
{
	ClearVisited();

	for (auto comp_iter = list_in.begin();
		comp_iter != list_in.end(); ++comp_iter)
	{
		auto comp = comp_iter->second;
		if (!GetLinkedComps(comp, list, size_limit))
		{
			list.insert(std::pair<unsigned long long, pCompInfo>
			(comp_iter->first, comp));
		}
	}

	return !list.empty();
}