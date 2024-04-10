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

#include <MainSettings.h>
#include <Names.h>
#include <compatibility.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>


MainSettings::MainSettings()
{
	m_dpi_scale_factor = 0.0;
	m_prj_panel_split = false;
	m_clip_panel_split = false;

	m_prj_save = false;
	m_prj_save_inc = false;
	m_time_id = "_T";
	m_save_compress = false;
	m_override_vox = true;
	m_last_tool = 0;

	m_slice_sequence = false;
	m_chann_sequence = false;
	m_digit_order = 0;
	m_ser_num = 0;
	m_skip_brick = false;
	m_load_mask = true;
	m_save_crop = false;
	m_save_filter = 0;
	m_vrp_embed = false;
	m_save_alpha = false;
	m_save_float = false;
	m_dpi = 72.0;
	m_realtime_compress = false;
	m_mov_bitrate = 20.0;
	m_mov_filename = "output.mov";

	m_script_break = true;
	m_run_script = false;
	m_script_file = "";

	m_inverse_slider = false;
	m_mulfunc = 0;
	m_time_span = 1;

	m_test_speed = false;
	m_test_param = false;
	m_test_wiref = false;

	m_peeling_layers = 1;
	m_micro_blend = false;
	m_grad_bg = false;
	m_mouse_int = true;
	m_pin_threshold = 10.0;
	m_soft_threshold = 0.0;
	m_line_width = 3.0;
	m_plane_mode = 0;

	m_shadow_dir = false;
	m_shadow_dir_x = 0.0;
	m_shadow_dir_y = 0.0;

	m_wav_color1 = 5;
	m_wav_color2 = 5;
	m_wav_color3 = 5;
	m_wav_color4 = 5;

	m_disp_id = 0;
	m_stereo = false;
	m_sbs = false;
	m_eye_dist = 20.0;
	m_stay_top = false;
	m_show_cursor = true;
	m_color_depth = 0;

	m_font_file = "";
	m_text_size = 12;
	m_text_color = 0;

	m_ruler_use_transf = false;
	m_ruler_relax_f1 = 2.0;
	m_ruler_infr = 2.0;
	m_ruler_df_f = false;
	m_ruler_relax_iter = 10;
	m_ruler_auto_relax = false;
	m_ruler_relax_type = 1;
	m_ruler_size_thresh = 5;
	m_pencil_dist = 30;
	m_point_volume_mode = 0;

	m_mem_swap = false;
	m_graphics_mem = 1000.0;
	m_large_data_size = 1000.0;
	m_force_brick_size = 128;
	m_up_time = 100;
	m_update_order = 0;
	m_invalidate_tex = false;
	m_detail_level_offset = 0;

	m_bg_type = 0;
	m_kx = 100;
	m_ky = 100;
	m_varth = 0.0001;
	m_gauth = 1;

	m_pvxml_flip_x = false;
	m_pvxml_flip_y = false;
	m_pvxml_seq_type = 1;

	m_api_type = 0;
	m_red_bit = 8;
	m_green_bit = 8;
	m_blue_bit = 8;
	m_alpha_bit = 8;
	m_depth_bit = 24;
	m_samples = 0;
	m_gl_major_ver = 4;
	m_gl_minor_ver = 6;
	m_gl_profile_mask = 2;
	m_use_max_texture_size = false;
	m_max_texture_size = 2048;
	m_no_tex_pack = false;
	m_cl_platform_id = 0;
	m_cl_device_id = 0;

	m_track_iter = 3;
	m_component_size = 25.0;
	m_consistent_color = true;
	m_try_merge = false;
	m_try_split = false;
	m_contact_factor = 0.6;
	m_similarity = 0.5;

	m_jvm_path = "";
	m_ij_path = "";
	m_bioformats_path = "";
	m_ij_mode = 0;

	m_ml_auto_start_all = false;
	m_cg_auto_start = false;
	m_vp_auto_start = false;
	m_vp_auto_apply = false;

	m_python_ver = 10;
}

