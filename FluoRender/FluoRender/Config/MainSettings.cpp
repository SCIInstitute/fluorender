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

#include <GL/glew.h>
#include <MainSettings.h>
#include <Names.h>
#include <Global.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>
#include <compatibility.h>
#include <filesystem>

MainSettings::MainSettings()
{
	m_dpi_scale_factor = 0.0;

	m_prj_save = false;
	m_prj_save_inc = false;
	m_time_id = L"_T";
	m_save_compress = false;
	m_override_vox = true;
	m_last_tool = 0;
	m_config_file_type = 0;
	m_capture_scale = 2.0;
	m_int_scale = 0.5;
	m_large_scale = 0.5;
	m_small_scale = 2.0;

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
	m_mov_filename = L"output.mp4";
	m_mpg_cache_size = -1;
	m_fp_convert = false;
	m_fp_min = 0;
	m_fp_max = 1;
	m_prg_size = 1e8;

	m_script_break = true;
	m_run_script = false;
	m_script_file = L"";

	m_inverse_slider = false;
	m_mulfunc = 0;
	m_time_span = 1;

	m_test_speed = false;
	m_test_param = false;
	m_test_wiref = false;

	m_peeling_layers = 1;
	m_micro_blend = false;
	m_grad_bg = false;
	m_interactive_quality = 2;//enable for large data
	m_pin_threshold = 10.0;
	m_line_width = 3.0;
	m_clip_mode = cm_Normal;
	m_clip_link = false;
	m_clip_hold = false;

	m_shadow_dir = false;
	m_shadow_dir_x = 0.0;
	m_shadow_dir_y = 0.0;

	m_wav_color1 = 5;
	m_wav_color2 = 5;
	m_wav_color3 = 5;
	m_wav_color4 = 5;

	m_disp_id = 0;
	m_hologram_mode = 0;
	m_xr_api = 1;
	m_mv_hmd = true;
	m_sbs = false;
	m_eye_dist = 20.0;
	m_holo_ip = "192.168.137.78";
	m_hologram_debug = 0;
	m_hologram_camera_mode = 0;
	m_lg_offset = 22;
	m_lg_dev_id = 0;
	m_stay_top = false;
	m_show_cursor = true;
	m_color_depth = 0;

	m_font_file = L"";
	m_text_size = 12;
	m_text_color = 0;

	m_ruler_use_transf = false;
	m_ruler_relax_f1 = 2.0;
	m_ruler_infr = 2.0;
	m_ruler_df_f = false;
	m_ruler_relax_iter = 10;
	m_ruler_relax_type = 1;
	m_ruler_size_thresh = 5;
	m_pencil_dist = 30;
	m_point_volume_mode = 2;

	m_stream_rendering = 2;//enable for large data
	m_mem_swap = false;
	m_graphics_mem = 1000.0;
	m_use_mem_limit = false;
	m_mem_limit = 1000.0;
	m_available_mem = 1000.0;
	m_large_data_size = 1000.0;
	m_small_data_size = 100.0;
	m_force_brick_size = 128;
	m_up_time = 100;
	m_update_order = 0;
	m_invalidate_tex = false;
	m_detail_level_offset = 0;
	m_inf_loop = false;

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

	m_jvm_path = L"";
	m_ij_path = L"";
	m_bioformats_path = L"";
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
	std::filesystem::path p = std::filesystem::current_path();
	std::wstring fbase = L"fluorender";
	std::wstring dft;
	//search for the file
	for (const auto& entry : std::filesystem::directory_iterator(p))
	{
		if (entry.is_regular_file())
		{
			std::wstring filename = entry.path().filename().wstring();
			std::wstring extension = entry.path().extension().wstring();
			if (filename.find(fbase) == 0 &&
				(extension == L".ini" ||
					extension == L".xml" ||
					extension == L".json"))
			{
				dft = entry.path().wstring();
			}
		}
	}

	std::shared_ptr<BaseTreeFile> fconfig =
		glbin_tree_file_factory.createTreeFile(dft, gstConfigFile);
	if (!fconfig)
		return;

	if (fconfig->LoadFile(dft))
		return;

	//project
	if (fconfig->Exists("/project"))
	{
		fconfig->SetPath("/project");
		fconfig->Read("save project", &m_prj_save, false);
		fconfig->Read("inc save", &m_prj_save_inc, false);
		fconfig->Read("time id", &m_time_id, std::wstring(L"_T"));
		fconfig->Read("save compress", &m_save_compress, false);
		fconfig->Read("override vox", &m_override_vox, true);
		fconfig->Read("last tool", &m_last_tool, 0);
		fconfig->Read("config file type", &m_config_file_type, 0);
		fconfig->Read("capture scale", &m_capture_scale, 2.0);
		fconfig->Read("int scale", &m_int_scale, 0.5);
		fconfig->Read("large scale", &m_large_scale, 0.5);
		fconfig->Read("small scale", &m_small_scale, 2.0);
	}
	//image
	if (fconfig->Exists("/image"))
	{
		fconfig->SetPath("/image");
		fconfig->Read("slice sequence", &m_slice_sequence, false);
		fconfig->Read("chann sequence", &m_chann_sequence, false);
		fconfig->Read("digit order", &m_digit_order, 0);
		fconfig->Read("ser num", &m_ser_num, 0);
		fconfig->Read("skip brick", &m_skip_brick, false);
		fconfig->Read("load mask", &m_load_mask, true);
		fconfig->Read("save crop", &m_save_crop, false);
		fconfig->Read("save filter", &m_save_filter, 0);
		fconfig->Read("vrp embed", &m_vrp_embed, false);
		fconfig->Read("save alpha", &m_save_alpha, false);
		fconfig->Read("save float", &m_save_float, false);
		fconfig->Read("dpi", &m_dpi, 72.0);
		fconfig->Read("rt compress", &m_realtime_compress, false);
		fconfig->Read("mov bitrate", &m_mov_bitrate, 20.0);
		fconfig->Read("mov filename", &m_mov_filename, std::wstring(L"output.mp4"));
		fconfig->Read("mpg cache size", &m_mpg_cache_size, -1);
		fconfig->Read("fp convert", &m_fp_convert, false);
		fconfig->Read("fp min", &m_fp_min, 0.0);
		fconfig->Read("fp max", &m_fp_max, 1.0);
		fconfig->Read("prg size", &m_prg_size, 1e8);
	}
	//script
	if (fconfig->Exists("/script"))
	{
		fconfig->SetPath("/script");
		fconfig->Read("script break", &m_script_break, true);
		fconfig->Read("script file", &m_mov_filename);
	}
	//ui
	if (fconfig->Exists("/ui"))
	{
		fconfig->SetPath("/ui");
		fconfig->Read("dpi scale factor", &m_dpi_scale_factor, 0.0);
		fconfig->Read("layout", &m_layout);
		std::string str;
		fconfig->Read("layout clip", &str);
		m_layout_clip = fconfig->DecodeXml(str);
		fconfig->Read("layout movie", &str); m_layout_movie = fconfig->DecodeXml(str);
		fconfig->Read("layout outadj", &str) ;m_layout_outadj = fconfig->DecodeXml(str);
		fconfig->Read("layout project", &str); m_layout_project = fconfig->DecodeXml(str);
		fconfig->Read("layout brush", &str); m_layout_brush = fconfig->DecodeXml(str);
		fconfig->Read("layout component", &str);m_layout_component = fconfig->DecodeXml(str);
		fconfig->Read("layout machine learning", &str);m_layout_machine_learning = fconfig->DecodeXml(str);
		fconfig->Read("layout measure", &str);m_layout_measure = fconfig->DecodeXml(str);
		fconfig->Read("layout settings", &str);m_layout_settings = fconfig->DecodeXml(str);
		fconfig->Read("layout track", &str);m_layout_track = fconfig->DecodeXml(str);
		fconfig->Read("invert slider", &m_inverse_slider, false);
		fconfig->Read("mulfunc", &m_mulfunc, 0);
		fconfig->Read("time span", &m_time_span, 1.0);
	}
	//test mode
	if (fconfig->Exists("/test mode"))
	{
		fconfig->SetPath("/test mode");
		fconfig->Read("speed", &m_test_speed, false);
		fconfig->Read("param", &m_test_param, false);
		fconfig->Read("wiref", &m_test_wiref, false);
	}
	//rendering
	if (fconfig->Exists("/rendering"))
	{
		fconfig->SetPath("/rendering");
		fconfig->Read("peeling layers", &m_peeling_layers, 1);
		fconfig->Read("micro blend", &m_micro_blend, false);
		fconfig->Read("grad bg", &m_grad_bg, false);
		fconfig->Read("interactive quality", &m_interactive_quality, 2);
		fconfig->Read("pin thresh", &m_pin_threshold, 10.0);
		fconfig->Read("line width", &m_line_width, 3.0);
		fconfig->Read("clip mode", &m_clip_mode, static_cast<int>(cm_Normal));
		fconfig->Read("clip link", &m_clip_link, false);
		fconfig->Read("clip hold", &m_clip_hold, false);
	}
	//shadow
	if (fconfig->Exists("/shadow"))
	{
		fconfig->SetPath("/shadow");
		fconfig->Read("dir enable", &m_shadow_dir, false);
		fconfig->Read("dir x", &m_shadow_dir_x, 0.0);
		fconfig->Read("dir y", &m_shadow_dir_y, 0.0);
	}
	//wavelength to color
	if (fconfig->Exists("/wavelength to color"))
	{
		fconfig->SetPath("/wavelength to color");
		fconfig->Read("c1", &m_wav_color1, 5);
		fconfig->Read("c2", &m_wav_color2, 5);
		fconfig->Read("c3", &m_wav_color3, 5);
		fconfig->Read("c4", &m_wav_color4, 5);
	}
	//display
	if (fconfig->Exists("/display"))
	{
		fconfig->SetPath("/display");
		fconfig->Read("disp id", &m_disp_id, 0);
		fconfig->Read("hologram mode", &m_hologram_mode, 0);
		fconfig->Read("xr api", &m_xr_api, 1);
		fconfig->Read("mv hmd", &m_mv_hmd, true);
		fconfig->Read("sbs enable", &m_sbs, false);
		fconfig->Read("eye dist", &m_eye_dist, 20.0);
		fconfig->Read("holo ip", &m_holo_ip, std::string("192.168.137.78"));
		fconfig->Read("hologram debug", &m_hologram_debug, 0);
		fconfig->Read("hologram camera mode", &m_hologram_camera_mode, 0);
		fconfig->Read("lg offset", &m_lg_offset, 22.0);
		fconfig->Read("lg dev id", &m_lg_dev_id, 0);
		fconfig->Read("stay top", &m_stay_top, false);
		fconfig->Read("show cursor", &m_show_cursor, true);
		fconfig->Read("color depth", &m_color_depth, 0);
	}
	//font
	if (fconfig->Exists("/font"))
	{
		fconfig->SetPath("/font");
		fconfig->Read("font file", &m_font_file);
		fconfig->Read("text size", &m_text_size, 12);
		fconfig->Read("text color", &m_text_color, 0);
	}
	//ruler settings
	if (fconfig->Exists("/ruler"))
	{
		fconfig->SetPath("/ruler");
		fconfig->Read("use transf", &m_ruler_use_transf, false);
		fconfig->Read("relax f1", &m_ruler_relax_f1, 2.0);
		fconfig->Read("infr", &m_ruler_infr, 2.0);
		fconfig->Read("df_f", &m_ruler_df_f, false);
		fconfig->Read("relax iter", &m_ruler_relax_iter, 10);
		fconfig->Read("relax type", &m_ruler_relax_type, 1);
		fconfig->Read("size thresh", &m_ruler_size_thresh, 5);
		fconfig->Read("pencil dist", &m_pencil_dist, 30.0);
		fconfig->Read("point volume", &m_point_volume_mode, 2);
	}
	//memory settings
	if (fconfig->Exists("/memory"))
	{
		fconfig->SetPath("/memory");
		fconfig->Read("stream rendering", &m_stream_rendering, 2);
		//enable mem swap
		fconfig->Read("mem swap", &m_mem_swap, false);
		//graphics memory limit
		fconfig->Read("graphics mem", &m_graphics_mem, 1000.0);
		//large data size
		fconfig->Read("large data size", &m_large_data_size, 1000.0);
		//small data size
		fconfig->Read("small data size", &m_small_data_size, 100.0);
		//force brick size
		fconfig->Read("force brick size", &m_force_brick_size, 128);
		//response time
		fconfig->Read("up time", &m_up_time, 100);
		fconfig->Read("update order", &m_update_order, 0);
		fconfig->Read("invalidate tex", &m_invalidate_tex, false);
		//detail level offset
		fconfig->Read("detail level offset", &m_detail_level_offset, 0);
		fconfig->Read("inf loop", &m_inf_loop, false);
	}
	//background removal paramters
	if (fconfig->Exists("/bg remove"))
	{
		fconfig->SetPath("/bg remove");
		fconfig->Read("bg rmv type", &m_bg_type, 0);
		fconfig->Read("bg kx", &m_kx, 100);
		fconfig->Read("bg ky", &m_ky, 100);
		fconfig->Read("bg varth", &m_varth, 0.0001);
		fconfig->Read("bg gauth", &m_gauth, 1.0);
	}
	//flags for pvxml flipping
	if (fconfig->Exists("/pvxml"))
	{
		fconfig->SetPath("/pvxml");
		fconfig->Read("flip_x", &m_pvxml_flip_x, false);
		fconfig->Read("flip_y", &m_pvxml_flip_y, false);
		fconfig->Read("seq_type", &m_pvxml_seq_type, 1);
	}
	//gpu
	if (fconfig->Exists("/gpu"))
	{
		fconfig->SetPath("/gpu");
		fconfig->Read("api_type", &m_api_type, 0);
		fconfig->Read("red_bit", &m_red_bit, 8);
		fconfig->Read("green_bit", &m_green_bit, 8);
		fconfig->Read("blue_bit", &m_blue_bit, 8);
		fconfig->Read("alpha_bit", &m_alpha_bit, 8);
		fconfig->Read("depth_bit", &m_depth_bit, 24);
		fconfig->Read("samples", &m_samples, 0);
		fconfig->Read("gl_major_ver", &m_gl_major_ver, 4);
		fconfig->Read("gl_minor_ver", &m_gl_minor_ver, 6);
		fconfig->Read("gl_profile_mask", &m_gl_profile_mask, 2);
		fconfig->Read("use_max_texture_size", &m_use_max_texture_size, false);
		fconfig->Read("max_texture_size", &m_max_texture_size, 2048);
		fconfig->Read("no_tex_pack", &m_no_tex_pack, false);
		fconfig->Read("cl_platform_id", &m_cl_platform_id, 0);
		fconfig->Read("cl_device_id", &m_cl_device_id, 0);
	}
	//tracking settings
	if (fconfig->Exists("/tracking"))
	{
		fconfig->SetPath("/tracking");
		fconfig->Read("track_iter", &m_track_iter, 3);
		fconfig->Read("component_size", &m_component_size, 25.0);
		fconfig->Read("consistent_color", &m_consistent_color, true);
		fconfig->Read("try_merge", &m_try_merge, false);
		fconfig->Read("try_split", &m_try_split, false);
		fconfig->Read("contact_factor", &m_contact_factor, 0.6);
		fconfig->Read("similarity", &m_similarity, 0.5);
	}
	// java
	if (fconfig->Exists("/java"))
	{
		fconfig->SetPath("/java");
		fconfig->Read("jvm_path", &m_jvm_path);
		fconfig->Read("ij_path", &m_ij_path);
		fconfig->Read("bioformats_path", &m_bioformats_path);
		fconfig->Read("ij_mode", &m_ij_mode, 0);
	}
	//machine learning settings
	if (fconfig->Exists("/ml"))
	{
		fconfig->SetPath("/ml");
		fconfig->Read("cg_table", &m_cg_table);
		fconfig->Read("vp_table", &m_vp_table);
		fconfig->Read("auto_start_all", &m_ml_auto_start_all, false);
		fconfig->Read("cg_auto_start", &m_cg_auto_start, false);
		fconfig->Read("vp_auto_start", &m_vp_auto_start, false);
		fconfig->Read("vp_auto_apply", &m_vp_auto_apply, false);
	}
	//python settings
	if (fconfig->Exists("/python"))
	{
		fconfig->SetPath("/python");
		fconfig->Read("version", &m_python_ver, 10);
	}

	m_automate_def.Read();
	m_brush_def.Read();
	m_comp_def.Read(gstConfigFile);
	m_outadj_def.Read();
	m_view_def.Read();
	m_vol_def.Read();
	m_movie_def.Read();
	m_colocal_def.Read();
}

