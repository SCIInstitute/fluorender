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
#include <ScriptProc.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <TrackDlg.h>
#include <NoiseCancellingDlg.h>
#include <ComponentDlg.h>
#include <MoviePanel.h>
#include <OutputAdjPanel.h>
#include <ScriptBreakDlg.h>
#include <RenderView.h>
#include <Root.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <MeshData.h>
#include <MeshGroup.h>
#include <TrackGroup.h>
#include <DataManager.h>
#include <BackgStat.h>
#include <CompEditor.h>
#include <WalkCycle.h>
#include <Registrator.h>
#include <PyBase.h>
#include <Camera2Ruler.h>
#include <ScriptVisitors.h>
#include <TreeFileFactory.h>
#include <VolumeCalculator.h>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <CompGenerator.h>
#include <VolumeSelector.h>
#include <VertexArray.h>
#include <VolumeRenderer.h>
#include <Texture.h>
#include <KernelExecutor.h>
#include <TrackMap.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <MovieMaker.h>
#include <BaseConvVolMesh.h>
#include <ColorMesh.h>
#include <msk_reader.h>
#include <msk_writer.h>
#include <lbl_reader.h>
#include <image_capture_factory.h>
#include <TableHistParams.h>
#include <Project.h>
#include <PyDlc.h>
#include <VolCache4D.h>
#include <Cell.h>
#include <compatibility.h>
#include <iostream>
#include <string> 
#include <sstream>
#include <fstream>
#include <filesystem>
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif
using namespace flrd;

ScriptProc::ScriptProc() :
	m_frame(0),
	m_break(true),
	m_rewind(false),
	m_break_count(0)
{
	m_output = fluo::ref_ptr<fluo::Group>(new fluo::Group());
	m_sel_labels = std::make_unique<CelpList>();
}

ScriptProc::~ScriptProc()
{
	if (m_fconfig)
		m_fconfig.reset();
}

void ScriptProc::LoadScriptFile()
{
	std::wstring scriptfile = glbin_settings.m_script_file;
	if (!scriptfile.empty() &&
		m_fconfig_name != scriptfile)
	{
		m_fconfig = glbin_tree_file_factory.createTreeFile(scriptfile, gstScriptFile);
		if (m_fconfig)
		{
			m_fconfig->LoadFile(scriptfile);
		}
	}
}

//run 4d script
//return 0:failure; 1:normal; 2:break
int ScriptProc::Run4DScript(TimeMask tm, bool rewind)
{
	LoadScriptFile();
	if (!m_fconfig)
		return 0;

	m_frame = glbin_current.mainframe;
	m_view = glbin_current.render_view;

	m_time_mask = tm;
	m_rewind = rewind;
	if (m_rewind)
		SetBreakCount();

	int i;
	std::string str;

	//tasks
	if (m_fconfig->Exists("/tasks"))
	{
		m_fconfig->SetPath("/tasks");
		long tasknum = 0;
		m_fconfig->Read("tasknum", &tasknum, 0l);
		for (i = 0; i < tasknum; i++)
		{
			str = "/tasks/task" + std::to_string(i);
			if (m_fconfig->Exists(str))
			{
				m_fconfig->SetPath(str);
				m_fconfig->Read("type", &str, std::string(""));
				m_type = str;
				if (str == "break")
				{
					if (RunBreak())
						if (m_break_count == 1)
						{
							//reset on first break
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
				else if (str == "comp_ruler")
					RunCompRuler();
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
				else if (str == "ruler_transform")
					RunRulerTransform();
				else if (str == "ruler_speed")
					RunRulerSpeed();
				else if (str == "generate_walk")
					RunGenerateWalk();
				else if (str == "convert_mesh")
					RunConvertMesh();
				else if (str == "save_volume")
					RunSaveVolume();
				else if (str == "save_mesh")
					RunSaveMesh();
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
				else if (str == "disable_script")
					DisableScript();
			}
		}
	}

	return 1;
}

bool ScriptProc::TimeCondition()
{
	auto view = m_view.lock();
	if (!m_fconfig || !view)
		return false;
	int time_mode;
	std::string mode_str;
	m_fconfig->Read("time_mode", &mode_str, std::string("TM_ALL_PRE"));
	time_mode = TimeMode(mode_str);
	if (m_rewind)
		return time_mode & m_time_mask & TM_REWIND;
	int curf = view->m_tseq_cur_num;
	int startf = view->m_begin_frame;
	int endf = view->m_end_frame;
	if (startf < 0 || startf > endf ||
		curf < startf || curf > endf)
		return false;
	int tm;//mask
	int startd = curf - startf;
	int endd = endf - curf;
	if (startd < 6)
		tm = TM_FIRST_BOTH << startd * 2;
	if (endd < 6)
		tm = TM_LAST_BOTH >> endd * 2;
	if (startd >= 6 && endd >= 6)
		tm = 0x14000;
	//time mode
	if (m_time_mask & tm & time_mode)
		return true;
	return false;
}

bool ScriptProc::GetVolumes(std::vector<std::shared_ptr<VolumeData>>& list)
{
	auto view = m_view.lock();
	if (!m_fconfig || !view)
		return false;
	int chan_mode;
	m_fconfig->Read("chan_mode", &chan_mode, 0);//0-cur vol;1-every vol;2-shown vol;3-hidden vol
	list.clear();
	if (chan_mode == 0)
	{
		auto vol = glbin_current.vol_data.lock();
		if (vol)
			list.push_back(vol);
		else
			return false;
	}
	else if (chan_mode == 1)
	{
		for (int i = 0; i < view->GetAllVolumeNum(); ++i)
			list.push_back(view->GetAllVolumeData(i));
	}
	else if (chan_mode == 2)
	{
		for (int i = 0; i < view->GetDispVolumeNum(); ++i)
			list.push_back(view->GetDispVolumeData(i));
	}
	else if (chan_mode == 3)
	{
		for (int i = 0; i < view->GetAllVolumeNum(); ++i)
		{
			auto vol = view->GetAllVolumeData(i);
			if (!vol->GetDisp())
				list.push_back(vol);
		}
	}
	return !list.empty();
}

//add traces to trace dialog
void ScriptProc::UpdateTraceDlg()
{
	//if (m_frame && m_frame->GetTrackDlg())
	//	m_frame->GetTrackDlg()->FluoUpdate();
	if (m_frame)
		m_frame->FluoUpdate(
			{ gstTrackList });
}

int ScriptProc::TimeMode(std::string& str)
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
	return STOI(str);
}

int ScriptProc::GetTimeNum()
{
	auto view = m_view.lock();
	if (!view)
		return 0;
	int startf = view->m_begin_frame;
	int endf = view->m_end_frame;
	if (endf >= startf)
		return endf - startf + 1;
	return 0;
}

std::wstring ScriptProc::GetInputFile(const std::wstring& str, const std::wstring& subd)
{
	std::wstring result = str;
	bool exist = false;
	if (result.empty())
		return result;
	exist = std::filesystem::exists(result);
	if (!exist)
	{
		//find in default folder
		std::wstring name = GET_NAME(result);
		std::filesystem::path p = std::filesystem::current_path();
		if (subd.empty())
			p = p / name;
		else
			p = p / subd / name;
		result = p.wstring();
		exist = std::filesystem::exists(result);
	}
	if (!exist)
	{
		//find in config file folder
		std::wstring path = GET_PATH(m_fconfig_name);
		std::filesystem::path p = path;
		p /= str;
		result = p.wstring();
		exist = std::filesystem::exists(result);
	}
	if (exist)
	{
		//make slash consistent
		std::filesystem::path p(result);
		result = p.make_preferred().wstring();
		return result;
	}
	else
		return L"";
}

std::wstring ScriptProc::GetSavePath(const std::wstring& str, const std::wstring& ext, bool rep)
{
	std::wstring temp = str;
	std::wstring path;
	fluo::Node* node = m_output->getChild("savepath");
	if (node)
	{
		std::wstring name;
		node->getValue("path", name);
		if (!name.empty())
		{
			std::wstring ext2 = L'.' + ext;
			if (GET_SUFFIX(name) != ext2)
				name += ext2;
			return name;
		}
	}

	//not found
	bool has_file = temp != GET_PATH(temp);
	bool absolute = std::filesystem::path(temp).is_absolute();

	if (temp.empty() ||
		temp == L"FILE_DLG")
	{
		//file dialog
		path = m_frame->ScriptDialog(
			L"Save Results",
			L"Output file(*." + ext + L")|*." + ext,
			2);
		if (path.empty())
			path = GetDataDir(ext);
	}
	else if (temp == L"DATA_DIR")
	{
		path = GetDataDir(ext);
	}
	else
	{
		if (absolute)
		{
			//absolute dir
			path = GET_PATH(temp);
			if (!std::filesystem::exists(path))
				MkDirW(path);
		}
		else
		{
			//relative
			std::wstring conf_path = GET_PATH(m_fconfig_name);
			std::filesystem::path p(conf_path);
			p /= GET_PATH(temp);
			path = p.wstring();
			if (!std::filesystem::exists(path))
				MkDirW(path);
		}
		if (has_file)
		{
			std::filesystem::path p(path);
			p /= GET_NAME(temp);
			path += p.wstring();
		}
		else
		{
			CHECK_TRAILING_SLASH(path);
			path += L"output01." + ext;//not containing filename
		}
	}
	if (!rep)
		path = INC_NUM_EXIST(path);

	node = m_output->getOrAddNode("savepath");
	path = STR_DIR_SEP(path);
	node->addSetValue("path", path);
	return path;
}

std::wstring ScriptProc::GetDataDir(const std::wstring& ext)
{
	//data dir
	auto view = m_view.lock();
	if (!view)
		return L"";
	auto vol = glbin_current.vol_data.lock();
	if (!vol)
		return L"";
	std::wstring path = vol->GetPath();
	std::filesystem::path p(path);
	p = p.parent_path();
	p /= L"output01." + ext;
	path = p.wstring();
	return path;
}

std::wstring ScriptProc::GetConfigFile(
	const std::wstring& str,
	const std::wstring& ext,
	const std::wstring& type,
	int mode)
{
	if (std::filesystem::exists(str))
		return str;
	long style = 1 | 16;
	if (mode == 1)
		style = 2 | 4;
	return m_frame->ScriptDialog(
		L"Choose " + type + L" file",
		type + L" file(*." + ext + L")|*." + ext,
		style);
}

int ScriptProc::GetItems(const std::wstring& str, std::vector<std::wstring>& items)
{
	if (!items.empty())
		items.clear();
	std::wistringstream s(str);
	std::wstring item;
	while (std::getline(s >> std::ws, item, L','))
		items.push_back(item);
	return int(items.size());
}

bool ScriptProc::GetRegistrationTransform(
	fluo::Vector& transl,
	fluo::Point& center,
	fluo::Vector& euler,
	int sn)
{
	auto view = m_view.lock();
	if (!view)
		return false;

	int curf = view->m_tseq_cur_num;
	int bgnf = view->m_begin_play_frame;
	typedef struct
	{
		fluo::Vector t;
		fluo::Point c;
		fluo::Vector e;
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
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	if (!m_frame) return;
	//VolumeCalculator* calculator = m_view->GetVolumeCalculator();
	//if (!calculator) return;

	double thresh, size;
	m_fconfig->Read("threshold", &thresh, 0.0);
	m_fconfig->Read("voxelsize", &size, 0.0);

	for (auto& it : vlist)
	{
		//m_view->m_cur_vol = *i;
		glbin_vol_calculator.SetVolumeA(it);

		//selection
		glbin_comp_def.m_nr_thresh = thresh;
		glbin_comp_def.m_nr_size = size;
		if (m_frame->GetNoiseCancellingDlg())
			m_frame->GetNoiseCancellingDlg()->Preview();
		//delete
		glbin_vol_calculator.CalculateGroup(6, L"", false);
	}
}

void ScriptProc::RunPreTracking()
{
	if (!TimeCondition())
		return;

	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		UpdateTraceDlg();

	//read the size threshold
	int slimit;
	m_fconfig->Read("size_limit", &slimit, 0);
	//before updating volume
	glbin_comp_analyzer.SetVolume(cur_vol);
	glbin_comp_analyzer.Analyze();
	flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
	m_sel_labels->clear();
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
			m_sel_labels->insert(std::pair<unsigned int, flrd::Celp>
				(it->second->Id(), celp));
		}
	}
}

