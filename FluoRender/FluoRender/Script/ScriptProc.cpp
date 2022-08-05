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

#include "ScriptProc.h"
#include <InfoVisitor.hpp>
#include <Renderview.hpp>
#include <Global.hpp>
#include <VolumeFactory.hpp>
#include <AgentFactory.hpp>
#include <MeasureAgent.hpp>
#include <TrackAgent.hpp>
#include <msk_reader.h>
#include <msk_writer.h>
#include <lbl_reader.h>
#include <Calculate/BackgStat.h>
#include <Calculate/VolumeCalculator.h>
#include <Calculate/KernelExecutor.h>
#include <Components/CompAnalyzer.h>
#include <Components/CompSelector.h>
#include <Components/CompEditor.h>
#include <Distance/RulerHandler.h>
#include <Tracking/Tracks.h>
#include <Selection/VolumeSelector.h>
#include <FLIVR/VertexArray.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/VolumeRenderer.h>
#include <utility.h>
#include <Nrrd/nrrd.h>
#include <iostream>
#include <string> 
#include <sstream>
#include <fstream> 
#include <compatibility.h>

using namespace flrd;

ScriptProc::ScriptProc() :
	m_frame(0),
	m_view(0),
	m_rewind(false)
{
	m_output = fluo::ref_ptr<fluo::Group>(new fluo::Group());
}

ScriptProc::~ScriptProc()
{
}

//run 4d script
void ScriptProc::Run4DScript(TimeMask tm, const std::wstring &scriptname, bool rewind)
{
/*	m_fconfig = 0;
	m_fconfig_name = L"";
	std::wstring scriptfile = GetInputFile(scriptname, L"Scripts");
	if (scriptfile.empty())
		return;
	m_fconfig_name = scriptfile;
	wxFileInputStream is(m_fconfig_name);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);
	m_fconfig = &fconfig;
	m_time_mask = tm;
	m_rewind = rewind;

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
				else if (str == "comp_select")
					RunCompSelect();
				else if (str == "comp_edit")
					RunCompEdit();
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
				else if (str == "export_analysis")
					ExportAnalysis();
			}
		}
	}
*/}

bool ScriptProc::TimeCondition()
{
/*	if (!m_fconfig || !m_view)
		return false;
	int time_mode;
	wxString str;
	m_fconfig->Read("time_mode", &str, "TM_ALL_PRE");
	std::string mode_str = str.ToStdString();
	time_mode = TimeMode(mode_str);
	if (m_rewind && !(time_mode & TM_REWIND))
		return false;
	long curf, startf, endf;
	m_view->getValue(gstCurrentFrame, curf);
	m_view->getValue(gstBeginFrame, startf);
	m_view->getValue(gstEndFrame, endf);
	if (startf < 0 || startf > endf ||
		curf < startf || curf > endf)
		return false;
	int tm;//mask
	int startd = curf - startf;
	int endd = endf - curf;
	if (startd < 6)
		tm = TM_FIRST_BOTH << startd*2;
	if (endd < 6)
		tm = TM_LAST_BOTH >> endd*2;
	if (startd >= 6 && endd >= 6)
		tm = 0x14000;
	//time mode
	if (m_time_mask & tm & time_mode)
		return true;
*/	return false;
}

bool ScriptProc::GetVolumes(fluo::VolumeList &list)
{
/*	if (!m_fconfig || !m_view)
		return false;
	int chan_mode;
	m_fconfig->Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;2-shown vol;3-hidden vol
	list.clear();
	if (chan_mode == 0)
	{
		fluo::VolumeData* vol = m_view->GetCurrentVolume();
		if (vol)
			list.push_back(vol);
		else
			return false;
	}
	else if (chan_mode == 1)
	{
		list = m_view->GetFullVolList();
	}
	else if (chan_mode == 2)
	{
		list = m_view->GetVolList();
	}
	else if (chan_mode == 3)
	{
		fluo::VolumeList temp = m_view->GetFullVolList();
		for (auto vd : temp)
		{
			if (!vd) continue;
			bool disp;
			vd->getValue(gstDisplay, disp);
			if (!disp) list.push_back(vd);
		}
	}
*/	return !list.empty();
}

//add traces to trace dialog
void ScriptProc::UpdateTraceDlg()
{
	//if (m_view && m_frame && m_frame->GetTraceDlg())
	//	m_frame->GetTraceDlg()->GetSettings(m_view);
}

