//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include "VolumeLoader.h"
#include <wx/utils.h> 

VolumeDecompressorThread::VolumeDecompressorThread(VolumeLoader *vl)
	: wxThread(wxTHREAD_DETACHED), m_vl(vl)
{

}

VolumeDecompressorThread::~VolumeDecompressorThread()
{
	wxCriticalSectionLocker enter(m_vl->m_pThreadCS);
	// the thread is being destroyed; make sure not to leave dangling pointers around
	m_vl->m_running_decomp_th--;
}

wxThread::ExitCode VolumeDecompressorThread::Entry()
{
/*	unsigned int st_time = GET_TICK_COUNT();

	m_vl->m_pThreadCS.Enter();
	m_vl->m_running_decomp_th++;
	m_vl->m_pThreadCS.Leave();

	while (1)
	{
		m_vl->m_pThreadCS.Enter();

		if (m_vl->m_decomp_queues.size() == 0)
		{
			m_vl->m_pThreadCS.Leave();
			break;
		}
		VolumeDecompressorData q = m_vl->m_decomp_queues[0];
		m_vl->m_decomp_queues.erase(m_vl->m_decomp_queues.begin());

		m_vl->m_pThreadCS.Leave();


		size_t bsize = (size_t)(q.b->nx())*(size_t)(q.b->ny())*(size_t)(q.b->nz())*(size_t)(q.b->nb(0));
		char *result = new char[bsize];
		if (TextureBrick::decompress_brick(result, q.in_data, bsize, q.in_size, q.finfo->type))
		{
			m_vl->m_pThreadCS.Enter();

			delete[] q.in_data;
			q.b->set_brkdata(result);
			q.b->set_loading_state(false);
			m_vl->m_pThreadCS.Leave();
		}
		else
		{
			delete[] result;

			m_vl->m_pThreadCS.Enter();
			delete[] q.in_data;
			m_vl->m_used_memory -= bsize;
			q.b->set_drawn(q.mode, true);
			q.b->set_loading_state(false);
			m_vl->m_pThreadCS.Leave();
		}
	}
*/
	return (wxThread::ExitCode)0;
}

/*
wxDEFINE_EVENT(wxEVT_VLTHREAD_COMPLETED, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_VLTHREAD_PAUSED, wxCommandEvent);
*/
VolumeLoaderThread::VolumeLoaderThread(VolumeLoader *vl)
	: wxThread(wxTHREAD_JOINABLE), m_vl(vl)
{

}

VolumeLoaderThread::~VolumeLoaderThread()
{
	wxCriticalSectionLocker enter(m_vl->m_pThreadCS);
	if (!m_vl->m_decomp_queues.empty())
	{
		for (int i = 0; i < m_vl->m_decomp_queues.size(); i++)
		{
			if (m_vl->m_decomp_queues[i].in_data != NULL)
				delete[] m_vl->m_decomp_queues[i].in_data;
			m_vl->m_used_memory -= m_vl->m_decomp_queues[i].datasize;
		}
		m_vl->m_decomp_queues.clear();
	}
	// the thread is being destroyed; make sure not to leave dangling pointers around
}

