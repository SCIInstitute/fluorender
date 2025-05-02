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
#include <Vertex.h>

using namespace flrd;

unsigned int Cell::GetVertexId()
{
	Verp vertex = m_vertex.lock();
	if (vertex)
		return vertex->Id();
	else
		return 0;
}

Pca& Cell::GetPca()
{
	return m_pca;
}

unsigned int Vertex::Id()
{
	return m_id;
}

void Vertex::Id(unsigned int id)
{
	m_id = id;
}

Vrtx Vertex::GetInterVert(InterGraph& graph)
{
	unsigned int key = static_cast<unsigned int>(graph.index);
	InterVertListIter iter = m_inter_verts.find(key);
	if (iter != m_inter_verts.end())
		return iter->second;
	else
		return InterGraph::null_vertex();
}

void Vertex::SetInterVert(InterGraph& graph,
	Vrtx inter_vert)
{
	unsigned int key = static_cast<unsigned int>(graph.index);
	InterVertListIter iter = m_inter_verts.find(key);
	if (iter != m_inter_verts.end())
		iter->second = inter_vert;
	else
		m_inter_verts.insert(
			std::pair<unsigned int, Vrtx>(key, inter_vert));
}

bool Vertex::GetRemovedFromGraph()
{
	for (auto iter = m_inter_verts.begin();
		iter != m_inter_verts.end(); ++iter)
		if (iter->second != InterGraph::null_vertex())
			return false;
	return true;
}

size_t Vertex::GetFrame(InterGraph& graph)
{
	unsigned int key = static_cast<unsigned int>(graph.index);
	InterVertListIter iter = m_inter_verts.find(key);
	if (iter != m_inter_verts.end())
		return graph[iter->second].frame;
	else
		return (size_t)-1;
}

bool Vertex::GetSplit()
{
	return m_split;
}

void Vertex::SetSplit(bool split)
{
	m_split = split;
}

void Vertex::SetCenter(fluo::Point& center)
{
	m_center = center;
}

void Vertex::SetSizeUi(unsigned int size)
{
	m_size_ui = size;
}

void Vertex::SetSizeD(double size)
{
	m_size_d = size;
}

void Vertex::Update()
{
	m_center = fluo::Point();
	m_size_ui = 0;
	m_size_d = 0;

	if (m_cells.size() == 0)
		return;

	for (CellBinIter iter = m_cells.begin();
		iter != m_cells.end(); ++iter)
	{
		Celp celp = iter->lock();
		if (!celp)
			continue;
		m_center += celp->GetCenter();
		m_size_ui += celp->GetSizeUi();
		m_size_d += celp->GetSizeD();
	}
	m_center /= static_cast<double>(m_cells.size());
}

size_t Vertex::GetCellNum()
{
	return m_cells.size();
}

Celp Vertex::GetCell(size_t idx)
{
	if (idx >= m_cells.size())
		return nullptr;
	else
		return m_cells[idx].lock();
}

int Vertex::FindCell(Celp& celp)
{
	for (size_t i = 0; i < m_cells.size(); ++i)
	{
		Celp celp0 = m_cells[i].lock();
		if (celp0 && celp0->Id() == celp->Id())
			return static_cast<int>(i);
	}
	return -1;
}

void Vertex::AddCell(Celp& celp, bool inc)
{
	if (FindCell(celp) >= 0)
		return;

	if (inc)
	{
		m_size_ui += celp->GetSizeUi();
		m_size_d += celp->GetSizeD();
	}
	m_cells.push_back(Celw(celp));

	m_split = false;
}

void Vertex::RemoveCell(Celp& celp)
{
	for (CellBinIter iter = m_cells.begin();
		iter != m_cells.end(); ++iter)
	{
		Celp celp0 = iter->lock();
		if (celp0 && celp0->Id() == celp->Id())
		{
			m_cells.erase(iter);
			m_split = false;
			return;
		}
	}
}

CellBinIter Vertex::GetCellsBegin()
{
	return m_cells.begin();
}

CellBinIter Vertex::GetCellsEnd()
{
	return m_cells.end();
}

fluo::Point& Vertex::GetCenter()
{
	return m_center;
}

unsigned int Vertex::GetSizeUi()
{
	return m_size_ui;
}

double Vertex::GetSizeD()
{
	return m_size_d;
}

fluo::BBox Vertex::GetBox()
{
	fluo::BBox box;
	for (CellBinIter iter = m_cells.begin();
		iter != m_cells.end(); ++iter)
	{
		Celp celp = iter->lock();
		box.extend(celp->GetBox());
	}
	return box;
}
