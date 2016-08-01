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
#include <boost/unordered_map.hpp>
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

	inline float Dist(const ClusterPoint &p1, const ClusterPoint &p2, float w)
	{
		FLIVR::Vector p1p2 = p1.center - p2.center;
		float int_diff = fabs(p1.intensity - p2.intensity);
		return p1p2.length() + w * int_diff;
	}

	typedef boost::unordered_map<unsigned int, ClusterPoint> Cluster;
	typedef Cluster::iterator ClusterIter;

	class ClusterSet
	{
	public:
		ClusterSet() {};
		~ClusterSet() {};

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
		bool Execute();

	private:
		Cluster m_data;
		ClusterSet m_result;
		unsigned int m_size;
		float m_eps;
		float m_intw;

	private:
		void ExpandCluster();
		Cluster GetNeighbors(ClusterPoint &p);
	};
}
#endif//FL_Dbscan_h