wxThread::ExitCode VolumeLoaderThread::Entry()
{
	//unsigned int st_time = GET_TICK_COUNT();

	while (m_vl->m_running_decomp_th > 0)
	{
		if (TestDestroy())
			return (wxThread::ExitCode)0;
		Sleep(10);
	}

	m_vl->m_pThreadCS.Enter();
	auto ite = m_vl->m_loaded.begin();
	while (ite != m_vl->m_loaded.end())
	{
		if (!ite->second.brick->isLoaded() && ite->second.brick->isLoading())
		{
			ite->second.brick->set_loading_state(false);
			ite = m_vl->m_loaded.erase(ite);
		}
		else
			ite++;
	}
	m_vl->m_pThreadCS.Leave();

	while (1)
	{
		if (TestDestroy())
			break;

		m_vl->m_pThreadCS.Enter();
		if (m_vl->m_queues.size() == 0)
		{
			m_vl->m_pThreadCS.Leave();
			break;
		}
		VolumeLoaderData b = m_vl->m_queues[0];
		b.brick->set_loading_state(false);
		m_vl->m_queues.erase(m_vl->m_queues.begin());
		m_vl->m_queued.push_back(b);
		m_vl->m_pThreadCS.Leave();

		if (!b.brick->isLoaded() && !b.brick->isLoading())
		{
			if (m_vl->m_used_memory >= m_vl->m_memory_limit)
			{
				m_vl->m_pThreadCS.Enter();
				while (1)
				{
					m_vl->CleanupLoadedBrick();
					if (m_vl->m_used_memory < m_vl->m_memory_limit || TestDestroy())
						break;
					m_vl->m_pThreadCS.Leave();
					Sleep(10);
					m_vl->m_pThreadCS.Enter();
				}
				m_vl->m_pThreadCS.Leave();
			}

			char *ptr = NULL;
			size_t readsize;
			TextureBrick::read_brick_without_decomp(ptr, readsize, b.finfo, this);
			if (!ptr) continue;

			if (b.finfo->type == BRICK_FILE_TYPE_RAW)
			{
				m_vl->m_pThreadCS.Enter();
				b.brick->set_brkdata(ptr);
				b.datasize = readsize;
				m_vl->AddLoadedBrick(b);
				m_vl->m_pThreadCS.Leave();
			}
			else
			{
				bool decomp_in_this_thread = false;
				VolumeDecompressorData dq;
				dq.b = b.brick;
				dq.finfo = b.finfo;
				dq.vd = b.vd;
				dq.mode = b.mode;
				dq.in_data = ptr;
				dq.in_size = readsize;

				size_t bsize = (size_t)(b.brick->nx())*(size_t)(b.brick->ny())*(size_t)(b.brick->nz())*(size_t)(b.brick->nb(0));
				b.datasize = bsize;
				dq.datasize = bsize;

				if (m_vl->m_max_decomp_th == 0)
					decomp_in_this_thread = true;
				else if (m_vl->m_max_decomp_th < 0 ||
					m_vl->m_running_decomp_th < m_vl->m_max_decomp_th)
				{
					VolumeDecompressorThread *dthread = new VolumeDecompressorThread(m_vl);
					if (dthread->Create() == wxTHREAD_NO_ERROR)
					{
						m_vl->m_pThreadCS.Enter();
						m_vl->m_decomp_queues.push_back(dq);
						m_vl->m_used_memory += bsize;
						b.brick->set_loading_state(true);
						m_vl->m_loaded[b.brick] = b;
						m_vl->m_pThreadCS.Leave();

						dthread->Run();
					}
					else
					{
						if (m_vl->m_running_decomp_th <= 0)
							decomp_in_this_thread = true;
						else
						{
							m_vl->m_pThreadCS.Enter();
							m_vl->m_decomp_queues.push_back(dq);
							m_vl->m_used_memory += bsize;
							b.brick->set_loading_state(true);
							m_vl->m_loaded[b.brick] = b;
							m_vl->m_pThreadCS.Leave();
						}
					}
				}
				else
				{
					m_vl->m_pThreadCS.Enter();
					m_vl->m_decomp_queues.push_back(dq);
					m_vl->m_used_memory += bsize;
					b.brick->set_loading_state(true);
					m_vl->m_loaded[b.brick] = b;
					m_vl->m_pThreadCS.Leave();
				}

				if (decomp_in_this_thread)
				{
/*					char *result = new char[bsize];
					if (TextureBrick::decompress_brick(result, dq.in_data, bsize, dq.in_size, dq.finfo->type))
					{
						m_vl->m_pThreadCS.Enter();
						delete[] dq.in_data;
						b.brick->set_brkdata(result);
						b.datasize = bsize;
						m_vl->m_used_memory += bsize;
						m_vl->m_loaded[b.brick] = b;
						m_vl->m_pThreadCS.Leave();
					}
					else
					{
						delete[] result;

						m_vl->m_pThreadCS.Enter();
						delete[] dq.in_data;
						m_vl->m_used_memory -= bsize;
						dq.b->set_drawn(dq.mode, true);
						m_vl->m_pThreadCS.Leave();
					}
*/				}
			}

		}
		else
		{
			size_t bsize = (size_t)(b.brick->nx())*(size_t)(b.brick->ny())*(size_t)(b.brick->nz())*(size_t)(b.brick->nb(0));
			b.datasize = bsize;

			m_vl->m_pThreadCS.Enter();
			if (m_vl->m_loaded.find(b.brick) != m_vl->m_loaded.end())
				m_vl->m_loaded[b.brick] = b;
			m_vl->m_pThreadCS.Leave();
		}
	}

	/*
	wxCommandEvent evt(wxEVT_VLTHREAD_COMPLETED, GetId());
	VolumeLoaderData *data = new VolumeLoaderData;
	data->m_cur_cat = 0;
	data->m_cur_item = 0;
	data->m_progress = 0;
	evt.SetClientData(data);
	wxPostEvent(m_pParent, evt);
	*/
	return (wxThread::ExitCode)0;
}

