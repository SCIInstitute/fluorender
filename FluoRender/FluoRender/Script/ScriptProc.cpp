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
#include <Distance/Camera2Ruler.h>
#include <ScriptVisitors.h>
#include <Global.h>
#include <DataManager.h>
#include <VRenderGLView.h>
#include <VRenderFrame.h>
#include <Calculate/BackgStat.h>
#include <Components/CompSelector.h>
#include <Components/CompEditor.h>
#include <Tracking/Registrator.h>
#include <Python/PyBase.h>
#include <utility.h>
#include <wx/filefn.h>
#include <wx/stdpaths.h>
#include <wx/mimetype.h>
#include <iostream>
#include <string> 
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace flrd;

ScriptProc::ScriptProc() :
	m_frame(0),
	m_view(0),
	m_break(true),
	m_rewind(false),
	m_break_count(0)
{
	m_output = fluo::ref_ptr<fluo::Group>(new fluo::Group());
}

ScriptProc::~ScriptProc()
{
}

//run 4d script
//return 0:failure; 1:normal; 2:break
int ScriptProc::Run4DScript(TimeMask tm, wxString &scriptname, bool rewind)
{
	m_fconfig = 0;
	m_fconfig_name = "";
	wxString scriptfile = GetInputFile(scriptname, "Scripts");
	if (scriptfile.IsEmpty())
		return 0;
	m_fconfig_name = scriptfile;
	wxFileInputStream is(m_fconfig_name);
	if (!is.IsOk())
		return 0;
	//wxFileConfig fconfig(is);
	m_fconfig = new wxFileConfig(is);
	m_time_mask = tm;
	m_rewind = rewind;
	if (m_rewind)
		SetBreakCount();

	int i;
	wxString str;

	//tasks
	if (m_fconfig->Exists("/tasks"))
	{
		m_fconfig->SetPath("/tasks");
		int tasknum = m_fconfig->Read("tasknum", 0l);
		for (i = 0; i < tasknum; i++)
		{
			str = wxString::Format("/tasks/task%d", i);
			if (m_fconfig->Exists(str))
			{
				m_fconfig->SetPath(str);
				m_fconfig->Read("type", &str, "");
				m_type = str;
				if (str == "break")
				{
					if (RunBreak())
						if (m_break_count == 1)
						{
							//reset on first break
							delete m_fconfig;
							return 2;
						}
				}
				else if (str == "noise_reduction")
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
				else if (str == "roi")
					RunRoi();
				else if (str == "roi_dff")
					RunRoiDff();
				else if (str == "ruler_info")
					RunRulerInfo();
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
				else if (str == "registration")
					RunRegistration();
				else if (str == "camera_points")
					RunCameraPoints();
				else if (str == "python")
					RunPython();
				else if (str == "video_analysis")
					RunDlcVideoAnalyze();
				else if (str == "get_rulers")
					RunDlcGetRulers();
				else if (str == "create_va_config")
					RunDlcCreateProj();
				else if (str == "create_va_label")
					RunDlcLabel();
				else if (str == "export_info")
					ExportInfo();
				else if (str == "export_analysis")
					ExportTemplate();
				else if (str == "export_spreadsheet")
					ExportSpreadsheet();
				else if (str == "change_data")
					ChangeData();
				else if (str == "change_script")
					ChangeScript();
				else if (str == "load_project")
					LoadProject();
			}
		}
	}

	delete m_fconfig;
	return 1;
}

