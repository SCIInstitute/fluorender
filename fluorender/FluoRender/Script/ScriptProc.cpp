/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#include "ScriptProc.h"
#include <DataManager.h>
#include <VRenderView.h>
#include <VRenderGLView.h>
#include <VRenderFrame.h>
#include <Calculate/BackgStat.h>
#include <utility.h>
#include <wx/filefn.h>
#include <wx/stdpaths.h>

using namespace flrd;

ScriptProc::ScriptProc() :
	m_frame(0),
	m_vrv(0),
	m_view(0)
{
}

ScriptProc::~ScriptProc()
{
}

//run 4d script
void ScriptProc::Run4DScript(int index, wxString &scriptname)
{
	if (!wxFileExists(scriptname))
	{
		std::wstring name = scriptname.ToStdWstring();
		name = GET_NAME(name);
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		scriptname = exePath + "\\Scripts\\" + name;

		if (!wxFileExists(scriptname))
			return;
	}
	wxFileInputStream is(scriptname);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	int i;
	wxString str;

	//tasks
	if (fconfig.Exists("/tasks"))
	{
		fconfig.SetPath("/tasks");
		int tasknum = fconfig.Read("tasknum", 0l);
		for (i = 0; i < tasknum; i++)
		{
			str = wxString::Format("/tasks/task%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				fconfig.Read("type", &str, "");
				if (str == "noise_reduction")
					RunNoiseReduction(index, fconfig);
				else if (str == "selection_tracking")
					RunSelectionTracking(index, fconfig);
				else if (str == "mask_tracking")
					RunMaskTracking(index, fconfig);
				else if (str == "random_colors")
					RunRandomColors(index, fconfig);
				else if (str == "fetch_mask")
					RunFetchMask(index, fconfig);
				else if (str == "clear_mask")
					RunClearMask(index, fconfig);
				else if (str == "save_mask")
					RunSaveMask(index, fconfig);
				else if (str == "opencl")
					RunOpenCL(index, fconfig);
				else if (str == "comp_analysis")
					RunCompAnalysis(index, fconfig);
				else if (str == "generate_comp")
					RunGenerateComp(index, fconfig);
				else if (str == "ruler_profile")
					RunRulerProfile(index, fconfig);
				else if (str == "save_volume")
					RunSaveVolume(index, fconfig);
				else if (str == "calculate")
					RunCalculate(index, fconfig);
				else if (str == "add_cells")
					RunAddCells(index, fconfig);
				else if (str == "link_cells")
					RunLinkCells(index, fconfig);
				else if (str == "unlink_cells")
					RunUnlinkCells(index, fconfig);
				else if (str == "backg_stat")
					RunBackgroundStat(index, fconfig);
			}
		}
	}
}

void ScriptProc::RunNoiseReduction(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	if (!calculator) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	double thresh, size;
	fconfig.Read("threshold", &thresh, 0.0);
	fconfig.Read("voxelsize", &size, 0.0);

	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_view->m_cur_vol = *i;
		calculator->SetVolumeA(*i);

		//selection
		if (m_frame->GetNoiseCancellingDlg())
			m_frame->GetNoiseCancellingDlg()->Preview(false, size, thresh);
		//delete
		calculator->CalculateGroup(6, "", false);
	}
}

//add traces to trace dialog
#define UPDATE_TRACE_DLG_AND_RETURN \
	{ \
		if (m_vrv && m_frame && m_frame->GetTraceDlg()) \
			m_frame->GetTraceDlg()->GetSettings(m_vrv); \
		return; \
	}

