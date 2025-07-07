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
#ifndef LookingGlassRenderer_h
#define LookingGlassRenderer_h

#include <memory>
#include <bridge_utils.hpp>

class LookingGlassRenderer
{
public:
	LookingGlassRenderer();
	~LookingGlassRenderer();

	bool Init();
	void Close();
	void SetDevIndex(int val) { m_dev_index = val; }
	int GetDisplayId();
	double GetHalfCone() { return m_viewCone / 2; }
	void SetPreset(int val);
	void Setup();
	void Clear();
	void Draw();
	void SetUpdating(bool val);//setting changed if true
	int GetCurView() { return m_cur_view; }
	bool GetFinished() { return m_finished; }
	double GetOffset();//range of offset [-1, 1]; 0 = center
	void BindRenderBuffer(int nx, int ny);

private:
	bool m_initialized = false;
	int m_dev_index = 0;

	std::unique_ptr<Controller> m_lg_controller;
	BridgeWindowData m_lg_data;
	std::vector<DisplayInfo> m_lg_displays;
	int m_cur_lg_display = 0;

	// quilt settings
	int m_preset = 0;		// Set up the quilt settings according to the preset passed
							// 0: 32 views
							// 1: 45 views, normally used one
							// 2: 45 views for 8k display
							// Feel free to customize if you want
	int m_width = 2048;		// Total width of the quilt texture
	int m_height = 2048;	// Total height of the quilt texture
	int m_rows = 8;			// Number of columns in the quilt
	int m_columns = 4;		// Number of rows in the quilt
	int m_totalViews = 32;	// The total number of views in the quilt.
							// Note that this number might be lower than rows *
							// columns
							// qs_viewWidth & qs_viewHeight could be calculated by given numbers
	int m_viewWidth = 512;	//quilt view dimensions
	int m_viewHeight = 256;

	double m_viewCone = 45.0;	//view angle
	int m_cur_view = 0;			//index to the view
	bool m_updating = false;	//still updating
	int m_upd_view = 0;			//view number when updating starts
	bool m_finished = true;		//finished rendering all views with consistent settings

private:
	void advance_views();
};

#endif//LookingGlassRenderer_h