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
#include <MovieMaker.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <MoviePanel.h>
#include <RenderViewPanel.h>
#include <StopWatch.hpp>
#include <StopWatchFactory.hpp>
#include <VideoEncoder.h>
#include <MainSettings.h>
#include <Project.h>
#include <Interpolator.h>
#include <TextureRenderer.h>
#include <VolumeRenderer.h>
#include <Names.h>
#include <iostream>
#include <filesystem>
#include <tiffio.h>

MovieMaker::MovieMaker() :
	m_frame(0),
	m_view(0),
	m_last_frame(-1),
	m_starting_rot(0),
	m_running(false),
	m_record(false),
	m_delayed_stop(false),
	m_timer_hold(false),
	m_reverse(false),
	m_loop(false),
	m_scroll_thumb_size(20)
{
	m_keyframe_enable = false;
	m_rotate = true;
	m_rot_axis = 1;
	m_rot_deg = 360;
	m_interpolation = 0;
	m_seq_mode = 0;

	m_full_frame_num = 360;
	m_clip_frame_num = 360;
	m_movie_len = 12;
	m_fps = 30;
	m_clip_start_frame = 0;
	m_clip_end_frame = 360;
	m_cur_frame = 0;
	m_cur_time = 0;
	m_cur_prog = 0;

	//seq numbers can be different from output frame numders
	m_seq_cur_num = 0;
	m_seq_all_num = 0;

	m_crop = false;
	m_crop_x = 0;
	m_crop_y = 0;
	m_crop_w = 0;
	m_crop_h = 0;

	m_sb_pos = 3;
	m_sb_x = 5;
	m_sb_y = 5;

	m_cam_lock = false;
	m_cam_lock_type = 1;

}

MovieMaker::~MovieMaker()
{
}

void MovieMaker::Play(bool back)
{
	if (m_running)
	{
		Stop();
	}
	else
	{
		m_reverse = back;
		Start();
	}
}

void MovieMaker::Start()
{
	if (!m_view)
		return;

	m_view->m_begin_play_frame = m_cur_frame;
	flvr::TextureRenderer::maximize_uptime_ = true;
	m_last_frame = 0;

	get_stopwatch()->interval(1000.0 / m_fps);
	get_stopwatch()->start();
	m_running = true;
}

void MovieMaker::Stop()
{
	get_stopwatch()->stop();
	glbin_video_encoder.close();
	m_record = false;
	flvr::TextureRenderer::maximize_uptime_ = false;
	m_reverse = false;
	m_running = false;
}

void MovieMaker::Resume()
{
	if (m_timer_hold)
	{
		get_stopwatch()->interval(1000.0 / m_fps);
		get_stopwatch()->start();
		m_timer_hold = false;
		m_running = true;
	}
}

void MovieMaker::Hold()
{
	if (!m_timer_hold && m_running)
	{
		get_stopwatch()->stop();
		m_timer_hold = true;
		m_running = false;
	}
}

void MovieMaker::Rewind()
{
	if (!m_view)
		return;
	Stop();
	SetCurrentFrame(m_clip_start_frame);
	SetRendering(true);
}

void MovieMaker::Forward()
{
	if (!m_view)
		return;
	Stop();
	SetCurrentFrame(m_clip_end_frame);
	SetRendering(false);
}

void MovieMaker::Reset()
{
	if (!m_view)
		return;
	m_view->m_tseq_cur_num =
		m_view->m_tseq_prv_num = 0;
	SetCurrentFrame(m_clip_start_frame);
	m_last_frame = -1;
	SetRendering(false);
}

void MovieMaker::PlaySave()
{
	if (!m_frame || !m_view)
		return;

	Rewind();

	std::filesystem::path file_path(m_filename);
	filetype_ = file_path.extension().string();
	if (filetype_ == ".mp4")
	{
		if (!m_crop)
		{
			m_crop_x = 0;
			m_crop_y = 0;
			m_crop_w = m_view->GetGLSize().w();
			m_crop_h = m_view->GetGLSize().h();
		}
		else if (m_view->GetEnlarge())
		{
			double scale = m_view->GetEnlargeScale();
			m_crop_w *= scale;
			m_crop_h *= scale;
		}

		glbin_video_encoder.open(m_filename, m_crop_w, m_crop_h,
			m_clip_frame_num + 1, m_fps, glbin_settings.m_mov_bitrate * 1e6);
	}
	m_filename = (file_path.parent_path() / file_path.stem()).wstring();
	m_record = true;
	if (glbin_settings.m_prj_save)
	{
		std::wstring new_folder;
		new_folder = m_filename + L"_project";
		MkDirW(new_folder);
		std::filesystem::path p(m_filename);
		std::wstring name = p.stem().wstring();
		p = new_folder;
		p /= name + L"_project.vrp";
		std::wstring prop_file = p.wstring();
		bool inc = std::filesystem::exists(p) &&
			glbin_settings.m_prj_save_inc;
		glbin_project.Save(prop_file, inc);
	}

	m_view->SetKeepEnlarge(true);
	Play(false);
}

