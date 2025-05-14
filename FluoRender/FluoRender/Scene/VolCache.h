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

namespace flvr
{
	class Texture;
}
namespace flrd
{
	class CacheQueue;
	class CQCallback;
	class VolCache
	{
	public:
		VolCache() :
			m_data(0),
			m_mask(0),
			m_label(0),
			m_tex(0),
			m_tnum(0),
			m_valid(false),
			m_modified(false),
			m_protect(false) {}

		flvr::Texture* GetTexture() { return m_tex; }
		Nrrd* GetNrrdData() { return m_data; }
		Nrrd* GetNrrdMask() { return m_mask; }
		Nrrd* GetNrrdLabel() { return m_label; }
		void* GetRawData() { if (m_data) return m_data->data; return 0; }
		void* GetRawMask() { if (m_mask) return m_mask->data; return 0; }
		void* GetRawLabel() { if (m_label) return m_label->data; return 0; }

	private:
		Nrrd* m_data;
		Nrrd* m_mask;
		Nrrd* m_label;
		//texture for 4d colormap
		flvr::Texture* m_tex;
		size_t m_tnum;//current time point number
		bool m_valid;
		bool m_modified;
		bool m_protect;

		friend class CacheQueue;
		friend class CQCallback;
	};

	typedef std::function<void(VolCache&)> VolCacheFunc;

	//callback manager
	class CQCallback
	{
	public:
		static void ReadVolCache(VolCache& vol_cache);
		static void FreeVolCache(VolCache& vol_cache);

		static bool handle_data;//read data from file and then free
		static bool handle_mask;//read mask from file and then free
		static bool handle_label;//read label from file and then free
		static bool access_data;//read data from memory and don't free
		static bool access_mask;//read mask from memory and don't free
		static bool access_label;//read label from mempry and don't free
		static bool return_data;//read data from gpu and dont free
		static bool return_mask;//read mask from gpu and don't free
		static bool return_label;//read label from gpu and don't free
		static bool save_data;//save data to disk if modified
		static bool save_mask;//save mask to disk if modified
		static bool save_label;//save label to disk if modified
		static bool build_tex;//build texture
		static bool time_cond0;//time conditional0: cur_time==cache_time, access; otherwise, handle

		static void SetHandleFlags(int flags);

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
		static bool HandleData(VolCache& vol_cache);
		static bool HandleMask(VolCache& vol_cache);
		static bool HandleLabel(VolCache& vol_cache);
		static bool AccessData(VolCache& vol_cache, bool ret);
		static bool AccessMask(VolCache& vol_cache, bool ret);
		static bool AccessLabel(VolCache& vol_cache, bool ret);
		static bool SaveData(VolCache& vol_cache);
		static bool SaveMask(VolCache& vol_cache);
		static bool SaveLabel(VolCache& vol_cache);
		static bool BuildTex(VolCache& vol_cache);

		static bool cond0;//actual time condition
	};

	//queue with a max size
	class CacheQueue
	{
	public:
		CacheQueue():
		m_max_size(1),
		m_new_cache(nullptr),
		m_del_cache(nullptr) {};
		~CacheQueue();

		inline void protect(size_t frame);
		inline void unprotect(size_t frame);
		inline void clear();
		inline void set_max_size(size_t size);
		inline size_t get_max_size();
		inline size_t size();
		inline VolCache get(size_t frame);
		inline void set_modified(size_t frame, bool value = true);
		inline void clear(size_t frame);

		void RegisterCacheQueueFuncs(const VolCacheFunc &fnew, const VolCacheFunc &fdel);
		void UnregisterCacheQueueFuncs();

		//external calls
		VolCacheFunc m_new_cache;
		VolCacheFunc m_del_cache;

	private:
		size_t m_max_size;
		std::deque<VolCache> m_queue;

		inline void pop_cache();
		inline void push_cache(const VolCache& val);
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
				m_del_cache(m_queue[i]);
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
						m_del_cache(*iter);
					m_queue.erase(iter);
					break;
				}
			}
		}
		else
		{
			if (m_del_cache)
				m_del_cache(m_queue.front());
			m_queue.pop_front();
		}
	}

	inline void CacheQueue::push_cache(const VolCache& val)
	{
		while (m_queue.size() > m_max_size - 1)
			pop_cache();
		m_queue.push_back(val);
	}

	inline VolCache CacheQueue::get(size_t frame)
	{
		VolCache vol_cache;
		bool found = false;
		int index = -1;
		for (size_t i = 0; i < m_queue.size(); ++i)
		{
			if (m_queue[i].m_tnum == frame)
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
				m_new_cache(m_queue[index]);
			}
			return m_queue[index];
		}
		else
		{
			vol_cache.m_tnum = frame;
			if (m_new_cache)
			{
				m_new_cache(vol_cache);
				push_cache(vol_cache);
			}
		}
		return vol_cache;
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
					m_del_cache(m_queue[i]);
				}
				m_queue.erase(m_queue.begin() + i);
				return;
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

}//namespace flrd

#endif//FL_VolCache_h