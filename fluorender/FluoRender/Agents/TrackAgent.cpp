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

#include <TrackAgent.hpp>
#include <TrackDlg.h>
#include <Tracks.h>
#include <TextureRenderer.h>

using namespace fluo;

TrackAgent::TrackAgent(TrackDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void TrackAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* TrackAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void TrackAgent::UpdateAllSettings()
{
	m_view = vrv;
	if (!m_view)
		return;

	m_view->getValue(gstCurrentFrame, m_cur_time);
	m_view->getValue(gstPreviousFrame, m_prv_time);

	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		wxString str;
		//cell size filter
		str = m_cell_size_text->GetValue();
		unsigned long ival;
		str.ToULong(&ival);
		trace_group->SetCellSize(ival);

		str = trace_group->GetPath();
		if (str != "")
			m_load_trace_text->SetValue(str);
		else
			m_load_trace_text->SetValue("Track map created but not saved");
		UpdateList();

		int ghost_num = trace_group->GetGhostNum();
		m_ghost_num_text->ChangeValue(wxString::Format("%d", ghost_num));
		m_ghost_num_sldr->SetValue(ghost_num);
		m_ghost_show_tail_chk->SetValue(trace_group->GetDrawTail());
		m_ghost_show_lead_chk->SetValue(trace_group->GetDrawLead());
	}
	else
	{
		m_load_trace_text->SetValue("No Track map");
	}

	//settings for tracking
	if (m_frame && m_frame->GetSettingDlg())
	{
		m_iter_num =
			m_frame->GetSettingDlg()->GetTrackIter();
		m_size_thresh =
			m_frame->GetSettingDlg()->GetComponentSize();
		m_consistent_color =
			m_frame->GetSettingDlg()->GetConsistentColor();
		m_try_merge =
			m_frame->GetSettingDlg()->GetTryMerge();
		m_try_split =
			m_frame->GetSettingDlg()->GetTrySplit();
		m_similarity =
			m_frame->GetSettingDlg()->GetSimilarity();
		m_contact_factor =
			m_frame->GetSettingDlg()->GetContactFactor();
		//
		m_map_iter_spin->SetValue(m_iter_num);
		m_map_size_spin->SetValue(m_size_thresh);
		m_map_consistent_btn->SetValue(m_consistent_color);
		m_map_merge_btn->SetValue(m_try_merge);
		m_map_split_btn->SetValue(m_try_split);
		m_map_similar_spin->SetValue(m_similarity);
		m_map_contact_spin->SetValue(m_contact_factor);
	}
}

void TrackAgent::UpdateTraces()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* traces = view->GetTraceGroup();
	if (!traces) return;
	int shuffle = 0;
	VolumeData* vd = view->GetCurrentVolume();
	if (vd) shuffle = vd->GetShuffle();

	dlg_.m_trace_list_curr->DeleteAllItems();

	flrd::CelpList sel_cells = traces->GetCellList();
	std::vector<flrd::Celp> cells;
	for (auto siter = sel_cells.begin();
		siter != sel_cells.end(); ++siter)
		cells.push_back(siter->second);

	if (cells.empty())
		return;
	else
		std::sort(cells.begin(), cells.end(),
			[](const flrd::Celp &c1, const flrd::Celp &c2) -> bool
	{
		unsigned int vid1 = c1->GetVertexId();
		unsigned int vid2 = c2->GetVertexId();
		if (vid1 == vid2)
			return c1->GetSizeUi() > c2->GetSizeUi();
		else
			return vid1 < vid2;
	});

	wxString gtype;
	unsigned int id;
	unsigned int vid;
	Color c;
	wxColor wxc;
	int size;
	Point center;
	bool prev, next;

	for (size_t i = 0; i < cells.size(); ++i)
	{
		id = cells[i]->Id();
		vid = cells[i]->GetVertexId();
		c = Color(id, shuffle);
		wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
		size = (int)(cells[i]->GetSizeUi());
		center = cells[i]->GetCenter();

		if (vid == 0)
			gtype = L"\u25ef";
		else
		{
			if (i == 0)
				prev = false;
			else
				prev = cells[i - 1]->GetVertexId() == vid;
			if (i == cells.size() - 1)
				next = false;
			else
				next = cells[i + 1]->GetVertexId() == vid;
			if (prev)
			{
				if (next)
					gtype = L"\u2502";
				else
					gtype = L"\u2514";
			}
			else
			{
				if (next)
					gtype = L"\u250c";
				else
					gtype = L"\u2500";
			}
		}

		dlg_.m_trace_list_curr->Append(gtype, id, color, size,
			center.x(), center.y(), center.z());
	}
}

