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
#ifndef _BRUSHDEFAULT_H_
#define _BRUSHDEFAULT_H_

#include <vector>

namespace flrd
{
	class VolumeSelector;
	enum class SelectMode : int;
}
struct BrushRadiusSet
{
	flrd::SelectMode type;//brush type
	double radius1;//radius 1
	double radius2;//radius 2
	bool use_radius2;//use radius 2
	int iter_num;//iteration number
};

class BrushDefault
{
public:
	BrushDefault();
	~BrushDefault();

	void Read();
	void Save();
	void Set(flrd::VolumeSelector* vs);
	void Apply(flrd::VolumeSelector* vs);

public:
	//default values
	int m_paint_hist_depth;		//paint history depth

	int m_iter_num;				//iteration number for growing

	//brush properties
	double m_ini_thresh;
	bool m_estimate_threshold;	//auto threshold
	double m_gm_falloff;
	double m_scl_falloff;
	double m_scl_translate;

	bool m_select_multi;		//0-only current; 1-select group;
	bool m_edge_detect;
	bool m_hidden_removal;
	bool m_ortho;
	bool m_update_order;		//brick accuracy

	//w2d
	double m_w2d;
	//paint stroke radius
	double m_brush_radius1;
	double m_brush_radius2;
	bool m_use_brush_radius2;
	//radius settings for individual brush types
	std::vector<BrushRadiusSet> m_brush_radius_sets;

	//paint stroke spacing
	double m_brush_spacing;
	//brush size relation
	bool m_brush_size_data;
};
#endif
