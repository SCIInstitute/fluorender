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
#ifndef FL_Dbscan_h
#define FL_Dbscan_h

#include <FLIVR/Point.h>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>

namespace FL
{
	struct ClusterPoint
	{
		unsigned int id;
		bool visited;
		FLIVR::Point center;
		float intensity;
	};

	typedef boost::shared_ptr<ClusterPoint> pClusterPoint;

	inline float Dist(const ClusterPoint &p1, const ClusterPoint &p2, float w)
	{
		FLIVR::Vector p1p2 = p1.center - p2.center;
		float int_diff = fabs(p1.intensity - p2.intensity);
		return p1p2.length() + w * int_diff;
	}

	class Cluster : public std::list<pClusterPoint>
	{
	public:
		bool find(pClusterPoint &p);
		void join(Cluster &cluster);
	};

	typedef Cluster::iterator ClusterIter;

	inline bool Cluster::find(pClusterPoint &p)
	{
		for (ClusterIter iter = this->begin();
			iter != this->end(); ++iter)
			if ((*iter)->id == p->id)
				return true;
		return false;
	}

	inline void Cluster::join(Cluster &cluster)
	{
		for (ClusterIter iter = cluster.begin();
			iter != cluster.end(); ++iter)
			if (!this->find(*iter))
				this->push_back(*iter);
	}

	typedef std::vector<Cluster>::iterator ClusterSetIter;
	class ClusterSet
	{
	public:
		ClusterSet() {};
		~ClusterSet() {};

		void push_back(const Cluster &cluster)
		{ m_list.push_back(cluster); }

		bool find(const pClusterPoint &p)
		{
			for (size_t i = 0; i < m_list.size(); ++i)
			for (ClusterIter iter = m_list[i].begin();
				iter != m_list[i].end(); ++iter)
				if ((*iter)->id == p->id)
					return true;
			return false;
		}

		size_t size()
		{ return m_list.size(); }
		Cluster& operator[](size_t idx)
		{ return m_list[idx]; }
		ClusterSetIter begin()
		{ return m_list.begin(); }

	private:
		std::vector<Cluster> m_list;
	};

	class ClusterDbscan
	{
	public:
		ClusterDbscan();
		~ClusterDbscan();

		void SetData(Cluster &data)
		{ m_data = data; }
		ClusterSet &GetResult()
		{ return m_result; }
		void SetSize(unsigned int size)
		{ m_size = size; }
		void SetEps(float eps)
		{ m_eps = eps; }
		void ResetIDCounter()
		{ m_id_counter = 1; }
		void AddClusterPoint(const FLIVR::Point &p, const float value);
		bool Execute();
		void GenerateNewIDs(unsigned int id, void* label,
			size_t nx, size_t ny, size_t nz);

	private:
		Cluster m_data;
		ClusterSet m_result;
		unsigned int m_size;
		float m_eps;
		float m_intw;
		unsigned int m_id_counter;

	private:
		void ExpandCluster(pClusterPoint& p, Cluster& neighbors, Cluster& cluster);
		Cluster GetNeighbors(pClusterPoint &p);
		bool FindId(void* label, unsigned int id,
			size_t nx, size_t ny, size_t nz);
	};

}
#endif//FL_Dbscan_h