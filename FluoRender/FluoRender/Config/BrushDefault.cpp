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
#include <Global.h>
#include <BrushDefault.h>
#include <Names.h>
#include <VolumeSelector.h>
#include <BaseTreeFile.h>
#include <TreeFileFactory.h>

BrushDefault::BrushDefault()
{
	m_paint_hist_depth = 1;

	m_iter_num = 10;

	m_ini_thresh = 0.0;
	m_estimate_threshold = false;
	m_gm_falloff = 1.0;
	m_scl_falloff = 0.0;
	m_scl_translate = 0.0;

	m_select_multi = false;
	m_edge_detect = false;
	m_hidden_removal = true;
	m_ortho = true;
	m_update_order = true;

	m_w2d = 0.0;
	m_brush_radius1 = 10;
	m_use_brush_radius2 = true;
	m_brush_radius2 = 30;

	m_brush_spacing = 0.1;
	m_brush_size_data = true;
}

BrushDefault::~BrushDefault()
{

}

void BrushDefault::Read()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	int ival;

	if (f->Exists("/brush default"))
		f->SetPath("/brush default");

	//history
	f->Read("hist depth", &m_paint_hist_depth, 1);
	//iterations
	f->Read("iter num", &m_iter_num, 10);
	//brush properties
	f->Read("ini thresh", &m_ini_thresh, 0.0);
	f->Read("auto thresh", &m_estimate_threshold, false);
	f->Read("gm falloff", &m_gm_falloff, 1.0);
	f->Read("scl falloff", &m_scl_falloff, 0.0);
	f->Read("scl translate", &m_scl_translate, 0.0);
	//select group
	f->Read("select group", &m_select_multi, false);
	//edge detect
	f->Read("edge detect", &m_edge_detect, false);
	//hidden removal
	f->Read("hidden removal", &m_hidden_removal, true);
	f->Read("ortho", &m_ortho, true);
	//brick accuracy
	f->Read("accurate bricks", &m_update_order, true);

	//2d influence
	f->Read("2d infl", &m_w2d, 0.0);
	//size 1
	f->Read("size1", &m_brush_radius1, 10.0);
	//size 2 link
	f->Read("use_size2", &m_use_brush_radius2, true);
	//size 2
	f->Read("size2", &m_brush_radius2, 30.0);
	//radius settings for individual brush types
	if (f->Exists("radius_settings"))
	{
		f->SetPath("radius_settings");
		int brush_num = 0;
		f->Read("num", &brush_num);
		if (m_brush_radius_sets.size() != brush_num)
			m_brush_radius_sets.resize(brush_num);
		for (int i = 0; i < brush_num; ++i)
		{
			std::string str = std::to_string(i);
			if (!f->Exists(str))
				continue;
			f->SetPath(str);
			//type
			if (f->Read("type", &ival))
				m_brush_radius_sets[i].type = static_cast<flrd::SelectMode>(ival);
			//radius 1
			f->Read("radius1", &(m_brush_radius_sets[i].radius1));
			//radius 2
			f->Read("radius2", &(m_brush_radius_sets[i].radius2));
			//use radius 2
			f->Read("use_radius2", &(m_brush_radius_sets[i].use_radius2));
			//iter num
			f->Read("iter num", &(m_brush_radius_sets[i].iter_num));
			//f->SetPath("/brush default/radius_settings");
			f->SetPath("..");
		}
	}
	if (m_brush_radius_sets.size() == 0)
	{
		flrd::BrushRadiusSet radius_set;
		//select brush
		radius_set.type = flrd::SelectMode::Append;
		radius_set.radius1 = 10;
		radius_set.radius2 = 30;
		radius_set.use_radius2 = true;
		radius_set.iter_num = m_iter_num;
		m_brush_radius_sets.push_back(radius_set);
		//erase
		radius_set.type = flrd::SelectMode::Eraser;
		m_brush_radius_sets.push_back(radius_set);
		//diffuse brush
		radius_set.type = flrd::SelectMode::Diffuse;
		m_brush_radius_sets.push_back(radius_set);
		//solid brush
		radius_set.type = flrd::SelectMode::Solid;
		radius_set.use_radius2 = false;
		m_brush_radius_sets.push_back(radius_set);
		//grow
		radius_set.type = flrd::SelectMode::Grow;
		radius_set.radius1 = 10;
		radius_set.radius2 = 10;
		radius_set.use_radius2 = false;
		radius_set.iter_num = 3;
		m_brush_radius_sets.push_back(radius_set);
	}
	if (f->Exists("/brush default"))
		f->SetPath("/brush default");
	//spacing
	f->Read("spacing", &m_brush_spacing, 0.1);
	//brush size relation
	f->Read("size_data", &m_brush_size_data, true);
}