bool ScriptProc::TimeCondition()
{
	if (!m_fconfig || !m_view)
		return false;
	int time_mode;
	wxString str;
	m_fconfig->Read("time_mode", &str, "TM_ALL_PRE");
	std::string mode_str = str.ToStdString();
	time_mode = TimeMode(mode_str);
	if (m_rewind)
		return time_mode & m_time_mask & TM_REWIND;
	int curf = m_view->m_tseq_cur_num;
	int startf = m_view->m_begin_frame;
	int endf = m_view->m_end_frame;
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

//add traces to trace dialog
void ScriptProc::UpdateTraceDlg()
{
	if (m_view && m_frame && m_frame->GetTraceDlg())
		m_frame->GetTraceDlg()->GetSettings(m_view);
}

int ScriptProc::TimeMode(std::string &str)
{
	if (str == "TM_NONE")
		return TM_NONE;
	if (str == "TM_REWIND")
		return TM_REWIND;
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
	int result = 0;
	try
	{
		result = std::stoi(str, nullptr, 0);
	}
	catch (...)
	{
		return result;
	}
	return result;
}

int ScriptProc::GetTimeNum()
{
	int startf = m_view->m_begin_frame;
	int endf = m_view->m_end_frame;
	if (endf >= startf)
		return endf - startf + 1;
	return 0;
}

wxString ScriptProc::GetInputFile(const wxString &str, const wxString &subd)
{
	wxString result = str;
	bool exist = false;
	if (result.IsEmpty())
		return result;
	exist = wxFileExists(result);
	if (!exist)
	{
		//find in default folder
		std::wstring name = result.ToStdWstring();
		name = GET_NAME(name);
		wxString path = wxStandardPaths::Get().GetExecutablePath();
		path = wxPathOnly(path);
		if (subd.IsEmpty())
			result = path + GETSLASH() + name;
		else
			result = path + GETSLASH() + subd + GETSLASH() + name;
		exist = wxFileExists(result);
	}
	if (!exist)
	{
		//find in config file folder
		wxString path = wxPathOnly(m_fconfig_name);
		result = path + GETSLASH() + str;
		exist = wxFileExists(result);
	}
	if (exist)
	{
		//make slash consistent
		std::filesystem::path p(result.ToStdWstring());
		result = p.make_preferred().string();
		return result;
	}
	else
		return "";
}

wxString ScriptProc::GetSavePath(const wxString &str, const wxString &ext, bool rep)
{
	wxString temp = str;
	wxString path;
	fluo::Node* node = m_output->getChild("savepath");
	if (node)
	{
		std::string name;
		node->getValue("path", name);
		if (!name.empty())
		{
			std::string ext2 = '.' + ext.ToStdString();
			if (GET_SUFFIX(name) != ext2)
				name += ext2;
			return name;
		}
	}

	//not found
	bool has_file = temp != wxPathOnly(temp);
	bool absolute = wxIsAbsolutePath(temp);

	if (temp.IsEmpty() ||
		temp == "FILE_DLG")
	{
		//file dialog
		path = m_frame->ScriptDialog(
			"Save Results",
			"Output file(*." + ext + ")|*." + ext,
			wxFD_SAVE);
		if (path.IsEmpty())
			path = GetDataDir(ext);
	}
	else if (temp == "DATA_DIR")
	{
		path = GetDataDir(ext);
	}
	else
	{
		if (absolute)
		{
			//absolute dir
			path = wxPathOnly(temp);
			if (!wxDirExists(path))
				MkDirW(path.ToStdWstring());
		}
		else
		{
			//relative
			wxString conf_path = wxPathOnly(m_fconfig_name);
			path = conf_path + GETSLASH() + wxPathOnly(temp);
			if (!wxDirExists(path))
				MkDirW(path.ToStdWstring());
		}
		if (has_file)
		{
			path += GETSLASH() + wxFileNameFromPath(temp);
		}
		else
		{
			wxString lc = path.Last();
			if (lc != "/" &&
				lc != "\\")
				path += GETSLASH();
			path += "output01." + ext;//not containing filename
		}
	}
	if (!rep)
			path = INC_NUM_EXIST(path);

	node = m_output->getOrAddNode("savepath");
	path = STR_DIR_SEP(path.ToStdString());
	node->addSetValue("path", path.ToStdString());
	return path;
}

wxString ScriptProc::GetDataDir(const wxString &ext)
{
	//data dir
	if (!m_view)
		return "";
	VolumeData* vol = m_view->m_cur_vol;
	if (!vol)
		return "";
	wxString path = vol->GetPath();
	path = wxPathOnly(path);
	path += GETSLASH();
	path += "output01." + ext;
	return path;
}

wxString ScriptProc::GetConfigFile(
	const wxString& str,
	const wxString& ext,
	const wxString& type,
	int mode)
{
	if (wxFileExists(str))
		return str;
	long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
	if (mode == 1)
		style = wxFD_SAVE | wxFD_OVERWRITE_PROMPT;
	return m_frame->ScriptDialog(
		"Choose "+type+" file",
		type+" file(*." + ext + ")|*." + ext,
		style);
}

bool ScriptProc::GetRegistrationTransform(
	fluo::Point& transl,
	fluo::Point& center,
	fluo::Point& euler,
	int sn)
{
	if (!m_view)
		return false;

	int curf = m_view->m_tseq_cur_num;
	int bgnf = m_view->m_begin_play_frame;
	typedef struct
	{
		fluo::Point t;
		fluo::Point c;
		fluo::Point e;
	} RegTrans;
	std::vector<RegTrans> list;

	int ln = std::max(bgnf + 1, curf - sn);
	for (int i = curf; i >= ln; --i)
	{
		std::string fstr = std::to_string(i);
		fluo::Node* n = m_output->getChild(fstr);
		if (!n)
			continue;
		fluo::Group* timeg = n->asGroup();
		if (!timeg)
			continue;
		fluo::Node* regg = timeg->getChild("registrator");
		if (regg)
		{
			RegTrans value;
			regg->getValue("transl", value.t);
			regg->getValue("center", value.c);
			regg->getValue("euler", value.e);
			list.push_back(value);
		}
	}

	if (list.empty())
		return false;
	double nf = (double)(list.size());
	//compute average
	RegTrans avg;
	for (auto& i : list)
	{
		avg.t += i.t;
		avg.c += i.c;
		avg.e += i.e;
	}
	avg.t /= nf;
	avg.c /= nf;
	avg.e /= nf;
	transl = avg.t;
	center = avg.c;
	euler = avg.e;
	return true;
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
	{
		UpdateTraceDlg();
		return;
	}
	unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
	unsigned int* label_data = (unsigned int*)(label_nrrd->data);
	if (!mask_data || !label_data)
	{
		UpdateTraceDlg();
		return;
	}
	int nx, ny, nz;
	cur_vol->GetResolution(nx, ny, nz);
	//update the mask according to the new label
	unsigned long long for_size = nx * ny * nz;
	std::memset((void*)mask_data, 0, sizeof(uint8)*for_size);
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

	if (m_view->m_tseq_cur_num == m_view->m_begin_play_frame)
	{
		//rewind
		flrd::ComponentSelector comp_selector(cur_vol);
		comp_selector.All();
		return;
	}

	double exttx, extty, exttz;
	m_fconfig->Read("ext_x", &exttx, 0.1);
	m_fconfig->Read("ext_y", &extty, 0.1);
	m_fconfig->Read("ext_z", &exttz, 0);
	fluo::Vector extt(exttx, extty, exttz);
	m_fconfig->Read("ext_a", &exttx, 0.1);
	m_fconfig->Read("ext_b", &extty, 0.1);
	m_fconfig->Read("ext_c", &exttz, 0);
	fluo::Vector exta(exttx, extty, exttz);
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
	tm_processor.SetStencilThresh(fluo::Point(stsize));
	//register file reading and deleteing functions
	glbin_reg_cache_queue_func(this,
		ScriptProc::ReadVolCacheDataLabel,
		ScriptProc::DelVolCacheDataLabel);

	tm_processor.TrackStencils(
		m_view->m_tseq_prv_num,
		m_view->m_tseq_cur_num,
		extt, exta,
		mode,
		m_view->m_begin_play_frame,
		sim);

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

void ScriptProc::RunCompSelect()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
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
}

void ScriptProc::RunCompEdit()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
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
			//else
			//	(*i)->AddEmptyLabel(0, true);
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
		(*i)->GetVR()->clear_tex_current();
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
	if (!m_view)
		return;
	if (!TimeCondition())
		return;

	wxString source;
	m_fconfig->Read("source", &source);
	int mode;
	m_fconfig->Read("format", &mode, 0);
	int mask;
	m_fconfig->Read("mask", &mask, 0);
	bool neg_mask = false;
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
	int smooth;
	m_fconfig->Read("smooth", &smooth, 0);

	fluo::Quaternion rot;
	fluo::Point transl, center;
	bool fix_size = false;
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
	else if (source == "registrator")
	{
		GetVolumes(vlist);
		fluo::Point euler;
		if (GetRegistrationTransform(transl, center, euler, smooth))
		{
			rot.FromEuler(euler.x(), euler.y(), euler.z());
			crop = true;
			fix_size = true;
			neg_mask = true;
		}
	}
	int chan_num = vlist.size();
	int time_num = GetTimeNum();
	int curf = m_view->m_tseq_cur_num;
	wxString ext, str;
	if (mode == 0 || mode == 1)
		ext = "tif";
	else if (mode == 2)
		ext = "nrrd";
	str = GetSavePath(pathname, ext);
	str = REM_EXT(str);
	str = REM_NUM(str);
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
			vstr += wxString::Format(format, (*i)->GetCurChannel() + 1);
		}
		//ext
		vstr += "." + ext;
		(*i)->Save(vstr, mode,
			mask, neg_mask,
			crop, filter,
			bake, compression,
			center, rot, transl,
			fix_size);
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
	clname = GetInputFile(clname, "CL_code");
	if (clname.IsEmpty())
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
}

