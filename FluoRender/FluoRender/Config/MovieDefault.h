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
#ifndef _MOVIEDEFAULT_H_
#define _MOVIEDEFAULT_H_

#include <Point.h>

class MovieMaker;
class MovieDefault
{
public:
	MovieDefault();
	~MovieDefault();

	void Read();
	void Save();
	void Set(MovieMaker* mm);
	void Apply(MovieMaker* mm);

public:
	//default values
	int m_selected_page;
	int m_view_idx;//index to current renderview
	bool m_slider_style;//0:normal, 1:jog

	bool m_keyframe_enable;//enable keyframe animation
	bool m_rotate;//enable roatation animation
	int m_rot_axis;	//0-x;1-y;2-z
	int m_rot_deg;
	int m_interpolation;//0-linear; 1-smooth
	int m_seq_mode;//0:none; 1:4d; 2:bat

	//movie properties
	int m_full_frame_num;
	double m_movie_len;//length in sec
	double m_fps;
	int m_clip_start_frame;
	int m_clip_end_frame;
	int m_cur_frame;
	double m_cur_time;//time in sec

	//cropping
	bool m_crop;//enable cropping
	int m_crop_x;
	int m_crop_y;
	int m_crop_w;
	int m_crop_h;
	//scale bar
	int m_sb_pos;
	int m_sb_x;
	int m_sb_y;

	//keys
	double m_key_duration;
	//cam lock
	bool m_cam_lock;
	int m_cam_lock_type;//0-not used;1-image center;2-click view;3-ruler;4-selection
};
#endif
