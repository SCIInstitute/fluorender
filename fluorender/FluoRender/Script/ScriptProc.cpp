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
#include <Flobject/InfoVisitor.hpp>
#include <DataManager.h>
#include <VRenderView.h>
#include <VRenderGLView.h>
#include <VRenderFrame.h>
#include <Calculate/BackgStat.h>
#include <utility.h>
#include <wx/filefn.h>
#include <wx/stdpaths.h>
#include <iostream>
#include <string> 
#include <sstream>
#include <fstream> 

using namespace flrd;

ScriptProc::ScriptProc() :
	m_frame(0),
	m_vrv(0),
	m_view(0)
{
	m_output = fluo::ref_ptr<fluo::Group>(new fluo::Group());
}

ScriptProc::~ScriptProc()
{
}

//run 4d script
void ScriptProc::Run4DScript(TimeMask tm, wxString &scriptname)
{
	m_fconfig = 0;
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
	m_fconfig = &fconfig;
	m_time_mask = tm;

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
				m_type = str;
				if (str == "noise_reduction")
					RunNoiseReduction();
				else if (str == "pre_tracking")
					RunPreTracking();
				else if (str == "post_tracking")
					RunPostTracking();
				else if (str == "mask_tracking")
					RunMaskTracking();
				else if (str == "random_colors")
					RunRandomColors();
				else if (str == "fetch_mask")
					RunFetchMask();
				else if (str == "clear_mask")
					RunClearMask();
				else if (str == "save_mask")
					RunSaveMask();
				else if (str == "opencl")
					RunOpenCL();
				else if (str == "comp_analysis")
					RunCompAnalysis();
				else if (str == "generate_comp")
					RunGenerateComp();
				else if (str == "ruler_profile")
					RunRulerProfile();
				else if (str == "save_volume")
					RunSaveVolume();
				else if (str == "calculate")
					RunCalculate();
				else if (str == "add_cells")
					RunAddCells();
				else if (str == "link_cells")
					RunLinkCells();
				else if (str == "unlink_cells")
					RunUnlinkCells();
				else if (str == "backg_stat")
					RunBackgroundStat();
				else if (str == "export_comp_analysis")
					ExportCompAnalysis();
			}
		}
	}
}

bool ScriptProc::TimeCondition()
{
	if (!m_fconfig || !m_view)
		return false;
	int time_mode, frame_mode;
	wxString str;
	m_fconfig->Read("time_mode", &str, "TM_PRE");
	time_mode = TimeMode(str.ToStdString());
	m_fconfig->Read("frame_mode", &str, "FM_ALL");
	frame_mode = FrameMode(str.ToStdString());
	int curf = m_view->m_tseq_cur_num;
	int startf = m_view->m_begin_frame;
	int endf = m_view->m_end_frame;
	//frame mode
	int fm;//mask
	int startd = curf - startf;
	int endd = endf - curf;
	if (startd < 15)
		fm = 1 << startd;
	if (endd < 15)
		fm = FM_LAST >> endd;
	if (startd >= 15 && endd >= 15)
		fm = 1 << 15;
	if (!(fm & frame_mode))
		return false;
	//time mode
	if (m_time_mask & time_mode)
		return true;
	return false;
}

bool ScriptProc::GetVolumes(std::vector<VolumeData*> &list)
{
	if (!m_fconfig || !m_view)
		return false;
	int chan_mode;
	m_fconfig->Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;2-shown vol;3-hidden vol
	list.clear();
	if (chan_mode == 0)
	{
		VolumeData* vol = m_view->m_cur_vol;
		if (vol)
			list.push_back(vol);
		else
			return false;
	}
	else if (chan_mode == 1)
	{
		for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
			list.push_back(m_view->GetAllVolumeData(i));
	}
	else if (chan_mode == 2)
	{
		for (int i = 0; i < m_view->GetDispVolumeNum(); ++i)
			list.push_back(m_view->GetDispVolumeData(i));
	}
	else if (chan_mode == 3)
	{
		for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
		{
			VolumeData* vol = m_view->GetAllVolumeData(i);
			if (!vol->GetDisp())
				list.push_back(vol);
		}
	}
	return !list.empty();
}

