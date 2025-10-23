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
#include <DataManager.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <Root.h>
#include <RenderView.h>
#include <VolumeGroup.h>
#include <VolumeData.h>
#include <MeshGroup.h>
#include <MeshData.h>
#include <AnnotData.h>
#include <VolCache4D.h>
#include <VolumeRenderer.h>
#include <RenderScheduler.h>
#include <base_vol_reader.h>
#include <imageJ_reader.h>
#include <tif_reader.h>
#include <png_reader.h>
#include <jpg_reader.h>
#include <jp2_reader.h>
#include <nrrd_reader.h>
#include <oib_reader.h>
#include <oif_reader.h>
#include <lsm_reader.h>
#include <pvxml_reader.h>
#include <brkxml_reader.h>
#include <czi_reader.h>
#include <nd2_reader.h>
#include <lif_reader.h>
#include <lof_reader.h>
#include <mpg_reader.h>
#include <dcm_reader.h>
#include <msk_reader.h>
#include <lbl_reader.h>
#include <base_mesh_reader.h>
#include <obj_reader.h>
#include <CurrentObjects.h>
#include <MovieMaker.h>
#include <Project.h>
#include <Value.hpp>
#include <FpRangeDlg.h>
#include <filesystem>

DataManager::DataManager() :
	Progress(),
	m_frame(0),
	m_cur_file(0),
	m_file_num(0)
{
	m_root = std::make_unique<Root>();
}

DataManager::~DataManager()
{
}

void DataManager::SetFrame(MainFrame* frame)
{
	m_frame = frame;
}

void DataManager::ClearAll()
{
	m_vd_list.clear();
	m_md_list.clear();
	m_ad_list.clear();
	m_vol_reader_list.clear();
	m_mesh_reader_list.clear();
	m_vd_cache_queue.clear();
}

void DataManager::SetVolumeDefault(const std::shared_ptr<VolumeData>& vd)
{
	bool use_ml = glbin_settings.m_vp_auto_apply;
	if (use_ml)
	{
		vd->ApplyMlVolProp();
		//props not managed by ml
		vd->SetSampleRate(glbin_vol_def.m_sample_rate);
		if (!vd->GetSpcFromFile())
			vd->SetBaseSpacings(
				glbin_vol_def.m_spcx,
				glbin_vol_def.m_spcy,
				glbin_vol_def.m_spcz);
		vd->SetLabelMode(glbin_vol_def.m_label_mode);
	}
	else
	{
		glbin_vol_def.Apply(vd.get());
	}
	//disable alpha for z = 1
	int nx, ny, nz;
	vd->GetResolution(nx, ny, nz);
	if (nz == 1)
		vd->SetAlphaEnable(false);
}

//set project path
//when data and project are moved, use project file's path
//if data's directory doesn't exist
void DataManager::SetProjectPath(const std::wstring& path)
{
	m_prj_file = path;
	m_prj_path.clear();
	std::filesystem::path p(path);
	m_prj_path = p.parent_path().wstring();
}

std::wstring DataManager::SearchProjectPath(const std::wstring& filename)
{
	if (m_prj_path.empty())
		return L"";

	std::filesystem::path original_path(filename);
	std::filesystem::path search_str;

	// Normalize the path to handle mixed separators
	original_path = original_path.lexically_normal();

	// Try partial path reconstruction from the end
	for (auto it = original_path.end(); it != original_path.begin();)
	{
		--it;
		search_str = std::filesystem::path(*it) / search_str;
		std::filesystem::path candidate = std::filesystem::path(m_prj_path) / search_str;

		if (std::filesystem::exists(candidate))
			return candidate.wstring();
	}

	// If not found, do a recursive search for the filename only
	std::wstring target_filename = original_path.filename().wstring();
	for (const auto& entry : std::filesystem::recursive_directory_iterator(m_prj_path))
	{
		if (entry.is_regular_file() && entry.path().filename() == target_filename)
		{
			return entry.path().wstring();
		}
	}

	return L"";
}

std::wstring DataManager::GetProjectFile()
{
	return m_prj_file;
}

