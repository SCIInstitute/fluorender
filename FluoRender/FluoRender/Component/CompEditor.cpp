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
#include <CompEditor.h>
#include <Global.h>
#include <RenderCanvas.h>

using namespace flrd;

ComponentEditor::ComponentEditor() :
	m_id(0),
	m_id_empty(true)
{

}

ComponentEditor::~ComponentEditor()
{

}

std::string ComponentEditor::GetOutput()
{
	return m_output;
}

fluo::Color ComponentEditor::GetColor()
{
	fluo::Color c;
	if (m_id_empty)
		c = fluo::Color(1, 1, 1);
	else
	{
		if (m_id == 0)
			c = fluo::Color(0.094117674, 0.654901961, 0.709803922);
		else
		{
			int shuffle = 0;
			if (glbin_current.vol_data)
				shuffle = glbin_current.vol_data->GetShuffle();
			c = fluo::Color(m_id, shuffle);
		}
	}
	return c;
}

//wxColor ComponentEditor::GetWxColor()
//{
//	fluo::Color c = GetColor();
//	wxColor wxc = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
//	return wxc;
//}

void ComponentEditor::Clean(int mode)
{
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;

	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Nrrd* nrrd_label = vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		switch (mode)
		{
		case 0:
		default:
			if (!data_mask[index])
				data_label[index] = 0;
			break;
		case 1:
			if (data_mask[index])
				data_label[index] = 0;
			break;
		}
	}
	vd->GetVR()->clear_tex_current();
}

void ComponentEditor::NewId(bool append, bool track)
{
	RenderCanvas* view = glbin_current.canvas;
	if (!view)
		return;

	//trace group
	TrackGroup *trkg = view->GetTrackGroup();
	if (!trkg)
	{
		view->CreateTrackGroup();
		trkg = view->GetTrackGroup();
	}

	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;
	Nrrd* nrrd_mask = 0;
	//get current mask
	nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
	{
		vd->AddEmptyMask(0);
		nrrd_mask = vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;

	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
	{
		vd->AddEmptyLabel();
		nrrd_label = tex->get_nrrd(tex->nlabel());
	}
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	//get ID of currently/previously masked region
	unsigned int id_vol = 0;
	if (m_id_empty)
	{
		for (index = 0; index < for_size; ++index)
		{
			if (data_mask[index] &&
				data_label[index])
			{
				id_vol = data_label[index];
				break;
			}
		}
	}

	//generate a unique ID
	unsigned int new_id = 0;
	unsigned int inc = 0;
	if (m_id_empty)
	{
		if (id_vol)
		{
			new_id = id_vol + 10;
			inc = 10;
		}
		else
		{
			new_id = 10;
			inc = 10;
		}
	}
	else
	{
		if (m_id)
		{
			new_id = m_id;
			inc = 253;
		}
		else
		{
			new_id = 0;
			inc = 0;
		}
	}
	unsigned int stop_id = new_id;
	if (inc)
	{
		while (vd->SearchLabel(new_id))
		{
			new_id += inc;
			if (new_id == stop_id)
			{
				m_output = "ID assignment failed. Type a different ID than " +
					std::to_string(stop_id);
				return;
			}
		}
	}

	//update label volume, set mask region to the new ID
	int i, j, k;
	Celp cell;
	if (new_id)
		cell = Celp(new Cell(new_id));
	for (i = 0; i < nx; ++i)
		for (j = 0; j < ny; ++j)
			for (k = 0; k < nz; ++k)
			{
				index = nx * ny*k + nx * j + i;
				if (data_mask[index])
				{
					if (append && data_label[index])
						continue;
					data_label[index] = new_id;
					if (new_id)
						cell->Inc(i, j, k, 1.0f);
				}
			}

	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();

	//save label mask to disk
	int cur_time = view->m_tseq_cur_num;
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());

	if (new_id && track)
	{
		//trkg->AddCell(cell, m_cur_time);
		pTrackMap track_map = trkg->GetTrackMap();
		glbin_trackmap_proc.SetTrackMap(track_map);
		//register file reading and deleteing functions
		glbin_reg_cache_queue_func(this, ComponentEditor::ReadVolCache, ComponentEditor::DelVolCache);
		glbin_cache_queue.set_max_size(4);
		//add
		cell->Calc();
		glbin_trackmap_proc.AddCellDup(cell, cur_time);
	}
}

void ComponentEditor::ReplaceId()
{
	if (m_id_empty)
		return;
	if (!m_id)
		return;
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;

	int cur_time = glbin_moviemaker.GetSeqCurNum();
	//get current mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	unsigned int old_id;
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		old_id = data_label[index];
		if (!data_mask[index] ||
			!old_id ||
			old_id == m_id)
			continue;

		data_label[index] = m_id;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());
}

