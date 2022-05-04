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
#include <Global.hpp>
#include <ShaderProgram.h>
#include <TextRenderer.h>
#include <KernelProgram.h>

using namespace fluo;

SettingAgent::SettingAgent(SettingDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
	setupInputs();
}

void SettingAgent::setupInputs()
{
	inputs_ = ValueCollection
	{
		//project page
		SaveProjectEnable,
		HardwareCompress,
		FontFile,
		TextSize,
		TextColorMode,
		LineWidth,
		PaintHistory,
		//rendering page
		MicroBlendEnable,
		PeelNum,
		ShadowDirEnable,
		ShadowDirX,
		ShadowDirY,
		PinThresh,
		LinkedRot,
		GradBg,
		VrEnable,
		VrEyeOffset,
		//Performance
		Adaptive,
		StreamEnable,
		UpdateOrder,
		GpuMemSize,
		LargeDataSize,
		BrickSize,
		ResponseTime,
		LodOffset,
		//Format
		OverrideVoxSpc,
		WaveColor1,
		WaveColor2,
		WaveColor3,
		WaveColor4,
		MaxTextureSizeEnable,
		MaxTextureSize,
		ClPlatformId,
		ClDeviceId,
		//Java
		ImagejMode,
		JvmPath,
		ImagejPath,
		BioformatsPath,
	};
}

void SettingAgent::setObject(Root* obj)
{
	InterfaceAgent::setObject(obj);
}

Root* SettingAgent::getObject()
{
	return dynamic_cast<Root*>(InterfaceAgent::getObject());
}

