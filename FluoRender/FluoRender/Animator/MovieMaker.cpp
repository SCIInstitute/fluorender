/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "MovieMaker.h"
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <StopWatch.hpp>
#include <StopWatchFactory.hpp>

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
	m_loop(false)
{
	m_keyframe_enable = false;
	m_rotate = true;
	m_rot_axis = 1;
	m_rot_deg = 360;
	m_rot_int_type = 0;
	m_time_seq = false;
	m_seq_mode = 0;

	m_frame_num = 361;
	m_movie_len = 12;
	m_fps = 30;
	m_start_frame = 0;
	m_end_frame = 360;
	m_cur_frame = 0;
	m_cur_time = 0;

	m_crop = false;
	m_crop_x = 0;
	m_crop_y = 0;
	m_crop_w = 0;
	m_crop_h = 0;

	m_cam_lock = false;
	m_cam_lock_type = 0;

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
	if (!m_keyframe_enable)
	{
		double rval[3];
		m_view->GetRotations(rval[0], rval[1], rval[2]);
		m_starting_rot = rval[m_rot_axis];
		while (m_starting_rot > 360.) m_starting_rot -= 360.;
		while (m_starting_rot < -360.) m_starting_rot += 360.;
		if (360. - std::abs(m_starting_rot) < 0.001)
			m_starting_rot = 0.;
	}

	get_stopwatch()->start();
	m_running = true;
}

void MovieMaker::Stop()
{
	get_stopwatch()->stop();
	glbin.get_video_encoder().close();
	m_record = false;
	flvr::TextureRenderer::maximize_uptime_ = false;
	m_reverse = false;
	m_running = false;
}

void MovieMaker::Resume()
{
	if (m_timer_hold)
	{
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
	SetCurrentFrame(m_start_frame);

	//m_view->SetParams(0.);
	//m_cur_frame_text->ChangeValue(wxString::Format("%d", m_cur_frame));
	//SetProgress(0.);
	SetRendering(true);
}

void MovieMaker::Reset()
{
	if (!m_view)
		return;
	m_view->m_tseq_cur_num =
		m_view->m_tseq_prv_num = 0;
	SetCurrentFrame(m_start_frame);
	m_last_frame = -1;
	SetRendering(false);
}

void MovieMaker::PlaySave()
{
	if (!m_frame || !m_view)
		return;

	Rewind();

	filetype_ = m_filename.SubString(m_filename.Len() - 4,
		m_filename.Len() - 1);
	if (filetype_.IsSameAs(wxString(".mov")))
	{
		if (!m_crop)
		{
			m_crop_x = 0;
			m_crop_y = 0;
			m_crop_w = m_view->GetGLSize().x;
			m_crop_h = m_view->GetGLSize().y;
		}
		else if (RenderCanvas::GetEnlarge())
		{
			double scale = RenderCanvas::GetEnlargeScale();
			m_crop_w *= scale;
			m_crop_h *= scale;
		}

		glbin.get_video_encoder().open(m_filename.ToStdString(), m_crop_w, m_crop_h, m_fps,
			glbin_settings.m_mov_bitrate * 1e6);
	}
	m_filename = m_filename.SubString(0, m_filename.Len() - 5);
	m_record = true;
	if (glbin_settings.m_prj_save)
	{
		wxString new_folder;
		new_folder = m_filename + "_project";
		MkDirW(new_folder.ToStdWstring());
		wstring name = m_filename.ToStdWstring();
		name = GET_NAME(name);
		wxString prop_file = new_folder + GETSLASH()
			+ name + "_project.vrp";
		bool inc = wxFileExists(prop_file) && glbin_settings.m_prj_save_inc;
		m_frame->SaveProject(prop_file, inc);
	}

	RenderCanvas::SetKeepEnlarge(true);
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
			m_view->Set4DSeqFrame(m_cur_frame, m_start_frame, m_end_frame, rewind);
		}
		else if (m_seq_mode == 2)
		{
			m_view->Set3DBatFrame(m_cur_frame, m_start_frame, m_end_frame, rewind);
		}

		//rotate animation
		if (m_rotate)
		{
			double rval[3];
			double val;
			m_view->GetRotations(rval[0], rval[1], rval[2]);
			val = rval[m_rot_axis];
			if (m_rot_int_type == 0)
				val = m_starting_rot + t * m_rot_deg;
			else if (m_rot_int_type == 1)
				val = m_starting_rot +
				(-2.0 * t * t * t + 3.0 * t * t) * m_rot_deg;
			rval[m_rot_axis] = val;
			m_view->SetRotations(rval[0], rval[1], rval[2], true);
		}
	}
	m_view->SetInteractive(false);
	//m_view->RefreshGL(39);
}