void DataManager::LoadVolumes(const std::vector<std::wstring>& files, bool withImageJ)
{
	fluo::ValueCollection vc;
	std::weak_ptr<VolumeData> vd_sel;
	std::weak_ptr<VolumeGroup> group_sel;
	auto view = glbin_current.render_view.lock();
	Root* root = glbin_data_manager.GetRoot();

	if (!root)
		return;
	if (!view)
		view = root->GetView(0);
	if (!view)
		return;

	double gpu_size = glbin_settings.m_graphics_mem;
	double data_size = glbin_settings.m_large_data_size;
	int brick_size = glbin_settings.m_force_brick_size;
	int resp_time = glbin_settings.m_up_time;

	bool enable_4d = false;
	bool enable_rot_lock = false;
	m_file_num = files.size();
	int all_ch_num = 0;
	bool large_data = false;

	for (m_cur_file = 0; m_cur_file < m_file_num; ++m_cur_file)
	{
		bool streaming = glbin_settings.m_mem_swap;
		std::string str_streaming;
		if (streaming)
		{
			str_streaming = "Large data streaming is currently ON. ";
			str_streaming += "FluoRender uses up to " + std::to_string(int(std::round(gpu_size))) + "MB GPU memory. ";
			str_streaming += "Data >" + std::to_string(int(data_size)) + "MB are divided into " + std::to_string(brick_size) + "voxel bricks. ";
			str_streaming += "System response time: " + std::to_string(resp_time) + "ms.";
		}
		else
			str_streaming = "Large data streaming is currently OFF.";
		SetProgress(std::round(100.0 * (m_cur_file + 1) / m_file_num), str_streaming);

		int ch_num = 0;
		std::wstring filename = files[m_cur_file];
		std::wstring suffix = GET_SUFFIX(filename);

		if (withImageJ)
			ch_num = LoadVolumeData(filename, LOAD_TYPE_IMAGEJ, true); //The type of data doesnt matter.
		else if (suffix == L".nrrd" || suffix == L".msk" || suffix == L".lbl")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_NRRD, false);
		else if (suffix == L".tif" || suffix == L".tiff")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_TIFF, false);
		else if (suffix == L".png")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_PNG, false);
		else if (suffix == L".jpg" || suffix == L".jpeg")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_JPG, false);
		else if (suffix == L".jp2")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_JP2, false);
		else if (suffix == L".oib")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_OIB, false);
		else if (suffix == L".oif")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_OIF, false);
		else if (suffix == L".lsm")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_LSM, false);
		else if (suffix == L".xml")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_PVXML, false);
		else if (suffix == L".vvd")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_BRKXML, false);
		else if (suffix == L".czi")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_CZI, false);
		else if (suffix == L".nd2")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_ND2, false);
		else if (suffix == L".lif")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_LIF, false);
		else if (suffix == L".lof")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_LOF, false);
		else if (suffix == L".mp4" || suffix == L".m4v" || suffix == L".mov" || suffix == L".avi" || suffix == L".wmv")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_MPG, false);
		else if (suffix == L".dcm" || suffix == L".dicom")
			ch_num = LoadVolumeData(filename, LOAD_TYPE_DCM, false);

		all_ch_num += ch_num;
		if (ch_num > 1)
		{
			auto group = view->AddOrGetGroup();
			if (group)
			{
				int nz_count = 0;
				for (int i = ch_num; i > 0; i--)
				{
					auto vd = GetVolumeData(GetVolumeNum() - i);
					if (vd)
					{
						view->AddVolumeData(vd, group->GetName());
						std::wstring vol_name = vd->GetName();
						if (vol_name.find(L"_1ch") != std::wstring::npos &&
							(i == 1 || i == 2))
							vd->SetDisp(false);
						if (vol_name.find(L"_2ch") != std::wstring::npos &&
							i == 1)
							vd->SetDisp(false);

						if (i == ch_num)
						{
							vd_sel = vd;
							group_sel = group;
						}

						if (vd->GetReader() && vd->GetReader()->GetTimeNum() > 1)
							enable_4d = true;

						int nx, ny, nz;
						vd->GetResolution(nx, ny, nz);
						if (nz == 1)
							nz_count++;

						//check if brick number is larger than 1
						double data_size = double(nx)*double(ny)*double(nz)/1.04e6;
						large_data = large_data || data_size > glbin_settings.m_large_data_size;
					}
				}
				if (m_cur_file > 0)
					group->SetDisp(false);
				if (nz_count == ch_num)
					enable_rot_lock = true;
			}
		}
		else if (ch_num == 1)
		{
			auto vd = GetVolumeData(GetVolumeNum() - 1);
			if (vd)
			{
				if (!vd->GetWlColor())
				{
					int chan_num = view->GetDispVolumeNum();
					fluo::Color color(1.0, 1.0, 1.0);
					if (chan_num == 0)
						color = fluo::Color(1.0, 0.0, 0.0);
					else if (chan_num == 1)
						color = fluo::Color(0.0, 1.0, 0.0);
					else if (chan_num == 2)
						color = fluo::Color(0.0, 0.0, 1.0);

					if (chan_num >= 0 && chan_num < 3)
						vd->SetColor(color);
					else
						vd->RandomizeColor();
				}

				view->AddVolumeData(vd);
				vd_sel = vd;

				if (vd->GetReader() && vd->GetReader()->GetTimeNum() > 1)
				{
					view->m_tseq_cur_num = vd->GetReader()->GetCurTime();
					enable_4d = true;
				}

				int nx, ny, nz;
				vd->GetResolution(nx, ny, nz);
				if (nz == 1)
					enable_rot_lock = true;

				//check if brick number is larger than 1
				double data_size = double(nx)*double(ny)*double(nz)/1.04e6;
				large_data = large_data || data_size > glbin_settings.m_large_data_size;
			}
		}

		view->InitView(INIT_BOUNDS | INIT_CENTER);
		glbin_render_scheduler_manager.requestDraw(
			{ static_cast<int>(view->Id()) }, "Load volume refersh");
	}

	vc.insert(gstListCtrl);
	vc.insert(gstTreeCtrl);
	vc.insert(gstUpdateSync);
	if (auto vd_sel_ptr = vd_sel.lock())
		glbin_current.SetVolumeData(vd_sel_ptr);
	vc.insert(gstScaleFactor);

	if (enable_4d)
	{
		glbin_moviemaker.SetCurrentFrameSilently(view->m_tseq_cur_num, true);
		glbin_moviemaker.SetSeqMode(1);
		vc.insert(gstMovieAgent);
	}
	view->SetRotLock(enable_rot_lock);
	vc.insert(gstGearedEnable);
	if (enable_rot_lock && all_ch_num > 1)
	{
		glbin_settings.m_micro_blend = true;
		vc.insert(gstMicroBlendEnable);
	}
	else if (large_data)
	{
		glbin_settings.m_micro_blend = false;
		vc.insert(gstMicroBlendEnable);
	}
	vc.insert(gstVolumePropPanel);
	//update histogram
	vc.insert(gstUpdateHistogram);
	vc.insert(gstCurrentSelect);

	m_frame->UpdateProps(vc);

	SetProgress(0, "");

	m_file_num = 0;
}

