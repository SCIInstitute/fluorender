﻿/*
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
#include <MainFrame.h>
#include <RenderViewPanel.h>
#include <RenderCanvas.h>
#include <ClipPlanePanel.h>
#include <MoviePanel.h>
#include <TrackDlg.h>
#include <compatibility.h>
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

	glbin_data_manager.SetProjectPath(filename);

	//clear
	glbin_data_manager.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	frame->GetRenderCanvas(0)->ClearAll();
	for (int i = frame->GetCanvasNum() - 1; i > 0; i--)
		frame->DeleteRenderView(i);

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
		l_major = std::stol(ver_major);
		d_minor = std::stod(ver_minor);

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
		fconfig->Read("mouse int", &bval, true);
		glbin_settings.m_mouse_int = bval;
		fconfig->Read("mem swap", &bval, false);
		glbin_settings.m_mem_swap = bval;
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
		glbin_settings.GetMemorySettings();
		//peeling layers
		fconfig->Read("peeling layers", &ival, 1);
		glbin_settings.m_peeling_layers = ival;
		//UpdateProps({ gstMouseInt, gstStreamEnable, gstPeelNum });
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
					else if (suffix == L".tif" || suffix == ".tiff")
						loaded_num = glbin_data_manager.LoadVolumeData(filepath, LOAD_TYPE_TIFF, false, cur_chan, cur_time);
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
				}
				VolumeData* vd = 0;
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
							vd->SetBoundary(left_y);
							vd->SetSaturation(offset1);
							vd->SetLeftThresh(left_x);
							vd->SetRightThresh(left_x + width);
							if (fconfig->Read("widgetcolor", &cval))
								vd->SetColor(cval);
							if (fconfig->Read("widgetalpha", &dval))
								vd->SetAlpha(dval);
						}

						//transfer function
						if (fconfig->Read("3dgamma", &dval))
							vd->SetGamma(dval);
						if (fconfig->Read("boundary", &dval))
							vd->SetBoundary(dval);
						if (fconfig->Read("contrast", &dval))
							vd->SetSaturation(dval);
						if (fconfig->Read("left_thresh", &dval))
							vd->SetLeftThresh(dval);
						if (fconfig->Read("right_thresh", &dval))
							vd->SetRightThresh(dval);
						if (fconfig->Read("color", &cval))
							vd->SetColor(cval);
						if (fconfig->Read("hsv", &hval))
							vd->SetHSV(hval.hue(), hval.sat(), hval.val());
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
						double amb, diff, spec, shine;
						if (fconfig->Read("ambient", &amb) &&
							fconfig->Read("diffuse", &diff) &&
							fconfig->Read("specular", &spec) &&
							fconfig->Read("shininess", &shine))
							vd->SetMaterial(amb, diff, spec, shine);
						if (fconfig->Read("shading", &bval))
							vd->SetShadingEnable(bval);
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
								vd->SetBaseSpacings(vval.x(), vval.y(), vval.z());
						}
						else
						{
							if (fconfig->Read("b_res", &vval))
								vd->SetBaseSpacings(vval.x(), vval.y(), vval.z());
							if (fconfig->Read("s_res", &vval))
								vd->SetSpacingScales(vval.x(), vval.y(), vval.z());
						}
						if (fconfig->Read("scl", &vval))
							vd->SetScalings(vval.x(), vval.y(), vval.z());

						std::vector<fluo::Plane*>* planes = 0;
						if (vd->GetVR())
							planes = vd->GetVR()->get_planes();
						int iresx, iresy, iresz;
						vd->GetResolution(iresx, iresy, iresz);
						if (planes && planes->size() == 6)
						{
							//x1
							if (fconfig->Read("x1_vali", &dval))
								(*planes)[0]->ChangePlane(fluo::Point(abs(dval / iresx), 0.0, 0.0),
									fluo::Vector(1.0, 0.0, 0.0));
							else if (fconfig->Read("x1_val", &dval))
								(*planes)[0]->ChangePlane(fluo::Point(abs(dval), 0.0, 0.0),
									fluo::Vector(1.0, 0.0, 0.0));

							//x2
							if (fconfig->Read("x2_vali", &dval))
								(*planes)[1]->ChangePlane(fluo::Point(abs(dval / iresx), 0.0, 0.0),
									fluo::Vector(-1.0, 0.0, 0.0));
							else if (fconfig->Read("x2_val", &dval))
								(*planes)[1]->ChangePlane(fluo::Point(abs(dval), 0.0, 0.0),
									fluo::Vector(-1.0, 0.0, 0.0));

							//y1
							if (fconfig->Read("y1_vali", &dval))
								(*planes)[2]->ChangePlane(fluo::Point(0.0, abs(dval / iresy), 0.0),
									fluo::Vector(0.0, 1.0, 0.0));
							else if (fconfig->Read("y1_val", &dval))
								(*planes)[2]->ChangePlane(fluo::Point(0.0, abs(dval), 0.0),
									fluo::Vector(0.0, 1.0, 0.0));

							//y2
							if (fconfig->Read("y2_vali", &dval))
								(*planes)[3]->ChangePlane(fluo::Point(0.0, abs(dval / iresy), 0.0),
									fluo::Vector(0.0, -1.0, 0.0));
							else if (fconfig->Read("y2_val", &dval))
								(*planes)[3]->ChangePlane(fluo::Point(0.0, abs(dval), 0.0),
									fluo::Vector(0.0, -1.0, 0.0));

							//z1
							if (fconfig->Read("z1_vali", &dval))
								(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, abs(dval / iresz)),
									fluo::Vector(0.0, 0.0, 1.0));
							else if (fconfig->Read("z1_val", &dval))
								(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, abs(dval)),
									fluo::Vector(0.0, 0.0, 1.0));

							//z2
							if (fconfig->Read("z2_vali", &dval))
								(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, abs(dval / iresz)),
									fluo::Vector(0.0, 0.0, -1.0));
							else if (fconfig->Read("z2_val", &dval))
								(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, abs(dval)),
									fluo::Vector(0.0, 0.0, -1.0));
						}

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
							vd->SetColormapMode(ival);
						if (fconfig->Read("colormap_inv", &dval))
							vd->SetColormapInv(dval);
						if (fconfig->Read("colormap", &ival))
							vd->SetColormap(ival);
						if (fconfig->Read("colormap_proj", &ival))
							vd->SetColormapProj(ival);
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
							vd->SetMode(ival);
						//noise reduction
						if (fconfig->Read("noise_red", &bval))
							vd->SetNR(bval);
						//depth override
						if (fconfig->Read("depth_ovrd", &ival))
							vd->SetBlendMode(ival);

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
							BaseReader* br = &msk_reader;
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
				MeshData* md = glbin_data_manager.GetLastMeshData();
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
						if (fconfig->Read("lighting", &bval))
							md->SetLighting(bval);
						double shine, alpha;
						fconfig->Read("shininess", &shine, 30.0);
						fconfig->Read("alpha", &alpha, 0.5);
						fluo::Color amb, diff, spec;
						fconfig->Read("ambient", &amb);
						fconfig->Read("diffuse", &diff);
						fconfig->Read("specular", &spec);
						md->SetMaterial(amb, diff, spec, shine, alpha);
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
					glbin_data_manager.LoadAnnotations(wsval);
				}
			}
		}
		glbin_current.ann_data = glbin_data_manager.GetAnnotations(cur_ann_data);
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
				frame->CreateRenderView();
			RenderCanvas* canvas = frame->GetLastRenderCanvas();
			if (!canvas)
				continue;

			canvas->ClearAll();

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
						VolumeData* vd = glbin_data_manager.GetVolumeData(wsval);
						if (vd)
							canvas->AddVolumeData(vd);
					}
				}
				canvas->SetVolPopDirty();
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
						MeshData* md = glbin_data_manager.GetMeshData(wsval);
						if (md)
							canvas->AddMeshData(md);
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
									VolumeData* vd = glbin_data_manager.GetVolumeData(wsval);
									if (vd)
										canvas->AddVolumeData(vd);
								}
							}
							break;
							case 3://mesh data
							{
								if (fconfig->Read("name", &wsval))
								{
									MeshData* md = glbin_data_manager.GetMeshData(wsval);
									if (md)
										canvas->AddMeshData(md);
								}
							}
							break;
							case 4://annotations
							{
								if (fconfig->Read("name", &wsval))
								{
									Annotations* ann = glbin_data_manager.GetAnnotations(wsval);
									if (ann)
										canvas->AddAnnotations(ann);
								}
							}
							break;
							case 5://group
							{
								if (fconfig->Read("name", &wsval))
								{
									if (fconfig->Read("id", &ival))
										DataGroup::SetID(ival);
									wsval = canvas->AddGroup(wsval);
									DataGroup* group = canvas->GetGroup(wsval);
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
													VolumeData* vd = glbin_data_manager.GetVolumeData(wsval);
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
									canvas->SetVolPopDirty();
								}
							}
							break;
							case 6://mesh group
							{
								if (fconfig->Read("name", &wsval))
								{
									if (fconfig->Read("id", &ival))
										MeshGroup::SetID(ival);
									wsval = canvas->AddMGroup(wsval);
									MeshGroup* group = canvas->GetMGroup(wsval);
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
													MeshData* md = glbin_data_manager.GetMeshData(wsval);
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
									canvas->SetMeshPopDirty();
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
					canvas->LoadTrackGroup(wsval);
				}
			}

			//properties
			path = "/views/" + std::to_string(i) + "/properties";
			if (fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				if (fconfig->Read("drawall", &bval))
					canvas->SetDraw(bval);
				//properties
				if (fconfig->Read("persp", &bval))
					canvas->SetPersp(bval);
				if (fconfig->Read("free", &bval))
					canvas->SetFree(bval);
				if (fconfig->Read("aov", &dval))
					canvas->SetAov(dval);
				double nearclip;
				double farclip;
				if (fconfig->Read("nearclip", &nearclip) &&
					fconfig->Read("farclip", &farclip))
				{
					canvas->SetNearClip(nearclip);
					canvas->SetFarClip(farclip);
					if (glbin_xr_renderer)
						glbin_xr_renderer->SetClips(nearclip, farclip);
				}
				if (fconfig->Read("backgroundcolor", &cval))
					canvas->SetBackgroundColor(cval);
				if (fconfig->Read("volmethod", &ival))
					canvas->SetVolMethod(ival);
				if (fconfig->Read("fog", &bval))
					canvas->SetFog(bval);
				if (fconfig->Read("fogintensity", &dval))
					canvas->SetFogIntensity(dval);
				if (fconfig->Read("draw_camctr", &bval))
					canvas->m_draw_camctr = bval;
				if (fconfig->Read("draw_info", &ival))
					canvas->m_draw_info = ival;
				if (fconfig->Read("draw_legend", &bval))
					canvas->m_draw_legend = bval;

				//camera
				if (fconfig->Read("translation", &vval))
					canvas->SetTranslations(vval);
				if (fconfig->Read("rotation", &vval))
						canvas->SetRotations(vval, false);
				if (fconfig->Read("zero_quat", &qval))
						canvas->SetZeroQuat(qval);
				if (fconfig->Read("center", &pval))
					canvas->SetCenters(pval);
				if (fconfig->Read("centereyedist", &dval))
					canvas->SetCenterEyeDist(dval);
				if (fconfig->Read("radius", &dval, 5.0))
					canvas->SetRadius(dval);
				if (fconfig->Read("initdist", &dval))
					canvas->SetInitDist(dval);
				else
					canvas->SetInitDist(canvas->GetRadius() / tan(d2r(canvas->GetAov() / 2.0)));
				if (fconfig->Read("scale_mode", &ival))
					canvas->m_scale_mode = ival;
				if (fconfig->Read("scale", &dval))
					canvas->m_scale_factor = dval;
				else
					canvas->m_scale_factor = canvas->GetRadius() / tan(d2r(canvas->GetAov() / 2.0)) / canvas->GetInitDist();
				if (fconfig->Read("pin_rot_center", &bval))
					canvas->SetPinRotCenter(bval);
				//object
				if (fconfig->Read("obj_center", &pval))
					canvas->SetObjCenters(pval);
				if (fconfig->Read("obj_trans", &vval))
					canvas->SetObjTrans(vval);
				if (fconfig->Read("obj_rot", &vval))
				{
					if (l_major <= 2 && d_minor < 24.3)
						vval += fluo::Vector(0, 180, 180);
					canvas->SetObjRot(vval);
				}
				//scale bar
				if (fconfig->Read("disp_scale_bar", &bval))
					canvas->m_disp_scale_bar = bval;
				if (fconfig->Read("disp_scale_bar_text", &bval))
					canvas->m_disp_scale_bar_text = bval;
				if (fconfig->Read("sb_length", &dval))
					canvas->m_sb_length = dval;
				if (fconfig->Read("sb_text", &wsval))
					canvas->m_sb_text = wsval;
				if (fconfig->Read("sb_num", &wsval))
					canvas->m_sb_num = wsval;
				if (fconfig->Read("sb_unit", &ival))
					canvas->m_sb_unit = ival;

				//2d sdjustment settings
				if (fconfig->Read("gamma", &cval))
					canvas->SetGammaColor(cval);
				if (fconfig->Read("brightness", &cval))
					canvas->SetBrightness(cval);
				if (fconfig->Read("hdr", &cval))
					canvas->SetHdr(cval);
				if (fconfig->Read("sync_r", &bval))
					canvas->SetSync(0, bval);
				if (fconfig->Read("sync_g", &bval))
					canvas->SetSync(1, bval);
				if (fconfig->Read("sync_b", &bval))
					canvas->SetSync(2, bval);

				//clipping plane rotations
				if (fconfig->Read("clip_mode", &ival))
					canvas->SetClipMode(ival);
				double rotx_cl, roty_cl, rotz_cl;
				if (fconfig->Read("rot_cl", &vval))
					canvas->SetClippingPlaneRotations(vval);
				else if (fconfig->Read("rotx_cl", &rotx_cl) &&
					fconfig->Read("roty_cl", &roty_cl) &&
					fconfig->Read("rotz_cl", &rotz_cl))
					canvas->SetClippingPlaneRotations(fluo::Vector(rotx_cl, roty_cl, rotz_cl));

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
			if (canvas->GetRulerList() && fconfig->Exists(path))
			{
				fconfig->SetPath(path);
				ReadRulerList(gstProjectFile, i);
			}
		}
		glbin_current.canvas = frame->GetRenderCanvas(cur_canvas);
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
			frame->GetClipPlanPanel()->SetXLink(bval);
		if (fconfig->Read("y_link", &bval))
			frame->GetClipPlanPanel()->SetYLink(bval);
		if (fconfig->Read("z_link", &bval))
			frame->GetClipPlanPanel()->SetZLink(bval);
	}

	//movie panel
	path = "/movie_panel";
	if (fconfig->Exists(path))
	{
		fconfig->SetPath(path);

		//set settings for frame
		RenderCanvas* canvas = 0;
		if (fconfig->Read("key frame enable", &bval))
			glbin_moviemaker.SetKeyframeEnable(bval);
		if (fconfig->Read("views_cmb", &ival))
		{
			canvas = frame->GetRenderCanvas(ival);
			glbin_moviemaker.SetView(canvas);
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
				glbin_moviemaker.SetCurrentTime(curf);
				RenderCanvas* canvas = frame->GetLastRenderCanvas();
				if (canvas)
				{
					canvas->Set4DSeqFrame(curf, startf, endf, false);
				}
			}
		}
		if (fconfig->Read("run_script", &bval))
			glbin_settings.m_run_script = bval;
		if (fconfig->Read("script_file", &wsval))
			glbin_settings.m_script_file = wsval;
		frame->GetMoviePanel()->FluoUpdate();
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

	frame->RefreshCanvases();
	frame->UpdateProps({}, 0, 0);
}

void Project::Save(const std::wstring& filename, bool inc)
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
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
	fconfig->Write("mouse int", glbin_settings.m_mouse_int);
	fconfig->Write("mem swap", glbin_settings.m_mem_swap);
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
	wsval = glbin_current.canvas ? glbin_current.canvas->GetName().ToStdWstring() : L"";
	fconfig->Write("canvas", wsval);
	wsval = glbin_current.vol_group ? glbin_current.vol_group->GetName() : L"";
	fconfig->Write("vol group", wsval);
	wsval = glbin_current.mesh_group ? glbin_current.mesh_group->GetName() : L"";
	fconfig->Write("mesh group", wsval);
	wsval = glbin_current.vol_data ? glbin_current.vol_data->GetName() : L"";
	fconfig->Write("vol data", wsval);
	wsval = glbin_current.mesh_data ? glbin_current.mesh_data->GetName() : L"";
	fconfig->Write("mesh data", wsval);
	wsval = glbin_current.ann_data ? glbin_current.ann_data->GetName() : L"";
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

		VolumeData* vd = glbin_data_manager.GetVolumeData(i);
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
					fluo::Point(), fluo::Quaternion(), fluo::Point(), false);
				fconfig->Write("path", p.wstring());
				new_chan = true;
			}
			else
				fconfig->Write("path", filepath);
			BaseReader* reader = vd->GetReader();
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
			fconfig->Write("3dgamma", vd->GetGamma());
			fconfig->Write("boundary", vd->GetBoundary());
			fconfig->Write("contrast", vd->GetSaturation());
			fconfig->Write("left_thresh", vd->GetLeftThresh());
			fconfig->Write("right_thresh", vd->GetRightThresh());
			fconfig->Write("color", vd->GetColor());
			fconfig->Write("hsv", vd->GetHSVColor());
			fconfig->Write("mask_color", vd->GetMaskColor());
			fconfig->Write("mask_color_set", vd->GetMaskColorSet());
			fconfig->Write("enable_alpha", vd->GetAlphaEnable());
			fconfig->Write("alpha", vd->GetAlpha());
			double amb, diff, spec, shine;
			vd->GetMaterial(amb, diff, spec, shine);
			fconfig->Write("ambient", amb);
			fconfig->Write("diffuse", diff);
			fconfig->Write("specular", spec);
			fconfig->Write("shininess", shine);
			fconfig->Write("shading", vd->GetShadingEnable());
			fconfig->Write("samplerate", vd->GetSampleRate());

			//resolution scale
			fconfig->Write("res", vd->GetSpacings());
			fconfig->Write("b_res", vd->GetBaseSpacings());
			fconfig->Write("s_res", vd->GetSpacingScales());
			fconfig->Write("scl", vd->GetScalings());

			//planes
			std::vector<fluo::Plane*>* planes = 0;
			if (vd->GetVR())
				planes = vd->GetVR()->get_planes();
			if (planes && planes->size() == 6)
			{
				fluo::Plane* plane = 0;
				double abcd[4];

				//x1
				plane = (*planes)[0];
				plane->get_copy(abcd);
				fconfig->Write("x1_val", abcd[3]);
				//x2
				plane = (*planes)[1];
				plane->get_copy(abcd);
				fconfig->Write("x2_val", abcd[3]);
				//y1
				plane = (*planes)[2];
				plane->get_copy(abcd);
				fconfig->Write("y1_val", abcd[3]);
				//y2
				plane = (*planes)[3];
				plane->get_copy(abcd);
				fconfig->Write("y2_val", abcd[3]);
				//z1
				plane = (*planes)[4];
				plane->get_copy(abcd);
				fconfig->Write("z1_val", abcd[3]);
				//z2
				plane = (*planes)[5];
				plane->get_copy(abcd);
				fconfig->Write("z2_val", abcd[3]);
			}

			//2d adjustment settings
			fconfig->Write("gamma", vd->GetGammaColor());
			fconfig->Write("brightness", vd->GetBrightness());
			fconfig->Write("hdr", vd->GetHdr());
			fconfig->Write("sync_r", vd->GetSync(0));
			fconfig->Write("sync_g", vd->GetSync(1));
			fconfig->Write("sync_b", vd->GetSync(2));

			//colormap settings
			fconfig->Write("colormap_mode", vd->GetColormapMode());
			fconfig->Write("colormap_inv", vd->GetColormapInv());
			fconfig->Write("colormap", vd->GetColormap());
			fconfig->Write("colormap_proj", vd->GetColormapProj());
			double low, high;
			vd->GetColormapValues(low, high);
			fconfig->Write("colormap_lo_value", low);
			fconfig->Write("colormap_hi_value", high);

			//high transp
			fconfig->Write("alpha_power", vd->GetAlphaPower());
			//inversion
			fconfig->Write("inv", vd->GetInvert());
			//mip enable
			fconfig->Write("mode", vd->GetMode());
			//noise reduction
			fconfig->Write("noise_red", vd->GetNR());
			//depth override
			fconfig->Write("depth_ovrd", vd->GetBlendMode());

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

		MeshData* md = glbin_data_manager.GetMeshData(i);
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
			fconfig->Write("lighting", md->GetLighting());
			//material
			fluo::Color amb, diff, spec;
			double shine, alpha;
			md->GetMaterial(amb, diff, spec, shine, alpha);
			fconfig->Write("ambient", amb);
			fconfig->Write("diffuse", diff);
			fconfig->Write("specular", spec);
			fconfig->Write("shininess", shine);
			fconfig->Write("alpha", alpha);
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
	num = glbin_data_manager.GetAnnotationNum();
	fconfig->Write("num", num);
	for (size_t i = 0; i < num; i++)
	{
		Annotations* ann = glbin_data_manager.GetAnnotations(i);
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
	num = frame->GetCanvasNum();
	fconfig->Write("num", num);
	for (size_t i = 0; i < static_cast<size_t>(frame->GetCanvasNum()); i++)
	{
		RenderCanvas* canvas = frame->GetRenderCanvas(i);
		if (canvas)
		{
			//view layers
			path = "/views/" + std::to_string(i) + "/layers";
			fconfig->SetPath(path);
			int layer_num = canvas->GetLayerNum();
			fconfig->Write("num", layer_num);
			for (int j = 0; j < layer_num; j++)
			{
				TreeLayer* layer = canvas->GetLayer(j);
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
					DataGroup* group = (DataGroup*)layer;

					fconfig->Write("type", 5);
					fconfig->Write("name", layer->GetName());
					fconfig->Write("id", DataGroup::GetID());
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
					MeshGroup* group = (MeshGroup*)layer;

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
			int ival = canvas->GetTrackFileExist(true);
			if (ival == 1)
			{
				std::wstring new_folder;
				new_folder = filename2 + L"_files";
				MkDirW(new_folder);
				std::filesystem::path p(new_folder);
				p /= GET_NAME(filename2) + L".track";
				canvas->SaveTrackGroup(p.wstring());
			}
			fconfig->Write("track_file",canvas->GetTrackGroupFile());

			//properties
			path = "/views/" + std::to_string(i) + "/properties";
			fconfig->SetPath(path);
			fconfig->Write("drawall", canvas->GetDraw());
			fconfig->Write("persp", canvas->GetPersp());
			fconfig->Write("free", canvas->GetFree());
			fconfig->Write("aov", canvas->GetAov());
			fconfig->Write("nearclip", canvas->GetNearClip());
			fconfig->Write("farclip", canvas->GetFarClip());
			fconfig->Write("backgroundcolor", canvas->GetBackgroundColor());
			fconfig->Write("drawtype", canvas->GetDrawType());
			fconfig->Write("volmethod", canvas->GetVolMethod());
			fconfig->Write("fog", canvas->GetFog());
			fconfig->Write("fogintensity", canvas->GetFogIntensity());
			fconfig->Write("draw_camctr", canvas->m_draw_camctr);
			fconfig->Write("draw_info", canvas->m_draw_info);
			fconfig->Write("draw_legend", canvas->m_draw_legend);

			//camera
			fconfig->Write("translation", canvas->GetTranslations());
			fconfig->Write("rotation", canvas->GetRotations());
			fconfig->Write("zero_quat", canvas->GetZeroQuat());
			fconfig->Write("center", canvas->GetCenters());
			fconfig->Write("centereyedist", canvas->GetCenterEyeDist());
			fconfig->Write("radius", canvas->GetRadius());
			fconfig->Write("initdist", canvas->GetInitDist());
			fconfig->Write("scale_mode", canvas->m_scale_mode);
			fconfig->Write("scale", canvas->m_scale_factor);
			fconfig->Write("pin_rot_center", canvas->m_pin_rot_ctr);
			//object
			fconfig->Write("obj_center", canvas->GetObjCenters());
			fconfig->Write("obj_trans", canvas->GetObjTrans());
			fconfig->Write("obj_rot", canvas->GetObjRot());
			//scale bar
			fconfig->Write("disp_scale_bar", canvas->m_disp_scale_bar);
			fconfig->Write("disp_scale_bar_text", canvas->m_disp_scale_bar_text);
			fconfig->Write("sb_length", canvas->m_sb_length);
			fconfig->Write("sb_text", canvas->m_sb_text);
			fconfig->Write("sb_num", canvas->m_sb_num);
			fconfig->Write("sb_unit", canvas->m_sb_unit);

			//2d adjustment
			fconfig->Write("gamma", canvas->GetGammaColor());
			fconfig->Write("brightness", canvas->GetBrightness());
			fconfig->Write("hdr", canvas->GetHdr());
			fconfig->Write("sync_r", canvas->GetSync(0));
			fconfig->Write("sync_g", canvas->GetSync(1));
			fconfig->Write("sync_b", canvas->GetSync(2));

			//clipping plane rotations
			fconfig->Write("clip_mode", canvas->GetClipMode());
			fconfig->Write("rot_cl", canvas->GetClippingPlaneRotations());

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
	fconfig->Write("x_link", frame->GetClipPlanPanel()->GetXLink());
	fconfig->Write("y_link", frame->GetClipPlanPanel()->GetYLink());
	fconfig->Write("z_link", frame->GetClipPlanPanel()->GetZLink());
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

	glbin_data_manager.SetProjectPath(L"");
	//SetTitle(m_title);
	//clear
	glbin_data_manager.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	frame->GetRenderCanvas(0)->ClearAll();
	for (int i = frame->GetCanvasNum() - 1; i > 0; i--)
		frame->DeleteRenderView(i);
	RenderViewPanel::ResetID();
	glbin_current.SetRoot();
	glbin_moviemaker.Stop();
	glbin_moviemaker.SetView(frame->GetRenderCanvas(0));
	glbin_mov_def.Apply(&glbin_moviemaker);
	glbin_interpolator.Clear();
	glbin_volume_point.SetVolumeData(0);
	glbin_comp_analyzer.ClearCompGroup();
}

void Project::ExportRulerList(const std::wstring& filename)
{
	flrd::RulerList* list = glbin_current.GetRulerList();
	RenderCanvas* canvas = glbin_current.canvas;
	if (!list || !canvas)
	{
		return;
	}

	std::wofstream os;
	OutputStreamOpenW(os, filename);

	std::string str;
	std::wstring unit;
	int num_points;
	fluo::Point p;
	flrd::Ruler* ruler;
	switch (canvas->m_sb_unit)
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
		ruler->SetWorkTime(canvas->m_tseq_cur_num);

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
			str = "RGB(" +
				std::to_string(int(std::round(color.r() * 255))) + ", " +
				std::to_string(int(std::round(color.g() * 255))) + ", " +
				std::to_string(int(std::round(color.b() * 255))) + ")";
		}
		else
			str = "N/A";
		os << str << "\t";

		//branch count
		str = std::to_string(ruler->GetNumBranch());
		os << str << "\t";
		//length
		str = std::to_string(ruler->GetLength());
		os << str << "\t";
		//angle
		str = std::to_string(ruler->GetAngle());
		os << str << "\t";

		str = "";
		//start and end points
		num_points = ruler->GetNumPoint();
		if (num_points > 0)
		{
			p = ruler->GetPoint(0);
			str += p.to_string();
		}
		if (num_points > 1)
		{
			p = ruler->GetPoint(num_points - 1);
			str += p.to_string();
		}
		else
			str += "\t\t\t";
		os << str;

		//time
		if (ruler->GetTransient())
			str = std::to_string(ruler->GetTransTime());
		else
			str = "N/A";
		os << str << "\t";

		//info values v1 v2
		os << ruler->GetInfoValues() << "\n";

		//export points
		if (ruler->GetNumPoint() > 2)
		{
			os << ruler->GetPosNames();
			os << ruler->GetPosValues();
		}

		//export profile
		std::vector<flrd::ProfileBin>* profile = ruler->GetProfile();
		if (profile && profile->size())
		{
			double sumd = 0.0;
			unsigned long long sumull = 0;
			os << ruler->GetInfoProfile() << "\n";
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
		fconfig->Write("type", ruler->GetRulerType());
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
		return;

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
				ruler->SetRulerType(ival);
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
				std::string path_nobr = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i) + "/points/" + std::to_string(j);
				std::string path_br = "/views/" + std::to_string(view_index) + "/rulers/" + std::to_string(i) + "/branches/" + std::to_string(j);
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