void MovieMaker::SetRendering(bool rewind)
{
	if (!m_view)
		return;

	double t = GetCurProg();
	//advanced options
	if (m_keyframe_enable)
	{
		if (glbin_interpolator.GetLastIndex() > 0)
		{
			m_view->SetLockCamObject(m_cam_lock && m_running);
			m_view->SetParams(t);
		}
	}
	else
	{
		//basic options
		if (m_seq_mode == 1)
		{
			m_view->Set4DSeqFrame(m_seq_cur_num, m_clip_start_frame, m_clip_end_frame, rewind);
		}
		else if (m_seq_mode == 2)
		{
			m_view->Set3DBatFrame(m_seq_cur_num, m_clip_start_frame, m_clip_end_frame, rewind);
		}

		//rotate animation
		if (m_rotate)
		{
			double val;
			fluo::Vector rval = m_view->GetRotations();
			val = rval[m_rot_axis];
			if (m_interpolation == 0)
				val = m_starting_rot + t * m_rot_deg;
			else if (m_interpolation == 1)
				val = m_starting_rot +
				(-2.0 * t * t * t + 3.0 * t * t) * m_rot_deg;
			rval[m_rot_axis] = val;
			m_view->SetRotations(rval, true);
		}
	}
	if (m_running)
	{
		m_view->SetInteractive(false);
		m_frame->UpdateProps({
			gstMovProgSlider,
			gstCurrentFrame,
			gstMovCurTime,
			gstMovPlay,
			gstMovSeqNum },
			2, m_frame->GetMoviePanel());
	}
}