void DataManager::StartupLoad(const std::vector<std::wstring>& files, bool run_mov, bool with_imagej)
{
	auto view = glbin_current.render_view.lock();
	if (view)
		view->Init();

	if (!files.empty())
	{
		std::wstring filename = files[0];
		std::wstring suffix = GET_SUFFIX(filename);

		if (suffix == L".vrp")
		{
			glbin_project.Open(files[0]);
			m_frame->FluoUpdate({ gstMainFrameTitle });
		}
		else if (suffix == L".nrrd" ||
			suffix == L".msk" ||
			suffix == L".lbl" ||
			suffix == L".tif" ||
			suffix == L".tiff" ||
			suffix == L".oib" ||
			suffix == L".oif" ||
			suffix == L".lsm" ||
			suffix == L".xml" ||
			suffix == L".vvd" ||
			suffix == L".nd2" ||
			suffix == L".czi" ||
			suffix == L".lif" ||
			suffix == L".lof" ||
			suffix == L".mp4" ||
			suffix == L".m4v" ||
			suffix == L".mov" ||
			suffix == L".avi" ||
			suffix == L".wmv" ||
			suffix == L".dcm" ||
			suffix == L".dicom")
		{
			LoadVolumes(files, with_imagej);
		}
		else if (suffix == L".obj")
		{
			LoadMeshes(files);
		}
		else if (with_imagej)
		{
			LoadVolumes(files, with_imagej);
		}
	}

	if (run_mov)
	{
		glbin_moviemaker.SetFileName(glbin_settings.m_mov_filename);
		glbin_moviemaker.PlaySave();
	}
}