void ScriptProc::RunGenerateComp()
{
	if (!m_frame) return;
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;
	if (!(m_frame->GetComponentDlg()))
		return;

	wxString ml_table_file;
	m_fconfig->Read("ml_table", &ml_table_file);
	ml_table_file = GetInputFile(ml_table_file, "Database");
	bool use_ml = !ml_table_file.empty();
	bool use_sel;
	m_fconfig->Read("use_sel", &use_sel, false);
	double tfac;
	m_fconfig->Read("th_factor", &tfac, 1.0);
	wxString cmdfile;
	m_fconfig->Read("comp_command", &cmdfile);
	cmdfile = GetInputFile(cmdfile, "Commands");
	if (cmdfile.IsEmpty())
		m_frame->GetComponentDlg()->ResetCmd();
	else
		m_frame->GetComponentDlg()->LoadCmd(cmdfile);

	for (auto i = vlist.begin();
		i != vlist.end(); ++i)
	{
		m_view->m_cur_vol = *i;
		if (use_ml)
		{
			flrd::TableHistParams& table = glbin.get_cg_table();
			table.open(ml_table_file.ToStdString());
			m_frame->GetComponentDlg()->ApplyRecord();
		}
		else
			m_frame->GetComponentDlg()->PlayCmd(use_sel, tfac);
	}
}

