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
#include <ClusterMethod.h>
#include <Cell.h>
#include <boost/qvm/vec_access.hpp>

using namespace flrd;

ClusterMethod::ClusterMethod() :
	m_id_counter(1),
	m_use_init_cluster(false),
	m_spc({1, 1, 1}),
	Progress()
{
	m_out_cells = std::make_unique<CelpList>();
}

void ClusterMethod::AddClusterPoint(const EmVec &p, const float value, int cid)
{
	pClusterPoint pp(new ClusterPoint);
	pp->id = m_id_counter++;
	pp->cid = cid;
	pp->visited = false;
	pp->noise = true;
	using namespace boost::qvm;
	pp->centeri = p;
	pp->centerf = { A0(p)*A0(m_spc), A1(p)*A1(m_spc), A2(p)*A2(m_spc) };
	pp->intensity = value;
	m_data.push_back(pp);
	if (cid > -1)
		m_use_init_cluster = true;
}

void ClusterMethod::GenerateNewIDs(unsigned int id, void* label,
	const fluo::Vector& size,
	bool out_cells, unsigned int inc)
{
	m_id_list.clear();
	if (out_cells)
		m_out_cells->clear();

	unsigned int id2 = id;
	unsigned long long index;
	int i, j, k;
	Cell* cell = 0;

	size_t ticks = 0;
	for (size_t ii = 0; ii < m_result.size(); ++ii)
		ticks += m_result[ii].size();
	if (!ticks)
		ticks = 1;
	else
		ticks /= 100;
	size_t count = 0;
	for (size_t ii = 0; ii < m_result.size(); ++ii)
	{
		Cluster &cluster = m_result[ii];
		do
		{
			id2 += inc;
			if (id2 == id)
				break;
		} while (!id2 || FindId(label, id2, size));

		if (out_cells)
			cell = new Cell(id2);

		for (ClusterIter iter = cluster.begin();
			iter != cluster.end(); ++iter)
		{
			i = int(boost::qvm::A0((*iter)->centeri) + 0.5);
			if (i <= 0 || i >= size.intx() - 1)
				continue;
			j = int(boost::qvm::A1((*iter)->centeri) + 0.5);
			if (j <= 0 || j >= size.inty() - 1)
				continue;
			k = int(boost::qvm::A2((*iter)->centeri) + 0.5);
			if (k < 0 || k > size.intz() - 1)
				continue;
			index = size.get_size_xy()*k + size.intx()*j + i;
			((unsigned int*)label)[index] = id2;

			if (out_cells)
				cell->Inc(fluo::Point(i, j, k), (*iter)->intensity);

			if (count % 100 == 0)
				SetProgress(static_cast<int>(count / ticks),
					"Generating IDs.");
			count++;
		}
		m_id_list.push_back(id2);

		if (out_cells)
			m_out_cells->insert(std::pair<unsigned int, Celp>
				(id2, Celp(cell)));

	}
}

bool ClusterMethod::FindId(void* label, unsigned int id,
	const fluo::Vector& size)
{
	unsigned long long for_size = (unsigned long long)size.get_size_xyz();
	unsigned long long index;
	for (index = 0; index < for_size; ++index)
	{
		if (((unsigned int*)label)[index] == id)
			return true;
	}
	return false;
}

void ClusterMethod::AddIDsToData()
{
	for (size_t ii = 0; ii < m_result.size(); ++ii)
	{
		Cluster &cluster = m_result[ii];
		for (auto iter = cluster.begin();
			iter != cluster.end(); ++iter)
		{
			(*iter)->cid = static_cast<int>(ii);
		}
	}
}

CelpList& ClusterMethod::GetCellList()
{
	return *m_out_cells;
}