int ScriptProc::TimeMode(std::string &str)
{
	if (str == "TM_NONE")
		return TM_NONE;
	if (str == "TM_ALL_PRE")
		return TM_ALL_PRE;
	if (str == "TM_ALL_POST")
		return TM_ALL_POST;
	if (str == "TM_FIRST_PRE")
		return TM_FIRST_PRE;
	if (str == "TM_FIRST_POST")
		return TM_FIRST_POST;
	if (str == "TM_FIRST_BOTH")
		return TM_FIRST_BOTH;
	if (str == "TM_LAST_PRE")
		return TM_LAST_PRE;
	if (str == "TM_LAST_POST")
		return TM_LAST_POST;
	if (str == "TM_LAST_BOTH")
		return TM_LAST_BOTH;
	if (str == "TM_ALL_PRE_FIRST_BOTH")
		return TM_ALL_PRE_FIRST_BOTH;
	if (str == "TM_ALL_POST_FIRST_BOTH")
		return TM_ALL_POST_FIRST_BOTH;
	if (str == "TM_ALL_PRE_LAST_BOTH")
		return TM_ALL_PRE_LAST_BOTH;
	if (str == "TM_ALL_POST_LAST_BOTH")
		return TM_ALL_POST_LAST_BOTH;
	if (str == "TM_ALL_PRE_REWIND")
		return TM_ALL_PRE_REWIND;
	if (str == "TM_ALL_POST_REWIND")
		return TM_ALL_POST_REWIND;
	if (str == "TM_ALL")
		return TM_ALL;
	return std::stoi(str, nullptr, 0);
}

int ScriptProc::GetTimeNum()
{
	long startf, endf;
	m_view->getValue(gstBeginFrame, startf);
	m_view->getValue(gstEndFrame, endf);
	if (endf >= startf)
		return endf - startf + 1;
	return 0;
}

std::wstring ScriptProc::GetInputFile(const std::wstring &str, const std::wstring &subd)
{
	std::wstring result = str;
	bool exist = false;
	if (result.empty())
		return result;
	exist = FILE_EXISTS(result);
	if (!exist)
	{
		//find in default folder
		std::wstring name = result;
		name = GET_NAME(name);
		std::wstring path = glbin.getExecutablePath();
		path = GET_PATH(path);
		result = path + GETSLASH() + subd + GETSLASH() + name;
		exist = FILE_EXISTS(result);
	}
	if (!exist)
	{
		//find in config file folder
		std::wstring path = GET_PATH(m_fconfig_name);
		result = path + GETSLASH() + str;
		exist = FILE_EXISTS(result);
	}
	if (exist)
		return result;
	else
		return L"";
}

std::wstring ScriptProc::GetSavePath(const std::wstring &str, const std::wstring &ext, bool rep)
{
	std::wstring temp = str;
	std::wstring path;
	fluo::Node* node = m_output->getChild("savepath");
	if (node)
	{
		std::wstring name;
		node->getValue("path", name);
		if (!name.empty())
			return name;
	}
	//not found
	if (temp.empty() ||
		temp == L"FILE_DLG")
	{
		//file dialog
		//path = m_frame->ScriptDialog(
		//	"Save Results",
		//	"Output file(*." + ext + ")|*." + ext,
		//	wxFD_SAVE);
		if (path.empty())
			path = GetDataDir(ext);
	}
	else if (temp == L"DATA_DIR")
	{
		path = GetDataDir(ext);
	}
	else
	{
		//specific dir
		path = temp;
		if (temp.find(L'.') != std::wstring::npos)
			path = GET_PATH(temp);
		if (!DIR_EXISTS(path))
			MkDirW(path);
		if (!DIR_EXISTS(path))
			return L"";
		if (path == temp)
		{
			wchar_t lc = path.back();
			if (lc != L'/' &&
				lc != L'\\')
				path += GETSLASH();
			path += L"output01." + ext;//not containing filename
		}
		else
			path = temp;//containing filename
	}
	if (!rep)
	{
		while (FILE_EXISTS(path))
			path = IncreaseNum(path);
	}
	node = m_output->getOrAddNode("savepath");
	path = GET_PATH(path);
	node->addSetValue("path", path);
	return path;
}