void TrackAgent::UpdateList()
{
	if (!m_view)
		return;

	int shuffle = 0;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (vd)
		shuffle = vd->GetShuffle();
	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (trace_group)
	{
		int cur_time = trace_group->GetCurTime();
		int prv_time = trace_group->GetPrvTime();
		wxString item_gtype;
		wxString item_id;
		wxString item_size;
		wxString item_x;
		wxString item_y;
		wxString item_z;
		unsigned long id;
		double hue;
		long size;
		double x, y, z;
		//copy current to previous
		if (cur_time != prv_time)
		{
			m_trace_list_prev->DeleteAllItems();
			long item = -1;
			for (;;)
			{
				item = m_trace_list_curr->GetNextItem(item,
					wxLIST_NEXT_ALL,
					wxLIST_STATE_DONTCARE);
				if (item != -1)
				{
					item_gtype = m_trace_list_curr->GetText(item, 0);
					item_id = m_trace_list_curr->GetText(item, 1);
					item_size = m_trace_list_curr->GetText(item, 2);
					item_x = m_trace_list_curr->GetText(item, 3);
					item_y = m_trace_list_curr->GetText(item, 4);
					item_z = m_trace_list_curr->GetText(item, 5);
					item_id.ToULong(&id);
					fluo::Color c(id, shuffle);
					wxColor color(c.r() * 255, c.g() * 255, c.b() * 255);
					item_size.ToLong(&size);
					item_x.ToDouble(&x);
					item_y.ToDouble(&y);
					item_z.ToDouble(&z);
					m_trace_list_prev->Append(item_gtype, id, color, size, x, y, z);
				}
				else break;
			}

			if (cur_time >= 0) m_cur_time = cur_time;
			if (prv_time >= 0) m_prv_time = prv_time;

			//set tiem text
			wxString str;
			str = wxString::Format("\tCurrent T: %d", m_cur_time);
			m_cell_time_curr_st->SetLabel(str);
			if (m_prv_time != m_cur_time)
				m_cell_time_prev_st->SetLabel(
					wxString::Format("\tPrevious T: %d", m_prv_time));
			else
				m_cell_time_prev_st->SetLabel("\tPrevious T");
		}
	}
	UpdateTraces();
	dlg_.Layout();
}

void TrackAgent::AddLabel(long item, TraceListCtrl* trace_list_ctrl, flrd::CelpList &list)
{
	wxString str;
	unsigned long id;
	unsigned long size;
	double x, y, z;

	str = trace_list_ctrl->GetText(item, 1);
	str.ToULong(&id);
	str = trace_list_ctrl->GetText(item, 2);
	str.ToULong(&size);
	str = trace_list_ctrl->GetText(item, 3);
	str.ToDouble(&x);
	str = trace_list_ctrl->GetText(item, 4);
	str.ToDouble(&y);
	str = trace_list_ctrl->GetText(item, 5);
	str.ToDouble(&z);

	flrd::Celp cell(new flrd::Cell(id));
	cell->SetSizeUi(size);
	cell->SetSizeD(size);
	fluo::Point p(x, y, z);
	cell->SetCenter(p);
	list.insert(std::pair<unsigned int, flrd::Celp>
		(id, cell));
}

