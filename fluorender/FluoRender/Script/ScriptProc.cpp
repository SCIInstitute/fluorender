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
#include <utility.h>
#include <wx/filefn.h>
#include <wx/stdpaths.h>

using namespace FL;

ScriptProc::ScriptProc() :
	m_frame(0),
	m_vrv(0),
	m_view(0),
	m_vd(0)
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
				else if (str == "sparse_tracking")
					RunSparseTracking(index, fconfig);
				else if (str == "random_colors")
					RunRandomColors(index, fconfig);
				else if (str == "fetch_mask")
					RunFetchMask(index, fconfig);
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
			}
		}
	}
}

void ScriptProc::RunNoiseReduction(int index, wxFileConfig &fconfig)
{
	if (!m_view || !m_vd || !m_frame) return;
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
		vlist.push_back(m_vd);
	}
	else
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
				vlist.push_back(m_view->GetDispVolumeData(i));
	}

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_vd = *i;
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
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	if (!m_vd)
		UPDATE_TRACE_DLG_AND_RETURN;

	if (time_mode == 0)
	{
		//read the size threshold
		int slimit;
		fconfig.Read("size_limit", &slimit, 0);
		//before updating volume
		FL::ComponentAnalyzer comp_analyzer(m_vd);
		comp_analyzer.Analyze(true, true);
		FL::CompList* list = comp_analyzer.GetCompList();
		m_sel_labels.clear();
		for (auto it = list->begin();
			it != list->end(); ++it)
		{
			if (it->second->sumi > slimit)
			{
				FL::pCell cell(new FL::Cell(it->second->id));
				cell->SetSizeUi(it->second->sumi);
				cell->SetSizeF(it->second->sumd);
				cell->SetCenter(it->second->pos);
				cell->SetBox(it->second->box);
				m_sel_labels.insert(pair<unsigned int, FL::pCell>
					(it->second->id, cell));
			}
		}
	}
	else if (time_mode == 1)
	{
		//after updating volume
		if (m_trace_group &&
			m_trace_group->GetTrackMap()->GetFrameNum())
		{
			//create new id list
			m_trace_group->SetCurTime(m_tseq_cur_num);
			m_trace_group->SetPrvTime(m_tseq_prv_num);
			m_trace_group->UpdateCellList(m_sel_labels);
			TextureRenderer::vertex_array_manager_.set_dirty(VA_Traces);
		}

		Nrrd* mask_nrrd = m_cur_vol->GetMask(false);
		Nrrd* label_nrrd = m_cur_vol->GetLabel(false);
		if (!mask_nrrd || !label_nrrd)
			UPDATE_TRACE_DLG_AND_RETURN;
		unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
		unsigned int* label_data = (unsigned int*)(label_nrrd->data);
		if (!mask_data || !label_data)
			UPDATE_TRACE_DLG_AND_RETURN;
		int nx, ny, nz;
		m_cur_vol->GetResolution(nx, ny, nz);
		//update the mask according to the new label
		unsigned long long for_size = nx * ny * nz;
		memset((void*)mask_data, 0, sizeof(uint8)*for_size);
		for (unsigned long long idx = 0;
			idx < for_size; ++idx)
		{
			unsigned int label_value = label_data[idx];
			if (m_trace_group &&
				m_trace_group->GetTrackMap()->GetFrameNum())
			{
				if (m_trace_group->FindCell(label_value))
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

void ScriptProc::RunSparseTracking(int index, wxFileConfig &fconfig)
{
	if (!m_trace_group)
		CreateTraceGroup();

	FL::pTrackMap track_map = m_trace_group->GetTrackMap();
	FL::TrackMapProcessor tm_processor(track_map);
	int resx, resy, resz;
	m_cur_vol->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	m_cur_vol->GetSpacings(spcx, spcy, spcz);
	tm_processor.SetBits(m_cur_vol->GetBits());
	tm_processor.SetScale(m_cur_vol->GetScalarScale());
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	//tm_processor.SetSizeThresh(component_size);
	//tm_processor.SetContactThresh(contact_factor);
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		boost::bind(&VRenderGLView::ReadVolCache, this, _1),
		boost::bind(&VRenderGLView::DelVolCache, this, _1));

	tm_processor.TrackStencils(m_tseq_prv_num, m_tseq_cur_num);

	//add traces to trace dialog
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (m_vrv && vr_frame && vr_frame->GetTraceDlg())
		vr_frame->GetTraceDlg()->GetSettings(m_vrv);
}

void ScriptProc::RunRandomColors(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...

	int hmode;
	fconfig.Read("huemode", &hmode, 1);

	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(m_cur_vol);
	}
	else
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
	}

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		//generate RGB volumes
		m_selector.SetVolume(*i);
		m_selector.CompExportRandomColor(hmode, 0, 0, 0, false, false);
	}
}

