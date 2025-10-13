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

#ifndef OpenXrRenderer_h
#define OpenXrRenderer_h

#include <compatibility.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <unknwn.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>
#endif
#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
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

//placeholder for openxr. remove if supported in the future
#ifdef __APPLE__
#ifndef XR_TYPE_GRAPHICS_BINDING_OPENGL_MACOS_KHR
#define XR_TYPE_GRAPHICS_BINDING_OPENGL_MACOS_KHR 1000123000
#endif
typedef struct XrGraphicsBindingOpenGLMacOSKHR
{
	XrStructureType type;	// Must be XR_TYPE_GRAPHICS_BINDING_OPENGL_MACOS_KHR
	void* next;				// Pointer to the next structure in a structure chain, or NULL
	void* cglContext;		// OpenGL context handle
	void* cglPixelFormat;	// pixelformat handle
} XrGraphicsBindingOpenGLMacOSKHR;
#endif

XrBool32 OpenXRMessageCallbackFunction(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity, XrDebugUtilsMessageTypeFlagsEXT messageType, const XrDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

class OpenXrRenderer : public BaseXrRenderer
{
public:
	OpenXrRenderer();
	virtual ~OpenXrRenderer();

	bool Init(void*, void*, uint64_t) override;
	void Close() override;

	void GetControllerStates() override;

	void BeginFrame() override;
	void EndFrame() override;
	void Draw(const std::vector<std::shared_ptr<flvr::Framebuffer>> &fbos) override;

protected:
	std::string m_app_name;
	std::string m_eng_name;

	XrInstance m_instance = XR_NULL_HANDLE;

	uint32_t m_extensionCount = 0;
	std::vector<XrExtensionProperties> m_extensionProperties;
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
	SwapchainInfo m_swapchain_infos_color = {};
	SwapchainInfo m_swapchain_infos_depth = {};
	std::vector<int64_t> m_preferred_color_formats = {};
	std::vector<int64_t> m_preferred_depth_formats = {};
	uint32_t m_color_image_index = 0;
	uint32_t m_depth_image_index = 0;

	//rendering
	XrFrameState m_frame_state = {XR_TYPE_FRAME_STATE};
	XrFrameWaitInfo m_frame_wait_info = {XR_TYPE_FRAME_WAIT_INFO};
	XrFrameBeginInfo m_frame_begin_info = { XR_TYPE_FRAME_BEGIN_INFO };
	struct RenderLayerInfo {
		XrTime predictedDisplayTime = 0;
		std::vector<XrCompositionLayerBaseHeader*> layers;
		XrCompositionLayerProjection layerProjection = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		std::vector<XrCompositionLayerProjectionView> layerProjectionViews;
		std::vector<XrCompositionLayerDepthInfoKHR> depthInfoViews;
	};
	RenderLayerInfo m_render_layer_info;

	XrActionSet m_act_set = XR_NULL_HANDLE;
	XrAction m_js_act;//joystick
	XrActionStateVector2f m_js_state[2] = { {XR_TYPE_ACTION_STATE_VECTOR2F}, {XR_TYPE_ACTION_STATE_VECTOR2F} };
	XrAction m_grab_act;//grabbing data
	XrActionStateFloat m_grab_state[2] = { {XR_TYPE_ACTION_STATE_FLOAT}, {XR_TYPE_ACTION_STATE_FLOAT} };
	XrAction m_pose_act;//hand or controller
	// The XrPaths for left and right hand hands or controllers.
	XrPath m_hand_paths[2] = {0, 0};
	// The spaces that represents the two hand poses.
	XrSpace m_hand_pose_space[2];
	XrActionStatePose m_hand_pose_state[2] = { {XR_TYPE_ACTION_STATE_POSE}, {XR_TYPE_ACTION_STATE_POSE} };
	// In STAGE space, viewHeightM should be 0. In LOCAL space, it should be offset downwards, below the viewer's initial position.
	float m_view_hm = 1.5f;
	// The current poses obtained from the XrSpaces.
	XrPosef m_hand_pose[2] = {
		{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -m_view_hm}},
		{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -m_view_hm}} };

	//hand tracking
	PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXT = nullptr;
	PFN_xrDestroyHandTrackerEXT xrDestroyHandTrackerEXT = nullptr;
	PFN_xrLocateHandJointsEXT xrLocateHandJointsEXT = nullptr;
	bool m_hand_tracking_supported = false;
	bool m_use_hand_tracking = false;
	XrHandTrackerEXT m_hand_tracker[2];

protected:
	virtual void SetExtensions();
	virtual void CheckExtensions();
	virtual bool CheckExtension(const char* extensionName);

	virtual bool CreateInstance();
	virtual void DestroyInstance();

	virtual void CreateDebugMessenger();
	virtual void DestroyDebugMessenger();

	virtual void GetInstanceProperties();
	virtual bool GetSystemID();
	virtual bool GetViewConfigurationViews();
	virtual bool GetEnvironmentBlendModes();

	virtual bool CreateSession(void*, void*, uint64_t);
	virtual void DestroySession();

	virtual bool CreateReferenceSpace();
	virtual void DestroyReferenceSpace();

	virtual bool CreateSwapchains();
	virtual void DestroySwapchains();
	int64_t SelectSwapchainFormat(
		const std::vector<int64_t>& enum_formats,
		const std::vector<int64_t>& pref_formats);

	virtual bool CreateSwapchainImages(int type, uint32_t count, SwapchainInfo& info);
	virtual void* CreateImageView(int type, int eye, void* format, void* tid);//0:color 1:depth
	virtual void DestroyImageView(void*& imageView);

	virtual void PollEvents();
	virtual void PollActions(XrTime predictedTime);

	XrPath CreateXrPath(const char* path_string)
	{
		XrPath xrPath;
		OPENXR_CHECK(xrStringToPath(m_instance, path_string, &xrPath), "Failed to create XrPath from string.");
		return xrPath;
	}
	std::string FromXrPath(XrPath path)
	{
		uint32_t strl;
		char text[XR_MAX_PATH_LENGTH];
		XrResult res;
		res = xrPathToString(m_instance, path, XR_MAX_PATH_LENGTH, &strl, text);
		std::string str;
		if (res == XR_SUCCESS)
		{
			str = text;
		}
		else
		{
			OPENXR_CHECK(res, "Failed to retrieve path.");
		}
		return str;
	}
	virtual bool CreateAction(XrAction &xrAction,
		const char *name, XrActionType xrActionType,
		std::vector<const char *> subaction_paths = {});
	virtual bool CreateActionSet();
	virtual bool SuggestBindings();
	virtual void RecordCurrentBindings();
	virtual XrSpace CreateActionPoseSpace(XrAction action, const char* path = nullptr);
	virtual void CreateActionPoses();
	virtual void AttachActionSet();
	virtual void DestroyActions();

	virtual void LoadFunctions();

	virtual bool CreateHandTrackers();
	virtual bool TrackHands();

	float DetectHand(const XrHandJointLocationEXT* jointLocations);

	void ApplyEyeOffsets(XrView* views, int eye_index);
	glm::mat4 XrPoseToMat4(const XrPosef& pose);
};

#endif//OpenXrRenderer_h