//ID link controls
void TrackAgent::CellUpdate()
{
	if (m_view)
	{
		wxString str = m_ghost_num_text->GetValue();
		long ival;
		str.ToLong(&ival);
		flrd::Tracks* trace_group = m_view->GetTraceGroup();
		if (trace_group)
			trace_group->SetGhostNum(ival);
		else
			m_view->CreateTraceGroup();

		m_view->GetTraces(false);
		m_view->Update(39);
		GetSettings(m_view);
	}
}

void TrackAgent::CellFull()
{
	if (!m_view)
		return;

	//cell size filter
	wxString str = m_cell_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	unsigned int slimit = (unsigned int)ival;
	//get current mask
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.SetMinNum(true, slimit);
	comp_selector.CompFull();
	//update view
	CellUpdate();

	//frame
	glbin_agtf->getBrushToolAgent(gstBrushToolAgent)->UpdateUndoRedo();
}

void TrackAgent::CellLink(bool exclusive)
{
	if (!m_view)
		return;

	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
	{
		m_view->CreateTraceGroup();
		trace_group = m_view->GetTraceGroup();
	}

	//get selections
	long item;
	//current T
	flrd::CelpList list_cur;
	//previous T
	flrd::CelpList list_prv;
	//current list
	item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}
	//previous list
	item = -1;
	while (true)
	{
		item = m_trace_list_prev->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_prev, list_prv);
	}
	if (list_prv.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_prev->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_prev, list_prv);
		}
	}
	if (list_cur.size() == 0 ||
		list_prv.size() == 0)
		return;

	//link them
	trace_group->LinkCells(list_cur, list_prv,
		m_cur_time, m_prv_time, exclusive);

	//update view
	m_view->Update(39);
}

void TrackAgent::CellNewID(bool append)
{
	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.NewId(m_cell_new_id,
		m_cell_new_id_empty, append);
	CellUpdate();
}

void TrackAgent::CellEraseID()
{
	if (!m_view)
		return;

	//trace group
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
	{
		m_view->CreateTraceGroup();
		trace_group = m_view->GetTraceGroup();
	}

	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	//get prev mask
	Nrrd* nrrd_mask = vd->GetMask(false);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;

	//get prev label
	Nrrd* nrrd_label = vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	long nx, ny, nz;
	vd->getValue(gstResX, nx);
	vd->getValue(gstResY, ny);
	vd->getValue(gstResZ, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;

	//find old IDs
	set<unsigned int> id_list;
	for (index = 0; index < for_size; ++index)
	{
		if (data_mask[index] && data_label[index])
			id_list.insert(data_label[index]);
	}

	if (!id_list.empty())
	{
		//current mask
		nrrd_mask = vd->GetMask(true);
		if (!nrrd_mask)
			return;
		data_mask = (unsigned char*)(nrrd_mask->data);
		if (!data_mask)
			return;

		for (index = 0; index < for_size; ++index)
		{
			if (data_label[index] &&
				id_list.find(data_label[index])
				!= id_list.end() &&
				!data_mask[index])
				data_label[index] = 0;
		}

		//invalidate label mask in gpu
		vd->GetRenderer()->clear_tex_current();
		//save label mask to disk
		long chan;
		vd->getValue(gstChannel, chan);
		vd->SaveLabel(true, m_cur_time, chan);
	}

	CellUpdate();
}

void TrackAgent::CellReplaceID()
{
	//current T
	flrd::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}

	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.Replace(m_cell_new_id,
		m_cell_new_id_empty, list_cur);

	CellUpdate();
}

void TrackAgent::CellCombineID()
{
	//current T
	flrd::CelpList list_cur;
	//fill current list
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_cur);
	}
	if (list_cur.empty())
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_cur);
		}
	}

	flrd::ComponentEditor editor;
	editor.SetView(m_view);
	editor.Combine(list_cur);

	//update view
	CellUpdate();
}

