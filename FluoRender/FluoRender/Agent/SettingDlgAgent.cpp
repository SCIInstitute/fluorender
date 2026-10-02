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
#include <CurrentObjects.h>
#include <Coordinator.h>
#include <DataManager.h>
#include <TextRenderer.h>
#include <ModalDlg.h>

SettingDlgAgent::SettingDlgAgent(
	SettingDlg* dlg) :
	Agent(dlg)
{

}

void SettingDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool update_all = request.values.empty();

	double dval;
	int ival;
	bool bval;

	//project page
	//project save
	if (update_all || request.HasValue(gstSaveProjectEnable))
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
	if (update_all || request.HasValue(gstFontFile))
	{
		std::vector<std::string> font_list;
		if (GetFontList(font_list))
			dlg->UpdateFontFile(font_list);
	}
	if (update_all || request.HasValue(gstSettingsFont))
	{
		std::filesystem::path p(glbin_settings.m_font_file);
		auto str = p.stem().string();
		dlg->UpdateSettingsFont(str,
			glbin_settings.m_text_size,
			glbin_settings.m_text_color);
	}

	//line width
	if (update_all || request.HasValue(gstLineWidth))
	{
		dval = glbin_settings.m_line_width;
		dlg->UpdateLineWidth(dval);
	}

	//paint history depth
	if (update_all || request.HasValue(gstPaintHistory))
	{
		ival = glbin_brush_def.m_paint_hist_depth;
		dlg->UpdatePaintHistory(ival);
	}

	//pencil distance
	if (update_all || request.HasValue(gstPencilDist))
	{
		dval = glbin_settings.m_pencil_dist;
		dlg->UpdatePencilDist(dval);	
	}

	//micro blending
	if (update_all || request.HasValue(gstMicroBlendEnable))
	{
		bval = glbin_settings.m_micro_blend;
		dlg->UpdateMicroBlendEnable(bval);
	}

	//depth peeling
	if (update_all || request.HasValue(gstPeelNum))
	{
		ival = glbin_settings.m_peeling_layers;
		dlg->UpdatePeelNum(ival);
	}

	//rotations
	if (update_all || request.HasValue(gstPinThreshold))
	{
		dval = glbin_settings.m_pin_threshold;
		dlg->UpdatePinThreshold(dval);
	}

	//rot link
	if (update_all || request.HasValue(gstRotLink))
	{
		bval = glbin_settings.m_linked_rot;
		dlg->UpdateRotLink(bval);
	}

	//gradient background
	if (update_all || request.HasValue(gstGradBg))
	{
		bval = glbin_settings.m_grad_bg;
		dlg->UpdateGradBg(bval);
	}

	//match background color
	if (update_all || request.HasValue(gstClearColorBg))
	{
		bval = glbin_settings.m_clear_color_bg;
		dlg->UpdateClearColorBg(bval);
	}

	//performance page
	//mouse interactions
	if (update_all || request.HasValue(gstMouseInt))
	{
		ival = glbin_settings.m_interactive_quality;
		dlg->UpdateMouseInt(ival);
	}

	//memory settings
	if (update_all || request.HasValue(gstStreamEnable))
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
	if (update_all || request.HasValue(gstAutomate))
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
	if (update_all || request.HasValue(gstHologramMode))
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
	if (update_all || request.HasValue(gstFullscreenDisplay))
	{
		ival = glbin_settings.m_disp_id;
		dlg->UpdateFullscreenDisplay(ival);
	}

	//color depth
	if (update_all || request.HasValue(gstDisplayColorDepth))
	{
		ival = glbin_settings.m_color_depth;
		dlg->UpdateDisplayColorDepth(ival);
	}

	//format page
	//wavelength to color
	if (update_all || request.HasValue(gstWavelengthColors))
	{
		int val1 = glbin_settings.m_wav_color1;
		int val2 = glbin_settings.m_wav_color2;
		int val3 = glbin_settings.m_wav_color3;
		int val4 = glbin_settings.m_wav_color4;
		dlg->UpdateWavelengthColor(val1, val2, val3, val4);
	}

	//max texture size
	if (update_all || request.HasValue(gstMaxTextureSize))
	{
		bval = glbin_settings.m_use_max_texture_size;
		if (bval)
			ival = glbin_settings.m_max_texture_size;
		else
			ival = flvr::ShaderProgram::max_texture_size();
		dlg->UpdateMaxTextureSize(bval, ival);
	}

	if (update_all || request.HasValue(gstDeviceTree))
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
	if (update_all || request.HasValue(gstSettingsJava))
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
	if (request.HasValue(gstSaveProjectEnable))
		SetProjectSave();
	if (request.HasValue(gstSaveProjectInc))
		SetProjectSaveInc();
	if (request.HasValue(gstRealtimeCompress))
		SetRealtimeCompress();
	if (request.HasValue(gstScriptBreakEnable))
		SetScriptBreak();
	if (request.HasValue(gstInverseSliders))
		SetInverseSliders();
	if (request.HasValue(gstMulFuncBtn))
		SetMulFuncBtn();
	if (request.HasValue(gstConfigFileType))
		SetConfigFileType();
	if (request.HasValue(gstYDir))
		SetYDir();
	if (request.HasValue(gstMouseInt))
		SetInteractiveQuality();
	if (request.HasValue(gstPeelNum))
		SetPeelingLayers();
	if (request.HasValue(gstMicroBlendEnable))
		SetMicroBlend();
	if (request.HasValue(gstGradBg))
		SetGradBg();
	if (request.HasValue(gstClearColorBg))
		SetClearColorBg();
	if (request.HasValue(gstPinThreshold))
		SetPinThreshold();
	if (request.HasValue(gstRotLink))
		SetRotLink();
	if (request.HasValue(gstHologramMode))
		SetHologramMode();
	if (request.HasValue(gstXrApi))
		SetXrApi();
	if (request.HasValue(gstMvHmd))
		SetMvHmd();
	if (request.HasValue(gstSbs))
		SetSbs();
	if (request.HasValue(gstEyeDist))
		SetEyeDist();
	if (request.HasValue(gstHoloIp))
		SetHoloIp();
	if (request.HasValue(gstLgOffset))
		SetLgOffset();
	if (request.HasValue(gstLgQuilt))
		SetLgQuilt();
	if (request.HasValue(gstLgCameraMode))
		SetLgCameraMode();
	if (request.HasValue(gstFullscreenDisplay))
		SetDispId();
	if (request.HasValue(gstDisplayColorDepth))
		SetColorDepth();
	if (request.HasValue(gstWavelengthColors))
		SetWavelengthColor();
	if (request.HasValue(gstMaxTextureSizeEnable))
		SetMaxTextureSizeEnable();
	if (request.HasValue(gstMaxTextureSize))
		SetMaxTextureSize();
	if (request.HasValue(gstStreamEnable))
		SetStreamEnable();
	if (request.HasValue(gstUpdateOrder))
		SetUpdateOrder();
	if (request.HasValue(gstGraphicsMem))
		SetGraphicsMem();
	if (request.HasValue(gstLargeDataSize))
		SetLargeData();
	if (request.HasValue(gstBrickSize))
		SetBrickSize();
	if (request.HasValue(gstResponseTime))
		SetResponseTime();
	if (request.HasValue(gstLodOffset))
		SetDetailLevelOffset();
	if (request.HasValue(gstFontFile))
		SetFont();
	if (request.HasValue(gstFontSize))
		SetFontSize();
	if (request.HasValue(gstTextColor))
		SetTextColor();
	if (request.HasValue(gstLineWidth))
		SetLineWidth();
	if (request.HasValue(gstPaintHistory))
		SetPaintHistDepth();
	if (request.HasValue(gstPencilDist))
		SetPencilDist();
	if (request.HasValue(gstJavaJvm))
		SetJavaJvm();
	if (request.HasValue(gstJavaIJ))
		SetJavaIJ();
	if (request.HasValue(gstJavaBioformats))
		SetJavaBioformats();
	if (request.HasValue(gstJavaJvmBrowse))
		SetJavaJvmBrowse();
	if (request.HasValue(gstJavaIJBrowse))
		SetJavaIJBrowse();
	if (request.HasValue(gstJavaBioformatsBrowse))
		SetJavaBioformatsBrowse();
	if (request.HasValue(gstJavaEnable))
		SetJavaEnable();
	if (request.HasValue(gstJavaIJEnable))
		SetJavaIJEnable();
	if (request.HasValue(gstDeviceTree))
		SetDevice();
	if (request.HasValue(gstAutomate))
		SetAutomation();
	if (request.HasValue(gstResetSettings))
		Reset();
	if (request.HasValue(gstRecommendedSettings))
		SetRecommended();
}