void ScriptProc::RunSelectionTracking(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol)
		UPDATE_TRACE_DLG_AND_RETURN;


	int tseq_cur_num = m_view->m_tseq_cur_num;
	int tseq_prv_num = m_view->m_tseq_prv_num;

	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	if (time_mode == 0)
	{
		//read the size threshold
		int slimit;
		fconfig.Read("size_limit", &slimit, 0);
		//before updating volume
		flrd::ComponentAnalyzer comp_analyzer(cur_vol);
		comp_analyzer.Analyze(true, true);
		flrd::CelpList* list = comp_analyzer.GetCelpList();
		m_sel_labels.clear();
		for (auto it = list->begin();
			it != list->end(); ++it)
		{
			if (it->second->GetSize() > slimit)
			{
				flrd::Celp celp(new flrd::Cell(it->second->Id()));
				celp->Copy(it->second, true);
				//celp->SetSizeUi(it->second->sumi);
				//celp->SetSizeD(it->second->sumd);
				//celp->SetCenter(it->second->pos);
				//celp->SetBox(it->second->box);
				m_sel_labels.insert(pair<unsigned int, flrd::Celp>
					(it->second->Id(), celp));
			}
		}
	}
	else if (time_mode == 1)
	{
		TraceGroup* tg = m_view->GetTraceGroup();
		if (!tg) return;

		//after updating volume
		if (tg->GetTrackMap()->GetFrameNum())
		{
			//create new id list
			tg->SetCurTime(tseq_cur_num);
			tg->SetPrvTime(tseq_prv_num);
			tg->UpdateCellList(m_sel_labels);
			flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
		}

		Nrrd* mask_nrrd = cur_vol->GetMask(false);
		Nrrd* label_nrrd = cur_vol->GetLabel(false);
		if (!mask_nrrd || !label_nrrd)
			UPDATE_TRACE_DLG_AND_RETURN;
		unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
		unsigned int* label_data = (unsigned int*)(label_nrrd->data);
		if (!mask_data || !label_data)
			UPDATE_TRACE_DLG_AND_RETURN;
		int nx, ny, nz;
		cur_vol->GetResolution(nx, ny, nz);
		//update the mask according to the new label
		unsigned long long for_size = nx * ny * nz;
		memset((void*)mask_data, 0, sizeof(uint8)*for_size);
		for (unsigned long long idx = 0;
			idx < for_size; ++idx)
		{
			unsigned int label_value = label_data[idx];
			if (tg->GetTrackMap()->GetFrameNum())
			{
				if (tg->FindCell(label_value))
					mask_data[idx] = 255;
			}
			else
			{
				if (m_sel_labels.find(label_value) != m_sel_labels.end())
					mask_data[idx] = 255;
			}
		}
		UPDATE_TRACE_DLG_AND_RETURN;
	}
}

void ScriptProc::RunMaskTracking(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg)
	{
		m_view->CreateTraceGroup();
		tg = m_view->GetTraceGroup();
	}

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int tseq_prv_num = m_view->m_tseq_prv_num;
	int view_begin_frame = m_view->m_begin_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	double extx, exty, extz;
	fconfig.Read("ext_x", &extx, 0.1);
	fconfig.Read("ext_y", &exty, 0.1);
	fconfig.Read("ext_z", &extz, 0);
	fluo::Vector ext(extx, exty, extz);

	int iter;
	fconfig.Read("iter", &iter, 25);
	double eps;
	fconfig.Read("eps", &eps, 1e-3);
	int fsize;
	fconfig.Read("fsize", &fsize, 1);
	int mode;
	fconfig.Read("compare", &mode, 0);

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	cur_vol->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(cur_vol->GetBits());
	tm_processor.SetScale(cur_vol->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetMaxIter(iter);
	tm_processor.SetEps(eps);
	tm_processor.SetFilterSize(fsize);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&ScriptProc::ReadVolCache, this, std::placeholders::_1),
		std::bind(&ScriptProc::DelVolCache, this, std::placeholders::_1));

	tm_processor.TrackStencils(tseq_prv_num, tseq_cur_num,
		ext, mode, view_begin_frame);

	//add traces to trace dialog
	if (m_vrv && m_frame && m_frame->GetTraceDlg())
		m_frame->GetTraceDlg()->GetSettings(m_vrv);
}