void ScriptProc::RunRulerProfile()
{
	if (!m_frame) return;
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	RulerHandler* ruler_handler = m_view->GetRulerHandler();
	if (!ruler_handler) return;
	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	int ival;
	double dval;
	m_fconfig->Read("fsize", &ival, 1);
	ruler_handler->SetFsize(ival);
	m_fconfig->Read("sample_type", &ival, 1);
	ruler_handler->SetSampleType(ival);
	m_fconfig->Read("step_len", &dval, 1);
	ruler_handler->SetStepLength(dval);
	bool bg_int = ruler_handler->GetBackground();
	ruler_handler->SetBackground(false);

	int curf = m_view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

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
		fluo::Group* cmdg = chg->getOrAddGroup(m_type.ToStdString());
		cmdg->addSetValue("type", m_type.ToStdString());

		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			//for each ruler
			flrd::Ruler* ruler = (*ruler_list)[i];
			if (!ruler) continue;
			if (!ruler->GetDisp()) continue;
			ruler->SetWorkTime(curf);
			fluo::Node* ruler_node = cmdg->getOrAddNode(std::to_string(ruler->Id()+1));
			ruler_node->addSetValue("type", std::string("ruler"));

			vector<flrd::ProfileBin>* profile = ruler->GetProfile();
			if (profile && profile->size())
			{
				double dval, dist;
				//max intensity
				ruler->GetProfileMaxValue(dval, dist);
				ruler_node->addSetValue("max_int", dval);
				ruler_node->addSetValue("max_dist", dist);
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
			}
		}
	}
	ruler_handler->SetBackground(bg_int);
}

void ScriptProc::RunRoi()
{
	if (!TimeCondition())
		return;
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	RulerHandler* ruler_handler = m_view->GetRulerHandler();
	if (!ruler_handler) return;
	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	int curf = m_view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		ruler_handler->SetVolumeData(*itvol);

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

		for (size_t i = 0; i < ruler_list->size(); ++i)
		{
			//for each ruler
			flrd::Ruler* ruler = (*ruler_list)[i];
			if (!ruler) continue;
			if (!ruler->GetDisp()) continue;
			ruler->SetWorkTime(curf);

			if (ruler_handler->Roi(ruler))
			{
				fluo::Node* ruler_node = cmdg->getOrAddNode(std::to_string(ruler->Id() + 1));
				ruler_node->addSetValue("type", std::string("ruler"));

				double mean_int = ruler->GetMeanInt();
				ruler_node->addSetValue("roi_mean", mean_int);
			}
		}
	}
}

void ScriptProc::RunRoiDff()
{
	if (!TimeCondition())
		return;

	wxString str;
	std::string valname, bgname;
	if (m_fconfig->Read("value_name", &str))
		valname = str.ToStdString();
	if (m_fconfig->Read("bg_name", &str))
		bgname = str.ToStdString();
	double var = 0.001;
	m_fconfig->Read("var_cut", &var);
	int mode = 0;
	m_fconfig->Read("output_mode", &mode);//0:output to template; 1:output to csv
	int filter = 0;
	m_fconfig->Read("filter_mode", &filter);//0:constant; 1:ls

	RoiVisitor visitor(valname, bgname, var, mode, filter);
	m_output->accept(visitor);
	visitor.computeOut();
	visitor.output(m_output.get());
}