void ScriptProc::RunPostTracking()
{
	if (!TimeCondition())
		return;

	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		UpdateTraceDlg();

	auto view = m_view.lock();
	if (!view)
		return;

	TrackGroup* tg = view->GetTrackGroup();
	if (!tg) return;

	//after updating volume
	if (tg->GetTrackMap()->GetFrameNum())
	{
		//create new id list
		tg->SetCurTime(view->m_tseq_cur_num);
		tg->SetPrvTime(view->m_tseq_prv_num);
		tg->UpdateCellList(*m_sel_labels);
		glbin_vertex_array_manager.set_dirty(flvr::VAType::VA_Traces);
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
	std::memset((void*)mask_data, 0, sizeof(uint8_t) * for_size);
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
			if (m_sel_labels->find(label_value) != m_sel_labels->end())
				mask_data[idx] = 255;
		}
	}
	UpdateTraceDlg();
}

void ScriptProc::RunMaskTracking()
{
	if (!TimeCondition())
		return;

	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	TrackGroup* tg = view->GetTrackGroup();
	if (!tg)
	{
		view->CreateTrackGroup();
		tg = view->GetTrackGroup();
	}

	if (view->m_tseq_cur_num == view->m_begin_play_frame)
	{
		//rewind
		//flrd::ComponentSelector comp_selector(cur_vol);
		glbin_comp_selector.All();
		return;
	}

	double exttx, extty, exttz;
	m_fconfig->Read("ext_x", &exttx, 0.1);
	m_fconfig->Read("ext_y", &extty, 0.1);
	m_fconfig->Read("ext_z", &exttz, 0.0);
	fluo::Vector extt(exttx, extty, exttz);
	m_fconfig->Read("ext_a", &exttx, 0.1);
	m_fconfig->Read("ext_b", &extty, 0.1);
	m_fconfig->Read("ext_c", &exttz, 0.0);
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
	glbin_trackmap_proc.SetTrackMap(track_map);
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	double spcx, spcy, spcz;
	cur_vol->GetSpacings(spcx, spcy, spcz);
	glbin_trackmap_proc.SetBits(cur_vol->GetBits());
	glbin_trackmap_proc.SetScale(cur_vol->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	glbin_trackmap_proc.SetSpacing(spcx, spcy, spcz);
	glbin_trackmap_proc.SetMaxIter(iter);
	glbin_trackmap_proc.SetEps(eps);
	glbin_trackmap_proc.SetFilterSize(fsize);
	glbin_trackmap_proc.SetStencilThresh(fluo::Point(stsize));
	//register file reading and deleteing functions
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(cur_vol.get());
	if (cache_queue)
		cache_queue->SetHandleFlags(
			flvr::CQCallback::HDL_DATA |
			flvr::CQCallback::HDL_LABEL |
			flvr::CQCallback::SAV_LABEL);

	glbin_trackmap_proc.TrackStencils(
		view->m_tseq_prv_num,
		view->m_tseq_cur_num,
		extt, exta,
		mode,
		view->m_begin_play_frame,
		sim);

	UpdateTraceDlg();
}

void ScriptProc::RunRandomColors()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	//VolumeSelector* selector = m_view->GetVolumeSelector();
	//if (!selector)
	//	return;

	int hmode;
	m_fconfig->Read("huemode", &hmode, 1);

	for (auto& it : vlist)
	{
		//generate RGB volumes
		glbin_vol_selector.SetVolume(it);
		std::shared_ptr<VolumeData> vd_r;
		std::shared_ptr<VolumeData> vd_g;
		std::shared_ptr<VolumeData> vd_b;
		glbin_vol_selector.CompExportRandomColor(hmode, vd_r, vd_g, vd_b, false, false);
	}
}