void MainSettings::Save()
{
	std::shared_ptr<BaseTreeFile> fconfig =
		glbin_tree_file_factory.createTreeFile(m_config_file_type, gstConfigFile);
	if (!fconfig)
		return;

	fconfig->Write("ver_major", std::string(VERSION_MAJOR_TAG));
	fconfig->Write("ver_minor", std::string(VERSION_MINOR_TAG));

	//project
	fconfig->SetPath("/project");
	fconfig->Write("save project", m_prj_save);
	fconfig->Write("inc save", m_prj_save_inc);
	fconfig->Write("time id", m_time_id);
	fconfig->Write("save compress", m_save_compress);
	fconfig->Write("override vox", m_override_vox);
	fconfig->Write("last tool", m_last_tool);
	fconfig->Write("config file type", m_config_file_type);
	fconfig->Write("capture scale", m_capture_scale);
	fconfig->Write("int scale", m_int_scale);
	fconfig->Write("large scale", m_large_scale);
	fconfig->Write("small scale", m_small_scale);

	//image
	fconfig->SetPath("/image");
	fconfig->Write("slice sequence", m_slice_sequence);
	fconfig->Write("chann sequence", m_chann_sequence);
	fconfig->Write("digit order", m_digit_order);
	fconfig->Write("ser num", m_ser_num);
	fconfig->Write("skip brick", m_skip_brick);
	fconfig->Write("load mask", m_load_mask);
	fconfig->Write("save crop", m_save_crop);
	fconfig->Write("save filter", m_save_filter);
	fconfig->Write("vrp embed", m_vrp_embed);
	fconfig->Write("save alpha", m_save_alpha);
	fconfig->Write("save float", m_save_float);
	fconfig->Write("dpi", m_dpi);
	fconfig->Write("rt compress", m_realtime_compress);
	fconfig->Write("mov bitrate", m_mov_bitrate);
	fconfig->Write("mov filename", m_mov_filename);
	fconfig->Write("mpg cache size", m_mpg_cache_size);
	fconfig->Write("fp convert", m_fp_convert);
	fconfig->Write("fp min", m_fp_min);
	fconfig->Write("fp max", m_fp_max);
	fconfig->Write("prg size", m_prg_size);

	//script
	fconfig->SetPath("/script");
	fconfig->Write("script break", m_script_break);
	fconfig->Write("script file", m_script_file);

	//ui
	fconfig->SetPath("/ui");
	fconfig->Write("dpi scale factor", m_dpi_scale_factor);
	fconfig->Write("layout", m_layout);
	std::string str;
	str = fconfig->EncodeXml(m_layout_clip);
	fconfig->Write("layout clip", str);
	str = fconfig->EncodeXml(m_layout_movie); fconfig->Write("layout movie", str);
	str = fconfig->EncodeXml(m_layout_outadj); fconfig->Write("layout outadj", str);
	str = fconfig->EncodeXml(m_layout_project); fconfig->Write("layout project", str);
	str = fconfig->EncodeXml(m_layout_brush); fconfig->Write("layout brush", str);
	str = fconfig->EncodeXml(m_layout_component); fconfig->Write("layout component", str);
	str = fconfig->EncodeXml(m_layout_machine_learning); fconfig->Write("layout machine learning", str);
	str = fconfig->EncodeXml(m_layout_measure); fconfig->Write("layout measure", str);
	str = fconfig->EncodeXml(m_layout_settings); fconfig->Write("layout settings", str);
	str = fconfig->EncodeXml(m_layout_track); fconfig->Write("layout track", str);
	fconfig->Write("invert slider", m_inverse_slider);
	fconfig->Write("mulfunc", m_mulfunc);
	fconfig->Write("time span", m_time_span);

	//test mode
	fconfig->SetPath("/test mode");
	fconfig->Write("speed", m_test_speed);
	fconfig->Write("param", m_test_param);
	fconfig->Write("wiref", m_test_wiref);

	//rendering
	fconfig->SetPath("/rendering");
	fconfig->Write("peeling layers", m_peeling_layers);
	fconfig->Write("micro blend", m_micro_blend);
	fconfig->Write("grad bg", m_grad_bg);
	fconfig->Write("interactive quality", m_interactive_quality);
	fconfig->Write("pin thresh", m_pin_threshold);
	fconfig->Write("line width", m_line_width);
	fconfig->Write("clip mode", m_clip_mode);
	fconfig->Write("clip link", m_clip_link);
	fconfig->Write("clip hold", m_clip_hold);

	//shadow
	fconfig->SetPath("/shadow");
	fconfig->Write("dir enable", m_shadow_dir);
	fconfig->Write("dir x", m_shadow_dir_x);
	fconfig->Write("dir y", m_shadow_dir_y);

	//wavelength to color
	fconfig->SetPath("/wavelength to color");
	fconfig->Write("c1", m_wav_color1);
	fconfig->Write("c2", m_wav_color2);
	fconfig->Write("c3", m_wav_color3);
	fconfig->Write("c4", m_wav_color4);

	//display
	fconfig->SetPath("/display");
	fconfig->Write("disp id", m_disp_id);
	fconfig->Write("hologram mode", m_hologram_mode);
	fconfig->Write("xr api", m_xr_api);
	fconfig->Write("mv hmd", m_mv_hmd);
	fconfig->Write("sbs enable", m_sbs);
	fconfig->Write("eye dist", m_eye_dist);
	fconfig->Write("holo ip", m_holo_ip);
	fconfig->Write("hologram debug", m_hologram_debug);
	fconfig->Write("hologram camera mode", m_hologram_camera_mode);
	fconfig->Write("lg offset", m_lg_offset);
	fconfig->Write("lg dev id", m_lg_dev_id);
	fconfig->Write("stay top", m_stay_top);
	fconfig->Write("show cursor", m_show_cursor);
	fconfig->Write("color depth", m_color_depth);

	//font
	fconfig->SetPath("/font");
	fconfig->Write("font file", m_font_file);
	fconfig->Write("text size", m_text_size);
	fconfig->Write("text color", m_text_color);

	//ruler settings
	fconfig->SetPath("/ruler");
	fconfig->Write("use transf", m_ruler_use_transf);
	fconfig->Write("relax f1", m_ruler_relax_f1);
	fconfig->Write("infr", m_ruler_infr);
	fconfig->Write("df_f", m_ruler_df_f);
	fconfig->Write("relax iter", m_ruler_relax_iter);
	fconfig->Write("relax type", m_ruler_relax_type);
	fconfig->Write("size thresh", m_ruler_size_thresh);
	fconfig->Write("pencil dist", m_pencil_dist);
	fconfig->Write("point volume", m_point_volume_mode);

	//memory settings
	fconfig->SetPath("/memory");
	fconfig->Write("stream rendering", m_stream_rendering);
	//enable mem swap
	fconfig->Write("mem swap", m_mem_swap);
	//graphics memory limit
	fconfig->Write("graphics mem", m_graphics_mem);
	//large data size
	fconfig->Write("large data size", m_large_data_size);
	//small data size
	fconfig->Write("small data size", m_small_data_size);
	//force brick size
	fconfig->Write("force brick size", m_force_brick_size);
	//response time
	fconfig->Write("up time", m_up_time);
	fconfig->Write("update order", m_update_order);
	fconfig->Write("invalidate tex", m_invalidate_tex);
	//detail level offset
	fconfig->Write("detail level offset", m_detail_level_offset);
	fconfig->Write("inf loop", m_inf_loop);

	//background removal paramters
	fconfig->SetPath("/bg remove");
	fconfig->Write("bg rmv type", m_bg_type);
	fconfig->Write("bg kx", m_kx);
	fconfig->Write("bg ky", m_ky);
	fconfig->Write("bg varth", m_varth);
	fconfig->Write("bg gauth", m_gauth);

	//flags for pvxml flipping
	fconfig->SetPath("/pvxml");
	fconfig->Write("flip_x", m_pvxml_flip_x);
	fconfig->Write("flip_y", m_pvxml_flip_y);
	fconfig->Write("seq_type", m_pvxml_seq_type);

	//gpu
	fconfig->SetPath("/gpu");
	fconfig->Write("api_type", m_api_type);
	fconfig->Write("red_bit", m_red_bit);
	fconfig->Write("green_bit", m_green_bit);
	fconfig->Write("blue_bit", m_blue_bit);
	fconfig->Write("alpha_bit", m_alpha_bit);
	fconfig->Write("depth_bit", m_depth_bit);
	fconfig->Write("samples", m_samples);
	fconfig->Write("gl_major_ver", m_gl_major_ver);
	fconfig->Write("gl_minor_ver", m_gl_minor_ver);
	fconfig->Write("gl_profile_mask", m_gl_profile_mask);
	fconfig->Write("use_max_texture_size", m_use_max_texture_size);
	fconfig->Write("max_texture_size", m_max_texture_size);
	fconfig->Write("no_tex_pack", m_no_tex_pack);
	fconfig->Write("cl_platform_id", m_cl_platform_id);
	fconfig->Write("cl_device_id", m_cl_device_id);

	//tracking settings
	fconfig->SetPath("/tracking");
	fconfig->Write("track_iter", m_track_iter);
	fconfig->Write("component_size", m_component_size);
	fconfig->Write("consistent_color", m_consistent_color);
	fconfig->Write("try_merge", m_try_merge);
	fconfig->Write("try_split", m_try_split);
	fconfig->Write("contact_factor", m_contact_factor);
	fconfig->Write("similarity", m_similarity);

	// java
	fconfig->SetPath("/java");
	fconfig->Write("jvm_path", m_jvm_path);
	fconfig->Write("ij_path", m_ij_path);
	fconfig->Write("bioformats_path", m_bioformats_path);
	fconfig->Write("ij_mode", m_ij_mode);

	//machine learning settings
	fconfig->SetPath("/ml");
	fconfig->Write("cg_table", m_cg_table);
	fconfig->Write("vp_table", m_vp_table);
	fconfig->Write("auto_start_all", m_ml_auto_start_all);
	fconfig->Write("cg_auto_start", m_cg_auto_start);
	fconfig->Write("vp_auto_start", m_vp_auto_start);
	fconfig->Write("vp_auto_apply", m_vp_auto_apply);

	//python settings
	fconfig->SetPath("/python");
	fconfig->Write("version", m_python_ver);

	m_automate_def.Save();
	m_brush_def.Save();
	m_comp_def.Save(gstConfigFile);
	m_outadj_def.Save();
	m_view_def.Save();
	m_vol_def.Save();
	m_movie_def.Save();
	m_colocal_def.Save();

	std::filesystem::path p = std::filesystem::current_path();
	p /= "fluorender";
	switch (m_config_file_type)
	{
	case 0://ini
	default:
		p += ".ini";
		break;
	case 1://xml
		p += ".xml";
		break;
	case 2://json
		p += ".json";
		break;
	case 3://pole
		p += ".pconf";
		break;
	}
	std::wstring dft = p.wstring();
	fconfig->SaveFile(dft);
}

std::vector<std::string> MainSettings::GetJvmArgs()
{
	std::vector<std::string> args;
	args.push_back(ws2s(m_jvm_path));
	args.push_back(ws2s(m_ij_path));
	args.push_back(ws2s(m_bioformats_path));
	return args;
}

void MainSettings::GetMemorySettings()
{
	//from gl
	GLenum error = glGetError();
	GLint mem_info[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, mem_info);
	error = glGetError();
	if (error == GL_INVALID_ENUM)
	{
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, mem_info);
		error = glGetError();
		if (error == GL_INVALID_ENUM)
			m_use_mem_limit = true;
	}
	m_mem_limit = m_available_mem = m_use_mem_limit ? m_graphics_mem : mem_info[0] / 1024.0;
}