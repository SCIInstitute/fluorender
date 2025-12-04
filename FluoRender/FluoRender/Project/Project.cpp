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
#include <Project.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <MainFrame.h>
#include <TreePanel.h>
#include <RenderViewPanel.h>
#include <ClipPlanePanel.h>
#include <MoviePanel.h>
#include <OutputAdjPanel.h>
#include <ProjectPanel.h>
#include <BrushToolDlg.h>
#include <ComponentDlg.h>
#include <MachineLearningDlg.h>
#include <MeasureDlg.h>
#include <SettingDlg.h>
#include <TrackDlg.h>
#include <RenderView.h>
#include <VolumeGroup.h>
#include <MeshGroup.h>
#include <Root.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <AnnotData.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <compatibility.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>
#include <base_vol_reader.h>
#include <msk_reader.h>
#include <VolumeRenderer.h>
#include <MeshRenderer.h>
#include <BaseXrRenderer.h>
#include <VolumeSelector.h>
#include <MovieMaker.h>
#include <Interpolator.h>
#include <VolumePoint.h>
#include <CompAnalyzer.h>
#include <RefreshScheduler.h>
#include <Ruler.h>
#include <format>

Project::Project() :
	Progress()
{
}

Project::~Project()
{
}

void Project::Open(const std::wstring& filename)
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
		return;
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;

	glbin_data_manager.SetProjectPath(filename);

	//clear
	glbin_data_manager.ClearAll();
	VolumeGroup::ResetID();
	MeshGroup::ResetID();
	root->GetView(0)->ClearAll();
	for (int i = root->GetViewNum() - 1; i > 0; i--)
		frame->DeleteRenderViewPanel(i);

	std::shared_ptr<BaseTreeFile> fconfig =
		glbin_tree_file_factory.createTreeFile(filename, gstProjectFile);
	if (!fconfig)
		return;

	if (fconfig->LoadFile(filename))
		return;

	std::string ver_major, ver_minor;
	long l_major;
	double d_minor;
	l_major = 1;
	if (fconfig->Read("ver_major", &ver_major) &&
		fconfig->Read("ver_minor", &ver_minor))
	{
		l_major = STOL(ver_major);
		d_minor = STOD(ver_minor);

		if (l_major > VERSION_MAJOR)
			SetProgress(0, "The project file is saved by a newer version of FluoRender.\n" \
				"Please check update and download the new version.");
		else if (d_minor > VERSION_MINOR)
			SetProgress(0, "The project file is saved by a newer version of FluoRender.\n" \
				"Please check update and download the new version.");
	}

	int ticks = 3;
	int tick_cnt = 1;
	fconfig->Read("ticks", &ticks);

	SetProgress(0, "FluoRender is reading the project file. Please wait.");

	bool bval;
	double dval;
	int ival;
	std::string path;
	std::string sval;
	std::wstring wsval;
	fluo::Color cval;
	fluo::HSVColor hval;
	fluo::Vector vval;
	fluo::Quaternion qval;
	fluo::Point pval;

	//read streaming mode
	path = "/settings";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		fconfig->Read("interactive quality", &ival, 2);
		glbin_settings.m_interactive_quality = ival;
		fconfig->Read("stream rendering", &ival, 2);
		glbin_settings.m_stream_rendering = ival;
		fconfig->Read("graphics mem", &dval, 1000.0);
		glbin_settings.m_graphics_mem = dval;
		fconfig->Read("large data size", &dval, 1000.0);
		glbin_settings.m_large_data_size = dval;
		fconfig->Read("force brick size", &ival, 128);
		glbin_settings.m_force_brick_size = ival;
		fconfig->Read("up time", &ival, 100);
		glbin_settings.m_up_time = ival;
		fconfig->Read("update order", &ival, 0);
		glbin_settings.m_update_order = ival;
		fconfig->Read("inf loop", &bval, false);
		glbin_settings.m_inf_loop = bval;
		//graphics memory setting may have changed
		glbin_settings.GetGraphicsInfo();
		//peeling layers
		fconfig->Read("peeling layers", &ival, 1);
		glbin_settings.m_peeling_layers = ival;
		//UpdateProps({ gstMouseInt, gstStreamEnable, gstPeelNum });
		glbin_data_manager.UpdateStreamMode(-1.0);
	}

	//current
	glbin_current.SetRoot();
	std::wstring cur_canvas, cur_vol_group, cur_mesh_group,
		cur_vol_data, cur_mesh_data, cur_ann_data;
	path = "/current";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		fconfig->Read("canvas", &cur_canvas);
		fconfig->Read("vol group", &cur_vol_group);
		fconfig->Read("mesh group", &cur_mesh_group);
		fconfig->Read("vol data", &cur_vol_data);
		fconfig->Read("mesh data", &cur_mesh_data);
		fconfig->Read("ann data", &cur_ann_data);
	}

	//read data list
	//volume
	path = "/data/volume";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		size_t num = 0;
		fconfig->Read("num", &num);
		for (size_t i = 0; i < num; i++)
		{
			SetProgress(100 * tick_cnt / ticks,
				"FluoRender is reading and processing volume data. Please wait.");

			glbin_data_manager.SetRange(
				std::round(100.0 * tick_cnt / ticks),
				std::round(100.0 * (tick_cnt + 1) / ticks));
			path = "/data/volume/" + std::to_string(i);
			if (fconfig->Exists(path))
			{
				int loaded_num = 0;
				fconfig->SetPath(path);
				fconfig->Read("compression", &bval);
				glbin_settings.m_realtime_compress = bval;
				fconfig->Read("skip_brick", &bval);
				glbin_settings.m_skip_brick = bval;
				//path
				std::wstring filepath;
				if (fconfig->Read("path", &filepath))
				{
					int cur_chan = 0;
					if (!fconfig->Read("cur_chan", &cur_chan))
						if (fconfig->Read("tiff_chan", &cur_chan))
							cur_chan--;
					int cur_time = 0;
					fconfig->Read("cur_time", &cur_time);
					//reader type
					int reader_type = 0;
					fconfig->Read("reader_type", &reader_type);
					fconfig->Read("slice_seq", &glbin_settings.m_slice_sequence);
					fconfig->Read("time_id", &glbin_settings.m_time_id);
					fconfig->Read("fp_convert", &glbin_settings.m_fp_convert);
					fconfig->Read("fp_min", &glbin_settings.m_fp_min);
					fconfig->Read("fp_max", &glbin_settings.m_fp_max, 1.0);
					std::wstring suffix = GET_SUFFIX(filepath);
					if (reader_type == READER_IMAGEJ_TYPE)
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_IMAGEJ, true, cur_chan, cur_time);
					else if (suffix == L".nrrd" || suffix == L".msk" || suffix == L".lbl")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_NRRD, false, cur_chan, cur_time);
					else if (suffix == L".tif" || suffix == L".tiff")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_TIFF, false, cur_chan, cur_time);
					else if (suffix == L".png")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_PNG, false, cur_chan, cur_time);
					else if (suffix == L".jpg" || suffix == L".jpeg")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_JPG, false, cur_chan, cur_time);
					else if (suffix == L".jp2")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_JP2, false, cur_chan, cur_time);
					else if (suffix == L".oib")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_OIB, false, cur_chan, cur_time);
					else if (suffix == L".oif")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_OIF, false, cur_chan, cur_time);
					else if (suffix == L".lsm")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_LSM, false, cur_chan, cur_time);
					else if (suffix == L".xml")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_PVXML, false, cur_chan, cur_time);
					else if (suffix == L".vvd")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_BRKXML, false, cur_chan, cur_time);
					else if (suffix == L".czi")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_CZI, false, cur_chan, cur_time);
					else if (suffix == L".nd2")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_ND2, false, cur_chan, cur_time);
					else if (suffix == L".lif")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_LIF, false, cur_chan, cur_time);
					else if (suffix == L".lof")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_LOF, false, cur_chan, cur_time);
					else if (suffix == L".mp4" || suffix == L".m4v" || suffix == L".mov" || suffix == L".avi" || suffix == L".wmv")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_MPG, false, cur_chan, cur_time);
					else if (suffix == L".dcm" || suffix == L".dicom")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_DCM, false, cur_chan, cur_time);
				}
				std::shared_ptr<VolumeData> vd;
				if (loaded_num)
					vd = glbin_data_manager.GetLastVolumeData();
				if (vd)
				{
					if (fconfig->Read("name", &wsval))
						vd->SetName(wsval);//setname
					//volume properties
					path = "properties";
					if (fconfig->Exists(path))
					{
						fconfig->SetPath(path);
						if (fconfig->Read("display", &bval))
							vd->SetDisp(bval);

						//old colormap
						if (fconfig->Read("widget", &wsval))
						{
							int type;
							float left_x, left_y, width, height, offset1, offset2, gamma;
							wchar_t token[256] = {};
							token[255] = L'\0';
							const wchar_t* sstr = wsval.c_str();
							std::wstringstream ss(sstr);
							ss.read(token, 255);
							wchar_t c = L'x';
							while (!isspace(c)) ss.read(&c, 1);
							ss >> type >> left_x >> left_y >> width >>
								height >> offset1 >> offset2 >> gamma;
							vd->SetGamma(gamma);
							vd->SetBoundaryLow(left_y);
							vd->SetLowOffset(offset1);
							vd->SetHighOffset(offset2);
							vd->SetLeftThresh(left_x);
							vd->SetRightThresh(left_x + width);
							if (fconfig->Read("widgetcolor", &cval))
								vd->SetColor(cval);
							if (fconfig->Read("widgetalpha", &dval))
								vd->SetAlpha(dval);
						}

						//transfer function
						if (fconfig->Read("3dgamma enable", &bval))
							vd->SetGammaEnable(bval);
						if (fconfig->Read("3dgamma", &dval))
							vd->SetGamma(dval);
						if (fconfig->Read("boundary enable", &bval))
							vd->SetBoundaryEnable(bval);
						if (fconfig->Read("boundary low", &dval))
							vd->SetBoundaryLow(dval);
						if (fconfig->Read("boundary high", &dval))
							vd->SetBoundaryHigh(dval);
						if (fconfig->Read("boundary max", &dval))
							vd->SetBoundaryMax(dval);
						if (fconfig->Read("minmax enable", &bval))
							vd->SetMinMaxEnable(bval);
						if (fconfig->Read("low offset", &dval))
							vd->SetLowOffset(dval);
						if (fconfig->Read("high offset", &dval))
							vd->SetHighOffset(dval);
						if (fconfig->Read("threshold enable", &bval))
							vd->SetThreshEnable(bval);
						if (fconfig->Read("left_thresh", &dval))
							vd->SetLeftThresh(dval);
						if (fconfig->Read("right_thresh", &dval))
							vd->SetRightThresh(dval);
						if (fconfig->Read("luminance enable", &bval))
							vd->SetLuminanceEnable(bval);
						if (fconfig->Read("luminance", &dval))
							vd->SetLuminance(dval);
						if (fconfig->Read("color", &cval))
							vd->SetColor(cval);
						if (fconfig->Read("mask_color", &cval))
						{
							if (fconfig->Read("mask_color_set", &bval))
								vd->SetMaskColor(cval, bval);
							else
								vd->SetMaskColor(cval);
						}
						if (fconfig->Read("enable_alpha", &bval))
							vd->SetAlphaEnable(bval);
						if (fconfig->Read("alpha", &dval))
							vd->SetAlpha(dval);

						//shading
						if (fconfig->Read("shading", &bval))
							vd->SetShadingEnable(bval);
						if (fconfig->Read("shading_strength", &dval))
							vd->SetShadingStrength(dval);
						if (fconfig->Read("shading_shine", &dval))
							vd->SetShadingShine(dval);
						if (fconfig->Read("samplerate", &dval))
						{
							if (l_major < 2)
								vd->SetSampleRate(dval / 5.0);
							else
								vd->SetSampleRate(dval);
						}

						//spacings and scales
						if (!vd->isBrxml())
						{
							if (fconfig->Read("res", &vval))
								vd->SetBaseSpacing(vval);
						}
						else
						{
							if (fconfig->Read("b_res", &vval))
								vd->SetBaseSpacing(vval);
							if (fconfig->Read("s_res", &vval))
								vd->SetSpacingScale(vval);
						}
						if (fconfig->Read("scl", &vval))
							vd->SetScaling(vval);

						//clip values
						if (fconfig->Read("clip_xneg", &dval))
							vd->SetClipValue(fluo::ClipPlane::XNeg, dval);
						if (fconfig->Read("clip_xpos", &dval))
							vd->SetClipValue(fluo::ClipPlane::XPos, dval);
						if (fconfig->Read("clip_yneg", &dval))
							vd->SetClipValue(fluo::ClipPlane::YNeg, dval);
						if (fconfig->Read("clip_ypos", &dval))
							vd->SetClipValue(fluo::ClipPlane::YPos, dval);
						if (fconfig->Read("clip_zneg", &dval))
							vd->SetClipValue(fluo::ClipPlane::ZNeg, dval);
						if (fconfig->Read("clip_zpos", &dval))
							vd->SetClipValue(fluo::ClipPlane::ZPos, dval);
						//clip rotation
						if (fconfig->Read("clip_rot", &vval))
							vd->SetClipRotation(vval);
						//clip link
						if (fconfig->Read("clip_link_x", &bval))
							vd->SetLink(fluo::ClipPlane::XNeg, bval);
						if (fconfig->Read("clip_link_y", &bval))
							vd->SetLink(fluo::ClipPlane::YNeg, bval);
						if (fconfig->Read("clip_link_z", &bval))
							vd->SetLink(fluo::ClipPlane::ZNeg, bval);
						//dist
						if (fconfig->Read("clip_dist_x", &ival))
							vd->SetLinkedDist(fluo::ClipPlane::XNeg, ival);
						if (fconfig->Read("clip_dist_y", &ival))
							vd->SetLinkedDist(fluo::ClipPlane::YNeg, ival);
						if (fconfig->Read("clip_dist_z", &ival))
							vd->SetLinkedDist(fluo::ClipPlane::ZNeg, ival);

						//2d adjustment settings
						if (fconfig->Read("gamma", &cval))
							vd->SetGammaColor(cval);
						if (fconfig->Read("brightness", &cval))
							vd->SetBrightness(cval);
						if (fconfig->Read("hdr", &cval))
							vd->SetHdr(cval);
						if (fconfig->Read("sync_r", &bval))
							vd->SetSync(0, bval);
						if (fconfig->Read("sync_g", &bval))
							vd->SetSync(1, bval);
						if (fconfig->Read("sync_b", &bval))
							vd->SetSync(2, bval);

						//colormap settings
						if (fconfig->Read("colormap_mode", &ival))
							vd->SetColorMode(static_cast<flvr::ColorMode>(ival));
						if (fconfig->Read("colormap_inv", &dval))
							vd->SetColormapInv(dval);
						if (fconfig->Read("colormap", &ival))
							vd->SetColormap(ival);
						if (fconfig->Read("colormap_proj", &ival))
							vd->SetColormapProj(static_cast<flvr::ColormapProj>(ival));
						double low, high;
						if (fconfig->Read("colormap_lo_value", &low) &&
							fconfig->Read("colormap_hi_value", &high))
						{
							vd->SetColormapValues(low, high);
						}

						//high transp
						if (fconfig->Read("alpha_power", &dval))
							vd->SetAlphaPower(dval);
						//inversion
						if (fconfig->Read("inv", &bval))
							vd->SetInvert(bval);
						//mip enable
						if (fconfig->Read("mode", &ival))
							vd->SetRenderMode(static_cast<flvr::RenderMode>(ival));
						//noise reduction
						if (fconfig->Read("noise_red", &bval))
							vd->SetNR(bval);
						//depth override
						if (fconfig->Read("channel_mix_mode", &ival))
							vd->SetChannelMixMode(static_cast<ChannelMixMode>(ival));

						//shadow
						if (fconfig->Read("shadow", &bval))
							vd->SetShadowEnable(bval);
						//shaodw intensity
						if (fconfig->Read("shadow_darkness", &dval))
							vd->SetShadowIntensity(dval);

						//legend
						if (fconfig->Read("legend", &bval))
							vd->SetLegend(bval);

						//mask
						if (fconfig->Read("mask", &wsval))
						{
							MSKReader msk_reader;
							msk_reader.SetFile(wsval);
							BaseVolReader* br = &msk_reader;
							Nrrd* mask = br->Convert(true);
							if (mask)
								vd->LoadMask(mask);
						}
					}
				}
			}
			tick_cnt++;
		}
		glbin_current.vol_data = glbin_data_manager.GetVolumeData(cur_vol_data);
	}
	//mesh
	path = "/data/mesh";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		size_t num = 0;
		fconfig->Read("num", &num);
		for (size_t i = 0; i < num; i++)
		{
			SetProgress(100 * tick_cnt / ticks,
				"FluoRender is reading and processing mesh data. Please wait.");

			path = "/data/mesh/" + std::to_string(i);
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				if (fconfig->Read("path", &wsval))
				{
					glbin_data_manager.LoadMeshData(wsval);
				}
				auto md = glbin_data_manager.GetLastMeshData();
				if (md)
				{
					if (fconfig->Read("name", &wsval))
						md->SetName(wsval);//setname
					//mesh properties
					path = "properties";
					if (fconfig->Exists(path))
					{
						fconfig->SetPath(path);
						if (fconfig->Read("display", &bval))
							md->SetDisp(bval);
						//lighting
						if (fconfig->Read("shading", &bval))
							md->SetShading(bval);
						if (fconfig->Read("shading_strength", &dval))
							md->SetShadingStrength(dval);
						if (fconfig->Read("shininess", &dval, 30.0))
							md->SetShadingShine(dval);
						if (fconfig->Read("alpha", &dval, 0.5))
							md->SetAlpha(dval);
						if (fconfig->Read("color", &cval))
							md->SetColor(cval);
						//2d adjusment settings
						if (fconfig->Read("gamma", &cval))
							md->SetGammaColor(cval);
						if (fconfig->Read("brightness", &cval))
							md->SetBrightness(cval);
						if (fconfig->Read("hdr", &cval))
							md->SetHdr(cval);
						if (fconfig->Read("sync_r", &bval))
							md->SetSync(0, bval);
						if (fconfig->Read("sync_g", &bval))
							md->SetSync(1, bval);
						if (fconfig->Read("sync_b", &bval))
							md->SetSync(2, bval);
						//shadow
						if (fconfig->Read("shadow", &bval))
							md->SetShadowEnable(bval);
						double darkness;
						if (fconfig->Read("shadow_darkness", &darkness))
							md->SetShadowIntensity(darkness);

						//clip values
						if (fconfig->Read("clip_xneg", &dval))
							md->SetClipValue(fluo::ClipPlane::XNeg, dval);
						if (fconfig->Read("clip_xpos", &dval))
							md->SetClipValue(fluo::ClipPlane::XPos, dval);
						if (fconfig->Read("clip_yneg", &dval))
							md->SetClipValue(fluo::ClipPlane::YNeg, dval);
						if (fconfig->Read("clip_ypos", &dval))
							md->SetClipValue(fluo::ClipPlane::YPos, dval);
						if (fconfig->Read("clip_zneg", &dval))
							md->SetClipValue(fluo::ClipPlane::ZNeg, dval);
						if (fconfig->Read("clip_zpos", &dval))
							md->SetClipValue(fluo::ClipPlane::ZPos, dval);
						//clip rotation
						if (fconfig->Read("clip_rot", &vval))
							md->SetClipRotation(vval);
						//clip link
						if (fconfig->Read("clip_link_x", &bval))
							md->SetLink(fluo::ClipPlane::XNeg, bval);
						if (fconfig->Read("clip_link_y", &bval))
							md->SetLink(fluo::ClipPlane::YNeg, bval);
						if (fconfig->Read("clip_link_z", &bval))
							md->SetLink(fluo::ClipPlane::ZNeg, bval);
						//dist
						if (fconfig->Read("clip_dist_x", &ival))
							md->SetLinkedDist(fluo::ClipPlane::XNeg, ival);
						if (fconfig->Read("clip_dist_y", &ival))
							md->SetLinkedDist(fluo::ClipPlane::YNeg, ival);
						if (fconfig->Read("clip_dist_z", &ival))
							md->SetLinkedDist(fluo::ClipPlane::ZNeg, ival);

						//mesh transform
						path = "../transform";
						if (fconfig->Exists(path))
						{
							fconfig->SetPath(path);
							if (fconfig->Read("translation", &vval))
								md->SetTranslation(vval);
							if (fconfig->Read("rotation", &vval))
								md->SetRotation(vval);
							if (fconfig->Read("scaling", &vval))
								md->SetScaling(vval);
						}
					}
				}
			}
			tick_cnt++;
		}
		glbin_current.mesh_data = glbin_data_manager.GetMeshData(cur_mesh_data);
	}
	//annotations
	path = "/data/annotations";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		size_t num = 0;
		fconfig->Read("num", &num);
		for (size_t i = 0; i < num; i++)
		{
			path = "/data/annotations/" + std::to_string(i);
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				if (fconfig->Read("path", &wsval))
				{
					glbin_data_manager.LoadAnnotData(wsval);
				}
			}
		}
		glbin_current.ann_data = glbin_data_manager.GetAnnotData(cur_ann_data);
	}

	//views
	path = "/views";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		size_t num = 0;
		fconfig->Read("num", &num);

		for (size_t i = 0; i < num; i++)
		{
			if (i > 0)
				frame->CreateRenderViewPanel();
			auto view = root->GetLastView();
			if (!view)
				continue;

			view->ClearAll();

			//old
			//volumes
			path = "/views/" + std::to_string(i) + "/volumes";
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				int num2 = 0;
				fconfig->Read("num", &num2);
				for (int j = 0; j < num2; j++)
				{
					if (fconfig->Read("name" + std::to_string(j), &wsval))
					{
						auto vd = glbin_data_manager.GetVolumeData(wsval);
						if (vd)
							view->AddVolumeData(vd);
					}
				}
				view->SetVolPopDirty();
			}
			//meshes
			path = "/views/" + std::to_string(i) + "/meshes";
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				int num2 = 0;
				fconfig->Read("num", &num2);
				for (int j = 0; j < num2; j++)
				{
					if (fconfig->Read("name" + std::to_string(j), &wsval))
					{
						auto md = glbin_data_manager.GetMeshData(wsval);
						if (md)
							view->AddMeshData(md);
					}
				}
			}

			//new
			path = "/views/" + std::to_string(i) + "/layers";
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);

				//view layers
				int layer_num = 0;
				fconfig->Read("num", &layer_num);
				for (int j = 0; j < layer_num; j++)
				{
					path = "/views/" + std::to_string(i) + "/layers/" + std::to_string(j);
					if (fconfig->Exists(path))
					{
						fconfig->SetPath(path);
						int type;
						if (fconfig->Read("type", &type))
						{
							switch (type)
							{
							case 2://volume data
							{
								if (fconfig->Read("name", &wsval))
								{
									auto vd = glbin_data_manager.GetVolumeData(wsval);
									if (vd)
										view->AddVolumeData(vd);
								}
							}
							break;
							case 3://mesh data
							{
								if (fconfig->Read("name", &wsval))
								{
									auto md = glbin_data_manager.GetMeshData(wsval);
									if (md)
										view->AddMeshData(md);
								}
							}
							break;
							case 4://annotations
							{
								if (fconfig->Read("name", &wsval))
								{
									auto ann = glbin_data_manager.GetAnnotData(wsval);
									if (ann)
										view->AddAnnotData(ann);
								}
							}
							break;
							case 5://group
							{
								if (fconfig->Read("name", &wsval))
								{
									if (fconfig->Read("id", &ival))
										VolumeGroup::SetID(ival);
									wsval = view->AddGroup(wsval);
									auto group = view->GetGroup(wsval);
									if (group)
									{
										//display
										if (fconfig->Read("display", &bval))
											group->SetDisp(bval);
										//2d adjustment
										if (fconfig->Read("gamma", &cval))
											group->SetGammaColor(cval);
										if (fconfig->Read("brightness", &cval))
											group->SetBrightness(cval);
										if (fconfig->Read("hdr", &cval))
											group->SetHdr(cval);
										if (fconfig->Read("sync_r", &bval))
											group->SetSync(0, bval);
										if (fconfig->Read("sync_g", &bval))
											group->SetSync(1, bval);
										if (fconfig->Read("sync_b", &bval))
											group->SetSync(2, bval);
										//sync volume properties
										if (fconfig->Read("sync_vp", &bval))
											group->SetVolumeSyncProp(bval);
										//volumes
										path = "/views/" + std::to_string(i) + "/layers/" + std::to_string(j) + "/volumes";
										if (fconfig->Exists(path))
										{
											fconfig->SetPath(path);
											int vol_num = 0;
											fconfig->Read("num", &vol_num);
											for (int k = 0; k < vol_num; k++)
											{
												if (fconfig->Read("vol_" + std::to_string(k), &wsval))
												{
													auto vd = glbin_data_manager.GetVolumeData(wsval);
													if (vd)
													{
														group->InsertVolumeData(k - 1, vd);
														//AddProps(2, view, group, vd);
													}
												}
											}
										}
										if (group->GetName() == cur_vol_group)
											glbin_current.vol_group = group;
									}
									view->SetVolPopDirty();
								}
							}
							break;
							case 6://mesh group
							{
								if (fconfig->Read("name", &wsval))
								{
									if (fconfig->Read("id", &ival))
										MeshGroup::SetID(ival);
									wsval = view->AddMGroup(wsval);
									auto group = view->GetMGroup(wsval);
									if (group)
									{
										//display
										if (fconfig->Read("display", &bval))
											group->SetDisp(bval);
										//sync mesh properties
										if (fconfig->Read("sync_mp", &bval))
											group->SetMeshSyncProp(bval);
										//meshes
										path = "/views/" + std::to_string(i) + "/layers/" + std::to_string(j) + "/meshes";
										if (fconfig->Exists(path))
										{
											fconfig->SetPath(path);
											int mesh_num = 0;
											fconfig->Read("num", &mesh_num);
											for (int k = 0; k < mesh_num; k++)
											{
												if (fconfig->Read("mesh_" + std::to_string(k), &wsval))
												{
													auto md = glbin_data_manager.GetMeshData(wsval);
													if (md)
													{
														group->InsertMeshData(k - 1, md);
														//AddProps(3, view, 0, 0, md);
														//AddProps(6, view, 0, 0, md);
													}
												}
											}
										}
										if (group->GetName() == cur_mesh_group)
											glbin_current.mesh_group = group;
									}
									view->SetMeshPopDirty();
								}
							}
							break;
							}
						}
					}
				}
			}

			//tracking group
			path = "/views/" + std::to_string(i) + "/track_group";
			if (fconfig->Exists(path))
			{
				if (fconfig->Read("track_file", &wsval))
				{
					view->LoadTrackGroup(wsval);
				}
			}

			//properties
			path = "/views/" + std::to_string(i) + "/properties";
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				if (fconfig->Read("drawall", &bval))
					view->SetDraw(bval);
				//properties
				if (fconfig->Read("persp", &bval))
					view->SetPersp(bval);
				if (fconfig->Read("cam mode", &ival))
					view->SetCamMode(ival);
				if (fconfig->Read("aov", &dval))
					view->SetAov(dval);
				double nearclip;
				double farclip;
				if (fconfig->Read("nearclip", &nearclip) &&
					fconfig->Read("farclip", &farclip))
				{
					view->SetNearClip(nearclip);
					view->SetFarClip(farclip);
					if (glbin_xr_renderer)
						glbin_xr_renderer->SetClips(nearclip, farclip);
				}
				if (fconfig->Read("backgroundcolor", &cval))
					view->SetBackgroundColor(cval);
				if (fconfig->Read("channel_mix_mode", &ival))
					view->SetChannelMixMode(static_cast<ChannelMixMode>(ival));
				if (fconfig->Read("fog", &bval))
					view->SetFog(bval);
				if (fconfig->Read("fogintensity", &dval))
					view->SetFogIntensity(dval);
				if (fconfig->Read("draw_camctr", &bval))
					view->m_draw_camctr = bval;
				if (fconfig->Read("draw_info", &ival))
					view->m_draw_info = ival;
				if (fconfig->Read("draw_legend", &bval))
					view->m_draw_legend = bval;

				//camera
				if (fconfig->Read("translation", &vval))
					view->SetTranslations(vval);
				if (fconfig->Read("rotation", &vval))
						view->SetRotations(vval, false);
				if (fconfig->Read("zero_quat", &qval))
						view->SetZeroQuat(qval);
				if (fconfig->Read("center", &pval))
					view->SetCenters(pval);
				if (fconfig->Read("centereyedist", &dval))
					view->SetCenterEyeDist(dval);
				if (fconfig->Read("radius", &dval, 5.0))
					view->SetRadius(dval);
				if (fconfig->Read("initdist", &dval))
					view->SetInitDist(dval);
				else
					view->SetInitDist(view->GetRadius() / tan(d2r(view->GetAov() / 2.0)));
				if (fconfig->Read("scale_mode", &ival))
					view->m_scale_mode = ival;
				if (fconfig->Read("scale", &dval))
					view->m_scale_factor = dval;
				else
					view->m_scale_factor = view->GetRadius() / tan(d2r(view->GetAov() / 2.0)) / view->GetInitDist();
				if (fconfig->Read("pin_rot_center", &bval))
					view->SetPinRotCenter(bval, false);
				//object
				if (fconfig->Read("obj_center", &pval))
					view->SetObjCenters(pval);
				if (fconfig->Read("obj_trans", &vval))
					view->SetObjTrans(vval);
				if (fconfig->Read("obj_rot", &vval))
				{
					if (l_major <= 2 && d_minor < 24.3)
						vval += fluo::Vector(0, 180, 180);
					view->SetObjRot(vval);
				}
				//colormap
				if (fconfig->Read("colormap_disp", &ival))
					view->m_colormap_disp = ival;
				//scale bar
				if (fconfig->Read("scalebar_disp", &ival))
					view->m_scalebar_disp = ival;
				if (fconfig->Read("sb_length", &dval))
					view->m_sb_length = dval;
				if (fconfig->Read("sb_text", &wsval))
					view->m_sb_text = wsval;
				if (fconfig->Read("sb_num", &wsval))
					view->m_sb_num = wsval;
				if (fconfig->Read("sb_unit", &ival))
					view->m_sb_unit = ival;

				//2d sdjustment settings
				if (fconfig->Read("gamma", &cval))
					view->SetGammaColor(cval);
				if (fconfig->Read("brightness", &cval))
					view->SetBrightness(cval);
				if (fconfig->Read("hdr", &cval))
					view->SetHdr(cval);
				if (fconfig->Read("sync_r", &bval))
					view->SetSync(0, bval);
				if (fconfig->Read("sync_g", &bval))
					view->SetSync(1, bval);
				if (fconfig->Read("sync_b", &bval))
					view->SetSync(2, bval);

				//clipping plane rotations
				if (fconfig->Read("clip_mode", &ival))
					view->SetClipMode(ival);

				//painting parameters
				if (fconfig->Read("brush_use_pres", &dval))
					glbin_vol_selector.SetBrushUsePres(dval);
				double size1, size2;
				if (fconfig->Read("brush_size_1", &size1) &&
					fconfig->Read("brush_size_2", &size2))
					glbin_vol_selector.SetBrushSize(size1, size2);
				if (fconfig->Read("brush_spacing", &dval))
					glbin_vol_selector.SetBrushSpacing(dval);
				if (fconfig->Read("brush_iteration", &dval))
					glbin_vol_selector.SetBrushIteration(dval);
				if (fconfig->Read("brush_size_data", &dval))
					glbin_vol_selector.SetBrushSizeData(dval);
				if (fconfig->Read("brush_translate", &dval))
					glbin_vol_selector.SetBrushSclTranslate(dval);
				if (fconfig->Read("w2d", &dval))
					glbin_vol_selector.SetW2d(dval);
			}

			//rulers
			path = "/views/" + std::to_string(i) + "/rulers";
			if (view->GetRulerList() && fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				ReadRulerList(gstProjectFile, i);
			}
		}
		glbin_current.render_view = root->GetView(cur_canvas);
	}

	//clipping planes
	path = "/prop_panel";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		if (fconfig->Read("clip link", &bval))
			glbin_settings.m_clip_link = bval;
		if (fconfig->Read("clip hold", &bval))
			glbin_settings.m_clip_hold = bval;
		if (fconfig->Read("plane mode", &ival))
			glbin_settings.m_clip_mode = ival;
		if (fconfig->Read("x_link", &bval))
			frame->GetClipPlanePanel()->SetXLink(bval);
		if (fconfig->Read("y_link", &bval))
			frame->GetClipPlanePanel()->SetYLink(bval);
		if (fconfig->Read("z_link", &bval))
			frame->GetClipPlanePanel()->SetZLink(bval);
	}

	//movie panel
	path = "/movie_panel";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);

		//set settings for frame
		std::shared_ptr<RenderView> view;
		if (fconfig->Read("views_cmb", &ival))
		{
			view = root->GetView(ival);
			glbin_moviemaker.SetView(view);
		}
		if (fconfig->Read("rot_check", &bval))
			glbin_moviemaker.SetRotateEnable(bval);
		if (fconfig->Read("seq_mode", &ival))
			glbin_moviemaker.SetSeqMode(ival);
		if (fconfig->Read("x_rd", &bval))
		{
			if (bval)
				glbin_moviemaker.SetRotateAxis(0);
		}
		if (fconfig->Read("y_rd", &bval))
		{
			if (bval)
				glbin_moviemaker.SetRotateAxis(1);
		}
		if (fconfig->Read("z_rd", &bval))
		{
			if (bval)
				glbin_moviemaker.SetRotateAxis(2);
		}
		if (fconfig->Read("rot_axis", &ival))
			glbin_moviemaker.SetRotateAxis(ival);
		if (fconfig->Read("rot_deg", &ival))
			glbin_moviemaker.SetRotateDeg(ival);
		if (fconfig->Read("movie_len", &dval))
			glbin_moviemaker.SetMovieLength(dval);
		if (fconfig->Read("fps", &dval))
			glbin_moviemaker.SetFps(dval);
		if (fconfig->Read("crop", &bval))
			glbin_moviemaker.SetCropEnable(bval);
		if (fconfig->Read("crop_x", &ival))
			glbin_moviemaker.SetCropX(ival);
		if (fconfig->Read("crop_y", &ival))
			glbin_moviemaker.SetCropY(ival);
		if (fconfig->Read("crop_w", &ival))
			glbin_moviemaker.SetCropW(ival);
		if (fconfig->Read("crop_h", &ival))
			glbin_moviemaker.SetCropH(ival);
		if (fconfig->Read("full frame num", &ival))
			glbin_moviemaker.SetFullFrameNum(ival);
		int startf = 0, endf = 0, curf = 0;
		if (fconfig->Read("start_frame", &startf) &&
			fconfig->Read("end_frame", &endf))
			glbin_moviemaker.SetClipStartEndFrames(startf, endf);
		if (fconfig->Read("cur_frame", &curf))
		{
			if (curf && curf >= startf && curf <= endf)
			{
				glbin_moviemaker.SetCurrentFrameSilently(curf, false);
				auto view = root->GetLastView();
				if (view)
				{
					view->Set4DSeqFrame(curf, startf, endf, false);
				}
			}
		}
		if (fconfig->Read("key frame enable", &bval))
			glbin_moviemaker.SetKeyframeEnable(bval, false);
		if (fconfig->Read("run_script", &bval))
			glbin_settings.m_run_script = bval;
		if (fconfig->Read("script_file", &wsval))
			glbin_settings.m_script_file = wsval;
		//frame->GetMoviePanel()->FluoUpdate();
	}

	//ui layout
	path = "/ui_layout";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		bool update = false;
		if (fconfig->Read("dpi scale factor", &dval))
			update = fluo::InEpsilon(dval, frame->GetDPIScaleFactor());
		if (update && fconfig->Read("layout", &sval))
			frame->LoadPerspective(sval);
		if (fconfig->Read("layout clip", &sval))
			frame->GetClipPlanePanel()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout movie", &sval))
			frame->GetMoviePanel()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout outadj", &sval))
			frame->GetOutAdjPanel()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout project", &sval))
			frame->GetProjectPanel()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout brush", &sval))
			frame->GetBrushToolDlg()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout component", &sval))
			frame->GetComponentDlg()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout machine learning", &sval))
			frame->GetMachineLearningDlg()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout measure", &sval))
			frame->GetMeasureDlg()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout track", &sval))
			frame->GetTrackDlg()->LoadPerspective(fconfig->DecodeXml(sval));
		if (fconfig->Read("layout measure", &sval))
			frame->GetSettingDlg()->LoadPerspective(fconfig->DecodeXml(sval));
	}

	//interpolator
	path = "/interpolator";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);
		glbin_interpolator.Clear();
		if (fconfig->Read("max_id", &ival))
			Interpolator::m_id = ival;
		std::vector<FlKeyGroup*>* key_list = glbin_interpolator.GetKeyList();
		int group_num = 0;
		fconfig->Read("num", &group_num);
		for (int i = 0; i < group_num; i++)
		{
			path = "/interpolator/" + std::to_string(i);
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				FlKeyGroup* key_group = new FlKeyGroup;
				if (fconfig->Read("id", &ival))
					key_group->id = ival;
				if (fconfig->Read("t", &dval))
					key_group->t = dval;
				if (fconfig->Read("dt", &dval))
					key_group->dt = dval;
				else
				{
					if (key_list->empty())
						key_group->dt = 0;
					else
						key_group->dt = key_group->t - key_list->back()->t;
				}
				if (fconfig->Read("type", &ival))
					key_group->type = ival;
				if (fconfig->Read("desc", &wsval))
					key_group->desc = wsval;
				path = "/interpolator/" + std::to_string(i) + "/keys";
				if (fconfig->Exists(path))
				{
					fconfig->SetPath(path);
					size_t key_num = 0;
					fconfig->Read("num", &key_num);
					for (size_t j = 0; j < key_num; j++)
					{
						path = "/interpolator/" + std::to_string(i) + "/keys/" + std::to_string(j);
						if (fconfig->Exists(path))
						{
							fconfig->SetPath(path);
							int key_type;
							if (fconfig->Read("type", &key_type))
							{
								FlKeyCode code;
								if (fconfig->Read("l0", &ival))
									code.l0 = ival;
								if (fconfig->Read("l0_name", &sval))
									code.l0_name = sval;
								if (fconfig->Read("l1", &ival))
									code.l1 = ival;
								if (fconfig->Read("l1_name", &sval))
									code.l1_name = sval;
								if (fconfig->Read("l2", &ival))
									code.l2 = ival;
								if (fconfig->Read("l2_name", &sval))
									code.l2_name = sval;
								switch (key_type)
								{
								case FLKEY_TYPE_DOUBLE:
								{
									if (fconfig->Read("val", &dval))
									{
										FlKeyDouble* key = new FlKeyDouble(code, dval);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_QUATER:
								{
									if (fconfig->Read("val", &qval))
									{
										FlKeyQuaternion* key = new FlKeyQuaternion(code, qval);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_BOOLEAN:
								{
									if (fconfig->Read("val", &bval))
									{
										FlKeyBoolean* key = new FlKeyBoolean(code, bval);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_INT:
								{
									if (fconfig->Read("val", &ival))
									{
										FlKeyInt* key = new FlKeyInt(code, ival);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_COLOR:
								{
									if (fconfig->Read("val", &cval))
									{
										FlKeyColor* key = new FlKeyColor(code, cval);
										key_group->keys.push_back(key);
									}
								}
								break;
								}
							}
						}
					}
				}
				key_list->push_back(key_group);
			}
		}
	}

	SetProgress(0, "");

	glbin_refresh_scheduler_manager.requestDraw(DrawRequest("Open project refresh"));
	frame->UpdateProps({}, 0, 0);
	frame->GetTreePanel()->Select();//simulate selection
}

void Project::Save(const std::wstring& filename, bool inc)
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
		return;
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;

	std::wstring filename2 = filename;
	if (inc)
		filename2 = INC_NUM_EXIST(filename);

	std::shared_ptr<BaseTreeFile> fconfig =
		glbin_tree_file_factory.createTreeFile(glbin_settings.m_config_file_type, gstProjectFile);
	if (!fconfig)
		return;

	fconfig->Write("ver_major", std::string(VERSION_MAJOR_TAG));
	fconfig->Write("ver_minor", std::string(VERSION_MINOR_TAG));

	int ticks = glbin_data_manager.GetVolumeNum() + glbin_data_manager.GetMeshNum();
	ticks = ticks ? ticks : 1;
	int tick_cnt = 1;
	fconfig->Write("ticks", ticks);
	SetProgress(0, "FluoRender is saving the project file. Please wait.");

	std::string path;
	std::wstring wsval;

	path = "/settings";
	fconfig->SetPath(path);
	//save streaming mode
	fconfig->Write("interactive quality", glbin_settings.m_interactive_quality);
	fconfig->Write("stream rendering", glbin_settings.m_stream_rendering);
	fconfig->Write("graphics mem", glbin_settings.m_graphics_mem);
	fconfig->Write("large data size", glbin_settings.m_large_data_size);
	fconfig->Write("force brick size", glbin_settings.m_force_brick_size);
	fconfig->Write("up time", glbin_settings.m_up_time);
	fconfig->Write("update order", glbin_settings.m_update_order);
	fconfig->Write("inf loop", glbin_settings.m_inf_loop);
	//save peeling layers
	fconfig->Write("peeling layers", glbin_settings.m_peeling_layers);

	path = "/current";
	fconfig->SetPath(path);
	auto view = glbin_current.render_view.lock();
	wsval = view ? view->GetName() : L"";
	fconfig->Write("canvas", wsval);
	auto vol_group = glbin_current.vol_group.lock();
	wsval = vol_group ? vol_group->GetName() : L"";
	fconfig->Write("vol group", wsval);
	auto mesh_group = glbin_current.mesh_group.lock();
	wsval = mesh_group ? mesh_group->GetName() : L"";
	fconfig->Write("mesh group", wsval);
	auto vol_data = glbin_current.vol_data.lock();
	wsval = vol_data ? vol_data->GetName() : L"";
	fconfig->Write("vol data", wsval);
	auto mesh_data = glbin_current.mesh_data.lock();
	wsval = mesh_data ? mesh_data->GetName() : L"";
	fconfig->Write("mesh data", wsval);
	auto ann_data = glbin_current.ann_data.lock();
	wsval = ann_data ? ann_data->GetName() : L"";
	fconfig->Write("ann data", wsval);

	//save data list
	//volume
	path = "/data/volume";
	fconfig->SetPath(path);
	size_t num = glbin_data_manager.GetVolumeNum();
	fconfig->Write("num", num);
	for (size_t i = 0; i < num; i++)
	{
		SetProgress(100 * tick_cnt / ticks,
			"FluoRender is saving volume data. Please wait.");
		tick_cnt++;

		auto vd = glbin_data_manager.GetVolumeData(i);
		if (vd)
		{
			path = "/data/volume/" + std::to_string(i);
			//name
			fconfig->SetPath(path);
			fconfig->Write("name", vd->GetName());
			//compression
			fconfig->Write("compression", glbin_settings.m_realtime_compress);
			//skip brick
			fconfig->Write("skip_brick", vd->GetSkipBrick());
			//path
			std::wstring filepath = vd->GetPath();
			bool new_chan = false;
			if (filepath.empty() || glbin_settings.m_vrp_embed)
			{
				std::wstring new_folder;
				new_folder = filename2 + L"_files";
				MkDirW(new_folder);
				std::filesystem::path p(new_folder);
				p /= vd->GetName() + L".tif";
				vd->Save(p.wstring(), 0, 3, false,
					false, 0, false, glbin_settings.m_save_compress,
					fluo::Point(), fluo::Quaternion(), fluo::Vector(), false);
				fconfig->Write("path", p.wstring());
				new_chan = true;
			}
			else
				fconfig->Write("path", filepath);
			auto reader = vd->GetReader();
			if (reader)
			{
				//reader type
				fconfig->Write("reader_type", reader->GetType());
				fconfig->Write("slice_seq", reader->GetSliceSeq());
				fconfig->Write("time_id", reader->GetTimeId());
				//float convert
				fconfig->Write("fp_convert", reader->GetFpConvert());
				double minv, maxv;
				reader->GetFpRange(minv, maxv);
				fconfig->Write("fp_min", minv);
				fconfig->Write("fp_max", maxv);
			}
			else
			{
				fconfig->Write("slice_seq", false);
				fconfig->Write("time_id", std::wstring(L""));
			}
			fconfig->Write("cur_time", vd->GetCurTime());
			fconfig->Write("cur_chan", new_chan ? 0 : vd->GetCurChannel());

			//volume properties
			path = "properties";
			fconfig->SetPath(path);
			fconfig->Write("display", vd->GetDisp());

			//properties
			fconfig->Write("3dgamma enable", vd->GetGammaEnable());
			fconfig->Write("3dgamma", vd->GetGamma());
			fconfig->Write("boundary enable", vd->GetBoundaryEnable());
			fconfig->Write("boundary low", vd->GetBoundaryLow());
			fconfig->Write("boundary high", vd->GetBoundaryHigh());
			fconfig->Write("boundary max", vd->GetBoundaryMax());
			fconfig->Write("minmax enable", vd->GetMinMaxEnable());
			fconfig->Write("low offset", vd->GetLowOffset());
			fconfig->Write("high offset", vd->GetHighOffset());
			fconfig->Write("threshold enable", vd->GetThreshEnable());
			fconfig->Write("left_thresh", vd->GetLeftThresh());
			fconfig->Write("right_thresh", vd->GetRightThresh());
			fconfig->Write("luminance enable", vd->GetLuminanceEnable());
			fconfig->Write("luminance", vd->GetLuminance());
			fconfig->Write("color", vd->GetColor());
			fconfig->Write("mask_color", vd->GetMaskColor());
			fconfig->Write("mask_color_set", vd->GetMaskColorSet());
			fconfig->Write("enable_alpha", vd->GetAlphaEnable());
			fconfig->Write("alpha", vd->GetAlpha());
			fconfig->Write("shading", vd->GetShadingEnable());
			fconfig->Write("shading_strength", vd->GetShadingStrength());
			fconfig->Write("shading_shine", vd->GetShadingShine());
			fconfig->Write("samplerate", vd->GetSampleRate());

			//resolution scale
			fconfig->Write("res", vd->GetSpacing());
			fconfig->Write("b_res", vd->GetBaseSpacing());
			fconfig->Write("s_res", vd->GetSpacingScale());
			fconfig->Write("scl", vd->GetScaling());

			auto& cb = vd->GetClippingBox();
			//clip values
			double clips[6];
			cb.GetAllClipsWorld(clips);
			fconfig->Write("clip_xneg", clips[0]);
			fconfig->Write("clip_xpos", clips[1]);
			fconfig->Write("clip_yneg", clips[2]);
			fconfig->Write("clip_ypos", clips[3]);
			fconfig->Write("clip_zneg", clips[4]);
			fconfig->Write("clip_zpos", clips[5]);
			//clip rotation
			auto euler = cb.GetEuler();
			fconfig->Write("clip_rot", euler);
			//clip link
			fconfig->Write("clip_link_x", cb.GetLink(fluo::ClipPlane::XNeg));
			fconfig->Write("clip_link_y", cb.GetLink(fluo::ClipPlane::YNeg));
			fconfig->Write("clip_link_z", cb.GetLink(fluo::ClipPlane::ZNeg));
			//dist
			fconfig->Write("clip_dist_x", cb.GetLinkedDistWorld(fluo::ClipPlane::XNeg));
			fconfig->Write("clip_dist_y", cb.GetLinkedDistWorld(fluo::ClipPlane::YNeg));
			fconfig->Write("clip_dist_z", cb.GetLinkedDistWorld(fluo::ClipPlane::ZNeg));

			//2d adjustment settings
			fconfig->Write("gamma", vd->GetGammaColor());
			fconfig->Write("brightness", vd->GetBrightness());
			fconfig->Write("hdr", vd->GetHdr());
			fconfig->Write("sync_r", vd->GetSync(0));
			fconfig->Write("sync_g", vd->GetSync(1));
			fconfig->Write("sync_b", vd->GetSync(2));

			//colormap settings
			fconfig->Write("colormap_mode", static_cast<int>(vd->GetColorMode()));
			fconfig->Write("colormap_inv", vd->GetColormapInv());
			fconfig->Write("colormap", vd->GetColormap());
			fconfig->Write("colormap_proj", static_cast<int>(vd->GetColormapProj()));
			double low, high;
			vd->GetColormapValues(low, high);
			fconfig->Write("colormap_lo_value", low);
			fconfig->Write("colormap_hi_value", high);

			//high transp
			fconfig->Write("alpha_power", vd->GetAlphaPower());
			//inversion
			fconfig->Write("inv", vd->GetInvert());
			//mip enable
			fconfig->Write("mode", static_cast<int>(vd->GetRenderMode()));
			//noise reduction
			fconfig->Write("noise_red", vd->GetNR());
			//depth override
			fconfig->Write("channel_mix_mode", static_cast<int>(vd->GetChannelMixMode()));

			//shadow
			fconfig->Write("shadow", vd->GetShadowEnable());
			//shadow intensity
			fconfig->Write("shadow_darkness", vd->GetShadowIntensity());

			//legend
			fconfig->Write("legend", vd->GetLegend());

			//mask
			vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
			vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
		}
	}
	//mesh
	fconfig->SetPath("/data/mesh");
	num = glbin_data_manager.GetMeshNum();
	fconfig->Write("num", num);
	for (size_t i = 0; i < num; i++)
	{
		SetProgress(100 * tick_cnt / ticks,
			"FluoRender is saving mesh data. Please wait.");
		tick_cnt++;

		auto md = glbin_data_manager.GetMeshData(i);
		if (md)
		{
			if (md->GetPath() == L"" || glbin_settings.m_vrp_embed)
			{
				std::wstring new_folder;
				new_folder = filename2 + L"_files";
				MkDirW(new_folder);
				std::filesystem::path p(new_folder);
				p /= md->GetName() + L".obj";
				md->Save(p.wstring());
			}
			path = "/data/mesh/" + std::to_string(i);
			fconfig->SetPath(path);
			fconfig->Write("name", md->GetName());
			fconfig->Write("path", md->GetPath());
			//mesh prperties
			fconfig->SetPath("properties");
			fconfig->Write("display", md->GetDisp());
			//lighting
			fconfig->Write("shading", md->GetShading());
			fconfig->Write("shading_strength", md->GetShadingStrength());
			fconfig->Write("shininess", md->GetShadingShine());
			fconfig->Write("alpha", md->GetAlpha());
			fconfig->Write("color", md->GetColor());
			//2d adjustment settings
			fconfig->Write("gamma", md->GetGammaColor());
			fconfig->Write("brightness", md->GetBrightness());
			fconfig->Write("hdr", md->GetHdr());
			fconfig->Write("sync_r", md->GetSync(0));
			fconfig->Write("sync_g", md->GetSync(1));
			fconfig->Write("sync_b", md->GetSync(2));
			//shadow
			fconfig->Write("shadow", md->GetShadowEnable());
			fconfig->Write("shadow_darkness", md->GetShadowIntensity());

			auto& cb = md->GetClippingBox();
			//clip values
			double clips[6];
			cb.GetAllClipsWorld(clips);
			fconfig->Write("clip_xneg", clips[0]);
			fconfig->Write("clip_xpos", clips[1]);
			fconfig->Write("clip_yneg", clips[2]);
			fconfig->Write("clip_ypos", clips[3]);
			fconfig->Write("clip_zneg", clips[4]);
			fconfig->Write("clip_zpos", clips[5]);
			//clip rotation
			auto euler = cb.GetEuler();
			fconfig->Write("clip_rot", euler);
			//clip link
			fconfig->Write("clip_link_x", cb.GetLink(fluo::ClipPlane::XNeg));
			fconfig->Write("clip_link_y", cb.GetLink(fluo::ClipPlane::YNeg));
			fconfig->Write("clip_link_z", cb.GetLink(fluo::ClipPlane::ZNeg));
			//dist
			fconfig->Write("clip_dist_x", cb.GetLinkedDistWorld(fluo::ClipPlane::XNeg));
			fconfig->Write("clip_dist_y", cb.GetLinkedDistWorld(fluo::ClipPlane::YNeg));
			fconfig->Write("clip_dist_z", cb.GetLinkedDistWorld(fluo::ClipPlane::ZNeg));

			//mesh transform
			path = "../transform";
			fconfig->SetPath(path);
			fconfig->Write("translation", md->GetTranslation());
			fconfig->Write("rotation", md->GetRotation());
			fconfig->Write("scaling", md->GetScaling());
		}
	}
	//annotations
	path = "/data/annotations";
	fconfig->SetPath(path);
	num = glbin_data_manager.GetAnnotNum();
	fconfig->Write("num", num);
	for (size_t i = 0; i < num; i++)
	{
		auto ann = glbin_data_manager.GetAnnotData(i);
		if (ann)
		{
			if (ann->GetPath() == L"")
			{
				std::wstring new_folder;
				new_folder = filename2 + L"_files";
				MkDirW(new_folder);
				std::filesystem::path p(new_folder);
				p /= ann->GetName() + L".txt";
				ann->Save(p.wstring());
			}
			path = "/data/annotations/" + std::to_string(i);
			fconfig->SetPath(path);
			fconfig->Write("name", ann->GetName());
			fconfig->Write("path", ann->GetPath());
		}
	}
	//views
	path = "/views";
	fconfig->SetPath(path);
	num = static_cast<size_t>(root->GetViewNum());
	fconfig->Write("num", num);
	for (size_t i = 0; i < num; i++)
	{
		auto view = root->GetView(static_cast<int>(i));
		if (view)
		{
			//view layers
			path = "/views/" + std::to_string(i) + "/layers";
			fconfig->SetPath(path);
			int layer_num = view->GetLayerNum();
			fconfig->Write("num", layer_num);
			for (int j = 0; j < layer_num; j++)
			{
				auto layer = view->GetLayer(j);
				if (!layer)
					continue;
				path = "/views/" + std::to_string(i) + "/layers/" + std::to_string(j);
				fconfig->SetPath(path);
				switch (layer->IsA())
				{
				case 2://volume data
					fconfig->Write("type", 2);
					fconfig->Write("name", layer->GetName());
					break;
				case 3://mesh data
					fconfig->Write("type", 3);
					fconfig->Write("name", layer->GetName());
					break;
				case 4://annotations
					fconfig->Write("type", 4);
					fconfig->Write("name", layer->GetName());
					break;
				case 5://group
				{
					auto group = std::dynamic_pointer_cast<VolumeGroup>(layer);
					if (!group)
						break;

					fconfig->Write("type", 5);
					fconfig->Write("name", layer->GetName());
					fconfig->Write("id", VolumeGroup::GetID());
					//dispaly
					fconfig->Write("display", group->GetDisp());
					//2d adjustment
					fconfig->Write("gamma", group->GetGammaColor());
					fconfig->Write("brightness", group->GetBrightness());
					fconfig->Write("hdr", group->GetHdr());
					fconfig->Write("sync_r", group->GetSync(0));
					fconfig->Write("sync_g", group->GetSync(1));
					fconfig->Write("sync_b", group->GetSync(2));
					//sync volume properties
					fconfig->Write("sync_vp", group->GetVolumeSyncProp());
					//volumes
					path = "/views/" + std::to_string(i) + "/layers/" + std::to_string(j) + "/volumes";
					fconfig->SetPath(path);
					int vol_num = group->GetVolumeNum();
					fconfig->Write("num", vol_num);
					for (int k = 0; k < vol_num; k++)
						fconfig->Write("vol_" + std::to_string(k), group->GetVolumeData(k)->GetName());

				}
				break;
				case 6://mesh group
				{
					auto group = std::dynamic_pointer_cast<MeshGroup>(layer);
					if (!group)
						break;

					fconfig->Write("type", 6);
					fconfig->Write("name", layer->GetName());
					fconfig->Write("id", MeshGroup::GetID());
					//display
					fconfig->Write("display", group->GetDisp());
					//sync mesh properties
					fconfig->Write("sync_mp", group->GetMeshSyncProp());
					//meshes
					path = "/views/" + std::to_string(i) + "/layers/" + std::to_string(j) + "/meshes";
					fconfig->SetPath(path);
					int mesh_num = group->GetMeshNum();
					fconfig->Write("num", mesh_num);
					for (int k = 0; k < mesh_num; k++)
						fconfig->Write("mesh_" + std::to_string(k), group->GetMeshData(k)->GetName());
				}
				break;
				}
			}

			//tracking group
			path = "/views/" + std::to_string(i) + "/track_group";
			fconfig->SetPath(path);
			int ival = view->GetTrackFileExist(true);
			if (ival == 1)
			{
				std::wstring new_folder;
				new_folder = filename2 + L"_files";
				MkDirW(new_folder);
				std::filesystem::path p(new_folder);
				p /= GET_STEM(filename2) + L".track";
				view->SaveTrackGroup(p.wstring());
			}
			fconfig->Write("track_file",view->GetTrackGroupFile());

			//properties
			path = "/views/" + std::to_string(i) + "/properties";
			fconfig->SetPath(path);
			fconfig->Write("drawall", view->GetDraw());
			fconfig->Write("persp", view->GetPersp());
			fconfig->Write("cam mode", view->GetCamMode());
			fconfig->Write("aov", view->GetAov());
			fconfig->Write("nearclip", view->GetNearClip());
			fconfig->Write("farclip", view->GetFarClip());
			fconfig->Write("backgroundcolor", view->GetBackgroundColor());
			fconfig->Write("drawtype", view->GetDrawType());
			fconfig->Write("channel_mix_mode", static_cast<int>(view->GetChannelMixMode()));
			fconfig->Write("fog", view->GetFog());
			fconfig->Write("fogintensity", view->GetFogIntensity());
			fconfig->Write("draw_camctr", view->m_draw_camctr);
			fconfig->Write("draw_info", view->m_draw_info);
			fconfig->Write("draw_legend", view->m_draw_legend);

			//camera
			fconfig->Write("translation", view->GetTranslations());
			fconfig->Write("rotation", view->GetRotations());
			fconfig->Write("zero_quat", view->GetZeroQuat());
			fconfig->Write("center", view->GetCenters());
			fconfig->Write("centereyedist", view->GetCenterEyeDist());
			fconfig->Write("radius", view->GetRadius());
			fconfig->Write("initdist", view->GetInitDist());
			fconfig->Write("scale_mode", view->m_scale_mode);
			fconfig->Write("scale", view->m_scale_factor);
			fconfig->Write("pin_rot_center", view->m_pin_rot_ctr);
			//object
			fconfig->Write("obj_center", view->GetObjCenters());
			fconfig->Write("obj_trans", view->GetObjTrans());
			fconfig->Write("obj_rot", view->GetObjRot());
			//colormap
			fconfig->Write("colormap_disp", view->m_colormap_disp);
			//scalebar
			fconfig->Write("scalebar_disp", view->m_scalebar_disp);
			fconfig->Write("sb_length", view->m_sb_length);
			fconfig->Write("sb_text", view->m_sb_text);
			fconfig->Write("sb_num", view->m_sb_num);
			fconfig->Write("sb_unit", view->m_sb_unit);

			//2d adjustment
			fconfig->Write("gamma", view->GetGammaColor());
			fconfig->Write("brightness", view->GetBrightness());
			fconfig->Write("hdr", view->GetHdr());
			fconfig->Write("sync_r", view->GetSync(0));
			fconfig->Write("sync_g", view->GetSync(1));
			fconfig->Write("sync_b", view->GetSync(2));

			//clipping plane rotations
			fconfig->Write("clip_mode", view->GetClipMode());

			//painting parameters
			fconfig->Write("brush_use_pres", glbin_vol_selector.GetBrushUsePres());
			fconfig->Write("brush_size_1", glbin_vol_selector.GetBrushSize1());
			fconfig->Write("brush_size_2", glbin_vol_selector.GetBrushSize2());
			fconfig->Write("brush_spacing", glbin_vol_selector.GetBrushSpacing());
			fconfig->Write("brush_iteration", glbin_vol_selector.GetBrushIteration());
			fconfig->Write("brush_translate", glbin_vol_selector.GetBrushSclTranslate());
			fconfig->Write("w2d", glbin_vol_selector.GetW2d());

			//rulers
			path = "/views/" + std::to_string(i) + "/rulers";
			fconfig->SetPath(path);
			SaveRulerList(gstProjectFile, i);
		}
	}
	//clipping planes
	path = "/prop_panel";
	fconfig->SetPath(path);
	fconfig->Write("clip link", glbin_settings.m_clip_link);
	fconfig->Write("clip hold", glbin_settings.m_clip_hold);
	fconfig->Write("clip mode", glbin_settings.m_clip_mode);
	fconfig->Write("x_link", frame->GetClipPlanePanel()->GetXLink());
	fconfig->Write("y_link", frame->GetClipPlanePanel()->GetYLink());
	fconfig->Write("z_link", frame->GetClipPlanePanel()->GetZLink());
	//movie view
	path = "/movie_panel";
	fconfig->SetPath(path);
	fconfig->Write("key frame enable", glbin_moviemaker.GetKeyframeEnable());
	fconfig->Write("views_cmb", glbin_moviemaker.GetViewIndex());
	fconfig->Write("rot_check", glbin_moviemaker.GetRotateEnable());
	fconfig->Write("seq_mode", glbin_moviemaker.GetSeqMode());
	fconfig->Write("rot_axis", glbin_moviemaker.GetRotateAxis());
	fconfig->Write("rot_deg", glbin_moviemaker.GetRotateDeg());
	fconfig->Write("movie_len", glbin_moviemaker.GetMovieLength());
	fconfig->Write("fps", glbin_moviemaker.GetFps());
	fconfig->Write("crop", glbin_moviemaker.GetCropEnable());
	fconfig->Write("crop_x", glbin_moviemaker.GetCropX());
	fconfig->Write("crop_y", glbin_moviemaker.GetCropY());
	fconfig->Write("crop_w", glbin_moviemaker.GetCropW());
	fconfig->Write("crop_h", glbin_moviemaker.GetCropH());
	fconfig->Write("cur_frame", glbin_moviemaker.GetCurrentFrame());
	fconfig->Write("full frame num", glbin_moviemaker.GetFullFrameNum());
	fconfig->Write("start_frame", glbin_moviemaker.GetClipStartFrame());
	fconfig->Write("end_frame", glbin_moviemaker.GetClipEndFrame());
	fconfig->Write("run_script", glbin_settings.m_run_script);
	fconfig->Write("script_file", glbin_settings.m_script_file);
	//layout
	path = "/ui_layout";
	fconfig->SetPath(path);
	fconfig->Write("dpi scale factor", frame->GetDPIScaleFactor());
	fconfig->Write("layout", frame->SavePerspective().ToStdString());
	fconfig->Write("layout clip", fconfig->EncodeXml(frame->GetClipPlanePanel()->SavePerspective()));
	fconfig->Write("layout movie", fconfig->EncodeXml(frame->GetMoviePanel()->SavePerspective()));
	fconfig->Write("layout outadj", fconfig->EncodeXml(frame->GetOutAdjPanel()->SavePerspective()));
	fconfig->Write("layout project", fconfig->EncodeXml(frame->GetProjectPanel()->SavePerspective()));
	fconfig->Write("layout brush", fconfig->EncodeXml(frame->GetBrushToolDlg()->SavePerspective()));
	fconfig->Write("layout component", fconfig->EncodeXml(frame->GetComponentDlg()->SavePerspective()));
	fconfig->Write("layout machine learning", fconfig->EncodeXml(frame->GetMachineLearningDlg()->SavePerspective()));
	fconfig->Write("layout measure", fconfig->EncodeXml(frame->GetMeasureDlg()->SavePerspective()));
	fconfig->Write("layout settings", fconfig->EncodeXml(frame->GetSettingDlg()->SavePerspective()));
	fconfig->Write("layout track", fconfig->EncodeXml(frame->GetTrackDlg()->SavePerspective()));
	//interpolator
	path = "/interpolator";
	fconfig->SetPath(path);
	fconfig->Write("max_id", Interpolator::m_id);
	int group_num = glbin_interpolator.GetKeyNum();
	fconfig->Write("num", group_num);
	for (int i = 0; i < group_num; i++)
	{
		FlKeyGroup* key_group = glbin_interpolator.GetKeyGroup(i);
		if (key_group)
		{
			path = "/interpolator/" + std::to_string(i);
			fconfig->SetPath(path);
			fconfig->Write("id", key_group->id);
			fconfig->Write("t", key_group->t);
			fconfig->Write("dt", key_group->dt);
			fconfig->Write("type", key_group->type);
			fconfig->Write("desc", key_group->desc);
			size_t key_num = (int)key_group->keys.size();
			path = "/interpolator/" + std::to_string(i) + "/keys";
			fconfig->SetPath(path);
			fconfig->Write("num", key_num);
			for (size_t j = 0; j < key_num; j++)
			{
				FlKey* key = key_group->keys[j];
				if (key)
				{
					path = "/interpolator/" + std::to_string(i) + "/keys/" + std::to_string(j);
					fconfig->SetPath(path);
					fconfig->Write("type", key->GetType());
					FlKeyCode code = key->GetKeyCode();
					fconfig->Write("l0", code.l0);
					fconfig->Write("l0_name", code.l0_name);
					fconfig->Write("l1", code.l1);
					fconfig->Write("l1_name", code.l1_name);
					fconfig->Write("l2", code.l2);
					fconfig->Write("l2_name", code.l2_name);
					switch (key->GetType())
					{
					case FLKEY_TYPE_DOUBLE:
						fconfig->Write("val", ((FlKeyDouble*)key)->GetValue());
						break;
					case FLKEY_TYPE_QUATER:
						fconfig->Write("val", ((FlKeyQuaternion*)key)->GetValue());
						break;
					case FLKEY_TYPE_BOOLEAN:
						fconfig->Write("val", ((FlKeyBoolean*)key)->GetValue());
						break;
					case FLKEY_TYPE_INT:
						fconfig->Write("val", ((FlKeyInt*)key)->GetValue());
						break;
					case FLKEY_TYPE_COLOR:
						fconfig->Write("val", ((FlKeyColor*)key)->GetValue());
						break;
					}
				}
			}
		}
	}

	fconfig->SaveFile(filename2);

	SetProgress(0, "");
	glbin_data_manager.SetProjectPath(filename2);

	frame->UpdateProps({ gstListCtrl });
}

void Project::Reset()
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
		return;
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;

	glbin_data_manager.SetProjectPath(L"");
	//SetTitle(m_title);
	//clear
	glbin_data_manager.ClearAll();
	VolumeGroup::ResetID();
	MeshGroup::ResetID();
	root->GetView(0)->ClearAll();
	for (int i = root->GetViewNum() - 1; i > 0; i--)
		frame->DeleteRenderViewPanel(i);
	RenderViewPanel::ResetID();
	glbin_current.SetRoot();
	glbin_moviemaker.Stop();
	glbin_moviemaker.SetView(root->GetView(0));
	glbin_mov_def.Apply(&glbin_moviemaker);
	glbin_interpolator.Clear();
	glbin_volume_point.SetVolumeData(0);
	glbin_comp_analyzer.ClearCompGroup();
}

void Project::ExportRulerList(const std::wstring& filename)
{
	flrd::RulerList* list = glbin_current.GetRulerList();
	auto view = glbin_current.render_view.lock();
	if (!list || !view)
	{
		return;
	}

	std::wofstream os;
	OutputStreamOpenW(os, filename);

	std::wstring str;
	std::wstring unit;
	int num_points;
	fluo::Point p;
	flrd::Ruler* ruler;
	switch (view->m_sb_unit)
	{
	case 0:
		unit = L"nm";
		break;
	case 1:
	default:
		unit = L"\u03BCm";
		break;
	case 2:
		unit = L"mm";
		break;
	}

	int ruler_num = list->size();
	std::vector<unsigned int> groups;
	std::vector<int> counts;
	int group_num = list->GetGroupNumAndCount(groups, counts);
	std::vector<int> group_count(group_num, 0);

	if (ruler_num > 1)
		os << "Ruler Count:\t" << ruler_num << "\n";
	if (group_num > 1)
	{
		//group count
		os << "Group Count:\t" << group_num << "\n";
		for (int i = 0; i < group_num; ++i)
		{
			os << "Group " << groups[i];
			if (i < group_num - 1)
				os << "\t";
			else
				os << "\n";
		}
		for (int i = 0; i < group_num; ++i)
		{
			os << counts[i];
			if (i < group_num - 1)
				os << "\t";
			else
				os << "\n";
		}
	}

	os << "Name\tGroup\tCount\tColor\tBranch\tLength(" << unit << ")\tAngle/Pitch(Deg)\tx1\ty1\tz1\txn\tyn\tzn\tTime\tv1\tv2\n";

	double f = 0.0;
	fluo::Color color;
	for (size_t i = 0; i < list->size(); i++)
	{
		//for each ruler
		ruler = (*list)[i];
		if (!ruler)
			continue;
		ruler->SetWorkTime(view->m_tseq_cur_num);

		os << ruler->GetName() << "\t";

		//group and count
		unsigned int group = ruler->Group();
		os << group << "\t";
		int count = 0;
		auto iter = std::find(groups.begin(), groups.end(), group);
		if (iter != groups.end())
		{
			int index = std::distance(groups.begin(), iter);
			count = ++group_count[index];
		}
		os << count << "\t";

		//color
		if (ruler->GetUseColor())
		{
			color = ruler->GetColor();
			str = L"RGB(" +
				std::to_wstring(int(std::round(color.r() * 255))) + L", " +
				std::to_wstring(int(std::round(color.g() * 255))) + L", " +
				std::to_wstring(int(std::round(color.b() * 255))) + L")";
		}
		else
			str = L"N/A";
		os << str << "\t";

		//branch count
		str = std::to_wstring(ruler->GetNumBranch());
		os << str << "\t";
		//length
		str = std::to_wstring(ruler->GetLength());
		os << str << "\t";
		//angle
		str = std::to_wstring(ruler->GetAngle());
		os << str << "\t";

		str = L"";
		//start and end points
		num_points = ruler->GetNumPoint();
		if (num_points > 0)
		{
			p = ruler->GetPoint(0);
			str += s2ws(p.to_string());
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1);
			str += s2ws(p.to_string());
		}
		else
			str += L"\t\t\t";
		os << str;

		//time
		if (ruler->GetTransient())
			str = std::to_wstring(ruler->GetTransTime());
		else
			str = L"N/A";
		os << str << "\t";

		//info values v1 v2
		os << s2ws(ruler->GetInfoValues()) << "\n";

		//export points
		if (ruler->GetNumPoint() > 2)
		{
			os << s2ws(ruler->GetPosNames());
			os << s2ws(ruler->GetPosValues());
		}

		//export profile
		std::vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (profile && profile->size())
		{
			double sumd = 0.0;
			unsigned long long sumull = 0;
			os << s2ws(ruler->GetInfoProfile()) << "\n";
			for (size_t j = 0; j < profile->size(); ++j)
			{
				//for each profile
				int pixels = (*profile)[j].m_pixels;
				if (pixels <= 0)
					os << "0.0\t";
				else
				{
					os << (*profile)[j].m_accum / pixels << "\t";
					sumd += (*profile)[j].m_accum;
					sumull += pixels;
				}
			}
			//if (m_ruler_df_f)
			//{
			//	double avg = 0.0;
			//	if (sumull != 0)
			//		avg = sumd / double(sumull);
			//	if (i == 0)
			//	{
			//		f = avg;
			//		os << "\t" << f << "\t";
			//	}
			//	else
			//	{
			//		double df = avg - f;
			//		if (f == 0.0)
			//			os << "\t" << df << "\t";
			//		else
			//			os << "\t" << df / f << "\t";
			//	}
			//}
			os << "\n";
		}
	}

	os.close();
}

void Project::SaveRulerList(const std::string &gst_name, int view_index)
{
	flrd::RulerList* list = glbin_current.GetRulerList();
	if (!list || list->empty())
		return;

	std::shared_ptr<BaseTreeFile> fconfig = glbin_tree_file_factory.getTreeFile(gst_name);
	if (!fconfig)
		return;

	std::string path;
	size_t num = list->size();
	fconfig->Write("num", num);
	for (size_t i = 0; i < num; ++i)
	{
		flrd::Ruler* ruler = (*list)[i];
		if (!ruler) continue;
		path = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i);
		fconfig->SetPath(path);
		fconfig->Write("name", ruler->GetName());
		fconfig->Write("group", ruler->Group());
		fconfig->Write("type", static_cast<int>(ruler->GetRulerMode()));
		fconfig->Write("display", ruler->GetDisp());
		fconfig->Write("transient", ruler->GetTransient());
		fconfig->Write("time", ruler->GetTransTime());
		fconfig->Write("info_names", ruler->GetInfoNames());
		fconfig->Write("info_values", ruler->GetInfoValues());
		fconfig->Write("use_color", ruler->GetUseColor());
		fconfig->Write("color", ruler->GetColor());
		fconfig->Write("interp", ruler->GetInterp());
		size_t branch_num = ruler->GetNumBranch();
		fconfig->Write("num", branch_num);
		path = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i) + "/branches";
		fconfig->SetPath(path);
		fconfig->Write("num", branch_num);
		for (size_t j = 0; j < branch_num; ++j)
		{
			std::string path_br = path + std::to_string(j);
			fconfig->SetPath(path_br);
			size_t branch_point_num = ruler->GetNumBranchPoint(j);
			fconfig->Write("num", branch_point_num);
			fconfig->Write("time_point", true);
			for (size_t k = 0; k < branch_point_num; ++k)
			{
				flrd::RulerPoint* rp = ruler->GetRulerPoint(j, k);
				if (!rp) continue;
				std::string path2 = path_br + "/point" + std::to_string(k);
				fconfig->SetPath(path2);
				fconfig->Write("num", rp->GetTimeNum());
				fconfig->Write("locked", rp->GetLocked());
				fconfig->Write("id", rp->GetId());
				for (size_t tpi = 0; tpi < rp->GetTimeNum(); ++tpi)
				{
					std::string tpn = "tp" + std::to_string(tpi);
					size_t t = 0;
					fluo::Point p;
					if (rp->GetTimeAndPoint(tpi, t, p))
					{
						fconfig->Write(tpn + "_time", t);
						fconfig->Write(tpn + "_pos", p);
					}
				}
			}
		}
	}
}

