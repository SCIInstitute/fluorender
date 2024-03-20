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

#include <MovieDefault.h>
#include <Names.h>
#include <Animator/MovieMaker.h>

MovieDefault::MovieDefault()
{
	m_view_idx = 0;
	m_slider_style = false;

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
}

MovieDefault::~MovieDefault()
{

}

void MovieDefault::Read(wxFileConfig& f)
{
	wxString str;
	if (f.Exists("/movie default"))
		f.SetPath("/movie default");

	f.Read("view idx", &m_view_idx, 0);
	f.Read("slider style", &m_slider_style, false);

	f.Read("keyframe enable", &m_slider_style, false);
	f.Read("rotate", &m_rotate, true);
	f.Read("rot axis", &m_rot_axis, 1);
	f.Read("rot deg", &m_rot_deg, 360);
	f.Read("rot int type", &m_rot_int_type, 0);
	f.Read("time seq", &m_time_seq, false);
	f.Read("seq mode", &m_seq_mode, 0);

	f.Read("frame num", &m_frame_num, 361);
	f.Read("movie len", &m_movie_len, 12);
	f.Read("fps", &m_fps, 30);
	f.Read("start frame", &m_start_frame, 0);
	f.Read("end frame", &m_end_frame, 360);
	f.Read("cur frame", &m_cur_frame, 0);
	f.Read("cur time", &m_cur_time, 0);

	f.Read("crop", &m_crop, false);
	f.Read("crop x", &m_crop_x, 0);
	f.Read("crop y", &m_crop_y, 0);
	f.Read("crop w", &m_crop_w, 0);
	f.Read("crop h", &m_crop_h, 0);
}

void MovieDefault::Save(wxFileConfig& f)
{
	wxString str;
	f.SetPath("/movie default");

	f.Write("view idx", m_view_idx);
	f.Write("slider style", m_slider_style);

	f.Write("keyframe enable", m_slider_style);
	f.Write("rotate", m_rotate);
	f.Write("rot axis", m_rot_axis);
	f.Write("rot deg", m_rot_deg);
	f.Write("rot int type", m_rot_int_type);
	f.Write("time seq", m_time_seq);
	f.Write("seq mode", m_seq_mode);

	f.Write("frame num", m_frame_num);
	f.Write("movie len", m_movie_len);
	f.Write("fps", m_fps);
	f.Write("start frame", m_start_frame);
	f.Write("end frame", m_end_frame);
	f.Write("cur frame", m_cur_frame);
	f.Write("cur time", m_cur_time);

	f.Write("crop", m_crop);
	f.Write("crop x", m_crop_x);
	f.Write("crop y", m_crop_y);
	f.Write("crop w", m_crop_w);
	f.Write("crop h", m_crop_h);
}

void MovieDefault::Set(MovieMaker* mm)
{
	if (!mm)
		return;

	m_keyframe_enable = mm->GetKeyframeEnable();
	m_rotate = mm->GetRotateEnable();
	m_rot_axis = mm->GetRotateAxis();
	m_rot_deg = mm->GetRotateDeg();
	m_rot_int_type = mm->GetRotIntType();
	m_time_seq = mm->GetTimeSeqEnable();
	m_seq_mode = mm->GetSeqMode();

	m_frame_num = mm->GetFrameNum();
	m_movie_len = mm->GetMovieLength();
	m_fps = mm->GetFps();
	m_start_frame = mm->GetStartFrame();
	m_end_frame = mm->GetEndFrame();
	m_cur_frame = mm->GetCurrentFrame();
	m_cur_time = mm->GetCurrentTime();
}

void MovieDefault::Apply(MovieMaker* mm)
{
	if (!mm)
		return;

	mm->SetKeyframeEnable(m_keyframe_enable);
	mm->SetRotateEnable(m_rotate);
	mm->SetRotateAxis(m_rot_axis);
	mm->SetRotateDeg(m_rot_deg);
	mm->SetRotIntType(m_rot_int_type);
	mm->SetTimeSeqEnable(m_time_seq);
	mm->SetSeqMode(m_seq_mode);

	mm->SetFrameNum(m_frame_num);
	mm->SetMovieLength(m_movie_len);
	mm->SetFps(m_fps);
	mm->SetStartFrame(m_start_frame);
	mm->SetEndFrame(m_end_frame);
	mm->SetCurrentFrame(m_cur_frame);
	mm->SetCurrentTime(m_cur_time);
}