void ScriptProc::RunRandomColors(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...

	int hmode;
	fconfig.Read("huemode", &hmode, 1);

	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}

	VolumeSelector* selector = m_view->GetVolumeSelector();
	if (!selector)
		return;

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		//generate RGB volumes
		selector->SetVolume(*i);
		selector->CompExportRandomColor(hmode, 0, 0, 0, false, false);
	}
}

void ScriptProc::RunFetchMask(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}
	bool bmask, blabel;
	fconfig.Read("mask", &bmask, 1);
	fconfig.Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		BaseReader* reader = (*i)->GetReader();
		if (!reader)
			return;
		//load and replace the mask
		if (bmask)
		{
			MSKReader msk_reader;
			wstring mskname = reader->GetCurMaskName(tseq_cur_num, (*i)->GetCurChannel());
			msk_reader.SetFile(mskname);
			Nrrd* mask_nrrd_new = msk_reader.Convert(tseq_cur_num, (*i)->GetCurChannel(), true);
			if (mask_nrrd_new)
				(*i)->LoadMask(mask_nrrd_new);
			//else
			//	(*i)->AddEmptyMask(0, true);
		}
		//load and replace the label
		if (blabel)
		{
			LBLReader lbl_reader;
			wstring lblname = reader->GetCurLabelName(tseq_cur_num, (*i)->GetCurChannel());
			lbl_reader.SetFile(lblname);
			Nrrd* label_nrrd_new = lbl_reader.Convert(tseq_cur_num, (*i)->GetCurChannel(), true);
			if (label_nrrd_new)
				(*i)->LoadLabel(label_nrrd_new);
			else
				(*i)->AddEmptyLabel(0, true);
		}
	}
}

void ScriptProc::RunClearMask(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}
	bool bmask, blabel;
	fconfig.Read("mask", &bmask, 1);
	fconfig.Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		//clear the mask
		if (bmask)
		{
			(*i)->AddEmptyMask(0, true);
		}
		//clear the label
		if (blabel)
		{
			(*i)->AddEmptyLabel(0, true);
		}
	}
}

void ScriptProc::RunSaveMask(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}
	bool bmask, blabel;
	fconfig.Read("mask", &bmask, 1);
	fconfig.Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		if (bmask)
			(*i)->SaveMask(true, tseq_cur_num, (*i)->GetCurChannel());
		if (blabel)
			(*i)->SaveLabel(true, tseq_cur_num, (*i)->GetCurChannel());
	}
}

void ScriptProc::RunSaveVolume(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...

	wxString source;
	fconfig.Read("source", &source);
	int mode;
	fconfig.Read("format", &mode, 0);
	bool crop;
	fconfig.Read("crop", &crop, false);
	int filter;
	fconfig.Read("filter", &filter, 0);
	bool bake;
	fconfig.Read("bake", &bake, false);
	bool compression;
	fconfig.Read("compress", &compression, false);
	wxString str, pathname;
	fconfig.Read("savepath", &pathname, "");
	bool del_vol;
	fconfig.Read("delete", &del_vol, false);
	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		MkDirW(str.ToStdWstring());
	if (!wxDirExists(str))
		return;

	VolumeData* vd = m_view->GetDispVolumeData(0);
	if (!vd)
		return;
	int time_num = vd->GetReader()->GetTimeNum();
	std::vector<VolumeData*> vlist;
	if (source == "channels" ||
		source == "")
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}
	else if (source == "calculator")
	{
		VolumeCalculator* calculator = m_view->GetVolumeCalculator();
		if (!calculator) return;
		VolumeData* vd = 0;
		while (vd = calculator->GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "selector")
	{
		VolumeSelector* selector = m_view->GetVolumeSelector();
		if (!selector) return;
		VolumeData* vd = 0;
		while (vd = selector->GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "executor")
	{
		KernelExecutor* executor = m_view->GetKernelExecutor();
		if (!executor) return;
		VolumeData* vd = 0;
		while (vd = executor->GetResult(true))
			vlist.push_back(vd);
	}
	int chan_num = vlist.size();

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		str = pathname;
		//time
		wxString format = wxString::Format("%d", time_num);
		int fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", fr_length);
		str += wxString::Format(format, tseq_cur_num);
		//channel
		if (chan_num > 1)
		{
			format = wxString::Format("%d", chan_num);
			int ch_length = format.Length();
			format = wxString::Format("_CH%%0%dd", ch_length + 1);
			str += wxString::Format(format, (*i)->GetCurChannel() + 1);
		}
		//ext
		if (mode == 0 || mode == 1)
			str += ".tif";
		else if (mode == 2)
			str += ".nrrd";
		fluo::Quaternion qtemp;
		(*i)->Save(str, mode, crop, filter, bake, compression, qtemp);
		if (del_vol)
			delete *i;
	}
}