void ScriptProc::RunCompSelect()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	int mode;
	m_fconfig->Read("mode", &mode, 0);
	int comp_min;
	m_fconfig->Read("comp_min", &comp_min, 0);
	int comp_max;
	m_fconfig->Read("comp_max", &comp_max, 0);

	for (auto& it : vlist)
	{
		glbin_current.SetVolumeData(it);

		switch (mode)
		{
		case 0:
			glbin_comp_selector.All();
			break;
		case 1:
			glbin_comp_selector.Clear();
			break;
		case 2:
		default:
			if (comp_min)
			{
				glbin_comp_selector.SetUseMin(true);
				glbin_comp_selector.SetMinNum(comp_min);
			}
			if (comp_max)
			{
				glbin_comp_selector.SetUseMax(true);
				glbin_comp_selector.SetMaxNum(comp_max);
			}
			glbin_comp_selector.Select(true);
		}
	}
}

void ScriptProc::RunCompEdit()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	int edit_type;
	m_fconfig->Read("edit_type", &edit_type, 0);
	int mode;
	m_fconfig->Read("mode", &mode, 0);

	for (auto& it : vlist)
	{
		glbin_current.SetVolumeData(it);

		switch (edit_type)
		{
		case 0:
			glbin_comp_editor.Clean(mode);
			break;
		}
	}
}

void ScriptProc::RunFetchMask()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	auto view = m_view.lock();
	if (!view)
		return;

	int curf = view->m_tseq_cur_num;
	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, true);
	m_fconfig->Read("label", &blabel, true);

	for (auto& it : vlist)
	{
		auto reader = it->GetReader();
		if (!reader)
			return;
		//load and replace the mask
		if (bmask)
		{
			MSKReader msk_reader;
			std::wstring mskname = reader->GetCurMaskName(curf, it->GetCurChannel());
			msk_reader.SetFile(mskname);
			Nrrd* mask_nrrd_new = msk_reader.Convert(curf, it->GetCurChannel(), true);
			if (mask_nrrd_new)
				it->LoadMask(mask_nrrd_new);
			//else
			//	it->AddEmptyMask(0, true);
		}
		//load and replace the label
		if (blabel)
		{
			LBLReader lbl_reader;
			std::wstring lblname = reader->GetCurLabelName(curf, it->GetCurChannel());
			lbl_reader.SetFile(lblname);
			Nrrd* label_nrrd_new = lbl_reader.Convert(curf, it->GetCurChannel(), true);
			if (label_nrrd_new)
				it->LoadLabel(label_nrrd_new);
			//else
			//	it->AddEmptyLabel(0, true);
		}
	}
}

void ScriptProc::RunClearMask()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, true);
	m_fconfig->Read("label", &blabel, true);

	for (auto& it : vlist)
	{
		//clear the mask
		if (bmask)
			it->AddEmptyMask(0, true);
		//clear the label
		if (blabel)
			it->AddEmptyLabel(0, true);
		it->GetVR()->clear_tex_current();
	}
}

void ScriptProc::RunSaveMask()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	auto view = m_view.lock();
	if (!view)
		return;

	int curf = view->m_tseq_cur_num;
	bool bmask, blabel;
	m_fconfig->Read("mask", &bmask, true);
	m_fconfig->Read("label", &blabel, true);

	for (auto& it : vlist)
	{
		if (bmask)
			it->SaveMask(true, curf, it->GetCurChannel());
		if (blabel)
			it->SaveLabel(true, curf, it->GetCurChannel());
	}
}