std::wstring ScriptProc::GetDataDir(const std::wstring &ext)
{
	//data dir
	if (!m_view)
		return L"";
	fluo::VolumeData* vol = m_view->GetCurrentVolume();
	if (!vol)
		return L"";
	std::wstring str;
	vol->getValue(gstDataPath, str);
	str = GET_PATH(str);
	str += GETSLASH();
	str += L"output01." + ext;
	return str;
}

std::wstring ScriptProc::RemoveExt(const std::wstring& str)
{
	int pos = str.find_last_of(L'.');
	if (pos != std::wstring::npos)
		return str.substr(0, pos-1);
	return str;
}

std::wstring ScriptProc::RemoveNum(const std::wstring& str)
{
	std::wstring tmp = RemoveExt(str);
	while (std::iswdigit(tmp.back()))
		tmp = tmp.substr(0, tmp.length()-1);
	return tmp;
}

std::wstring ScriptProc::IncreaseNum(const std::wstring& str)
{
	int pos = str.find_last_of(L'.');
	std::wstring ext;
	if (pos != std::wstring::npos)
		ext = str.substr(pos, str.length() - pos);
	std::wstring tmp = str.substr(0, pos-1);
	std::wstring digits;
	while (std::iswdigit(tmp.back()))
	{
		digits = tmp.back() + digits;
		tmp = tmp.substr(0, tmp.length() - 1);
	}
	int len = digits.length();
	if (!len)
	{
		return tmp + L"01" + ext;
	}
	long num = std::stol(digits);
	num++;
	digits = std::to_wstring(num);
	if (digits.length() < len)
	{
		std::wostringstream ss;
		ss << std::setw(len) << std::setfill(L'0') << num;
		digits = ss.str();
	}
	return tmp + digits + ext;
}

void ScriptProc::RunNoiseReduction()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	if (!calculator) return;

	double thresh, size;
	m_fconfig->Read("threshold", &thresh, 0.0);
	m_fconfig->Read("voxelsize", &size, 0.0);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_view->setRefValue(gstCurrentVolume, *i);
		calculator->SetVolumeA(*i);

		//selection
		glbin_agtf->getNoiseReduceAgent()->Preview();
		//delete
		calculator->CalculateGroup(6, "", false);
	}
*/}

void ScriptProc::RunPreTracking()
{
/*	if (!TimeCondition())
		return;

	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
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
*/}

void ScriptProc::RunPostTracking()
{
	if (!TimeCondition())
		return;

	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
	if (!cur_vol)
		UpdateTraceDlg();

	flrd::Tracks* tg = m_view->GetTraceGroup();
	if (!tg) return;

	//after updating volume
	if (tg->GetTrackMap()->GetFrameNum())
	{
		//create new id list
		long curf, prvf;
		m_view->getValue(gstCurrentFrame, curf);
		m_view->getValue(gstPreviousFrame, prvf);
		tg->SetCurTime(curf);
		tg->SetPrvTime(prvf);
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
	long nx, ny, nz;
	cur_vol->getValue(gstResX, nx);
	cur_vol->getValue(gstResY, ny);
	cur_vol->getValue(gstResZ, nz);
	//update the mask according to the new label
	unsigned long long for_size = nx * ny * nz;
	memset((void*)mask_data, 0, sizeof(uint8_t)*for_size);
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
/*	if (!TimeCondition())
		return;

	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
	if (!cur_vol) return;
	flrd::Tracks* tg = m_view->GetTraceGroup();
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
	int stsize;
	m_fconfig->Read("stsize", &stsize, 2);
	int sim;
	m_fconfig->Read("sim", &sim, 0);

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long resx, resy, resz;
	cur_vol->getValue(gstResX, resx);
	cur_vol->getValue(gstResY, resy);
	cur_vol->getValue(gstResZ, resz);
	double spcx, spcy, spcz;
	cur_vol->getValue(gstSpcX, spcx);
	cur_vol->getValue(gstSpcY, spcy);
	cur_vol->getValue(gstSpcZ, spcz);
	long bits;
	cur_vol->getValue(gstBits, bits);
	tm_processor.SetBits(bits);
	double scale;
	cur_vol->getValue(gstIntScale, scale);
	tm_processor.SetScale(scale);
	tm_processor.SetSizes(resx, resy, resz);
	tm_processor.SetSpacings(spcx, spcy, spcz);
	tm_processor.SetMaxIter(iter);
	tm_processor.SetEps(eps);
	tm_processor.SetFilterSize(fsize);
	tm_processor.SetStencilThresh(fluo::Point(stsize));
	//register file reading and deleteing functions
	tm_processor.RegisterCacheQueueFuncs(
		std::bind(&ScriptProc::ReadVolCache, this, std::placeholders::_1),
		std::bind(&ScriptProc::DelVolCache, this, std::placeholders::_1));

	long curf, prvf, beginf;
	m_view->getValue(gstCurrentFrame, curf);
	m_view->getValue(gstPreviousFrame, prvf);
	m_view->getValue(gstBeginFrame, beginf);
	tm_processor.TrackStencils(prvf, curf, ext, mode, beginf, sim);

	UpdateTraceDlg();
*/}

void ScriptProc::RunRandomColors()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
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
*/}