void ScriptProc::RunFetchMask(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(m_cur_vol);
	}
	else
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
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
			wstring mskname = reader->GetCurMaskName(m_tseq_cur_num, (*i)->GetCurChannel());
			msk_reader.SetFile(mskname);
			Nrrd* mask_nrrd_new = msk_reader.Convert(m_tseq_cur_num, (*i)->GetCurChannel(), true);
			if (mask_nrrd_new)
				(*i)->LoadMask(mask_nrrd_new);
			//else
			//	(*i)->AddEmptyMask(0, true);
		}
		//load and replace the label
		if (blabel)
		{
			LBLReader lbl_reader;
			wstring lblname = reader->GetCurLabelName(m_tseq_cur_num, (*i)->GetCurChannel());
			lbl_reader.SetFile(lblname);
			Nrrd* label_nrrd_new = lbl_reader.Convert(m_tseq_cur_num, (*i)->GetCurChannel(), true);
			if (label_nrrd_new)
				(*i)->LoadLabel(label_nrrd_new);
			else
				(*i)->AddEmptyLabel(0, true);
		}
	}
}

void ScriptProc::RunSaveMask(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...
	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(m_cur_vol);
	}
	else
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
	}
	bool bmask, blabel;
	fconfig.Read("mask", &bmask, 1);
	fconfig.Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		if (bmask)
			(*i)->SaveMask(true, m_tseq_cur_num, (*i)->GetCurChannel());
		if (blabel)
			(*i)->SaveLabel(true, m_tseq_cur_num, (*i)->GetCurChannel());
	}
}

void ScriptProc::RunSaveVolume(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
			return;
	}
	fconfig.Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;...

	wxString source;
	fconfig.Read("source", &source);
	int mode;
	fconfig.Read("format", &mode, 0);
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
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	if (!wxDirExists(str))
		return;

	if (!m_vd_pop_list[0])
		return;
	int time_num = m_vd_pop_list[0]->GetReader()->GetTimeNum();
	std::vector<VolumeData*> vlist;
	if (source == "channels" ||
		source == "")
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
	}
	else if (source == "calculator")
	{
		VolumeData* vd = 0;
		while (vd = m_calculator.GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "selector")
	{
		VolumeData* vd = 0;
		while (vd = m_selector.GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "executor")
	{
		VolumeData* vd = 0;
		while (vd = m_kernel_executor.GetResult(true))
			vlist.push_back(vd);
	}
	int chan_num = vlist.size();

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		str = pathname;
		//time
		wxString format = wxString::Format("%d", time_num);
		m_fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", m_fr_length);
		str += wxString::Format(format, m_tseq_cur_num);
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
		(*i)->Save(str, mode, bake, compression);
		if (del_vol)
			delete *i;
	}
}

void ScriptProc::RunCalculate(int index, wxFileConfig &fconfig)
{
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
			return;
	}

	int vol_a_index;
	fconfig.Read("vol_a", &vol_a_index, 0);
	int vol_b_index;
	fconfig.Read("vol_b", &vol_b_index, 0);
	wxString sOper;
	fconfig.Read("operator", &sOper, "");

	//get volumes
	VolumeData* vol_a = 0;
	if (vol_a_index >= 0 && vol_a_index < (int)m_vd_pop_list.size())
		vol_a = m_vd_pop_list[vol_a_index];
	VolumeData* vol_b = 0;
	if (vol_b_index >= 0 && vol_b_index < (int)m_vd_pop_list.size())
		vol_b = m_vd_pop_list[vol_b_index];
	if (!vol_a && !vol_b)
		return;

	//calculate
	SetVolumeA(vol_a);
	SetVolumeB(vol_b);
	if (sOper == "subtract")
		Calculate(1, "", false);
	else if (sOper == "add")
		Calculate(2, "", false);
	else if (sOper == "divide")
		Calculate(3, "", false);
	else if (sOper == "colocate")
		Calculate(4, "", false);
	else if (sOper == "fill")
		Calculate(9, "", false);
}

void ScriptProc::RunOpenCL(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
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
		vlist.push_back(m_cur_vol);
	}
	else
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
	}

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		(*i)->GetVR()->clear_tex_current();
		m_kernel_executor.LoadCode(clname);
		m_kernel_executor.SetVolume(*i);
		m_kernel_executor.SetDuplicate(true);
		m_kernel_executor.Execute();
	}
}

void ScriptProc::RunCompAnalysis(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
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
		wxFileName::Mkdir(str, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
	if (!wxDirExists(str))
		return;

	std::vector<VolumeData*> vlist;
	if (chan_mode == 0)
	{
		vlist.push_back(m_cur_vol);
	}
	else
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
	}
	int chan_num = vlist.size();

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		FL::ComponentAnalyzer comp_analyzer(*i);
		comp_analyzer.Analyze(selected, consistent);
		string result_str;
		string comp_header = wxString::Format("%d", m_tseq_cur_num);
		comp_analyzer.OutputCompListStr(result_str, verbose, comp_header);

		//save append
		bool sf_script = m_tseq_cur_num == m_begin_frame;
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
			file.Write(wxString::Format("Time point: %d\n", m_tseq_cur_num));
		file.Write(result_str);
		file.Close();
	}
}