SettingDlg* SettingDlgAgent::GetDialog() const
{
	return static_cast<SettingDlg*>(GetOwner());
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

void SettingDlgAgent::SetProjectSave()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_prj_save = dlg->GetProjectSave();
}

void SettingDlgAgent::SetProjectSaveInc()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_prj_save_inc = dlg->GetProjectSaveInc();
}

void SettingDlgAgent::SetRealtimeCompress()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_realtime_compress = dlg->GetRealtimeCompress();
}

void SettingDlgAgent::SetScriptBreak()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_script_break = dlg->GetScriptBreak();
}

void SettingDlgAgent::SetInverseSliders()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_inverse_slider = dlg->GetInverseSliders();
}

void SettingDlgAgent::SetMulFuncBtn()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_mulfunc = dlg->GetMulFuncBtnUse();
		NotifyDataToUI({ gstMultiFuncTips });
	}
}

void SettingDlgAgent::SetConfigFileType()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_config_file_type = dlg->GetConfigFileType();
}

void SettingDlgAgent::SetYDir()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_y_dir = dlg->GetYDir();
}

void SettingDlgAgent::SetInteractiveQuality()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_interactive_quality = dlg->GetMouseInt();
}

void SettingDlgAgent::SetPeelingLayers()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_peeling_layers = dlg->GetPeelNum();
		NotifyViewUpdate({ gstPeelNum });
	}
}

