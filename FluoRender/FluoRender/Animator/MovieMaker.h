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
#ifndef _MOVIEMAKER_H_
#define _MOVIEMAKER_H_

#include <wx/string.h>
#include <vector>
#include <string>

namespace fluo
{
	class StopWatch;
}
class MainFrame;
class RenderCanvas;
class MovieMaker
{
public:
	MovieMaker();
	~MovieMaker();

	//timer for playback.
	bool Action();
	//play
	bool IsRunning()
	{
		return m_running;
	}
	bool IsReverse()
	{
		return m_reverse;
	}
	bool IsLoop()
	{
		return m_loop;
	}
	bool IsPaused()
	{
		return m_cur_frame != 0;
	}
	void Play(bool back);
	void Start();
	void Stop();
	void Resume();
	void Hold();
	void Rewind();
	void Forward();
	void Reset();
	void PlaySave();

	//set the renderview and progress bars/text
	void SetRendering(bool rewind);
	//write frames to file
	void WriteFrameToFile();

	//settings
	void SetMainFrame(MainFrame* frame);
	void SetView(RenderCanvas* view);
	RenderCanvas* GetView();
	int GetViewIndex();
	void SetFileName(const wxString& filename) { m_filename = filename; }
	void SetLoop(bool val) { m_loop = val; }
	bool GetLoop() { return m_loop; }

	void SetKeyframeEnable(bool val);
	bool GetKeyframeEnable() { return m_keyframe_enable; }
	void SetRotateEnable(bool val);
	bool GetRotateEnable() { return m_rotate; }
	void SetRotateAxis(int val);
	int GetRotateAxis() { return m_rot_axis; }
	void SetRotateDeg(int val);
	int GetRotateDeg() { return m_rot_deg; }
	void SetRotIntType(int val)
	{
		if (val > -1 && val < 2)
			m_rot_int_type = val;
	}
	int GetRotIntType() { return m_rot_int_type; }
	void SetSeqMode(int val);
	int GetSeqMode() { return m_seq_mode; }
	void SetFullFrameNum(int val);
	int GetFullFrameNum()
	{
		return m_full_frame_num;
	}
	void SetMovieLength(double val)
	{
		m_movie_len = val;
		if (val > 0)
			m_fps = (m_clip_frame_num - 1) / m_movie_len;
	}
	double GetMovieLength() { return m_movie_len; }
	void SetFps(double val)
	{
		m_fps = val;
		if (val > 0)
			m_movie_len = (m_clip_frame_num - 1) / m_fps;
	}
	double GetFps() { return m_fps; }
	void SetClipStartEndFrames(int val1, int val2);
	void SetClipStartFrame(int val);
	int GetClipStartFrame() { return m_clip_start_frame; }
	void SetClipEndFrame(int val);
	int GetClipEndFrame() { return m_clip_end_frame; }
	void SetCurrentFrame(int val);
	int GetCurrentFrame() { return m_cur_frame; }
	void SetCurrentTime(double val);
	double GetCurProg()
	{
		if (m_keyframe_enable)
			return m_cur_frame;
		else
			return double(m_cur_frame) / (m_full_frame_num - 1);
	}
	double GetCurrentTime() { return m_cur_time; }
	int GetScrollThumbSize() { return m_scroll_thumb_size; }

	//crop
	void SetCropEnable(bool val);
	bool GetCropEnable() { return m_crop; }
	void SetCropValues(int, int, int, int);
	void SetCropX(int val);
	int GetCropX() { return m_crop_x; }
	void SetCropY(int val);
	int GetCropY() { return m_crop_y; }
	void SetCropW(int val);
	int GetCropW() { return m_crop_w; }
	void SetCropH(int val);
	int GetCropH() { return m_crop_h; }
	void SetCamLock(bool val) { m_cam_lock = val; }
	bool GetCamLock() { return m_cam_lock; }
	void SetCamLockType(int val)
	{
		if (val > 0 && val < 5)
			m_cam_lock_type = val;
		else
			m_cam_lock_type = 0;
	}
	int GetCamLockType() { return m_cam_lock_type; }

	//autokey functions
	void MakeKeys(int type);
	std::vector<std::string> GetAutoKeyTypes();
	void AutoKeyChanComb(int comb);
	bool MoveOne(std::vector<bool>& chan_mask, int lv);
	bool GetMask(std::vector<bool>& chan_mask);

private:
	MainFrame* m_frame;
	RenderCanvas* m_view;
	bool m_running;
	int m_last_frame;//last frame nunmber to save
	double m_starting_rot;//starting degree of rotation
	bool m_record;
	bool m_delayed_stop;
	bool m_timer_hold;//for temporary hold
	bool m_reverse;//play backward
	bool m_loop;//rewind after finish and restart play
	//save
	wxString m_filename;
	wxString filetype_;
	int m_scroll_thumb_size;

	//settings
	bool m_keyframe_enable;//enable keyframe animation
	bool m_rotate;//enable roatation animation
	int m_rot_axis;	//0-x;1-y;2-z
	int m_rot_deg;
	int m_rot_int_type;//0-linear; 1-smooth
	int m_seq_mode;//0:none; 1:4d; 2:bat

	//movie properties
	int m_full_frame_num;//the full length movie always starts from 0 and ends at f-1
	int m_clip_frame_num;
	double m_movie_len;//length in sec
	double m_fps;
	int m_clip_start_frame;
	int m_clip_end_frame;
	int m_cur_frame;
	double m_cur_time;//time in sec
	double m_cur_prog;//normalized time between 0 and 1

	//cropping
	bool m_crop;//enable cropping
	int m_crop_x;
	int m_crop_y;
	int m_crop_w;
	int m_crop_h;

	//cam lock
	bool m_cam_lock;
	int m_cam_lock_type;//0-not used;1-image center;2-click view;3-ruler;4-selection

private:
	fluo::StopWatch* get_stopwatch();
};

#endif//_MOVIEMAKER_H_