size_t DataManager::LoadVolumeData(const std::wstring &filename, int type, bool withImageJ, int ch_num, int t_num)
{
	bool isURL = false;
	bool downloaded = false;
	std::wstring downloaded_filepath;
	bool downloaded_metadata = false;
	std::wstring downloaded_metadatafilepath;

	std::wstring pathname = filename;
	if (!std::filesystem::exists(pathname))
	{
		pathname = SearchProjectPath(pathname);
		if (!std::filesystem::exists(pathname))
			return 0;
	}

	size_t result = 0;
	std::shared_ptr<BaseVolReader> reader;

	for (size_t i=0; i<m_vol_reader_list.size(); i++)
	{
		std::wstring wstr = pathname;
		if (m_vol_reader_list[i]->Match(wstr))
		{
			reader = m_vol_reader_list[i];
			break;
		}
	}

	int reader_return = -1;
	if (reader)
	{
		reader->SetProgressFunc(GetProgressFunc());
		bool preprocess = false;
		if (reader->GetSliceSeq() != glbin_settings.m_slice_sequence)
		{
			reader->SetSliceSeq(glbin_settings.m_slice_sequence);
			preprocess = true;
		}
		if (reader->GetChannSeq() != glbin_settings.m_chann_sequence)
		{
			reader->SetChannSeq(glbin_settings.m_chann_sequence);
			preprocess = true;
		}
		if (reader->GetDigitOrder() != glbin_settings.m_digit_order)
		{
			reader->SetDigitOrder(glbin_settings.m_digit_order);
			preprocess = true;
		}
		std::wstring str_w = glbin_settings.m_time_id;
		if (reader->GetTimeId() != str_w)
		{
			reader->SetTimeId(str_w);
			preprocess = true;
		}
		if (preprocess)
			reader_return = reader->Preprocess();
	}
	else
	{
		//RGB tiff
		//TODO: Loading with imageJ irrespective of the file type.
		if (withImageJ == true)
			reader = std::make_shared<ImageJReader>();
		else {
			if (type == LOAD_TYPE_TIFF)
				reader = std::make_shared<TIFReader>();
			else if (type == LOAD_TYPE_PNG)
				reader = std::make_shared<PNGReader>();
			else if (type == LOAD_TYPE_JPG)
				reader = std::make_shared<JPGReader>();
			else if (type == LOAD_TYPE_JP2)
				reader = std::make_shared<JP2Reader>();
			else if (type == LOAD_TYPE_NRRD)
				reader = std::make_shared<NRRDReader>();
			else if (type == LOAD_TYPE_OIB)
				reader = std::make_shared<OIBReader>();
			else if (type == LOAD_TYPE_OIF)
				reader = std::make_shared<OIFReader>();
			else if (type == LOAD_TYPE_LSM)
				reader = std::make_shared<LSMReader>();
			else if (type == LOAD_TYPE_PVXML)
			{
				auto pvxml_reader = std::make_shared<PVXMLReader>();
				pvxml_reader->SetFlipX(glbin_settings.m_pvxml_flip_x);
				pvxml_reader->SetFlipY(glbin_settings.m_pvxml_flip_y);
				pvxml_reader->SetSeqType(glbin_settings.m_pvxml_seq_type);
				reader = pvxml_reader;
			}
			else if (type == LOAD_TYPE_BRKXML)
				reader = std::make_shared<BRKXMLReader>();
			else if (type == LOAD_TYPE_CZI)
				reader = std::make_shared<CZIReader>();
			else if (type == LOAD_TYPE_ND2)
				reader = std::make_shared<ND2Reader>();
			else if (type == LOAD_TYPE_LIF)
				reader = std::make_shared<LIFReader>();
			else if (type == LOAD_TYPE_LOF)
				reader = std::make_shared<LOFReader>();
			else if (type == LOAD_TYPE_MPG)
				reader = std::make_shared<MPGReader>();
			else if (type == LOAD_TYPE_DCM)
				reader = std::make_shared<DCMReader>();
			else
				return result;
		}
		
		reader->SetProgressFunc(GetProgressFunc());

		m_vol_reader_list.push_back(reader);
		reader->SetFile(pathname);
		reader->SetSliceSeq(glbin_settings.m_slice_sequence);
		reader->SetChannSeq(glbin_settings.m_chann_sequence);
		reader->SetDigitOrder(glbin_settings.m_digit_order);
		reader->SetTimeId(glbin_settings.m_time_id);
		reader_return = reader->Preprocess();
	}

	if (type == LOAD_TYPE_TIFF)
	{
		if (!glbin_settings.m_fp_convert &&
			reader->GetFpConvert())
		{
			double minv, maxv;
			reader->GetFpRange(minv, maxv);
			glbin_settings.m_fp_min = minv;
			glbin_settings.m_fp_max = maxv;
			FpRangeDlg* dlg = m_frame->GetFpRangeDlg();
			dlg->CenterOnParent();
			int rval = dlg->ShowModal();
		}
		reader->SetFpRange(glbin_settings.m_fp_min, glbin_settings.m_fp_max);
	}

	if (reader_return > 0)
	{
		std::string err_str = BaseVolReader::GetError(reader_return);
		SetProgress(0, err_str);
		int i = (int)m_vol_reader_list.size() - 1;
		if (m_vol_reader_list[i]) {
			m_vol_reader_list.erase(m_vol_reader_list.begin() + (int)m_vol_reader_list.size() - 1);
		}
		return result;
	}

	//align data for compression if vtc is not supported
	if (glbin_settings.m_realtime_compress)
	{
		reader->SetResize(1);
		reader->SetAlignment(4);
	}

	if (glbin_settings.m_ser_num > 0)
		reader->LoadBatch(glbin_settings.m_ser_num);
	int chan = reader->GetChanNum();

	int v1, v2;
	if (m_file_num)
	{
		v1 = std::round(100.0 * m_cur_file / m_file_num);
		v2 = std::round(100.0 * (m_cur_file + 1) / m_file_num);
		SetRange(v1, v2);
	}
	else
	{
		v1 = GetMin();
		v2 = GetMax();
	}
	int r = GetRange();

	for (size_t i=(ch_num>=0?ch_num:0);
		i<(ch_num>=0?ch_num+1:chan); i++)
	{
		reader->SetRange(v1 + std::round(double(i) * r / chan),
			v1 + std::round(double(i + 1) * r / chan));

		auto vd = std::make_shared<VolumeData>();
		if (!vd)
			continue;

		vd->SetSkipBrick(glbin_settings.m_skip_brick);
		Nrrd* data = reader->Convert(t_num>=0?t_num:reader->GetCurTime(), i, true);
		if (!data)
			continue;

		std::wstring name;
		if (type != LOAD_TYPE_BRKXML)
		{
			name = reader->GetDataName();
			if (chan > 1)
				name += L"_Ch" + std::to_wstring(i + 1);
		}
		else
		{
			name = reader->GetDataName();
			std::filesystem::path p(name);
			name = p.stem().wstring();
			if (ch_num > 1)
				name = L"_Ch" + std::to_wstring(i);
			pathname = filename;
			auto breader = std::dynamic_pointer_cast<BRKXMLReader>(reader);
			if (breader)
			{
				breader->SetCurChan(i);
				breader->SetCurTime(0);
			}
		}

		vd->SetReader(reader);
		vd->SetCompression(glbin_settings.m_realtime_compress);

		bool valid_spc = reader->IsSpcInfoValid();
		if (vd->Load(data, name, pathname))
		{
			if (glbin_settings.m_load_mask)
			{
				//mask
				MSKReader msk_reader;
				std::wstring str = reader->GetCurMaskName(t_num >= 0 ? t_num : reader->GetCurTime(), i);
				msk_reader.SetFile(str);
				Nrrd* mask = msk_reader.Convert(0, 0, true);
				if (mask)
					vd->LoadMask(mask);
				//label mask
				LBLReader lbl_reader;
				str = reader->GetCurLabelName(t_num >= 0 ? t_num : reader->GetCurTime(), i);
				lbl_reader.SetFile(str);
				Nrrd* label = lbl_reader.Convert(0, 0, true);
				if (label)
					vd->LoadLabel(label);
			}
			if (type == LOAD_TYPE_BRKXML)
			{
				auto breader = std::dynamic_pointer_cast<BRKXMLReader>(reader);
				if (breader)
					breader->SetLevel(0);
			}
			//for 2D data
			int xres, yres, zres;
			vd->GetResolution(xres, yres, zres);
			double zspcfac = (double)std::max(xres, yres) / 256.0;
			if (zspcfac < 1.0) zspcfac = 1.0;
			if (zres == 1) vd->SetBaseSpacings(reader->GetXSpc(), reader->GetYSpc(), reader->GetXSpc()*zspcfac);
			else vd->SetBaseSpacings(reader->GetXSpc(), reader->GetYSpc(), reader->GetZSpc());
			vd->SetSpcFromFile(valid_spc);
			vd->SetScalarScale(reader->GetScalarScale());
			vd->SetMinMaxValue(reader->GetMinValue(), reader->GetMaxValue());
			vd->SetCurTime(reader->GetCurTime());
			vd->SetCurChannel(i);
			//++
			result++;
		}
		else
		{
			continue;
		}

		AddVolumeData(vd);
		SetVolumeDefault(vd);

		//get excitation wavelength
		double wavelength = reader->GetExcitationWavelength(i);
		if (wavelength > 0.0)
		{
			fluo::Color col = GetWavelengthColor(wavelength);
			vd->SetColor(col);
			vd->SetWlColor();
		}
		else if (wavelength < 0.)
		{
			fluo::Color white(1.0, 1.0, 1.0);
			vd->SetColor(white);
			vd->SetWlColor();
		}
		else
		{
			fluo::Color white(1.0, 1.0, 1.0);
			fluo::Color red(1.0, 0.0, 0.0);
			fluo::Color green(0.0, 1.0, 0.0);
			fluo::Color blue(0.0, 0.0, 1.0);
			if (chan == 1)
			{
				vd->SetColor(white);
			}
			else
			{
				if (i == 0)
					vd->SetColor(red);
				else if (i == 1)
					vd->SetColor(green);
				else if (i == 2)
					vd->SetColor(blue);
				else
					vd->SetColor(white);
			}
		}
		if (type == LOAD_TYPE_MPG)
			vd->SetAlphaEnable(false);

		SetProgress(std::round(100.0 * (i + 1) / chan), "NOT_SET");
	}

	SetRange(0, 100);
	return result;
}