void SettingDlgAgent::SetMicroBlend()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_micro_blend = dlg->GetMicroBlend();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetGradBg()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_grad_bg = dlg->GetGradBg();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetClearColorBg()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_clear_color_bg = dlg->GetClearColorBg();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetPinThreshold()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_pin_threshold = dlg->GetPinThreshold();
		UpdateDataToUI({ gstPinThreshold });
	}
}

void SettingDlgAgent::SetRotLink()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_linked_rot = dlg->GetRotLink();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetHologramMode()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_hologram_mode = dlg->GetHologramMode();
		NotifyViewUpdate({ gstHologramMode });
	}
}

void SettingDlgAgent::SetXrApi()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_xr_api = dlg->GetXrApi();
		if (glbin_settings.m_xr_api == 4)
			glbin_settings.m_eye_dist = 0;
		else
			glbin_settings.m_eye_dist = 20;
		NotifyViewUpdate({ gstHologramMode });
	}
}

void SettingDlgAgent::SetMvHmd()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_mv_hmd = dlg->GetMvHmd();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetSbs()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_sbs = dlg->GetSbs();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetEyeDist()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_eye_dist = dlg->GetEyeDist();
		NotifyViewUpdate({ gstEyeDist });
	}
}

void SettingDlgAgent::SetHoloIp()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		auto str = dlg->GetHoloIp();
		if (!str.empty())
		{
			glbin_settings.m_holo_ip = str;
			NotifyViewUpdate({ gstNull });
		}
	}
}

void SettingDlgAgent::SetLgOffset()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_lg_offset = dlg->GetLgOffset();
		NotifyViewUpdate({ gstLgOffset });
	}
}

void SettingDlgAgent::SetLgQuilt()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_hologram_debug = dlg->GetLgQuilt();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetLgCameraMode()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_hologram_camera_mode = dlg->GetLgCameraMode();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetDispId()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_disp_id = dlg->GetDispId();
	}
}

void SettingDlgAgent::SetColorDepth()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;
	int ival = dlg->GetDispColorDepth();
	switch (ival)
	{
	case 0://8
		glbin_settings.m_color_depth = ival;
		glbin_settings.m_red_bit = 8;
		glbin_settings.m_green_bit = 8;
		glbin_settings.m_blue_bit = 8;
		glbin_settings.m_alpha_bit = 8;
		break;
	case 1://10
		glbin_settings.m_color_depth = ival;
		glbin_settings.m_red_bit = 10;
		glbin_settings.m_green_bit = 10;
		glbin_settings.m_blue_bit = 10;
		glbin_settings.m_alpha_bit = 2;
		break;
	case 2://16
		glbin_settings.m_color_depth = ival;
		glbin_settings.m_red_bit = 16;
		glbin_settings.m_green_bit = 16;
		glbin_settings.m_blue_bit = 16;
		glbin_settings.m_alpha_bit = 16;
		break;
	}
}