void ScriptProc::RunCompSelect()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	int mode;
	m_fconfig->Read("mode", &mode, 0);
	int comp_min;
	m_fconfig->Read("comp_min", &comp_min, 0);
	int comp_max;
	m_fconfig->Read("comp_max", &comp_max, 0);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		flrd::ComponentSelector comp_selector(*i);

		switch (mode)
		{
		case 0:
			comp_selector.All();
			break;
		case 1:
			comp_selector.Clear();
			break;
		case 2:
		default:
			if (comp_min)
				comp_selector.SetMinNum(true, comp_min);
			if (comp_max)
				comp_selector.SetMaxNum(true, comp_max);
			comp_selector.Select(true);
		}
	}
*/}

void ScriptProc::RunCompEdit()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	int edit_type;
	m_fconfig->Read("edit_type", &edit_type, 0);
	int mode;
	m_fconfig->Read("mode", &mode, 0);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		flrd::ComponentEditor editor;
		editor.SetVolume(*i);

		switch (edit_type)
		{
		case 0:
			editor.Clean(mode);
			break;
		}
	}
*/}

void ScriptProc::RunFetchMask()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, 1);
	m_fconfig->Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		BaseReader* reader = (*i)->GetReader();
		if (!reader)
			return;
		long chan;
		(*i)->getValue(gstChannel, chan);
		//load and replace the mask
		if (bmask)
		{
			MSKReader msk_reader;
			wstring mskname = reader->GetCurMaskName(curf, chan);
			msk_reader.SetFile(mskname);
			Nrrd* mask_nrrd_new = msk_reader.Convert(curf, chan, true);
			if (mask_nrrd_new)
				(*i)->LoadMask(mask_nrrd_new);
			//else
			//	(*i)->AddEmptyMask(0, true);
		}
		//load and replace the label
		if (blabel)
		{
			LBLReader lbl_reader;
			wstring lblname = reader->GetCurLabelName(curf, chan);
			lbl_reader.SetFile(lblname);
			Nrrd* label_nrrd_new = lbl_reader.Convert(curf, chan, true);
			if (label_nrrd_new)
				(*i)->LoadLabel(label_nrrd_new);
			//else
			//	(*i)->AddEmptyLabel(0, true);
		}
	}
*/}

void ScriptProc::RunClearMask()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
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
		(*i)->GetRenderer()->clear_tex_current();
	}
*/}

void ScriptProc::RunSaveMask()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, 1);
	m_fconfig->Read("label", &blabel, 1);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		long chan;
		(*i)->getValue(gstChannel, chan);
		if (bmask)
			(*i)->SaveMask(true, curf, chan);
		if (blabel)
			(*i)->SaveLabel(true, curf, chan);
	}
*/}

