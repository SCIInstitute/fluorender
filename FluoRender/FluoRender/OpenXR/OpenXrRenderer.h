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

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <unknwn.h>
#endif
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <glm/glm.hpp>

#define OPENXR_CHECK(x, y)                                                                                                                                  \
    {                                                                                                                                                       \
        XrResult result = (x);                                                                                                                              \
        if (!XR_SUCCEEDED(result)) {                                                                                                                        \
            std::cerr << "ERROR: OPENXR: " << int(result) << y << std::endl; \
        }                                                                                                                                                   \
    }

XrBool32 OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

template <typename T>
inline bool BitwiseCheck(const T& value, const T& checkValue) {
	return ((value & checkValue) == checkValue);
}

class OpenXrRenderer
{
public:
	OpenXrRenderer();
	~OpenXrRenderer();

	bool Init(void*, void*);
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
	bool m_initialized;
#ifdef _WIN32
	XrInstance m_instance;

	std::vector<const char*> m_activeInstanceExtensions = {};
	XrDebugUtilsMessengerEXT m_debugUtilsMessenger = XR_NULL_HANDLE;

	XrSystemId m_sys_id;

	std::vector<XrViewConfigurationType> m_app_view_configs =
	{
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO
	};
	std::vector<XrViewConfigurationType> m_view_configs;
	XrViewConfigurationType m_view_config =
		XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
	std::vector<XrViewConfigurationView> m_view_config_views;

	std::vector<XrEnvironmentBlendMode> m_app_env_blend_modes =
	{
		XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
		XR_ENVIRONMENT_BLEND_MODE_ADDITIVE
	};
	std::vector<XrEnvironmentBlendMode> m_env_blend_modes = {};
	XrEnvironmentBlendMode m_env_blend_mode =
		XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM;

	XrSession m_session;
	XrSpace m_space;

	struct SwapchainInfo
	{
		XrSwapchain swapchain = XR_NULL_HANDLE;
		int64_t swapchainFormat = 0;
		std::vector<void*> imageViews;
	};
	std::vector<SwapchainInfo> m_swapchain_infos_color = {};
	std::vector<SwapchainInfo> m_swapchain_infos_depth = {};

	XrActionSet m_act_set;
	XrAction m_act_left;
	XrAction m_act_right;
	XrFrameState m_frame_state;

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

private:
	bool CreateInstance();
	void DestroyInstance();

	void CreateDebugMessenger();
	void DestroyDebugMessenger();

	void GetInstanceProperties();
	bool GetSystemID();
	bool GetViewConfigurationViews();
	bool GetEnvironmentBlendModes();

	bool CreateSession(void* hdc, void* hglrc);
	void DestroySession();

	bool CreateReferenceSpace();
	void DestroyReferenceSpace();

	bool CreateSwapchains();
	void DestroySwapchains();

	void* CreateImageView(int type, XrSwapchain swapchain, uint32_t index);//0:color 1:depth
	void DestroyImageView(void*& imageView);
};

inline bool IsStringInVector(
	std::vector<const char*> list,
	const char* name)
{
	bool found = false;
	for (auto& item : list)
	{
		if (strcmp(name, item) == 0)
		{
			found = true;
			break;
		}
	}
	return found;
}

#endif//OpenXrRenderer_h