void DataManager::LoadMeshes(const std::vector<std::wstring>& files)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	auto view = glbin_current.render_view.lock();

	if (!view)
		view = root->GetView(0);
	if (!view)
		return;

	fluo::ValueCollection vc;
	std::weak_ptr<MeshData> md_sel;
	std::shared_ptr<MeshGroup> group;
	size_t fn = files.size();
	if (fn > 1)
		group = view->AddOrGetMGroup();
	bool enable_4d = false;

	for (size_t i = 0; i < fn; i++)
	{
		SetProgress(static_cast<int>(std::round(100.0 * (i + 1) / fn)),
			"FluoRender is reading and processing selected mesh data. Please wait.");

		std::wstring filename = files[i];
		LoadMeshData(filename);

		auto md = GetLastMeshData();
		if (view && md)
		{
			if (group)
			{
				group->InsertMeshData(group->GetMeshNum() - 1, md);
				view->SetMeshPopDirty();
			}
			else
				view->AddMeshData(md);

			if (i == int(fn - 1))
				md_sel = md;

			if (md->GetReader() && md->GetReader()->GetTimeNum() > 1)
			{
				view->m_tseq_cur_num = md->GetReader()->GetCurTime();
				enable_4d = true;
			}
		}
	}

	if (auto md_sel_ptr = md_sel.lock())
		glbin_current.SetMeshData(md_sel_ptr);

	if (view)
		view->InitView(INIT_BOUNDS | INIT_CENTER);

	if (enable_4d)
	{
		glbin_moviemaker.SetCurrentFrameSilently(view->m_tseq_cur_num, true);
		glbin_moviemaker.SetSeqMode(1);
		vc.insert(gstMovieAgent);
	}

	glbin_render_scheduler_manager.requestDraw(
		{ static_cast<int>(view->Id()) }, "Load volume refersh");

	vc.insert({ gstCurrentSelect, gstMeshPropPanel, gstListCtrl, gstTreeCtrl });
	m_frame->UpdateProps(vc);

	SetProgress(0, "");
}