void ScriptProc::RunRulerInfo()
{
	if (!TimeCondition())
		return;
	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	int curf = m_view->m_tseq_cur_num;
	std::string fn = std::to_string(curf);
	//output
	//script command
	fluo::Group* cmdg = m_output->getOrAddGroup(m_type.ToStdString());
	cmdg->addSetValue("type", m_type.ToStdString());

	for (size_t i = 0; i < ruler_list->size(); ++i)
	{
		//for each ruler
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		fluo::Group* ruler_group = cmdg->getOrAddGroup(std::to_string(ruler->Id() + 1));
		ruler_group->addSetValue("type", std::string("ruler"));
		ruler_group->addSetValue("name", ruler->GetName().ToStdString());
		ruler->SetWorkTime(curf);
		for (size_t j = 0; j < ruler->GetNumPoint(); ++j)
		{
			fluo::Node* point_node = ruler_group->getOrAddNode(std::to_string(j));
			fluo::Point point = ruler->GetPoint(j);
			point_node->addSetValue(fn, point);
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

	m_frame->GetTraceDlg()->GetSettings(m_view);
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
	std::vector<VolumeData*> vlist;
	if (!GetVolumes(vlist))
		return;

	int curf = m_view->m_tseq_cur_num;
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
		fluo::Group* cmdg = chg->getOrAddGroup(m_type.ToStdString());
		cmdg->addSetValue("type", m_type.ToStdString());
		//result node
		fluo::Node* node = cmdg->getOrAddNode("result");
		node->addSetValue("type", m_type.ToStdString());
		node->addSetValue(bgs.GetTypeName(iindx), result);
	}
}

void ScriptProc::RunRegistration()
{
	if (!m_view)
		return;
	if (!TimeCondition())
		return;

	//always work on the selected volume
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int curf = m_view->m_tseq_cur_num;
	int prvf = m_view->m_tseq_prv_num;
	int bgnf = m_view->m_begin_play_frame;
	std::string curfstr = std::to_string(curf);
	std::string prvfstr = std::to_string(prvf);

	bool use_mask;
	m_fconfig->Read("use_mask", &use_mask, false);
	double exttx, extty, exttz;
	m_fconfig->Read("ext_x", &exttx, 0.1);
	m_fconfig->Read("ext_y", &extty, 0.1);
	m_fconfig->Read("ext_z", &exttz, 0);
	fluo::Vector extt(exttx, extty, exttz);
	m_fconfig->Read("ext_a", &exttx, 0.1);
	m_fconfig->Read("ext_b", &extty, 0.1);
	m_fconfig->Read("ext_c", &exttz, 0);
	fluo::Vector exta(exttx, extty, exttz);
	int iter;
	m_fconfig->Read("iter", &iter, 50);
	int plevel;
	m_fconfig->Read("plevel", &plevel, 4);
	double eps;
	m_fconfig->Read("eps", &eps, 1e-3);
	//precision level
	plevel = std::max(plevel, int(-std::log10(eps)));
	int fsize;
	m_fconfig->Read("fsize", &fsize, 1);
	int mode;
	m_fconfig->Read("compare", &mode, 1);
	int sim;
	m_fconfig->Read("sim", &sim, 1);

	//rewind
	if (curf == bgnf)
	{
		m_view->SetObjCtrOff(0, 0, 0);
		m_view->SetObjRotOff(0, 0, 0);
		fluo::Transform tf;
		tf.load_identity();
		m_view->SetOffsetTransform(tf);
		return;
	}
	//save transformation for each time
	//time group
	fluo::Group* timeg = m_output->getOrAddGroup(prvfstr);
	fluo::Node* regg_prv = timeg->getChild("registrator");
	if (!regg_prv)
	{
		regg_prv = timeg->getOrAddNode("registrator");
		//no transformation for fisrt time point
		regg_prv->addSetValue("transl", fluo::Point());
		regg_prv->addSetValue("center", fluo::Point());
		regg_prv->addSetValue("euler", fluo::Point());
		regg_prv->addSetValue("rot", fluo::Quaternion());
		regg_prv->addSetValue("transform", fluo::Transform());
	}
	timeg = m_output->getOrAddGroup(curfstr);
	timeg->addSetValue("type", std::string("time"));
	timeg->addSetValue("t", long(curf));
	//registration group
	fluo::Node* regg_cur = timeg->getOrAddGroup("registrator");
	fluo::Point pt, pr;

	flrd::Registrator registrator;
	registrator.SetUseMask(use_mask);
	registrator.SetExtension(extt, exta);
	registrator.SetMaxIter(iter);
	registrator.SetConvNum(plevel);
	registrator.SetFilterSize(fsize);
	registrator.SetMethod(sim);
	registrator.SetVolumeData(cur_vol);
	if (use_mask)
		glbin_reg_cache_queue_func(this,
			ScriptProc::ReadVolCacheDataMask,
			ScriptProc::DelVolCacheData);
	else
		glbin_reg_cache_queue_func(this,
			ScriptProc::ReadVolCacheData,
			ScriptProc::DelVolCacheData);
	fluo::Point transl, transl2, center, center2, euler;
	fluo::Transform tf;
	fluo::Quaternion rot;
	if (regg_prv)
	{
		if (regg_prv->getValue("transl", transl))
			registrator.SetTranslate(transl);
		if (regg_prv->getValue("center", center))
			registrator.SetCenter(center);
		if (regg_prv->getValue("euler", euler))
			registrator.SetEuler(euler);
		if (regg_prv->getValue("transform", tf))
			registrator.SetTransform(tf);
	}

	if (registrator.Run(prvf, curf, mode, bgnf))
	{
		transl = registrator.GetTranslate();
		transl2 = registrator.GetTranslateVol();
		center = registrator.GetCenter();
		center2 = registrator.GetCenterVol();
		euler = registrator.GetEuler();
		rot.FromEuler(euler.x(), euler.y(), euler.z());
		tf = registrator.GetTransform();
		regg_cur->addSetValue("transl", transl);
		regg_cur->addSetValue("center", center);
		regg_cur->addSetValue("euler", euler);
		regg_cur->addSetValue("rot", rot);
		regg_cur->addSetValue("transform", tf);
		//apply transform to current view
		m_view->SetObjCtrOff(transl2.x(), transl2.y(), transl2.z());
		m_view->SetObjRotCtrOff(center2.x(), center2.y(), center2.z());
		m_view->SetObjRotOff(euler.x(), euler.y(), euler.z());
		m_view->SetOffsetTransform(tf);
	}
}

void ScriptProc::RunCameraPoints()
{
	if (!m_view)
		return;
	if (!TimeCondition())
		return;

	wxString prj2;
	m_fconfig->Read("project_file", &prj2);
	prj2 = GetConfigFile(prj2, "vrp", "FluoRender Project", 0);
	double scale;
	m_fconfig->Read("scale", &scale, 1000);
	double focal;
	m_fconfig->Read("focal", &focal, 2);
	int correct;
	m_fconfig->Read("correct", &correct);
	wxString name1, name2;
	m_fconfig->Read("name1", &name1);
	m_fconfig->Read("name2", &name2);

	RulerList* ruler_list = m_view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol)
		return;
	int nx, ny, nz;
	cur_vol->GetResolution(nx, ny, nz);

	Camera2Ruler c2r;
	c2r.SetImageSize(nx, ny);
	c2r.SetScale(scale);
	c2r.SetFocal(focal);
	c2r.SetList(1, ruler_list);
	c2r.SetRange(1, m_view->m_begin_frame, m_view->m_end_frame);
	c2r.SetList(2, prj2.ToStdString());
	std::set<std::string> names;
	names.insert(name1.ToStdString());
	names.insert(name2.ToStdString());
	c2r.SetNames(names);
	c2r.Run();
	if (correct == 2)
		c2r.Correct(name1.ToStdString(), name2.ToStdString());
	RulerList* result_list = c2r.GetResult();
	if (result_list && !result_list->empty())
	{
		ruler_list->DeleteRulers();
		ruler_list->assign(result_list->begin(), result_list->end());
		delete result_list;
	}
}

