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
#ifndef FL_VolCache_h
#define FL_VolCache_h

#include <nrrd.h>
#include <deque>
#include <functional>
#include <memory>

class VolumeData;
namespace flvr
{
	class Texture;
	class CacheQueue;
	class CQCallback;
	class VolCache4D
	{
	public:
		VolCache4D(const std::shared_ptr<VolumeData>& vd) :
			m_vd(vd),
			m_data(0),
			m_mask(0),
			m_label(0),
			m_tnum(0),
			m_valid(false),
			m_modified(false),
			m_protect(false),
			own_data(false),
			own_mask(false),
			own_label(false),
			handle_data(false),
			handle_mask(false),
			handle_label(false),
			access_data(false),
			access_mask(false),
			access_label(false),
			return_data(false),
			return_mask(false),
			return_label(false),
			save_data(false),
			save_mask(false),
			save_label(false),
			build_tex(false),
			time_cond0(false) {}

		Nrrd* GetNrrdData() { return m_data; }
		Nrrd* GetNrrdMask() { return m_mask; }
		Nrrd* GetNrrdLabel() { return m_label; }
		void* GetRawData() { if (m_data) return m_data->data; return 0; }
		void* GetRawMask() { if (m_mask) return m_mask->data; return 0; }
		void* GetRawLabel() { if (m_label) return m_label->data; return 0; }
		size_t GetTime() { return m_tnum; }

		void SetHandleFlags(int flags);
		void Reset()
		{
			own_data = false;
			own_mask = false;
			own_label = false;
			handle_data = false;
			handle_mask = false;
			handle_label = false;
			access_data = false;
			access_mask = false;
			access_label = false;
			return_data = false;
			return_mask = false;
			return_label = false;
			save_data = false;
			save_mask = false;
			save_label = false;
			build_tex = false;
			time_cond0 = false;
			m_vd.reset();
			m_data = 0;
			m_mask = 0;
			m_label = 0;
			m_tnum = 0;
			m_valid = false;
			m_modified = false;
			m_protect = false;
		}

		bool own_data;//if cache owns data for release
		bool own_mask;//if cache owns mask for release
		bool own_label;//if cache owns label for release
		bool handle_data;//read data from file and then free
		bool handle_mask;//read mask from file and then free
		bool handle_label;//read label from file and then free
		bool access_data;//read data from memory and don't free
		bool access_mask;//read mask from memory and don't free
		bool access_label;//read label from mempry and don't free
		bool return_data;//read data from gpu and dont free
		bool return_mask;//read mask from gpu and don't free
		bool return_label;//read label from gpu and don't free
		bool save_data;//save data to disk if modified
		bool save_mask;//save mask to disk if modified
		bool save_label;//save label to disk if modified
		bool build_tex;//build texture
		bool time_cond0;//time conditional0: cur_time==cache_time, access; otherwise, handle

	private:
		std::weak_ptr<VolumeData> m_vd;
		Nrrd* m_data;
		Nrrd* m_mask;
		Nrrd* m_label;

		size_t m_tnum;//current time point number
		bool m_valid;
		bool m_modified;
		bool m_protect;

		friend class CacheQueue;
		friend class CQCallback;
	};

	typedef std::function<void(VolCache4D&)> VolCacheFunc;

	//callback manager
	class CQCallback
	{
	public:
		static void ReadVolCache(VolCache4D& vol_cache);
		static void FreeVolCache(VolCache4D& vol_cache);

		static constexpr int HDL_DATA		= 1 << 0;
		static constexpr int HDL_MASK		= 1 << 1;
		static constexpr int HDL_LABEL		= 1 << 2;
		static constexpr int ACS_DATA		= 1 << 3;
		static constexpr int ACS_MASK		= 1 << 4;
		static constexpr int ACS_LABEL		= 1 << 5;
		static constexpr int RET_DATA		= 1 << 6;
		static constexpr int RET_MASK		= 1 << 7;
		static constexpr int RET_LABEL		= 1 << 8;
		static constexpr int SAV_DATA		= 1 << 9;
		static constexpr int SAV_MASK		= 1 << 10;
		static constexpr int SAV_LABEL		= 1 << 11;
		static constexpr int BLD_TEX		= 1 << 12;
		static constexpr int TIME_COND0		= 1 << 13;//time conditional0

	private:
		static bool HandleData(VolCache4D& vol_cache);
		static bool HandleMask(VolCache4D& vol_cache);
		static bool HandleLabel(VolCache4D& vol_cache);
		static bool AccessData(VolCache4D& vol_cache);
		static bool AccessMask(VolCache4D& vol_cache);
		static bool AccessLabel(VolCache4D& vol_cache);
		static bool SaveData(VolCache4D& vol_cache);
		static bool SaveMask(VolCache4D& vol_cache);
		static bool SaveLabel(VolCache4D& vol_cache);
		static bool BuildTex(VolCache4D& vol_cache);

		static bool cond0;//actual time condition
	};

	//queue with a max size
	class CacheQueue
	{
	public:
		CacheQueue(const std::shared_ptr<VolumeData>& vd):
			m_vd(vd),
			m_max_size(2),
			m_new_cache(nullptr),
			m_del_cache(nullptr),
			m_flags(0) {};
		~CacheQueue();