bool DataManager::LoadMeshData(const std::wstring &filename)
{
	std::wstring pathname = filename;
	if (!std::filesystem::exists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!std::filesystem::exists(pathname))
			return false;
	}

	std::shared_ptr<BaseMeshReader> reader;

	for (size_t i=0; i<m_mesh_reader_list.size(); i++)
	{
		std::wstring wstr = pathname;
		if (m_mesh_reader_list[i]->Match(wstr))
		{
			reader = m_mesh_reader_list[i];
			break;
		}
	}

	int reader_return = -1;
	if (reader)
	{
		reader->SetProgressFunc(GetProgressFunc());
		bool preprocess = false;
		std::wstring str_w = glbin_settings.m_time_id;
		if (reader->GetTimeId() != str_w)
		{
			reader->SetTimeId(str_w);
			preprocess = true;
		}
		if (preprocess)
			reader_return = reader->Preprocess();
	}
	else
	{
		if (GET_SUFFIX(pathname) == L".obj")
			reader = std::make_shared<ObjReader>();
		else
			return false;
	}
	reader->SetProgressFunc(GetProgressFunc());
	m_mesh_reader_list.push_back(reader);
	reader->SetFile(pathname);
	reader->SetTimeId(glbin_settings.m_time_id);
	reader_return = reader->Preprocess();

	if (reader_return > 0)
	{
		std::string err_str = BaseMeshReader::GetError(reader_return);
		SetProgress(0, err_str);
		int i = (int)m_mesh_reader_list.size() - 1;
		if (m_mesh_reader_list[i]) {
			m_mesh_reader_list.erase(m_mesh_reader_list.begin() + (int)m_mesh_reader_list.size() - 1);
		}
		return false;
	}

	if (glbin_settings.m_ser_num > 0)
		reader->LoadBatch(glbin_settings.m_ser_num);

	reader->SetRange(0, 100);

	auto md = std::make_shared<MeshData>();
	if (!md)
		return false;
	auto model = reader->Convert();
	md->Load(model);
	//md->Load(pathname);
	md->SetReader(reader);

	std::wstring name = md->GetName();
	std::wstring new_name = name;
	size_t i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name + L"_" + std::to_wstring(i);
	if (i>1)
		md->SetName(new_name);
	//m_md_list.push_back(md);

	AddMeshData(md);

	SetProgress(100, "NOT_SET");
	SetRange(0, 100);
	return true;
}

bool DataManager::AddMeshData(const std::shared_ptr<MeshData>& md)
{
	//check if already exist
	for (auto& it : m_md_list)
		if (it == md)
			return false;

	//make sure unique name
	std::wstring name = md->GetName();
	std::wstring new_name = name;
	size_t i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name + L"_" + std::to_wstring(i);
	if (i>1)
		md->SetName(new_name);
	m_md_list.push_back(md);
	return true;
}

std::shared_ptr<VolumeData> DataManager::GetVolumeData(size_t index)
{
	if (index<m_vd_list.size())
		return m_vd_list[index];
	else
		return nullptr;
}