void ScriptProc::RunSaveVolume()
{
	if (!TimeCondition())
		return;

	auto view = m_view.lock();
	if (!view)
		return;

	std::string source;
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
	std::wstring pathname;
	m_fconfig->Read("savepath", &pathname, std::wstring(L""));
	bool del_vol;
	m_fconfig->Read("delete", &del_vol, false);
	int smooth;
	m_fconfig->Read("smooth", &smooth, 0);

	fluo::Quaternion rot;
	fluo::Vector transl, euler;
	fluo::Point center;
	bool fix_size = false;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (source == "channels" ||
		source == "")
	{
		GetVolumes(vlist);
	}
	else if (source == "calculator")
	{
		while (auto vd = glbin_vol_calculator.GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "selector")
	{
		while (auto vd = glbin_vol_selector.GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "executor")
	{
		while (auto vd = glbin_kernel_executor.GetResult(true))
			vlist.push_back(vd);
	}
	else if (source == "registrator")
	{
		GetVolumes(vlist);
		if (GetRegistrationTransform(transl, center, euler, smooth))
		{
			rot.FromEuler(euler);
			crop = true;
			fix_size = true;
			neg_mask = true;
		}
	}
	int chan_num = vlist.size();
	int time_num = GetTimeNum();
	int curf = view->m_tseq_cur_num;
	std::wstring ext, name;
	if (mode == 0 || mode == 1)
		ext = L"tif";
	else if (mode == 2)
		ext = L"nrrd";
	name = GetSavePath(pathname, ext);
	name = REM_EXT(name);
	name = REM_NUM(name);
	if (name.empty())
		return;
	for (auto& it : vlist)
	{
		//time
		std::wstring format = std::to_wstring(time_num);
		int fr_length = format.length();
		std::wstring vstr = name + L"_T" + MAKE_NUMW(curf, fr_length);
		//channel
		if (chan_num > 1)
		{
			format = std::to_wstring(chan_num);
			int ch_length = format.length();
			vstr += L"_CH" + MAKE_NUMW(it->GetCurChannel() + 1, ch_length + 1);
		}
		//ext
		vstr += L"." + ext;
		it->Save(vstr, mode,
			mask, neg_mask,
			crop, filter,
			bake, compression,
			center, rot, transl,
			fix_size);
	}
}

void ScriptProc::RunSaveMesh()
{
	if (!TimeCondition())
		return;

	auto view = m_view.lock();
	if (!view)
		return;

	std::wstring pathname;
	m_fconfig->Read("savepath", &pathname, std::wstring(L""));

	int time_num = GetTimeNum();
	int curf = view->m_tseq_cur_num;
	std::wstring ext, name;
	ext = L"obj";
	name = GetSavePath(pathname, ext);
	name = REM_EXT(name);
	name = REM_NUM(name);
	if (name.empty())
		return;
	for (int i = 0; i < glbin_data_manager.GetMeshNum(); ++i)
	{
		auto md = glbin_data_manager.GetMeshData(i);
		if (!md)
			continue;
		std::wstring format = std::to_wstring(time_num);
		int fr_length = format.length();
		std::wstring str = name + L"_T" + MAKE_NUMW(curf, fr_length);
		str += L"." + ext;
		md->Save(str);
	}
}

void ScriptProc::RunCalculate()
{
	if (!TimeCondition())
		return;

	auto view = m_view.lock();
	if (!view)
		return;
	//VolumeCalculator* calculator = view->GetVolumeCalculator();
	//if (!calculator) return;
	int vol_a_index;
	m_fconfig->Read("vol_a", &vol_a_index, 0);
	int vol_b_index;
	m_fconfig->Read("vol_b", &vol_b_index, 0);
	std::string sOper;
	m_fconfig->Read("operator", &sOper, std::string(""));

	int vlist_size = view->GetDispVolumeNum();
	//get volumes
	std::shared_ptr<VolumeData> vol_a;
	if (vol_a_index >= 0 && vol_a_index < vlist_size)
		vol_a = view->GetDispVolumeData(vol_a_index);
	std::shared_ptr<VolumeData> vol_b;
	if (vol_b_index >= 0 && vol_b_index < vlist_size)
		vol_b = view->GetDispVolumeData(vol_b_index);
	if (!vol_a && !vol_b)
		return;

	//calculate
	glbin_vol_calculator.SetVolumeA(vol_a);
	glbin_vol_calculator.SetVolumeB(vol_b);
	if (sOper == "subtract")
		glbin_vol_calculator.CalculateGroup(1, L"", false);
	else if (sOper == "add")
		glbin_vol_calculator.CalculateGroup(2, L"", false);
	else if (sOper == "divide")
		glbin_vol_calculator.CalculateGroup(3, L"", false);
	else if (sOper == "colocate")
		glbin_vol_calculator.CalculateGroup(4, L"", false);
	else if (sOper == "fill")
		glbin_vol_calculator.CalculateGroup(9, L"", false);
}

void ScriptProc::RunOpenCL()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	std::wstring clname;
	m_fconfig->Read("clpath", &clname, std::wstring(L""));
	clname = GetInputFile(clname, L"CL_code");
	int repeat = 0;
	m_fconfig->Read("repeat", &repeat, 0);
	std::string code_save;
	if (!clname.empty())
	{
		code_save = glbin_kernel_executor.GetCode();
		glbin_kernel_executor.LoadCode(clname);
	}
	int repeat_save = -1;
	if (repeat)
	{
		repeat_save = glbin_kernel_executor.GetRepeat();
		glbin_kernel_executor.SetRepeat(repeat);
	}
	glbin_kernel_executor.SetDuplicate(true);

	for (auto& it : vlist)
	{
		it->GetVR()->clear_tex_current();
		glbin_kernel_executor.SetVolume(it);
		glbin_kernel_executor.Execute();
	}

	//restore saved settings
	if (!code_save.empty())
		glbin_kernel_executor.SetCode(code_save);
	if (repeat_save > -1)
		glbin_kernel_executor.SetRepeat(repeat_save);
}

void ScriptProc::RunCompAnalysis()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	auto view = m_view.lock();
	if (!view)
		return;

	int verbose;
	m_fconfig->Read("verbose", &verbose, 0);
	bool consistent;
	m_fconfig->Read("consistent", &consistent, true);
	bool selected;
	m_fconfig->Read("selected", &selected, false);
	int slimit;
	m_fconfig->Read("slimit", &slimit, 5);

	int curf = view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);
	fluo::Vector lens;

	glbin_comp_analyzer.SetSizeLimit(slimit);
	glbin_comp_analyzer.SetConsistent(consistent);
	bool old_selected = glbin_comp_analyzer.GetUseSel();
	glbin_comp_analyzer.SetUseSel(selected);
	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		int bn = (*itvol)->GetAllBrickNum();
		glbin_comp_analyzer.SetVolume(*itvol);
		glbin_comp_analyzer.Analyze();

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

		CelpList* celp_list = glbin_comp_analyzer.GetCelpList();
		CellGraph* graph = glbin_comp_analyzer.GetCellGraph();
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

	//restore
	glbin_comp_analyzer.SetUseSel(old_selected);
}

void ScriptProc::RunCompRuler()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list || ruler_list->empty())
		return;

	int dim;
	m_fconfig->Read("dim", &dim);//2 or 3
	std::wstring name;
	m_fconfig->Read("name", &name);
	double len;
	m_fconfig->Read("length", &len);//physical length
	flrd::Ruler* ruler = ruler_list->GetRuler(name);
	if (!ruler)
		return;
	if (ruler->GetNumPoint() < 2)
		return;

	ruler->SetWorkTime(0);
	fluo::Point o = ruler->GetPoint(0);
	if (dim < 3)
		o.z(0);
	fluo::Point p = ruler->GetPoint(1);
	if (dim < 3)
		p.z(0);
	fluo::Vector vx = p - o;
	double rl = vx.length();//length of ruler
	if (rl == 0)
		return;
	vx.normalize();
	fluo::Vector vy;
	fluo::Vector vz(vx.x(), 0, vx.z());
	if (vz.length() > 0)
	{
		vz = fluo::Cross(vx, vz);
		vz.normalize();
		vy = fluo::Cross(vz, vx);
	}
	else
	{
		vy = fluo::Vector(1, 0, 0);
		vz = fluo::Vector(0, 0, -1);
	}
	fluo::Transform tf(o, vx, vy, vz);
	tf.post_scale(fluo::Vector(len / rl));

	int curf = view->m_tseq_cur_num;
	std::string fn = std::to_string(curf);
	//output
	//script command
	fluo::Group* cmdg = m_output->getOrAddGroup(m_type);
	cmdg->addSetValue("type", m_type);//ruler_transform
	cmdg->addSetValue("dim", (long)dim);

	//convert comp to ruler
	//time group
	fluo::Group* timeg = m_output->getOrAddGroup(fn);
	//channel group
	int ch = 0;
	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		fluo::Group* chg = timeg->getOrAddGroup(std::to_string(ch));
		//script command
		fluo::Group* compg = chg->getOrAddGroup("comp_analysis");
		//comps
		for (int i = 0; i < compg->getNumChildren(); ++i)
		{
			fluo::Node* node = compg->getChild(i);
			std::string comp_name = node->getName();
			fluo::Point point;
			node->getValue("comp_center", point);
			//convert
			fluo::Group* ruler_group = cmdg->getOrAddGroup(std::to_string(i + 1));
			ruler_group->addSetValue("type", std::string("ruler"));
			ruler_group->addSetValue("name", comp_name);
			fluo::Node* point_node = ruler_group->getOrAddNode(std::to_string(0));
			if (dim < 3)
				point.z(0);
			tf.project_inplace(point);
			point_node->addSetValue(fn, point);
		}
	}
}

void ScriptProc::RunGenerateComp()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	std::wstring ml_table_file;
	m_fconfig->Read("ml_table", &ml_table_file);
	ml_table_file = GetInputFile(ml_table_file, L"Database");
	bool use_ml = !ml_table_file.empty();
	bool use_sel;
	m_fconfig->Read("use_sel", &use_sel, false);
	double tfac;
	m_fconfig->Read("th_factor", &tfac, 1.0);
	std::wstring cmdfile;
	m_fconfig->Read("comp_command", &cmdfile);
	cmdfile = GetInputFile(cmdfile, L"Commands");
	if (cmdfile.empty())
		glbin_comp_generator.ResetCmd();
	else
		glbin_comp_generator.LoadCmd(cmdfile);

	for (auto it = vlist.begin();
		it != vlist.end(); ++it)
	{
		glbin_comp_generator.SetVolumeData(*it);
		if (use_ml)
		{
			flrd::TableHistParams& table = glbin.get_cg_table();
			table.open(ml_table_file);
			glbin_comp_generator.ApplyRecord();
		}
		else
			glbin_comp_generator.PlayCmd(tfac);
	}
}