VolumeLoader::VolumeLoader()
{
	m_thread = NULL;
	m_running_decomp_th = 0;
	m_max_decomp_th = wxThread::GetCPUCount() - 1;
	if (m_max_decomp_th < 0)
		m_max_decomp_th = -1;
	m_memory_limit = 10000000LL;
	m_used_memory = 0LL;
}

VolumeLoader::~VolumeLoader()
{
	if (m_thread)
	{
		if (m_thread->IsAlive())
		{
			m_thread->Delete();
			if (m_thread->IsAlive()) m_thread->Wait();
		}
		delete m_thread;
	}
	RemoveAllLoadedBrick();
}

void VolumeLoader::Queue(VolumeLoaderData brick)
{
	wxCriticalSectionLocker enter(m_pThreadCS);
	m_queues.push_back(brick);
}

void VolumeLoader::ClearQueues()
{
	if (!m_queues.empty())
	{
		Abort();
		m_queues.clear();
	}
}

void VolumeLoader::Set(vector<VolumeLoaderData> vld)
{
	Abort();
	//StopAll();
	m_queues = vld;
}

void VolumeLoader::Abort()
{
	if (m_thread)
	{
		if (m_thread->IsAlive())
		{
			m_thread->Delete();
			if (m_thread->IsAlive()) m_thread->Wait();
		}
		delete m_thread;
		m_thread = NULL;
	}
}

void VolumeLoader::StopAll()
{
	Abort();

	while (m_running_decomp_th > 0)
	{
		wxMilliSleep(10);
	}
}

bool VolumeLoader::Run()
{
	Abort();
	//StopAll();
	if (!m_queued.empty())
		m_queued.clear();

	m_thread = new VolumeLoaderThread(this);
	if (m_thread->Create() != wxTHREAD_NO_ERROR)
	{
		delete m_thread;
		m_thread = NULL;
		return false;
	}
	m_thread->Run();

	return true;
}

void VolumeLoader::CleanupLoadedBrick()
{
	long long required = 0;

	for (int i = 0; i < m_queues.size(); i++)
	{
		TextureBrick *b = m_queues[i].brick;
		if (!m_queues[i].brick->isLoaded())
			required += (size_t)b->nx()*(size_t)b->ny()*(size_t)b->nz()*(size_t)b->nb(0);
	}

	vector<VolumeLoaderData> vd_undisp;
	vector<VolumeLoaderData> b_undisp;
	vector<VolumeLoaderData> b_drawn;
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
		for (int i = 0; i < vd_undisp.size(); i++)
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
		for (int i = 0; i < b_undisp.size(); i++)
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
		for (int i = 0; i < b_drawn.size(); i++)
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
		for (int i = m_queues.size() - 1; i >= 0; i--)
		{
			TextureBrick *b = m_queues[i].brick;
			if (b->isLoaded() && m_loaded.find(b) != m_loaded.end())
			{
				bool skip = false;
				for (int j = m_queued.size() - 1; j >= 0; j--)
				{
					if (m_queued[j].brick == b && !b->drawn(m_queued[j].mode))
						skip = true;
				}
				if (!skip)
				{
					b->freeBrkData();
					long long datasize = (size_t)(b->nx())*(size_t)(b->ny())*(size_t)(b->nz())*(size_t)(b->nb(0));
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
	StopAll();
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
	StopAll();
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

void VolumeLoader::GetPalams(long long &used_mem, int &running_decomp_th, int &queue_num, int &decomp_queue_num)
{
	long long us = 0;
	int ll = 0;
	/*	for(auto e : m_loaded)
	{
	if (e.second.brick->isLoaded())
	us += e.second.datasize;
	if (!e.second.brick->get_disp())
	ll++;
	}
	*/	used_mem = m_used_memory;
	running_decomp_th = m_running_decomp_th;
	queue_num = m_queues.size();
	decomp_queue_num = m_decomp_queues.size();
}

