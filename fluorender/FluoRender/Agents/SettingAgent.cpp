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

#include <SettingAgent.hpp>
#include <SettingDlg.h>

using namespace fluo;

SettingAgent::SettingAgent(SettingDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void SettingAgent::setObject(Root* obj)
{
	InterfaceAgent::setObject(obj);
}

Root* SettingAgent::getObject()
{
	return dynamic_cast<Root*>(InterfaceAgent::getObject());
}

void SettingAgent::UpdateAllSettings()
{
	//update user interface
	//save project
	m_prj_save_chk->SetValue(m_prj_save);
	//realtime compression
	m_realtime_cmp_chk->SetValue(m_realtime_compress);
	//mouse interactions
	m_mouse_int_chk->SetValue(m_mouse_int);
	//depth peeling
	m_peeling_layers_sldr->SetValue(m_peeling_layers);
	m_peeling_layers_text->ChangeValue(wxString::Format("%d", m_peeling_layers));
	//micro blending
	m_micro_blend_chk->SetValue(m_micro_blend);
	//shadow direction
	if (m_shadow_dir)
	{
		m_shadow_dir_chk->SetValue(true);
		m_shadow_dir_sldr->Enable();
		m_shadow_dir_text->Enable();
	}
	else
	{
		m_shadow_dir_chk->SetValue(false);
		m_shadow_dir_sldr->Disable();
		m_shadow_dir_text->Disable();
		m_shadow_dir_x = 0.0;
		m_shadow_dir_y = 0.0;
	}
	double deg = GetShadowDir();
	//	double deg = fluo::r2d(atan2(m_shadow_dir_y, m_shadow_dir_x));
	m_shadow_dir_sldr->SetValue(int(deg + 0.5));
	m_shadow_dir_text->ChangeValue(wxString::Format("%.2f", deg));
	//rot center anchor thresh
	m_pin_threshold_sldr->SetValue(int(m_pin_threshold*10.0));
	m_pin_threshold_text->ChangeValue(wxString::Format("%.0f", m_pin_threshold*100.0));
	//gradient background
	m_grad_bg_chk->SetValue(m_grad_bg);
	//stereo
	m_stereo_chk->SetValue(m_stereo);
	m_eye_dist_sldr->SetValue(int(m_eye_dist*10.0));
	m_eye_dist_text->ChangeValue(wxString::Format("%.1f", m_eye_dist));
	//override vox
	m_override_vox_chk->SetValue(m_override_vox);
	//wavelength to color
	m_wav_color1_cmb->Select(m_wav_color1 - 1);
	m_wav_color2_cmb->Select(m_wav_color2 - 1);
	m_wav_color3_cmb->Select(m_wav_color3 - 1);
	m_wav_color4_cmb->Select(m_wav_color4 - 1);
	//max texture size
	m_max_texture_size_chk->SetValue(m_use_max_texture_size);
	if (m_use_max_texture_size)
	{
		flvr::ShaderProgram::set_max_texture_size(m_max_texture_size);
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", m_max_texture_size));
		m_max_texture_size_text->Enable();
	}
	else
	{
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", flvr::ShaderProgram::
				max_texture_size()));
		m_max_texture_size_text->Disable();
	}
	//no tex pack
	flvr::ShaderProgram::set_no_tex_upack(m_no_tex_pack);
	//font
	wxString str = m_font_file.BeforeLast('.');
	int font_sel = m_font_cmb->FindString(str);
	if (font_sel != wxNOT_FOUND)
		m_font_cmb->Select(font_sel);
	long font_size;
	for (unsigned int i = 0; i < m_font_size_cmb->GetCount(); ++i)
	{
		str = m_font_size_cmb->GetString(i);
		if (str.ToLong(&font_size) &&
			font_size <= m_text_size)
			m_font_size_cmb->Select(i);
	}
	m_text_color_cmb->Select(m_text_color);
	//line width
	m_line_width_text->SetValue(wxString::Format("%.0f", m_line_width));
	m_line_width_sldr->SetValue(int(m_line_width + 0.5));
	//paint history depth
	m_paint_hist_depth_text->ChangeValue(wxString::Format("%d", m_paint_hist_depth));
	m_paint_hist_depth_sldr->SetValue(m_paint_hist_depth);
	//memory settings
	m_streaming_chk->SetValue(m_mem_swap);
	EnableStreaming(m_mem_swap);
	m_update_order_rbox->SetSelection(m_update_order);
	m_graphics_mem_text->ChangeValue(wxString::Format("%d", (int)m_graphics_mem));
	m_graphics_mem_sldr->SetValue((int)(m_graphics_mem / 100.0));
	m_large_data_text->ChangeValue(wxString::Format("%d", (int)m_large_data_size));
	m_large_data_sldr->SetValue((int)(m_large_data_size / 10.0));
	m_block_size_text->ChangeValue(wxString::Format("%d", m_force_brick_size));
	m_block_size_sldr->SetValue(int(log(m_force_brick_size) / log(2.0) + 0.5));
	m_response_time_text->ChangeValue(wxString::Format("%d", m_up_time));
	m_response_time_sldr->SetValue(int(m_up_time / 10.0));
	m_detail_level_offset_text->ChangeValue(wxString::Format("%d", -m_detail_level_offset));
	m_detail_level_offset_sldr->SetValue(-m_detail_level_offset);

	//java
	m_java_jvm_text->SetValue(m_jvm_path);
	m_java_ij_text->SetValue(m_ij_path);
	m_java_bioformats_text->SetValue(m_bioformats_path);
	switch (m_ij_mode)
	{
	case 0:
		mp_radio_button_imagej->SetValue(true);
		m_java_jvm_text->Enable(true);
		m_java_bioformats_text->Enable(true);
		m_browse_jvm_btn->Enable(true);
		m_browse_bioformats_btn->Enable(true);
		break;
	case 1:
		mp_radio_button_fiji->SetValue(true);
		m_java_jvm_text->Enable(false);
		m_java_bioformats_text->Enable(false);
		m_browse_jvm_btn->Enable(false);
		m_browse_bioformats_btn->Enable(false);
		break;
	}
}