void MovieMaker::WriteFrameToFile()
{
	if (!m_view)
		return;

	std::wostringstream oss;
	oss << m_clip_frame_num;
	std::wstring s_length = oss.str();
	int length = s_length.length();
	oss.str(L""); oss.clear();
	oss << m_filename << L"_" << std::setfill(L'0') << std::setw(length) << m_last_frame << L".tif";
	std::wstring outputfilename = oss.str();

	//capture
	bool bmov = filetype_ == ".mp4";
	int chann = glbin_settings.m_save_alpha ? 4 : 3;
	bool fp32 = bmov ? false : glbin_settings.m_save_float;
	float dpi = glbin_settings.m_dpi;
	int x, y, w, h;
	void* image = 0;
	m_view->ReadPixels(chann, fp32, x, y, w, h, &image);

	if (bmov)
	{
		//flip vertically 
		unsigned char* flip = new unsigned char[w * h * 3];
		for (size_t yy = 0; yy < (size_t)h; yy++)
			for (size_t xx = 0; xx < (size_t)w; xx++)
				memcpy(flip + 3 * (w * yy + xx), (unsigned char*)image + chann * (w * (h - yy - 1) + xx), 3);
		glbin_video_encoder.set_frame_rgb_data(flip);
		glbin_video_encoder.write_video_frame(m_last_frame);
		if (flip)
			delete[]flip;
		if (image)
			delete[]image;
	}
	else
	{
		TIFF* out = TIFFOpenW(outputfilename.c_str(), "wb");
		if (!out) return;
		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
		if (fp32)
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		else
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
			//TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		if (glbin_settings.m_save_compress)
			TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
		//dpi
		TIFFSetField(out, TIFFTAG_XRESOLUTION, dpi);
		TIFFSetField(out, TIFFTAG_YRESOLUTION, dpi);
		TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

		tsize_t linebytes = chann * w * (fp32 ? 4 : 1);
		void* buf = NULL;
		buf = _TIFFmalloc(linebytes);
		//TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
		for (uint32 row = 0; row < (uint32)h; row++)
		{
			if (fp32)
			{
				float* line = ((float*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			else
			{// check the index here, and figure out why not using h*linebytes
				unsigned char* line = ((unsigned char*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			if (TIFFWriteScanline(out, buf, row, 0) < 0)
				break;
		}
		TIFFClose(out);
		if (buf)
			_TIFFfree(buf);
		if (image)
			delete[]image;
	}
}

void MovieMaker::SetMainFrame(MainFrame* frame)
{
	m_frame = frame;
}

void MovieMaker::SetView(RenderView* view)
{
	m_view = view;
}

RenderView* MovieMaker::GetView()
{
	return m_view;
}

int MovieMaker::GetViewIndex()
{
	return glbin_current.GetViewId(m_view);
}

void MovieMaker::SetKeyframeEnable(bool val)
{
	if (val == m_keyframe_enable)
		return;
	m_keyframe_enable = val;
	if (m_keyframe_enable)
	{
		//get settings from interpolator
		int n = std::round(glbin_interpolator.GetLastT());
		SetFullFrameNum(n);
	}
	else
	{
		//get settings from basic
		switch (m_seq_mode)
		{
		case 0:
			SetRotateEnable(true);
			break;
		case 1:
			SetSeqMode(1);
			break;
		case 2:
			SetSeqMode(2);
			break;
		}
	}
	SetCurrentFrame(m_clip_start_frame);
}

void MovieMaker::SetRotateEnable(bool val)
{
	m_keyframe_enable = false;
	if (val)
	{
		m_rotate = true;
		SetRotateDeg(m_rot_deg);
	}
	else
	{
		if (m_seq_mode != 0)
			m_rotate = false;
	}
}

void MovieMaker::SetRotateAxis(int val)
{
	if (val > -1 && val < 3)
		m_rot_axis = val;
}

void MovieMaker::SetRotateDeg(int val)
{
	m_rot_deg = val;
	if (!m_keyframe_enable && m_rotate && m_seq_mode == 0)
	{
		SetFullFrameNum(val);
	}
}

void MovieMaker::SetSeqMode(int val)
{
	if (val > -1 && val < 3)
		m_seq_mode = val;
	m_keyframe_enable = false;

	int sf, ef;
	switch (m_seq_mode)
	{
	case 0:
		SetRotateEnable(true);
		break;
	case 1:
		if (m_view)
		{
			m_view->Get4DSeqRange(sf, ef);
			if (ef > 0)
			{
				m_rotate = false;
				m_seq_all_num = ef - sf;
				SetFullFrameNum(m_seq_all_num);
				SetSeqCurNum(m_view->m_tseq_cur_num);
				//SetClipStartEndFrames(0, ef);
			}
			else
			{
				m_seq_mode = 0;
				SetRotateEnable(true);
			}
		}
		break;
	case 2:
		if (m_view)
		{
			m_view->Get3DBatRange(sf, ef);
			if (ef > 0)
			{
				m_rotate = false;
				m_seq_all_num = ef - sf;
				SetFullFrameNum(m_seq_all_num);
				SetSeqCurNum(m_view->m_tseq_cur_num);
				//SetClipStartEndFrames(0, ef);
			}
			else
			{
				m_seq_mode = 0;
				m_seq_all_num = 0;
				m_seq_cur_num = 0;
				SetRotateEnable(true);
			}
		}
		break;
	}
}

void MovieMaker::SetSeqCurNum(int val)
{
	if (m_seq_all_num == 0)
		return;
	m_seq_cur_num = fluo::RotateClamp2(val, 0, m_seq_all_num);
	if (m_keyframe_enable)
	{
		if (m_seq_mode == 1)
		{
			m_view->Set4DSeqFrame(m_seq_cur_num, m_clip_start_frame, m_clip_end_frame, false);
		}
		else if (m_seq_mode == 2)
		{
			m_view->Set3DBatFrame(m_seq_cur_num, m_clip_start_frame, m_clip_end_frame, false);
		}
	}
	else if (m_seq_mode > 0)
	{
		m_cur_frame = std::round((double)m_seq_cur_num * m_full_frame_num / m_seq_all_num);
		SetCurrentFrame(m_cur_frame, false);
	}
}

void MovieMaker::SetFullFrameNum(int val)
{
	//it resets the whole movie length
	m_full_frame_num = val;
	SetClipStartEndFrames(0, val);
}

void MovieMaker::SetClipStartEndFrames(int val1, int val2)
{
	m_clip_start_frame = val1 >= 0 ? val1 : 0;
	m_clip_end_frame = val2;
	if (m_clip_start_frame >= m_clip_end_frame)
		m_clip_end_frame = m_clip_start_frame + 1;
	m_clip_frame_num = m_clip_end_frame - m_clip_start_frame;
	if (m_clip_end_frame > m_full_frame_num)
		m_full_frame_num = val2;
	if (m_fps > 0)
		m_movie_len = m_clip_frame_num / m_fps;
	SetCurrentFrame(m_cur_frame);
}

void MovieMaker::SetClipStartFrame(int val)
{
	m_clip_start_frame = val >= 0 ? val : 0;
	if (m_clip_start_frame >= m_clip_end_frame)
		m_clip_start_frame = m_clip_end_frame - 1;
	m_clip_frame_num = m_clip_end_frame - m_clip_start_frame;
	if (m_fps > 0)
		m_movie_len = m_clip_frame_num / m_fps;
	SetCurrentFrame(m_cur_frame);
}

void MovieMaker::SetClipEndFrame(int val)
{
	m_clip_end_frame = val;
	if (m_clip_start_frame >= m_clip_end_frame)
		m_clip_end_frame = m_clip_start_frame + 1;
	m_clip_frame_num = m_clip_end_frame - m_clip_start_frame;
	if (m_clip_end_frame > m_full_frame_num)
		m_full_frame_num = val;
	if (m_fps > 0)
		m_movie_len = m_clip_frame_num / m_fps;
	SetCurrentFrame(m_cur_frame);
}

void MovieMaker::SetCurrentFrame(int val, bool upd_seq)
{
	if (m_rotate && !IsPaused())
	{
		fluo::Vector rval = m_view->GetRotations();
		double r = rval[m_rot_axis];
		m_starting_rot = fluo::RotateClamp(r, 0, 360);
	}
	m_cur_frame = fluo::RotateClamp2(val, m_clip_start_frame, m_clip_end_frame);
	m_cur_time = (m_cur_frame - m_clip_start_frame) / m_fps;
	if (upd_seq && !m_keyframe_enable && m_seq_mode > 0)
	{
		m_seq_cur_num = std::round((double)m_cur_frame * m_seq_all_num / m_full_frame_num);
	}

	SetRendering(false);
}

void MovieMaker::SetCurrentTime(double val)
{
	int frame = m_clip_start_frame + std::round(val * m_fps);
	SetCurrentFrame(frame);
}

void MovieMaker::SetCropEnable(bool val)
{
	m_crop = val;
	if (m_view)
	{
		if (val)
		{
			m_view->CalcFrame();
			m_view->EnableFrame();
		}
		else
			m_view->DisableFrame();
	}
}

void MovieMaker::SetCropValues(int x, int y, int w, int h)
{
	m_crop_x = x;
	m_crop_y = y;
	m_crop_w = w;
	m_crop_h = h;
}

void MovieMaker::SetCropX(int val)
{
	m_crop_x = val;
}

void MovieMaker::SetCropY(int val)
{
	m_crop_y = val;
}

void MovieMaker::SetCropW(int val)
{
	m_crop_w = val;
}

void MovieMaker::SetCropH(int val)
{
	m_crop_h = val;
}

void MovieMaker::InsertKey(int index)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	if (!m_view)
		m_view = root->GetView(0);
	if (!m_view)
		return;

	FlKeyCode keycode;
	FlKeyDouble* flkey = 0;
	FlKeyQuaternion* flkeyQ = 0;
	FlKeyBoolean* flkeyB = 0;
	FlKeyInt* flkeyI = 0;
	FlKeyColor* flkeyC = 0;

	double t = glbin_interpolator.GetLastT();
	int kn = glbin_interpolator.GetKeyNum();
	t = kn == 0 ? t : t + m_key_duration;

	glbin_interpolator.Begin(t, m_key_duration);

	//for all volumes
	for (int i = 0; i < glbin_data_manager.GetVolumeNum(); i++)
	{
		VolumeData* vd = glbin_data_manager.GetVolumeData(i);
		keycode.l0 = 1;
		keycode.l0_name = ws2s(m_view->GetName());
		keycode.l1 = 2;
		keycode.l1_name = ws2s(vd->GetName());
		//display
		keycode.l2 = 0;
		keycode.l2_name = "display";
		flkeyB = new FlKeyBoolean(keycode, vd->GetDisp());
		glbin_interpolator.AddKey(flkeyB);
		//clipping planes
		std::vector<fluo::Plane*>* planes = vd->GetVR()->get_planes();
		if (!planes)
			continue;
		if (planes->size() != 6)
			continue;
		fluo::Plane* plane = 0;
		double abcd[4];
		//x1
		plane = (*planes)[0];
		plane->get_copy(abcd);
		keycode.l2_name = "x1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//x2
		plane = (*planes)[1];
		plane->get_copy(abcd);
		keycode.l2_name = "x2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//y1
		plane = (*planes)[2];
		plane->get_copy(abcd);
		keycode.l2_name = "y1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//y2
		plane = (*planes)[3];
		plane->get_copy(abcd);
		keycode.l2_name = "y2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//z1
		plane = (*planes)[4];
		plane->get_copy(abcd);
		keycode.l2_name = "z1_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//z2
		plane = (*planes)[5];
		plane->get_copy(abcd);
		keycode.l2_name = "z2_val";
		flkey = new FlKeyDouble(keycode, abs(abcd[3]));
		glbin_interpolator.AddKey(flkey);
		//t
		int frame = vd->GetCurTime();
		keycode.l2_name = "frame";
		flkey = new FlKeyDouble(keycode, frame);
		glbin_interpolator.AddKey(flkey);
		//primary color
		fluo::Color pc = vd->GetColor();
		keycode.l2_name = "color";
		flkeyC = new FlKeyColor(keycode, pc);
		glbin_interpolator.AddKey(flkeyC);

		//volume properties
		//minmax
		keycode.l2_name = "minmax enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetMinMaxEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "low_offset";
		flkey = new FlKeyDouble(keycode, vd->GetLowOffset());
		glbin_interpolator.AddKey(flkey);
		keycode.l2_name = "high_offset";
		flkey = new FlKeyDouble(keycode, vd->GetHighOffset());
		glbin_interpolator.AddKey(flkey);
		//gamma
		keycode.l2_name = "gamma3d enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetGammaEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "gamma3d";
		flkey = new FlKeyDouble(keycode, vd->GetGamma());
		glbin_interpolator.AddKey(flkey);
		//alpha
		keycode.l2_name = "alpha enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetAlphaEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "alpha";
		flkey = new FlKeyDouble(keycode, vd->GetAlpha());
		glbin_interpolator.AddKey(flkey);
		//luminance is color
		//sample rate
		keycode.l2_name = "sample rate enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetSampleRateEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "sample rate";
		flkey = new FlKeyDouble(keycode, vd->GetSampleRate());
		glbin_interpolator.AddKey(flkey);
		//threshold
		keycode.l2_name = "threshold enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetThreshEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "low threshold";
		flkey = new FlKeyDouble(keycode, vd->GetLeftThresh());
		glbin_interpolator.AddKey(flkey);
		keycode.l2_name = "high threshold";
		flkey = new FlKeyDouble(keycode, vd->GetRightThresh());
		glbin_interpolator.AddKey(flkey);
		//boundary
		keycode.l2_name = "boundary enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetBoundaryEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "boundary";
		flkey = new FlKeyDouble(keycode, vd->GetBoundary());
		glbin_interpolator.AddKey(flkey);
		//shading
		keycode.l2_name = "shading enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetShadingEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "low shading";
		flkey = new FlKeyDouble(keycode, vd->GetLowShading());
		glbin_interpolator.AddKey(flkey);
		keycode.l2_name = "high shading";
		flkey = new FlKeyDouble(keycode, vd->GetHiShading());
		glbin_interpolator.AddKey(flkey);
		//shadow
		keycode.l2_name = "shadow enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetShadowEnable());
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "shadow intensity";
		flkey = new FlKeyDouble(keycode, vd->GetShadowIntensity());
		glbin_interpolator.AddKey(flkey);
		//colormap
		keycode.l2_name = "colormap enable";
		flkeyB = new FlKeyBoolean(keycode, vd->GetColormapMode() > 0);
		glbin_interpolator.AddKey(flkeyB);
		keycode.l2_name = "colormap low";
		flkey = new FlKeyDouble(keycode, vd->GetColormapLow());
		glbin_interpolator.AddKey(flkey);
		keycode.l2_name = "colormap high";
		flkey = new FlKeyDouble(keycode, vd->GetColormapHigh());
		glbin_interpolator.AddKey(flkey);
	}
	//for the view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	keycode.l1 = 1;
	keycode.l1_name = ws2s(m_view->GetName());
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";
	fluo::Quaternion q = m_view->GetRotQuat();
	flkeyQ = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkeyQ);
	//translation
	fluo::Vector vval = m_view->GetTranslations();
	//x
	keycode.l2_name = "translation_x";
	flkey = new FlKeyDouble(keycode, vval.x());
	glbin_interpolator.AddKey(flkey);
	//y
	keycode.l2_name = "translation_y";
	flkey = new FlKeyDouble(keycode, vval.y());
	glbin_interpolator.AddKey(flkey);
	//z
	keycode.l2_name = "translation_z";
	flkey = new FlKeyDouble(keycode, vval.z());
	glbin_interpolator.AddKey(flkey);
	//centers
	fluo::Point pval = m_view->GetCenters();
	//x
	keycode.l2_name = "center_x";
	flkey = new FlKeyDouble(keycode, pval.x());
	glbin_interpolator.AddKey(flkey);
	//y
	keycode.l2_name = "center_y";
	flkey = new FlKeyDouble(keycode, pval.y());
	glbin_interpolator.AddKey(flkey);
	//z
	keycode.l2_name = "center_z";
	flkey = new FlKeyDouble(keycode, pval.z());
	glbin_interpolator.AddKey(flkey);
	//obj traslation
	vval = m_view->GetObjTrans();
	//x
	keycode.l2_name = "obj_trans_x";
	flkey = new FlKeyDouble(keycode, vval.x());
	glbin_interpolator.AddKey(flkey);
	//y
	keycode.l2_name = "obj_trans_y";
	flkey = new FlKeyDouble(keycode, vval.y());
	glbin_interpolator.AddKey(flkey);
	//z
	keycode.l2_name = "obj_trans_z";
	flkey = new FlKeyDouble(keycode, vval.z());
	glbin_interpolator.AddKey(flkey);
	//scale
	double scale = m_view->m_scale_factor;
	keycode.l2_name = "scale";
	flkey = new FlKeyDouble(keycode, scale);
	glbin_interpolator.AddKey(flkey);
	//intermixing mode
	int ival = m_view->GetVolMethod();
	keycode.l2_name = "volmethod";
	flkeyI = new FlKeyInt(keycode, ival);
	glbin_interpolator.AddKey(flkeyI);
	//perspective angle
	bool persp = m_view->GetPersp();
	double aov = m_view->GetAov();
	if (!persp)
		aov = 9.9;
	keycode.l2_name = "aov";
	flkey = new FlKeyDouble(keycode, aov);
	glbin_interpolator.AddKey(flkey);
	//shadow dir
	keycode.l2_name = "shadow dir enable";
	flkeyB = new FlKeyBoolean(keycode, glbin_settings.m_shadow_dir);
	glbin_interpolator.AddKey(flkeyB);
	keycode.l2_name = "shadow dir x";
	flkey = new FlKeyDouble(keycode, glbin_settings.m_shadow_dir_x);
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "shadow dir y";
	flkey = new FlKeyDouble(keycode, glbin_settings.m_shadow_dir_y);
	glbin_interpolator.AddKey(flkey);

	glbin_interpolator.End();

	FlKeyGroup* group = glbin_interpolator.GetKeyGroup(glbin_interpolator.GetLastIndex());
	if (group)
		group->type = m_interpolation;

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
	glbin_moviemaker.SetCurrentFrame(glbin_moviemaker.GetClipEndFrame());
}

bool MovieMaker::Action()
{
	if (!m_running)
		return false;
	if (!get_stopwatch()->check())
		return false;

	//get all of the progress info
	if (m_delayed_stop)
	{
		m_delayed_stop = false;
		if (m_record)
			WriteFrameToFile();
		SetCurrentFrame(m_cur_frame);
		Stop();
		m_view->SetKeepEnlarge(false);
		return true;
	}

	//if (flvr::TextureRenderer::get_mem_swap() &&
	//	flvr::TextureRenderer::get_start_update_loop() &&
	//	!flvr::TextureRenderer::get_done_update_loop())
	//{
	//	if (!m_view) return;
	//	m_view->SetInteractive(false);
	//	m_view->RefreshGL(39, false, false);
	//	return;
	//}

	//move time
	if (m_reverse)
		SetCurrentFrame(m_cur_frame - 1);
	else
		SetCurrentFrame(m_cur_frame + 1);

	//update the rendering frame since we have advanced.
	if (m_last_frame != m_cur_frame)
	{
		if (m_record)
			WriteFrameToFile();
		m_last_frame = m_cur_frame;
		SetRendering(false);
	}

	//check stop
	if (!m_loop || m_record)
	{
		if (m_reverse)
		{
			if (m_cur_frame == m_clip_start_frame)
				m_delayed_stop = true;
		}
		else
		{
			if (m_cur_frame == m_clip_end_frame)
				m_delayed_stop = true;
		}
	}

	return true;
}

fluo::StopWatch* MovieMaker::get_stopwatch()
{
	fluo::StopWatch* result = glbin_swhf.findFirst(gstMovStopWatch);
	if (!result)
	{
		result = glbin_swhf.build();
		result->setName(gstMovStopWatch);
		result->interval(1000.0 / m_fps);
	}
	return result;
}

void MovieMaker::MakeKeys(int type)
{
	switch (type)
	{
	case 0:
		MakeKeysCameraTumble();
		break;
	case 1:
		MakeKeysCameraZoom();
		break;
	case 2:
		MakeKeysTimeSequence();
		break;
	case 3:
		MakeKeysTimeColormap();
		break;
	case 4:
		MakeKeysClipZ(0);
		break;
	case 5:
		MakeKeysClipZ(1);
		break;
	case 6:
		AddChannToView();
		break;
	case 7:
		KeyChannComb();
		break;
	case 8:
		MakeKeysChannComb(1);
		break;
	case 9:
		MakeKeysChannComb(2);
		break;
	case 10:
		MakeKeysChannComb(3);
		break;
	case 11:
		MakeKeysLookingGlass(44);
	}
}

std::vector<std::string> MovieMaker::GetAutoKeyTypes()
{
	std::vector<std::string> result;
	//0
	result.push_back("Camera tumble left and right");
	//1
	result.push_back("Camera zoom in and out");
	//2
	result.push_back("Time sequence playback and reverse");
	//3
	result.push_back("Time sequence change colors");
	//4
	result.push_back("Slice down the Z direction and back");
	//5
	result.push_back("Single Z section move down and back");
	//6
	result.push_back("Add channels one by one to view");
	//7
	result.push_back("Channel combinations");
	//8
	result.push_back("Channel combination nC1");
	//9
	result.push_back("Channel combination nC2");
	//10
	result.push_back("Channel combination nC3");
	//11
	result.push_back("Looking Glass light field");
	return result;
}

void MovieMaker::MakeKeysCameraTumble()
{
	FlKeyCode keycode;
	FlKeyQuaternion* flkey = 0;
	FlKeyGroup* kg = 0;
	fluo::Quaternion q;
	double x, y, z;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += m_key_duration;

	//for the view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	keycode.l1 = 1;
	keycode.l1_name = ws2s(m_view->GetName());
	//rotation
	keycode.l2 = 0;
	keycode.l2_name = "rotation";

	//initial
	glbin_interpolator.Begin(t, m_key_duration);
	q = m_view->GetRotQuat();
	flkey = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Initial angle";
	t += m_key_duration;
	//negative y
	glbin_interpolator.Begin(t, m_key_duration);
	q.ToEuler(x, y, z);
	y -= m_key_duration;
	q.FromEuler(x, y, z);
	flkey = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Rotate " + std::to_wstring(int(m_key_duration)) + L" degrees in one direction";
	t += m_key_duration;
	//restore
	glbin_interpolator.Begin(t, m_key_duration);
	q = m_view->GetRotQuat();
	flkey = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Rotate back to the initial angle";
	t += m_key_duration;
	//positive y
	glbin_interpolator.Begin(t, m_key_duration);
	q.ToEuler(x, y, z);
	y += m_key_duration;
	q.FromEuler(x, y, z);
	flkey = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Rotate " + std::to_wstring(int(m_key_duration)) + L" degrees in aother direction";
	t += m_key_duration;
	//restore
	glbin_interpolator.Begin(t, m_key_duration);
	q = m_view->GetRotQuat();
	flkey = new FlKeyQuaternion(keycode, q);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Rotate back to the initial angle";

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
}

void MovieMaker::MakeKeysCameraZoom()
{
	FlKeyCode keycode;
	FlKeyDouble* flkey = 0;
	FlKeyGroup* kg = 0;
	double val = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += m_key_duration;

	//for the view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	keycode.l1 = 1;
	keycode.l1_name = ws2s(m_view->GetName());
	//scale
	keycode.l2 = 0;
	keycode.l2_name = "scale";

	//initial
	glbin_interpolator.Begin(t, m_key_duration);
	val = m_view->m_scale_factor;
	flkey = new FlKeyDouble(keycode, val);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Initial zoom factor";
	t += m_key_duration;
	//zoom in
	glbin_interpolator.Begin(t, m_key_duration);
	val *= m_key_duration / 10;
	flkey = new FlKeyDouble(keycode, val);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Zoom in to " + std::to_wstring(int(val)) + L" times";
	t += m_key_duration;
	//restore
	glbin_interpolator.Begin(t, m_key_duration);
	val = m_view->m_scale_factor;
	flkey = new FlKeyDouble(keycode, val);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Zoom out to the initial view";

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
}

void MovieMaker::MakeKeysTimeSequence()
{
	if (m_seq_all_num <= 0)
		return;

	FlKeyCode keycode;
	FlKeyDouble* flkey = 0;
	FlKeyGroup* kg = 0;
	double val = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += m_key_duration;

	//for the view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	//time point
	keycode.l2 = 0;
	keycode.l2_name = "frame";

	//initial
	glbin_interpolator.Begin(t, m_seq_all_num);
	for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		keycode.l1 = 2;
		keycode.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode, 0);
		glbin_interpolator.AddKey(flkey);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Initial time point";
	t += m_seq_all_num;
	//move to end
	glbin_interpolator.Begin(t, m_seq_all_num);
	for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		keycode.l1 = 2;
		keycode.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode, m_seq_all_num);
		glbin_interpolator.AddKey(flkey);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"End of time sequence";
	t += m_seq_all_num;
	//restore
	glbin_interpolator.Begin(t, m_seq_all_num);
	for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		keycode.l1 = 2;
		keycode.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode, 0);
		glbin_interpolator.AddKey(flkey);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Back to the initial time point";

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
}

void MovieMaker::MakeKeysTimeColormap()
{
	if (m_seq_all_num <= 0)
		return;

	fluo::Color c[7];
	double v[7];
	int tt[7];
	int dt[7];
	VolumeData* vd = glbin_current.vol_data;
	if (!vd)
		vd = m_view->GetAllVolumeData(0);
	if (!vd)
		return;
	double low, high;
	vd->GetColormapValues(low, high);
	v[0] = 0;
	v[1] = low;
	v[3] = (low + high) / 2.0;
	v[2] = (low + v[3]) / 2.0;
	v[4] = (v[3] + high) / 2.0;
	v[5] = high;
	v[6] = 1;
	for (int i = 0; i < 7; ++i)
	{
		c[i] = vd->GetColorFromColormap(v[i]);
		tt[i] = std::round(m_seq_all_num * v[i]);
	}
	dt[0] = 0;
	for (int i = 1; i < 7; ++i)
		dt[i] = tt[i] - tt[i - 1];

	FlKeyCode keycode;
	FlKeyColor* flkey = 0;
	FlKeyDouble* flkey_frame = 0;
	FlKeyGroup* kg = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += m_key_duration;

	////for the view
	//keycode.l0 = 1;
	//keycode.l0_name = ws2s(m_view->GetName());
	////time point
	//keycode.l2 = 0;
	//keycode.l2_name = "frame";

	////initial
	//glbin_interpolator.Begin(t, m_seq_all_num);
	//for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	//{
	//	VolumeData* vd = m_view->GetAllVolumeData(i);
	//	if (!vd)
	//		continue;
	//	keycode.l1 = 2;
	//	keycode.l1_name = vd->GetName();
	//	flkey = new FlKeyDouble(keycode, 0);
	//	glbin_interpolator.AddKey(flkey);
	//}
	//glbin_interpolator.End();

	//for the view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	//color
	keycode.l2 = 0;

	for (int i = 0; i < 7; ++i)
	{
		if (i == 0 && tt[i] == tt[i + 1])
			continue;
		if (i == 6 && tt[i] == tt[i - 1])
			continue;
		glbin_interpolator.Begin(t, dt[i]);
		for (int j = 0; j < m_view->GetAllVolumeNum(); ++j)
		{
			VolumeData* vd = m_view->GetAllVolumeData(j);
			if (!vd)
				continue;
			keycode.l1 = 2;
			keycode.l1_name = ws2s(vd->GetName());
			keycode.l2_name = "color";
			flkey = new FlKeyColor(keycode, c[i]);
			glbin_interpolator.AddKey(flkey);
			//time
			keycode.l2_name = "frame";
			flkey_frame = new FlKeyDouble(keycode, tt[i]);
			glbin_interpolator.AddKey(flkey_frame);
		}
		glbin_interpolator.End();
		kg = glbin_interpolator.GetKeyGroupFromTime(t);
		if (kg) kg->desc = L"Change time sequence to color " + std::to_wstring(i + 1);
		if (i < 6)
			t += dt[i + 1];
	}

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
}

void MovieMaker::MakeKeysClipZ(int type)
{
	int n = m_view->GetAllVolumeNum();
	if (n <= 0)
		return;

	int nz = 0;
	int x, y, z;
	for (int i = 0; i < n; ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		vd->GetResolution(x, y, z);
		nz = std::max(z, nz);
	}
	if (nz <= 1)
		return;
	nz -= 1;

	FlKeyCode keycode1, keycode2;
	FlKeyDouble* flkey = 0;
	FlKeyGroup* kg = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += nz;

	//for the view
	keycode1.l0 = 1;
	keycode1.l0_name = ws2s(m_view->GetName());
	keycode2.l0 = 1;
	keycode2.l0_name = ws2s(m_view->GetName());
	//time point
	keycode1.l2 = 0;
	keycode1.l2_name = "z1_val";
	keycode2.l2 = 0;
	keycode2.l2_name = "z2_val";

	//initial
	glbin_interpolator.Begin(t, nz);
	for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		keycode1.l1 = 2;
		keycode1.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode1, 0);
		glbin_interpolator.AddKey(flkey);
		keycode2.l1 = 2;
		keycode2.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode2,
			type ? 1.0 / z : 1);
		glbin_interpolator.AddKey(flkey);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = type ? L"First Z section" : L"Initial clipping plane position";
	t += nz;
	//move down
	glbin_interpolator.Begin(t, nz);
	for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		keycode1.l1 = 2;
		keycode1.l1_name = ws2s(vd->GetName());
		int x, y, z;
		vd->GetResolution(x, y, z);
		flkey = new FlKeyDouble(keycode1, 1 - 1.0 / z);
		glbin_interpolator.AddKey(flkey);
		keycode2.l1 = 2;
		keycode2.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode2, 1);
		glbin_interpolator.AddKey(flkey);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = type ? L"Last Z section"  : L"Slice the Z plane down";
	t += nz;
	//restore
	glbin_interpolator.Begin(t, nz);
	for (int i = 0; i < m_view->GetAllVolumeNum(); ++i)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		if (!vd)
			continue;
		keycode1.l1 = 2;
		keycode1.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode1, 0);
		glbin_interpolator.AddKey(flkey);
		keycode2.l1 = 2;
		keycode2.l1_name = ws2s(vd->GetName());
		flkey = new FlKeyDouble(keycode2,
			type ? 1.0 / z : 1);
		glbin_interpolator.AddKey(flkey);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = type ? L"Back to the first Z section"  : L"Slice back up to the intial clipping plane position";

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
}