void ScriptProc::RunNoiseReduction()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	if (!m_frame) return;
	VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	if (!calculator) return;

	double thresh, size;
	m_fconfig->Read("threshold", &thresh, 0.0);
	m_fconfig->Read("voxelsize", &size, 0.0);

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
void ScriptProc::UpdateTraceDlg()
{
	if (m_vrv && m_frame && m_frame->GetTraceDlg())
		m_frame->GetTraceDlg()->GetSettings(m_vrv);
}

int ScriptProc::TimeMode(std::string &str)
{
	if (str == "TM_NONE")
		return TM_NONE;
	if (str == "TM_PRE")
		return TM_PRE;
	if (str == "TM_POST")
		return TM_POST;
	if (str == "TM_ALL")
		return TM_ALL;
	return std::stoi(str, nullptr, 0);
}

int ScriptProc::FrameMode(std::string &str)
{
	if (str == "FM_NONE")
		return FM_NONE;
	if (str == "FM_FIRST")
		return FM_FIRST;
	if (str == "FM_LAST")
		return FM_LAST;
	if (str == "FM_EXFIRST")
		return FM_EXFIRST;
	if (str == "FM_EXLAST")
		return FM_EXLAST;
	if (str == "FM_EXBOTH")
		return FM_EXBOTH;
	if (str == "FM_ALL")
		return FM_ALL;
	if (str.find("0x") != std::string::npos ||
		str.find("0X") != std::string::npos)
		return std::stoi(str, nullptr, 16);
	return std::stoi(str, nullptr, 0);
}

wxString ScriptProc::GetPath(wxString &str)
{
	wxString path;
	//fluo
	return path;
}

void ScriptProc::RunPreTracking()
{
	if (!TimeCondition())
		return;

	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol)
		UpdateTraceDlg();

	//read the size threshold
	int slimit;
	m_fconfig->Read("size_limit", &slimit, 0);
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
			m_sel_labels.insert(std::pair<unsigned int, flrd::Celp>
				(it->second->Id(), celp));
		}
	}
}

void ScriptProc::RunPostTracking()
{
	if (!TimeCondition())
		return;

	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol)
		UpdateTraceDlg();

	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg) return;

	//after updating volume
	if (tg->GetTrackMap()->GetFrameNum())
	{
		//create new id list
		tg->SetCurTime(m_view->m_tseq_cur_num);
		tg->SetPrvTime(m_view->m_tseq_prv_num);
		tg->UpdateCellList(m_sel_labels);
		flvr::TextureRenderer::vertex_array_manager_.set_dirty(flvr::VA_Traces);
	}

	Nrrd* mask_nrrd = cur_vol->GetMask(false);
	Nrrd* label_nrrd = cur_vol->GetLabel(false);
	if (!mask_nrrd || !label_nrrd)
		UpdateTraceDlg();
	unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
	unsigned int* label_data = (unsigned int*)(label_nrrd->data);
	if (!mask_data || !label_data)
		UpdateTraceDlg();
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
	UpdateTraceDlg();
}

void ScriptProc::RunMaskTracking()
{
	if (!TimeCondition())
		return;

	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg)
	{
		m_view->CreateTraceGroup();
		tg = m_view->GetTraceGroup();
	}

	double extx, exty, extz;
	m_fconfig->Read("ext_x", &extx, 0.1);
	m_fconfig->Read("ext_y", &exty, 0.1);
	m_fconfig->Read("ext_z", &extz, 0);
	fluo::Vector ext(extx, exty, extz);
	int iter;
	m_fconfig->Read("iter", &iter, 25);
	double eps;
	m_fconfig->Read("eps", &eps, 1e-3);
	int fsize;
	m_fconfig->Read("fsize", &fsize, 1);
	int mode;
	m_fconfig->Read("compare", &mode, 0);

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

	tm_processor.TrackStencils(
		m_view->m_tseq_prv_num,
		m_view->m_tseq_cur_num,
		ext, mode,
		m_view->m_begin_frame);

	UpdateTraceDlg();
}