void ScriptProc::RunPython()
{
	if (!TimeCondition())
		return;

	wxString cmd;
	m_fconfig->Read("command", &cmd);
	if (cmd.IsEmpty())
		return;
	flrd::PyBase* python = glbin.get_add_pybase("python");
	if (!python)
		return;
	python->Init();
	python->Run(flrd::PyBase::ot_Run_SimpleString, cmd.ToStdString());
	python->Exit();
}

void ScriptProc::RunDlcVideoAnalyze()
{
	if (!m_view)
		return;
	if (!TimeCondition())
		return;
	//always work on the selected volume
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	flrd::PyDlc* dlc = glbin.get_add_python<flrd::PyDlc>("dlc");
	if (!dlc)
		return;
	if (dlc->GetState() == 2)
		return;//busy, already created

	wxString filename;
	m_fconfig->Read("config", &filename);
	filename = GetConfigFile(filename, "yaml", "Config", 0);
	std::string fn = cur_vol->GetPath().ToStdString();
	std::string cfg = filename.ToStdString();
	if (fn.empty() || cfg.empty())
		return;

	fluo::Group* dlcg = m_output->getOrAddGroup("dlc");
	dlcg->addSetValue(fn, false);

	//run dlc
	dlc->Init();
	dlc->LoadDlc();
	dlc->SetConfigFile(cfg);
	dlc->SetVideoFile(fn);
	dlc->AnalyzeVideo();
}

void ScriptProc::RunDlcGetRulers()
{
	if (!m_frame || !m_view)
		return;
	if (!TimeCondition())
		return;
	//always work on the selected volume
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	int toff = 0;
	m_fconfig->Read("time_offset", &toff, 0);
	fluo::Group* dlcg = m_output->getOrAddGroup("dlc");
	bool analyzed = false;
	std::string fn = cur_vol->GetPath().ToStdString();
	dlcg->getValue(fn, analyzed);
	if (analyzed)
		return;

	flrd::PyDlc* dlc = glbin.get_add_python<flrd::PyDlc>("dlc");
	if (!dlc)
		return;
	if (!dlc->GetResultFile())
	{
		//busy
		if (m_view->m_tseq_cur_num == m_view->m_end_frame)
			m_frame->GetMovieView()->Reset();//rewind and restart video
		return;
	}

	RulerHandler* rhdl = m_view->GetRulerHandler();
	if (!rhdl)
		return;
	//std::filesystem::path p(fn);
	//if (p.extension().string() != ".m4v")//dlc may have problem decoding m4v files
	//	toff = 0;

	int errs = dlc->GetDecodeErrorCount();
	dlc->AddRulers(rhdl, toff + errs);
	dlc->Exit();
	dlcg->addSetValue(fn, true);
}

void ScriptProc::RunDlcCreateProj()
{
	if (!m_frame || !m_view)
		return;
	if (!TimeCondition())
		return;
	//always work on the selected volume
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;

	flrd::PyDlc* dlc = glbin.get_add_python<flrd::PyDlc>("dlc");
	if (!dlc)
		return;
	RulerHandler* rhdl = m_view->GetRulerHandler();
	if (!rhdl)
		return;

	//resoultion
	int nx, ny, nz;
	cur_vol->GetResolution(nx, ny, nz);
	dlc->SetFrameSize(nx, ny);
	//range
	dlc->SetFrameNumber(m_view->m_end_all_frame);
	dlc->SetFrameRange(m_view->m_begin_frame, m_view->m_end_frame);

	wxString filename;
	m_fconfig->Read("config", &filename);
	filename = GetConfigFile(filename, "yaml", "Config", 1);
	dlc->SetConfigFile(filename.ToStdString());
	std::string str = cur_vol->GetPath().ToStdString();
	dlc->SetVideoFile(str);
	std::filesystem::path p(str);
	str = p.stem().string();
	dlc->CreateConfigFile(str, "FluoRender", rhdl);
}