void MovieMaker::AddChannToView()
{
	FlKeyCode keycode;
	FlKeyBoolean* flkeyB = 0;
	FlKeyGroup* kg = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += m_key_duration;

	int numChan = m_view->GetAllVolumeNum();

	//for view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	//display only
	keycode.l2 = 0;
	keycode.l2_name = "display";

	//initial
	glbin_interpolator.Begin(t, m_key_duration);
	//for all volumes
	for (int i = 0; i < numChan; i++)
	{
		VolumeData* vd = m_view->GetAllVolumeData(i);
		keycode.l1 = 2;
		keycode.l1_name = ws2s(vd->GetName());
		flkeyB = new FlKeyBoolean(keycode, false);
		glbin_interpolator.AddKey(flkeyB);
	}
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"All channels turned off";
	t += m_key_duration;
	//one by one
	for (int i = 0; i < numChan; i++)
	{
		glbin_interpolator.Begin(t, m_key_duration);
		//for all volumes
		for (int j = 0; j < numChan; j++)
		{
			VolumeData* vd = m_view->GetAllVolumeData(j);
			keycode.l1 = 2;
			keycode.l1_name = ws2s(vd->GetName());
			flkeyB = new FlKeyBoolean(keycode, j <= i);
			glbin_interpolator.AddKey(flkeyB);
		}
		glbin_interpolator.End();
		kg = glbin_interpolator.GetKeyGroupFromTime(t);
		if (kg) kg->desc = L"Channel " + std::to_wstring(i) + L" added to view";
		t += m_key_duration;
	}

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()) + m_key_duration);
}