void ScriptProc::RunRulerProfile()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	int ival;
	double dval;
	m_fconfig->Read("fsize", &ival, 1);
	glbin_ruler_handler.SetFsize(ival);
	m_fconfig->Read("sample_type", &ival, 1);
	glbin_ruler_handler.SetSampleType(ival);
	m_fconfig->Read("step_len", &dval, 1.0);
	glbin_ruler_handler.SetStepLength(dval);

	int curf = view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		glbin_current.SetVolumeData(*itvol);
		glbin_ruler_handler.ProfileAll();

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
			flrd::Ruler* ruler = (*ruler_list)[i];
			if (!ruler) continue;
			if (!ruler->GetDisp()) continue;
			ruler->SetWorkTime(curf);
			fluo::Node* ruler_node = cmdg->getOrAddNode(std::to_string(ruler->Id() + 1));
			ruler_node->addSetValue("type", std::string("ruler"));

			std::vector<flrd::ProfileBin>* profile = ruler->GetProfile();
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
}

void ScriptProc::RunRoi()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;

	int curf = view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{
		glbin_current.SetVolumeData(*itvol);

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
			flrd::Ruler* ruler = (*ruler_list)[i];
			if (!ruler) continue;
			if (!ruler->GetDisp()) continue;
			ruler->SetWorkTime(curf);

			if (glbin_ruler_handler.Roi(ruler))
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

	std::string valname, bgname;
	m_fconfig->Read("value_name", &valname);
	m_fconfig->Read("bg_name", &bgname);
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

void ScriptProc::RunAddCells()
{
	if (!TimeCondition())
		return;
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	TrackGroup* tg = view->GetTrackGroup();
	if (!tg)
	{
		view->CreateTrackGroup();
		tg = view->GetTrackGroup();
	}

	flrd::pTrackMap track_map = tg->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	glbin_trackmap_proc.SetBits(cur_vol->GetBits());
	glbin_trackmap_proc.SetScale(cur_vol->GetScalarScale());
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	glbin_trackmap_proc.AddCells(*m_sel_labels,
		view->m_tseq_cur_num);
}

void ScriptProc::RunLinkCells()
{
	if (!TimeCondition())
		return;

	auto view = m_view.lock();
	if (!view)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	TrackGroup* trkg = view->GetTrackGroup();
	if (!trkg)
		return;

	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz);
	flrd::pTrackMap track_map = trkg->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	glbin_trackmap_proc.SetBits(vd->GetBits());
	glbin_trackmap_proc.SetScale(vd->GetScalarScale());
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	//register file reading and deleteing functions
	glbin_trackmap_proc.LinkAddedCells(*m_sel_labels, view->m_tseq_cur_num, view->m_tseq_cur_num - 1);
	glbin_trackmap_proc.LinkAddedCells(*m_sel_labels, view->m_tseq_cur_num, view->m_tseq_cur_num + 1);
	glbin_trackmap_proc.RefineMap(view->m_tseq_cur_num, false);
}

void ScriptProc::RunUnlinkCells()
{
	if (!TimeCondition())
		return;

	auto view = m_view.lock();
	if (!view)
		return;
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		return;
	TrackGroup* tg = view->GetTrackGroup();
	if (!tg)
		return;

	flrd::pTrackMap track_map = tg->GetTrackMap();
	glbin_trackmap_proc.SetTrackMap(track_map);
	glbin_trackmap_proc.SetBits(cur_vol->GetBits());
	glbin_trackmap_proc.SetScale(cur_vol->GetScalarScale());
	int resx, resy, resz;
	cur_vol->GetResolution(resx, resy, resz);
	glbin_trackmap_proc.SetSizes(resx, resy, resz);
	glbin_trackmap_proc.RemoveCells(*m_sel_labels,
		view->m_tseq_cur_num);
}

void ScriptProc::RunBackgroundStat()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	int curf = view->m_tseq_cur_num;
	int chan_num = vlist.size();
	int ch = 0;
	std::string fn = std::to_string(curf);

	for (auto itvol = vlist.begin();
		itvol != vlist.end(); ++itvol, ++ch)
	{

		flrd::BackgStat bgs(itvol->get());
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
		double varth = 0, gauth = 0;
		m_fconfig->Read("varth", &varth, 0.0);
		m_fconfig->Read("gauth", &gauth, 0.0);
		if (varth > 0.0 && gauth > 0.0)
			bgs.SetThreshold(static_cast<float>(varth), static_cast<float>(gauth));

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
}

void ScriptProc::RunRegistration()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	//always work on the selected volume
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol) return;

	int curf = view->m_tseq_cur_num;
	int prvf = view->m_tseq_prv_num;
	int bgnf = view->m_begin_play_frame;
	std::string curfstr = std::to_string(curf);
	std::string prvfstr = std::to_string(prvf);

	bool use_mask;
	m_fconfig->Read("use_mask", &use_mask, false);
	double exttx, extty, exttz;
	m_fconfig->Read("ext_x", &exttx, 0.1);
	m_fconfig->Read("ext_y", &extty, 0.1);
	m_fconfig->Read("ext_z", &exttz, 0.0);
	fluo::Vector extt(exttx, extty, exttz);
	m_fconfig->Read("ext_a", &exttx, 0.1);
	m_fconfig->Read("ext_b", &extty, 0.1);
	m_fconfig->Read("ext_c", &exttz, 0.0);
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
		view->SetObjCtrOff(fluo::Vector(0));
		view->SetObjRotOff(fluo::Vector(0));
		fluo::Transform tf;
		tf.load_identity();
		view->SetOffsetTransform(tf);
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
	registrator.SetVolumeData(cur_vol.get());
	flvr::CacheQueue* cache_queue = glbin_data_manager.GetCacheQueue(cur_vol.get());
	if (cache_queue)
	{
		if (use_mask)
			cache_queue->SetHandleFlags(
				flvr::CQCallback::HDL_DATA |
				flvr::CQCallback::ACS_MASK |
				flvr::CQCallback::RET_MASK |
				flvr::CQCallback::SAV_LABEL);
		else
			cache_queue->SetHandleFlags(
				flvr::CQCallback::HDL_DATA);
	}
	fluo::Vector transl, transl2;
	fluo::Point center, center2;
	fluo::Vector euler;
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
		rot.FromEuler(euler);
		tf = registrator.GetTransform();
		regg_cur->addSetValue("transl", transl);
		regg_cur->addSetValue("center", center);
		regg_cur->addSetValue("euler", euler);
		regg_cur->addSetValue("rot", rot);
		regg_cur->addSetValue("transform", tf);
		//apply transform to current view
		view->SetObjCtrOff(fluo::Vector(transl2));
		view->SetObjRotCtrOff(fluo::Vector(center2));
		view->SetObjRotOff(fluo::Vector(euler));
		view->SetOffsetTransform(tf);
	}
}

void ScriptProc::GetRulers(const std::wstring& vrp, int& startf, int& endf)
{
	std::shared_ptr<BaseTreeFile> ruler_file =
		glbin_tree_file_factory.createTreeFile(vrp, gstRulerFile);
	if (!ruler_file)
		return;

	if (ruler_file->LoadFile(vrp))
		return;

	//movie panel
	startf = 0;
	endf = 0;
	if (ruler_file->Exists("/movie_panel"))
	{
		ruler_file->SetPath("/movie_panel");
		ruler_file->Read("start_frame", &startf, 0);
		ruler_file->Read("end_frame", &endf, 0);
	}
	//views
	if (ruler_file->Exists("/views"))
	{
		ruler_file->SetPath("/views");
		long num = 0l;
		ruler_file->Read("num", &num, 0l);
		if (num < 1)
			return;

		if (ruler_file->Exists("/views/0/rulers"))
		{
			ruler_file->SetPath("/views/0/rulers");
			glbin_project.ReadRulerList(gstRulerFile, 0);
		}
	}
}