//Component tools
void TrackAgent::CompDelete()
{
	if (!m_view)
		return;

	long item = -1;
	wxString str;
	unsigned long ival;
	vector<unsigned long long> ids;
	for (;;)
	{
		item = m_trace_list_curr->GetNextItem(item,
			wxLIST_NEXT_ALL,
			wxLIST_STATE_DONTCARE);

		if (item == -1)
			break;
		else if (m_trace_list_curr->
			GetItemState(item, wxLIST_STATE_SELECTED)
			== wxLIST_STATE_SELECTED)
			continue;
		else
		{
			str = m_trace_list_curr->GetItemText(item, 1);
			if (str.ToULong(&ival) && ival)
				ids.push_back(ival);
		}
	}

	//get current vd
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	if (ids.size() == 1)
	{
		comp_selector.SetId(ids[0]);
		comp_selector.Delete();
	}
	else
		comp_selector.Delete(ids);

	//update view
	CellUpdate();

	//frame
	glbin_agtf->getBrushToolAgent(gstBrushToolAgent)->UpdateUndoRedo();
}

void TrackAgent::CompClear()
{
	if (!m_view)
		return;

	//get current vd
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.Clear();

	//update view
	CellUpdate();
	m_trace_list_prev->DeleteAllItems();

	//frame
	glbin_agtf->findFirst(gstBrushToolAgent)->asBrushToolAgent()->UpdateUndoRedo();
}

//uncertainty filter
void TrackAgent::UncertainFilter(bool input)
{
	if (!m_view)
		return;
	//trace group
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	flrd::CelpList list_in, list_out;

	//fill inlist
	if (input)
	{
		long item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_in);
		}
		if (list_in.size() == 0)
		{
			item = -1;
			while (true)
			{
				item = m_trace_list_curr->GetNextItem(
					item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
				if (item == -1)
					break;
				else
					AddLabel(item, m_trace_list_curr, list_in);
			}
		}
		if (list_in.empty())
			return;
	}

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	wxString str = m_comp_uncertain_low_text->GetValue();
	long ival;
	str.ToLong(&ival);
	tm_processor.SetUncertainLow(ival);
	tm_processor.GetCellsByUncertainty(list_in, list_out, m_cur_time);

	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentSelector comp_selector(vd);
	comp_selector.SelectList(list_out);

	//update view
	CellUpdate();

	//frame
	glbin_agtf->getBrushToolAgent(gstBrushToolAgent)->UpdateUndoRedo();
}

void TrackAgent::LinkAddedCells(flrd::CelpList &list)
{
	if (!m_view)
		return;

	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(3);
	tm_processor.LinkAddedCells(list, m_cur_time, m_cur_time - 1);
	tm_processor.LinkAddedCells(list, m_cur_time, m_cur_time + 1);
	RefineMap(true, false);
}

void TrackAgent::ClearTrack()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group) return;

	trace_group->Clear();
	dlg_.m_load_trace_text->SetValue("No Track map");
}

void TrackAgent::SaveOutputResult(wxString &filename)
{
	std::ofstream os;
	OutputStreamOpen(os, filename.ToStdString());

	wxString str;
	str = m_stat_text->GetValue();

	os << str;

	os.close();
}