void MovieMaker::MakeKeysChannComb(int comb)
{
	Root* root = glbin_data_manager.GetRoot();
	if (!root)
		return;
	if (!m_view)
		m_view = root->GetView(0);
	if (!m_view)
		return;

	FlKeyCode keycode;
	FlKeyBoolean* flkeyB = 0;
	FlKeyGroup* kg = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += m_key_duration;

	int i;
	int numChan = m_view->GetAllVolumeNum();
	std::vector<bool> chan_mask;
	//initiate mask
	for (i = 0; i < numChan; i++)
	{
		if (i < comb)
			chan_mask.push_back(true);
		else
			chan_mask.push_back(false);
	}

	do
	{
		glbin_interpolator.Begin(t, m_key_duration);

		//for all volumes
		for (i = 0; i < m_view->GetAllVolumeNum(); i++)
		{
			VolumeData* vd = m_view->GetAllVolumeData(i);
			keycode.l0 = 1;
			keycode.l0_name = ws2s(m_view->GetName());
			keycode.l1 = 2;
			keycode.l1_name = ws2s(vd->GetName());
			//display only
			keycode.l2 = 0;
			keycode.l2_name = "display";
			flkeyB = new FlKeyBoolean(keycode, chan_mask[i]);
			glbin_interpolator.AddKey(flkeyB);
		}

		glbin_interpolator.End();
		kg = glbin_interpolator.GetKeyGroupFromTime(t);
		if (kg) kg->desc = L"Displaying " + std::to_wstring(comb) + (comb == 1 ? L" channel" : L" channels");
		t += m_key_duration;
	} while (GetMask(chan_mask));

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()) + m_key_duration);
}