void SettingDlgAgent::SetWavelengthColor()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		int ival = dlg->GetWavelengthColorSel();
		switch (ival)
		{
		case 0:
			glbin_settings.m_wav_color1 = dlg->GetWavelengthColor();
			break;
		case 1:
			glbin_settings.m_wav_color2 = dlg->GetWavelengthColor();
			break;
		case 2:
			glbin_settings.m_wav_color3 = dlg->GetWavelengthColor();
			break;
		case 3:
			glbin_settings.m_wav_color4 = dlg->GetWavelengthColor();
			break;
		}
	}
}

void SettingDlgAgent::SetMaxTextureSizeEnable()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		bool bval = dlg->GetMaxTextureSizeUse();
		glbin_settings.m_use_max_texture_size = bval;
		if (bval)
			flvr::ShaderProgram::set_max_texture_size(glbin_settings.m_max_texture_size);
		else
			flvr::ShaderProgram::reset_max_texture_size();
		UpdateDataToUI({ gstMaxTextureSize });
	}
}

void SettingDlgAgent::SetMaxTextureSize()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		int ival = dlg->GetMaxTextureSize();
		glbin_settings.m_max_texture_size = ival;
		flvr::ShaderProgram::set_max_texture_size(ival);
	}
}

void SettingDlgAgent::SetStreamEnable()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_stream_rendering = dlg->GetStreamEnable();
		glbin_data_manager.UpdateStreamMode(-1.0);
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetUpdateOrder()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_update_order = dlg->GetUpdateOrder();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetGraphicsMem()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		double dval = dlg->GetGraphicsMem();
		if (dval > 0.0)
		{
			glbin_settings.m_graphics_mem = dval;
			UpdateDataToUI({ gstStreamEnable });
		}
	}
}

void SettingDlgAgent::SetLargeData()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		double dval = dlg->GetLargeDataSize();
		if (dval > 0.0)
		{
			glbin_settings.m_large_data_size = dval;
			UpdateDataToUI({ gstStreamEnable });
		}
	}
}

void SettingDlgAgent::SetBrickSize()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		double dval = dlg->GetBrickSize();
		if (dval > 0.0)
		{
			glbin_settings.m_force_brick_size = dval;
			UpdateDataToUI({ gstStreamEnable });
		}
	}
}

void SettingDlgAgent::SetResponseTime()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		int ival = dlg->GetResponseTime();
		if (ival > 0)
		{
			glbin_settings.m_up_time = ival;
			UpdateDataToUI({ gstStreamEnable });
		}
	}
}

void SettingDlgAgent::SetDetailLevelOffset()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetDetailLevelOffset();
	glbin_settings.m_detail_level_offset = -ival;
	NotifyViewUpdate({ gstStreamEnable });
}

void SettingDlgAgent::SetFont()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto str = dlg->GetFontFileName();
	if (str.empty())
		return;

	glbin_settings.m_font_file = str + L".ttf";
	std::filesystem::path p = GetDataRoot();
	p = p / "Fonts" / (str + L".ttf");
	glbin_text_tex_manager.load_face(p.wstring());
	glbin_text_tex_manager.set_size(glbin_settings.m_text_size);
	NotifyViewUpdate({ gstNull });
}

void SettingDlgAgent::SetFontSize()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetFontSize();
	if (ival <= 0)
		return;
	glbin_settings.m_text_size = ival;
	glbin_text_tex_manager.set_size(ival);
	NotifyViewUpdate({ gstNull });
}

void SettingDlgAgent::SetTextColor()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_settings.m_text_color = dlg->GetTextColor();
		NotifyViewUpdate({ gstNull });
	}
}

void SettingDlgAgent::SetLineWidth()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetLineWidth();
	if (ival <= 0)
		return;

	glbin_settings.m_line_width = ival;
	NotifyViewUpdate({ gstLineWidth });
}

void SettingDlgAgent::SetPaintHistDepth()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetPaintHistDepth();
	if (ival < 0)
		return;

	glbin_brush_def.m_paint_hist_depth = ival;
	UpdateDataToUI({ gstPaintHistory });
	//flvr::BrickTexture::mask_undo_num_ = (size_t)(ival);
}

void SettingDlgAgent::SetPencilDist()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	int ival = dlg->GetPencilDist();
	if (ival <= 0)
		return;

	glbin_settings.m_pencil_dist = ival;
	UpdateDataToUI({ gstPencilDist });
}