MainSettings::~MainSettings()
{

}

void MainSettings::Read()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "fluorender.ini";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//project
	if (fconfig.Exists("/project"))
	{
		fconfig.SetPath("/project");
		fconfig.Read("save project", &m_prj_save, false);
		fconfig.Read("inc save", &m_prj_save_inc, false);
		fconfig.Read("time id", &m_time_id, "_T");
		fconfig.Read("save compress", &m_save_compress, false);
		fconfig.Read("override vox", &m_override_vox, true);
		fconfig.Read("last tool", &m_last_tool, 0);
	}
	//image
	if (fconfig.Exists("/image"))
	{
		fconfig.SetPath("/image");
		fconfig.Read("slice sequence", &m_slice_sequence, false);
		fconfig.Read("chann sequence", &m_chann_sequence, false);
		fconfig.Read("digit order", &m_digit_order, 0);
		fconfig.Read("ser num", &m_ser_num, 0);
		fconfig.Read("skip brick", &m_skip_brick, false);
		fconfig.Read("load mask", &m_load_mask, true);
		fconfig.Read("save crop", &m_save_crop, false);
		fconfig.Read("save filter", &m_save_filter, 0);
		fconfig.Read("vrp embed", &m_vrp_embed, false);
		fconfig.Read("save alpha", &m_save_alpha, false);
		fconfig.Read("save float", &m_save_float, false);
		fconfig.Read("dpi", &m_dpi, 72.0);
		fconfig.Read("rt compress", &m_realtime_compress, false);
		fconfig.Read("mov bitrate", &m_mov_bitrate, 20.0);
		fconfig.Read("mov filename", &m_mov_filename, "output.mov");
	}
	//script
	if (fconfig.Exists("/script"))
	{
		fconfig.SetPath("/script");
		fconfig.Read("script break", &m_script_break, true);
		fconfig.Read("script file", &m_script_file, "");
	}
	//ui
	if (fconfig.Exists("/ui"))
	{
		fconfig.SetPath("/ui");
		fconfig.Read("dpi scale factor", &m_dpi_scale_factor, 0.0);
		fconfig.Read("layout", &m_layout);
		fconfig.Read("prj panel split", &m_prj_panel_split, false);
		fconfig.Read("clip panel split", &m_clip_panel_split, false);
		fconfig.Read("invert slider", &m_inverse_slider, false);
		fconfig.Read("mulfunc", &m_mulfunc, 0);
		fconfig.Read("time span", &m_time_span, 1);
	}
	//test mode
	if (fconfig.Exists("/test mode"))
	{
		fconfig.SetPath("/test mode");
		fconfig.Read("speed", &m_test_speed, false);
		fconfig.Read("param", &m_test_param, false);
		fconfig.Read("wiref", &m_test_wiref, false);
	}
	//rendering
	if (fconfig.Exists("/rendering"))
	{
		fconfig.SetPath("/rendering");
		fconfig.Read("peeling layers", &m_peeling_layers, 1);
		fconfig.Read("micro blend", &m_micro_blend, false);
		fconfig.Read("grad bg", &m_grad_bg, false);
		fconfig.Read("mouse int", &m_mouse_int, true);
		fconfig.Read("pin thresh", &m_pin_threshold, 10.0);
		fconfig.Read("soft thresh", &m_soft_threshold, 0.0);
		fconfig.Read("line width", &m_line_width, 3.0);
		fconfig.Read("clip plane disp", &m_plane_mode, 0);
	}
	//shadow
	if (fconfig.Exists("/shadow"))
	{
		fconfig.SetPath("/shadow");
		fconfig.Read("dir enable", &m_shadow_dir, false);
		fconfig.Read("dir x", &m_shadow_dir_x, 0.0);
		fconfig.Read("dir y", &m_shadow_dir_y, 0.0);
	}
	//wavelength to color
	if (fconfig.Exists("/wavelength to color"))
	{
		fconfig.SetPath("/wavelength to color");
		fconfig.Read("c1", &m_wav_color1, 5);
		fconfig.Read("c2", &m_wav_color2, 5);
		fconfig.Read("c3", &m_wav_color3, 5);
		fconfig.Read("c4", &m_wav_color4, 5);
	}
	//display
	if (fconfig.Exists("/display"))
	{
		fconfig.SetPath("/display");
		fconfig.Read("disp id", &m_disp_id, 0);
		fconfig.Read("stereo enable", &m_stereo, false);
		fconfig.Read("sbs enable", &m_sbs, false);
		fconfig.Read("eye dist", &m_eye_dist, 20.0);
		fconfig.Read("stay top", &m_stay_top, false);
		fconfig.Read("show cursor", &m_show_cursor, true);
		fconfig.Read("color depth", &m_color_depth, 0);
	}
	//font
	if (fconfig.Exists("/font"))
	{
		fconfig.SetPath("/font");
		fconfig.Read("font file", &m_font_file, "");
		fconfig.Read("text size", &m_text_size, 12);
		fconfig.Read("text color", &m_text_color, 0);
	}
	//ruler settings
	if (fconfig.Exists("/ruler"))
	{
		fconfig.SetPath("/ruler");
		fconfig.Read("use transf", &m_ruler_use_transf, false);
		fconfig.Read("relax f1", &m_ruler_relax_f1, 2.0);
		fconfig.Read("infr", &m_ruler_infr, 2.0);
		fconfig.Read("df_f", &m_ruler_df_f, false);
		fconfig.Read("relax iter", &m_ruler_relax_iter, 10);
		fconfig.Read("auto relax", &m_ruler_auto_relax, false);
		fconfig.Read("relax type", &m_ruler_relax_type, 1);
		fconfig.Read("size thresh", &m_ruler_size_thresh, 5);
		fconfig.Read("pencil dist", &m_pencil_dist, 30);
		fconfig.Read("point volume", &m_point_volume_mode, 0);
	}
	//memory settings
	if (fconfig.Exists("/memory"))
	{
		fconfig.SetPath("/memory");
		//enable mem swap
		fconfig.Read("mem swap", &m_mem_swap, false);
		//graphics memory limit
		fconfig.Read("graphics mem", &m_graphics_mem, 1000.0);
		//large data size
		fconfig.Read("large data size", &m_large_data_size, 1000.0);
		//force brick size
		fconfig.Read("force brick size", &m_force_brick_size, 128);
		//response time
		fconfig.Read("up time", &m_up_time, 100);
		fconfig.Read("update order", &m_update_order, 0);
		fconfig.Read("invalidate tex", &m_invalidate_tex, false);
		//detail level offset
		fconfig.Read("detail level offset", &m_detail_level_offset, 0);
	}
	//background removal paramters
	if (fconfig.Exists("/bg remove"))
	{
		fconfig.SetPath("/bg remove");
		fconfig.Read("bg rmv type", &m_bg_type, 0);
		fconfig.Read("bg kx", &m_kx, 100);
		fconfig.Read("bg ky", &m_ky, 100);
		fconfig.Read("bg varth", &m_varth, 0.0001);
		fconfig.Read("bg gauth", &m_gauth, 1);
	}
	//flags for pvxml flipping
	if (fconfig.Exists("/pvxml"))
	{
		fconfig.SetPath("/pvxml");
		fconfig.Read("flip_x", &m_pvxml_flip_x, false);
		fconfig.Read("flip_y", &m_pvxml_flip_y, false);
		fconfig.Read("seq_type", &m_pvxml_seq_type, 1);
	}
	//gpu
	if (fconfig.Exists("/gpu"))
	{
		fconfig.SetPath("/gpu");
		fconfig.Read("api_type", &m_api_type, 0);
		fconfig.Read("red_bit", &m_red_bit, 8);
		fconfig.Read("green_bit", &m_green_bit, 8);
		fconfig.Read("blue_bit", &m_blue_bit, 8);
		fconfig.Read("alpha_bit", &m_alpha_bit, 8);
		fconfig.Read("depth_bit", &m_depth_bit, 24);
		fconfig.Read("samples", &m_samples, 0);
		fconfig.Read("gl_major_ver", &m_gl_major_ver, 4);
		fconfig.Read("gl_minor_ver", &m_gl_minor_ver, 6);
		fconfig.Read("gl_profile_mask", &m_gl_profile_mask, 2);
		fconfig.Read("use_max_texture_size", &m_use_max_texture_size, false);
		fconfig.Read("max_texture_size", &m_max_texture_size, 2048);
		fconfig.Read("no_tex_pack", &m_no_tex_pack, false);
		fconfig.Read("cl_platform_id", &m_cl_platform_id, 0);
		fconfig.Read("cl_device_id", &m_cl_device_id, 0);
	}
	//tracking settings
	if (fconfig.Exists("/tracking"))
	{
		fconfig.SetPath("/tracking");
		fconfig.Read("track_iter", &m_track_iter, 3);
		fconfig.Read("component_size", &m_component_size, 25.0);
		fconfig.Read("consistent_color", &m_consistent_color, true);
		fconfig.Read("try_merge", &m_try_merge, false);
		fconfig.Read("try_split", &m_try_split, false);
		fconfig.Read("contact_factor", &m_contact_factor, 0.6);
		fconfig.Read("similarity", &m_similarity, 0.5);
	}
	// java
	if (fconfig.Exists("/java"))
	{
		fconfig.SetPath("/java");
		fconfig.Read("jvm_path", &m_jvm_path, "");
		fconfig.Read("ij_path", &m_ij_path, "");
		fconfig.Read("bioformats_path", &m_bioformats_path, "");
		fconfig.Read("ij_mode", &m_ij_mode, 0);
	}
	//machine learning settings
	if (fconfig.Exists("/ml"))
	{
		wxString str;
		fconfig.SetPath("/ml");
		fconfig.Read("cg_table", &str, "");
		m_cg_table = str.ToStdString();
		fconfig.Read("vp_table", &str, "");
		m_vp_table = str.ToStdString();
		fconfig.Read("auto_start_all", &m_ml_auto_start_all, false);
		fconfig.Read("cg_auto_start", &m_cg_auto_start, false);
		fconfig.Read("vp_auto_start", &m_vp_auto_start, false);
		fconfig.Read("vp_auto_apply", &m_vp_auto_apply, false);
	}
	//python settings
	if (fconfig.Exists("/python"))
	{
		fconfig.SetPath("/python");
		fconfig.Read("version", &m_python_ver, 10);
	}

	//EnableStreaming(m_mem_swap);
	m_brush_def.Read(fconfig);
	m_comp_def.Read(fconfig);
	m_outadj_def.Read(fconfig);
	m_view_def.Read(fconfig);
	m_vol_def.Read(fconfig);
	m_movie_def.Read(fconfig);
}

