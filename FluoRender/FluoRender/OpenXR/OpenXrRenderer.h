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

#ifndef OpenXrRenderer_h
#define OpenXrRenderer_h

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <glm/glm.hpp>

class OpenXrRenderer
{
public:
	OpenXrRenderer();
	~OpenXrRenderer();

	bool Init();
	void Close();

	uint32_t GetSize(int i)
	{
		return m_size[i];
	}

	glm::mat4 GetProjectionMatrix(int eye_index, float near_clip, float far_clip);
	glm::mat4 GetModelViewMatrix();
	void GetControllerStates();
	float GetControllerLeftThumbstickX() { return m_left_x; }
	float GetControllerLeftThumbstickY() { return m_left_y; }
	float GetControllerRightThumbstickX() { return m_right_x; }
	float GetControllerRightThumbstickY() { return m_right_y; }

	void BeginFrame();
	void EndFrame();

	void DrawLeft(uint32_t left_buffer);
	void DrawRight(uint32_t right_buffer);

private:
#ifdef _WIN32
	XrInstance m_instance;
	XrSystemId m_sys_id;
	XrSession m_session;
	XrSpace m_space;
	XrActionSet m_act_set;
	XrAction m_act_left;
	XrAction m_act_right;
	XrFrameState m_frame;
	XrSwapchain m_swap_chain_left;
	XrSwapchain m_swap_chain_right;
	XrSwapchainImageAcquireInfo m_acquire_info;
	XrSwapchainImageWaitInfo m_wait_info;
	XrSwapchainImageReleaseInfo m_release_info;
	XrCompositionLayerProjection m_layer_proj;
	XrCompositionLayerProjectionView m_proj_views[2];
#endif
	uint32_t m_size[2];
	float m_left_x;
	float m_left_y;
	float m_right_x;
	float m_right_y;
	float m_dead_zone;
	float m_scaler;
};

#endif//OpenXrRenderer_h