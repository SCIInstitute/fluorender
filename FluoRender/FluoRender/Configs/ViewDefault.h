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
#ifndef _VIEWDEFAULT_H_
#define _VIEWDEFAULT_H_

#include <Types/Color.h>
#include <Types/Vector.h>
#include <Types/Point.h>
#include <wx/fileconf.h>

class RenderCanvas;
class ViewDefault
{
public:
	ViewDefault();
	~ViewDefault();

	void Read(wxFileConfig& f);
	void Save(wxFileConfig& f);
	void Set(RenderCanvas* view);
	void Apply(RenderCanvas* view);

public:
	//default values
	int m_vol_method;
	fluo::Color m_bg_color;
	bool m_draw_camctr;
	double m_camctr_size;
	int m_draw_info;
	bool m_draw_legend;
	bool m_draw_colormap;
	bool m_mouse_focus;
	bool m_persp;
	double m_aov;
	bool m_free;
	fluo::Point m_center;
	fluo::Vector m_rot;
	bool m_rot_lock;
	bool m_rot_slider;
	bool m_pin_rot_center;
	int m_scale_mode;
	double m_scale_factor;
	bool m_use_fog;
	double m_fog_intensity;
};
#endif