void ScriptProc::RunRandomColors()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	VolumeSelector* selector = m_view->GetVolumeSelector();
	if (!selector)
		return;

	int hmode;
	m_fconfig->Read("huemode", &hmode, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		//generate RGB volumes
		selector->SetVolume(*i);
		selector->CompExportRandomColor(hmode, 0, 0, 0, false, false);
	}
}

void ScriptProc::RunFetchMask()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	int curf = m_view->m_tseq_cur_num;
	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, 1);
	m_fconfig->Read("label", &blabel, 1);

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
			wstring mskname = reader->GetCurMaskName(curf, (*i)->GetCurChannel());
			msk_reader.SetFile(mskname);
			Nrrd* mask_nrrd_new = msk_reader.Convert(curf, (*i)->GetCurChannel(), true);
			if (mask_nrrd_new)
				(*i)->LoadMask(mask_nrrd_new);
			//else
			//	(*i)->AddEmptyMask(0, true);
		}
		//load and replace the label
		if (blabel)
		{
			LBLReader lbl_reader;
			wstring lblname = reader->GetCurLabelName(curf, (*i)->GetCurChannel());
			lbl_reader.SetFile(lblname);
			Nrrd* label_nrrd_new = lbl_reader.Convert(curf, (*i)->GetCurChannel(), true);
			if (label_nrrd_new)
				(*i)->LoadLabel(label_nrrd_new);
			else
				(*i)->AddEmptyLabel(0, true);
		}
	}
}

void ScriptProc::RunClearMask()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, 1);
	m_fconfig->Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		//clear the mask
		if (bmask)
			(*i)->AddEmptyMask(0, true);
		//clear the label
		if (blabel)
			(*i)->AddEmptyLabel(0, true);
	}
}

void ScriptProc::RunSaveMask()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	int curf = m_view->m_tseq_cur_num;
	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, 1);
	m_fconfig->Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		if (bmask)
			(*i)->SaveMask(true, curf, (*i)->GetCurChannel());
		if (blabel)
			(*i)->SaveLabel(true, curf, (*i)->GetCurChannel());
	}
}