void SettingAgent::ReadSettings()
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "fluorender.set";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//gradient magnitude
	if (fconfig.Exists("/gm calculation"))
	{
		fconfig.SetPath("/gm calculation");
		m_gmc_mode = fconfig.Read("mode", 2l);
	}
	//depth peeling
	if (fconfig.Exists("/peeling layers"))
	{
		fconfig.SetPath("/peeling layers");
		m_peeling_layers = fconfig.Read("value", 1l);
	}
	//micro blending
	if (fconfig.Exists("/micro blend"))
	{
		fconfig.SetPath("/micro blend");
		fconfig.Read("mode", &m_micro_blend, false);
	}
	//save project
	if (fconfig.Exists("/save project"))
	{
		fconfig.SetPath("/save project");
		fconfig.Read("mode", &m_prj_save, false);
	}
	//save alpha
	if (fconfig.Exists("/save alpha"))
	{
		fconfig.SetPath("/save alpha");
		fconfig.Read("mode", &m_save_alpha, false);
	}
	//save float
	if (fconfig.Exists("/save float"))
	{
		fconfig.SetPath("/save float");
		fconfig.Read("mode", &m_save_float, false);
	}
	//realtime compression
	if (fconfig.Exists("/realtime compress"))
	{
		fconfig.SetPath("/realtime compress");
		fconfig.Read("mode", &m_realtime_compress, false);
	}
	//skip empty bricks
	if (fconfig.Exists("/skip bricks"))
	{
		fconfig.SetPath("/skip bricks");
		fconfig.Read("mode", &m_skip_bricks, false);
	}
	//mouse interactions
	if (fconfig.Exists("/mouse int"))
	{
		fconfig.SetPath("/mouse int");
		fconfig.Read("mode", &m_mouse_int, true);
	}
	//shadow
	if (fconfig.Exists("/dir shadow"))
	{
		fconfig.SetPath("/dir shadow");
		fconfig.Read("mode", &m_shadow_dir, false);
		fconfig.Read("x", &m_shadow_dir_x, 0.0);
		fconfig.Read("y", &m_shadow_dir_y, 0.0);
	}
	//rot center anchor thresh
	if (fconfig.Exists("/pin threshold"))
	{
		fconfig.SetPath("/pin threshold");
		fconfig.Read("value", &m_pin_threshold, 10.0);
	}
	//stereo
	if (fconfig.Exists("/stereo"))
	{
		fconfig.SetPath("/stereo");
		fconfig.Read("enable_stereo", &m_stereo, false);
		fconfig.Read("eye dist", &m_eye_dist, 20.0);
	}
	//test mode
	if (fconfig.Exists("/test mode"))
	{
		fconfig.SetPath("/test mode");
		fconfig.Read("speed", &m_test_speed, false);
		fconfig.Read("param", &m_test_param, false);
		fconfig.Read("wiref", &m_test_wiref, false);
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
	//time sequence identifier
	if (fconfig.Exists("/time id"))
	{
		fconfig.SetPath("/time id");
		fconfig.Read("value", &m_time_id);
	}
	//gradient background
	if (fconfig.Exists("/grad bg"))
	{
		fconfig.SetPath("/grad bg");
		fconfig.Read("value", &m_grad_bg);
	}
	//save compressed
	if (fconfig.Exists("/save cmp"))
	{
		bool save_cmp;
		fconfig.SetPath("/save cmp");
		fconfig.Read("value", &save_cmp);
		RenderFrame::SetCompression(save_cmp);
	}
	//override vox
	if (fconfig.Exists("/override vox"))
	{
		fconfig.SetPath("/override vox");
		fconfig.Read("value", &m_override_vox);
	}
	//soft threshold
	if (fconfig.Exists("/soft threshold"))
	{
		fconfig.SetPath("/soft threshold");
		fconfig.Read("value", &m_soft_threshold);
	}
	//run script
	if (fconfig.Exists("/run script"))
	{
		fconfig.SetPath("/run script");
		fconfig.Read("value", &m_run_script);
		fconfig.Read("file", &m_script_file);
	}
	//paint history depth
	if (fconfig.Exists("/paint history"))
	{
		fconfig.SetPath("/paint history");
		fconfig.Read("value", &m_paint_hist_depth);
	}
	//text font
	if (fconfig.Exists("/text font"))
	{
		fconfig.SetPath("/text font");
		fconfig.Read("file", &m_font_file);
		fconfig.Read("value", &m_text_size);
		fconfig.Read("color", &m_text_color);
	}
	//line width
	if (fconfig.Exists("/line width"))
	{
		fconfig.SetPath("/line width");
		fconfig.Read("value", &m_line_width);
	}
	//full screen
	if (fconfig.Exists("/full screen"))
	{
		fconfig.SetPath("/full screen");
		fconfig.Read("stay top", &m_stay_top);
		fconfig.Read("show cursor", &m_show_cursor);
	}
	//last tool
	if (fconfig.Exists("/last tool"))
	{
		fconfig.SetPath("/last tool");
		fconfig.Read("value", &m_last_tool);
	}
	//memory settings
	if (fconfig.Exists("/memory settings"))
	{
		fconfig.SetPath("/memory settings");
		//enable mem swap
		fconfig.Read("mem swap", &m_mem_swap);
		//graphics memory limit
		fconfig.Read("graphics mem", &m_graphics_mem);
		//large data size
		fconfig.Read("large data size", &m_large_data_size);
		//force brick size
		fconfig.Read("force brick size", &m_force_brick_size);
		//response time
		fconfig.Read("up time", &m_up_time);
		//detail level offset
		fconfig.Read("detail level offset", &m_detail_level_offset);
	}
	EnableStreaming(m_mem_swap);
	//update order
	if (fconfig.Exists("/update order"))
	{
		fconfig.SetPath("/update order");
		fconfig.Read("value", &m_update_order);
	}
	//invalidate texture
	if (fconfig.Exists("/invalidate tex"))
	{
		fconfig.SetPath("/invalidate tex");
		fconfig.Read("value", &m_invalidate_tex);
	}
	//point volume mode
	if (fconfig.Exists("/point volume mode"))
	{
		fconfig.SetPath("/point volume mode");
		fconfig.Read("value", &m_point_volume_mode);
	}
	//ruler settings
	if (fconfig.Exists("/ruler"))
	{
		fconfig.SetPath("/ruler");
		fconfig.Read("use transf", &m_ruler_use_transf);
		fconfig.Read("time dep", &m_ruler_time_dep);
		fconfig.Read("relax f1", &m_ruler_relax_f1);
		fconfig.Read("infr", &m_ruler_infr);
		fconfig.Read("df_f", &m_ruler_df_f);
		fconfig.Read("relax iter", &m_ruler_relax_iter);
		fconfig.Read("auto relax", &m_ruler_auto_relax);
		fconfig.Read("relax type", &m_ruler_relax_type);
		fconfig.Read("size thresh", &m_ruler_size_thresh);
	}
	//flags for pvxml flipping
	if (fconfig.Exists("/pvxml"))
	{
		fconfig.SetPath("/pvxml");
		fconfig.Read("flip_x", &m_pvxml_flip_x);
		fconfig.Read("flip_y", &m_pvxml_flip_y);
		fconfig.Read("seq_type", &m_pvxml_seq_type);
	}
	//pixel format
	if (fconfig.Exists("/pixel format"))
	{
		fconfig.SetPath("/pixel format");
		fconfig.Read("api_type", &m_api_type);
		fconfig.Read("red_bit", &m_red_bit);
		fconfig.Read("green_bit", &m_green_bit);
		fconfig.Read("blue_bit", &m_blue_bit);
		fconfig.Read("alpha_bit", &m_alpha_bit);
		fconfig.Read("depth_bit", &m_depth_bit);
		fconfig.Read("samples", &m_samples);
	}
	//tracking settings
	if (fconfig.Exists("/tracking"))
	{
		fconfig.SetPath("/tracking");
		fconfig.Read("track_iter", &m_track_iter);
		fconfig.Read("component_size", &m_component_size);
		fconfig.Read("consistent_color", &m_consistent_color);
		fconfig.Read("try_merge", &m_try_merge);
		fconfig.Read("try_split", &m_try_split);
		fconfig.Read("contact_factor", &m_contact_factor);
		fconfig.Read("similarity", &m_similarity);
	}
	//context attrib
	if (fconfig.Exists("/context attrib"))
	{
		fconfig.SetPath("/context attrib");
		fconfig.Read("gl_major_ver", &m_gl_major_ver);
		fconfig.Read("gl_minor_ver", &m_gl_minor_ver);
		fconfig.Read("gl_profile_mask", &m_gl_profile_mask);
	}
	//max texture size
	if (fconfig.Exists("/max texture size"))
	{
		fconfig.SetPath("/max texture size");
		fconfig.Read("use_max_texture_size", &m_use_max_texture_size);
		fconfig.Read("max_texture_size", &m_max_texture_size);
	}
	//no tex pack
	if (fconfig.Exists("/no tex pack"))
	{
		fconfig.SetPath("/no tex pack");
		fconfig.Read("no_tex_pack", &m_no_tex_pack);
	}
	//cl device
	if (fconfig.Exists("/cl device"))
	{
		fconfig.SetPath("/cl device");
		fconfig.Read("platform_id", &m_cl_platform_id);
		fconfig.Read("device_id", &m_cl_device_id);
	}
	//clipping plane display mode
	if (fconfig.Exists("/clipping planes"))
	{
		fconfig.SetPath("/clipping planes");
		fconfig.Read("mode", &m_plane_mode);
	}

	// java paths load.
	if (fconfig.Exists("/Java"))
	{
		fconfig.SetPath("/Java");
		fconfig.Read("jvm_path", &m_jvm_path);
		fconfig.Read("ij_path", &m_ij_path);
		fconfig.Read("bioformats_path", &m_bioformats_path);
		fconfig.Read("ij_mode", &m_ij_mode);
	}
}