void SettingDlgAgent::SetJavaJvm()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_jvm_path = dlg->GetJavaJvm();
}

void SettingDlgAgent::SetJavaIJ()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_ij_path = dlg->GetJavaIJ();
}

void SettingDlgAgent::SetJavaBioformats()
{
	auto dlg = GetDialog();
	if (dlg)
		glbin_settings.m_bioformats_path = dlg->GetJavaBioformats();
}

void SettingDlgAgent::SetJavaJvmBrowse()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

#ifdef _WIN32
	ModalDlg fopendlg(
		dlg, "Choose the jvm dll file",
		"", "", "*.dll", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#else
	ModalDlg fopendlg(
		dlg, "Choose the libjvm.dylib file",
		"", "", "*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		glbin_settings.m_jvm_path = filename;
		UpdateDataToUI({ gstSettingsJava });
	}
}

void SettingDlgAgent::SetJavaIJBrowse()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

#ifdef _WIN32	
	wxDirDialog fopendlg(
		dlg, "Choose the imageJ/fiji directory",
		"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
#else
	ModalDlg fopendlg(
		dlg, "Choose the imageJ/fiji app",
		"", "", "*.app", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
#endif

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
#ifdef _DARWIN
		//filename = filename + "/Contents/Java/ij.jar";
#endif
		glbin_settings.m_ij_path = filename;
		UpdateDataToUI({ gstSettingsJava });
	}
}

void SettingDlgAgent::SetJavaBioformatsBrowse()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Choose the bioformats jar",
		"", "", "*.jar", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		wxString filename = fopendlg.GetPath();
		glbin_settings.m_bioformats_path = filename;
		UpdateDataToUI({ gstSettingsJava });
	}
}

void SettingDlgAgent::SetJavaEnable()
{
	glbin_settings.m_ij_mode = 0;
	UpdateDataToUI({ gstSettingsJava });
}

void SettingDlgAgent::SetJavaIJEnable()
{
	glbin_settings.m_ij_mode = 1;
	UpdateDataToUI({ gstSettingsJava });
}

void SettingDlgAgent::SetDevice()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetDeviceSelection();
	if (sel.platform_id != -1 && sel.device_id != -1)
	{
		glbin_settings.m_cl_platform_id = sel.platform_id;
		glbin_settings.m_cl_device_id = sel.device_id;
	}
}

void SettingDlgAgent::SetAutomation()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel = dlg->GetAutomationSel();
	switch (sel.id)
	{
	case 0://histogram
		glbin_automate_def.m_histogram = sel.index;
		break;
	case 1://paint size
		glbin_automate_def.m_paint_size = sel.index;
		break;
	case 2://compo gen
		glbin_automate_def.m_comp_gen = sel.index;
		break;
	case 3://colocalize
		glbin_automate_def.m_colocalize = sel.index;
		break;
	case 4://relax ruler
		glbin_automate_def.m_relax_ruler = sel.index;
		break;
	}
}

void SettingDlgAgent::Reset()
{
	glbin_settings.Reset();
	glbin.apply_processor_settings();
	glbin_comp_def.Apply(glbin_clusterizer);
	glbin_comp_def.Apply(glbin_comp_analyzer);
	glbin_comp_def.Apply(glbin_comp_generator);
	glbin_comp_def.Apply(glbin_comp_selector);
	glbin_brush_def.Apply(glbin_vol_selector);
	glbin_mesh_def.Apply(glbin_conv_vol_mesh);
	glbin_mov_def.Apply(glbin_moviemaker);
	glbin_data_manager.UpdateStreamMode(-1.0);
	NotifyViewUpdate({});
}

void SettingDlgAgent::SetRecommended()
{
	glbin_settings.Read("fluorender_default");
	glbin.apply_processor_settings();
	glbin_comp_def.Apply(glbin_clusterizer);
	glbin_comp_def.Apply(glbin_comp_analyzer);
	glbin_comp_def.Apply(glbin_comp_generator);
	glbin_comp_def.Apply(glbin_comp_selector);
	glbin_brush_def.Apply(glbin_vol_selector);
	glbin_mesh_def.Apply(glbin_conv_vol_mesh);
	glbin_mov_def.Apply(glbin_moviemaker);
	glbin_data_manager.UpdateStreamMode(-1.0);
	NotifyViewUpdate({});
}