void ScriptProc::RunDlcLabel()
{
	if (!m_view)
		return;
	if (!TimeCondition())
		return;
	RulerHandler* rhdl = m_view->GetRulerHandler();
	if (!rhdl)
		return;
	flrd::PyDlc* dlc = glbin.get_add_python<flrd::PyDlc>("dlc");
	if (!dlc)
		return;

	int curf = m_view->m_tseq_cur_num;
	int endf = m_view->m_end_frame;
	int fn = m_view->m_total_frames;
	int temp = fn;
	int fn_len = 0;
	while (temp)
	{
		temp /= 10;
		fn_len++;
	}
	//write frame
	std::set<size_t> keys;
	rhdl->GetKeyFrames(keys);
	if (keys.find(size_t(curf)) != keys.end())
	{
		int vn = std::min(m_view->GetAllVolumeNum(), 3);
		VolumeData* vd_rgb[3] = { 0, 0, 0 };
		int nx, ny, nz;
		for (int i = 0; i < vn; ++i)
			vd_rgb[i] = m_view->GetAllVolumeData(i);

		//get data
		vd_rgb[0]->GetResolution(nx, ny, nz);
		char* image = new char[nx*ny*3]();
		size_t isize = nx * ny;
		char* datar = 0;
		if (vd_rgb[0])
			datar = (char*)(vd_rgb[0]->GetTexture()->get_nrrd(0)->data);
		char* datag = 0;
		if (vd_rgb[1])
			datag = (char*)(vd_rgb[1]->GetTexture()->get_nrrd(0)->data);
		char* datab = 0;
		if (vd_rgb[2])
			datab = (char*)(vd_rgb[2]->GetTexture()->get_nrrd(0)->data);
		for (int i = 0; i < isize; ++i)
		{
			if (datar)
				image[i * 3] = datar[i];
			if (datag)
				image[i * 3 + 1] = datag[i];
			if (datab)
				image[i * 3 + 2] = datab[i];
		}

		//get path
		std::string filename = dlc->GetLabelPath();
		std::ostringstream oss;
		oss << std::setw(fn_len) << std::setfill('0') << curf;
		filename = filename + GETSLASHA() + "img" + oss.str() + ".tif";

		//write tiff
		TIFF* out = TIFFOpen(filename.c_str(), "wb");
		if (!out)
			return;
		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, nx);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, ny);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3);
		TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		//dpi
		TIFFSetField(out, TIFFTAG_XRESOLUTION, 72);
		TIFFSetField(out, TIFFTAG_YRESOLUTION, 72);
		TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		tsize_t linebytes = 3 * nx;
		void* buf = NULL;
		buf = _TIFFmalloc(linebytes);
		for (uint32 row = 0; row < (uint32)ny; row++)
		{
			unsigned char* line = ((unsigned char*)image) + row * 3 * nx;
			memcpy(buf, line, linebytes);
			if (TIFFWriteScanline(out, buf, row, 0) < 0)
				break;
		}
		TIFFClose(out);
		if (buf)
			_TIFFfree(buf);
		if (image)
			delete[]image;
	}
	
	if (curf == endf)
	{
		//write hdf
		dlc->SetFrameNumber(fn);
		dlc->WriteHDF(rhdl);
		int maxiters = 100;
		m_fconfig->Read("maxiters", &maxiters, 100);

#ifdef _DARWIN
		dlc->Init();
		dlc->LoadDlc();
		dlc->Train(maxiters);
#else
		std::string cmd = dlc->GetTrainCmd(maxiters);
		m_frame->GetMovieView()->HoldRun();
		wxExecute(cmd);
		m_frame->GetMovieView()->ResumeRun();
#endif
	}
}

void ScriptProc::ExportInfo()
{
	if (!TimeCondition())
		return;

	wxString outputfile;
	m_fconfig->Read("output", &outputfile);
	outputfile = GetSavePath(outputfile, "csv", false);
	if (outputfile.IsEmpty())
		return;
	int tnum;
	m_fconfig->Read("type_num", &tnum, 0);
	std::set<std::string> tnames;
	for (int i = 0; i < tnum; ++i)
	{
		wxString str;
		if (m_fconfig->Read(
			wxString::Format("type_name%d", i),
			&str))
			tnames.insert(str.ToStdString());
	}

	//print lines
	std::ofstream ofs(outputfile.ToStdString());
	OutCoordVisitor visitor(ofs, tnames);
	m_output->accept(visitor);
	ofs.close();

	wxMimeTypesManager manager;
	wxFileType* filetype = manager.GetFileTypeFromExtension("csv");
	if (filetype)
	{
		wxString command = filetype->GetOpenCommand(outputfile);
		m_frame->GetMovieView()->HoldRun();
		wxExecute(command);
		m_frame->GetMovieView()->ResumeRun();
	}
}

