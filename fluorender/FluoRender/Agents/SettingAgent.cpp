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
		if (fconfig.Read("value", &lval)) setValue(PeelNum, lval);
	}
	//micro blending
	if (fconfig.Exists("/micro blend"))
	{
		fconfig.SetPath("/micro blend");
		if (fconfig.Read("mode", &bval)) setValue(MicroBlendEnable, bval);
	}
	//save project
	if (fconfig.Exists("/save project"))
	{
		fconfig.SetPath("/save project");
		if (fconfig.Read("mode", &bval)) setValue(SaveProjectEnable, bval);
	}
	//save alpha
	if (fconfig.Exists("/save alpha"))
	{
		fconfig.SetPath("/save alpha");
		if (fconfig.Read("mode", &bval)) setValue(CaptureAlpha, bval);
	}
	//save float
	if (fconfig.Exists("/save float"))
	{
		fconfig.SetPath("/save float");
		if (fconfig.Read("mode", &bval)) setValue(CaptureFloat, bval);
	}
	//realtime compression
	if (fconfig.Exists("/realtime compress"))
	{
		fconfig.SetPath("/realtime compress");
		if (fconfig.Read("mode", &bval)) setValue(HardwareCompress, bval);
	}
	//skip empty bricks
	if (fconfig.Exists("/skip bricks"))
	{
		fconfig.SetPath("/skip bricks");
		if (fconfig.Read("mode", &bval)) setValue(SkipBrick, bval);
	}
	//mouse interactions
	if (fconfig.Exists("/mouse int"))
	{
		fconfig.SetPath("/mouse int");
		if (fconfig.Read("mode", &bval)) setValue(Adaptive, bval);
	}
	//shadow
	if (fconfig.Exists("/dir shadow"))
	{
		fconfig.SetPath("/dir shadow");
		if (fconfig.Read("mode", &bval)) setValue(ShadowDirEnable, bval);
		if (fconfig.Read("x", &dval)) setValue(ShadowDirX, dval);
		if (fconfig.Read("y", &dval)) setValue(ShadowDirY, dval);
	}
	//rot center anchor thresh
	if (fconfig.Exists("/pin threshold"))
	{
		fconfig.SetPath("/pin threshold");
		if (fconfig.Read("value", &dval)) setValue(PinThresh, dval);
	}
	//stereo
	if (fconfig.Exists("/stereo"))
	{
		fconfig.SetPath("/stereo");
		if (fconfig.Read("enable_stereo", &bval)) setValue(VrEnable, bval);
		if (fconfig.Read("eye dist", &dval)) setValue(VrEyeOffset, dval);
	}
	//test mode
	if (fconfig.Exists("/test mode"))
	{
		fconfig.SetPath("/test mode");
		if (fconfig.Read("speed", &bval)) setValue(TestSpeed, bval);
		if (fconfig.Read("param", &bval)) setValue(TestParam, bval);
		if (fconfig.Read("wiref", &bval)) setValue(TestWiref, bval);
	}
	//wavelength to color
	if (fconfig.Exists("/wavelength to color"))
	{
		fconfig.SetPath("/wavelength to color");
		if (fconfig.Read("c1", &lval)) setValue(WaveColor1, lval);
		if (fconfig.Read("c2", &lval)) setValue(WaveColor2, lval);
		if (fconfig.Read("c3", &lval)) setValue(WaveColor3, lval);
		if (fconfig.Read("c4", &lval)) setValue(WaveColor4, lval);
	}
	//time sequence identifier
	if (fconfig.Exists("/time id"))
	{
		fconfig.SetPath("/time id");
		if (fconfig.Read("value", &sval)) setValue(TimeFileId, sval.ToStdString());
	}
	//gradient background
	if (fconfig.Exists("/grad bg"))
	{
		fconfig.SetPath("/grad bg");
		if (fconfig.Read("value", &bval)) setValue(GradBg, bval);
	}
	//save compressed
	if (fconfig.Exists("/save cmp"))
	{
		bool save_cmp;
		fconfig.SetPath("/save cmp");
		if (fconfig.Read("value", &bval)) setValue(CaptureCompress, bval);
	}
	//override vox
	if (fconfig.Exists("/override vox"))
	{
		fconfig.SetPath("/override vox");
		if (fconfig.Read("value", &bval)) setValue(OverrideVoxSpc, bval);
	}
	//soft threshold
	if (fconfig.Exists("/soft threshold"))
	{
		fconfig.SetPath("/soft threshold");
		if (fconfig.Read("value", &dval)) setValue(SoftThresh, dval);
	}
	//run script
	if (fconfig.Exists("/run script"))
	{
		fconfig.SetPath("/run script");
		if (fconfig.Read("value", &bval)) setValue(RunScript, bval);
		if (fconfig.Read("file", &sval)) setValue(ScriptFile, sval.ToStdWstring());
	}
	//paint history depth
	if (fconfig.Exists("/paint history"))
	{
		fconfig.SetPath("/paint history");
		if (fconfig.Read("value", &lval)) setValue(PaintHistory, lval);
	}
	//text font
	if (fconfig.Exists("/text font"))
	{
		fconfig.SetPath("/text font");
		if (fconfig.Read("file", &sval)) setValue(FontFile, sval.ToStdString());
		if (fconfig.Read("value", &dval)) setValue(TextSize, dval);
		if (fconfig.Read("color", &lval)) setValue(TextColorMode, lval);
	}
	//line width
	if (fconfig.Exists("/line width"))
	{
		fconfig.SetPath("/line width");
		if (fconfig.Read("value", &dval)) setValue(LineWidth, dval);
	}
	//full screen
	if (fconfig.Exists("/full screen"))
	{
		fconfig.SetPath("/full screen");
		if (fconfig.Read("stay top", &bval)) setValue(StayOnTop, bval);
		if (fconfig.Read("show cursor", &bval)) setValue(ShowCursor, bval);
	}
	//last tool
	if (fconfig.Exists("/last tool"))
	{
		fconfig.SetPath("/last tool");
		if (fconfig.Read("value", &lval)) setValue(LastTool, lval);
	}
	//tracking settings
	if (fconfig.Exists("/tracking"))
	{
		fconfig.SetPath("/tracking");
		if (fconfig.Read("track_iter", &lval)) setValue(TrackIter, lval);
		if (fconfig.Read("component_size", &lval)) setValue(CompSizeLimit, lval);
		if (fconfig.Read("consistent_color", &bval)) setValue(CompConsistent, bval);
		if (fconfig.Read("try_merge", &bval)) setValue(TryMerge, bval);
		if (fconfig.Read("try_split", &bval)) setValue(TrySplit, bval);
		if (fconfig.Read("contact_factor", &dval)) setValue(ContactFactor, dval);
		if (fconfig.Read("similarity", &dval)) setValue(Similarity, dval);
	}
	//memory settings
	if (fconfig.Exists("/memory settings"))
	{
		fconfig.SetPath("/memory settings");
		//enable mem swap
		if (fconfig.Read("mem swap", &bval))
		{
			setValue(StreamEnable, bval);
			dlg_.EnableStreaming(bval);
		}
		//graphics memory limit
		if (fconfig.Read("graphics mem", &dval)) setValue(GpuMemSize, dval);
		//large data size
		if (fconfig.Read("large data size", &dval)) setValue(LargeDataSize, dval);
		//force brick size
		if (fconfig.Read("force brick size", &lval)) setValue(BrickSize, lval);
		//response time
		if (fconfig.Read("up time", &lval)) setValue(ResponseTime, lval);
		//detail level offset
		if (fconfig.Read("detail level offset", &lval)) setValue(LodOffset, lval);
	}
	//update order
	if (fconfig.Exists("/update order"))
	{
		fconfig.SetPath("/update order");
		if (fconfig.Read("value", &lval)) setValue(UpdateOrder, lval);
	}
	//invalidate texture
	//point volume mode
	if (fconfig.Exists("/point volume mode"))
	{
		fconfig.SetPath("/point volume mode");
		if (fconfig.Read("value", &lval)) setValue(PointVolumeMode, lval);
	}
	//ruler settings
	if (fconfig.Exists("/ruler"))
	{
		fconfig.SetPath("/ruler");
		if (fconfig.Read("use transf", &bval)) setValue(RulerUseTransf, bval);
		if (fconfig.Read("time dep", &bval)) setValue(RulerTransient, bval);
		if (fconfig.Read("relax f1", &dval)) setValue(RulerF1, dval);
		if (fconfig.Read("infr", &dval)) setValue(RulerInfr, dval);
		if (fconfig.Read("df_f", &bval)) setValue(RulerDfoverf, bval);
		if (fconfig.Read("relax iter", &lval)) setValue(RulerRelaxIter, lval);
		if (fconfig.Read("auto relax", &bval)) setValue(RulerRelax, bval);
		if (fconfig.Read("relax type", &lval)) setValue(RulerRelaxType, lval);
		if (fconfig.Read("size thresh", &lval)) setValue(RulerSizeThresh, lval);
	}
	//flags for pvxml flipping
	if (fconfig.Exists("/pvxml"))
	{
		fconfig.SetPath("/pvxml");
		if (fconfig.Read("flip_x", &bval)) setValue(PvxmlFlipX, bval);
		if (fconfig.Read("flip_y", &bval)) setValue(PvxmlFlipY, bval);
		if (fconfig.Read("seq_type", &lval)) setValue(PvxmlSeqType, lval);
	}
	//pixel format
	if (fconfig.Exists("/pixel format"))
	{
		fconfig.SetPath("/pixel format");
		if (fconfig.Read("api_type", &lval)) setValue(ApiType, lval);
		if (fconfig.Read("red_bit", &lval)) setValue(OutputBitR, lval);
		if (fconfig.Read("green_bit", &lval)) setValue(OutputBitG, lval);
		if (fconfig.Read("blue_bit", &lval)) setValue(OutputBitB, lval);
		if (fconfig.Read("alpha_bit", &lval)) setValue(OutputBitA, lval);
		if (fconfig.Read("depth_bit", &lval)) setValue(OutputBitD, lval);
		if (fconfig.Read("samples", &lval)) setValue(OutputSamples, lval);
	}
	//context attrib
	if (fconfig.Exists("/context attrib"))
	{
		fconfig.SetPath("/context attrib");
		if (fconfig.Read("gl_major_ver", &lval)) setValue(GlVersionMajor, lval);
		if (fconfig.Read("gl_minor_ver", &lval)) setValue(GlVersionMinor, lval);
		if (fconfig.Read("gl_profile_mask", &lval)) setValue(GlProfileMask, lval);
	}
	//max texture size
	if (fconfig.Exists("/max texture size"))
	{
		fconfig.SetPath("/max texture size");
		if (fconfig.Read("use_max_texture_size", &bval)) setValue(MaxTextureSizeEnable, bval);
		if (fconfig.Read("max_texture_size", &lval)) setValue(MaxTextureSize, lval);
	}
	//no tex pack
	if (fconfig.Exists("/no tex pack"))
	{
		fconfig.SetPath("/no tex pack");
		if (fconfig.Read("no_tex_pack", &bval)) setValue(NoTexPack, bval);
	}
	//cl device
	if (fconfig.Exists("/cl device"))
	{
		fconfig.SetPath("/cl device");
		if (fconfig.Read("platform_id", &lval)) setValue(ClPlatformId, lval);
		if (fconfig.Read("device_id", &lval)) setValue(ClDeviceId, lval);
	}
	//clipping plane display mode
	if (fconfig.Exists("/clipping planes"))
	{
		fconfig.SetPath("/clipping planes");
		if (fconfig.Read("mode", &lval)) setValue(ClipPlaneMode, lval);
	}

	// java paths load.
	if (fconfig.Exists("/Java"))
	{
		fconfig.SetPath("/Java");
		if (fconfig.Read("jvm_path", &sval)) setValue(JvmPath, sval.ToStdString());
		if (fconfig.Read("ij_path", &sval)) setValue(ImagejPath, sval.ToStdString());
		if (fconfig.Read("bioformats_path", &sval)) setValue(BioformatsPath, sval.ToStdString());
		if (fconfig.Read("ij_mode", &lval)) setValue(ImagejMode, lval);
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
	getValue(PeelNum, lval); fconfig.Write("value", lval);

	fconfig.SetPath("/micro blend");
	getValue(MicroBlendEnable, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/save project");
	getValue(SaveProjectEnable, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/save alpha");
	getValue(CaptureAlpha, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/save float");
	getValue(CaptureFloat, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/realtime compress");
	getValue(HardwareCompress, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/skip bricks");
	getValue(SkipBrick, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/mouse int");
	getValue(Adaptive, bval); fconfig.Write("mode", bval);

	fconfig.SetPath("/dir shadow");
	getValue(ShadowDirEnable, bval); fconfig.Write("mode", bval);
	getValue(ShadowDirX, dval); fconfig.Write("x", dval);
	getValue(ShadowDirY, dval); fconfig.Write("y", dval);

	fconfig.SetPath("/pin threshold");
	getValue(PinThresh, dval); fconfig.Write("value", dval);

	fconfig.SetPath("/stereo");
	getValue(VrEnable, bval); fconfig.Write("enable_stereo", bval);
	getValue(VrEyeOffset, dval); fconfig.Write("eye dist", dval);

	fconfig.SetPath("/test mode");
	getValue(TestSpeed, bval); fconfig.Write("speed", bval);
	getValue(TestParam, bval); fconfig.Write("param", bval);
	getValue(TestWiref, bval); fconfig.Write("wiref", bval);

	fconfig.SetPath("/wavelength to color");
	getValue(WaveColor1, lval); fconfig.Write("c1", lval);
	getValue(WaveColor2, lval); fconfig.Write("c2", lval);
	getValue(WaveColor3, lval); fconfig.Write("c3", lval);
	getValue(WaveColor4, lval); fconfig.Write("c4", lval);

	fconfig.SetPath("/time id");
	getValue(TimeFileId, sval); fconfig.Write("value", wxString(sval));

	fconfig.SetPath("/grad bg");
	getValue(GradBg, bval); fconfig.Write("value", bval);

	fconfig.SetPath("/save cmp");
	getValue(CaptureCompress, bval); fconfig.Write("value", bval);

	fconfig.SetPath("/override vox");
	getValue(OverrideVoxSpc, bval); fconfig.Write("value", bval);

	fconfig.SetPath("/soft threshold");
	getValue(SoftThresh, dval); fconfig.Write("value", dval);

	fconfig.SetPath("/run script");
	getValue(RunScript, bval); fconfig.Write("value", bval);
	getValue(ScriptFile, wsval); fconfig.Write("file", wxString(wsval));

	fconfig.SetPath("/paint history");
	getValue(PaintHistory, lval); fconfig.Write("value", lval);

	fconfig.SetPath("/text font");
	getValue(FontFile, sval); fconfig.Write("file", wxString(sval));
	getValue(TextSize, dval); fconfig.Write("value", dval);
	getValue(TextColorMode, lval); fconfig.Write("color", lval);

	fconfig.SetPath("/line width");
	getValue(LineWidth, dval); fconfig.Write("value", dval);

	//full screen
	fconfig.SetPath("/full screen");
	getValue(StayOnTop, bval); fconfig.Write("stay top", bval);
	getValue(ShowCursor, bval); fconfig.Write("show cursor", bval);

	//last tool
	fconfig.SetPath("/last tool");
	getValue(LastTool, lval); fconfig.Write("value", lval);

	//components
	fconfig.SetPath("/tracking");
	getValue(TrackIter, lval); fconfig.Write("track_iter", lval);
	getValue(CompSizeLimit, lval); fconfig.Write("component_size", lval);
	getValue(CompConsistent, bval); fconfig.Write("consistent_color", bval);
	getValue(TryMerge, bval); fconfig.Write("try_merge", bval);
	getValue(TrySplit, bval); fconfig.Write("try_split", bval);
	getValue(ContactFactor, dval); fconfig.Write("contact_factor", dval);
	getValue(Similarity, dval); fconfig.Write("similarity", dval);

	//memory settings
	fconfig.SetPath("/memory settings");
	getValue(StreamEnable, bval); fconfig.Write("mem swap", bval);
	getValue(GpuMemSize, dval); fconfig.Write("graphics mem", dval);
	getValue(LargeDataSize, dval); fconfig.Write("large data size", dval);
	getValue(BrickSize, lval); fconfig.Write("force brick size", lval);
	getValue(ResponseTime, lval); fconfig.Write("up time", lval);
	getValue(LodOffset, lval); fconfig.Write("detail level offset", lval);

	//update order
	fconfig.SetPath("/update order");
	getValue(UpdateOrder, lval); fconfig.Write("value", lval);

	//point volume mode
	fconfig.SetPath("/point volume mode");
	getValue(PointVolumeMode, lval); fconfig.Write("value", lval);

	//ruler settings
	fconfig.SetPath("/ruler");
	getValue(RulerUseTransf, bval); fconfig.Write("use transf", bval);
	getValue(RulerTransient, bval); fconfig.Write("time dep", bval);
	getValue(RulerF1, dval); fconfig.Write("relax f1", dval);
	getValue(RulerInfr, dval); fconfig.Write("infr", dval);
	getValue(RulerDfoverf, bval); fconfig.Write("df_f", bval);
	getValue(RulerRelaxIter, lval); fconfig.Write("relax iter", lval);
	getValue(RulerRelax, bval); fconfig.Write("auto relax", bval);
	getValue(RulerRelaxType, lval); fconfig.Write("relax type", lval);
	getValue(RulerSizeThresh, lval); fconfig.Write("size thresh", lval);

	//flags for flipping pvxml
	fconfig.SetPath("/pvxml");
	getValue(PvxmlFlipX, bval); fconfig.Write("flip_x", bval);
	getValue(PvxmlFlipY, bval); fconfig.Write("flip_y", bval);
	getValue(PvxmlSeqType, lval); fconfig.Write("seq_type", lval);

	//pixel format
	fconfig.SetPath("/pixel format");
	getValue(ApiType, lval); fconfig.Write("api_type", lval);
	getValue(OutputBitR, lval); fconfig.Write("red_bit", lval);
	getValue(OutputBitG, lval); fconfig.Write("green_bit", lval);
	getValue(OutputBitB, lval); fconfig.Write("blue_bit", lval);
	getValue(OutputBitA, lval); fconfig.Write("alpha_bit", lval);
	getValue(OutputBitD, lval); fconfig.Write("depth_bit", lval);
	getValue(OutputSamples, lval); fconfig.Write("samples", lval);

	//context attrib
	fconfig.SetPath("/context attrib");
	getValue(GlVersionMajor, lval); fconfig.Write("gl_major_ver", lval);
	getValue(GlVersionMinor, lval); fconfig.Write("gl_minor_ver", lval);
	getValue(GlProfileMask, lval); fconfig.Write("gl_profile_mask", lval);

	//max texture size
	fconfig.SetPath("/max texture size");
	getValue(MaxTextureSizeEnable, bval); fconfig.Write("use_max_texture_size", bval);
	getValue(MaxTextureSize, lval); fconfig.Write("max_texture_size", lval);

	//no tex pack
	fconfig.SetPath("/no tex pack");
	getValue(NoTexPack, bval); fconfig.Write("no_tex_pack", bval);

	//cl device
	fconfig.SetPath("/cl device");
	getValue(ClPlatformId, lval); fconfig.Write("platform_id", lval);
	getValue(ClDeviceId, lval); fconfig.Write("device_id", lval);

	//clipping plane mode
	fconfig.SetPath("/clipping planes");
	getValue(ClipPlaneMode, lval); fconfig.Write("mode", lval);

	// java paths
	fconfig.SetPath("/Java");
	getValue(JvmPath, sval); fconfig.Write("jvm_path", wxString(sval));
	getValue(ImagejPath, sval); fconfig.Write("ij_path", wxString(sval));
	getValue(BioformatsPath, sval); fconfig.Write("bioformats_path", wxString(sval));
	getValue(ImagejMode, lval); fconfig.Write("ij_mode", lval);

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
	getValue(MaxTextureSizeEnable, bval);
	if (bval)
	{
		long lval;
		getValue(MaxTextureSize, lval);
		flvr::ShaderProgram::set_max_texture_size(lval);
	}
	else
		flvr::ShaderProgram::reset_max_texture_size();
}

void SettingAgent::OnMaxTextureSize(Event& event)
{
	bool bval;
	getValue(MaxTextureSizeEnable, bval);
	if (bval)
	{
		long lval;
		getValue(MaxTextureSize, lval);
		flvr::ShaderProgram::set_max_texture_size(lval);
	}
}

void SettingAgent::OnFontFile(Event& event)
{
	std::string sval;
	getValue(FontFile, sval);
	wxString exePath = glbin.getExecutablePath();
	wxString loc = exePath + GETSLASH() + "Fonts" +
		GETSLASH() + sval;
	flvr::TextRenderer::text_texture_manager_.load_face(loc.ToStdString());
	double dval;
	getValue(TextSize, dval);
	flvr::TextRenderer::text_texture_manager_.SetSize(dval);
	//for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
	//{
	//	fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
	//	if (!view) continue;
	//	view->setValue(TextSize, m_text_size);
	//	//view->RefreshGL(39);
	//}
}