void ScriptProc::RunSaveVolume()
{
/*	if (!TimeCondition())
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
	wxString pathname;
	m_fconfig->Read("savepath", &pathname, "");
	bool del_vol;
	m_fconfig->Read("delete", &del_vol, false);

	fluo::VolumeList vlist;
	if (source == "channels" ||
		source == "")
	{
		GetVolumes(vlist);
	}
	else if (source == "calculator")
	{
		VolumeCalculator* calculator = m_view->GetVolumeCalculator();
		if (!calculator) return;
		fluo::VolumeData* vd = 0;
		while (vd = calculator->GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "selector")
	{
		VolumeSelector* selector = m_view->GetVolumeSelector();
		if (!selector) return;
		fluo::VolumeData* vd = 0;
		while (vd = selector->GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "executor")
	{
		KernelExecutor* executor = m_view->GetKernelExecutor();
		if (!executor) return;
		fluo::VolumeData* vd = 0;
		while (vd = executor->GetResult(true))
			vlist.push_back(vd);
	}
	int chan_num = vlist.size();
	int time_num = GetTimeNum();
	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	wxString ext, str;
	if (mode == 0 || mode == 1)
		ext = "tif";
	else if (mode == 2)
		ext = "nrrd";
	str = GetSavePath(pathname, ext);
	str = RemoveExt(str);
	str = RemoveNum(str);
	if (str.IsEmpty())
		return;
	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		//time
		wxString format = wxString::Format("%d", time_num);
		int fr_length = format.Length();
		format = wxString::Format("_T%%0%dd", fr_length);
		wxString vstr = str;
		vstr += wxString::Format(format, curf);
		//channel
		if (chan_num > 1)
		{
			format = wxString::Format("%d", chan_num);
			int ch_length = format.Length();
			format = wxString::Format("_CH%%0%dd", ch_length + 1);
			long chan;
			(*i)->getValue(gstChannel, chan);
			vstr += wxString::Format(format, chan + 1);
		}
		//ext
		vstr += "." + ext;
		fluo::Quaternion qtemp;
		(*i)->SaveData(vstr.ToStdWstring(), mode, crop, filter, bake, compression, qtemp);
		if (del_vol)
			glbin_volf->remove(*i);
	}
*/}

void ScriptProc::RunCalculate()
{
/*	if (!TimeCondition())
		return;

	VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	if (!calculator) return;

	int vol_a_index;
	m_fconfig->Read("vol_a", &vol_a_index, 0);
	int vol_b_index;
	m_fconfig->Read("vol_b", &vol_b_index, 0);
	wxString sOper;
	m_fconfig->Read("operator", &sOper, "");

	size_t vlist_size = m_view->GetVolListSize();
	//get volumes
	fluo::VolumeData* vol_a = 0;
	if (vol_a_index >= 0 && vol_a_index < vlist_size)
		vol_a = m_view->GetShownVolume(vol_a_index);
	fluo::VolumeData* vol_b = 0;
	if (vol_b_index >= 0 && vol_b_index < vlist_size)
		vol_b = m_view->GetShownVolume(vol_b_index);
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
*/}

void ScriptProc::RunOpenCL()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	KernelExecutor* executor = m_view->GetKernelExecutor();
	if (!executor) return;

	wxString clname;
	m_fconfig->Read("clpath", &clname, "");
	clname = GetInputFile(clname, "CL_code");
	if (clname.IsEmpty())
		return;

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		(*i)->GetRenderer()->clear_tex_current();
		executor->LoadCode(clname.ToStdString());
		executor->SetVolume(*i);
		executor->SetDuplicate(true);
		executor->Execute();
	}
*/}

void ScriptProc::RunCompAnalysis()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
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

	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);
	fluo::Vector lens;

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		int bn = (*itvol)->GetAllBrickNum();
		flrd::ComponentAnalyzer comp_analyzer(*itvol);
		comp_analyzer.SetSizeLimit(slimit);
		comp_analyzer.Analyze(selected, consistent);

		//output
		//time group
		fluo::Group* timeg = m_output->getOrAddGroup(fn);
		timeg->addSetValue("type", std::string("time"));
		timeg->addSetValue("t", long(curf));
		//channel group
		fluo::Group* chg = timeg->getOrAddGroup(std::to_string(ch));
		chg->addSetValue("type", std::string("channel"));
		chg->addSetValue("ch", long(ch));
		//script command
		fluo::Group* cmdg = chg->getOrAddGroup(m_type.ToStdString());
		cmdg->addSetValue("type", m_type.ToStdString());

		CelpList* celp_list = comp_analyzer.GetCelpList();
		CellGraph* graph = comp_analyzer.GetCellGraph();
		if (!celp_list || !graph)
			continue;
		double sx = celp_list->sx;
		double sy = celp_list->sy;
		double sz = celp_list->sz;
		double size_scale = sx * sy * sz;
		double maxscale;
		(*itvol)->getValue(gstMaxScale, maxscale);
		double scalarscale;
		(*itvol)->getValue(gstIntScale, scalarscale);
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

			//result node
			fluo::Node* node = cmdg->getOrAddNode(std::to_string(id));
			node->addSetValue("type", std::string("comp"));
			node->addSetValue("comp_center", itc->second->GetCenter(sx, sy, sz));
			node->addSetValue("comp_size_ui", (unsigned long)(itc->second->GetSizeUi()));
			node->addSetValue("comp_size_d", itc->second->GetSizeD(scalarscale));
			node->addSetValue("comp_phys_size_ui", size_scale * itc->second->GetSizeUi());
			node->addSetValue("comp_phys_size_d", itc->second->GetSizeD(size_scale * scalarscale));
			node->addSetValue("comp_ext_size_ui", (unsigned long)(itc->second->GetExtUi()));
			node->addSetValue("comp_ext_size_d", itc->second->GetExtD(scalarscale));
			node->addSetValue("comp_mean", itc->second->GetMean(maxscale));
			node->addSetValue("comp_stdev", itc->second->GetStd(maxscale));
			node->addSetValue("comp_min", itc->second->GetMin(maxscale));
			node->addSetValue("comp_max", itc->second->GetMax(maxscale));
			node->addSetValue("comp_distp", itc->second->GetDistp());
			node->addSetValue("comp_pca_lens", lens.x());
		}
	}