void SettingAgent::SaveSettings()
{
	wxString app_name = "FluoRender " +
		wxString::Format("%d.%.1f", VERSION_MAJOR, float(VERSION_MINOR));
	wxString vendor_name = "FluoRender";
	wxString local_name = "fluorender.set";
	wxFileConfig fconfig(app_name, vendor_name, local_name, "",
		wxCONFIG_USE_LOCAL_FILE);

	fconfig.Write("ver_major", VERSION_MAJOR_TAG);
	fconfig.Write("ver_minor", VERSION_MINOR_TAG);

	fconfig.SetPath("/gm calculation");
	fconfig.Write("mode", m_gmc_mode);

	fconfig.SetPath("/peeling layers");
	fconfig.Write("value", m_peeling_layers);

	fconfig.SetPath("/micro blend");
	fconfig.Write("mode", m_micro_blend);

	fconfig.SetPath("/save project");
	fconfig.Write("mode", m_prj_save);

	fconfig.SetPath("/save alpha");
	fconfig.Write("mode", m_save_alpha);

	fconfig.SetPath("/save float");
	fconfig.Write("mode", m_save_float);

	fconfig.SetPath("/realtime compress");
	fconfig.Write("mode", m_realtime_compress);

	fconfig.SetPath("/skip bricks");
	fconfig.Write("mode", m_skip_bricks);

	fconfig.SetPath("/mouse int");
	fconfig.Write("mode", m_mouse_int);

	fconfig.SetPath("/dir shadow");
	fconfig.Write("mode", m_shadow_dir);
	fconfig.Write("x", m_shadow_dir_x);
	fconfig.Write("y", m_shadow_dir_y);

	fconfig.SetPath("/pin threshold");
	fconfig.Write("value", m_pin_threshold);

	fconfig.SetPath("/stereo");
	fconfig.Write("enable_stereo", m_stereo);
	fconfig.Write("eye dist", m_eye_dist);

	fconfig.SetPath("/test mode");
	fconfig.Write("speed", m_test_speed);
	fconfig.Write("param", m_test_param);
	fconfig.Write("wiref", m_test_wiref);

	fconfig.SetPath("/wavelength to color");
	fconfig.Write("c1", m_wav_color1);
	fconfig.Write("c2", m_wav_color2);
	fconfig.Write("c3", m_wav_color3);
	fconfig.Write("c4", m_wav_color4);

	fconfig.SetPath("/time id");
	fconfig.Write("value", m_time_id);

	fconfig.SetPath("/grad bg");
	fconfig.Write("value", m_grad_bg);

	fconfig.SetPath("/override vox");
	fconfig.Write("value", m_override_vox);

	fconfig.SetPath("/soft threshold");
	fconfig.Write("value", m_soft_threshold);

	fconfig.SetPath("/save cmp");
	fconfig.Write("value", RenderFrame::GetCompression());

	fconfig.SetPath("/run script");
	fconfig.Write("value", m_run_script);
	fconfig.Write("file", m_script_file);

	fconfig.SetPath("/paint history");
	fconfig.Write("value", m_paint_hist_depth);

	fconfig.SetPath("/text font");
	fconfig.Write("file", m_font_file);
	fconfig.Write("value", m_text_size);
	fconfig.Write("color", m_text_color);

	fconfig.SetPath("/line width");
	fconfig.Write("value", m_line_width);

	//full screen
	fconfig.SetPath("/full screen");
	fconfig.Write("stay top", m_stay_top);
	fconfig.Write("show cursor", m_show_cursor);

	//last tool
	fconfig.SetPath("/last tool");
	fconfig.Write("value", m_last_tool);

	//components
	fconfig.SetPath("/tracking");
	fconfig.Write("track_iter", m_track_iter);
	fconfig.Write("component_size", m_component_size);
	fconfig.Write("consistent_color", m_consistent_color);
	fconfig.Write("try_merge", m_try_merge);
	fconfig.Write("try_split", m_try_split);
	fconfig.Write("contact_factor", m_contact_factor);
	fconfig.Write("similarity", m_similarity);

	//memory settings
	fconfig.SetPath("/memory settings");
	fconfig.Write("mem swap", m_mem_swap);
	fconfig.Write("graphics mem", m_graphics_mem);
	fconfig.Write("large data size", m_large_data_size);
	fconfig.Write("force brick size", m_force_brick_size);
	fconfig.Write("up time", m_up_time);
	fconfig.Write("detail level offset", m_detail_level_offset);
	EnableStreaming(m_mem_swap);

	//update order
	fconfig.SetPath("/update order");
	fconfig.Write("value", m_update_order);

	//invalidate texture
	fconfig.SetPath("/invalidate tex");
	fconfig.Write("value", m_invalidate_tex);

	//point volume mode
	fconfig.SetPath("/point volume mode");
	fconfig.Write("value", m_point_volume_mode);

	//ruler settings
	fconfig.SetPath("/ruler");
	fconfig.Write("use transf", m_ruler_use_transf);
	fconfig.Write("time dep", m_ruler_time_dep);
	fconfig.Write("relax f1", m_ruler_relax_f1);
	fconfig.Write("infr", m_ruler_infr);
	fconfig.Write("df_f", m_ruler_df_f);
	fconfig.Write("relax iter", m_ruler_relax_iter);
	fconfig.Write("auto relax", m_ruler_auto_relax);
	fconfig.Write("relax type", m_ruler_relax_type);
	fconfig.Write("size thresh", m_ruler_size_thresh);

	//flags for flipping pvxml
	fconfig.SetPath("/pvxml");
	fconfig.Write("flip_x", m_pvxml_flip_x);
	fconfig.Write("flip_y", m_pvxml_flip_y);
	fconfig.Write("seq_type", m_pvxml_seq_type);

	//pixel format
	fconfig.SetPath("/pixel format");
	fconfig.Write("api_type", m_api_type);
	fconfig.Write("red_bit", m_red_bit);
	fconfig.Write("green_bit", m_green_bit);
	fconfig.Write("blue_bit", m_blue_bit);
	fconfig.Write("alpha_bit", m_alpha_bit);
	fconfig.Write("depth_bit", m_depth_bit);
	fconfig.Write("samples", m_samples);

	//context attrib
	fconfig.SetPath("/context attrib");
	fconfig.Write("gl_major_ver", m_gl_major_ver);
	fconfig.Write("gl_minor_ver", m_gl_minor_ver);
	fconfig.Write("gl_profile_mask", m_gl_profile_mask);

	//max texture size
	fconfig.SetPath("/max texture size");
	fconfig.Write("use_max_texture_size", m_use_max_texture_size);
	fconfig.Write("max_texture_size", m_max_texture_size);

	//no tex pack
	fconfig.SetPath("/no tex pack");
	fconfig.Write("no_tex_pack", m_no_tex_pack);

	//cl device
	fconfig.SetPath("/cl device");
	fconfig.Write("platform_id", m_cl_platform_id);
	fconfig.Write("device_id", m_cl_device_id);

	// java paths
	fconfig.SetPath("/Java");
	fconfig.Write("jvm_path", getJVMPath());
	fconfig.Write("ij_path", getIJPath());
	fconfig.Write("bioformats_path", getBioformatsPath());
	fconfig.Write("ij_mode", getIJMode());

	//clipping plane mode
	fconfig.SetPath("/clipping planes");
	fluo::ClipPlaneAgent* agent =
		glbin_agtf->findFirst(gstClipPlaneAgent)->asClipPlaneAgent();
	if (agent)
	{
		long lval;
		agent->getValue(gstClipPlaneMode, lval);
		m_plane_mode = lval;
	}
	fconfig.Write("mode", m_plane_mode);

	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "fluorender.set";
	SaveConfig(fconfig, dft);
}