void ScriptProc::RunCameraPoints()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	std::wstring prj2;
	m_fconfig->Read("project_file", &prj2);
	prj2 = GetConfigFile(prj2, L"vrp", L"FluoRender Project", 0);
	std::wstring str;
	m_fconfig->Read("names", &str);
	std::vector<std::wstring> names;
	int correct = GetItems(str, names);
	double slope;
	m_fconfig->Read("slope", &slope, 0.0);
	bool affine;
	m_fconfig->Read("affine", &affine, false);
	bool persp;
	m_fconfig->Read("persp", &persp, true);
	bool metric;
	m_fconfig->Read("metric", &metric, true);

	RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list || ruler_list->empty()) return;
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		return;
	int nx, ny, nz;
	cur_vol->GetResolution(nx, ny, nz);

	Camera2Ruler c2r;
	c2r.SetImageSize(nx, ny);
	c2r.SetList(1, ruler_list);
	c2r.SetRange(1, view->m_begin_frame, view->m_end_frame);
	int startf, endf;
	GetRulers(prj2, startf, endf);
	c2r.SetList(2, startf, endf);
	c2r.SetNames(names);
	c2r.SetAffine(affine);
	c2r.SetPersp(persp);
	c2r.SetMetric(metric);
	c2r.Run();
	c2r.SetSlope(slope);
	c2r.Correct();

	RulerList* result_list = c2r.GetResult();
	if (result_list && !result_list->empty())
	{
		ruler_list->DeleteRulers();
		ruler_list->assign(result_list->begin(), result_list->end());
		delete result_list;
	}

	//turn off all channels
	for (int i = 0; i < view->GetDispVolumeNum(); ++i)
	{
		auto vd = view->GetDispVolumeData(i);
		if (!vd)
			continue;
		vd->SetDisp(false);
	}
	//turn off script
	if (m_frame)
	{
		glbin_settings.m_run_script = false;
		m_frame->GetMoviePanel()->FluoUpdate({ gstRunScript });
	}
}

void ScriptProc::RunRulerInfo()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list || ruler_list->empty())
		return;

	int curf = view->m_tseq_cur_num;
	std::string fn = std::to_string(curf);
	//output
	//script command
	fluo::Group* cmdg = m_output->getOrAddGroup(m_type);
	cmdg->addSetValue("type", m_type);

	for (size_t i = 0; i < ruler_list->size(); ++i)
	{
		//for each ruler
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		fluo::Group* ruler_group = cmdg->getOrAddGroup(std::to_string(ruler->Id() + 1));
		ruler_group->addSetValue("type", std::string("ruler"));
		ruler_group->addSetValue("name", ruler->GetName());
		ruler->SetWorkTime(curf);
		for (size_t j = 0; j < ruler->GetNumPoint(); ++j)
		{
			fluo::Node* point_node = ruler_group->getOrAddNode(std::to_string(j));
			fluo::Point point = ruler->GetPoint(j);
			point_node->addSetValue(fn, point);
		}
	}
}

void ScriptProc::RunRulerTransform()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	RulerList* ruler_list = view->GetRulerList();
	if (!ruler_list || ruler_list->empty())
		return;

	int dim;
	m_fconfig->Read("dim", &dim);//2 or 3
	std::wstring name;
	m_fconfig->Read("name", &name);
	double len;
	m_fconfig->Read("length", &len);//physical length
	flrd::Ruler* ruler = ruler_list->GetRuler(name);
	if (!ruler)
		return;
	if (ruler->GetNumPoint() < 2)
		return;

	//
	ruler->SetWorkTime(0);
	fluo::Point o = ruler->GetPoint(0);
	if (dim < 3)
		o.z(0);
	fluo::Point p = ruler->GetPoint(1);
	if (dim < 3)
		p.z(0);
	fluo::Vector vx = p - o;
	double rl = vx.length();//length of ruler
	if (rl == 0)
		return;
	vx.normalize();
	fluo::Vector vy;
	fluo::Vector vz(vx.x(), 0, vx.z());
	if (vz.length() > 0)
	{
		vz = fluo::Cross(vx, vz);
		vz.normalize();
		vy = fluo::Cross(vz, vx);
	}
	else
	{
		vy = fluo::Vector(1, 0, 0);
		vz = fluo::Vector(0, 0, -1);
	}
	fluo::Transform tf(o, vx, vy, vz);
	tf.post_scale(fluo::Vector(len / rl));

	int curf = view->m_tseq_cur_num;
	std::string fn = std::to_string(curf);
	//output
	//script command
	fluo::Group* cmdg = m_output->getOrAddGroup(m_type);
	cmdg->addSetValue("type", m_type);//ruler_transform
	cmdg->addSetValue("dim", (long)dim);

	for (size_t i = 0; i < ruler_list->size(); ++i)
	{
		//for each ruler
		flrd::Ruler* ruler = (*ruler_list)[i];
		if (!ruler) continue;
		if (!ruler->GetDisp()) continue;
		fluo::Group* ruler_group = cmdg->getOrAddGroup(std::to_string(ruler->Id() + 1));
		ruler_group->addSetValue("type", std::string("ruler"));
		ruler_group->addSetValue("name", ruler->GetName());
		ruler->SetWorkTime(curf);
		for (size_t j = 0; j < ruler->GetNumPoint(); ++j)
		{
			fluo::Node* point_node = ruler_group->getOrAddNode(std::to_string(j));
			fluo::Point point = ruler->GetPoint(j);
			if (dim < 3)
				point.z(0);
			tf.project_inplace(point);
			point_node->addSetValue(fn, point);
		}
	}
}

void ScriptProc::RunRulerSpeed()
{
	if (!TimeCondition())
		return;

	std::string type;
	m_fconfig->Read("type_name", &type);
	double time;//time between two samples
	m_fconfig->Read("time", &time, 1.0);
	double fps;//alternative
	if (m_fconfig->Read("fps", &fps, 1.0))
	{
		time = 1 / fps;
	}

	fluo::Node* node = m_output->getChild(type);
	if (!node)
		return;
	fluo::Group* cmdg_old = node->asGroup();
	if (!cmdg_old)
		return;
	long dim;
	cmdg_old->getValue("dim", dim);

	fluo::Point p0, p1;
	std::string stdstr;
	//output
	//script command
	fluo::Group* cmdg = m_output->getOrAddGroup(m_type);
	cmdg->addSetValue("type", m_type);//ruler_speed
	cmdg->addSetValue("dim", dim);

	for (size_t i = 0; i < cmdg_old->getNumChildren(); ++i)
	{
		node = cmdg_old->getChild(i);
		if (!node)
			continue;
		fluo::Group* ruler_group_old = node->asGroup();
		if (!ruler_group_old)
			continue;

		stdstr = ruler_group_old->getName();
		fluo::Group* ruler_group = cmdg->getOrAddGroup(stdstr);
		ruler_group_old->getValue("type", stdstr);
		ruler_group->addSetValue("type", stdstr);
		ruler_group_old->getValue("name", stdstr);
		ruler_group->addSetValue("name", stdstr);
		for (size_t j = 0; j < ruler_group_old->getNumChildren(); ++j)
		{
			node = ruler_group_old->getChild(j);
			if (!node)
				continue;

			fluo::Node* point_node = ruler_group->getOrAddNode(std::to_string(j));

			fluo::ValueVector names = node->getValueNames(3);
			for (size_t k = 0; k < names.size(); ++k)
			{
				if (k == 0)
				{
					node->getValue(names[k], p0);
					continue;
				}
				node->getValue(names[k], p1);
				//speed vector
				fluo::Vector speed = (p1 - p0) / time;
				point_node->addSetValue(names[k], fluo::Point(speed));//save as point
				p0 = p1;
			}
		}
	}
}

