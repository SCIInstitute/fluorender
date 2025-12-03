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
#ifndef FL_ClusterMethod_h
#define FL_ClusterMethod_h

#include <Vector.h>
#include <boost/qvm/vec.hpp>
#include <boost/qvm/mat.hpp>
#include <boost/qvm/vec_operations.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>
#include <Progress.h>
#include <memory>

namespace flrd
{
	class CelpList;

	typedef boost::qvm::vec<double, 3> EmVec;
	typedef boost::qvm::mat<double, 3, 3> EmMat;

	struct ClusterPoint
	{
		unsigned int id;
		int cid;//cluster id at initialization
		bool visited;
		bool noise;
		EmVec centeri;
		EmVec centerf;
		float intensity;
	};

	typedef boost::shared_ptr<ClusterPoint> pClusterPoint;

	inline float Dist(const ClusterPoint &p1, const ClusterPoint &p2, float w)
	{
		EmVec p1p2 = p1.centerf - p2.centerf;
		float int_diff = fabs(p1.intensity - p2.intensity);
		return static_cast<float>(boost::qvm::mag(p1p2)) + w * int_diff;
	}

	class Cluster : public std::list<pClusterPoint>
	{
	public:
		inline bool find(pClusterPoint &p);
		inline void join(Cluster &cluster);
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

		int find_(const pClusterPoint &p)
		{
			for (size_t i = 0; i < m_list.size(); ++i)
			for (ClusterIter iter = m_list[i].begin();
				iter != m_list[i].end(); ++iter)
				if ((*iter)->id == p->id)
					return int(i);
			return -1;
		}

		size_t size()
		{ return m_list.size(); }
		Cluster& operator[](size_t idx)
		{ return m_list[idx]; }
		ClusterSetIter begin()
		{ return m_list.begin(); }
		void clear()
		{ m_list.clear(); }
		void resize(size_t n)
		{ m_list.resize(n); }

	private:
		std::vector<Cluster> m_list;
	};

	class ClusterMethod : public Progress
	{
	public:
		ClusterMethod();
		virtual ~ClusterMethod() {};

		void SetData(Cluster &data)
		{ m_data = data; }
		Cluster &GetData()
		{ return m_data; }
		void AddIDsToData();
		void SetUseInitCluster(bool val)
		{ m_use_init_cluster = val; }
		ClusterSet &GetResult()
		{ return m_result; }
		size_t GetCluterNum()
		{ return m_result.size(); }
		void ResetIDCounter()
		{ m_id_counter = 1; }
		void SetSpacing(const fluo::Vector& spc)
		{ m_spc = {spc.x(), spc.y(), spc.z()}; }
		void AddClusterPoint(const EmVec &p, const float value, int cid=-1);
		void GenerateNewIDs(unsigned int id, void* label,
			size_t nx, size_t ny, size_t nz,
			bool out_cells = false, unsigned int inc = 42);
		bool FindId(void* label, unsigned int id,
			size_t nx, size_t ny, size_t nz);
		std::vector<unsigned int> &GetNewIDs()
		{ return m_id_list; }
		virtual bool Execute() = 0;
		virtual float GetProb() = 0;

		CelpList& GetCellList();

	protected:
		Cluster m_data;
		ClusterSet m_result;
		unsigned int m_id_counter;
		std::vector<unsigned int> m_id_list;
		bool m_use_init_cluster;
		EmVec m_spc;//spacings
		//output cells
		std::unique_ptr<CelpList> m_out_cells;
	};
}
#endif//FL_ClusterMethod_h