void SettingAgent::UpdateFui(const ValueCollection &names)
{
	bool bval; long lval; double dval;
	std::string sval;
	bool update_all = names.empty();

	//update user interface
	//save project
	if (update_all || FOUND_VALUE(SaveProjectEnable))
	{
		getValue(SaveProjectEnable, bval);
		dlg_.m_prj_save_chk->SetValue(bval);
	}
	//realtime compression
	if (update_all || FOUND_VALUE(HardwareCompress))
	{
		getValue(HardwareCompress, bval);
		dlg_.m_realtime_cmp_chk->SetValue(bval);
	}
	//mouse interactions
	if (update_all || FOUND_VALUE(Adaptive))
	{
		getValue(Adaptive, bval);
		dlg_.m_mouse_int_chk->SetValue(bval);
	}
	//depth peeling
	if (update_all || FOUND_VALUE(PeelNum))
	{
		getValue(PeelNum, lval);
		dlg_.m_peeling_layers_sldr->SetValue(lval);
		dlg_.m_peeling_layers_text->ChangeValue(wxString::Format("%d", lval));
	}
	//micro blending
	if (update_all || FOUND_VALUE(MicroBlendEnable))
	{
		getValue(MicroBlendEnable, bval);
		dlg_.m_micro_blend_chk->SetValue(bval);
	}
	//shadow direction
	if (update_all || FOUND_VALUE(ShadowDirEnable))
	{
		getValue(ShadowDirEnable, bval);
		dlg_.m_shadow_dir_chk->SetValue(bval);
		dlg_.m_shadow_dir_sldr->Enable(bval);
		dlg_.m_shadow_dir_text->Enable(bval);
	}
	if (update_all || FOUND_VALUE(ShadowDirX) || FOUND_VALUE(ShadowDirY))
	{
		double dx, dy;
		getValue(ShadowDirX, dx); getValue(ShadowDirY, dy);
		double deg = fluo::r2d(atan2(dy, dx));
		dlg_.m_shadow_dir_sldr->SetValue(int(deg + 0.5));
		dlg_.m_shadow_dir_text->ChangeValue(wxString::Format("%.2f", deg));
	}
	//rot center anchor thresh
	if (update_all || FOUND_VALUE(PinThresh))
	{
		getValue(PinThresh, dval);
		dlg_.m_pin_threshold_sldr->SetValue(int(dval*10.0));
		dlg_.m_pin_threshold_text->ChangeValue(wxString::Format("%.0f", dval*100.0));
	}
	//gradient background
	if (update_all || FOUND_VALUE(GradBg))
	{
		getValue(GradBg, bval);
		dlg_.m_grad_bg_chk->SetValue(bval);
	}
	//stereo
	if (update_all || FOUND_VALUE(VrEnable))
	{
		getValue(VrEnable, bval);
		dlg_.m_stereo_chk->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(VrEyeOffset))
	{
		getValue(VrEyeOffset, dval);
		dlg_.m_eye_dist_sldr->SetValue(int(dval*10.0));
		dlg_.m_eye_dist_text->ChangeValue(wxString::Format("%.1f", dval));
	}
	//override vox
	if (update_all || FOUND_VALUE(OverrideVoxSpc))
	{
		getValue(OverrideVoxSpc, bval);
		dlg_.m_override_vox_chk->SetValue(bval);
	}
	//wavelength to color
	if (update_all || FOUND_VALUE(WaveColor1))
	{
		getValue(WaveColor1, lval);
		dlg_.m_wav_color1_cmb->Select(lval - 1);
	}
	if (update_all || FOUND_VALUE(WaveColor2))
	{
		getValue(WaveColor2, lval);
		dlg_.m_wav_color2_cmb->Select(lval - 1);
	}
	if (update_all || FOUND_VALUE(WaveColor3))
	{
		getValue(WaveColor3, lval);
		dlg_.m_wav_color3_cmb->Select(lval - 1);
	}
	if (update_all || FOUND_VALUE(WaveColor4))
	{
		getValue(WaveColor4, lval);
		dlg_.m_wav_color4_cmb->Select(lval - 1);
	}
	//max texture size
	if (update_all || FOUND_VALUE(MaxTextureSizeEnable))
	{
		getValue(MaxTextureSizeEnable, bval);
		dlg_.m_max_texture_size_chk->SetValue(bval);
	}
	if (update_all || FOUND_VALUE(MaxTextureSize))
	{
		getValue(MaxTextureSizeEnable, bval);
		if (bval)
		{
			getValue(MaxTextureSize, lval);
			dlg_.m_max_texture_size_text->SetValue(
				wxString::Format("%d", lval));
			dlg_.m_max_texture_size_text->Enable();
		}
		else
		{
			dlg_.m_max_texture_size_text->SetValue(
				wxString::Format("%d", flvr::ShaderProgram::
					max_texture_size()));
			dlg_.m_max_texture_size_text->Disable();
		}
	}
	//no tex pack
	//if (update_all || FOUND_VALUE(NoTexPack))
	//{
	//	getValue(NoTexPack, bval);
	//	flvr::ShaderProgram::set_no_tex_upack(bval);
	//}
	//font
	if (update_all || FOUND_VALUE(FontFile))
	{
		getValue(FontFile, sval);
		wxString str = sval;
		str = str.BeforeLast('.');
		int font_sel = dlg_.m_font_cmb->FindString(str);
		if (font_sel != wxNOT_FOUND)
			dlg_.m_font_cmb->Select(font_sel);
	}
	if (update_all || FOUND_VALUE(TextSize))
	{
		long font_size;
		getValue(TextSize, dval);
		wxString str = sval;
		for (unsigned int i = 0; i < dlg_.m_font_size_cmb->GetCount(); ++i)
		{
			str = dlg_.m_font_size_cmb->GetString(i);
			if (str.ToLong(&font_size) &&
				font_size <= long(dval))
				dlg_.m_font_size_cmb->Select(i);
		}
	}
	if (update_all || FOUND_VALUE(TextColorMode))
	{
		getValue(TextColorMode, lval);
		dlg_.m_text_color_cmb->Select(lval);
	}
	//line width
	if (update_all || FOUND_VALUE(LineWidth))
	{
		getValue(LineWidth, dval);
		dlg_.m_line_width_text->SetValue(wxString::Format("%.0f", dval));
		dlg_.m_line_width_sldr->SetValue(int(dval + 0.5));
	}
	//paint history depth
	if (update_all || FOUND_VALUE(PaintHistory))
	{
		getValue(PaintHistory, lval);
		dlg_.m_paint_hist_depth_text->ChangeValue(wxString::Format("%d", lval));
		dlg_.m_paint_hist_depth_sldr->SetValue(lval);
	}
	//memory settings
	if (update_all || FOUND_VALUE(StreamEnable))
	{
		getValue(StreamEnable, bval);
		dlg_.m_streaming_chk->SetValue(bval);
		dlg_.EnableStreaming(bval);
	}
	if (update_all || FOUND_VALUE(UpdateOrder))
	{
		getValue(UpdateOrder, lval);
		dlg_.m_update_order_rbox->SetSelection(lval);
	}
	if (update_all || FOUND_VALUE(GpuMemSize))
	{
		getValue(GpuMemSize, dval);
		dlg_.m_graphics_mem_text->ChangeValue(wxString::Format("%d", (int)dval));
		dlg_.m_graphics_mem_sldr->SetValue((int)(dval / 100.0));
	}
	if (update_all || FOUND_VALUE(LargeDataSize))
	{
		getValue(LargeDataSize, dval);
		dlg_.m_large_data_text->ChangeValue(wxString::Format("%d", (int)dval));
		dlg_.m_large_data_sldr->SetValue((int)(dval / 10.0));
	}
	if (update_all || FOUND_VALUE(BrickSize))
	{
		getValue(BrickSize, lval);
		dlg_.m_block_size_text->ChangeValue(wxString::Format("%d", lval));
		dlg_.m_block_size_sldr->SetValue(int(log(double(lval)) / log(2.0) + 0.5));
	}
	if (update_all || FOUND_VALUE(ResponseTime))
	{
		getValue(ResponseTime, lval);
		dlg_.m_response_time_text->ChangeValue(wxString::Format("%d", lval));
		dlg_.m_response_time_sldr->SetValue(int(lval / 10.0));
	}
	if (update_all || FOUND_VALUE(LodOffset))
	{
		getValue(LodOffset, lval);
		dlg_.m_detail_level_offset_text->ChangeValue(wxString::Format("%d", -lval));
		dlg_.m_detail_level_offset_sldr->SetValue(-lval);
	}

	//java
	if (update_all || FOUND_VALUE(JvmPath))
	{
		getValue(JvmPath, sval);
		dlg_.m_java_jvm_text->SetValue(JvmPath);
	}
	if (update_all || FOUND_VALUE(ImagejPath))
	{
		getValue(ImagejPath, sval);
		dlg_.m_java_ij_text->SetValue(sval);
	}
	if (update_all || FOUND_VALUE(BioformatsPath))
	{
		getValue(BioformatsPath, sval);
		dlg_.m_java_bioformats_text->SetValue(sval);
	}
	if (update_all || FOUND_VALUE(ImagejMode))
	{
		getValue(ImagejMode, lval);
		switch (lval)
		{
		case 0:
			dlg_.mp_radio_button_imagej->SetValue(true);
			dlg_.m_java_jvm_text->Enable(true);
			dlg_.m_java_bioformats_text->Enable(true);
			dlg_.m_browse_jvm_btn->Enable(true);
			dlg_.m_browse_bioformats_btn->Enable(true);
			break;
		case 1:
			dlg_.mp_radio_button_fiji->SetValue(true);
			dlg_.m_java_jvm_text->Enable(false);
			dlg_.m_java_bioformats_text->Enable(false);
			dlg_.m_browse_jvm_btn->Enable(false);
			dlg_.m_browse_bioformats_btn->Enable(false);
			break;
		}
	}
}