*/}

void ScriptProc::RunGenerateComp()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	bool use_sel;
	m_fconfig->Read("use_sel", &use_sel);
	double tfac = 1.0;
	m_fconfig->Read("th_factor", &tfac);
	wxString cmdfile;
	m_fconfig->Read("comp_command", &cmdfile);
	cmdfile = GetInputFile(cmdfile, "Commands");
	if (cmdfile.IsEmpty())
		glbin_agtf->getComponentAgent()->ResetCmd();
	else
		glbin_agtf->getComponentAgent()->LoadCmd(cmdfile);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_view->setRefValue(gstCurrentVolume, *i);
		glbin_agtf->getComponentAgent()->setValue(gstUseSelection, use_sel);
		glbin_agtf->getComponentAgent()->PlayCmd(use_sel, tfac);
	}
*/}

void ScriptProc::RunRulerProfile()
{
	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	RulerHandler* ruler_handler = m_view->GetRulerHandler();
	if (!ruler_handler) return;
	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

	//get df/f setting
	bool df_f = false;
	glbin_agtf->getMeasureAgent()->getValue(gstRulerDfoverf, df_f);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		ruler_handler->SetVolumeData(*itvol);
		for (size_t i = 0; i < ruler_list->size(); ++i)
			ruler_handler->Profile(i);

		//output
		//time group
		fluo::Group* timeg = m_output->getOrAddGroup(fn);
		timeg->addSetValue("type", std::string("time"));
		timeg->addSetValue("t", long(curf));
		//channel group
		fluo::Group* chg = timeg->getOrAddGroup(std::to_string(ch));
		chg->addSetValue("type", std::string("channel"));
		chg->addSetValue("ch", long(ch));
		//script command
		fluo::Group* cmdg = chg->getOrAddGroup(m_type);
		cmdg->addSetValue("type", m_type);

		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			//for each ruler
			fluo::Node* ruler_node = cmdg->getOrAddNode(std::to_string(i));
			ruler_node->addSetValue("type", std::string("ruler"));
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
					int pixels = (*profile)[j].m_pixels;
					if (pixels <= 0)
						dval = 0;
					else
					{
						dval = (*profile)[j].m_accum / pixels;
						sumd += (*profile)[j].m_accum;
						sumull += pixels;
					}
					ruler_node->addSetValue(std::to_string(j), dval);
				}
				if (df_f)
				{
					if (i == 0)
					{
						double avg = 0.0;
						if (sumull != 0)
							avg = sumd / double(sumull);
						ruler_node->addSetValue("f", avg);
					}
				}
			}
		}
	}
}

void ScriptProc::RunAddCells()
{
	if (!TimeCondition())
		return;
	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
	if (!cur_vol) return;
	flrd::Tracks* tg = m_view->GetTraceGroup();
	if (!tg)
	{
		m_view->CreateTraceGroup();
		tg = m_view->GetTraceGroup();
	}

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long bits;
	cur_vol->getValue(gstBits, bits);
	tm_processor.SetBits(bits);
	double scale;
	cur_vol->getValue(gstIntScale, scale);
	tm_processor.SetScale(scale);
	long resx, resy, resz;
	cur_vol->getValue(gstResX, resx);
	cur_vol->getValue(gstResY, resy);
	cur_vol->getValue(gstResZ, resz);
	tm_processor.SetSizes(resx, resy, resz);
	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	tm_processor.AddCells(m_sel_labels,curf);
}

