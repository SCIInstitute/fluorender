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

#ifndef _VOLUMELOADER_H_
#define _VOLUMELOADER_H_

#include <DataManager.h>
#include <TextureBrick.h>

class VolumeLoader;

struct VolumeLoaderData
{
	flvr::FileLocInfo *finfo;
	flvr::TextureBrick *brick;
	VolumeData *vd;
	unsigned long long datasize;
	int mode;
};

class VolumeLoader
{
public:
	VolumeLoader();
	~VolumeLoader();
	void Queue(VolumeLoaderData brick);
	void ClearQueues();
	void Set(std::vector<VolumeLoaderData> vld);
	bool Run();
	void SetMemoryLimitByte(long long limit) { m_memory_limit = limit; }
	void CleanupLoadedBrick();
	void RemoveAllLoadedBrick();
	void RemoveBrickVD(VolumeData *vd);

	static bool sort_data_dsc(const VolumeLoaderData b1, const VolumeLoaderData b2)
	{
		return b2.brick->get_d() > b1.brick->get_d();
	}
	static bool sort_data_asc(const VolumeLoaderData b1, const VolumeLoaderData b2)
	{
		return b2.brick->get_d() < b1.brick->get_d();
	}

protected:
	std::vector<VolumeLoaderData> m_queues;
	std::vector<VolumeLoaderData> m_queued;
	std::unordered_map<flvr::TextureBrick*, VolumeLoaderData> m_loaded;
	bool m_valid;

	long long m_memory_limit;
	long long m_used_memory;

	inline void AddLoadedBrick(VolumeLoaderData lbd)
	{
		m_loaded[lbd.brick] = lbd;
		m_used_memory += lbd.datasize;
	}
};

#endif//_VOLUMELOADER_H_