void Project::ReadRulerList(const std::string &gst_name, int view_index)
{
	flrd::RulerList* list = glbin_current.GetRulerList();
	if (!list)
	{
		Root* root = glbin_data_manager.GetRoot();
		if (!root)
			return;
		auto view = root->GetView(0);
		if (view)
			list = view->GetRulerList();
		if (!list)
			return;
	}

	std::shared_ptr<BaseTreeFile> fconfig = glbin_tree_file_factory.getTreeFile(gst_name);
	if (!fconfig)
		return;

	std::string path;
	std::string sval;
	std::wstring wsval;
	int ival;
	bool bval;
	fluo::Color cval;
	fluo::Point pval;

	list->clear();
	size_t ruler_num = 0;
	fconfig->Read("num", &ruler_num);
	for (size_t i = 0; i < ruler_num; ++i)
	{
		path = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i);
		if (fconfig->Exists(path))
		{
			fconfig->SetPath(path);
			flrd::Ruler* ruler = new flrd::Ruler();
			if (fconfig->Read("name", &wsval))
				ruler->SetName(wsval);
			if (fconfig->Read("group", &ival))
				ruler->Group(ival);
			if (fconfig->Read("type", &ival))
				ruler->SetRulerMode(static_cast<flrd::RulerMode>(ival));
			if (fconfig->Read("display", &bval))
				ruler->SetDisp(bval);
			if (fconfig->Read("transient", &bval))
				ruler->SetTransient(bval);
			if (fconfig->Read("time", &ival))
				ruler->SetTransTime(ival);
			if (fconfig->Read("info_names", &sval))
				ruler->SetInfoNames(sval);
			if (fconfig->Read("info_values", &sval))
				ruler->SetInfoValues(sval);
			if (fconfig->Read("use_color", &bval))
			{
				if (bval && fconfig->Read("color", &cval))
					ruler->SetColor(cval);
			}
			if (fconfig->Read("interp", &ival))
				ruler->SetInterp(ival);
			size_t branch_num = 0;
			fconfig->Read("num", &branch_num);
			//num could be points or branch
			for (size_t j = 0; j < branch_num; ++j)
			{
				//if points
				std::string path_nobr = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i) + "/points" + std::to_string(j);
				std::string path_br = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i) + "/branches" + std::to_string(j);
				if (fconfig->Exists(path_nobr))
				{
					fconfig->SetPath(path_nobr);
					if (fconfig->Read("point", &pval))
						ruler->AddPoint(pval);
				}
				//if branch
				else if (fconfig->Exists(path_br))
				{
					fconfig->SetPath(path_br);
					size_t branch_point_num = 0;
					if (fconfig->Read("num", &branch_point_num))
					{
						bool time_point = false;
						fconfig->Read("time_point", &time_point, false);
						for (size_t k = 0; k < branch_point_num; ++k)
						{
							if (time_point)
							{
								std::string path2 = path_br + "/point" + std::to_string(k);
								fconfig->SetPath(path2);
								size_t tnum;
								bool locked;
								fconfig->Read("num", &tnum);
								fconfig->Read("locked", &locked);
								//fconfig->Read("id", &id);
								size_t t;
								for (size_t tpi = 0; tpi < tnum; ++tpi)
								{
									std::string tpn = "tp" + std::to_string(tpi);
									if (fconfig->Read(tpn + "_time", &t) &&
										fconfig->Read(tpn + "_pos", &pval))
									{
										ruler->SetWorkTime(t);
										if (j > 0 && k == 0)
										{
											flrd::pRulerPoint pp = ruler->FindPRulerPoint(pval);
											pp->SetLocked(locked);
											ruler->AddBranch(pp);
										}
										else
										{
											if (tpi == 0)
											{
												ruler->AddPoint(pval);
												flrd::pRulerPoint pp = ruler->FindPRulerPoint(pval);
												pp->SetLocked(locked);
											}
											else
											{
												flrd::pRulerPoint pp = ruler->GetPRulerPoint(k);
												if (pp)
												{
													pp->SetPoint(pval, t);
													pp->SetLocked(locked);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			ruler->SetFinished();
			list->push_back(ruler);
		}
	}
}