void ScriptProc::RunSaveVolume()
{
	if (!TimeCondition())
		return;

	wxString source;
	m_fconfig->Read("source", &source);
	int mode;
	m_fconfig->Read("format", &mode, 0);
	bool crop;
	m_fconfig->Read("crop", &crop, false);
	int filter;
	m_fconfig->Read("filter", &filter, 0);
	bool bake;
	m_fconfig->Read("bake", &bake, false);
	bool compression;
	m_fconfig->Read("compress", &compression, false);
	wxString str, pathname;
	m_fconfig->Read("savepath", &pathname, "");
	bool del_vol;
	m_fconfig->Read("delete", &del_vol, false);
	str = wxPathOnly(pathname);
	if (!wxDirExists(str))
		MkDirW(str.ToStdWstring());
	if (!wxDirExists(str))
		return;

	std::vector<VolumeData*> vlist;
	if (source == "channels" ||
		source == "")
	{
		GetVolumes(vlist);
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
	int time_num;
	if (!vlist.empty())
		time_num = vlist[0]->GetReader()->GetTimeNum();
	int curf = m_view->m_tseq_cur_num;
	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		str = pathname;
		//time
		wxString format = wxString::Format("%d", time_num);
		int fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", fr_length);
		str += wxString::Format(format, curf);
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

void ScriptProc::RunCalculate()
{
	if (!TimeCondition())
		return;

	VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	if (!calculator) return;

	int vol_a_index;
	m_fconfig->Read("vol_a", &vol_a_index, 0);
	int vol_b_index;
	m_fconfig->Read("vol_b", &vol_b_index, 0);
	wxString sOper;
	m_fconfig->Read("operator", &sOper, "");

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

void ScriptProc::RunOpenCL()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	KernelExecutor* executor = m_view->GetKernelExecutor();
	if (!executor) return;

	wxString clname;
	m_fconfig->Read("clpath", &clname, "");
	if (!wxFileExists(clname))
		return;

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

void ScriptProc::RunCompAnalysis()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	int verbose;
	m_fconfig->Read("verbose", &verbose, 0);
	bool consistent;
	m_fconfig->Read("consistent", &consistent, true);
	bool selected;
	m_fconfig->Read("selected", &selected, false);
	int slimit;
	m_fconfig->Read("slimit", &slimit, 5);

	int curf = m_view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	fluo::Vector lens;
	std::string fn = std::to_string(curf);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		int bn = (*itvol)->GetAllBrickNum();
		flrd::ComponentAnalyzer comp_analyzer(*itvol);
		comp_analyzer.SetSizeLimit(slimit);
		comp_analyzer.Analyze(selected, consistent);
		fluo::Group* vol_group = m_output->getOrAddGroup(std::to_string(ch));
		vol_group->addSetValue("type", std::string("vol_group"));
		CelpList* celp_list = comp_analyzer.GetCelpList();
		CellGraph* graph = comp_analyzer.GetCellGraph();
		if (!celp_list || !graph)
			continue;
		double sx = celp_list->sx;
		double sy = celp_list->sy;
		double sz = celp_list->sz;
		double size_scale = sx * sy * sz;
		double maxscale = (*itvol)->GetMaxScale();
		double scalarscale = (*itvol)->GetScalarScale();
		for (auto itc = celp_list->begin();
			itc != celp_list->end(); ++itc)
		{
			std::list<unsigned int> ids;
			std::list<unsigned int> brick_ids;
			bool added = false;

			if (bn > 1)
			{
				if (graph->Visited(itc->second))
					continue;

				CelpList list;
				if (graph->GetLinkedComps(itc->second, list, slimit))
				{
					for (auto iter = list.begin();
						iter != list.end(); ++iter)
					{
						ids.push_back(iter->second->Id());
						brick_ids.push_back(iter->second->BrickId());
					}
					added = true;
				}
			}
			if (!added)
			{
				ids.push_back(itc->second->Id());
				brick_ids.push_back(itc->second->BrickId());
			}

			//pca
			itc->second->GetPca().Compute();
			lens = itc->second->GetPca().GetLengths();

			unsigned long id = ids.front();
			fluo::Group* comp_group = vol_group->getOrAddGroup(std::to_string(id));
			comp_group->addSetValue("type", std::string("comp_group"));
			fluo::Node* node;
			node = comp_group->getOrAddNode("center");
			node->addSetValue(fn, itc->second->GetCenter(sx, sy, sz));
			node = comp_group->getOrAddNode("size_ui");
			node->addSetValue(fn, (unsigned long)(itc->second->GetSizeUi()));
			node = comp_group->getOrAddNode("size_d");
			node->addSetValue(fn, itc->second->GetSizeD(scalarscale));
			node = comp_group->getOrAddNode("phys_size_ui");
			node->addSetValue(fn, size_scale * itc->second->GetSizeUi());
			node = comp_group->getOrAddNode("phys_size_d");
			node->addSetValue(fn, itc->second->GetSizeD(size_scale * scalarscale));
			node = comp_group->getOrAddNode("ext_size_ui");
			node->addSetValue(fn, (unsigned long)(itc->second->GetExtUi()));
			node = comp_group->getOrAddNode("ext_size_d");
			node->addSetValue(fn, itc->second->GetExtD(scalarscale));
			node = comp_group->getOrAddNode("mean");
			node->addSetValue(fn, itc->second->GetMean(maxscale));
			node = comp_group->getOrAddNode("stdev");
			node->addSetValue(fn, itc->second->GetStd(maxscale));
			node = comp_group->getOrAddNode("min");
			node->addSetValue(fn, itc->second->GetMin(maxscale));
			node = comp_group->getOrAddNode("max");
			node->addSetValue(fn, itc->second->GetMax(maxscale));
			node = comp_group->getOrAddNode("distp");
			node->addSetValue(fn, itc->second->GetDistp());
			node = comp_group->getOrAddNode("pca_lens");
			node->addSetValue(fn, lens);
		}
	}
}

void ScriptProc::RunGenerateComp()
{
	if (!m_frame) return;
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	bool use_sel;
	m_fconfig->Read("use_sel", &use_sel);
	double tfac = 1.0;
	m_fconfig->Read("th_factor", &tfac);

	if (!(m_frame->GetComponentDlg()))
		return;

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_view->m_cur_vol = *i;
		m_frame->GetComponentDlg()->PlayCmd(use_sel, tfac);
	}
}