void ScriptProc::RunLinkCells()
{
	if (!TimeCondition())
		return;

	glbin_agtf->getTrackAgent()->LinkAddedCells(m_sel_labels);
	//m_frame->GetTraceDlg()->GetSettings(m_view);
	//m_frame->GetTraceDlg()->LinkAddedCells(m_sel_labels);
}

void ScriptProc::RunUnlinkCells()
{
	if (!TimeCondition())
		return;

	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
	if (!cur_vol) return;
	flrd::Tracks* tg = m_view->GetTraceGroup();
	if (!tg) return;

	flrd::pTrackMap track_map = tg->GetTrackMap();
	flrd::TrackMapProcessor tm_processor(track_map);
	long bits;
	cur_vol->getValue(gstBits, bits);
	tm_processor.SetBits(bits);
	double scale;
	cur_vol->getValue(gstIntScale, scale);
	tm_processor.SetScale(scale);
	long resx, resy, resz;
	cur_vol->getValue(gstResX, resx);
	cur_vol->getValue(gstResY, resy);
	cur_vol->getValue(gstResZ, resz);
	tm_processor.SetSizes(resx, resy, resz);
	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	tm_processor.RemoveCells(m_sel_labels, curf);
}

void ScriptProc::RunBackgroundStat()
{
/*	if (!TimeCondition())
		return;
	fluo::VolumeList vlist;
	if (!GetVolumes(vlist))
		return;

	long curf;
	m_view->getValue(gstCurrentFrame, curf);
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{

		flrd::BackgStat bgs(*itvol);
		bool bval;
		m_fconfig->Read("use_mask", &bval, false);
		bgs.SetUseMask(bval);
		int itype;
		m_fconfig->Read("stat_type", &itype, 0);
		bgs.SetType(itype);
		int iindx;
		m_fconfig->Read("stat_indx", &iindx, 0);
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
		float result = bgs.GetResultf(iindx);

		//output
		//time group
		fluo::Group* timeg = m_output->getOrAddGroup(fn);
		timeg->addSetValue("type", std::string("time"));
		timeg->addSetValue("t", long(curf));
		//channel group
		fluo::Group* chg = timeg->getOrAddGroup(std::to_string(ch));
		chg->addSetValue("type", std::string("channel"));
		chg->addSetValue("ch", long(ch));
		//script command
		fluo::Group* cmdg = chg->getOrAddGroup(m_type);
		cmdg->addSetValue("type", m_type);
		//result node
		fluo::Node* node = cmdg->getOrAddNode("result");
		node->addSetValue("type", m_type);
		node->addSetValue(bgs.GetTypeName(iindx), result);
	}
*/}

