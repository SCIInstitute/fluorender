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
#ifndef _STATE_H_
#define _STATE_H_

#include <Value.hpp>

class BaseState
{
public:
	BaseState()
	{ }

	bool m_refresh = false;
	bool m_key_ctrl = false;//ctrl

};

class MouseState : public BaseState
{
public:
	MouseState()
	{ }

	bool m_valid_focus_slider = false;
	bool m_reset_focus_slider = false;
	bool m_scroll_focus_slider = false;
	//mouse state
	bool m_mouse_left_down = false;
	bool m_mouse_left_up = false;
	bool m_mouse_left_is_down = false;
	bool m_mouse_right_down = false;
	bool m_mouse_right_up = false;
	bool m_mouse_right_is_down = false;
	bool m_mouse_middle_down = false;
	bool m_mouse_middle_up = false;
	bool m_mouse_middle_is_down = false;
	bool m_mouse_drag = false;
	int m_mouse_wheel_rotate = 0;
	int m_mouse_wheel_delta = 0;
	//key state
	bool m_key_alt = false;//alt
};

class IdleState : public BaseState
{
public:
	IdleState()
	{
	}

	bool m_request_more = false;
	bool m_movie_maker_render_canvas = false;
	bool m_erase_background = false;
	bool m_start_loop = false;
	bool m_set_focus = false;
	double m_benchmark_fps = 0;
	bool m_looking_glass_changed = false;
	bool m_mouse_over = false;
	bool m_exit_fullscreen = false;
	bool m_fullscreen = false;

	//focus update
	bool m_set_cur_focus = false;
	bool m_set_previous_focus = false;

	//mouse state
	bool m_mouse_left = false;
	//key state
	bool m_key_paint = false;//shift
	bool m_key_erase = false;//x
	bool m_key_diff = false;//z
	bool m_key_refresh = false;//f5
	bool m_key_main_mode = false;//v
	bool m_key_mask_mode = false;//b
	bool m_key_left = false;//left
	bool m_key_right = false;//right
	bool m_key_up = false;//up
	bool m_key_down = false;//down
	bool m_key_mov_forward = false;//d
	bool m_key_mov_backward = false;//a
	bool m_key_mov_play = false;//space
	bool m_key_clip_up = false;//s
	bool m_key_clip_down = false;//w
	bool m_key_cell_full = false;//f
	bool m_key_cell_link = false;//l
	bool m_key_cell_new_id = false;//n
	bool m_key_cell_clear = false;//c
	bool m_key_cell_include = false;//enter
	bool m_key_cell_exclude = false;//back slash
	bool m_key_save_mask = false;//m
	bool m_key_exit_fullscreen = false;//esc
	bool m_key_fullscreen = false;//f11
	bool m_key_brush_size_down = false;//[
	bool m_key_brush_size_up = false;//]
	bool m_key_ruler_relax = false;//r

	fluo::ValueCollection m_value_collection = {};
};

#endif//_STATE_H_