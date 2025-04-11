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
	virtual ~BaseState() = default;
};

class MouseState : public BaseState
{
public:
	MouseState()
	{ }
};

class IdleState : public BaseState
{
public:
	IdleState()
	{
	}

	bool m_request_more = false;
	bool m_movie_maker_render_canvas = false;
	bool m_refresh = false;
	bool m_erase_background = false;
	bool m_start_loop = false;
	bool m_set_focus = false;
	double m_benchmark_fps = 0;
	bool m_looking_glass_changed = false;
	bool m_mouse_over = false;
	fluo::ValueCollection m_value_collection = {};
};

#endif//_STATE_H_