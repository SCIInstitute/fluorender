/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <wx/wfstream.h>

Project::Project() :
	Progress()
{
}

Project::~Project()
{
}

void Project::Open(wxString& filename)
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
		return;

	glbin_data_manager.SetProjectPath(filename);

	int iVal;
	int i, j, k;
	//clear
	glbin_data_manager.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	frame->GetRenderCanvas(0)->ClearAll();
	for (i = frame->GetCanvasNum() - 1; i > 0; i--)
		frame->DeleteRenderView(i);

	wxFileInputStream is(filename);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);
	wxString ver_major, ver_minor;
	long l_major;
	double d_minor;
	l_major = 1;
	if (fconfig.Read("ver_major", &ver_major) &&
		fconfig.Read("ver_minor", &ver_minor))
	{
		ver_major.ToLong(&l_major);
		ver_minor.ToDouble(&d_minor);

		if (l_major > VERSION_MAJOR)
			::wxMessageBox("The project file is saved by a newer version of FluoRender.\n" \
				"Please check update and download the new version.");
		else if (d_minor > VERSION_MINOR)
			::wxMessageBox("The project file is saved by a newer version of FluoRender.\n" \
				"Please check update and download the new version.");
	}

	int ticks = 3;
	int tick_cnt = 1;
	fconfig.Read("ticks", &ticks);

	SetProgress(0, "FluoRender is reading the project file. Please wait.");

	bool bval;
	double dval;
	int ival;
	//read streaming mode
	if (fconfig.Exists("/settings"))
	{
		fconfig.SetPath("/settings");
		fconfig.Read("mouse int", &bval, true);
		glbin_settings.m_mouse_int = bval;
		fconfig.Read("mem swap", &bval, false);
		glbin_settings.m_mem_swap = bval;
		fconfig.Read("graphics mem", &dval, 1000.0);
		glbin_settings.m_graphics_mem = dval;
		fconfig.Read("large data size", &dval, 1000.0);
		glbin_settings.m_large_data_size = dval;
		fconfig.Read("force brick size", &ival, 128);
		glbin_settings.m_force_brick_size = ival;
		fconfig.Read("up time", &ival, 100);
		glbin_settings.m_up_time = ival;
		fconfig.Read("update order", &ival, 0);
		glbin_settings.m_update_order = ival;
		fconfig.Read("inf loop", &bval, false);
		glbin_settings.m_inf_loop = bval;
		//graphics memory setting may have changed
		glbin_settings.GetMemorySettings();
		//peeling layers
		fconfig.Read("peeling layers", &ival, 1);
		glbin_settings.m_peeling_layers = ival;
		//UpdateProps({ gstMouseInt, gstStreamEnable, gstPeelNum });
	}

	//current
	glbin_current.SetRoot();
	wxString cur_canvas, cur_vol_group, cur_mesh_group,
		cur_vol_data, cur_mesh_data, cur_ann_data;
	if (fconfig.Exists("/current"))
	{
		fconfig.SetPath("/current");
		fconfig.Read("canvas", &cur_canvas, "");
		fconfig.Read("vol group", &cur_vol_group, "");
		fconfig.Read("mesh group", &cur_mesh_group, "");
		fconfig.Read("vol data", &cur_vol_data, "");
		fconfig.Read("mesh data", &cur_mesh_data, "");
		fconfig.Read("ann data", &cur_ann_data, "");
	}

	//read data list
	//volume
	if (fconfig.Exists("/data/volume"))
	{
		fconfig.SetPath("/data/volume");
		int num = fconfig.Read("num", 0l);
		for (i = 0; i < num; i++)
		{
			SetProgress(100 * tick_cnt / ticks,
				"FluoRender is reading and processing volume data. Please wait.");


			wxString str;
			str = wxString::Format("/data/volume/%d", i);
			if (fconfig.Exists(str))
			{
				int loaded_num = 0;
				fconfig.SetPath(str);
				bool compression = false;
				fconfig.Read("compression", &compression);
				glbin_settings.m_realtime_compress = compression;
				bool skip_brick = false;
				fconfig.Read("skip_brick", &skip_brick);
				glbin_settings.m_skip_brick = skip_brick;
				//path
				if (fconfig.Read("path", &str))
				{
					int cur_chan = 0;
					if (!fconfig.Read("cur_chan", &cur_chan))
						if (fconfig.Read("tiff_chan", &cur_chan))
							cur_chan--;
					int cur_time = 0;
					fconfig.Read("cur_time", &cur_time);
					//reader type
					int reader_type = 0;
					fconfig.Read("reader_type", &reader_type);
					bool slice_seq = 0;
					fconfig.Read("slice_seq", &slice_seq);
					glbin_settings.m_slice_sequence = slice_seq;
					wxString time_id;
					fconfig.Read("time_id", &time_id);
					glbin_settings.m_time_id = time_id;
					bool fp_convert = false;
					double minv, maxv;
					fconfig.Read("fp_convert", &fp_convert, false);
					fconfig.Read("fp_min", &minv, 0);
					fconfig.Read("fp_max", &maxv, 1);
					glbin_settings.m_fp_convert = fp_convert;
					glbin_settings.m_fp_min = minv;
					glbin_settings.m_fp_max = maxv;
					wxString suffix = str.Mid(str.Find('.', true)).MakeLower();
					if (reader_type == READER_IMAGEJ_TYPE)
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_IMAGEJ, true, cur_chan, cur_time);
					else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_NRRD, false, cur_chan, cur_time);
					else if (suffix == ".tif" || suffix == ".tiff")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_TIFF, false, cur_chan, cur_time);
					else if (suffix == ".oib")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_OIB, false, cur_chan, cur_time);
					else if (suffix == ".oif")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_OIF, false, cur_chan, cur_time);
					else if (suffix == ".lsm")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_LSM, false, cur_chan, cur_time);
					else if (suffix == ".xml")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_PVXML, false, cur_chan, cur_time);
					else if (suffix == ".vvd")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_BRKXML, false, cur_chan, cur_time);
					else if (suffix == ".czi")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_CZI, false, cur_chan, cur_time);
					else if (suffix == ".nd2")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_ND2, false, cur_chan, cur_time);
					else if (suffix == ".lif")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_LIF, false, cur_chan, cur_time);
					else if (suffix == ".lof")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_LOF, false, cur_chan, cur_time);
					else if (suffix == ".mp4" || suffix == ".m4v" || suffix == ".mov" || suffix == ".avi" || suffix == ".wmv")
						loaded_num = glbin_data_manager.LoadVolumeData(str, LOAD_TYPE_MPG, false, cur_chan, cur_time);
				}
				VolumeData* vd = 0;
				if (loaded_num)
					vd = glbin_data_manager.GetLastVolumeData();
				if (vd)
				{
					if (fconfig.Read("name", &str))
						vd->SetName(str);//setname
					//volume properties
					if (fconfig.Exists("properties"))
					{
						fconfig.SetPath("properties");
						bool disp;
						if (fconfig.Read("display", &disp))
							vd->SetDisp(disp);

						//old colormap
						if (fconfig.Read("widget", &str))
						{
							int type;
							float left_x, left_y, width, height, offset1, offset2, gamma;
							wchar_t token[256] = {};
							token[255] = '\0';
							const wchar_t* sstr = str.wc_str();
							std::wstringstream ss(sstr);
							ss.read(token, 255);
							wchar_t c = 'x';
							while (!isspace(c)) ss.read(&c, 1);
							ss >> type >> left_x >> left_y >> width >>
								height >> offset1 >> offset2 >> gamma;
							vd->SetGamma(gamma);
							vd->SetBoundary(left_y);
							vd->SetSaturation(offset1);
							vd->SetLeftThresh(left_x);
							vd->SetRightThresh(left_x + width);
							if (fconfig.Read("widgetcolor", &str))
							{
								float red, green, blue;
								if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)) {
									fluo::Color col(red, green, blue);
									vd->SetColor(col);
								}
							}
							double alpha;
							if (fconfig.Read("widgetalpha", &alpha))
								vd->SetAlpha(alpha);
						}

						//transfer function
						double dval;
						bool bval;
						if (fconfig.Read("3dgamma", &dval))
							vd->SetGamma(dval);
						if (fconfig.Read("boundary", &dval))
							vd->SetBoundary(dval);
						if (fconfig.Read("contrast", &dval))
							vd->SetSaturation(dval);
						if (fconfig.Read("left_thresh", &dval))
							vd->SetLeftThresh(dval);
						if (fconfig.Read("right_thresh", &dval))
							vd->SetRightThresh(dval);
						if (fconfig.Read("color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)) {
								fluo::Color col(red, green, blue);
								vd->SetColor(col);
							}
						}
						if (fconfig.Read("hsv", &str))
						{
							float hue, sat, val;
							if (SSCANF(str.c_str(), "%f%f%f", &hue, &sat, &val))
								vd->SetHSV(hue, sat, val);
						}
						if (fconfig.Read("mask_color", &str))
						{
							float red, green, blue;
							if (SSCANF(str.c_str(), "%f%f%f", &red, &green, &blue)) {
								fluo::Color col(red, green, blue);
								if (fconfig.Read("mask_color_set", &bval))
									vd->SetMaskColor(col, bval);
								else
									vd->SetMaskColor(col);
							}
						}
						if (fconfig.Read("enable_alpha", &bval))
							vd->SetAlphaEnable(bval);
						if (fconfig.Read("alpha", &dval))
							vd->SetAlpha(dval);

						//shading
						double amb, diff, spec, shine;
						if (fconfig.Read("ambient", &amb) &&
							fconfig.Read("diffuse", &diff) &&
							fconfig.Read("specular", &spec) &&
							fconfig.Read("shininess", &shine))
							vd->SetMaterial(amb, diff, spec, shine);
						bool shading;
						if (fconfig.Read("shading", &shading))
							vd->SetShadingEnable(shading);
						double srate;
						if (fconfig.Read("samplerate", &srate))
						{
							if (l_major < 2)
								vd->SetSampleRate(srate / 5.0);
							else
								vd->SetSampleRate(srate);
						}

						//spacings and scales
						if (!vd->isBrxml())
						{
							if (fconfig.Read("res", &str))
							{
								double resx, resy, resz;
								if (SSCANF(str.c_str(), "%lf%lf%lf", &resx, &resy, &resz))
								{
									vd->SetBaseSpacings(resx, resy, resz);
								}
							}
						}
						else
						{
							if (fconfig.Read("b_res", &str))
							{
								double b_resx, b_resy, b_resz;
								if (SSCANF(str.c_str(), "%lf%lf%lf", &b_resx, &b_resy, &b_resz))
									vd->SetBaseSpacings(b_resx, b_resy, b_resz);
							}
							if (fconfig.Read("s_res", &str))
							{
								double s_resx, s_resy, s_resz;
								if (SSCANF(str.c_str(), "%lf%lf%lf", &s_resx, &s_resy, &s_resz))
									vd->SetSpacingScales(s_resx, s_resy, s_resz);
							}
						}
						if (fconfig.Read("scl", &str))
						{
							double sclx, scly, sclz;
							if (SSCANF(str.c_str(), "%lf%lf%lf", &sclx, &scly, &sclz))
								vd->SetScalings(sclx, scly, sclz);
						}

						vector<fluo::Plane*>* planes = 0;
						if (vd->GetVR())
							planes = vd->GetVR()->get_planes();
						int iresx, iresy, iresz;
						vd->GetResolution(iresx, iresy, iresz);
						if (planes && planes->size() == 6)
						{
							double val;
							wxString splane;

							//x1
							if (fconfig.Read("x1_vali", &val))
								(*planes)[0]->ChangePlane(fluo::Point(abs(val / iresx), 0.0, 0.0),
									fluo::Vector(1.0, 0.0, 0.0));
							else if (fconfig.Read("x1_val", &val))
								(*planes)[0]->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
									fluo::Vector(1.0, 0.0, 0.0));

							//x2
							if (fconfig.Read("x2_vali", &val))
								(*planes)[1]->ChangePlane(fluo::Point(abs(val / iresx), 0.0, 0.0),
									fluo::Vector(-1.0, 0.0, 0.0));
							else if (fconfig.Read("x2_val", &val))
								(*planes)[1]->ChangePlane(fluo::Point(abs(val), 0.0, 0.0),
									fluo::Vector(-1.0, 0.0, 0.0));

							//y1
							if (fconfig.Read("y1_vali", &val))
								(*planes)[2]->ChangePlane(fluo::Point(0.0, abs(val / iresy), 0.0),
									fluo::Vector(0.0, 1.0, 0.0));
							else if (fconfig.Read("y1_val", &val))
								(*planes)[2]->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
									fluo::Vector(0.0, 1.0, 0.0));

							//y2
							if (fconfig.Read("y2_vali", &val))
								(*planes)[3]->ChangePlane(fluo::Point(0.0, abs(val / iresy), 0.0),
									fluo::Vector(0.0, -1.0, 0.0));
							else if (fconfig.Read("y2_val", &val))
								(*planes)[3]->ChangePlane(fluo::Point(0.0, abs(val), 0.0),
									fluo::Vector(0.0, -1.0, 0.0));

							//z1
							if (fconfig.Read("z1_vali", &val))
								(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, abs(val / iresz)),
									fluo::Vector(0.0, 0.0, 1.0));
							else if (fconfig.Read("z1_val", &val))
								(*planes)[4]->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
									fluo::Vector(0.0, 0.0, 1.0));

							//z2
							if (fconfig.Read("z2_vali", &val))
								(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, abs(val / iresz)),
									fluo::Vector(0.0, 0.0, -1.0));
							else if (fconfig.Read("z2_val", &val))
								(*planes)[5]->ChangePlane(fluo::Point(0.0, 0.0, abs(val)),
									fluo::Vector(0.0, 0.0, -1.0));
						}

						//2d adjustment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
								fluo::Color col(r, g, b);
								vd->SetGammaColor(col);
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
								fluo::Color col(r, g, b);
								vd->SetBrightness(col);
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
								fluo::Color col(r, g, b);
								vd->SetHdr(col);
							}
						}
						bool bVal;
						if (fconfig.Read("sync_r", &bVal))
							vd->SetSync(0, bVal);
						if (fconfig.Read("sync_g", &bVal))
							vd->SetSync(1, bVal);
						if (fconfig.Read("sync_b", &bVal))
							vd->SetSync(2, bVal);

						//colormap settings
						if (fconfig.Read("colormap_mode", &iVal))
							vd->SetColormapMode(iVal);
						if (fconfig.Read("colormap_inv", &dval))
							vd->SetColormapInv(dval);
						if (fconfig.Read("colormap", &iVal))
							vd->SetColormap(iVal);
						if (fconfig.Read("colormap_proj", &iVal))
							vd->SetColormapProj(iVal);
						double low, high;
						if (fconfig.Read("colormap_lo_value", &low) &&
							fconfig.Read("colormap_hi_value", &high))
						{
							vd->SetColormapValues(low, high);
						}

						//high transp
						if (fconfig.Read("alpha_power", &dval))
							vd->SetAlphaPower(dval);
						//inversion
						if (fconfig.Read("inv", &bVal))
							vd->SetInvert(bVal);
						//mip enable
						if (fconfig.Read("mode", &iVal))
							vd->SetMode(iVal);
						//noise reduction
						if (fconfig.Read("noise_red", &bVal))
							vd->SetNR(bVal);
						//depth override
						if (fconfig.Read("depth_ovrd", &iVal))
							vd->SetBlendMode(iVal);

						//shadow
						if (fconfig.Read("shadow", &bVal))
							vd->SetShadowEnable(bVal);
						//shaodw intensity
						if (fconfig.Read("shadow_darkness", &dval))
							vd->SetShadowIntensity(dval);

						//legend
						if (fconfig.Read("legend", &bVal))
							vd->SetLegend(bVal);

						//mask
						if (fconfig.Read("mask", &str))
						{
							MSKReader msk_reader;
							wstring maskname = str.ToStdWstring();
							msk_reader.SetFile(maskname);
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
	if (fconfig.Exists("/data/mesh"))
	{
		fconfig.SetPath("/data/mesh");
		int num = fconfig.Read("num", 0l);
		for (i = 0; i < num; i++)
		{
			SetProgress(100 * tick_cnt / ticks,
				"FluoRender is reading and processing mesh data. Please wait.");

			wxString str;
			str = wxString::Format("/data/mesh/%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				if (fconfig.Read("path", &str))
				{
					glbin_data_manager.LoadMeshData(str);
				}
				MeshData* md = glbin_data_manager.GetLastMeshData();
				if (md)
				{
					if (fconfig.Read("name", &str))
						md->SetName(str);//setname
					//mesh properties
					if (fconfig.Exists("properties"))
					{
						fconfig.SetPath("properties");
						bool disp;
						if (fconfig.Read("display", &disp))
							md->SetDisp(disp);
						//lighting
						bool lighting;
						if (fconfig.Read("lighting", &lighting))
							md->SetLighting(lighting);
						double shine, alpha;
						float r = 0.0f, g = 0.0f, b = 0.0f;
						if (fconfig.Read("ambient", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						fluo::Color amb(r, g, b);
						if (fconfig.Read("diffuse", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						fluo::Color diff(r, g, b);
						if (fconfig.Read("specular", &str))
							SSCANF(str.c_str(), "%f%f%f", &r, &g, &b);
						fluo::Color spec(r, g, b);
						fconfig.Read("shininess", &shine, 30.0);
						fconfig.Read("alpha", &alpha, 0.5);
						md->SetMaterial(amb, diff, spec, shine, alpha);
						//2d adjusment settings
						if (fconfig.Read("gamma", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
								fluo::Color col(r, g, b);
								md->SetGammaColor(col);
							}
						}
						if (fconfig.Read("brightness", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
								fluo::Color col(r, g, b);
								md->SetBrightness(col);
							}
						}
						if (fconfig.Read("hdr", &str))
						{
							float r, g, b;
							if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
								fluo::Color col(r, g, b);
								md->SetHdr(col);
							}
						}
						bool bVal;
						if (fconfig.Read("sync_r", &bVal))
							md->SetSync(0, bVal);
						if (fconfig.Read("sync_g", &bVal))
							md->SetSync(1, bVal);
						if (fconfig.Read("sync_b", &bVal))
							md->SetSync(2, bVal);
						//shadow
						if (fconfig.Read("shadow", &bVal))
							md->SetShadowEnable(bVal);
						double darkness;
						if (fconfig.Read("shadow_darkness", &darkness))
							md->SetShadowIntensity(darkness);

						//mesh transform
						if (fconfig.Exists("../transform"))
						{
							fconfig.SetPath("../transform");
							float x, y, z;
							if (fconfig.Read("translation", &str))
							{
								if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
									md->SetTranslation(x, y, z);
							}
							if (fconfig.Read("rotation", &str))
							{
								if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
									md->SetRotation(x, y, z);
							}
							if (fconfig.Read("scaling", &str))
							{
								if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
									md->SetScaling(x, y, z);
							}
						}

					}
				}
			}
			tick_cnt++;
		}
		glbin_current.mesh_data = glbin_data_manager.GetMeshData(cur_mesh_data);
	}
	//annotations
	if (fconfig.Exists("/data/annotations"))
	{
		fconfig.SetPath("/data/annotations");
		int num = fconfig.Read("num", 0l);
		for (i = 0; i < num; i++)
		{
			wxString str;
			str = wxString::Format("/data/annotations/%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				if (fconfig.Read("path", &str))
				{
					glbin_data_manager.LoadAnnotations(str);
				}
			}
		}
		glbin_current.ann_data = glbin_data_manager.GetAnnotations(cur_ann_data);
	}

	bool bVal;
	//views
	if (fconfig.Exists("/views"))
	{
		fconfig.SetPath("/views");
		int num = fconfig.Read("num", 0l);

		for (i = 0; i < num; i++)
		{
			if (i > 0)
				frame->CreateRenderView();
			RenderCanvas* canvas = frame->GetLastRenderCanvas();
			if (!canvas)
				continue;

			canvas->ClearAll();

			wxString str;
			//old
			//volumes
			str = wxString::Format("/views/%d/volumes", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				int num = fconfig.Read("num", 0l);
				for (j = 0; j < num; j++)
				{
					if (fconfig.Read(wxString::Format("name%d", j), &str))
					{
						VolumeData* vd = glbin_data_manager.GetVolumeData(str);
						if (vd)
							canvas->AddVolumeData(vd);
					}
				}
				canvas->SetVolPopDirty();
			}
			//meshes
			str = wxString::Format("/views/%d/meshes", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				int num = fconfig.Read("num", 0l);
				for (j = 0; j < num; j++)
				{
					if (fconfig.Read(wxString::Format("name%d", j), &str))
					{
						MeshData* md = glbin_data_manager.GetMeshData(str);
						if (md)
							canvas->AddMeshData(md);
					}
				}
			}

			//new
			str = wxString::Format("/views/%d/layers", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);

				//view layers
				int layer_num = fconfig.Read("num", 0l);
				for (j = 0; j < layer_num; j++)
				{
					if (fconfig.Exists(wxString::Format("/views/%d/layers/%d", i, j)))
					{
						fconfig.SetPath(wxString::Format("/views/%d/layers/%d", i, j));
						int type;
						if (fconfig.Read("type", &type))
						{
							switch (type)
							{
							case 2://volume data
							{
								if (fconfig.Read("name", &str))
								{
									VolumeData* vd = glbin_data_manager.GetVolumeData(str);
									if (vd)
										canvas->AddVolumeData(vd);
								}
							}
							break;
							case 3://mesh data
							{
								if (fconfig.Read("name", &str))
								{
									MeshData* md = glbin_data_manager.GetMeshData(str);
									if (md)
										canvas->AddMeshData(md);
								}
							}
							break;
							case 4://annotations
							{
								if (fconfig.Read("name", &str))
								{
									Annotations* ann = glbin_data_manager.GetAnnotations(str);
									if (ann)
										canvas->AddAnnotations(ann);
								}
							}
							break;
							case 5://group
							{
								if (fconfig.Read("name", &str))
								{
									int id;
									if (fconfig.Read("id", &id))
										DataGroup::SetID(id);
									str = canvas->AddGroup(str);
									DataGroup* group = canvas->GetGroup(str);
									if (group)
									{
										//display
										if (fconfig.Read("display", &bVal))
										{
											group->SetDisp(bVal);
										}
										//2d adjustment
										if (fconfig.Read("gamma", &str))
										{
											float r, g, b;
											if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
												fluo::Color col(r, g, b);
												group->SetGammaColor(col);
											}
										}
										if (fconfig.Read("brightness", &str))
										{
											float r, g, b;
											if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
												fluo::Color col(r, g, b);
												group->SetBrightness(col);
											}
										}
										if (fconfig.Read("hdr", &str))
										{
											float r, g, b;
											if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
												fluo::Color col(r, g, b);
												group->SetHdr(col);
											}
										}
										if (fconfig.Read("sync_r", &bVal))
											group->SetSync(0, bVal);
										if (fconfig.Read("sync_g", &bVal))
											group->SetSync(1, bVal);
										if (fconfig.Read("sync_b", &bVal))
											group->SetSync(2, bVal);
										//sync volume properties
										if (fconfig.Read("sync_vp", &bVal))
											group->SetVolumeSyncProp(bVal);
										//volumes
										if (fconfig.Exists(wxString::Format("/views/%d/layers/%d/volumes", i, j)))
										{
											fconfig.SetPath(wxString::Format("/views/%d/layers/%d/volumes", i, j));
											int vol_num = fconfig.Read("num", 0l);
											for (k = 0; k < vol_num; k++)
											{
												if (fconfig.Read(wxString::Format("vol_%d", k), &str))
												{
													VolumeData* vd = glbin_data_manager.GetVolumeData(str);
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
								if (fconfig.Read("name", &str))
								{
									int id;
									if (fconfig.Read("id", &id))
										MeshGroup::SetID(id);
									str = canvas->AddMGroup(str);
									MeshGroup* group = canvas->GetMGroup(str);
									if (group)
									{
										//display
										if (fconfig.Read("display", &bVal))
											group->SetDisp(bVal);
										//sync mesh properties
										if (fconfig.Read("sync_mp", &bVal))
											group->SetMeshSyncProp(bVal);
										//meshes
										if (fconfig.Exists(wxString::Format("/views/%d/layers/%d/meshes", i, j)))
										{
											fconfig.SetPath(wxString::Format("/views/%d/layers/%d/meshes", i, j));
											int mesh_num = fconfig.Read("num", 0l);
											for (k = 0; k < mesh_num; k++)
											{
												if (fconfig.Read(wxString::Format("mesh_%d", k), &str))
												{
													MeshData* md = glbin_data_manager.GetMeshData(str);
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
			if (fconfig.Exists(wxString::Format("/views/%d/track_group", i)))
			{
				if (fconfig.Read("track_file", &str))
				{
					canvas->LoadTrackGroup(str);
				}
			}

			//properties
			if (fconfig.Exists(wxString::Format("/views/%d/properties", i)))
			{
				float x, y, z, w;
				fconfig.SetPath(wxString::Format("/views/%d/properties", i));
				bool draw;
				if (fconfig.Read("drawall", &draw))
					canvas->SetDraw(draw);
				//properties
				bool persp;
				if (fconfig.Read("persp", &persp))
					canvas->SetPersp(persp);
				else
					canvas->SetPersp(true);
				bool free;
				if (fconfig.Read("free", &free))
					canvas->SetFree(free);
				else
					canvas->SetFree(false);
				double aov;
				if (fconfig.Read("aov", &aov))
					canvas->SetAov(aov);
				double nearclip;
				if (fconfig.Read("nearclip", &nearclip))
					canvas->SetNearClip(nearclip);
				double farclip;
				if (fconfig.Read("farclip", &farclip))
					canvas->SetFarClip(farclip);
				if (fconfig.Read("backgroundcolor", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
						fluo::Color col(r, g, b);
						canvas->SetBackgroundColor(col);
					}
				}
				int volmethod;
				if (fconfig.Read("volmethod", &volmethod))
					canvas->SetVolMethod(volmethod);
				bool fog;
				if (fconfig.Read("fog", &fog))
					canvas->SetFog(fog);
				double fogintensity;
				if (fconfig.Read("fogintensity", &fogintensity))
					canvas->SetFogIntensity(fogintensity);
				if (fconfig.Read("draw_camctr", &bVal))
				{
					canvas->m_draw_camctr = bVal;
				}
				if (fconfig.Read("draw_info", &iVal))
				{
					canvas->m_draw_info = iVal;
				}
				if (fconfig.Read("draw_legend", &bVal))
				{
					canvas->m_draw_legend = bVal;
				}

				//camera
				if (fconfig.Read("translation", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetTranslations(x, y, z);
				}
				if (fconfig.Read("rotation", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetRotations(x, y, z, false);
				}
				if (fconfig.Read("zero_quat", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f%f", &x, &y, &z, &w))
						canvas->SetZeroQuat(x, y, z, w);
				}
				if (fconfig.Read("center", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetCenters(x, y, z);
				}
				double dist;
				if (fconfig.Read("centereyedist", &dist))
					canvas->SetCenterEyeDist(dist);
				double radius = 5.0;
				if (fconfig.Read("radius", &radius))
					canvas->SetRadius(radius);
				double initdist;
				if (fconfig.Read("initdist", &initdist))
					canvas->SetInitDist(initdist);
				else
					canvas->SetInitDist(radius / tan(d2r(canvas->GetAov() / 2.0)));
				int scale_mode;
				if (fconfig.Read("scale_mode", &scale_mode))
					canvas->m_scale_mode = scale_mode;
				double scale;
				if (!fconfig.Read("scale", &scale))
					scale = radius / tan(d2r(canvas->GetAov() / 2.0)) / dist;
				canvas->m_scale_factor = scale;
				bool pin_rot_center;
				if (fconfig.Read("pin_rot_center", &pin_rot_center))
					canvas->SetPinRotCenter(pin_rot_center);
				//object
				if (fconfig.Read("obj_center", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetObjCenters(x, y, z);
				}
				if (fconfig.Read("obj_trans", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
						canvas->SetObjTrans(x, y, z);
				}
				if (fconfig.Read("obj_rot", &str))
				{
					if (SSCANF(str.c_str(), "%f%f%f", &x, &y, &z))
					{
						if (l_major <= 2 && d_minor < 24.3)
							canvas->SetObjRot(x, y + 180.0, z + 180.0);
						else
							canvas->SetObjRot(x, y, z);
					}
				}
				//scale bar
				bool disp;
				if (fconfig.Read("disp_scale_bar", &disp))
					canvas->m_disp_scale_bar = disp;
				if (fconfig.Read("disp_scale_bar_text", &disp))
					canvas->m_disp_scale_bar_text = disp;
				double length;
				if (fconfig.Read("sb_length", &length))
					canvas->m_sb_length = length;
				if (fconfig.Read("sb_text", &str))
					canvas->m_sb_text = str;
				if (fconfig.Read("sb_num", &str))
					canvas->m_sb_num = str;
				int unit;
				if (fconfig.Read("sb_unit", &unit))
					canvas->m_sb_unit = unit;

				//2d sdjustment settings
				if (fconfig.Read("gamma", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
						fluo::Color col(r, g, b);
						canvas->SetGammaColor(col);
					}
				}
				if (fconfig.Read("brightness", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
						fluo::Color col(r, g, b);
						canvas->SetBrightness(col);
					}
				}
				if (fconfig.Read("hdr", &str))
				{
					float r, g, b;
					if (SSCANF(str.c_str(), "%f%f%f", &r, &g, &b)) {
						fluo::Color col(r, g, b);
						canvas->SetHdr(col);
					}
				}
				if (fconfig.Read("sync_r", &bVal))
					canvas->SetSync(0, bVal);
				if (fconfig.Read("sync_g", &bVal))
					canvas->SetSync(1, bVal);
				if (fconfig.Read("sync_b", &bVal))
					canvas->SetSync(2, bVal);

				//clipping plane rotations
				int clip_mode;
				if (fconfig.Read("clip_mode", &clip_mode))
					canvas->SetClipMode(clip_mode);
				double rotx_cl, roty_cl, rotz_cl;
				if (fconfig.Read("rotx_cl", &rotx_cl) &&
					fconfig.Read("roty_cl", &roty_cl) &&
					fconfig.Read("rotz_cl", &rotz_cl))
				{
					canvas->SetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
				}

				//painting parameters
				double dVal;
				if (fconfig.Read("brush_use_pres", &bVal))
					glbin_vol_selector.SetBrushUsePres(bVal);
				double size1, size2;
				if (fconfig.Read("brush_size_1", &size1) &&
					fconfig.Read("brush_size_2", &size2))
					glbin_vol_selector.SetBrushSize(size1, size2);
				if (fconfig.Read("brush_spacing", &dVal))
					glbin_vol_selector.SetBrushSpacing(dVal);
				if (fconfig.Read("brush_iteration", &dVal))
					glbin_vol_selector.SetBrushIteration(dVal);
				if (fconfig.Read("brush_size_data", &bVal))
					glbin_vol_selector.SetBrushSizeData(bVal);
				if (fconfig.Read("brush_translate", &dVal))
					glbin_vol_selector.SetBrushSclTranslate(dVal);
				if (fconfig.Read("w2d", &dVal))
					glbin_vol_selector.SetW2d(dVal);

			}

			//rulers
			if (canvas->GetRulerList() &&
				fconfig.Exists(wxString::Format("/views/%d/rulers", i)))
			{
				fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
				glbin_ruler_handler.Read(fconfig, i);
			}
		}
		glbin_current.canvas = frame->GetRenderCanvas(cur_canvas);
	}

	//clipping planes
	if (fconfig.Exists("/prop_panel"))
	{
		fconfig.SetPath("/prop_panel");
		bool bval;
		if (fconfig.Read("clip link", &bval))
			glbin_settings.m_clip_link = bval;
		if (fconfig.Read("clip hold", &bval))
			glbin_settings.m_clip_hold = bval;
		if (fconfig.Read("plane mode", &ival))
			glbin_settings.m_clip_mode = ival;
		if (fconfig.Read("x_link", &bval))
			frame->GetClipPlanPanel()->SetXLink(bval);
		if (fconfig.Read("y_link", &bval))
			frame->GetClipPlanPanel()->SetYLink(bval);
		if (fconfig.Read("z_link", &bval))
			frame->GetClipPlanPanel()->SetZLink(bval);
	}

	//movie panel
	if (fconfig.Exists("/movie_panel"))
	{
		fconfig.SetPath("/movie_panel");
		wxString sVal;
		bool bVal;
		int iVal;
		double dVal;

		//set settings for frame
		RenderCanvas* canvas = 0;
		if (fconfig.Read("key frame enable", &bVal))
			glbin_moviemaker.SetKeyframeEnable(bVal);
		if (fconfig.Read("views_cmb", &iVal))
		{
			canvas = frame->GetRenderCanvas(iVal);
			glbin_moviemaker.SetView(canvas);
		}
		if (fconfig.Read("rot_check", &bVal))
			glbin_moviemaker.SetRotateEnable(bVal);
		if (fconfig.Read("seq_mode", &iVal))
			glbin_moviemaker.SetSeqMode(iVal);
		if (fconfig.Read("x_rd", &bVal))
		{
			if (bVal)
				glbin_moviemaker.SetRotateAxis(0);
		}
		if (fconfig.Read("y_rd", &bVal))
		{
			if (bVal)
				glbin_moviemaker.SetRotateAxis(1);
		}
		if (fconfig.Read("z_rd", &bVal))
		{
			if (bVal)
				glbin_moviemaker.SetRotateAxis(2);
		}
		if (fconfig.Read("rot_axis", &iVal))
			glbin_moviemaker.SetRotateAxis(iVal);
		if (fconfig.Read("rot_deg", &iVal))
			glbin_moviemaker.SetRotateDeg(iVal);
		if (fconfig.Read("movie_len", &dVal))
			glbin_moviemaker.SetMovieLength(dVal);
		if (fconfig.Read("fps", &dVal))
			glbin_moviemaker.SetFps(dVal);
		if (fconfig.Read("crop", &bVal))
			glbin_moviemaker.SetCropEnable(bVal);
		if (fconfig.Read("crop_x", &iVal))
			glbin_moviemaker.SetCropX(iVal);
		if (fconfig.Read("crop_y", &iVal))
			glbin_moviemaker.SetCropY(iVal);
		if (fconfig.Read("crop_w", &iVal))
			glbin_moviemaker.SetCropW(iVal);
		if (fconfig.Read("crop_h", &iVal))
			glbin_moviemaker.SetCropH(iVal);
		if (fconfig.Read("full frame num", &iVal))
			glbin_moviemaker.SetFullFrameNum(iVal);
		int startf = 0, endf = 0, curf = 0;
		if (fconfig.Read("start_frame", &startf) &&
			fconfig.Read("end_frame", &endf))
			glbin_moviemaker.SetClipStartEndFrames(startf, endf);
		if (fconfig.Read("cur_frame", &curf))
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
		if (fconfig.Read("run_script", &bVal))
			glbin_settings.m_run_script = bVal;
		if (fconfig.Read("script_file", &sVal))
			glbin_settings.m_script_file = sVal;
		frame->GetMoviePanel()->FluoUpdate();
	}

	//ui layout
	if (fconfig.Exists("/ui_layout"))
	{
		fconfig.SetPath("/ui_layout");
		wxString str;
		double dval;
		bool update = false;
		if (fconfig.Read("dpi scale factor", &dval))
			update = fluo::InEpsilon(dval, frame->GetDPIScaleFactor());
		if (update && fconfig.Read("layout", &str))
			frame->LoadPerspective(str);
	}

	//selection

	//interpolator
	if (fconfig.Exists("/interpolator"))
	{
		wxString str;
		wxString sVal;
		double dVal;

		fconfig.SetPath("/interpolator");
		glbin_interpolator.Clear();
		if (fconfig.Read("max_id", &iVal))
			Interpolator::m_id = iVal;
		vector<FlKeyGroup*>* key_list = glbin_interpolator.GetKeyList();
		int group_num = fconfig.Read("num", 0l);
		for (i = 0; i < group_num; i++)
		{
			str = wxString::Format("/interpolator/%d", i);
			if (fconfig.Exists(str))
			{
				fconfig.SetPath(str);
				FlKeyGroup* key_group = new FlKeyGroup;
				if (fconfig.Read("id", &iVal))
					key_group->id = iVal;
				if (fconfig.Read("t", &dVal))
					key_group->t = dVal;
				if (fconfig.Read("dt", &dVal))
					key_group->dt = dVal;
				else
				{
					if (key_list->empty())
						key_group->dt = 0;
					else
						key_group->dt = key_group->t - key_list->back()->t;
				}
				if (fconfig.Read("type", &iVal))
					key_group->type = iVal;
				if (fconfig.Read("desc", &sVal))
					key_group->desc = sVal.ToStdString();
				str = wxString::Format("/interpolator/%d/keys", i);
				if (fconfig.Exists(str))
				{
					fconfig.SetPath(str);
					int key_num = fconfig.Read("num", 0l);
					for (j = 0; j < key_num; j++)
					{
						str = wxString::Format("/interpolator/%d/keys/%d", i, j);
						if (fconfig.Exists(str))
						{
							fconfig.SetPath(str);
							int key_type;
							if (fconfig.Read("type", &key_type))
							{
								FlKeyCode code;
								if (fconfig.Read("l0", &iVal))
									code.l0 = iVal;
								if (fconfig.Read("l0_name", &sVal))
									code.l0_name = sVal.ToStdString();
								if (fconfig.Read("l1", &iVal))
									code.l1 = iVal;
								if (fconfig.Read("l1_name", &sVal))
									code.l1_name = sVal.ToStdString();
								if (fconfig.Read("l2", &iVal))
									code.l2 = iVal;
								if (fconfig.Read("l2_name", &sVal))
									code.l2_name = sVal.ToStdString();
								switch (key_type)
								{
								case FLKEY_TYPE_DOUBLE:
								{
									if (fconfig.Read("val", &dVal))
									{
										FlKeyDouble* key = new FlKeyDouble(code, dVal);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_QUATER:
								{
									if (fconfig.Read("val", &sVal))
									{
										double x, y, z, w;
										if (SSCANF(sVal.c_str(), "%lf%lf%lf%lf",
											&x, &y, &z, &w))
										{
											fluo::Quaternion qval = fluo::Quaternion(x, y, z, w);
											FlKeyQuaternion* key = new FlKeyQuaternion(code, qval);
											key_group->keys.push_back(key);
										}
									}
								}
								break;
								case FLKEY_TYPE_BOOLEAN:
								{
									if (fconfig.Read("val", &bVal))
									{
										FlKeyBoolean* key = new FlKeyBoolean(code, bVal);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_INT:
								{
									if (fconfig.Read("val", &iVal))
									{
										FlKeyInt* key = new FlKeyInt(code, iVal);
										key_group->keys.push_back(key);
									}
								}
								break;
								case FLKEY_TYPE_COLOR:
								{
									if (fconfig.Read("val", &sVal))
									{
										double r, g, b;
										if (SSCANF(sVal.c_str(), "%lf%lf%lf",
											&r, &g, &b))
										{
											fluo::Color cval = fluo::Color(r, g, b);
											FlKeyColor* key = new FlKeyColor(code, cval);
											key_group->keys.push_back(key);
										}
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

void Project::Save(wxString& filename, bool inc)
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
		return;

	wxString filename2 = filename;
	if (inc)
		filename2 = INC_NUM_EXIST(filename);

	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = filename2;
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	int i, j, k;
	fconfig.Write("ver_major", VERSION_MAJOR_TAG);
	fconfig.Write("ver_minor", VERSION_MINOR_TAG);

	int ticks = glbin_data_manager.GetVolumeNum() + glbin_data_manager.GetMeshNum();
	ticks = ticks ? ticks : 1;
	int tick_cnt = 1;
	fconfig.Write("ticks", ticks);
	SetProgress(0, "FluoRender is saving the project file. Please wait.");

	wxString str;

	fconfig.SetPath("/settings");
	//save streaming mode
	fconfig.Write("mouse int", glbin_settings.m_mouse_int);
	fconfig.Write("mem swap", glbin_settings.m_mem_swap);
	fconfig.Write("graphics mem", glbin_settings.m_graphics_mem);
	fconfig.Write("large data size", glbin_settings.m_large_data_size);
	fconfig.Write("force brick size", glbin_settings.m_force_brick_size);
	fconfig.Write("up time", glbin_settings.m_up_time);
	fconfig.Write("update order", glbin_settings.m_update_order);
	fconfig.Write("inf loop", glbin_settings.m_inf_loop);
	//save peeling layers
	fconfig.Write("peeling layers", glbin_settings.m_peeling_layers);

	fconfig.SetPath("/current");
	str = glbin_current.canvas ? glbin_current.canvas->GetName() : wxString("");
	fconfig.Write("canvas", str);
	str = glbin_current.vol_group ? glbin_current.vol_group->GetName() : wxString("");
	fconfig.Write("vol group", str);
	str = glbin_current.mesh_group ? glbin_current.mesh_group->GetName() : wxString("");
	fconfig.Write("mesh group", str);
	str = glbin_current.vol_data ? glbin_current.vol_data->GetName() : wxString("");
	fconfig.Write("vol data", str);
	str = glbin_current.mesh_data ? glbin_current.mesh_data->GetName() : wxString("");
	fconfig.Write("mesh data", str);
	str = glbin_current.ann_data ? glbin_current.ann_data->GetName() : wxString("");
	fconfig.Write("ann data", str);

	//save data list
	//volume
	fconfig.SetPath("/data/volume");
	fconfig.Write("num", glbin_data_manager.GetVolumeNum());
	for (i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
	{
		SetProgress(100 * tick_cnt / ticks,
			"FluoRender is saving volume data. Please wait.");
		tick_cnt++;

		VolumeData* vd = glbin_data_manager.GetVolumeData(i);
		if (vd)
		{
			str = wxString::Format("/data/volume/%d", i);
			//name
			fconfig.SetPath(str);
			str = vd->GetName();
			fconfig.Write("name", str);
			//compression
			fconfig.Write("compression", glbin_settings.m_realtime_compress);
			//skip brick
			fconfig.Write("skip_brick", vd->GetSkipBrick());
			//path
			str = vd->GetPath();
			bool new_chan = false;
			if (str == "" || glbin_settings.m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + vd->GetName() + ".tif";
				vd->Save(str, 0, 3, false,
					false, 0, false, glbin_settings.m_save_compress,
					fluo::Point(), fluo::Quaternion(), fluo::Point(), false);
				fconfig.Write("path", str);
				new_chan = true;
			}
			else
				fconfig.Write("path", str);
			BaseReader* reader = vd->GetReader();
			if (reader)
			{
				//reader type
				fconfig.Write("reader_type", reader->GetType());
				fconfig.Write("slice_seq", reader->GetSliceSeq());
				str = reader->GetTimeId();
				fconfig.Write("time_id", str);
				//float convert
				fconfig.Write("fp_convert", reader->GetFpConvert());
				double minv, maxv;
				reader->GetFpRange(minv, maxv);
				fconfig.Write("fp_min", minv);
				fconfig.Write("fp_max", maxv);
			}
			else
			{
				fconfig.Write("slice_seq", false);
				fconfig.Write("time_id", "");
			}
			fconfig.Write("cur_time", vd->GetCurTime());
			fconfig.Write("cur_chan", new_chan ? 0 : vd->GetCurChannel());

			//volume properties
			fconfig.SetPath("properties");
			fconfig.Write("display", vd->GetDisp());

			//properties
			fconfig.Write("3dgamma", vd->GetGamma());
			fconfig.Write("boundary", vd->GetBoundary());
			fconfig.Write("contrast", vd->GetSaturation());
			fconfig.Write("left_thresh", vd->GetLeftThresh());
			fconfig.Write("right_thresh", vd->GetRightThresh());
			fluo::Color color = vd->GetColor();
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("color", str);
			double hue, sat, val;
			vd->GetHSV(hue, sat, val);
			str = wxString::Format("%f %f %f", hue, sat, val);
			fconfig.Write("hsv", str);
			color = vd->GetMaskColor();
			str = wxString::Format("%f %f %f", color.r(), color.g(), color.b());
			fconfig.Write("mask_color", str);
			fconfig.Write("mask_color_set", vd->GetMaskColorSet());
			fconfig.Write("enable_alpha", vd->GetAlphaEnable());
			fconfig.Write("alpha", vd->GetAlpha());
			double amb, diff, spec, shine;
			vd->GetMaterial(amb, diff, spec, shine);
			fconfig.Write("ambient", amb);
			fconfig.Write("diffuse", diff);
			fconfig.Write("specular", spec);
			fconfig.Write("shininess", shine);
			fconfig.Write("shading", vd->GetShadingEnable());
			fconfig.Write("samplerate", vd->GetSampleRate());

			//resolution scale
			double resx, resy, resz;
			double b_resx, b_resy, b_resz;
			double s_resx, s_resy, s_resz;
			double sclx, scly, sclz;
			vd->GetSpacings(resx, resy, resz);
			vd->GetBaseSpacings(b_resx, b_resy, b_resz);
			vd->GetSpacingScales(s_resx, s_resy, s_resz);
			vd->GetScalings(sclx, scly, sclz);
			str = wxString::Format("%lf %lf %lf", resx, resy, resz);
			fconfig.Write("res", str);
			str = wxString::Format("%lf %lf %lf", b_resx, b_resy, b_resz);
			fconfig.Write("b_res", str);
			str = wxString::Format("%lf %lf %lf", s_resx, s_resy, s_resz);
			fconfig.Write("s_res", str);
			str = wxString::Format("%lf %lf %lf", sclx, scly, sclz);
			fconfig.Write("scl", str);

			//planes
			vector<fluo::Plane*>* planes = 0;
			if (vd->GetVR())
				planes = vd->GetVR()->get_planes();
			if (planes && planes->size() == 6)
			{
				fluo::Plane* plane = 0;
				double abcd[4];

				//x1
				plane = (*planes)[0];
				plane->get_copy(abcd);
				fconfig.Write("x1_val", abcd[3]);
				//x2
				plane = (*planes)[1];
				plane->get_copy(abcd);
				fconfig.Write("x2_val", abcd[3]);
				//y1
				plane = (*planes)[2];
				plane->get_copy(abcd);
				fconfig.Write("y1_val", abcd[3]);
				//y2
				plane = (*planes)[3];
				plane->get_copy(abcd);
				fconfig.Write("y2_val", abcd[3]);
				//z1
				plane = (*planes)[4];
				plane->get_copy(abcd);
				fconfig.Write("z1_val", abcd[3]);
				//z2
				plane = (*planes)[5];
				plane->get_copy(abcd);
				fconfig.Write("z2_val", abcd[3]);
			}

			//2d adjustment settings
			str = wxString::Format("%f %f %f", vd->GetGammaColor().r(), vd->GetGammaColor().g(), vd->GetGammaColor().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", vd->GetBrightness().r(), vd->GetBrightness().g(), vd->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", vd->GetHdr().r(), vd->GetHdr().g(), vd->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", vd->GetSync(0));
			fconfig.Write("sync_g", vd->GetSync(1));
			fconfig.Write("sync_b", vd->GetSync(2));

			//colormap settings
			fconfig.Write("colormap_mode", vd->GetColormapMode());
			fconfig.Write("colormap_inv", vd->GetColormapInv());
			fconfig.Write("colormap", vd->GetColormap());
			fconfig.Write("colormap_proj", vd->GetColormapProj());
			double low, high;
			vd->GetColormapValues(low, high);
			fconfig.Write("colormap_lo_value", low);
			fconfig.Write("colormap_hi_value", high);

			//high transp
			fconfig.Write("alpha_power", vd->GetAlphaPower());
			//inversion
			fconfig.Write("inv", vd->GetInvert());
			//mip enable
			fconfig.Write("mode", vd->GetMode());
			//noise reduction
			fconfig.Write("noise_red", vd->GetNR());
			//depth override
			fconfig.Write("depth_ovrd", vd->GetBlendMode());

			//shadow
			fconfig.Write("shadow", vd->GetShadowEnable());
			//shadow intensity
			fconfig.Write("shadow_darkness", vd->GetShadowIntensity());

			//legend
			fconfig.Write("legend", vd->GetLegend());

			//mask
			vd->SaveMask(true, vd->GetCurTime(), vd->GetCurChannel());
			vd->SaveLabel(true, vd->GetCurTime(), vd->GetCurChannel());
		}
	}
	//mesh
	fconfig.SetPath("/data/mesh");
	fconfig.Write("num", glbin_data_manager.GetMeshNum());
	for (i = 0; i < glbin_data_manager.GetMeshNum(); i++)
	{
		SetProgress(100 * tick_cnt / ticks,
			"FluoRender is saving mesh data. Please wait.");
		tick_cnt++;

		MeshData* md = glbin_data_manager.GetMeshData(i);
		if (md)
		{
			if (md->GetPath() == "" || glbin_settings.m_vrp_embed)
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + md->GetName() + ".obj";
				md->Save(str);
			}
			str = wxString::Format("/data/mesh/%d", i);
			fconfig.SetPath(str);
			str = md->GetName();
			fconfig.Write("name", str);
			str = md->GetPath();
			fconfig.Write("path", str);
			//mesh prperties
			fconfig.SetPath("properties");
			fconfig.Write("display", md->GetDisp());
			//lighting
			fconfig.Write("lighting", md->GetLighting());
			//material
			fluo::Color amb, diff, spec;
			double shine, alpha;
			md->GetMaterial(amb, diff, spec, shine, alpha);
			str = wxString::Format("%f %f %f", amb.r(), amb.g(), amb.b());
			fconfig.Write("ambient", str);
			str = wxString::Format("%f %f %f", diff.r(), diff.g(), diff.b());
			fconfig.Write("diffuse", str);
			str = wxString::Format("%f %f %f", spec.r(), spec.g(), spec.b());
			fconfig.Write("specular", str);
			fconfig.Write("shininess", shine);
			fconfig.Write("alpha", alpha);
			//2d adjustment settings
			str = wxString::Format("%f %f %f", md->GetGammaColor().r(), md->GetGammaColor().g(), md->GetGammaColor().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", md->GetBrightness().r(), md->GetBrightness().g(), md->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", md->GetHdr().r(), md->GetHdr().g(), md->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", md->GetSync(0));
			fconfig.Write("sync_g", md->GetSync(1));
			fconfig.Write("sync_b", md->GetSync(2));
			//shadow
			fconfig.Write("shadow", md->GetShadowEnable());
			fconfig.Write("shadow_darkness", md->GetShadowIntensity());

			//mesh transform
			fconfig.SetPath("../transform");
			double x, y, z;
			md->GetTranslation(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("translation", str);
			md->GetRotation(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("rotation", str);
			md->GetScaling(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("scaling", str);
		}
	}
	//annotations
	fconfig.SetPath("/data/annotations");
	fconfig.Write("num", glbin_data_manager.GetAnnotationNum());
	for (i = 0; i < glbin_data_manager.GetAnnotationNum(); i++)
	{
		Annotations* ann = glbin_data_manager.GetAnnotations(i);
		if (ann)
		{
			if (ann->GetPath() == "")
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				str = new_folder + GETSLASH() + ann->GetName() + ".txt";
				ann->Save(str);
			}
			str = wxString::Format("/data/annotations/%d", i);
			fconfig.SetPath(str);
			str = ann->GetName();
			fconfig.Write("name", str);
			str = ann->GetPath();
			fconfig.Write("path", str);
		}
	}
	//views
	fconfig.SetPath("/views");
	fconfig.Write("num", frame->GetCanvasNum());
	for (i = 0; i < frame->GetCanvasNum(); i++)
	{
		RenderCanvas* canvas = frame->GetRenderCanvas(i);
		if (canvas)
		{
			str = wxString::Format("/views/%d", i);
			fconfig.SetPath(str);
			//view layers
			str = wxString::Format("/views/%d/layers", i);
			fconfig.SetPath(str);
			fconfig.Write("num", canvas->GetLayerNum());
			for (j = 0; j < canvas->GetLayerNum(); j++)
			{
				TreeLayer* layer = canvas->GetLayer(j);
				if (!layer)
					continue;
				str = wxString::Format("/views/%d/layers/%d", i, j);
				fconfig.SetPath(str);
				switch (layer->IsA())
				{
				case 2://volume data
					fconfig.Write("type", 2);
					fconfig.Write("name", layer->GetName());
					break;
				case 3://mesh data
					fconfig.Write("type", 3);
					fconfig.Write("name", layer->GetName());
					break;
				case 4://annotations
					fconfig.Write("type", 4);
					fconfig.Write("name", layer->GetName());
					break;
				case 5://group
				{
					DataGroup* group = (DataGroup*)layer;

					fconfig.Write("type", 5);
					fconfig.Write("name", layer->GetName());
					fconfig.Write("id", DataGroup::GetID());
					//dispaly
					fconfig.Write("display", group->GetDisp());
					//2d adjustment
					str = wxString::Format("%f %f %f", group->GetGammaColor().r(),
						group->GetGammaColor().g(), group->GetGammaColor().b());
					fconfig.Write("gamma", str);
					str = wxString::Format("%f %f %f", group->GetBrightness().r(),
						group->GetBrightness().g(), group->GetBrightness().b());
					fconfig.Write("brightness", str);
					str = wxString::Format("%f %f %f", group->GetHdr().r(),
						group->GetHdr().g(), group->GetHdr().b());
					fconfig.Write("hdr", str);
					fconfig.Write("sync_r", group->GetSync(0));
					fconfig.Write("sync_g", group->GetSync(1));
					fconfig.Write("sync_b", group->GetSync(2));
					//sync volume properties
					fconfig.Write("sync_vp", group->GetVolumeSyncProp());
					//volumes
					str = wxString::Format("/views/%d/layers/%d/volumes", i, j);
					fconfig.SetPath(str);
					fconfig.Write("num", group->GetVolumeNum());
					for (k = 0; k < group->GetVolumeNum(); k++)
						fconfig.Write(wxString::Format("vol_%d", k), group->GetVolumeData(k)->GetName());

				}
				break;
				case 6://mesh group
				{
					MeshGroup* group = (MeshGroup*)layer;

					fconfig.Write("type", 6);
					fconfig.Write("name", layer->GetName());
					fconfig.Write("id", MeshGroup::GetID());
					//display
					fconfig.Write("display", group->GetDisp());
					//sync mesh properties
					fconfig.Write("sync_mp", group->GetMeshSyncProp());
					//meshes
					str = wxString::Format("/views/%d/layers/%d/meshes", i, j);
					fconfig.SetPath(str);
					fconfig.Write("num", group->GetMeshNum());
					for (k = 0; k < group->GetMeshNum(); k++)
						fconfig.Write(wxString::Format("mesh_%d", k), group->GetMeshData(k)->GetName());
				}
				break;
				}
			}

			//tracking group
			fconfig.SetPath("/views/%d/track_group");
			int ival = canvas->GetTrackFileExist(true);
			if (ival == 1)
			{
				wxString new_folder;
				new_folder = filename2 + "_files";
				MkDirW(new_folder.ToStdWstring());
				std::wstring wstr = filename2.ToStdWstring();
				str = new_folder + GETSLASH() + GET_NAME(wstr) + ".track";
				canvas->SaveTrackGroup(str);
			}
			fconfig.Write("track_file", canvas->GetTrackGroupFile());

			//properties
			fconfig.SetPath(wxString::Format("/views/%d/properties", i));
			fconfig.Write("drawall", canvas->GetDraw());
			fconfig.Write("persp", canvas->GetPersp());
			fconfig.Write("free", canvas->GetFree());
			fconfig.Write("aov", canvas->GetAov());
			fconfig.Write("nearclip", canvas->GetNearClip());
			fconfig.Write("farclip", canvas->GetFarClip());
			fluo::Color bkcolor;
			bkcolor = canvas->GetBackgroundColor();
			str = wxString::Format("%f %f %f", bkcolor.r(), bkcolor.g(), bkcolor.b());
			fconfig.Write("backgroundcolor", str);
			fconfig.Write("drawtype", canvas->GetDrawType());
			fconfig.Write("volmethod", canvas->GetVolMethod());
			fconfig.Write("fog", canvas->GetFog());
			fconfig.Write("fogintensity", (double)canvas->GetFogIntensity());
			fconfig.Write("draw_camctr", canvas->m_draw_camctr);
			fconfig.Write("draw_info", canvas->m_draw_info);
			fconfig.Write("draw_legend", canvas->m_draw_legend);

			double x, y, z;
			//camera
			canvas->GetTranslations(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("translation", str);
			canvas->GetRotations(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("rotation", str);
			fluo::Quaternion q = canvas->GetZeroQuat();
			str = wxString::Format("%f %f %f %f", q.x, q.y, q.z, q.w);
			fconfig.Write("zero_quat", str);
			canvas->GetCenters(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("center", str);
			fconfig.Write("centereyedist", canvas->GetCenterEyeDist());
			fconfig.Write("radius", canvas->GetRadius());
			fconfig.Write("initdist", canvas->GetInitDist());
			fconfig.Write("scale_mode", canvas->m_scale_mode);
			fconfig.Write("scale", canvas->m_scale_factor);
			fconfig.Write("pin_rot_center", canvas->m_pin_rot_ctr);
			//object
			canvas->GetObjCenters(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_center", str);
			canvas->GetObjTrans(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_trans", str);
			canvas->GetObjRot(x, y, z);
			str = wxString::Format("%f %f %f", x, y, z);
			fconfig.Write("obj_rot", str);
			//scale bar
			fconfig.Write("disp_scale_bar", canvas->m_disp_scale_bar);
			fconfig.Write("disp_scale_bar_text", canvas->m_disp_scale_bar_text);
			fconfig.Write("sb_length", canvas->m_sb_length);
			str = canvas->m_sb_text;
			fconfig.Write("sb_text", str);
			str = canvas->m_sb_num;
			fconfig.Write("sb_num", str);
			fconfig.Write("sb_unit", canvas->m_sb_unit);

			//2d adjustment
			str = wxString::Format("%f %f %f", canvas->GetGammaColor().r(),
				canvas->GetGammaColor().g(), canvas->GetGammaColor().b());
			fconfig.Write("gamma", str);
			str = wxString::Format("%f %f %f", canvas->GetBrightness().r(),
				canvas->GetBrightness().g(), canvas->GetBrightness().b());
			fconfig.Write("brightness", str);
			str = wxString::Format("%f %f %f", canvas->GetHdr().r(),
				canvas->GetHdr().g(), canvas->GetHdr().b());
			fconfig.Write("hdr", str);
			fconfig.Write("sync_r", canvas->GetSync(0));
			fconfig.Write("sync_g", canvas->GetSync(1));
			fconfig.Write("sync_b", canvas->GetSync(2));

			//clipping plane rotations
			fconfig.Write("clip_mode", canvas->GetClipMode());
			double rotx_cl, roty_cl, rotz_cl;
			canvas->GetClippingPlaneRotations(rotx_cl, roty_cl, rotz_cl);
			fconfig.Write("rotx_cl", rotx_cl);
			fconfig.Write("roty_cl", roty_cl);
			fconfig.Write("rotz_cl", rotz_cl);

			//painting parameters
			fconfig.Write("brush_use_pres", glbin_vol_selector.GetBrushUsePres());
			fconfig.Write("brush_size_1", glbin_vol_selector.GetBrushSize1());
			fconfig.Write("brush_size_2", glbin_vol_selector.GetBrushSize2());
			fconfig.Write("brush_spacing", glbin_vol_selector.GetBrushSpacing());
			fconfig.Write("brush_iteration", glbin_vol_selector.GetBrushIteration());
			fconfig.Write("brush_translate", glbin_vol_selector.GetBrushSclTranslate());
			fconfig.Write("w2d", glbin_vol_selector.GetW2d());

			//rulers
			fconfig.SetPath(wxString::Format("/views/%d/rulers", i));
			glbin_ruler_handler.Save(fconfig, i);
		}
	}
	//clipping planes
	fconfig.SetPath("/prop_panel");
	fconfig.Write("clip link", glbin_settings.m_clip_link);
	fconfig.Write("clip hold", glbin_settings.m_clip_hold);
	fconfig.Write("clip mode", glbin_settings.m_clip_mode);
	fconfig.Write("x_link", frame->GetClipPlanPanel()->GetXLink());
	fconfig.Write("y_link", frame->GetClipPlanPanel()->GetYLink());
	fconfig.Write("z_link", frame->GetClipPlanPanel()->GetZLink());
	//movie view
	fconfig.SetPath("/movie_panel");
	fconfig.Write("key frame enable", glbin_moviemaker.GetKeyframeEnable());
	fconfig.Write("views_cmb", glbin_moviemaker.GetViewIndex());
	fconfig.Write("rot_check", glbin_moviemaker.GetRotateEnable());
	fconfig.Write("seq_mode", glbin_moviemaker.GetSeqMode());
	fconfig.Write("rot_axis", glbin_moviemaker.GetRotateAxis());
	fconfig.Write("rot_deg", glbin_moviemaker.GetRotateDeg());
	fconfig.Write("movie_len", glbin_moviemaker.GetMovieLength());
	fconfig.Write("fps", glbin_moviemaker.GetFps());
	fconfig.Write("crop", glbin_moviemaker.GetCropEnable());
	fconfig.Write("crop_x", glbin_moviemaker.GetCropX());
	fconfig.Write("crop_y", glbin_moviemaker.GetCropY());
	fconfig.Write("crop_w", glbin_moviemaker.GetCropW());
	fconfig.Write("crop_h", glbin_moviemaker.GetCropH());
	fconfig.Write("cur_frame", glbin_moviemaker.GetCurrentFrame());
	fconfig.Write("full frame num", glbin_moviemaker.GetFullFrameNum());
	fconfig.Write("start_frame", glbin_moviemaker.GetClipStartFrame());
	fconfig.Write("end_frame", glbin_moviemaker.GetClipEndFrame());
	fconfig.Write("run_script", glbin_settings.m_run_script);
	fconfig.Write("script_file", glbin_settings.m_script_file);
	//layout
	fconfig.SetPath("/ui_layout");
	fconfig.Write("dpi scale factor", frame->GetDPIScaleFactor());
	fconfig.Write("layout", frame->SavePerspective());
	//interpolator
	fconfig.SetPath("/interpolator");
	fconfig.Write("max_id", Interpolator::m_id);
	int group_num = glbin_interpolator.GetKeyNum();
	fconfig.Write("num", group_num);
	for (i = 0; i < group_num; i++)
	{
		FlKeyGroup* key_group = glbin_interpolator.GetKeyGroup(i);
		if (key_group)
		{
			str = wxString::Format("/interpolator/%d", i);
			fconfig.SetPath(str);
			fconfig.Write("id", key_group->id);
			fconfig.Write("t", key_group->t);
			fconfig.Write("dt", key_group->dt);
			fconfig.Write("type", key_group->type);
			str = key_group->desc;
			fconfig.Write("desc", str);
			int key_num = (int)key_group->keys.size();
			str = wxString::Format("/interpolator/%d/keys", i);
			fconfig.SetPath(str);
			fconfig.Write("num", key_num);
			for (j = 0; j < key_num; j++)
			{
				FlKey* key = key_group->keys[j];
				if (key)
				{
					str = wxString::Format("/interpolator/%d/keys/%d", i, j);
					fconfig.SetPath(str);
					int key_type = key->GetType();
					fconfig.Write("type", key_type);
					FlKeyCode code = key->GetKeyCode();
					fconfig.Write("l0", code.l0);
					str = code.l0_name;
					fconfig.Write("l0_name", str);
					fconfig.Write("l1", code.l1);
					str = code.l1_name;
					fconfig.Write("l1_name", str);
					fconfig.Write("l2", code.l2);
					str = code.l2_name;
					fconfig.Write("l2_name", str);
					switch (key_type)
					{
					case FLKEY_TYPE_DOUBLE:
					{
						double dval = ((FlKeyDouble*)key)->GetValue();
						fconfig.Write("val", dval);
					}
					break;
					case FLKEY_TYPE_QUATER:
					{
						fluo::Quaternion qval = ((FlKeyQuaternion*)key)->GetValue();
						str = wxString::Format("%lf %lf %lf %lf",
							qval.x, qval.y, qval.z, qval.w);
						fconfig.Write("val", str);
					}
					break;
					case FLKEY_TYPE_BOOLEAN:
					{
						bool bval = ((FlKeyBoolean*)key)->GetValue();
						fconfig.Write("val", bval);
					}
					break;
					case FLKEY_TYPE_INT:
					{
						int ival = ((FlKeyInt*)key)->GetValue();
						fconfig.Write("val", ival);
					}
					break;
					case FLKEY_TYPE_COLOR:
					{
						fluo::Color cval = ((FlKeyColor*)key)->GetValue();
						str = wxString::Format("%lf %lf %lf",
							cval.r(), cval.g(), cval.b());
						fconfig.Write("val", str);
					}
					break;
					}
				}
			}
		}
	}

	SaveConfig(fconfig, filename2);

	SetProgress(0, "");
	glbin_data_manager.SetProjectPath(filename2);

	frame->UpdateProps({ gstListCtrl });
}

void Project::Reset()
{
	MainFrame* frame = glbin_current.mainframe;
	if (!frame)
		return;

	glbin_data_manager.SetProjectPath("");
	//SetTitle(m_title);
	//clear
	glbin_data_manager.ClearAll();
	DataGroup::ResetID();
	MeshGroup::ResetID();
	//m_output_adj_panel->SetVolumeData(0);
	//m_output_adj_panel->SetGroup(0);
	//m_output_adj_panel->SetGroupLink(0);
	frame->GetRenderCanvas(0)->ClearAll();
	for (int i = frame->GetCanvasNum() - 1; i > 0; i--)
		frame->DeleteRenderView(i);
	RenderViewPanel::ResetID();
	glbin_current.SetRoot();
	glbin_moviemaker.Stop();
	glbin_moviemaker.SetView(frame->GetRenderCanvas(0));
	glbin_mov_def.Apply(&glbin_moviemaker);
	glbin_interpolator.Clear();
}