void ScriptProc::ExportTemplate()
{
	if (!m_frame) return;
	if (!TimeCondition())
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
	std::ifstream ifs(tempfile.ToStdString());
	std::ofstream ofs(outputfile.ToStdString());
	std::string line;
	int replace = 0;//1:data;2:value name;
	while (std::getline(ifs, line))
	{
		//if d3 is local, set path to working dir
		if (line.find("<script src=\"./d3") != std::string::npos)
		{
			//source
			ofs << "    <script src=\"";
			wxString path = wxStandardPaths::Get().GetExecutablePath();
			path = wxPathOnly(path);
			ofs << path.ToStdString();
			ofs << "/Templates/d3.v4.min.js\"></script>" << std::endl;
			continue;
		}
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
			OutTempVisitor visitor(ofs, vnames,
				m_view->GetAllVolumeNum());
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
	m_frame->GetMovieView()->HoldRun();
	::wxLaunchDefaultBrowser(outputfile);
	m_frame->GetMovieView()->ResumeRun();
}

void ScriptProc::ExportSpreadsheet()
{
	if (!TimeCondition())
		return;

	wxString outputfile;
	m_fconfig->Read("output", &outputfile);
	outputfile = GetSavePath(outputfile, "csv", false);
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

	//print lines
	std::ofstream ofs(outputfile.ToStdString());
	OutCsvVisitor visitor(ofs, vnames);
	m_output->accept(visitor);
	ofs.close();

	wxMimeTypesManager manager;
	wxFileType* filetype = manager.GetFileTypeFromExtension("csv");
	if (filetype)
	{
		wxString command = filetype->GetOpenCommand(outputfile);
		m_frame->GetMovieView()->HoldRun();
		wxExecute(command);
		m_frame->GetMovieView()->ResumeRun();
	}
}

void ScriptProc::ChangeData()
{
	if (!m_frame)
		return;
	if (!TimeCondition())
		return;

	bool clear;
	m_fconfig->Read("clear", &clear, true);
	wxString filename;
	m_fconfig->Read("input", &filename, "");
	filename = GetInputFile(filename, "Data");
	bool imagej;
	m_fconfig->Read("imagej", &imagej, false);

	VolumeData* vd = 0;
	if (clear)
	{
		//m_frame->GetTree()->DeleteAll();
		m_view->GetRulerHandler()->DeleteAll(false);
		m_frame->GetDataManager()->ClearAll();
		m_frame->GetAdjustView()->SetVolumeData(0);
		m_frame->GetAdjustView()->SetGroup(0);
		m_frame->GetAdjustView()->SetGroupLink(0);
		m_frame->GetView(0)->ClearAll();
		DataGroup::ResetID();
		MeshGroup::ResetID();
	}
	if (!filename.IsEmpty())
	{
		wxArrayString files;
		files.Add(filename);
		m_frame->LoadVolumes(files, imagej);
		m_view->m_cur_vol_save = m_view->GetAllVolumeData(0);
	}
}

void ScriptProc::ChangeScript()
{
	if (!m_frame)
		return;
	if (!TimeCondition())
		return;

	bool run_script;
	m_fconfig->Read("run_script", &run_script, false);
	wxString filename;
	m_fconfig->Read("script_file", &filename, "");
	filename = GetInputFile(filename, "Scripts");

	if (!run_script)
		m_frame->GetSettingDlg()->SetRunScript(run_script);
	if (!filename.IsEmpty())
		m_frame->GetSettingDlg()->SetScriptFile(filename);
	m_frame->GetMovieView()->GetScriptSettings(false);
	m_fconfig_name = filename;
}

void ScriptProc::LoadProject()
{
	if (!m_frame)
		return;
	if (!TimeCondition())
		return;

	wxString filename;
	m_fconfig->Read("project_file", &filename, "");
	filename = GetInputFile(filename, "Data");

	m_frame->OpenProject(filename);
}

bool ScriptProc::RunBreak()
{
	if (!m_frame)
		return false;
	if (!m_break)
		return false;
	m_break_count++;
	if (!TimeCondition())
		return false;

	wxString info;
	m_fconfig->Read("info", &info, "");
	//info.Replace("\n", "\n");
	bool reset = false;
	m_fconfig->Read("reset", &reset, false);

	ScriptBreakDlg* dlg = m_frame->GetScriptBreakDlg();
	if (!dlg)
		return false;
	dlg->SetScriptName(m_fconfig_name);
	dlg->SetInfo(info);
	dlg->Hold();
	return reset;
}

//read/delete volume cache
void ScriptProc::ReadVolCacheData(flrd::VolCache& vol_cache)
{
	if (!m_view) return;
	//get volume, readers
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return;

	int chan = cur_vol->GetCurChannel();
	int frame = vol_cache.frame;

	Nrrd* data = reader->Convert(frame, chan, true);
	vol_cache.nrrd_data = data;
	vol_cache.data = data->data;
	if (data)
		vol_cache.valid = true;
}

void ScriptProc::ReadVolCacheDataMask(flrd::VolCache& vol_cache)
{
	if (!m_view) return;
	//get volume, readers
	VolumeData* cur_vol = m_view->m_cur_vol;
	if (!cur_vol) return;
	BaseReader* reader = cur_vol->GetReader();
	if (!reader)
		return;

	int chan = cur_vol->GetCurChannel();
	int frame = vol_cache.frame;

	Nrrd* data = reader->Convert(frame, chan, true);
	vol_cache.nrrd_data = data;
	if (data)
		vol_cache.data = data->data;

	Nrrd* mask = cur_vol->GetMask(true);
	vol_cache.nrrd_mask = mask;
	if (mask)
		vol_cache.mask = mask->data;

	if (data)
		vol_cache.valid = true;
}

void ScriptProc::ReadVolCacheDataLabel(flrd::VolCache& vol_cache)
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
		unsigned int *val32 = new (std::nothrow) unsigned int[mem_size]();
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

void ScriptProc::DelVolCacheData(flrd::VolCache& vol_cache)
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
}

void ScriptProc::DelVolCacheDataLabel(flrd::VolCache& vol_cache)
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