void MovieMaker::WriteFrameToFile()
{
	if (!m_view)
		return;

	wxString s_length = wxString::Format("%d", m_frame_num);
	int length = s_length.Length();
	wxString format = wxString::Format("_%%0%dd", length);
	wxString outputfilename = wxString::Format("%s" + format + "%s", m_filename,
		m_last_frame, ".tif");

	//capture
	bool bmov = filetype_.IsSameAs(".mov");
	int chann = glbin_settings.m_save_alpha ? 4 : 3;
	bool fp32 = bmov ? false : glbin_settings.m_save_float;
	float dpi = glbin_settings.m_dpi;
	int x, y, w, h;
	void* image = 0;
	m_view->ReadPixels(chann, fp32, x, y, w, h, &image);

	string str_fn = outputfilename.ToStdString();
	if (bmov)
	{
		//flip vertically 
		unsigned char* flip = new unsigned char[w * h * 3];
		for (size_t yy = 0; yy < (size_t)h; yy++)
			for (size_t xx = 0; xx < (size_t)w; xx++)
				memcpy(flip + 3 * (w * yy + xx), (unsigned char*)image + chann * (w * (h - yy - 1) + xx), 3);
		bool worked = glbin.get_video_encoder().set_frame_rgb_data(flip);
		worked = glbin.get_video_encoder().write_video_frame(m_last_frame);
		if (flip)
			delete[]flip;
		if (image)
			delete[]image;
	}
	else
	{
		TIFF* out = TIFFOpen(str_fn.c_str(), "wb");
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

void MovieMaker::MakeKeys(int type)
{
	switch (type)
	{
	case 0:
		AutoKeyChanComb(1);
		break;
	case 1:
		AutoKeyChanComb(2);
		break;
	case 2:
		AutoKeyChanComb(3);
		break;
	}
}

std::vector<std::string> MovieMaker::GetAutoKeyTypes()
{
	std::vector<std::string> result;
	result.push_back("Channel combination nC1");
	result.push_back("Channel combination nC2");
	result.push_back("Channel combination nC3");
	return result;
}

void MovieMaker::AutoKeyChanComb(int comb)
{
	if (!m_frame)
		return;
	if (!m_view)
	{
		if (m_frame->GetView(0))
			m_view = m_frame->GetView(0);
		else
			return;
	}

	FlKeyCode keycode;
	FlKeyBoolean* flkeyB = 0;

	double t = glbin_interpolator.GetLastT();
	t = t < 0.0 ? 0.0 : t;
	if (t > 0.0) t += m_movie_len;

	int i;
	int numChan = m_view->GetAllVolumeNum();
	vector<bool> chan_mask;
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
		glbin_interpolator.Begin(t, m_movie_len);

		//for all volumes
		for (i = 0; i < m_view->GetAllVolumeNum(); i++)
		{
			VolumeData* vd = m_view->GetAllVolumeData(i);
			keycode.l0 = 1;
			keycode.l0_name = m_view->GetName();
			keycode.l1 = 2;
			keycode.l1_name = vd->GetName();
			//display only
			keycode.l2 = 0;
			keycode.l2_name = "display";
			flkeyB = new FlKeyBoolean(keycode, chan_mask[i]);
			glbin_interpolator.AddKey(flkeyB);
		}

		glbin_interpolator.End();
		t += m_movie_len;
	} while (GetMask(chan_mask));

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

void MovieMaker::SetMainFrame(MainFrame* frame)
{
	m_frame = frame;
}

void MovieMaker::SetView(RenderCanvas* view)
{
	m_view = view;
}

RenderCanvas* MovieMaker::GetView()
{
	return m_view;
}

int MovieMaker::GetViewIndex()
{
	if (!m_view || !m_frame)
		return -1;

	for (int i = 0; i < m_frame->GetViewNum(); ++i)
	{
		if (m_view == m_frame->GetView(i))
			return i;
	}

	return -1;
}

void MovieMaker::SetTimeSeqEnable(bool val)
{
	m_time_seq = val;
	if (m_time_seq)
	{
		if (!m_seq_mode)
			m_seq_mode = 1;

		if (m_seq_mode == 1)
		{
			if (m_view)
				m_view->Get4DSeqRange(m_start_frame, m_end_frame);
		}
		else if (m_seq_mode == 2)
		{
			if (m_view)
				m_view->Get3DBatRange(m_start_frame, m_end_frame);
		}
	}
	else
	{
		SetStartFrame(0);
		SetEndFrame(m_rot_deg);
	}
}

void MovieMaker::SetCropEnable(bool val)
{
	m_crop = val;
	if (m_view)
	{
		if (val)
		{
			m_view->CalcFrame();
			m_view->GetFrame(m_crop_x, m_crop_y, m_crop_w, m_crop_h);
			m_crop_x = std::round(m_crop_x + m_crop_w / 2.0);
			m_crop_y = std::round(m_crop_y + m_crop_h / 2.0);
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
	if (m_view)
		m_view->SetFrame(std::round(m_crop_x - m_crop_w / 2.0),
			std::round(m_crop_y - m_crop_h / 2.0), m_crop_w, m_crop_h);
}

void MovieMaker::SetCropX(int val)
{
	m_crop_x = val;
	if (m_view)
		m_view->SetFrame(std::round(m_crop_x - m_crop_w / 2.0),
			std::round(m_crop_y - m_crop_h / 2.0), m_crop_w, m_crop_h);
}

void MovieMaker::SetCropY(int val)
{
	m_crop_y = val;
	if (m_view)
		m_view->SetFrame(std::round(m_crop_x - m_crop_w / 2.0),
			std::round(m_crop_y - m_crop_h / 2.0), m_crop_w, m_crop_h);
}

void MovieMaker::SetCropW(int val)
{
	m_crop_w = val;
	if (m_view)
		m_view->SetFrame(std::round(m_crop_x - m_crop_w / 2.0),
			std::round(m_crop_y - m_crop_h / 2.0), m_crop_w, m_crop_h);
}

void MovieMaker::SetCropH(int val)
{
	m_crop_h = val;
	if (m_view)
		m_view->SetFrame(std::round(m_crop_x - m_crop_w / 2.0),
			std::round(m_crop_y - m_crop_h / 2.0), m_crop_w, m_crop_h);
}

bool MovieMaker::OnTimer()
{
	if (!m_running)
		return false;
	if (!get_stopwatch()->check())
		return false;

	//get all of the progress info
	if (m_delayed_stop)
	{
		if (m_record)
			WriteFrameToFile();
		m_delayed_stop = false;
		Stop();
		RenderCanvas::SetKeepEnlarge(false);
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
	if (!m_loop)
	{
		if (m_reverse)
		{
			if (m_cur_frame == m_start_frame)
				m_delayed_stop = true;
		}
		else
		{
			if (m_cur_frame == m_end_frame)
				m_delayed_stop = true;
		}
	}

	return true;
}

fluo::StopWatch* MovieMaker::get_stopwatch()
{
	fluo::StopWatch* result = glbin_swhf->findFirst(gstMovStopWatch);
	if (!result)
	{
		result = glbin_swhf->build();
		result->setName(gstMovStopWatch);
	}
	return result;
}