//auto tracking
void TrackAgent::GenMap()
{
	if (!m_view)
		return;

	//get trace group
	m_view->CreateTraceGroup();
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;

	//start progress
	m_stat_text->SetValue("Generating track map.\n");
	wxGetApp().Yield();
	int frames = reader->GetTimeNum();

	//get and set parameters
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetSizeThresh(m_size_thresh);
	tm_processor.SetContactThresh(m_contact_factor);
	tm_processor.SetSimilarThresh(m_similarity);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	tm_processor.SetMerge(m_try_merge);
	tm_processor.SetSplit(m_try_split);

	//start timing
	std::chrono::high_resolution_clock::time_point t1 =
		std::chrono::high_resolution_clock::now();
	//initialization
	for (int i = 0; i < frames; ++i)
	{
		tm_processor.InitializeFrame(i);
		(*m_stat_text) << wxString::Format("Time point %d initialized.\n", i);
		wxGetApp().Yield();

		if (i < 1)
			continue;

		//link maps 1 and 2
		tm_processor.LinkFrames(i - 1, i);
		(*m_stat_text) << wxString::Format("Time point %d linked.\n", i);
		wxGetApp().Yield();

		//check contacts and merge cells
		tm_processor.ResolveGraph(i - 1, i);
		tm_processor.ResolveGraph(i, i - 1);
		(*m_stat_text) << wxString::Format("Time point %d merged.\n", i - 1);
		wxGetApp().Yield();

		if (i < 2)
			continue;

		//further process
		tm_processor.ProcessFrames(i - 2, i - 1);
		tm_processor.ProcessFrames(i - 1, i - 2);
		(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
		wxGetApp().Yield();
	}
	//last frame
	tm_processor.ProcessFrames(frames - 2, frames - 1);
	tm_processor.ProcessFrames(frames - 1, frames - 2);
	(*m_stat_text) << wxString::Format("Time point %d processed.\n", frames - 1);
	wxGetApp().Yield();

	//iterations
	for (size_t iteri = 0; iteri < m_iter_num; ++iteri)
	{
		for (int i = 2; i <= frames; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i - 2, i - 1);
			tm_processor.ProcessFrames(i - 1, i - 2);
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i - 1);
			wxGetApp().Yield();
		}
	}

	//consistent colors
	if (m_consistent_color)
	{
		(*m_stat_text) << wxString::Format("Set colors for frame 0\n");
		wxGetApp().Yield();
		tm_processor.MakeConsistent(0);
		//remaining frames
		for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
		{
			(*m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
			wxGetApp().Yield();
			tm_processor.MakeConsistent(fi - 1, fi);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 =
		std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	GetSettings(m_view);

	//CellUpdate();
}

void TrackAgent::RefineMap(bool cur_time, bool erase_v)
{
	if (!m_view)
		return;

	//get trace group
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (t < 0)
		m_stat_text->SetValue("Refining track map for all time points.\n");
	else
		m_stat_text->SetValue(wxString::Format(
			"Refining track map at time point %d.\n", t));
	wxGetApp().Yield();

	//start progress
	bool clear_counters = false;
	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	int start_frame, end_frame;
	if (t < 0)
	{
		start_frame = 0;
		end_frame = track_map->GetFrameNum() - 1;
		clear_counters = true;
	}
	else
		start_frame = end_frame = t;

	//get and set parameters
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetSizeThresh(m_size_thresh);
	tm_processor.SetContactThresh(m_contact_factor);
	tm_processor.SetSimilarThresh(m_similarity);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(4);
	//merge/split
	tm_processor.SetMerge(m_try_merge);
	tm_processor.SetSplit(m_try_split);

	std::chrono::high_resolution_clock::time_point t1 =
		std::chrono::high_resolution_clock::now();

	//not sure if counters need to be cleared for all refinement
	//if (clear_counters)
	//	tm_processor.ClearCounters();
	//iterations
	for (size_t iteri = 0; iteri < m_iter_num; ++iteri)
	{
		for (int i = start_frame - 1; i <= end_frame; ++i)
		{
			//further process
			tm_processor.ProcessFrames(i, i + 1, erase_v);
			tm_processor.ProcessFrames(i + 1, i, erase_v);
			(*m_stat_text) << wxString::Format("Time point %d processed.\n", i + 1);
			wxGetApp().Yield();
		}
	}

	//consistent colors
	if (m_consistent_color)
	{
		if (t < 0)
		{
			(*m_stat_text) << wxString::Format("Set colors for frame 0\n");
			wxGetApp().Yield();
			tm_processor.MakeConsistent(0);
			//remaining frames
			for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
			{
				(*m_stat_text) << wxString::Format("Set colors for frame %d\n", int(fi));
				wxGetApp().Yield();
				tm_processor.MakeConsistent(fi - 1, fi);
			}
		}
		else
		{
			tm_processor.MakeConsistent(t - 1, t);
		}
	}

	std::chrono::high_resolution_clock::time_point t2 =
		std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span =
		std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	(*m_stat_text) << wxString::Format("Wall clock time: %.4fs\n", time_span.count());

	CellUpdate();
}

int TrackAgent::GetTrackFileExist(bool save)
{
	if (!m_view) return 0;
	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return 0;
	std::wstring filename = trace_group->GetPath();
	if (wxFileExists(filename))
	{
		if (save)
		{
			m_view->SaveTraceGroup(filename);
			m_track_file = filename;
		}
		return 2;
	}
	else
		return 1;
}

void TrackAgent::LoadTrackFile(const std::wstring &file)
{
	if (!m_view) return;
	int rval = m_view->LoadTraceGroup(file);
	if (rval)
	{
		m_load_trace_text->SetValue(file);
		m_track_file = file;
	}
}

void TrackAgent::SaveTrackFile(const std::wstring &file)
{
	if (getObject()->SaveTraceGroup(file))
	{
		dlg_.m_load_trace_text->SetValue(file);
		setValue(gstTrackFile, file);
	}
}

void TrackAgent::ReadVolCache(flrd::VolCache& vol_cache)
{
	//get volume, readers
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	long chan;
	vd->getValue(gstChannel, chan);
	int frame = vol_cache.frame;
	long lval;
	m_view->getValue(gstCurrentFrame, lval);
	if (frame == lval)
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
		wstring lblname = reader->GetCurLabelName(frame, chan);
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

void TrackAgent::DelVolCache(flrd::VolCache& vol_cache)
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	BaseReader* reader = vd->GetReader();
	if (!reader)
		return;
	long chan;
	vd->getValue(gstChannel, chan);
	int frame = vol_cache.frame;

	if (vol_cache.valid && vol_cache.modified)
	{
		//save it first if modified
		//assume that only label is modified
		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)vol_cache.nrrd_label);
		double spcx, spcy, spcz;
		vd->getValue(gstSpcX, spcx);
		vd->getValue(gstSpcY, spcy);
		vd->getValue(gstSpcZ, spcz);
		msk_writer.SetSpacings(spcx, spcy, spcz);
		wstring filename = reader->GetCurLabelName(frame, chan);
		msk_writer.Save(filename, 1);
	}

	vol_cache.valid = false;
	long lval;
	m_view->getValue(gstCurrentFrame, lval);
	if (frame != lval)
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

void TrackAgent::OnTrackCellSize(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	long lval;
	getValue(gstTrackCellSize, lval);
	trace_group->SetCellSize(lval);
}

void TrackAgent::OnGhostNum(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	long lval;
	getValue(gstGhostNum, lval);
	trace_group->SetGhostNum(lval);
	//m_view->Update(39);
}

void TrackAgent::OnGhostTailEnable(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	bool bval;
	getValue(gstGhostTailEnable, bval);
	trace_group->SetDrawTail(bval);
	//m_view->Update(39);
}

void TrackAgent::OnGhostLeadEnable(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	bool bval;
	getValue(gstGhostLeadEnable, bval);
	trace_group->SetDrawLead(bval);
	//m_view->Update(39);
}

void TrackAgent::OnCompUncertainLow(Event& event)
{
	flrd::Tracks* trace_group = getObject()->GetTraceGroup();
	if (!trace_group) return;

	long lval;
	getValue(gstCompUncertainLow, lval);
	trace_group->SetUncertainLow(lval);
	UncertainFilter(true);
}

void TrackAgent::ConvertToRulers()
{
	Renderview* view = getObject();
	if (!view) return;
	flrd::Tracks* trace_group = view->GetTraceGroup();
	if (!trace_group) return;
	fluo::VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;

	double spcx, spcy, spcz;
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);

	//get rulers
	flrd::RulerList rulers;
	trace_group->GetMappedRulers(rulers);
	for (auto iter = rulers.begin();
		iter != rulers.end(); ++iter)
	{
		(*iter)->Scale(spcx, spcy, spcz);
		view->GetRulerList()->push_back(*iter);
	}
	//m_view->Update(39);
	//if (m_frame && m_frame->GetMeasureDlg())
	//	m_frame->GetMeasureDlg()->GetSettings(m_view);
	flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Rulers);
}