void ScriptProc::RunCalculate(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	if (!calculator) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}

	int vol_a_index;
	fconfig.Read("vol_a", &vol_a_index, 0);
	int vol_b_index;
	fconfig.Read("vol_b", &vol_b_index, 0);
	wxString sOper;
	fconfig.Read("operator", &sOper, "");

	int vlist_size = m_view->GetDispVolumeNum();
	//get volumes
	VolumeData* vol_a = 0;
	if (vol_a_index >= 0 && vol_a_index < vlist_size)
		vol_a = m_view->GetDispVolumeData(vol_a_index);
	VolumeData* vol_b = 0;
	if (vol_b_index >= 0 && vol_b_index < vlist_size)
		vol_b = m_view->GetDispVolumeData(vol_b_index);
	if (!vol_a && !vol_b)
		return;

	//calculate
	calculator->SetVolumeA(vol_a);
	calculator->SetVolumeB(vol_b);
	if (sOper == "subtract")
		calculator->CalculateGroup(1, "", false);
	else if (sOper == "add")
		calculator->CalculateGroup(2, "", false);
	else if (sOper == "divide")
		calculator->CalculateGroup(3, "", false);
	else if (sOper == "colocate")
		calculator->CalculateGroup(4, "", false);
	else if (sOper == "fill")
		calculator->CalculateGroup(9, "", false);
}

void ScriptProc::RunOpenCL(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	KernelExecutor* executor = m_view->GetKernelExecutor();
	if (!executor) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...

	wxString clname;
	fconfig.Read("clpath", &clname, "");
	if (!wxFileExists(clname))
		return;

	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		(*i)->GetVR()->clear_tex_current();
		executor->LoadCode(clname);
		executor->SetVolume(*i);
		executor->SetDuplicate(true);
		executor->Execute();
	}
}

void ScriptProc::RunCompAnalysis(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	wxString str, pathname;
	fconfig.Read("savepath", &pathname);
	int verbose;
	fconfig.Read("verbose", &verbose, 0);
	bool consistent;
	fconfig.Read("consistent", &consistent, true);
	bool selected;
	fconfig.Read("selected", &selected, false);

	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		MkDirW(str.ToStdWstring());
	if (!wxDirExists(str))
		return;

	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}
	int chan_num = vlist.size();

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		flrd::ComponentAnalyzer comp_analyzer(*i);
		comp_analyzer.Analyze(selected, consistent);
		string result_str;
		string comp_header = wxString::Format("%d", tseq_cur_num);
		comp_analyzer.OutputCompListStr(result_str, verbose, comp_header);

		//save append
		bool sf_script = tseq_cur_num == view_begin_frame;
		wxString filename = pathname;
		//channel
		if (chan_num > 1)
		{
			wxString format = wxString::Format("%d", chan_num);
			int ch_length = format.Length();
			format = wxString::Format("_CH%%0%dd", ch_length + 1);
			filename += wxString::Format(format, (*i)->GetCurChannel() + 1);
		}
		filename += ".txt";
		wxFile file(filename, sf_script ? wxFile::write : wxFile::write_append);
		if (!file.IsOpened())
			continue;
		if (sf_script && verbose == 0)
		{
			string header;
			comp_analyzer.OutputFormHeader(header);
			file.Write(wxString::Format("Time\t"));
			file.Write(header);
		}
		if (verbose == 1)
			file.Write(wxString::Format("Time point: %d\n", tseq_cur_num));
		file.Write(result_str);
		file.Close();
	}
}

