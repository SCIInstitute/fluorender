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

#include <compatibility.h>
#ifdef _WIN32
#include <unknwn.h>
#endif
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <BaseXrRenderer.h>

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

class OpenXrRenderer : public BaseXrRenderer
{
public:
	OpenXrRenderer();
	virtual ~OpenXrRenderer();

	bool Init(void*, void*) override;
	void Close() override;

	void GetControllerStates() override;

	void BeginFrame() override;
	void EndFrame() override;
	void Draw(const std::vector<flvr::Framebuffer*> &fbos) override;

private:
#ifdef _WIN32
	XrInstance m_instance = XR_NULL_HANDLE;

	std::vector<const char*> m_activeInstanceExtensions = {};
	std::vector<std::string> m_instanceExtensions = {};
	XrDebugUtilsMessengerEXT m_debugUtilsMessenger = XR_NULL_HANDLE;

	XrSystemId m_sys_id = 0;

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

	XrSession m_session = {};
	XrSessionState m_session_state = XR_SESSION_STATE_UNKNOWN;
	bool m_app_running = true;
	bool m_session_running = false;

	XrSpace m_space = XR_NULL_HANDLE;

	bool m_use_depth = false;
	struct SwapchainInfo
	{
		XrSwapchain swapchain = XR_NULL_HANDLE;
		int64_t swapchainFormat = 0;
		std::vector<void*> imageViews;
	};
	std::vector<SwapchainInfo> m_swapchain_infos_color = {};
	std::vector<SwapchainInfo> m_swapchain_infos_depth = {};

	//rendering
	XrFrameState m_frame_state = {XR_TYPE_FRAME_STATE};
	XrFrameWaitInfo m_frame_wait_info = {XR_TYPE_FRAME_WAIT_INFO};
	XrFrameBeginInfo m_frame_begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	struct RenderLayerInfo {
		XrTime predictedDisplayTime = 0;
		std::vector<XrCompositionLayerBaseHeader*> layers;
		XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
	};
	RenderLayerInfo m_render_layer_info;

	XrActionSet m_act_set = XR_NULL_HANDLE;
	XrAction m_act_left = XR_NULL_HANDLE;
	XrAction m_act_right = XR_NULL_HANDLE;

#endif

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

	void* CreateImageView(int type, void* tid);//0:color 1:depth
	void DestroyImageView(void*& imageView);

	void PollEvents();

	bool CreateActions();
	void DestroyActions();

	void ApplyEyeOffsets(XrView* views, int eye_index);
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