std::shared_ptr<MeshData> DataManager::GetMeshData(size_t index)
{
	if (index<m_md_list.size())
		return m_md_list[index];
	else
		return nullptr;
}

std::shared_ptr<VolumeData> DataManager::GetVolumeData(const std::wstring &name)
{
	for (size_t i=0 ; i<m_vd_list.size() ; i++)
	{
		if (name == m_vd_list[i]->GetName())
		{
			return m_vd_list[i];
		}
	}
	return nullptr;
}

std::shared_ptr<MeshData> DataManager::GetMeshData(const std::wstring &name)
{
	for (size_t i=0 ; i<m_md_list.size() ; i++)
	{
		if (name == m_md_list[i]->GetName())
		{
			return m_md_list[i];
		}
	}
	return nullptr;
}

size_t DataManager::GetVolumeIndex(const std::wstring &name)
{
	for (size_t i=0 ; i<m_vd_list.size() ; i++)
	{
		if (!m_vd_list[i])
			continue;
		if (name == m_vd_list[i]->GetName())
		{
			return i;
		}
	}
	return -1;
}

std::shared_ptr<VolumeData> DataManager::GetLastVolumeData()
{
	size_t num = m_vd_list.size();
	if (num)
		return m_vd_list[num-1];
	else
		return nullptr;
}

size_t DataManager::GetMeshIndex(const std::wstring &name)
{
	for (size_t i=0 ; i<m_md_list.size() ; i++)
	{
		if (name == m_md_list[i]->GetName())
		{
			return i;
		}
	}
	return -1;
}

std::shared_ptr<MeshData> DataManager::GetLastMeshData()
{
	size_t num = m_md_list.size();
	if (num)
		return m_md_list[num-1];
	else
		return nullptr;
}

void DataManager::RemoveVolumeData(size_t index)
{
	if (index < m_vd_list.size())
	{
		auto data = m_vd_list[index];
		if (data)
			m_vd_cache_queue.erase(data.get());
		m_vd_list.erase(m_vd_list.begin() + index);
	}
}

void DataManager::RemoveVolumeData(const std::wstring &name)
{
	for (size_t i = 0; i<m_vd_list.size(); i++)
	{
		if (name == m_vd_list[i]->GetName())
		{
			RemoveVolumeData(i);
		}
	}
}

void DataManager::RemoveMeshData(size_t index)
{
	if (index < m_md_list.size())
	{
		m_md_list.erase(m_md_list.begin()+index);
	}
}

void DataManager::ClearMeshSelection()
{
	for (auto it : m_md_list)
	{
		if (it)
			it->SetDrawBounds(false);
	}
}

size_t DataManager::GetVolumeNum()
{
	return m_vd_list.size();
}

size_t DataManager::GetMeshNum()
{
	return m_md_list.size();
}

void DataManager::AddVolumeData(const std::shared_ptr<VolumeData>& vd)
{
	if (!vd)
		return;

	std::wstring name = vd->GetName();
	std::wstring new_name = name;

	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name + L"_" + std::to_wstring(i);

	if (i>1)
		vd->SetName(new_name);

	if (glbin_settings.m_override_vox)
	{
		if (m_vd_list.size() > 0)
		{
			double spcx, spcy, spcz;
			m_vd_list[0]->GetBaseSpacings(spcx, spcy, spcz);
			vd->SetSpacings(spcx, spcy, spcz);
			vd->SetBaseSpacings(spcx, spcy, spcz);
			//vd->SetSpcFromFile(true);
		}
	}
	m_vd_list.push_back(vd);
	auto vol_cache_queue = std::make_shared<flvr::CacheQueue>(vd);
	vol_cache_queue->RegisterCacheQueueFuncs(flvr::CQCallback::ReadVolCache, flvr::CQCallback::FreeVolCache);
	//set up default vol cache mode
	vol_cache_queue->SetHandleFlags(flvr::CQCallback::HDL_DATA | flvr::CQCallback::TIME_COND0);
	vd->GetVR()->set_cache_queue(vol_cache_queue);
	m_vd_cache_queue[vd.get()] = vol_cache_queue;
}

std::shared_ptr<VolumeData> DataManager::DuplicateVolumeData(const std::shared_ptr<VolumeData>& vd)
{
	if (vd)
	{
		auto vd_new = std::make_shared<VolumeData>(*vd);
		AddVolumeData(vd_new);
		return vd_new;
	}
	return nullptr;
}

bool DataManager::LoadAnnotData(const std::wstring &filename)
{
	std::wstring pathname = filename;
	if (!std::filesystem::exists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!std::filesystem::exists(pathname))
			return false;
	}

	auto ann = std::make_shared<AnnotData>();
	ann->Load(pathname);

	std::wstring name = ann->GetName();
	std::wstring new_name = name;
	size_t i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name + L"_" + std::to_wstring(i);
	if (i>1)
		ann->SetName(new_name);
	m_ad_list.push_back(ann);

	return true;
}