void ScriptProc::RunGenerateComp(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	bool use_sel;
	fconfig.Read("use_sel", &use_sel);
	double tfac = 1.0;
	fconfig.Read("th_factor", &tfac);
	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(cur_vol);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			vlist.push_back(m_view->GetDispVolumeData(i));
	}

	if (!(m_frame->GetComponentDlg()))
		return;

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_view->m_cur_vol = *i;
		m_frame->GetComponentDlg()->PlayCmd(use_sel, tfac);
	}
}

void ScriptProc::RunRulerProfile(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	RulerHandler* ruler_handler = m_view->GetRulerHandler();
	if (!ruler_handler) return;
	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;
	int view_begin_frame = m_view->m_begin_frame;
	int view_end_frame = m_view->m_end_frame;

	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && tseq_cur_num == view_begin_frame) &&
			!(end_frame && tseq_cur_num == view_end_frame))
			return;
	}

	ruler_handler->SetVolumeData(cur_vol);
	for (size_t i = 0; i < ruler_list->size(); ++i)
		ruler_handler->Profile(i);

	if (tseq_cur_num == 0 ||
		m_script_output.IsEmpty())
	{
		wxString path;
		if (cur_vol)
		{
			path = cur_vol->GetPath();
			path = wxPathOnly(path);
		}
		path += GETSLASH();
		path += "profiles_1.txt";

		while (wxFileExists(path))
		{
			int pos = path.Find('_', true);
			if (pos == wxNOT_FOUND)
			{
				path = path.SubString(0, path.Length() - 4);
				path += "_1.txt";
			}
			else
			{
				wxString digits;
				for (int i = pos + 1; i < path.Length() - 1; ++i)
				{
					if (wxIsdigit(path[i]))
						digits += path[i];
					else
						break;
				}
				long num = 0;
				digits.ToLong(&num);
				path = path.SubString(0, pos);
				path += wxString::Format("%d.txt", num + 1);
			}
		}
		m_script_output = path;
	}

	//save append
	bool sf_script = tseq_cur_num == view_begin_frame;
	wxFile file(m_script_output, sf_script ? wxFile::write : wxFile::write_append);
	if (!file.IsOpened())
		return;

	//get df/f setting
	bool df_f = false;
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetSettingDlg())
		df_f = frame->GetSettingDlg()->GetRulerDF_F();
	double f = 0.0;

	for (size_t i = 0; i < ruler_list->size(); ++i)
	{
		//for each ruler
		wxString str;
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;

		vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (profile && profile->size())
		{
			double sumd = 0.0;
			unsigned long long sumull = 0;
			for (size_t j = 0; j < profile->size(); ++j)
			{
				//for each profile
				int pixels = (*profile)[j].m_pixels;
				if (pixels <= 0)
					str += "0.0\t";
				else
				{
					str += wxString::Format("%f\t", (*profile)[j].m_accum / pixels);
					sumd += (*profile)[j].m_accum;
					sumull += pixels;
				}
			}
			if (df_f)
			{
				double avg = 0.0;
				if (sumull != 0)
					avg = sumd / double(sumull);
				if (i == 0)
				{
					f = avg;
					str += wxString::Format("\t%f\t", f);
				}
				else
				{
					double df = avg - f;
					if (f == 0.0)
						str += wxString::Format("\t%f\t", df);
					else
						str += wxString::Format("\t%f\t", df / f);
				}
			}
		}
		str += "\t";
		file.Write(str);
	}
	file.Write("\n");
	file.Close();
}