void ScriptProc::RunRulerProfile()
{
	if (!m_frame) return;
	if (!TimeCondition())
		return;

	RulerHandler* ruler_handler = m_view->GetRulerHandler();
	if (!ruler_handler) return;
	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	ruler_handler->SetVolumeData(cur_vol);
	for (size_t i = 0; i < ruler_list->size(); ++i)
		ruler_handler->Profile(i);

	//get df/f setting
	bool df_f = false;
	if (m_frame->GetSettingDlg())
		df_f = m_frame->GetSettingDlg()->GetRulerDF_F();
	double f = 0.0;
	std::string fn = std::to_string(m_view->m_tseq_cur_num);

	for (size_t i = 0; i < ruler_list->size(); ++i)
	{
		//for each ruler
		fluo::Group* ruler_group = m_output->getOrAddGroup(std::to_string(i));
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;

		vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (profile && profile->size())
		{
			double dval;
			double sumd = 0.0;
			unsigned long long sumull = 0;
			for (size_t j = 0; j < profile->size(); ++j)
			{
				//for each profile
				fluo::Node* profile_node = ruler_group->getOrAddNode(std::to_string(j));
				int pixels = (*profile)[j].m_pixels;
				if (pixels <= 0)
					dval = 0;
				else
				{
					dval = (*profile)[j].m_accum / pixels;
					sumd += (*profile)[j].m_accum;
					sumull += pixels;
				}
				profile_node->addSetValue(fn, dval);
			}
			if (df_f)
			{
				double avg = 0.0;
				if (sumull != 0)
					avg = sumd / double(sumull);
				if (i == 0)
				{
					dval = f = avg;
				}
				else
				{
					double df = avg - f;
					if (f == 0.0)
						dval = df;
					else
						dval = df / f;
				}
				fluo::Node* dff_node = ruler_group->getOrAddNode("df_f");
				dff_node->addSetValue(fn, dval);
			}
		}
	}
}

void ScriptProc::RunAddCells()
{
	if (!TimeCondition())
		return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg)
	{
		m_view->CreateTraceGroup();
		tg = m_view->GetTraceGroup();
	}

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(cur_vol->GetBits());
	tm_processor.SetScale(cur_vol->GetScalarScale());
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.AddCells(m_sel_labels,
		m_view->m_tseq_cur_num);
}

void ScriptProc::RunLinkCells()
{
	if (!TimeCondition())
		return;

	if (!m_frame || !m_frame->GetTraceDlg())
		return;

	m_frame->GetTraceDlg()->GetSettings(m_vrv);
	m_frame->GetTraceDlg()->LinkAddedCells(m_sel_labels);
}

void ScriptProc::RunUnlinkCells()
{
	if (!TimeCondition())
		return;

	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	TraceGroup* tg = m_view->GetTraceGroup();
	if (!tg) return;

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	tm_processor.SetBits(cur_vol->GetBits());
	tm_processor.SetScale(cur_vol->GetScalarScale());
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.RemoveCells(m_sel_labels,
		m_view->m_tseq_cur_num);
}

void ScriptProc::RunBackgroundStat()
{
	if (!TimeCondition())
		return;

	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	flrd::BackgStat bgs(cur_vol);
	bool bval;
	m_fconfig->Read("use_mask", &bval, false);
	bgs.SetUseMask(bval);
	int itype;
	m_fconfig->Read("stat_type", &itype, 0);
	bgs.SetType(itype);
	int kx = 0, ky = 0;
	m_fconfig->Read("kx", &kx, 0);
	m_fconfig->Read("ky", &ky, 0);
	if (kx && ky)
		bgs.SetFeatureSize2D(kx, ky);
	float varth = 0, gauth = 0;
	m_fconfig->Read("varth", &varth, 0);
	m_fconfig->Read("gauth", &gauth, 0);
	if (varth > 0.0 && gauth > 0.0)
		bgs.SetThreshold(varth, gauth);

	bgs.Run();

	//output
	std::string str;
	float result = bgs.GetResultf();
	fluo::Node* node = m_output->getOrAddNode(m_type.ToStdString());
	node->addValue("stat_type", (long)itype);
	str = std::to_string(m_view->m_tseq_cur_num);
	node->addSetValue(str, result);
}