void DataManager::AddAnnotData(const std::shared_ptr<AnnotData>& ann)
{
	if (!ann)
		return;

	std::wstring name = ann->GetName();
	std::wstring new_name = name;

	size_t i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name + L"_" + std::to_wstring(i);
	if (i>1)
		ann->SetName(new_name);

	m_ad_list.push_back(ann);
}

void DataManager::RemoveAnnotData(size_t index)
{
	if (index < m_ad_list.size())
	{
		m_ad_list.erase(m_ad_list.begin()+index);
	}
}

size_t DataManager::GetAnnotNum()
{
	return m_ad_list.size();
}

std::shared_ptr<AnnotData> DataManager::GetAnnotData(size_t index)
{
	if (index<m_ad_list.size())
		return m_ad_list[index];
	else
		return nullptr;
}

std::shared_ptr<AnnotData> DataManager::GetAnnotData(const std::wstring &name)
{
	for (size_t i=0; i<m_ad_list.size(); i++)
	{
		if (name == m_ad_list[i]->GetName())
			return m_ad_list[i];
	}
	return nullptr;
}

size_t DataManager::GetAnnotIndex(const std::wstring &name)
{
	for (size_t i=0; i<m_ad_list.size(); i++)
	{
		if (!m_ad_list[i])
			continue;
		if (name == m_ad_list[i]->GetName())
			return i;
	}
	return -1;
}

std::shared_ptr<AnnotData> DataManager::GetLastAnnotData()
{
	size_t num = m_ad_list.size();
	if (num)
		return m_ad_list[num - 1];
	else
		return nullptr;
}

bool DataManager::CheckNames(const std::wstring &str)
{
	bool result = false;
	for (auto& vd : m_vd_list)
	{
		if (vd && vd->GetName()==str)
		{
			result = true;
			break;
		}
	}
	if (!result)
	{
		for (auto& md : m_md_list)
		{
			if (md && md->GetName()==str)
			{
				result = true;
				break;
			}
		}
	}
	if (!result)
	{
		for (auto& ann : m_ad_list)
		{
			if (ann && ann->GetName()==str)
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

fluo::Color DataManager::GetColor(int c)
{
	fluo::Color result(1.0, 1.0, 1.0);
	switch (c)
	{
	case 1://red
		result = fluo::Color(1.0, 0.0, 0.0);
		break;
	case 2://green
		result = fluo::Color(0.0, 1.0, 0.0);
		break;
	case 3://blue
		result = fluo::Color(0.0, 0.0, 1.0);
		break;
	case 4://cyan
		result = fluo::Color(0.0, 1.0, 1.0);
		break;
	case 5://magenta
		result = fluo::Color(1.0, 0.0, 1.0);
		break;
	case 6://yellow
		result = fluo::Color(1.0, 1.0, 0.0);
		break;
	case 7://orange
		result = fluo::Color(1.0, 0.5, 0.0);
		break;
	case 8://white
		result = fluo::Color(1.0, 1.0, 1.0);
		break;
	}
	return result;
}

fluo::Color DataManager::GetWavelengthColor(double wavelength)
{
	if (wavelength < 340.0)
		return fluo::Color(1.0, 1.0, 1.0);
	else if (wavelength < 440.0)
		return GetColor(glbin_settings.m_wav_color1);
	else if (wavelength < 500.0)
		return GetColor(glbin_settings.m_wav_color2);
	else if (wavelength < 600.0)
		return GetColor(glbin_settings.m_wav_color3);
	else if (wavelength < 750.0)
		return GetColor(glbin_settings.m_wav_color4);
	else
		return fluo::Color(1.0, 1.0, 1.0);
}

flvr::CacheQueue* DataManager::GetCacheQueue(VolumeData* vd)
{
	if (!vd)
		return nullptr;
	auto it = m_vd_cache_queue.find(vd);
	if (it != m_vd_cache_queue.end() && it->second)
		return it->second.get();
	return nullptr;
}

void DataManager::UpdateStreamMode(double data_size)
{
	int stream_rendering = glbin_settings.m_stream_rendering;

	switch (stream_rendering)
	{
	case 0://disable
		glbin_settings.m_mem_swap = false;
		break;
	case 1://enable
		glbin_settings.m_mem_swap = true;
		break;
	case 2://enable for large data
		if (data_size < 0.0)
			return;
		if (data_size > glbin_settings.m_large_data_size ||
			data_size > glbin_settings.m_mem_limit)
		{
			glbin_settings.m_mem_swap = true;
		}
		else
		{
			glbin_settings.m_mem_swap = false;
		}
		break;
	}
}