void ScriptProc::RunAddCells(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg)
	{
		m_view->CreateTraceGroup();
		tg = m_view->GetTraceGroup();
	}

	int tseq_cur_num = m_view->m_tseq_cur_num;

	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(cur_vol->GetBits());
	tm_processor.SetScale(cur_vol->GetScalarScale());
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.AddCells(m_sel_labels, tseq_cur_num);
}

void ScriptProc::RunLinkCells(int index, wxFileConfig &fconfig)
{
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	if (!m_frame || !m_frame->GetTraceDlg())
		return;

	m_frame->GetTraceDlg()->GetSettings(m_vrv);
	m_frame->GetTraceDlg()->LinkAddedCells(m_sel_labels);
}

void ScriptProc::RunUnlinkCells(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;

	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(cur_vol->GetBits());
	tm_processor.SetScale(cur_vol->GetScalarScale());
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.RemoveCells(m_sel_labels, tseq_cur_num);
}

void ScriptProc::RunBackgroundStat(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_frame) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int tseq_cur_num = m_view->m_tseq_cur_num;

	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	flrd::BackgStat bgs(cur_vol);
	bgs.Run();
}

//read/delete volume cache
void ScriptProc::ReadVolCache(flrd::VolCache& vol_cache)
{
	if (!m_view) return;
	//get volume, readers
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	int chan = cur_vol->GetCurChannel();
	int frame = vol_cache.frame;

	Nrrd* data = reader->Convert(frame, chan, true);
	vol_cache.nrrd_data = data;
	vol_cache.data = data->data;
	wstring lblname = reader->GetCurLabelName(frame, chan);
	lbl_reader.SetFile(lblname);
	Nrrd* label = lbl_reader.Convert(frame, chan, true);
	if (!label)
	{
		int resx, resy, resz;
		cur_vol->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		cur_vol->GetSpacings(spcx, spcy, spcz);
		label = nrrdNew();
		unsigned long long mem_size = (unsigned long long)resx*
			(unsigned long long)resy*(unsigned long long)resz;
		unsigned int *val32 = new (std::nothrow) unsigned int[mem_size];
		memset(val32, 0, sizeof(unsigned int)*mem_size);
		nrrdWrap(label, val32, nrrdTypeUInt, 3, (size_t)resx, (size_t)resy, (size_t)resz);
		nrrdAxisInfoSet(label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(label, nrrdAxisInfoMax, spcx*resx, spcy*resy, spcz*resz);
		nrrdAxisInfoSet(label, nrrdAxisInfoSize, (size_t)resx, (size_t)resy, (size_t)resz);
	}
	vol_cache.nrrd_label = label;
	vol_cache.label = label->data;
	if (data && label)
		vol_cache.valid = true;
}

void ScriptProc::DelVolCache(flrd::VolCache& vol_cache)
{
	if (!m_view) return;
	//get volume, readers
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	vol_cache.valid = false;
	if (vol_cache.data)
	{
		nrrdNuke((Nrrd*)vol_cache.nrrd_data);
		vol_cache.data = 0;
		vol_cache.nrrd_data = 0;
	}
	if (vol_cache.label)
	{
		int chan = cur_vol->GetCurChannel();
		int frame = vol_cache.frame;
		double spcx, spcy, spcz;
		cur_vol->GetSpacings(spcx, spcy, spcz);

		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)(vol_cache.nrrd_label));
		msk_writer.SetSpacings(spcx, spcy, spcz);
		BaseReader* reader = cur_vol->GetReader();
		if (reader)
		{
			wstring filename;
			filename = reader->GetCurLabelName(frame, chan);
			msk_writer.Save(filename, 1);
		}

		nrrdNuke((Nrrd*)vol_cache.nrrd_label);
		vol_cache.label = 0;
		vol_cache.nrrd_label = 0;
	}
}