void ScriptProc::ExportAnalysis()
{
/*	if (!TimeCondition())
		return;

	//template
	wxString tempfile;
	m_fconfig->Read("template", &tempfile);
	tempfile = GetInputFile(tempfile, "Templates");
	if (tempfile.IsEmpty())
		return;
	wxString outputfile;
	m_fconfig->Read("output", &outputfile);
	outputfile = GetSavePath(outputfile, "html", false);
	if (outputfile.IsEmpty())
		return;
	int vnum;
	m_fconfig->Read("value_num", &vnum, 0);
	std::set<std::string> vnames;
	for (int i = 0; i < vnum; ++i)
	{
		wxString str;
		if (m_fconfig->Read(
			wxString::Format("value_name%d", i),
			&str))
			vnames.insert(str.ToStdString());
	}
	wxString js_value;
	m_fconfig->Read("js_value", &js_value);

	//print lines
	class CompVisitor : public fluo::NodeVisitor
	{
	public:
		CompVisitor(std::ofstream &ofs,
			std::set<std::string> &vnames,
			int num) : fluo::NodeVisitor(),
			ofs_(&ofs),
			vnames_(vnames),
			chnum_(num)
		{
			setTraversalMode(fluo::NodeVisitor::TRAVERSE_CHILDREN);
		}

		virtual void apply(fluo::Node& node)
		{
			printValues(&node);
			traverse(node);
		}

		virtual void apply(fluo::Group& group)
		{
			std::string type;
			group.getValue("type", type);
			if (type == "time")
				t_ = group.getName();
			if (type == "channel")
				ch_ = group.getName();
			traverse(group);
		}

	protected:
		void printValues(fluo::Object* object)
		{
			if (!object)
				return;
			std::string str;
			object->getValue("type", str);
			if (str == "backg_stat")
			{
				for (auto it = vnames_.begin();
					it != vnames_.end(); ++it)
				{
					//local value
					fluo::ValueTuple vt{ *it, "", "" };
					if (object->getValue(vt))
					{
						auto vit = gvalues_.find(*it);
						if (vit == gvalues_.end())
							gvalues_.insert(std::pair<std::string, std::string>(
								*it, std::get<2>(vt)));
						else
							vit->second = std::get<2>(vt);
					}
				}
			}
			else if (str == "comp")
			{
				if (chnum_ > 1)
					*ofs_ << "CH-" << ch_ << " ";
				str = object->getName();
				*ofs_ << "ID-" << str << "," << t_;
				for (auto it = vnames_.begin();
					it != vnames_.end(); ++it)
				{
					*ofs_ << ",";
					//local value
					fluo::ValueTuple vt{ *it, "", "" };
					if (object->getValue(vt))
						*ofs_ << std::get<2>(vt);
					else
					{
						//global value
						auto vit = gvalues_.find(*it);
						if (vit == gvalues_.end())
							*ofs_ << "0";
						else
							*ofs_ << vit->second;
					}
				}
				*ofs_ << "\\n\\" << std::endl;
			}
		}

	private:
		std::ofstream *ofs_;
		std::set<std::string> vnames_;
		int chnum_;
		std::string t_;
		std::string ch_;
		std::unordered_map<std::string, std::string> gvalues_;
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
			ofs << "let csv_data = \"id,time";
			for (auto it = vnames.begin();
				it != vnames.end(); ++it)
				ofs << "," << *it;
			ofs << "\\n\\" << std::endl;
			CompVisitor visitor(ofs, vnames,
				m_view->GetFullVolListSize());
			m_output->accept(visitor);
			ofs << "\";" << std::endl;
		}
		if (line.find("#begin value name") != std::string::npos)
		{
			//value name
			replace = 2;
			ofs << "//#begin value name" << std::endl;
			ofs << "        value: " << js_value.ToStdString();
			if (js_value.find(',') == std::string::npos)
				ofs << ",";
			ofs << std::endl;
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

#ifdef _DARWIN
	outputfile.Replace(" ", "%20");
	outputfile = "file://" + outputfile;
#endif
	::wxLaunchDefaultBrowser(outputfile);
*/}

//read/delete volume cache
void ScriptProc::ReadVolCache(flrd::VolCache& vol_cache)
{
	if (!m_view) return;
	//get volume, readers
	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
	if (!cur_vol) return;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return;
	LBLReader lbl_reader;

	long chan;
	cur_vol->getValue(gstChannel, chan);
	int frame = vol_cache.frame;

	Nrrd* data = reader->Convert(frame, chan, true);
	vol_cache.nrrd_data = data;
	vol_cache.data = data->data;
	wstring lblname = reader->GetCurLabelName(frame, chan);
	lbl_reader.SetFile(lblname);
	Nrrd* label = lbl_reader.Convert(frame, chan, true);
	if (!label)
	{
		long resx, resy, resz;
		cur_vol->getValue(gstResX, resx);
		cur_vol->getValue(gstResY, resy);
		cur_vol->getValue(gstResZ, resz);
		double spcx, spcy, spcz;
		cur_vol->getValue(gstSpcX, spcx);
		cur_vol->getValue(gstSpcY, spcy);
		cur_vol->getValue(gstSpcZ, spcz);
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
	fluo::VolumeData* cur_vol = m_view->GetCurrentVolume();
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
		long chan;
		cur_vol->getValue(gstChannel, chan);
		int frame = vol_cache.frame;
		double spcx, spcy, spcz;
		cur_vol->getValue(gstSpcX, spcx);
		cur_vol->getValue(gstSpcY, spcy);
		cur_vol->getValue(gstSpcZ, spcz);

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