void SettingAgent::ReadSettings()
{
	wxString expath = glbin.getExecutablePath();
	wxString dft = expath + GETSLASH() + "fluorender.set";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	bool bval; long lval; double dval;
	wxString sval;
	//gradient magnitude
	//depth peeling
	if (fconfig.Exists("/peeling layers"))
	{
		fconfig.SetPath("/peeling layers");
		if (fconfig.Read("value", &lval)) setValue(gstPeelNum, lval);
	}
	//micro blending
	if (fconfig.Exists("/micro blend"))
	{
		fconfig.SetPath("/micro blend");
		if (fconfig.Read("mode", &bval)) setValue(gstMicroBlendEnable, bval);
	}
	//save project
	if (fconfig.Exists("/save project"))
	{
		fconfig.SetPath("/save project");
		if (fconfig.Read("mode", &bval)) setValue(gstSaveProjectEnable, bval);
	}
	//save alpha
	if (fconfig.Exists("/save alpha"))
	{
		fconfig.SetPath("/save alpha");
		if (fconfig.Read("mode", &bval)) setValue(gstCaptureAlpha, bval);
	}
	//save float
	if (fconfig.Exists("/save float"))
	{
		fconfig.SetPath("/save float");
		if (fconfig.Read("mode", &bval)) setValue(gstCaptureFloat, bval);
	}
	//realtime compression
	if (fconfig.Exists("/realtime compress"))
	{
		fconfig.SetPath("/realtime compress");
		if (fconfig.Read("mode", &bval)) setValue(gstHardwareCompress, bval);
	}
	//skip empty bricks
	if (fconfig.Exists("/skip bricks"))
	{
		fconfig.SetPath("/skip bricks");
		if (fconfig.Read("mode", &bval)) setValue(gstSkipBrick, bval);
	}
	//mouse interactions
	if (fconfig.Exists("/mouse int"))
	{
		fconfig.SetPath("/mouse int");
		if (fconfig.Read("mode", &bval)) setValue(gstAdaptive, bval);
	}
	//shadow
	if (fconfig.Exists("/dir shadow"))
	{
		fconfig.SetPath("/dir shadow");
		if (fconfig.Read("mode", &bval)) setValue(gstShadowDirEnable, bval);
		if (fconfig.Read("x", &dval)) setValue(gstShadowDirX, dval);
		if (fconfig.Read("y", &dval)) setValue(gstShadowDirY, dval);
	}
	//rot center anchor thresh
	if (fconfig.Exists("/pin threshold"))
	{
		fconfig.SetPath("/pin threshold");
		if (fconfig.Read("value", &dval)) setValue(gstPinThresh, dval);
	}
	//stereo
	if (fconfig.Exists("/stereo"))
	{
		fconfig.SetPath("/stereo");
		if (fconfig.Read("enable_stereo", &bval)) setValue(gstVrEnable, bval);
		if (fconfig.Read("eye dist", &dval)) setValue(gstVrEyeOffset, dval);
	}
	//test mode
	if (fconfig.Exists("/test mode"))
	{
		fconfig.SetPath("/test mode");
		if (fconfig.Read("speed", &bval)) setValue(gstTestSpeed, bval);
		if (fconfig.Read("param", &bval)) setValue(gstTestParam, bval);
		if (fconfig.Read("wiref", &bval)) setValue(gstTestWiref, bval);
	}
	//wavelength to color
	if (fconfig.Exists("/wavelength to color"))
	{
		fconfig.SetPath("/wavelength to color");
		if (fconfig.Read("c1", &lval)) setValue(gstWaveColor1, lval);
		if (fconfig.Read("c2", &lval)) setValue(gstWaveColor2, lval);
		if (fconfig.Read("c3", &lval)) setValue(gstWaveColor3, lval);
		if (fconfig.Read("c4", &lval)) setValue(gstWaveColor4, lval);
	}
	//time sequence identifier
	if (fconfig.Exists("/time id"))
	{
		fconfig.SetPath("/time id");
		if (fconfig.Read("value", &sval)) setValue(gstTimeFileId, sval.ToStdString());
	}
	//gradient background
	if (fconfig.Exists("/grad bg"))
	{
		fconfig.SetPath("/grad bg");
		if (fconfig.Read("value", &bval)) setValue(gstGradBg, bval);
	}
	//save compressed
	if (fconfig.Exists("/save cmp"))
	{
		bool save_cmp;
		fconfig.SetPath("/save cmp");
		if (fconfig.Read("value", &bval)) setValue(gstCaptureCompress, bval);
	}
	//override vox
	if (fconfig.Exists("/override vox"))
	{
		fconfig.SetPath("/override vox");
		if (fconfig.Read("value", &bval)) setValue(gstOverrideVoxSpc, bval);
	}
	//soft threshold
	if (fconfig.Exists("/soft threshold"))
	{
		fconfig.SetPath("/soft threshold");
		if (fconfig.Read("value", &dval)) setValue(gstSoftThresh, dval);
	}
	//run script
	if (fconfig.Exists("/run script"))
	{
		fconfig.SetPath("/run script");
		if (fconfig.Read("value", &bval)) setValue(gstRunScript, bval);
		if (fconfig.Read("file", &sval)) setValue(gstScriptFile, sval.ToStdWstring());
	}
	//paint history depth
	if (fconfig.Exists("/paint history"))
	{
		fconfig.SetPath("/paint history");
		if (fconfig.Read("value", &lval)) setValue(gstPaintHistory, lval);
	}
	//text font
	if (fconfig.Exists("/text font"))
	{
		fconfig.SetPath("/text font");
		if (fconfig.Read("file", &sval)) setValue(gstFontFile, sval.ToStdString());
		if (fconfig.Read("value", &dval)) setValue(gstTextSize, dval);
		if (fconfig.Read("color", &lval)) setValue(gstTextColorMode, lval);
	}
	//line width
	if (fconfig.Exists("/line width"))
	{
		fconfig.SetPath("/line width");
		if (fconfig.Read("value", &dval)) setValue(gstLineWidth, dval);
	}
	//full screen
	if (fconfig.Exists("/full screen"))
	{
		fconfig.SetPath("/full screen");
		if (fconfig.Read("stay top", &bval)) setValue(gstStayOnTop, bval);
		if (fconfig.Read("show cursor", &bval)) setValue(gstShowCursor, bval);
	}
	//last tool
	if (fconfig.Exists("/last tool"))
	{
		fconfig.SetPath("/last tool");
		if (fconfig.Read("value", &lval)) setValue(gstLastTool, lval);
	}
	//tracking settings
	if (fconfig.Exists("/tracking"))
	{
		fconfig.SetPath("/tracking");
		if (fconfig.Read("track_iter", &lval)) setValue(gstTrackIter, lval);
		if (fconfig.Read("component_size", &lval)) setValue(gstCompSizeLimit, lval);
		if (fconfig.Read("consistent_color", &bval)) setValue(gstCompConsistent, bval);
		if (fconfig.Read("try_merge", &bval)) setValue(gstTryMerge, bval);
		if (fconfig.Read("try_split", &bval)) setValue(gstTrySplit, bval);
		if (fconfig.Read("contact_factor", &dval)) setValue(gstContactFactor, dval);
		if (fconfig.Read("similarity", &dval)) setValue(gstSimilarity, dval);
	}
	//memory settings
	if (fconfig.Exists("/memory settings"))
	{
		fconfig.SetPath("/memory settings");
		//enable mem swap
		if (fconfig.Read("mem swap", &bval))
		{
			setValue(gstStreamEnable, bval);
			dlg_.EnableStreaming(bval);
		}
		//graphics memory limit
		if (fconfig.Read("graphics mem", &dval)) setValue(gstGpuMemSize, dval);
		//large data size
		if (fconfig.Read("large data size", &dval)) setValue(gstLargeDataSize, dval);
		//force brick size
		if (fconfig.Read("force brick size", &lval)) setValue(gstBrickSize, lval);
		//response time
		if (fconfig.Read("up time", &lval)) setValue(gstResponseTime, lval);
		//detail level offset
		if (fconfig.Read("detail level offset", &lval)) setValue(gstLodOffset, lval);
	}
	//update order
	if (fconfig.Exists("/update order"))
	{
		fconfig.SetPath("/update order");
		if (fconfig.Read("value", &lval)) setValue(gstUpdateOrder, lval);
	}
	//invalidate texture
	//point volume mode
	if (fconfig.Exists("/point volume mode"))
	{
		fconfig.SetPath("/point volume mode");
		if (fconfig.Read("value", &lval)) setValue(gstPointVolumeMode, lval);
	}
	//ruler settings
	if (fconfig.Exists("/ruler"))
	{
		fconfig.SetPath("/ruler");
		if (fconfig.Read("use transf", &bval)) setValue(gstRulerUseTransf, bval);
		if (fconfig.Read("time dep", &bval)) setValue(gstRulerTransient, bval);
		if (fconfig.Read("relax f1", &dval)) setValue(gstRulerF1, dval);
		if (fconfig.Read("infr", &dval)) setValue(gstRulerInfr, dval);
		if (fconfig.Read("df_f", &bval)) setValue(gstRulerDfoverf, bval);
		if (fconfig.Read("relax iter", &lval)) setValue(gstRulerRelaxIter, lval);
		if (fconfig.Read("auto relax", &bval)) setValue(gstRulerRelax, bval);
		if (fconfig.Read("relax type", &lval)) setValue(gstRulerRelaxType, lval);
		if (fconfig.Read("size thresh", &lval)) setValue(gstRulerSizeThresh, lval);
	}
	//flags for pvxml flipping
	if (fconfig.Exists("/pvxml"))
	{
		fconfig.SetPath("/pvxml");
		if (fconfig.Read("flip_x", &bval)) setValue(gstPvxmlFlipX, bval);
		if (fconfig.Read("flip_y", &bval)) setValue(gstPvxmlFlipY, bval);
		if (fconfig.Read("seq_type", &lval)) setValue(gstPvxmlSeqType, lval);
	}
	//pixel format
	if (fconfig.Exists("/pixel format"))
	{
		fconfig.SetPath("/pixel format");
		if (fconfig.Read("api_type", &lval)) setValue(gstApiType, lval);
		if (fconfig.Read("red_bit", &lval)) setValue(gstOutputBitR, lval);
		if (fconfig.Read("green_bit", &lval)) setValue(gstOutputBitG, lval);
		if (fconfig.Read("blue_bit", &lval)) setValue(gstOutputBitB, lval);
		if (fconfig.Read("alpha_bit", &lval)) setValue(gstOutputBitA, lval);
		if (fconfig.Read("depth_bit", &lval)) setValue(gstOutputBitD, lval);
		if (fconfig.Read("samples", &lval)) setValue(gstOutputSamples, lval);
	}
	//context attrib
	if (fconfig.Exists("/context attrib"))
	{
		fconfig.SetPath("/context attrib");
		if (fconfig.Read("gl_major_ver", &lval)) setValue(gstGlVersionMajor, lval);
		if (fconfig.Read("gl_minor_ver", &lval)) setValue(gstGlVersionMinor, lval);
		if (fconfig.Read("gl_profile_mask", &lval)) setValue(gstGlProfileMask, lval);
	}
	//max texture size
	if (fconfig.Exists("/max texture size"))
	{
		fconfig.SetPath("/max texture size");
		if (fconfig.Read("use_max_texture_size", &bval)) setValue(gstMaxTextureSizeEnable, bval);
		if (fconfig.Read("max_texture_size", &lval)) setValue(gstMaxTextureSize, lval);
	}
	//no tex pack
	if (fconfig.Exists("/no tex pack"))
	{
		fconfig.SetPath("/no tex pack");
		if (fconfig.Read("no_tex_pack", &bval)) setValue(gstNoTexPack, bval);
	}
	//cl device
	if (fconfig.Exists("/cl device"))
	{
		fconfig.SetPath("/cl device");
		if (fconfig.Read("platform_id", &lval)) setValue(gstClPlatformId, lval);
		if (fconfig.Read("device_id", &lval)) setValue(gstClDeviceId, lval);
	}
	//clipping plane display mode
	if (fconfig.Exists("/clipping planes"))
	{
		fconfig.SetPath("/clipping planes");
		if (fconfig.Read("mode", &lval)) setValue(gstClipPlaneMode, lval);
	}

	// java paths load.
	if (fconfig.Exists("/Java"))
	{
		fconfig.SetPath("/Java");
		if (fconfig.Read("jvm_path", &sval)) setValue(gstJvmPath, sval.ToStdString());
		if (fconfig.Read("ij_path", &sval)) setValue(gstImagejPath, sval.ToStdString());
		if (fconfig.Read("bioformats_path", &sval)) setValue(gstBioformatsPath, sval.ToStdString());
		if (fconfig.Read("ij_mode", &lval)) setValue(gstImagejMode, lval);
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

	bool bval; long lval; double dval;
	std::wstring wsval; std::string sval;

	fconfig.SetPath("/peeling layers");
	getValue(gstPeelNum, lval); fconfig.Write("value", lval);

	fconfig.SetPath("/micro blend");
	getValue(gstMicroBlendEnable, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/save project");
	getValue(gstSaveProjectEnable, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/save alpha");
	getValue(gstCaptureAlpha, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/save float");
	getValue(gstCaptureFloat, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/realtime compress");
	getValue(gstHardwareCompress, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/skip bricks");
	getValue(gstSkipBrick, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/mouse int");
	getValue(gstAdaptive, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/dir shadow");
	getValue(gstShadowDirEnable, bval); fconfig.Write("mode", bval);
	getValue(gstShadowDirX, dval); fconfig.Write("x", dval);
	getValue(gstShadowDirY, dval); fconfig.Write("y", dval);

	fconfig.SetPath("/pin threshold");
	getValue(gstPinThresh, dval); fconfig.Write("value", dval);

	fconfig.SetPath("/stereo");
	getValue(gstVrEnable, bval); fconfig.Write("enable_stereo", bval);
	getValue(gstVrEyeOffset, dval); fconfig.Write("eye dist", dval);

	fconfig.SetPath("/test mode");
	getValue(gstTestSpeed, bval); fconfig.Write("speed", bval);
	getValue(gstTestParam, bval); fconfig.Write("param", bval);
	getValue(gstTestWiref, bval); fconfig.Write("wiref", bval);

	fconfig.SetPath("/wavelength to color");
	getValue(gstWaveColor1, lval); fconfig.Write("c1", lval);
	getValue(gstWaveColor2, lval); fconfig.Write("c2", lval);
	getValue(gstWaveColor3, lval); fconfig.Write("c3", lval);
	getValue(gstWaveColor4, lval); fconfig.Write("c4", lval);

	fconfig.SetPath("/time id");
	getValue(gstTimeFileId, sval); fconfig.Write("value", wxString(sval));

	fconfig.SetPath("/grad bg");
	getValue(gstGradBg, bval); fconfig.Write("value", bval);

	fconfig.SetPath("/save cmp");
	getValue(gstCaptureCompress, bval); fconfig.Write("value", bval);

	fconfig.SetPath("/override vox");
	getValue(gstOverrideVoxSpc, bval); fconfig.Write("value", bval);

	fconfig.SetPath("/soft threshold");
	getValue(gstSoftThresh, dval); fconfig.Write("value", dval);

	fconfig.SetPath("/run script");
	getValue(gstRunScript, bval); fconfig.Write("value", bval);
	getValue(gstScriptFile, wsval); fconfig.Write("file", wxString(wsval));

	fconfig.SetPath("/paint history");
	getValue(gstPaintHistory, lval); fconfig.Write("value", lval);

	fconfig.SetPath("/text font");
	getValue(gstFontFile, sval); fconfig.Write("file", wxString(sval));
	getValue(gstTextSize, dval); fconfig.Write("value", dval);
	getValue(gstTextColorMode, lval); fconfig.Write("color", lval);

	fconfig.SetPath("/line width");
	getValue(gstLineWidth, dval); fconfig.Write("value", dval);

	//full screen
	fconfig.SetPath("/full screen");
	getValue(gstStayOnTop, bval); fconfig.Write("stay top", bval);
	getValue(gstShowCursor, bval); fconfig.Write("show cursor", bval);

	//last tool
	fconfig.SetPath("/last tool");
	getValue(gstLastTool, lval); fconfig.Write("value", lval);

	//components
	fconfig.SetPath("/tracking");
	getValue(gstTrackIter, lval); fconfig.Write("track_iter", lval);
	getValue(gstCompSizeLimit, lval); fconfig.Write("component_size", lval);
	getValue(gstCompConsistent, bval); fconfig.Write("consistent_color", bval);
	getValue(gstTryMerge, bval); fconfig.Write("try_merge", bval);
	getValue(gstTrySplit, bval); fconfig.Write("try_split", bval);
	getValue(gstContactFactor, dval); fconfig.Write("contact_factor", dval);
	getValue(gstSimilarity, dval); fconfig.Write("similarity", dval);

	//memory settings
	fconfig.SetPath("/memory settings");
	getValue(gstStreamEnable, bval); fconfig.Write("mem swap", bval);
	getValue(gstGpuMemSize, dval); fconfig.Write("graphics mem", dval);
	getValue(gstLargeDataSize, dval); fconfig.Write("large data size", dval);
	getValue(gstBrickSize, lval); fconfig.Write("force brick size", lval);
	getValue(gstResponseTime, lval); fconfig.Write("up time", lval);
	getValue(gstLodOffset, lval); fconfig.Write("detail level offset", lval);

	//update order
	fconfig.SetPath("/update order");
	getValue(gstUpdateOrder, lval); fconfig.Write("value", lval);

	//point volume mode
	fconfig.SetPath("/point volume mode");
	getValue(gstPointVolumeMode, lval); fconfig.Write("value", lval);

	//ruler settings
	fconfig.SetPath("/ruler");
	getValue(gstRulerUseTransf, bval); fconfig.Write("use transf", bval);
	getValue(gstRulerTransient, bval); fconfig.Write("time dep", bval);
	getValue(gstRulerF1, dval); fconfig.Write("relax f1", dval);
	getValue(gstRulerInfr, dval); fconfig.Write("infr", dval);
	getValue(gstRulerDfoverf, bval); fconfig.Write("df_f", bval);
	getValue(gstRulerRelaxIter, lval); fconfig.Write("relax iter", lval);
	getValue(gstRulerRelax, bval); fconfig.Write("auto relax", bval);
	getValue(gstRulerRelaxType, lval); fconfig.Write("relax type", lval);
	getValue(gstRulerSizeThresh, lval); fconfig.Write("size thresh", lval);

	//flags for flipping pvxml
	fconfig.SetPath("/pvxml");
	getValue(gstPvxmlFlipX, bval); fconfig.Write("flip_x", bval);
	getValue(gstPvxmlFlipY, bval); fconfig.Write("flip_y", bval);
	getValue(gstPvxmlSeqType, lval); fconfig.Write("seq_type", lval);

	//pixel format
	fconfig.SetPath("/pixel format");
	getValue(gstApiType, lval); fconfig.Write("api_type", lval);
	getValue(gstOutputBitR, lval); fconfig.Write("red_bit", lval);
	getValue(gstOutputBitG, lval); fconfig.Write("green_bit", lval);
	getValue(gstOutputBitB, lval); fconfig.Write("blue_bit", lval);
	getValue(gstOutputBitA, lval); fconfig.Write("alpha_bit", lval);
	getValue(gstOutputBitD, lval); fconfig.Write("depth_bit", lval);
	getValue(gstOutputSamples, lval); fconfig.Write("samples", lval);

	//context attrib
	fconfig.SetPath("/context attrib");
	getValue(gstGlVersionMajor, lval); fconfig.Write("gl_major_ver", lval);
	getValue(gstGlVersionMinor, lval); fconfig.Write("gl_minor_ver", lval);
	getValue(gstGlProfileMask, lval); fconfig.Write("gl_profile_mask", lval);

	//max texture size
	fconfig.SetPath("/max texture size");
	getValue(gstMaxTextureSizeEnable, bval); fconfig.Write("use_max_texture_size", bval);
	getValue(gstMaxTextureSize, lval); fconfig.Write("max_texture_size", lval);

	//no tex pack
	fconfig.SetPath("/no tex pack");
	getValue(gstNoTexPack, bval); fconfig.Write("no_tex_pack", bval);

	//cl device
	fconfig.SetPath("/cl device");
	getValue(gstClPlatformId, lval); fconfig.Write("platform_id", lval);
	getValue(gstClDeviceId, lval); fconfig.Write("device_id", lval);

	//clipping plane mode
	fconfig.SetPath("/clipping planes");
	getValue(gstClipPlaneMode, lval); fconfig.Write("mode", lval);

	// java paths
	fconfig.SetPath("/Java");
	getValue(gstJvmPath, sval); fconfig.Write("jvm_path", wxString(sval));
	getValue(gstImagejPath, sval); fconfig.Write("ij_path", wxString(sval));
	getValue(gstBioformatsPath, sval); fconfig.Write("bioformats_path", wxString(sval));
	getValue(gstImagejMode, lval); fconfig.Write("ij_mode", lval);

	wxString expath = glbin.getExecutablePath();
	wxString dft = expath + GETSLASH() + "fluorender.set";
	SaveConfig(fconfig, dft);
}

void SettingAgent::UpdateDeviceTree()
{
	dlg_.m_device_tree->DeleteAllItems();
	//cl device tree
	std::vector<flvr::CLPlatform>* devices = flvr::KernelProgram::GetDeviceList();
	int pid = flvr::KernelProgram::get_platform_id();
	int did = flvr::KernelProgram::get_device_id();
	wxTreeItemId root = dlg_.m_device_tree->AddRoot("Computer");
	std::string name;
	if (devices)
	{
		for (int i = 0; i < devices->size(); ++i)
		{
			flvr::CLPlatform* platform = &((*devices)[i]);
			name = platform->vendor;
			name.back() = ';';
			name += " " + platform->name;
			wxTreeItemId pfitem = dlg_.m_device_tree->AppendItem(root, name);
			for (int j = 0; j < platform->devices.size(); ++j)
			{
				flvr::CLDevice* device = &(platform->devices[j]);
				name = device->vendor;
				name.back() = ';';
				name += " " + device->name;
				wxTreeItemId dvitem = dlg_.m_device_tree->AppendItem(pfitem, name);
				if (i == pid && j == did)
					dlg_.m_device_tree->SelectItem(dvitem);
			}
		}
	}
	dlg_.m_device_tree->ExpandAll();
	dlg_.m_device_tree->SetFocus();
}

void SettingAgent::OnMaxTextureSizeEnable(Event& event)
{
	bool bval;
	getValue(gstMaxTextureSizeEnable, bval);
	if (bval)
	{
		long lval;
		getValue(gstMaxTextureSize, lval);
		flvr::ShaderProgram::set_max_texture_size(lval);
	}
	else
		flvr::ShaderProgram::reset_max_texture_size();
}

void SettingAgent::OnMaxTextureSize(Event& event)
{
	bool bval;
	getValue(gstMaxTextureSizeEnable, bval);
	if (bval)
	{
		long lval;
		getValue(gstMaxTextureSize, lval);
		flvr::ShaderProgram::set_max_texture_size(lval);
	}
}

void SettingAgent::OnFontFile(Event& event)
{
	std::string sval;
	getValue(gstFontFile, sval);
	wxString exePath = glbin.getExecutablePath();
	wxString loc = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + sval;
	flvr::TextRenderer::text_texture_manager_.load_face(loc.ToStdString());
	double dval;
	getValue(gstTextSize, dval);
	flvr::TextRenderer::text_texture_manager_.SetSize(dval);
	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(gstTextSize, m_text_size);
	//	//view->RefreshGL(39);
	//}
}