void MainSettings::Save()
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "fluorender.ini";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	fconfig.Write("ver_major", VERSION_MAJOR_TAG);
	fconfig.Write("ver_minor", VERSION_MINOR_TAG);

	//project
	fconfig.SetPath("/project");
	fconfig.Write("save project", m_prj_save);
	fconfig.Write("inc save", m_prj_save_inc);
	fconfig.Write("time id", m_time_id);
	fconfig.Write("save compress", m_save_compress);
	fconfig.Write("override vox", m_override_vox);
	fconfig.Write("last tool", m_last_tool);

	//image
	fconfig.SetPath("/image");
	fconfig.Write("slice sequence", m_slice_sequence);
	fconfig.Write("chann sequence", m_chann_sequence);
	fconfig.Write("digit order", m_digit_order);
	fconfig.Write("ser num", m_ser_num);
	fconfig.Write("skip brick", m_skip_brick);
	fconfig.Write("load mask", m_load_mask);
	fconfig.Write("save crop", m_save_crop);
	fconfig.Write("save filter", m_save_filter);
	fconfig.Write("vrp embed", m_vrp_embed);
	fconfig.Write("save alpha", m_save_alpha);
	fconfig.Write("save float", m_save_float);
	fconfig.Write("dpi", m_dpi);
	fconfig.Write("rt compress", m_realtime_compress);
	fconfig.Write("mov bitrate", m_mov_bitrate);
	fconfig.Write("mov filename", m_mov_filename);

	//script
	fconfig.SetPath("/script");
	fconfig.Write("script break", m_script_break);
	fconfig.Write("script file", m_script_file);

	//ui
	fconfig.SetPath("/ui");
	fconfig.Write("dpi scale factor", m_dpi_scale_factor);
	fconfig.Write("layout", m_layout);
	fconfig.Write("prj panel split", m_prj_panel_split);
	fconfig.Write("clip panel split", m_clip_panel_split);
	fconfig.Write("invert slider", m_inverse_slider);
	fconfig.Write("mulfunc", m_mulfunc);
	fconfig.Write("time span", m_time_span);

	//test mode
	fconfig.SetPath("/test mode");
	fconfig.Write("speed", m_test_speed);
	fconfig.Write("param", m_test_param);
	fconfig.Write("wiref", m_test_wiref);

	//rendering
	fconfig.SetPath("/rendering");
	fconfig.Write("peeling layers", m_peeling_layers);
	fconfig.Write("micro blend", m_micro_blend);
	fconfig.Write("grad bg", m_grad_bg);
	fconfig.Write("mouse int", m_mouse_int);
	fconfig.Write("pin thresh", m_pin_threshold);
	fconfig.Write("soft thresh", m_soft_threshold);
	fconfig.Write("line width", m_line_width);
	fconfig.Write("clip plane disp", m_plane_mode);

	//shadow
	fconfig.SetPath("/shadow");
	fconfig.Write("dir enable", m_shadow_dir);
	fconfig.Write("dir x", m_shadow_dir_x);
	fconfig.Write("dir y", m_shadow_dir_y);

	//wavelength to color
	fconfig.SetPath("/wavelength to color");
	fconfig.Write("c1", m_wav_color1);
	fconfig.Write("c2", m_wav_color2);
	fconfig.Write("c3", m_wav_color3);
	fconfig.Write("c4", m_wav_color4);

	//display
	fconfig.SetPath("/display");
	fconfig.Write("disp id", m_disp_id);
	fconfig.Write("stereo enable", m_stereo);
	fconfig.Write("sbs enable", m_sbs);
	fconfig.Write("eye dist", m_eye_dist);
	fconfig.Write("stay top", m_stay_top);
	fconfig.Write("show cursor", m_show_cursor);
	fconfig.Write("color depth", m_color_depth);

	//font
	fconfig.SetPath("/font");
	fconfig.Write("font file", m_font_file);
	fconfig.Write("text size", m_text_size);
	fconfig.Write("text color", m_text_color);

	//ruler settings
	fconfig.SetPath("/ruler");
	fconfig.Write("use transf", m_ruler_use_transf);
	fconfig.Write("relax f1", m_ruler_relax_f1);
	fconfig.Write("infr", m_ruler_infr);
	fconfig.Write("df_f", m_ruler_df_f);
	fconfig.Write("relax iter", m_ruler_relax_iter);
	fconfig.Write("auto relax", m_ruler_auto_relax);
	fconfig.Write("relax type", m_ruler_relax_type);
	fconfig.Write("size thresh", m_ruler_size_thresh);
	fconfig.Write("pencil dist", m_pencil_dist);
	fconfig.Write("point volume", m_point_volume_mode);

	//memory settings
	fconfig.SetPath("/memory");
	//enable mem swap
	fconfig.Write("mem swap", m_mem_swap);
	//graphics memory limit
	fconfig.Write("graphics mem", m_graphics_mem);
	//large data size
	fconfig.Write("large data size", m_large_data_size);
	//force brick size
	fconfig.Write("force brick size", m_force_brick_size);
	//response time
	fconfig.Write("up time", m_up_time);
	fconfig.Write("update order", m_update_order);
	fconfig.Write("invalidate tex", m_invalidate_tex);
	//detail level offset
	fconfig.Write("detail level offset", m_detail_level_offset);

	//background removal paramters
	fconfig.SetPath("/bg remove");
	fconfig.Write("bg rmv type", m_bg_type);
	fconfig.Write("bg kx", m_kx);
	fconfig.Write("bg ky", m_ky);
	fconfig.Write("bg varth", m_varth);
	fconfig.Write("bg gauth", m_gauth);

	//flags for pvxml flipping
	fconfig.SetPath("/pvxml");
	fconfig.Write("flip_x", m_pvxml_flip_x);
	fconfig.Write("flip_y", m_pvxml_flip_y);
	fconfig.Write("seq_type", m_pvxml_seq_type);

	//gpu
	fconfig.SetPath("/gpu");
	fconfig.Write("api_type", m_api_type);
	fconfig.Write("red_bit", m_red_bit);
	fconfig.Write("green_bit", m_green_bit);
	fconfig.Write("blue_bit", m_blue_bit);
	fconfig.Write("alpha_bit", m_alpha_bit);
	fconfig.Write("depth_bit", m_depth_bit);
	fconfig.Write("samples", m_samples);
	fconfig.Write("gl_major_ver", m_gl_major_ver);
	fconfig.Write("gl_minor_ver", m_gl_minor_ver);
	fconfig.Write("gl_profile_mask", m_gl_profile_mask);
	fconfig.Write("use_max_texture_size", m_use_max_texture_size);
	fconfig.Write("max_texture_size", m_max_texture_size);
	fconfig.Write("no_tex_pack", m_no_tex_pack);
	fconfig.Write("cl_platform_id", m_cl_platform_id);
	fconfig.Write("cl_device_id", m_cl_device_id);

	//tracking settings
	fconfig.SetPath("/tracking");
	fconfig.Write("track_iter", m_track_iter);
	fconfig.Write("component_size", m_component_size);
	fconfig.Write("consistent_color", m_consistent_color);
	fconfig.Write("try_merge", m_try_merge);
	fconfig.Write("try_split", m_try_split);
	fconfig.Write("contact_factor", m_contact_factor);
	fconfig.Write("similarity", m_similarity);

	// java
	fconfig.SetPath("/java");
	fconfig.Write("jvm_path", m_jvm_path);
	fconfig.Write("ij_path", m_ij_path);
	fconfig.Write("bioformats_path", m_bioformats_path);
	fconfig.Write("ij_mode", m_ij_mode);

	//machine learning settings
	fconfig.SetPath("/ml");
	fconfig.Write("cg_table", wxString(m_cg_table));
	fconfig.Write("vp_table", wxString(m_vp_table));
	fconfig.Write("auto_start_all", m_ml_auto_start_all);
	fconfig.Write("cg_auto_start", m_cg_auto_start);
	fconfig.Write("vp_auto_start", m_vp_auto_start);
	fconfig.Write("vp_auto_apply", m_vp_auto_apply);

	//python settings
	fconfig.SetPath("/python");
	fconfig.Write("version", m_python_ver);

	m_brush_def.Save(fconfig);
	m_comp_def.Save(fconfig);
	m_outadj_def.Save(fconfig);
	m_view_def.Save(fconfig);
	m_vol_def.Save(fconfig);
	m_movie_def.Save(fconfig);

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "fluorender.ini";
	SaveConfig(fconfig, dft);
}