void SettingAgent::UpdateDeviceTree()
{
	m_device_tree->DeleteAllItems();
	//cl device tree
	std::vector<flvr::CLPlatform>* devices = flvr::KernelProgram::GetDeviceList();
	int pid = flvr::KernelProgram::get_platform_id();
	int did = flvr::KernelProgram::get_device_id();
	wxTreeItemId root = m_device_tree->AddRoot("Computer");
	std::string name;
	if (devices)
	{
		for (int i = 0; i < devices->size(); ++i)
		{
			flvr::CLPlatform* platform = &((*devices)[i]);
			name = platform->vendor;
			name.back() = ';';
			name += " " + platform->name;
			wxTreeItemId pfitem = m_device_tree->AppendItem(root, name);
			for (int j = 0; j < platform->devices.size(); ++j)
			{
				flvr::CLDevice* device = &(platform->devices[j]);
				name = device->vendor;
				name.back() = ';';
				name += " " + device->name;
				wxTreeItemId dvitem = m_device_tree->AppendItem(pfitem, name);
				if (i == pid && j == did)
					m_device_tree->SelectItem(dvitem);
			}
		}
	}
	m_device_tree->ExpandAll();
	m_device_tree->SetFocus();
}

void SettingAgent::UpdateTextureSize()
{
	if (!m_use_max_texture_size)
	{
		m_max_texture_size_text->SetValue(
			wxString::Format("%d", flvr::ShaderProgram::
				max_texture_size()));
	}
	else
		flvr::ShaderProgram::set_max_texture_size(m_max_texture_size);
}

std::vector<std::string> SettingAgent::GetJvmArgs()
{
	std::vector<std::string> args;
	std::string jvm_path, ij_path, bioformats_path;
	getValue(gstJvmPath, jvm_path);
	getValue(gstImagejPath, ij_path);
	getValue(gstBioformatsPath, bioformats_path);
	args.push_back(jvm_path);
	args.push_back(ij_path);
	args.push_back(bioformats_path);
	return args;
}

