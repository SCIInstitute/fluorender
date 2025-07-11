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

#include <Size.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Controller;
struct DisplayInfo;
struct BridgeWindowData;
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
	void Setup();
	void Clear();
	void Draw();
	void SetUpdating(bool val);//setting changed if true
	int GetCurView() { return m_cur_view; }
	bool GetFinished() { return m_finished; }
	double GetOffset();//range of offset [-1, 1]; 0 = center
	void BindRenderBuffer(int nx, int ny);
	Size2D GetViewSize() const;
	void SetRenderViewSize(const Size2D& size)
	{
		m_render_view_size = size;
	}
	Size2D GetQuiltSize();
	Size2D GetQuiltLayout();
	float GetAspect();

	//camera handling
	void HandleCamera(bool persp);
	void SetCamera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
	{
		m_eye = eye;
		m_center = center;
		m_up = up;
	}
	void SetCameraSide(const glm::vec3& side)
	{
		m_side = side;
	}
	void GetCamera(glm::vec3& eye, glm::vec3& center, glm::vec3& up) const
	{
		eye = m_eye;
		center = m_center;
		up = m_up;
	}
	void HandleProjection(bool persp);
	void SetProjection(double aov, double aspect, double near_clip, double far_clip,
		double left, double right, double top, double bottom)
	{
		m_aov = aov;
		m_aspect = aspect;
		m_near = near_clip;
		m_far = far_clip;
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
	}
	glm::mat4 GetProjectionMatrix() const;

private:
	bool m_initialized = false;
	int m_dev_index = 0;

	std::unique_ptr<Controller> m_lg_controller;
	std::vector<std::shared_ptr<DisplayInfo>> m_lg_displays;
	int m_cur_lg_display = 0;
	std::unique_ptr<BridgeWindowData> m_lg_data;

	double m_viewCone = 45.0;	//view angle
	int m_cur_view = 0;			//index to the view
	bool m_updating = false;	//still updating
	int m_upd_view = 0;			//view number when updating starts
	bool m_finished = true;		//finished rendering all views with consistent settings

	Size2D m_render_view_size; //size of the render view, not the quilt view

	//camera handling
	glm::vec3 m_eye;		//camera eye position
	glm::vec3 m_center;		//camera center position
	glm::vec3 m_up;		//camera up vector
	glm::vec3 m_side;		//camera side vector
	//projection
	glm::mat4 m_proj_mat = glm::mat4(1.0); //projection matrix
	double m_aov = 0.0; //angle of view
	double m_aspect = 1.0; //aspect ratio
	double m_near = 0.1; //near plane
	double m_far = 1000.0; //far plane
	double m_left = -1.0; //left plane for orthographic projection
	double m_right = 1.0; //right plane for orthographic projection
	double m_top = 1.0; //top plane for orthographic projection
	double m_bottom = -1.0; //bottom plane for orthographic projection

private:
	void advance_views();

	void HandleCameraTurntable();
	void HandleCameraShifting(bool persp);
};

#endif//LookingGlassRenderer_h