void ScriptProc::RunGenerateWalk()
{
	if (!TimeCondition())
		return;

	std::wstring filename;
	m_fconfig->Read("cycle", &filename);
	filename = GetConfigFile(filename, L"csv", L"Cycle", 0);
	int length;
	m_fconfig->Read("length", &length);
	double dir;
	m_fconfig->Read("dir", &dir);

	flrd::WalkCycle cycle;
	cycle.ReadData(filename);
	cycle.Correct(0);
	glbin_ruler_handler.GenerateWalk(length, dir, cycle);
}

void ScriptProc::RunConvertMesh()
{
	if (!TimeCondition())
		return;
	std::vector<std::shared_ptr<VolumeData>> vlist;
	if (!GetVolumes(vlist))
		return;

	int mode;
	m_fconfig->Read("mode", &mode, 0);

	for (auto& it : vlist)
	{
		if (!it)
			continue;
		glbin_conv_vol_mesh->SetVolumeData(it);
		glbin_conv_vol_mesh->Update(true);
		glbin_conv_vol_mesh->MergeVertices(true);
		auto md = glbin_conv_vol_mesh->GetMeshData();
		if (it->GetLabel(false))
		{
			glbin_color_mesh.SetUseSel(true);
			glbin_color_mesh.SetUseComp(true);
		}
		else if (it->GetMask(false))
		{
			glbin_color_mesh.SetUseSel(true);
			glbin_color_mesh.SetUseComp(false);
		}
		else
		{
			glbin_color_mesh.SetUseSel(false);
			glbin_color_mesh.SetUseComp(false);
		}
		glbin_color_mesh.SetVolumeData(it);
		glbin_color_mesh.SetMeshData(md);
		glbin_color_mesh.Update();
		if (glbin_data_manager.AddMeshData(md))
		{
			auto view = m_view.lock();
			if (view)
				view->AddMeshData(md);
		}
	}
}

void ScriptProc::RunPython()
{
	if (!TimeCondition())
		return;

	std::string cmd;
	m_fconfig->Read("command", &cmd);
	if (cmd.empty())
		return;
	flrd::PyBase* python = glbin.get_add_pybase("python");
	if (!python)
		return;
	python->Init();
	python->Run(flrd::PyBase::ot_Run_SimpleString, cmd);
	python->Exit();
}

void ScriptProc::RunDlcVideoAnalyze()
{
	if (!TimeCondition())
		return;
	//always work on the selected volume
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol) return;

	flrd::PyDlc* dlc = glbin.get_add_pydlc("dlc");
	if (!dlc)
		return;
	if (dlc->GetState() == 2)
		return;//busy, already created

	std::wstring filename;
	m_fconfig->Read("config", &filename);
	filename = GetConfigFile(filename, L"yaml", L"Config", 0);
	std::wstring fn = cur_vol->GetPath();
	std::wstring cfg = filename;
	if (fn.empty() || cfg.empty())
		return;

	fluo::Group* dlcg = m_output->getOrAddGroup("dlc");
	dlcg->addSetValue(ws2s(fn), false);

	//run dlc
	dlc->Init();
	dlc->LoadDlc();
	dlc->SetConfigFile(cfg);
	dlc->SetVideoFile(fn);
	dlc->AnalyzeVideo();
}

void ScriptProc::RunDlcGetRulers()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	//always work on the selected volume
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol)
		return;

	int toff = 0;
	m_fconfig->Read("time_offset", &toff, 0);
	fluo::Group* dlcg = m_output->getOrAddGroup("dlc");
	bool analyzed = false;
	std::wstring fn = cur_vol->GetPath();
	dlcg->getValue(ws2s(fn), analyzed);
	if (analyzed)
		return;

	flrd::PyDlc* dlc = glbin.get_add_pydlc("dlc");
	if (!dlc)
		return;
	if (!dlc->GetResultFile())
	{
		//busy
		if (view->m_tseq_cur_num == view->m_end_frame)
			glbin_moviemaker.Reset();//rewind and restart video
		return;
	}

	//std::filesystem::path p(fn);
	//if (p.extension().string() != ".m4v")//dlc may have problem decoding m4v files
	//	toff = 0;

	int errs = dlc->GetDecodeErrorCount();
	dlc->AddRulers(&glbin_ruler_handler, toff + errs);
	dlc->Exit();
	dlcg->addSetValue(ws2s(fn), true);
}

void ScriptProc::RunDlcCreateProj()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;
	//always work on the selected volume
	auto cur_vol = glbin_current.vol_data.lock();
	if (!cur_vol) return;

	flrd::PyDlc* dlc = glbin.get_add_pydlc("dlc");
	if (!dlc)
		return;

	//resoultion
	int nx, ny, nz;
	cur_vol->GetResolution(nx, ny, nz);
	dlc->SetFrameSize(nx, ny);
	//range
	dlc->SetFrameNumber(view->m_end_all_frame);
	dlc->SetFrameRange(view->m_begin_frame, view->m_end_frame);

	std::wstring filename;
	m_fconfig->Read("config", &filename);
	filename = GetConfigFile(filename, L"yaml", L"Config", 1);
	dlc->SetConfigFile(filename);
	std::wstring stdstr = cur_vol->GetPath();
	dlc->SetVideoFile(stdstr);
	std::filesystem::path p(stdstr);
	stdstr = p.stem().wstring();
	dlc->CreateConfigFile(stdstr, L"FluoRender", &glbin_ruler_handler);
}

void ScriptProc::RunDlcLabel()
{
	if (!TimeCondition())
		return;
	flrd::PyDlc* dlc = glbin.get_add_pydlc("dlc");
	if (!dlc)
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	int curf = view->m_tseq_cur_num;
	int endf = view->m_end_frame;
	int fn = view->m_total_frames;
	int temp = fn;
	int fn_len = 0;
	while (temp)
	{
		temp /= 10;
		fn_len++;
	}
	//write frame
	std::set<size_t> keys;
	glbin_ruler_handler.GetKeyFrames(keys);
	if (keys.find(size_t(curf)) != keys.end())
	{
		int vn = std::min(view->GetAllVolumeNum(), 3);
		std::vector<std::shared_ptr<VolumeData>> vd_rgb(3);
		int nx, ny, nz;
		for (int i = 0; i < vn; ++i)
			vd_rgb[i] = view->GetAllVolumeData(i);

		//get data
		vd_rgb[0]->GetResolution(nx, ny, nz);
		char* image = new char[nx * ny * 3]();
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
		std::wstring filename = dlc->GetLabelPath();
		std::wostringstream oss;
		oss << std::setw(fn_len) << std::setfill(L'0') << curf;
		std::filesystem::path p(filename);
		p /= L"img" + oss.str() + L".tif";
		filename = p.wstring();

		//write tiff
		auto img_capture = CreateImageCapture(filename);
		if (img_capture)
		{
			img_capture->SetFilename(filename);
			img_capture->SetData(image, nx, ny, 3);
			img_capture->SetFlipVertically(false);
			img_capture->SetIsFloat(false);
			img_capture->SetUseCompression(false);
			img_capture->Write();
		}

		if (image)
			delete[]image;
	}

	if (curf == endf)
	{
		//write hdf
		dlc->SetFrameNumber(fn);
		dlc->WriteHDF(&glbin_ruler_handler);
		int maxiters = 100;
		m_fconfig->Read("maxiters", &maxiters, 100);

#ifdef _DARWIN
		dlc->Init();
		dlc->LoadDlc();
		dlc->Train(maxiters);
#else
		std::string cmd = dlc->GetTrainCmd(maxiters);
		glbin_moviemaker.Hold();
		std::thread([&cmd]{
		std::system(cmd.c_str());
			}).detach(); // Detach the thread to run independently
		glbin_moviemaker.Resume();
#endif
	}
}

