/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef REGISTRATOR_H
#define REGISTRATOR_H

#include <Types/Vector.h>
#include <Types/Point.h>
#include <Types/Transform.h>
#include <VolCache.h>
#include <boost/signals2.hpp>

class VolumeData;
namespace flrd
{
	class Registrator
	{
	public:
		Registrator();
		~Registrator();

		void SetExtension(const fluo::Vector& ext1,
			const fluo::Vector& ext2)
		{
			m_extt = ext1;
			m_exta = ext2;
		}
		void SetMaxIter(const int iter)
		{
			m_iter = iter;
		}
		void SetConvNum(const int num)
		{
			m_conv_num = num;
		}
		void SetMethod(const int method)
		{
			m_method = method;
		}
		void SetFilterSize(const int fsize)
		{
			m_fsize = fsize;
		}
		void SetVolumeData(VolumeData* vd)
		{
			m_vd = vd;
		}

		bool Run(size_t f1, size_t f2,
			int mode, size_t start);

		fluo::Point GetCenter()
		{
			return m_center;
		}
		fluo::Point GetCenterVol();
		void SetCenter(const fluo::Point &p)
		{
			m_center = p;
		}
		fluo::Point GetEuler()
		{
			return m_euler;
		}
		void SetEuler(const fluo::Point &e)
		{
			m_euler = e;
		}
		fluo::Transform GetTransform()
		{
			return m_tf;
		}
		void SetTransform(const fluo::Transform &tf)
		{
			m_tf = tf;
		}

		//connect and disconnect functions for cache queue
		typedef boost::function<void(VolCache&)> func_cache;
		void RegisterCacheQueueFuncs(const func_cache &fnew, const func_cache &fdel);
		void UnregisterCacheQueueFuncs();

	private:
		fluo::Vector m_extt;//transform extension
		fluo::Vector m_exta;//rotation extension
		int m_iter;//max iteration number
		int m_conv_num;//max convergence for step size shrink
		int m_method;//compare method
		int m_fsize;//filter size
		VolumeData *m_vd;

		//output
		fluo::Point m_center;//rigid transformation
		fluo::Point m_euler;
		fluo::Transform m_tf;

		//volume data cache
		CacheQueue m_vol_cache;
		boost::signals2::connection m_new_conn;
		boost::signals2::connection m_del_conn;

	};

	inline void Registrator::RegisterCacheQueueFuncs(
		const func_cache &fnew, const func_cache &fdel)
	{
		m_new_conn = m_vol_cache.m_new_cache.connect(fnew);
		m_del_conn = m_vol_cache.m_del_cache.connect(fdel);
	}

	inline void Registrator::UnregisterCacheQueueFuncs()
	{
		m_new_conn.disconnect();
		m_del_conn.disconnect();
	}

}
#endif//REGISTRATOR_H