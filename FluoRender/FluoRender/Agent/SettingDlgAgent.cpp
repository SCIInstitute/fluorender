/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <SettingDlgAgent.h>
#include <SettingDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <Directory.h>
#include <ShaderProgram.h>
#include <KernelProgram.h>

SettingDlgAgent::SettingDlgAgent(
	SettingDlg* dlg) :
	Agent(dlg)
{

}

bool SettingDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void SettingDlgAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void SettingDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	double dval;

	//project page
	//project save
	if (update_all || FOUND_VALUE(gstSaveProjectEnable))
	{
		SaveProjectInfo info(
			glbin_settings.m_prj_save,
			glbin_settings.m_prj_save_inc,
			glbin_settings.m_realtime_compress,
			glbin_settings.m_script_break,
			glbin_settings.m_inverse_slider,
			glbin_settings.m_mulfunc,
			glbin_settings.m_config_file_type,
			glbin_settings.m_y_dir);
		dlg->UpdateSaveProjectEnable(info);
	}

	//font
	if (update_all || FOUND_VALUE(gstFontFile))
	{
		std::vector<std::string> font_list;
		if (GetFontList(font_list))
			dlg->UpdateFontFile(font_list);
	}
	if (update_all || FOUND_VALUE(gstSettingsFont))
	{
		std::filesystem::path p(glbin_settings.m_font_file);
		auto str = p.stem().string();
		dlg->UpdateSettingsFont(str,
			glbin_settings.m_text_size,
			glbin_settings.m_text_color);
	}

	//line width
	if (update_all || FOUND_VALUE(gstLineWidth))
	{
		dval = glbin_settings.m_line_width;
		dlg->UpdateLineWidth(dval);
	}

	//paint history depth
	if (update_all || FOUND_VALUE(gstPaintHistory))
	{
	}

	//pencil distance
	if (update_all || FOUND_VALUE(gstPencilDist))
	{
	}

	//micro blending
	if (update_all || FOUND_VALUE(gstMicroBlendEnable))
		m_micro_blend_chk->SetValue(glbin_settings.m_micro_blend);

	//depth peeling
	if (update_all || FOUND_VALUE(gstPeelNum))
	{
		m_peeling_layers_sldr->ChangeValue(glbin_settings.m_peeling_layers);
		m_peeling_layers_text->ChangeValue(wxString::Format("%d", glbin_settings.m_peeling_layers));
	}

	//rotations
	if (update_all || FOUND_VALUE(gstSettingsRot))
	{
		//rot center anchor thresh
		m_pin_threshold_sldr->ChangeValue(std::round(glbin_settings.m_pin_threshold * 10.0));
		m_pin_threshold_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_pin_threshold * 100.0));
	}

	//gradient background
	if (update_all || FOUND_VALUE(gstGradBg))
		m_grad_bg_chk->SetValue(glbin_settings.m_grad_bg);

	//match background color
	if (update_all || FOUND_VALUE(gstClearColorBg))
		m_clear_color_bg_chk->SetValue(glbin_settings.m_clear_color_bg);

	//performance page
	//mouse interactions
	if (update_all || FOUND_VALUE(gstMouseInt))
		m_mouse_int_comb->SetSelection(glbin_settings.m_interactive_quality);

	//memory settings
	if (update_all || FOUND_VALUE(gstStreamEnable))
	{
		m_streaming_comb->SetSelection(glbin_settings.m_stream_rendering);
		m_update_order_comb->SetSelection(glbin_settings.m_update_order);
		m_graphics_mem_text->ChangeValue(wxString::Format("%d", (int)glbin_settings.m_graphics_mem));
		m_graphics_mem_sldr->ChangeValue(std::round(glbin_settings.m_graphics_mem / 100.0));
		m_large_data_text->ChangeValue(wxString::Format("%d", (int)glbin_settings.m_large_data_size));
		m_large_data_sldr->ChangeValue(std::round(glbin_settings.m_large_data_size / 10.0));
		m_block_size_text->ChangeValue(wxString::Format("%d", glbin_settings.m_force_brick_size));
		m_block_size_sldr->ChangeValue(std::round(log(glbin_settings.m_force_brick_size) / log(2.0)));
		m_response_time_text->ChangeValue(wxString::Format("%d", glbin_settings.m_up_time));
		m_response_time_sldr->ChangeValue(std::round(glbin_settings.m_up_time / 10.0));
		m_detail_level_offset_text->ChangeValue(wxString::Format("%d", -glbin_settings.m_detail_level_offset));
		m_detail_level_offset_sldr->ChangeValue(-glbin_settings.m_detail_level_offset);
	}

	//automate page
	if (update_all || FOUND_VALUE(gstAutomate))
	{
		auto it = m_automate_combo.find("histogram");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_histogram);
		}
		it = m_automate_combo.find("paint size");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_paint_size);
		}
		it = m_automate_combo.find("comp gen");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_comp_gen);
		}
		it = m_automate_combo.find("colocalize");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_colocalize);
		}
		it = m_automate_combo.find("relax ruler");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_relax_ruler);
		}
		it = m_automate_combo.find("conv vol mesh");
		if (it != m_automate_combo.end())
		{
			ComboEntry& entry = it->second;
			entry.combo->SetSelection(glbin_automate_def.m_conv_vol_mesh);
		}
	}

	//display page
	//stereo
	if (update_all || FOUND_VALUE(gstHologramMode))
	{
		if (glbin_settings.m_hologram_mode == 0)
		{
#ifdef _WIN32
			m_holo_ip_text->Disable();
#endif
			m_stereo_chk->SetValue(false);
			m_xr_api_cmb->Disable();
			m_mv_hmd_chk->Disable();
			m_sbs_chk->Disable();
			m_eye_dist_sldr->Disable();
			m_eye_dist_text->Disable();
			m_looking_glass_chk->SetValue(false);
			m_lg_offset_sldr->Disable();
			m_lg_offset_text->Disable();
			m_lg_quilt_cmb->Disable();
			m_lg_camera_mode_cmb->Disable();
		}
		else if (glbin_settings.m_hologram_mode == 1)
		{
#ifdef _WIN32
			if (glbin_settings.m_xr_api == 4)
				m_holo_ip_text->Enable();
			else
				m_holo_ip_text->Disable();
#endif
			m_stereo_chk->SetValue(true);
			m_xr_api_cmb->Enable();
			m_mv_hmd_chk->Enable();
			m_sbs_chk->Enable();
			m_eye_dist_sldr->Enable();
			m_eye_dist_text->Enable();
			m_looking_glass_chk->SetValue(false);
			m_lg_offset_sldr->Disable();
			m_lg_offset_text->Disable();
			m_lg_quilt_cmb->Disable();
			m_lg_camera_mode_cmb->Disable();
		}
		else if (glbin_settings.m_hologram_mode == 2)
		{
#ifdef _WIN32
			m_holo_ip_text->Disable();
#endif
			m_stereo_chk->SetValue(false);
			m_xr_api_cmb->Disable();
			m_mv_hmd_chk->Disable();
			m_sbs_chk->Disable();
			m_eye_dist_sldr->Disable();
			m_eye_dist_text->Disable();
			m_looking_glass_chk->SetValue(true);
			m_lg_offset_sldr->Enable();
			m_lg_offset_text->Enable();
			m_lg_quilt_cmb->Enable();
			m_lg_camera_mode_cmb->Enable();
		}
#ifdef _WIN32
		m_holo_ip_text->ChangeValue(wxString(glbin_settings.m_holo_ip));
#endif
		m_mv_hmd_chk->SetValue(glbin_settings.m_mv_hmd);
		m_xr_api_cmb->Select(glbin_settings.m_xr_api);
		m_sbs_chk->SetValue(glbin_settings.m_sbs);
		m_eye_dist_sldr->ChangeValue(std::round(glbin_settings.m_eye_dist * 10.0));
		m_eye_dist_text->ChangeValue(wxString::Format("%.1f", glbin_settings.m_eye_dist));
		m_lg_offset_sldr->ChangeValue(glbin_settings.m_lg_offset);
		m_lg_offset_text->ChangeValue(wxString::Format("%.0f", glbin_settings.m_lg_offset));
		m_lg_quilt_cmb->Select(glbin_settings.m_hologram_debug);
		m_lg_camera_mode_cmb->Select(glbin_settings.m_hologram_camera_mode);
	}

	//display id
	if (update_all || FOUND_VALUE(gstFullscreenDisplay))
		m_disp_id_comb->Select(glbin_settings.m_disp_id);

	//color depth
	if (update_all || FOUND_VALUE(gstDisplayColorDepth))
		m_color_depth_comb->Select(glbin_settings.m_color_depth);

	//format page
	//wavelength to color
	if (update_all || FOUND_VALUE(gstWavelengthColors))
	{
		m_wav_color1_cmb->Select(glbin_settings.m_wav_color1 - 1);
		m_wav_color2_cmb->Select(glbin_settings.m_wav_color2 - 1);
		m_wav_color3_cmb->Select(glbin_settings.m_wav_color3 - 1);
		m_wav_color4_cmb->Select(glbin_settings.m_wav_color4 - 1);
	}

	//max texture size
	if (update_all || FOUND_VALUE(gstMaxTextureSize))
	{
		bool bval = glbin_settings.m_use_max_texture_size;
		m_max_texture_size_chk->SetValue(bval);
		m_max_texture_size_text->Enable(bval);
		wxString str;
		if (bval)
			str = wxString::Format("%d", glbin_settings.m_max_texture_size);
		else
			str = wxString::Format("%d", flvr::ShaderProgram::max_texture_size());
		m_max_texture_size_text->ChangeValue(str);
	}

	if (update_all || FOUND_VALUE(gstDeviceTree))
	{
		m_device_tree->DeleteAllItems();
		//cl device tree
		std::vector<flvr::CLPlatform>* devices = flvr::KernelProgram::GetDeviceList();
		int pid = flvr::KernelProgram::get_platform_id();
		int did = flvr::KernelProgram::get_device_id();
		wxTreeItemId root = m_device_tree->AddRoot("Computer");
		if (devices)
		{
			for (int i = 0; i < devices->size(); ++i)
			{
				flvr::CLPlatform* platform = &((*devices)[i]);
				std::string name = platform->vendor;
				name += "; " + platform->name;
				wxTreeItemId pfitem = m_device_tree->AppendItem(root, name);
				for (int j = 0; j < platform->devices.size(); ++j)
				{
					flvr::CLDevice* device = &(platform->devices[j]);
					std::string name = device->vendor;
					name += "; " + device->name;
					name += "; " + device->version;
					wxTreeItemId dvitem = m_device_tree->AppendItem(pfitem, name);
					if (i == pid && j == did)
						m_device_tree->SelectItem(dvitem);
				}
			}
		}
		m_device_tree->ExpandAll();
		//m_device_tree->SetFocus();
	}

	//java
	if (update_all || FOUND_VALUE(gstSettingsJava))
	{
		m_java_jvm_text->ChangeValue(glbin_settings.m_jvm_path);
		m_java_ij_text->ChangeValue(glbin_settings.m_ij_path);
		m_java_bioformats_text->ChangeValue(glbin_settings.m_bioformats_path);
		switch (glbin_settings.m_ij_mode)
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
}

void SettingDlgAgent::UpdateData(const UpdateRequest& request)
{

}

SettingDlg* SettingDlgAgent::GetDialog() const
{
	return static_cast<SettingDlg*>(GetWindow());
}

bool SettingDlgAgent::GetFontList(std::vector<std::string>& list) const
{
	//populate fonts
	std::filesystem::path p = GetDataRoot();
	p /= "Fonts";
	if (std::filesystem::exists(p) && std::filesystem::is_directory(p))
	{
		for (const auto& entry : std::filesystem::directory_iterator(p))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".ttf")
			{
				list.push_back(entry.path().stem().string());
			}
		}
	}

	if (list.empty())
		return false;
	std::sort(list.begin(), list.end());
	return true;
}