bool MovieMaker::MoveOne(std::vector<bool>& chan_mask, int lv)
{
	int i;
	int cur_lv = 0;
	int lv_pos = -1;
	for (i = (int)chan_mask.size() - 1; i >= 0; i--)
	{
		if (chan_mask[i])
		{
			cur_lv++;
			if (cur_lv == lv)
			{
				lv_pos = i;
				break;
			}
		}
	}
	if (lv_pos >= 0)
	{
		if (lv_pos == (int)chan_mask.size() - lv)
			return MoveOne(chan_mask, ++lv);
		else
		{
			if (!chan_mask[lv_pos + 1])
			{
				for (i = lv_pos; i < (int)chan_mask.size(); i++)
				{
					if (i == lv_pos)
						chan_mask[i] = false;
					else if (i <= lv_pos + lv)
						chan_mask[i] = true;
					else
						chan_mask[i] = false;
				}
				return true;
			}
			else return false;//no space anymore
		}
	}
	else return false;
}

bool MovieMaker::GetMask(std::vector<bool>& chan_mask)
{
	return MoveOne(chan_mask, 1);
}

void MovieMaker::KeyChannComb()
{
	MakeKeysChannComb(1);
	MakeKeysChannComb(2);
	MakeKeysChannComb(3);
}

void MovieMaker::MakeKeysLookingGlass(int frames)
{
	FlKeyCode keycode;
	FlKeyDouble* flkey = 0;
	FlKeyGroup* kg = 0;

	double t = glbin_interpolator.GetLastT();
	if (t > 0.0) t += frames;

	fluo::Vector side = m_view->GetSide();
	fluo::Vector trans = side * (m_view->m_ortho_right - m_view->m_ortho_left);
	trans /= 4;

	//for the view
	keycode.l0 = 1;
	keycode.l0_name = ws2s(m_view->GetName());
	keycode.l1 = 1;
	keycode.l1_name = ws2s(m_view->GetName());
	//scale
	keycode.l2 = 0;

	//left
	glbin_interpolator.Begin(t, frames);
	keycode.l2_name = "obj_trans_x";
	flkey = new FlKeyDouble(keycode, trans.x());
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "obj_trans_y";
	flkey = new FlKeyDouble(keycode, trans.y());
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "obj_trans_z";
	flkey = new FlKeyDouble(keycode, trans.z());
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "aov";
	flkey = new FlKeyDouble(keycode, 42);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Initial view";
	t += frames;
	//right
	glbin_interpolator.Begin(t, frames);
	keycode.l2_name = "obj_trans_x";
	flkey = new FlKeyDouble(keycode, -trans.x());
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "obj_trans_y";
	flkey = new FlKeyDouble(keycode, -trans.y());
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "obj_trans_z";
	flkey = new FlKeyDouble(keycode, -trans.z());
	glbin_interpolator.AddKey(flkey);
	keycode.l2_name = "aov";
	flkey = new FlKeyDouble(keycode, 42);
	glbin_interpolator.AddKey(flkey);
	glbin_interpolator.End();
	kg = glbin_interpolator.GetKeyGroupFromTime(t);
	if (kg) kg->desc = L"Camera track horizontally";

	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
}