void TrackAgent::ConvertConsistent()
{
	if (!m_view)
		return;

	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;

	m_stat_text->SetValue("Generating consistent IDs in");
	wxGetApp().Yield();

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	double spcx, spcy, spcz;
	vd->getValue(gstResX, resx);
	vd->getValue(gstResY, resy);
	vd->getValue(gstResZ, resz);
	vd->getValue(gstSpcX, spcx);
	vd->getValue(gstSpcY, spcy);
	vd->getValue(gstSpcZ, spcz);
	long lval;
	vd->getValue(gstBits, lval);
	tm_processor.SetBits(lval);
	double dval;
	vd->getValue(gstIntScale, dval);
	tm_processor.SetScale(dval);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&TrackDlg::ReadVolCache, this, std::placeholders::_1),
		std::bind(&TrackDlg::DelVolCache, this, std::placeholders::_1));
	tm_processor.SetVolCacheSize(2);

	(*m_stat_text) << wxString::Format("Frame %d\n", 0);
	wxGetApp().Yield();
	tm_processor.MakeConsistent(0);

	//remaining frames
	for (size_t fi = 1; fi < track_map->GetFrameNum(); ++fi)
	{
		(*m_stat_text) << wxString::Format("Frame %d\n", int(fi));
		wxGetApp().Yield();
		tm_processor.MakeConsistent(fi - 1, fi);
	}

	CellUpdate();
}