void ScriptProc::RunGenerateComp(int index, wxFileConfig &fconfig)
{
	int time_mode, chan_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
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
		vlist.push_back(m_cur_vol);
	}
	else
	{
		for (auto i = m_vd_pop_list.begin();
			i != m_vd_pop_list.end(); ++i)
		{
			if ((*i)->GetDisp())
				vlist.push_back(*i);
		}
	}

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame || !(vr_frame->GetComponentDlg()))
		return;

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_cur_vol = *i;
		vr_frame->GetComponentDlg()->PlayCmd(use_sel, tfac);
	}
}

void ScriptProc::RunRulerProfile(int index, wxFileConfig &fconfig)
{
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	bool start_frame, end_frame;
	fconfig.Read("start_frame", &start_frame, false);
	fconfig.Read("end_frame", &end_frame, false);
	if (time_mode != index)
	{
		if (!(start_frame && m_tseq_cur_num == m_begin_frame) &&
			!(end_frame && m_tseq_cur_num == m_end_frame))
			return;
	}

	if (m_ruler_list.empty())
		return;

	m_ruler_handler.SetVolumeData(m_cur_vol);
	for (size_t i = 0; i < m_ruler_list.size(); ++i)
		m_ruler_handler.Profile(i);

	if (m_tseq_cur_num == 0 ||
		m_script_output.IsEmpty())
	{
		wxString path;
		if (m_cur_vol)
		{
			path = m_cur_vol->GetPath();
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
	bool sf_script = m_tseq_cur_num == m_begin_frame;
	wxFile file(m_script_output, sf_script ? wxFile::write : wxFile::write_append);
	if (!file.IsOpened())
		return;

	//get df/f setting
	bool df_f = false;
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetSettingDlg())
		df_f = frame->GetSettingDlg()->GetRulerDF_F();
	double f = 0.0;

	for (size_t i = 0; i < m_ruler_list.size(); ++i)
	{
		//for each ruler
		wxString str;
		FL::Ruler* ruler = m_ruler_list[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;

		vector<FL::ProfileBin>* profile = ruler->GetProfile();
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
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	if (!m_cur_vol)
		return;
	if (!m_trace_group)
		CreateTraceGroup();

	FL::pTrackMap track_map = m_trace_group->GetTrackMap();
	FL::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(m_cur_vol->GetBits());
	tm_processor.SetScale(m_cur_vol->GetScalarScale());
	int resx, resy, resz;
	m_cur_vol->GetResolution(resx, resy, resz);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.AddCells(m_sel_labels, m_tseq_cur_num);
}

void ScriptProc::RunLinkCells(int index, wxFileConfig &fconfig)
{
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame || !vr_frame->GetTraceDlg())
		return;

	vr_frame->GetTraceDlg()->GetSettings(m_vrv);
	vr_frame->GetTraceDlg()->LinkAddedCells(m_sel_labels);
}

void ScriptProc::RunUnlinkCells(int index, wxFileConfig &fconfig)
{
	int time_mode;
	fconfig.Read("time_mode", &time_mode, 0);//0-post-change;1-pre-change
	if (time_mode != index)
		return;

	if (!m_trace_group || !m_cur_vol)
		return;

	FL::pTrackMap track_map = m_trace_group->GetTrackMap();
	FL::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(m_cur_vol->GetBits());
	tm_processor.SetScale(m_cur_vol->GetScalarScale());
	int resx, resy, resz;
	m_cur_vol->GetResolution(resx, resy, resz);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.RemoveCells(m_sel_labels, m_tseq_cur_num);
}

//read/delete volume cache
void ScriptProc::ReadVolCache(FL::VolCache& vol_cache)
{
	//get volume, readers
	if (!m_cur_vol)
		return;
	BaseReader* reader = m_cur_vol->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	int chan = m_cur_vol->GetCurChannel();
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
		m_cur_vol->GetResolution(resx, resy, resz);
		double spcx, spcy, spcz;
		m_cur_vol->GetSpacings(spcx, spcy, spcz);
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

void ScriptProc::DelVolCache(FL::VolCache& vol_cache)
{
	if (!m_cur_vol)
		return;
	vol_cache.valid = false;
	if (vol_cache.data)
	{
		nrrdNuke((Nrrd*)vol_cache.nrrd_data);
		vol_cache.data = 0;
		vol_cache.nrrd_data = 0;
	}
	if (vol_cache.label)
	{
		int chan = m_cur_vol->GetCurChannel();
		int frame = vol_cache.frame;
		double spcx, spcy, spcz;
		m_cur_vol->GetSpacings(spcx, spcy, spcz);

		MSKWriter msk_writer;
		msk_writer.SetData((Nrrd*)(vol_cache.nrrd_label));
		msk_writer.SetSpacings(spcx, spcy, spcz);
		BaseReader* reader = m_cur_vol->GetReader();
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

