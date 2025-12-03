//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <VolumeLoader.h>
#include <TextureBrick.h>
#include <VolumeData.h>

VolumeLoader::VolumeLoader()
{
	m_memory_limit = 10000000LL;
	m_used_memory = 0LL;
}

VolumeLoader::~VolumeLoader()
{
	RemoveAllLoadedBrick();
}

void VolumeLoader::Queue(VolumeLoaderData brick)
{
	m_queues.push_back(brick);
}

void VolumeLoader::ClearQueues()
{
	if (!m_queues.empty())
	{
		m_queues.clear();
	}
}

void VolumeLoader::Set(std::vector<VolumeLoaderData> vld)
{
	m_queues = vld;
}

bool VolumeLoader::Run()
{
	auto ite = m_loaded.begin();
	while (ite != m_loaded.end())
	{
		if (!ite->second.brick->isLoaded() && ite->second.brick->isLoading())
		{
			ite->second.brick->set_loading_state(false);
			ite = m_loaded.erase(ite);
		}
		else
			ite++;
	}

	while (m_queues.size())
	{
		VolumeLoaderData b = m_queues[0];
		b.brick->set_loading_state(false);
		m_queues.erase(m_queues.begin());
		m_queued.push_back(b);

		if (!b.brick->isLoaded() && !b.brick->isLoading())
		{
			if (m_used_memory >= m_memory_limit)
			{
				while (1)
				{
					CleanupLoadedBrick();
					if (m_used_memory < m_memory_limit)
						break;
				}
			}

			char *ptr = NULL;
			size_t readsize;
			flvr::TextureBrick::read_brick_without_decomp(ptr, readsize, b.finfo, (void*)this);
			if (!ptr) continue;

			if (b.finfo->type == BRICK_FILE_TYPE_RAW)
			{
				b.brick->set_brkdata(ptr);
				b.datasize = readsize;
				AddLoadedBrick(b);
			}
		}
		else
		{
			auto res = b.brick->get_size();
			int nb = b.brick->nb(flvr::CompType::Data);
			size_t bsize = (size_t)(res.intx())*(size_t)(res.inty())*(size_t)(res.intz())*(size_t)(nb);
			b.datasize = bsize;

			if (m_loaded.find(b.brick) != m_loaded.end())
				m_loaded[b.brick] = b;
		}
	}

	return true;
}

void VolumeLoader::CleanupLoadedBrick()
{
	long long required = 0;

	for (size_t i = 0; i < m_queues.size(); i++)
	{
		flvr::TextureBrick *b = m_queues[i].brick;
		if (!m_queues[i].brick->isLoaded())
		{
			auto res = b->get_size();
			int nb = b->nb(flvr::CompType::Data);
			required += (size_t)(res.intx()) * (size_t)(res.inty()) * (size_t)(res.intz()) * (size_t)(nb);
		}
	}

	std::vector<VolumeLoaderData> vd_undisp;
	std::vector<VolumeLoaderData> b_undisp;
	std::vector<VolumeLoaderData> b_drawn;
	for (auto elem : m_loaded)
	{
		if (!elem.second.vd->GetDisp())
			vd_undisp.push_back(elem.second);
		else if (!elem.second.brick->get_disp())
			b_undisp.push_back(elem.second);
		else if (elem.second.brick->drawn(elem.second.mode))
			b_drawn.push_back(elem.second);
	}
	if (required > 0 || m_used_memory >= m_memory_limit)
	{
		for (size_t i = 0; i < vd_undisp.size(); i++)
		{
			if (!vd_undisp[i].brick->isLoaded())
				continue;
			vd_undisp[i].brick->freeBrkData();
			required -= vd_undisp[i].datasize;
			m_used_memory -= vd_undisp[i].datasize;
			m_loaded.erase(vd_undisp[i].brick);
			if (required <= 0 && m_used_memory < m_memory_limit)
				break;
		}
	}
	if (required > 0 || m_used_memory >= m_memory_limit)
	{
		for (size_t i = 0; i < b_undisp.size(); i++)
		{
			if (!b_undisp[i].brick->isLoaded())
				continue;
			b_undisp[i].brick->freeBrkData();
			required -= b_undisp[i].datasize;
			m_used_memory -= b_undisp[i].datasize;
			m_loaded.erase(b_undisp[i].brick);
			if (required <= 0 && m_used_memory < m_memory_limit)
				break;
		}
	}
	if (required > 0 || m_used_memory >= m_memory_limit)
	{
		for (size_t i = 0; i < b_drawn.size(); i++)
		{
			if (!b_drawn[i].brick->isLoaded())
				continue;
			b_drawn[i].brick->freeBrkData();
			required -= b_drawn[i].datasize;
			m_used_memory -= b_drawn[i].datasize;
			m_loaded.erase(b_drawn[i].brick);
			if (required <= 0 && m_used_memory < m_memory_limit)
				break;
		}
	}
	if (m_used_memory >= m_memory_limit)
	{
		for (size_t i = m_queues.size(); i > 0; i--)
		{
			flvr::TextureBrick *b = m_queues[i-1].brick;
			if (b->isLoaded() && m_loaded.find(b) != m_loaded.end())
			{
				bool skip = false;
				for (size_t j = m_queued.size(); j > 0; j--)
				{
					if (m_queued[j-1].brick == b && !b->drawn(m_queued[j-1].mode))
						skip = true;
				}
				if (!skip)
				{
					b->freeBrkData();
					auto res = b->get_size();
					int nb = b->nb(flvr::CompType::Data);
					long long datasize = (size_t)(res.intx()) * (size_t)(res.inty()) * (size_t)(res.intz()) * (size_t)(nb);
					required -= datasize;
					m_used_memory -= datasize;
					m_loaded.erase(b);
					if (m_used_memory < m_memory_limit)
						break;
				}
			}
		}
	}

}

void VolumeLoader::RemoveAllLoadedBrick()
{
	for (auto e : m_loaded)
	{
		if (e.second.brick->isLoaded())
		{
			e.second.brick->freeBrkData();
			m_used_memory -= e.second.datasize;
		}
	}
	m_loaded.clear();
}

void VolumeLoader::RemoveBrickVD(VolumeData *vd)
{
	auto ite = m_loaded.begin();
	while (ite != m_loaded.end())
	{
		if (ite->second.vd == vd && ite->second.brick->isLoaded())
		{
			ite->second.brick->freeBrkData();
			m_used_memory -= ite->second.datasize;
			ite = m_loaded.erase(ite);
		}
		else
			ite++;
	}
}

bool VolumeLoader::sort_data_dsc(const VolumeLoaderData b1, const VolumeLoaderData b2)
{
	return b2.brick->get_d() > b1.brick->get_d();
}

bool VolumeLoader::sort_data_asc(const VolumeLoaderData b1, const VolumeLoaderData b2)
{
	return b2.brick->get_d() < b1.brick->get_d();
}
