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
	int ival;
	bool bval;

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
		ival = glbin_brush_def.m_paint_hist_depth;
		dlg->UpdatePaintHistory(ival);
	}

	//pencil distance
	if (update_all || FOUND_VALUE(gstPencilDist))
	{
		dval = glbin_settings.m_pencil_dist;
		dlg->UpdatePencilDist(dval);	
	}

	//micro blending
	if (update_all || FOUND_VALUE(gstMicroBlendEnable))
	{
		bval = glbin_settings.m_micro_blend;
		dlg->UpdateMicroBlendEnable(bval);
	}

	//depth peeling
	if (update_all || FOUND_VALUE(gstPeelNum))
	{
		ival = glbin_settings.m_peeling_layers;
		dlg->UpdatePeelNum(ival);
	}

	//rotations
	if (update_all || FOUND_VALUE(gstSettingsRot))
	{
		dval = glbin_settings.m_pin_threshold;
		dlg->UpdateSettingRot(dval);
	}

	//gradient background
	if (update_all || FOUND_VALUE(gstGradBg))
	{
		bval = glbin_settings.m_grad_bg;
		dlg->UpdateGradBg(bval);
	}

	//match background color
	if (update_all || FOUND_VALUE(gstClearColorBg))
	{
		bval = glbin_settings.m_clear_color_bg;
		dlg->UpdateClearColorBg(bval);
	}

	//performance page
	//mouse interactions
	if (update_all || FOUND_VALUE(gstMouseInt))
	{
		ival = glbin_settings.m_interactive_quality;
		dlg->UpdateMouseInt(ival);
	}

	//memory settings
	if (update_all || FOUND_VALUE(gstStreamEnable))
	{
		StreamInfo info(
			glbin_settings.m_stream_rendering,
			glbin_settings.m_update_order,
			glbin_settings.m_graphics_mem,
			glbin_settings.m_large_data_size,
			glbin_settings.m_force_brick_size,
			glbin_settings.m_up_time,
			glbin_settings.m_detail_level_offset);
		dlg->UpdateStreamEable(info);
	}

	//automate page
	if (update_all || FOUND_VALUE(gstAutomate))
	{
		AutomateInfo info(
			glbin_automate_def.m_histogram,
			glbin_automate_def.m_paint_size,
			glbin_automate_def.m_comp_gen,
			glbin_automate_def.m_colocalize,
			glbin_automate_def.m_relax_ruler,
			glbin_automate_def.m_conv_vol_mesh);
		dlg->UpdateAutomate(info);
	}

	//display page
	//stereo
	if (update_all || FOUND_VALUE(gstHologramMode))
	{
		HologramInfo info(
			glbin_settings.m_hologram_mode,
			glbin_settings.m_xr_api,
			glbin_settings.m_holo_ip,
			glbin_settings.m_mv_hmd,
			glbin_settings.m_sbs,
			glbin_settings.m_eye_dist,
			glbin_settings.m_lg_offset,
			glbin_settings.m_hologram_debug,
			glbin_settings.m_hologram_camera_mode);
		dlg->UpdateHologramMode(info);
	}

	//display id
	if (update_all || FOUND_VALUE(gstFullscreenDisplay))
	{
		ival = glbin_settings.m_disp_id;
		dlg->UpdateFullscreenDisplay(ival);
	}

	//color depth
	if (update_all || FOUND_VALUE(gstDisplayColorDepth))
	{
		ival = glbin_settings.m_color_depth;
		dlg->UpdateDisplayColorDepth(ival);
	}

	//format page
	//wavelength to color
	if (update_all || FOUND_VALUE(gstWavelengthColors))
	{
		int val1 = glbin_settings.m_wav_color1;
		int val2 = glbin_settings.m_wav_color2;
		int val3 = glbin_settings.m_wav_color3;
		int val4 = glbin_settings.m_wav_color4;
		dlg->UpdateWavelengthColor(val1, val2, val3, val4);
	}

	//max texture size
	if (update_all || FOUND_VALUE(gstMaxTextureSize))
	{
		bval = glbin_settings.m_use_max_texture_size;
		if (bval)
			ival = glbin_settings.m_max_texture_size;
		else
			ival = flvr::ShaderProgram::max_texture_size();
		dlg->UpdateMaxTextureSize(bval, ival);
	}

	if (update_all || FOUND_VALUE(gstDeviceTree))
	{
		DeviceTreeInfo result;

		result.platform_id = flvr::KernelProgram::get_platform_id();
		result.device_id = flvr::KernelProgram::get_device_id();

		auto* devices = flvr::KernelProgram::GetDeviceList();
		if (devices)
		{
			for (const auto& platform : *devices)
			{
				std::vector<std::string> branch;

				branch.push_back(
					platform.vendor + "; " + platform.name);

				for (const auto& device : platform.devices)
				{
					branch.push_back(
						device.vendor + "; " +
						device.name + "; " +
						device.version);
				}

				result.tree.push_back(std::move(branch));
			}
			dlg->UpdateDeviceTree(result);
		}
	}

	//java
	if (update_all || FOUND_VALUE(gstSettingsJava))
	{
		std::wstring jvm = glbin_settings.m_jvm_path;
		std::wstring ij = glbin_settings.m_ij_path;
		std::wstring bioformats = glbin_settings.m_bioformats_path;
		ival = glbin_settings.m_ij_mode;
		dlg->UpdateSettingsJava(jvm, ij, bioformats, ival);
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