void BrushDefault::Save()
{
	std::shared_ptr<BaseTreeFile> f =
		glbin_tree_file_factory.getTreeFile(gstConfigFile);
	if (!f)
		return;

	f->SetPath("/brush default");

	//history
	f->Write("hist depth", m_paint_hist_depth);
	//iterations
	f->Write("iter num", m_iter_num);
	//brush properties
	f->Write("ini thresh", m_ini_thresh);
	f->Write("auto thresh", m_estimate_threshold);
	f->Write("gm falloff", m_gm_falloff);
	f->Write("scl falloff", m_scl_falloff);
	f->Write("scl translate", m_scl_translate);
	//select group
	f->Write("select group", m_select_multi);
	//edge detect
	f->Write("edge detect", m_edge_detect);
	//hidden removal
	f->Write("hidden removal", m_hidden_removal);
	f->Write("ortho", m_ortho);
	//brick accuracy
	f->Write("accurate bricks", m_update_order);

	//2d influence
	f->Write("2d infl", m_w2d);
	//size 1
	f->Write("size1", m_brush_radius1);
	//size 2 link
	f->Write("use_size2", m_use_brush_radius2);
	//size 2
	f->Write("size2", m_brush_radius2);
	//radius settings for individual brush types
	f->SetPath("/brush default/radius_settings");
	size_t brush_num = m_brush_radius_sets.size();
	f->Write("num", brush_num);
	for (size_t i = 0; i < brush_num; ++i)
	{
		flrd::BrushRadiusSet radius_set = m_brush_radius_sets[i];
		std::string str = "/brush default/radius_settings/" + std::to_string(i);
		f->SetPath(str);
		//type
		f->Write("type", static_cast<int>(radius_set.type));
		//radius 1
		f->Write("radius1", radius_set.radius1);
		//radius 2
		f->Write("radius2", radius_set.radius2);
		//use radius 2
		f->Write("use_radius2", radius_set.use_radius2);
		//iter
		f->Write("iter_num", radius_set.iter_num);
	}
	f->SetPath("/brush default");
	//spacing
	f->Write("spacing", m_brush_spacing);
	//brush size relation
	f->Write("size_data", m_brush_size_data);
}

void BrushDefault::Set(flrd::VolumeSelector* vs)
{
	if (!vs)
		return;

	m_iter_num = vs->GetBrushIteration();

	m_ini_thresh = vs->GetBrushIniThresh();
	m_estimate_threshold = vs->GetEstimateThreshold();
	m_gm_falloff = vs->GetBrushGmFalloff();
	m_scl_falloff = vs->GetBrushSclFalloff();
	m_scl_translate = vs->GetBrushSclTranslate();

	m_select_multi = vs->GetSelectGroup();
	m_edge_detect = vs->GetEdgeDetect();
	m_hidden_removal = vs->GetHiddenRemoval();
	m_ortho = vs->GetOrthographic();
	m_update_order = vs->GetUpdateOrder();

	m_w2d = vs->GetW2d();
	m_brush_radius1 = vs->GetBrushSize1();
	m_brush_radius2 = vs->GetBrushSize2();
	m_use_brush_radius2 = vs->GetUseBrushSize2();
	vs->GetBrushRadiusSet(m_brush_radius_sets);

	m_brush_spacing = vs->GetBrushSpacing();
	m_brush_size_data = vs->GetBrushSizeData();
}

void BrushDefault::Apply(flrd::VolumeSelector* vs)
{
	if (!vs)
		return;

	vs->SetBrushIteration(m_iter_num);

	vs->SetBrushIniThresh(m_ini_thresh);
	vs->SetEstimateThreshold(m_estimate_threshold);
	vs->SetBrushGmFalloff(m_gm_falloff);
	vs->SetBrushSclFalloff(m_scl_falloff);
	vs->SetBrushSclTranslate(m_scl_translate);

	vs->SetSelectGroup(m_select_multi);
	vs->SetEdgeDetect(m_edge_detect);
	vs->SetHiddenRemoval(m_hidden_removal);
	vs->SetOrthographic(m_ortho);
	vs->SetUpdateOrder(m_update_order);

	vs->SetW2d(m_w2d);
	vs->SetBrushSize(m_brush_radius1, m_brush_radius2);
	vs->SetUseBrushSize2(m_use_brush_radius2);
	vs->SetBrushRadiusSet(m_brush_radius_sets);

	vs->SetBrushSpacing(m_brush_spacing);
	vs->SetBrushSizeData(m_brush_size_data);

}
