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

#include <BrushDefault.h>
#include <Names.h>

BrushDefault::BrushDefault()
{
	m_paint_hist_depth = 0;

}

BrushDefault::~BrushDefault()
{

}

void BrushDefault::ReadDefault(wxFileConfig& f)
{
	double dval;
	int ival;
	bool bval;

	if (f.Exists("/brush default"))
		f.SetPath("/brush default");

	double val;
	int ival;
	bool bval;

	//brush properties
	if (fconfig.Read("brush_ini_thresh", &val))
		m_ini_thresh = val;
	if (fconfig.Read("brush_gm_falloff", &val))
		m_gm_falloff = val;
	if (fconfig.Read("brush_scl_falloff", &val))
		m_scl_falloff = val;
	if (fconfig.Read("brush_scl_translate", &val))
		m_scl_translate = val;
	//	m_calculator.SetThreshold(val);
	//auto thresh
	if (fconfig.Read("auto_thresh", &bval))
		m_estimate_threshold = bval;
	//edge detect
	if (fconfig.Read("edge_detect", &bval))
		m_edge_detect = bval;
	//hidden removal
	if (fconfig.Read("hidden_removal", &bval))
		m_hidden_removal = bval;
	//select group
	if (fconfig.Read("select_group", &bval))
		m_select_multi = bval ? 1 : 0;
	//brick accuracy
	if (fconfig.Read("accurate_bricks", &bval))
		m_update_order = bval;
	//2d influence
	if (fconfig.Read("brush_2dinfl", &val))
		m_w2d = val;
	//size 1
	if (fconfig.Read("brush_size1", &val) && val > 0.0)
		m_brush_radius1 = val;
	//size 2 link
	if (fconfig.Read("use_brush_size2", &bval))
		m_use_brush_radius2 = bval;
	//size 2
	if (fconfig.Read("brush_size2", &val) && val > 0.0)
		m_brush_radius2 = val;
	//radius settings for individual brush types
	if (fconfig.Exists("/radius_settings"))
	{
		fconfig.SetPath("/radius_settings");
		int brush_num = fconfig.Read("num", 0l);
		if (m_brush_radius_sets.size() != brush_num)
			m_brush_radius_sets.resize(brush_num);
		wxString str;
		for (int i = 0; i < brush_num; ++i)
		{
			str = wxString::Format("/radius_settings/%d", i);
			if (!fconfig.Exists(str))
				continue;
			fconfig.SetPath(str);
			//type
			fconfig.Read("type", &(m_brush_radius_sets[i].type));
			//radius 1
			fconfig.Read("radius1", &(m_brush_radius_sets[i].radius1));
			//radius 2
			fconfig.Read("radius2", &(m_brush_radius_sets[i].radius2));
			//use radius 2
			fconfig.Read("use_radius2", &(m_brush_radius_sets[i].use_radius2));
		}
		fconfig.SetPath("/");
	}
	if (m_brush_radius_sets.size() == 0)
	{
		BrushRadiusSet radius_set;
		//select brush
		radius_set.type = 2;
		radius_set.radius1 = 10;
		radius_set.radius2 = 30;
		radius_set.use_radius2 = true;
		m_brush_radius_sets.push_back(radius_set);
		//erase
		radius_set.type = 3;
		m_brush_radius_sets.push_back(radius_set);
		//diffuse brush
		radius_set.type = 4;
		m_brush_radius_sets.push_back(radius_set);
		//solid brush
		radius_set.type = 8;
		radius_set.use_radius2 = false;
		m_brush_radius_sets.push_back(radius_set);
	}
	m_brush_sets_index = 0;
	//iterations
	if (fconfig.Read("brush_iters", &ival))
	{
		switch (ival)
		{
		case 1:
			m_iter_num = BRUSH_TOOL_ITER_WEAK;
			break;
		case 2:
			m_iter_num = BRUSH_TOOL_ITER_NORMAL;
			break;
		case 3:
			m_iter_num = BRUSH_TOOL_ITER_STRONG;
			break;
		}
	}
	//brush size relation
	if (fconfig.Read("brush_size_data", &bval))
		m_brush_size_data = bval;
}

void BrushDefault::SaveDefault(wxFileConfig& f)
{

	//brush properties
	fconfig.Write("brush_ini_thresh", m_ini_thresh);
	fconfig.Write("brush_gm_falloff", m_gm_falloff);
	fconfig.Write("brush_scl_falloff", m_scl_falloff);
	fconfig.Write("brush_scl_translate", m_scl_translate);
	//auto thresh
	fconfig.Write("auto_thresh", m_estimate_threshold);
	//edge detect
	fconfig.Write("edge_detect", m_edge_detect);
	//hidden removal
	fconfig.Write("hidden_removal", m_hidden_removal);
	//select group
	fconfig.Write("select_group", m_select_multi == 1);
	//brick acccuracy
	fconfig.Write("accurate_bricks", m_update_order);
	//2d influence
	fconfig.Write("brush_2dinfl", m_w2d);
	//size 1
	fconfig.Write("brush_size1", m_brush_radius1);
	//size2 link
	fconfig.Write("use_brush_size2", m_use_brush_radius2);
	//size 2
	fconfig.Write("brush_size2", m_brush_radius2);
	//radius settings for individual brush types
	fconfig.SetPath("/radius_settings");
	int brush_num = m_brush_radius_sets.size();
	fconfig.Write("num", brush_num);
	for (int i = 0; i < brush_num; ++i)
	{
		BrushRadiusSet radius_set = m_brush_radius_sets[i];
		str = wxString::Format("/radius_settings/%d", i);
		fconfig.SetPath(str);
		//type
		fconfig.Write("type", radius_set.type);
		//radius 1
		fconfig.Write("radius1", radius_set.radius1);
		//radius 2
		fconfig.Write("radius2", radius_set.radius2);
		//use radius 2
		fconfig.Write("use_radius2", radius_set.use_radius2);
	}
	fconfig.SetPath("/");
	//iterations
	fconfig.Write("brush_iters", m_iter_num);
	//brush size relation
	fconfig.Write("brush_size_data", m_brush_size_data);
}