void ScriptProc::ExportCompAnalysis()
{
	if (!m_frame) return;
	if (!TimeCondition())
		return;

	//template
	wxString tempfile;
	m_fconfig->Read("template", &tempfile);
	if (!wxFileExists(tempfile))
	{
		std::wstring name = tempfile.ToStdWstring();
		name = GET_NAME(name);
		wxString exePath = wxStandardPaths::Get().GetExecutablePath();
		exePath = wxPathOnly(exePath);
		tempfile = exePath + "\\Templates\\" + name;
		if (!wxFileExists(tempfile))
			return;
	}
	wxString outputfile;
	m_fconfig->Read("output", &outputfile);
	if (outputfile.IsEmpty())
	{
		wxFileDialog *fopendlg = new wxFileDialog(
			m_frame, "Save Results", "", "",
			"Web page(*.html)|*.html",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int rval = fopendlg->ShowModal();
		if (rval == wxID_OK)
		{
			outputfile = fopendlg->GetPath();
		}
		delete fopendlg;
	}
	if (outputfile.IsEmpty())
		return;
	wxString valuename;
	m_fconfig->Read("value_name", &valuename);
	double tlimit;
	m_fconfig->Read("tlimit", &tlimit, 5);
	int thresh = int((100 - tlimit) *
		(m_view->m_end_frame -
		m_view->m_begin_frame + 1)
		/ 100 + 0.5);

	//print lines
	class CompVisitor : public fluo::NodeVisitor
	{
	public:
		CompVisitor(std::ofstream &ofs,
			std::string &vname,
			int num, int thresh) : fluo::NodeVisitor(),
			ofs_(&ofs),
			vname_(vname),
			chnum_(num),
			thresh_(thresh)
		{
			setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
		}

		virtual void apply(fluo::Node& node)
		{
			std::string nname = node.getName();
			if (nname == vname_)
			{
				printValues(&node);
			}
			traverse(node);
		}

		virtual void apply(fluo::Group& group)
		{
			std::string type;
			group.getValue("type", type);
			if (type == "vol_group")
				ch_ = group.getName();
			if (type == "comp_group")
				id_ = group.getName();
			traverse(group);
		}

	protected:
		void printValues(fluo::Object* object)
		{
			if (!object)
				return;
			//get all value names
			fluo::ValueVector names =
				object->getValueNames(2);
			if (names.size() < thresh_)
				return;
			for (auto it = names.begin();
				it != names.end(); ++it)
			{
				fluo::Value* value = object->getValuePointer(*it);
				if (value)
				{
					if (chnum_ > 1)
						*ofs_ << "CH-" << ch_ << " ";
					*ofs_ << "ID-" << id_ << "," <<
					value->getName() << "," <<
					*value << "\\n\\" << std::endl;
				}
			}
		}

	private:
		std::ofstream *ofs_;
		std::string vname_;
		int chnum_;
		int thresh_;
		std::string ch_;
		std::string id_;
	};

	std::ifstream ifs(tempfile.ToStdString());
	std::ofstream ofs(outputfile.ToStdString());
	std::string line;
	int replace = 0;//1:data;2:value name
	while (std::getline(ifs, line))
	{
		if (line.find("#begin data") != std::string::npos)
		{
			//data
			replace = 1;
			ofs << "//#begin data" << std::endl;
			ofs << "let csv_data = \"id,time," << valuename.ToStdString() << "\\n\\" << std::endl;
			CompVisitor visitor(ofs,
				valuename.ToStdString(),
				m_output->getNumChildren(),
				thresh);
			m_output->accept(visitor);
			ofs << "\";" << std::endl;
		}
		if (line.find("#begin value name") != std::string::npos)
		{
			//value name
			replace = 2;
			ofs << "//#begin value name" << std::endl;
			ofs << "        value: +d." << valuename.ToStdString() << std::endl;
		}
		if (line.find("#end") != std::string::npos)
		{
			replace = 0;
		}
		if (replace == 0)
			ofs << line << std::endl;
	}
	ifs.close();
	ofs.close();

	::wxLaunchDefaultBrowser(outputfile.ToStdString());
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