		inline void protect(size_t frame);
		inline void unprotect(size_t frame);
		inline void clear();
		inline void set_max_size(size_t size);
		inline size_t get_max_size();
		inline size_t size();
		inline VolCache4D* get(size_t frame);
		VolCache4D* get_offset(int toffset);
		inline void set_modified(size_t frame, bool value = true);
		inline void clear(size_t frame);
		inline void reset(size_t frame);

		void RegisterCacheQueueFuncs(const VolCacheFunc &fnew, const VolCacheFunc &fdel);
		void UnregisterCacheQueueFuncs();

		void SetHandleFlags(int flags) { m_flags = flags; }

		//external calls
		VolCacheFunc m_new_cache;
		VolCacheFunc m_del_cache;

	private:
		std::weak_ptr<VolumeData> m_vd;//each volume data has a queue
		size_t m_max_size;
		std::deque<VolCache4D> m_queue;
		int m_flags;

		inline void pop_cache();
		inline void push_cache(const VolCache4D& val);

	};

	inline CacheQueue::~CacheQueue()
	{
		//clear();
	}

	inline void CacheQueue::protect(size_t frame)
	{
		for (auto iter = m_queue.begin();
			iter != m_queue.end(); ++iter)
		{
			if (iter->m_tnum == frame)
			{
				iter->m_protect = true;
				break;
			}
		}
	}

	inline void CacheQueue::unprotect(size_t frame)
	{
		for (auto iter = m_queue.begin();
			iter != m_queue.end(); ++iter)
		{
			if (iter->m_tnum == frame)
			{
				iter->m_protect = false;
				break;
			}
		}
	}

	inline void CacheQueue::clear()
	{
		if (m_del_cache)
		{
			for (size_t i = 0; i < m_queue.size(); ++i)
			{
				m_queue[i].SetHandleFlags(m_flags);
				m_del_cache(m_queue[i]);
			}
		}
		m_queue.clear();
	}

	inline void CacheQueue::set_max_size(size_t size)
	{
		if (size == 0)
			return;
		m_max_size = size;
		while (m_queue.size() > m_max_size)
			pop_cache();
	}

	inline size_t CacheQueue::get_max_size()
	{
		return m_max_size;
	}

	inline size_t CacheQueue::size()
	{
		return m_queue.size();
	}

	inline void CacheQueue::pop_cache()
	{
		if (m_queue.front().m_protect)
		{
			for (auto iter = m_queue.begin();
				iter != m_queue.end(); ++iter)
			{
				if (!iter->m_protect)
				{
					if (m_del_cache)
					{
						iter->SetHandleFlags(m_flags);
						m_del_cache(*iter);
					}
					m_queue.erase(iter);
					break;
				}
			}
		}
		else
		{
			if (m_del_cache)
			{
				m_queue.front().SetHandleFlags(m_flags);
				m_del_cache(m_queue.front());
			}
			m_queue.pop_front();
		}
	}

	inline void CacheQueue::push_cache(const VolCache4D& val)
	{
		while (m_queue.size() > m_max_size - 1)
			pop_cache();
		m_queue.push_back(val);
	}

	inline VolCache4D* CacheQueue::get(size_t frame)
	{
		bool found = false;
		int index = -1;
		for (size_t i = 0; i < m_queue.size(); ++i)
		{
			if (m_queue[i].m_tnum == frame && m_queue[i].m_valid)
			{
				index = (int)i;
				found = true;
				break;
			}
		}
		if (found)
		{
			if (!m_queue[index].m_valid && m_new_cache)
			{
				m_queue[index].SetHandleFlags(m_flags);
				m_new_cache(m_queue[index]);
			}
			return &(m_queue[index]);
		}
		else
		{
			auto vd_ptr = m_vd.lock();
			if (vd_ptr)
			{
				VolCache4D vol_cache(vd_ptr);
				vol_cache.m_tnum = frame;
				if (m_new_cache)
				{
					vol_cache.SetHandleFlags(m_flags);
					m_new_cache(vol_cache);
				}
				push_cache(vol_cache);
			}
			if (!m_queue.empty())
				return &m_queue.back();
		}
		return 0;
	}

	inline void CacheQueue::set_modified(size_t frame, bool value)
	{
		for (size_t i = 0; i < m_queue.size(); ++i)
		{
			if (m_queue[i].m_tnum == frame)
			{
				if (m_queue[i].m_valid)
					m_queue[i].m_modified = value;
				return;
			}
		}
	}

	inline void CacheQueue::clear(size_t frame)
	{
		for (size_t i = 0; i < m_queue.size(); ++i)
		{
			if (m_queue[i].m_tnum == frame)
			{
				if (m_del_cache)
				{
					m_queue[i].SetHandleFlags(m_flags);
					m_del_cache(m_queue[i]);
				}
				m_queue.erase(m_queue.begin() + i);
				return;
			}
		}
	}

	inline void CacheQueue::reset(size_t frame)
	{
		for (size_t i = 0; i < m_queue.size(); ++i)
		{
			if (m_queue[i].m_tnum == frame)
			{
				m_queue[i].Reset();
				m_queue.erase(m_queue.begin() + i);
				break;
			}
		}
	}

	inline void CacheQueue::RegisterCacheQueueFuncs(
		const VolCacheFunc &fnew, const VolCacheFunc &fdel)
	{
		m_new_cache = fnew;
		m_del_cache = fdel;
	}

	inline void CacheQueue::UnregisterCacheQueueFuncs()
	{
		m_new_cache = nullptr;
		m_del_cache = nullptr;
	}

}//namespace flvr

#endif//FL_VolCache_h