void ScriptProc::ExportInfo()
{
	if (!TimeCondition())
		return;

	std::wstring outputfile;
	m_fconfig->Read("output", &outputfile);
	outputfile = GetSavePath(outputfile, L"csv", false);
	if (outputfile.empty())
		return;
	int tnum;
	m_fconfig->Read("type_num", &tnum, 0);
	std::set<std::string> tnames;
	std::string str;
	for (int i = 0; i < tnum; ++i)
	{
		if (m_fconfig->Read(
			"type_name" + std::to_string(i),
			&str))
			tnames.insert(str);
	}

	//print lines
#ifdef _WIN32
	std::ofstream ofs(outputfile);
#else
    std::ofstream ofs(ws2s(outputfile));
#endif
	OutCoordVisitor visitor(ofs, tnames);
	m_output->accept(visitor);
	ofs.close();

	glbin_moviemaker.Hold();
	OpenFileWithDefaultProgram(outputfile);
	glbin_moviemaker.Resume();
}

void ScriptProc::ExportTemplate()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	//template
	std::wstring tempfile;
	m_fconfig->Read("template", &tempfile);
	tempfile = GetInputFile(tempfile, L"Templates");
	if (tempfile.empty())
		return;
	std::wstring outputfile;
	m_fconfig->Read("output", &outputfile);
	outputfile = GetSavePath(outputfile, L"html", false);
	if (outputfile.empty())
		return;
	int vnum;
	m_fconfig->Read("value_num", &vnum, 0);
	std::set<std::string> vnames;
	std::string str;
	for (int i = 0; i < vnum; ++i)
	{
		if (m_fconfig->Read(
			"value_name" + std::to_string(i),
			&str))
			vnames.insert(str);
	}
	std::string js_value;
	m_fconfig->Read("js_value", &js_value);

	//print lines
#ifdef _WIN32
	std::ifstream ifs(tempfile);
	std::ofstream ofs(outputfile);
#else
    std::ifstream ifs(ws2s(tempfile));
    std::ofstream ofs(ws2s(outputfile));
#endif
	std::string line;
	int replace = 0;//1:data;2:value name;
	while (std::getline(ifs, line))
	{
		//if d3 is local, set path to working dir
		if (line.find("<script src=\"./d3") != std::string::npos)
		{
			//source
			ofs << "    <script src=\"";
			std::filesystem::path p = std::filesystem::current_path();
			ofs << p.string();
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
				view->GetAllVolumeNum());
			m_output->accept(visitor);
			ofs << "\";" << std::endl;
		}
		if (line.find("#begin value name") != std::string::npos)
		{
			//value name
			replace = 2;
			ofs << "//#begin value name" << std::endl;
			ofs << "        value: " << js_value;
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

#ifdef __APPLE__

    // Replace spaces with %20
    std::wstring encoded;
    for (wchar_t ch : outputfile) {
        if (ch == L' ') {
            encoded += L"%20";
        } else {
            encoded += ch;
        }
    }

    // Prepend file://
    outputfile = L"file://" + encoded;
#endif
	glbin_moviemaker.Hold();
	OpenFileWithDefaultProgram(outputfile);
	glbin_moviemaker.Resume();
}

void ScriptProc::ExportSpreadsheet()
{
	if (!TimeCondition())
		return;

	std::wstring outputfile;
	m_fconfig->Read("output", &outputfile);
	outputfile = GetSavePath(outputfile, L"csv", false);
	if (outputfile.empty())
		return;
	int vnum;
	m_fconfig->Read("value_num", &vnum, 0);
	std::set<std::string> vnames;
	std::string str;
	for (int i = 0; i < vnum; ++i)
	{
		if (m_fconfig->Read(
			"value_name" + std::to_string(i),
			&str))
			vnames.insert(str);
	}

	//print lines
#ifdef _WIN32
	std::ofstream ofs(outputfile);
#else
    std::ofstream ofs(ws2s(outputfile));
#endif
	OutCsvVisitor visitor(ofs, vnames);
	m_output->accept(visitor);
	ofs.close();

	glbin_moviemaker.Hold();
	OpenFileWithDefaultProgram(outputfile);
	glbin_moviemaker.Resume();
}

void ScriptProc::ChangeData()
{
	if (!TimeCondition())
		return;
	auto view = m_view.lock();
	if (!view)
		return;

	bool clear;
	m_fconfig->Read("clear", &clear, true);
	std::wstring filename;
	m_fconfig->Read("input", &filename, std::wstring(L""));
	filename = GetInputFile(filename, L"Data");
	bool imagej;
	m_fconfig->Read("imagej", &imagej, false);

	if (clear)
	{
		glbin_ruler_handler.DeleteAll(false);
		glbin_data_manager.ClearAll();
		Root* root = glbin_data_manager.GetRoot();
		if (root)
			root->GetView(0)->ClearAll();
		VolumeGroup::ResetID();
		MeshGroup::ResetID();
	}
	if (!filename.empty())
	{
		std::vector<std::wstring> files;
		files.push_back(filename);
		glbin_data_manager.LoadVolumes(files, imagej);
		view->m_cur_vol_save = view->GetAllVolumeData(0);
	}
}

void ScriptProc::ChangeScript()
{
	if (!TimeCondition())
		return;

	bool run_script;
	m_fconfig->Read("run_script", &run_script, false);
	std::wstring filename;
	m_fconfig->Read("script_file", &filename, std::wstring(L""));
	filename = GetInputFile(filename, L"Scripts");

	if (!run_script)
		glbin_settings.m_run_script = run_script;
	if (!filename.empty())
		glbin_settings.m_script_file = filename;
	if (m_frame)
		m_frame->GetMoviePanel()->FluoUpdate({ gstRunScript, gstScriptList, gstScriptFile });
}

void ScriptProc::LoadProject()
{
	if (!TimeCondition())
		return;

	std::wstring filename;
	m_fconfig->Read("project_file", &filename, std::wstring(L""));
	filename = GetInputFile(filename, L"Data");

	glbin_project.Open(filename);
}

void ScriptProc::DisableScript()
{
	if (!TimeCondition())
		return;

	glbin_settings.m_run_script = false;
	if (m_frame)
		m_frame->GetMoviePanel()->FluoUpdate({ gstRunScript });
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

	m_fconfig->Read("info", &m_info, std::string(""));
	//info.Replace("\n", "\n");
	bool reset = false;
	m_fconfig->Read("reset", &reset, false);

	ScriptBreakDlg* dlg = m_frame->GetScriptBreakDlg();
	if (!dlg)
		return false;
	dlg->Hold();
	return reset;
}

void ScriptProc::OpenFileWithDefaultProgram(const std::wstring& filename)
{
#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific code
    ShellExecuteW(NULL, L"open", filename.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif defined(__APPLE__) || defined(__MACH__)
    // macOS-specific code
    std::string command = "open " + ws2s(filename);
    std::system(command.c_str());
#elif defined(__linux__)
    // Linux-specific code
    std::string command = "xdg-open " + ws2s(filename);
    std::system(command.c_str());
#else
    #error "Unsupported operating system"
#endif
}