void TrackAgent::AnalyzeComp()
{
	if (!m_view)
		return;
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	flrd::ComponentAnalyzer comp_analyzer(vd);
	comp_analyzer.Analyze(true, true);
	string str;
	comp_analyzer.OutputCompListStr(str, 1);
	m_stat_text->SetValue(str);
}

void TrackAgent::AnalyzeLink()
{
	if (!m_view)
		return;

	flrd::Tracks* trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	size_t frames = trace_group->GetTrackMap()->GetFrameNum();
	if (frames == 0)
		m_stat_text->SetValue("ERROR! Generate a track map first.\n");
	else
		m_stat_text->SetValue(
			wxString::Format("Time point number: %d\n", int(frames)));

	(*m_stat_text) << "Time\tIn Orphan\tOut Orphan\tIn Multi\tOut Multi\n";
	flrd::VertexList in_orphan_list;
	flrd::VertexList out_orphan_list;
	flrd::VertexList in_multi_list;
	flrd::VertexList out_multi_list;
	for (size_t fi = 0; fi < frames; ++fi)
	{
		trace_group->GetLinkLists(fi,
			in_orphan_list, out_orphan_list,
			in_multi_list, out_multi_list);
		(*m_stat_text) << int(fi) << "\t" <<
			int(in_orphan_list.size()) << "\t" <<
			int(out_orphan_list.size()) << "\t" <<
			int(in_multi_list.size()) << "\t" <<
			int(out_multi_list.size()) << "\n";
	}
}

