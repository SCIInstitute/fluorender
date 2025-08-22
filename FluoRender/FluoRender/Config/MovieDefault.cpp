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

#include <MovieDefault.h>
#include <Names.h>
#include <Global.h>
#include <MovieMaker.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

MovieDefault::MovieDefault()
{
	m_selected_page = 0;
	m_view_idx = 0;
	m_slider_style = false;

	m_keyframe_enable = false;
	m_rotate = true;
	m_rot_axis = 1;
	m_rot_deg = 360;
	m_interpolation = 0;
	m_seq_mode = 0;

	m_full_frame_num = 360;
	m_movie_len = 12;
	m_fps = 30;
	m_clip_start_frame = 0;
	m_clip_end_frame = 360;
	m_cur_frame = 0;
	m_cur_time = 0;

	m_crop = false;
	m_crop_x = 0;
	m_crop_y = 0;
	m_crop_w = 0;
	m_crop_h = 0;

	m_sb_pos = 3;
	m_sb_x = 5;
	m_sb_y = 5;

	m_key_duration = 30;

	m_cam_lock = false;
	m_cam_lock_type = 1;
}

MovieDefault::~MovieDefault()
{

}

void MovieDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	if (f->Exists("/movie default"))
		f->SetPath("/movie default");

	f->Read("view idx", &m_view_idx, 0);
	f->Read("slider style", &m_slider_style, false);

	f->Read("keyframe enable", &m_slider_style, false);
	f->Read("rotate", &m_rotate, true);
	f->Read("rot axis", &m_rot_axis, 1);
	f->Read("rot deg", &m_rot_deg, 360);
	f->Read("rot int type", &m_interpolation, 0);
	f->Read("seq mode", &m_seq_mode, 0);

	f->Read("full frame num", &m_full_frame_num, 360);
	f->Read("movie len", &m_movie_len, 12.0);
	f->Read("fps", &m_fps, 30.0);
	f->Read("clip start frame", &m_clip_start_frame, 0);
	f->Read("clip end frame", &m_clip_end_frame, 360);
	f->Read("cur frame", &m_cur_frame, 0);
	f->Read("cur time", &m_cur_time, 0.0);

	f->Read("crop", &m_crop, false);
	f->Read("crop x", &m_crop_x, 0);
	f->Read("crop y", &m_crop_y, 0);
	f->Read("crop w", &m_crop_w, 0);
	f->Read("crop h", &m_crop_h, 0);
	f->Read("scalebar pos", &m_sb_pos, 3);
	f->Read("scalebar x", &m_sb_x, 5);
	f->Read("scalebar y", &m_sb_y, 5);

	f->Read("key duration", &m_key_duration, 30.0);
	f->Read("cam lock", &m_cam_lock, false);
	f->Read("cam lock type", &m_cam_lock_type, 1);
}

void MovieDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/movie default");

	f->Write("view idx", m_view_idx);
	f->Write("slider style", m_slider_style);

	f->Write("keyframe enable", m_slider_style);
	f->Write("rotate", m_rotate);
	f->Write("rot axis", m_rot_axis);
	f->Write("rot deg", m_rot_deg);
	f->Write("rot int type", m_interpolation);
	f->Write("seq mode", m_seq_mode);

	f->Write("full frame num", m_full_frame_num);
	f->Write("movie len", m_movie_len);
	f->Write("fps", m_fps);
	f->Write("clip start frame", m_clip_start_frame);
	f->Write("clip end frame", m_clip_end_frame);
	f->Write("cur frame", m_cur_frame);
	f->Write("cur time", m_cur_time);

	f->Write("crop", m_crop);
	f->Write("crop x", m_crop_x);
	f->Write("crop y", m_crop_y);
	f->Write("crop w", m_crop_w);
	f->Write("crop h", m_crop_h);
	f->Write("scalebar pos", m_sb_pos);
	f->Write("scalebar x", m_sb_x);
	f->Write("scalebar y", m_sb_y);

	f->Write("key duration", m_key_duration);
	f->Write("cam lock", m_cam_lock);
	f->Write("cam lock type", m_cam_lock_type);
}

void MovieDefault::Set(MovieMaker* mm)
{
	if (!mm)
		return;

	m_keyframe_enable = mm->GetKeyframeEnable();
	m_rotate = mm->GetRotateEnable();
	m_rot_axis = mm->GetRotateAxis();
	m_rot_deg = mm->GetRotateDeg();
	m_interpolation = mm->GetInterpolation();
	m_seq_mode = mm->GetSeqMode();

	m_full_frame_num = mm->GetFullFrameNum();
	m_movie_len = mm->GetMovieLength();
	m_fps = mm->GetFps();
	m_clip_start_frame = mm->GetClipStartFrame();
	m_clip_end_frame = mm->GetClipEndFrame();
	m_cur_frame = mm->GetCurrentFrame();
	m_cur_time = mm->GetCurrentTime();

	m_crop = mm->GetCropEnable();
	m_crop_x = mm->GetCropX();
	m_crop_y = mm->GetCropY();
	m_crop_w = mm->GetCropW();
	m_crop_h = mm->GetCropH();
	m_sb_pos = mm->GetScalebarPos();
	m_sb_x = mm->GetScalebarX();
	m_sb_y = mm->GetScalebarY();

	m_key_duration = mm->GetKeyDuration();
	m_cam_lock = mm->GetCamLock();
	m_cam_lock_type = mm->GetCamLockType();
}

void MovieDefault::Apply(MovieMaker* mm)
{
	if (!mm)
		return;

	mm->SetKeyframeEnable(m_keyframe_enable, true);
	mm->SetRotateEnable(m_rotate);
	mm->SetRotateAxis(m_rot_axis);
	mm->SetRotateDeg(m_rot_deg);
	mm->SetInterpolation(m_interpolation);
	//mm->SetSeqMode(m_seq_mode);

	//mm->SetFullFrameNum(m_full_frame_num);
	//mm->SetMovieLength(m_movie_len);
	mm->SetFps(m_fps);
	//mm->SetClipStartEndFrames(m_clip_start_frame, m_clip_end_frame);
	//mm->SetCurrentFrame(m_cur_frame);
	//mm->SetCurrentTime(m_cur_time);

	mm->SetCropEnable(m_crop);
	mm->SetCropX(m_crop_x);
	mm->SetCropY(m_crop_y);
	mm->SetCropW(m_crop_w);
	mm->SetCropH(m_crop_h);
	mm->SetScalebarPos(m_sb_pos);
	mm->SetScalebarDist(m_sb_x, m_sb_y);

	mm->SetKeyDuration(m_key_duration);
	mm->SetCamLock(m_cam_lock);
	mm->SetCamLockType(m_cam_lock_type);
}