void ComponentEditor::ReplaceList()
{
	if (m_id_empty)
		return;
	if (!m_id)
		return;
	RenderCanvas* view = glbin_current.canvas;
	if (!view)
		return;
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;
	TrackGroup *trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;

	//trace group
	glbin_trackmap_proc.SetTrackMap(trkg->GetTrackMap());
	bool track_map = trkg && trkg->GetTrackMap()->GetFrameNum();
	int cur_time = glbin_moviemaker.GetSeqCurNum();

	//get current mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//replace ID
	CelpListIter cell_iter;
	std::unordered_map<unsigned int, unsigned int> list_rep;
	std::unordered_map<unsigned int, unsigned int>::iterator list_rep_iter;
	unsigned int old_id, new_id;
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		old_id = data_label[index];
		if (!data_mask[index] ||
			!old_id ||
			old_id == m_id)
			continue;

		list_rep_iter = list_rep.find(old_id);
		if (list_rep_iter != list_rep.end())
		{
			data_label[index] = list_rep_iter->second;
			continue;
		}

		cell_iter = m_list.find(old_id);
		if (cell_iter != m_list.end())
		{
			new_id = m_id;
			while (vd->SearchLabel(new_id))
				new_id += 253;
			//add cell to list_rep
			list_rep.insert(std::pair<unsigned int, unsigned int>
				(old_id, new_id));
			glbin_trackmap_proc.ReplaceCellID(old_id, new_id,
					cur_time);
			data_label[index] = new_id;
		}
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());
}

void ComponentEditor::CombineId()
{
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;
	int cur_time = glbin_moviemaker.GetSeqCurNum();
	//get current mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//combine IDs
	unsigned int id = 0;
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (!data_mask[index] ||
			!data_label[index])
			continue;
		if (!id)
			id = data_label[index];
		if (data_label[index] != id)
			data_label[index] = id;
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());
}

void ComponentEditor::CombineList()
{
	RenderCanvas* view = glbin_current.canvas;
	if (!view)
		return;
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;

	if (m_list.size() <= 1)
		return;//nothing to combine
	//trace group
	TrackGroup *trkg = glbin_current.GetTrackGroup();
	if (!trkg)
		return;
	glbin_trackmap_proc.SetTrackMap(trkg->GetTrackMap());
	int cur_time = glbin_moviemaker.GetSeqCurNum();

	//find the largest cell in the list
	flrd::Celp cell;
	flrd::CelpListIter cell_iter;
	for (cell_iter = m_list.begin();
		cell_iter != m_list.end(); ++cell_iter)
	{
		if (cell)
		{
			if (cell_iter->second->GetSizeUi() >
				cell->GetSizeUi())
				cell = cell_iter->second;
		}
		else
			cell = cell_iter->second;
	}
	if (!cell)
		return;

	//get current mask
	Nrrd* nrrd_mask = vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	flvr::Texture* tex = vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//combine IDs
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (!data_mask[index] ||
			!data_label[index])
			continue;
		cell_iter = m_list.find(data_label[index]);
		if (cell_iter != m_list.end())
			data_label[index] = cell->Id();
	}
	//invalidate label mask in gpu
	vd->GetVR()->clear_tex_current();
	//save label mask to disk
	vd->SaveLabel(true, cur_time, vd->GetCurChannel());

	//modify graphs
	glbin_trackmap_proc.CombineCells(cell, m_list,
		cur_time);
}

//read/delete volume cache
void ComponentEditor::ReadVolCache(VolCache& vol_cache)
{
	//get volume, readers
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	int cur_time = glbin_moviemaker.GetSeqCurNum();
	int chan = vd->GetCurChannel();
	int frame = vol_cache.frame;

	if (frame == cur_time)
	{
		flvr::Texture* tex = vd->GetTexture();
		if (!tex)
			return;

		Nrrd* data = tex->get_nrrd(0);
		vol_cache.nrrd_data = data;
		vol_cache.data = data->data;
		Nrrd* label = tex->get_nrrd(tex->nlabel());
		vol_cache.nrrd_label = label;
		vol_cache.label = label->data;
		if (data && label)
			vol_cache.valid = true;
	}
	else
	{
		Nrrd* data = reader->Convert(frame, chan, true);
		if (!data)
			return;
		vol_cache.nrrd_data = data;
		vol_cache.data = data->data;
		std::wstring lblname = reader->GetCurLabelName(frame, chan);
		lbl_reader.SetFile(lblname);
		Nrrd* label = lbl_reader.Convert(frame, chan, true);
		if (!label)
			return;
		vol_cache.nrrd_label = label;
		vol_cache.label = label->data;
		if (data && label)
			vol_cache.valid = true;
	}
}

void ComponentEditor::DelVolCache(VolCache& vol_cache)
{
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	int chan = vd->GetCurChannel();
	int frame = vol_cache.frame;
	int cur_time = glbin_moviemaker.GetSeqCurNum();

	if (vol_cache.valid && vol_cache.modified)
	{
		//save it first if modified
		//assume that only label is modified
		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)vol_cache.nrrd_label);
		double spcx, spcy, spcz;
		vd->GetSpacings(spcx, spcy, spcz);
		msk_writer.SetSpacings(spcx, spcy, spcz);
		std::wstring filename = reader->GetCurLabelName(frame, chan);
		msk_writer.Save(filename, 1);
	}

	vol_cache.valid = false;
	if (frame != cur_time)
	{
		if (vol_cache.data)
			nrrdNuke((Nrrd*)vol_cache.nrrd_data);
		if (vol_cache.label)
			nrrdNuke((Nrrd*)vol_cache.nrrd_label);
	}
	vol_cache.data = 0;
	vol_cache.nrrd_data = 0;
	vol_cache.label = 0;
	vol_cache.nrrd_label = 0;
}