void TrackAgent::AnalyzeUncertainHist()
{
	if (!m_view)
		return;
	//trace group
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	flrd::CelpList list_in;
	//fill inlist
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_in);
	}
	if (list_in.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_in);
		}
	}

	m_stat_text->SetValue("");

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	if (list_in.empty())
	{
		flrd::UncertainHist hist1, hist2;
		tm_processor.GetUncertainHist(hist1, hist2, m_cur_time);
		//header
		(*m_stat_text) << "In\n";
		(*m_stat_text) << "Level\t" << "Frequency\n";
		int count = 0;
		for (auto iter = hist1.begin();
			iter != hist1.end(); ++iter)
		{
			while (iter->second.level > count)
			{
				(*m_stat_text) << count++ << "\t" << "0\n";
			}
			(*m_stat_text) << int(iter->second.level) << "\t" <<
				int(iter->second.count) << "\n";
			count++;
		}

		//header
		(*m_stat_text) << "\n";
		(*m_stat_text) << "Out\n";
		(*m_stat_text) << "Level\t" << "Frequency\n";
		count = 0;
		for (auto iter = hist2.begin();
			iter != hist2.end(); ++iter)
		{
			while (iter->second.level > count)
			{
				(*m_stat_text) << count++ << "\t" << "0\n";
			}
			(*m_stat_text) << int(iter->second.level) << "\t" <<
				int(iter->second.count) << "\n";
			count++;
		}
	}
	else
	{
		tm_processor.GetCellUncertainty(list_in, m_cur_time);
		//header
		(*m_stat_text) << "ID\t" << "In\t" << "Out\n";
		for (auto iter = list_in.begin();
			iter != list_in.end(); ++iter)
		{
			wxString sid = wxString::Format("%u", iter->second->Id());
			(*m_stat_text) << sid << "\t" <<
				int(iter->second->GetCount0()) << "\t" <<
				int(iter->second->GetCount1()) << "\n";
		}
	}
}

void TrackAgent::AnalyzePath()
{
	if (!m_view)
		return;
	//trace group
	flrd::Tracks *trace_group = m_view->GetTraceGroup();
	if (!trace_group)
		return;
	if (!trace_group->GetTrackMap()->GetFrameNum())
		return;
	flrd::CelpList list_in;
	//fill inlist
	long item = -1;
	while (true)
	{
		item = m_trace_list_curr->GetNextItem(
			item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;
		else
			AddLabel(item, m_trace_list_curr, list_in);
	}
	if (list_in.size() == 0)
	{
		item = -1;
		while (true)
		{
			item = m_trace_list_curr->GetNextItem(
				item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
			if (item == -1)
				break;
			else
				AddLabel(item, m_trace_list_curr, list_in);
		}
	}

	m_stat_text->SetValue("");

	flrd::pTrackMap track_map = trace_group->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	if (list_in.empty())
		return;

	m_stat_text->SetValue("");
	std::ostream os(m_stat_text);

	if (m_cur_time > 0)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time - 1 << ":\n";
		flrd::PathList paths_prv;
		tm_processor.GetPaths(list_in, paths_prv, m_cur_time, m_cur_time - 1);
		for (size_t i = 0; i < paths_prv.size(); ++i)
			os << paths_prv[i];
	}
	if (m_cur_time < track_map->GetFrameNum() - 1)
	{
		(*m_stat_text) << "Paths of T" << m_cur_time << " to T" << m_cur_time + 1 << ":\n";
		flrd::PathList paths_nxt;
		tm_processor.GetPaths(list_in, paths_nxt, m_cur_time, m_cur_time + 1);
		for (size_t i = 0; i < paths_nxt.size(); ++i)
			os << paths_nxt[i];
	}
}

void TrackAgent::Shuffle()
{
	if (!m_view)
		return;

	//get current vd
	fluo::VolumeData* vd = m_view->GetCurrentVolume();
	if (!vd)
		return;

	vd->IncShuffle();
	m_view->Update(39);
}