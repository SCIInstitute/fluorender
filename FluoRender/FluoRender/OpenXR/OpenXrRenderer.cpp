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

#include <GL/glew.h>
#include <Framebuffer.h>
#include <OpenXrRenderer.h>
#include <Global.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <vector>
#include <algorithm>
#ifdef _DEBUG
#include <Debug.h>
#endif

OpenXrRenderer::OpenXrRenderer() :
	BaseXrRenderer()
{
}

OpenXrRenderer::~OpenXrRenderer()
{

}

bool OpenXrRenderer::Init(void* hdc, void* hglrc)
{
	if (m_initialized)
		return m_initialized;

	XrResult result;

	// OpenXR initialization
	if (!CreateInstance())
		return false;

#ifdef _DEBUG
	CreateDebugMessenger();
	GetInstanceProperties();
#endif

	if (!GetSystemID())
		return false;

	CreateActionSet();
	SuggestBindings();

	// Get render size
	if (!GetViewConfigurationViews())
		return false;

	GetEnvironmentBlendModes();

	if (!CreateSession(hdc, hglrc))
		return false;

	CreateActionPoses();
	AttachActionSet();

	if (!CreateReferenceSpace())
		return false;

	if (!CreateSwapchains())
		return false;

	m_initialized = true;

	return m_initialized;
}

void OpenXrRenderer::Close()
{
	DestroyActions();
	DestroySwapchains();
	DestroyReferenceSpace();
	DestroySession();
#ifdef _DEBUG
	DestroyDebugMessenger();
#endif
	DestroyInstance();
}

void OpenXrRenderer::GetControllerStates()
{
	bool grab[2] = { false, false };
	//grab
	if (m_grab_state[0].isActive &&
		m_grab_state[0].currentState > 0.5f)
	{
		grab[0] = true;
	}
	if (m_grab_state[1].isActive &&
		m_grab_state[1].currentState > 0.5f)
	{
		grab[1] = true;
	}

	if (grab[0] && !m_grab_prev[0])
	{
		m_grab[0] = !m_grab[0];
		m_grab[1] = false;
	}
	if (grab[1] && !m_grab_prev[1])
	{
		m_grab[0] = false;
		m_grab[1] = !m_grab[1];
	}
	m_grab_prev[0] = grab[0];
	m_grab_prev[1] = grab[1];

	if (m_grab[0])
	{
		m_grab_mat = XrPoseToMat4(m_hand_pose[0]);
		m_grab_mat[3][0] = 0.0;
		m_grab_mat[3][1] = 0.0;
		m_grab_mat[3][2] = 0.0;
	}
	else if (m_grab[1])
	{
		m_grab_mat = XrPoseToMat4(m_hand_pose[1]);
		m_grab_mat[3][0] = 0.0;
		m_grab_mat[3][1] = 0.0;
		m_grab_mat[3][2] = 0.0;
	}
	else
	{
		m_grab_mat = glm::mat4(1.0);
	}


	//joystick
	if (m_js_state[0].isActive)
	{
		m_left_x = m_js_state[0].currentState.x;
		m_left_y = m_js_state[0].currentState.y;
	}
	if (m_js_state[1].isActive)
	{
		m_right_x = m_js_state[1].currentState.x;
		m_right_y = m_js_state[1].currentState.y;
	}

	if (m_left_x > -m_dead_zone && m_left_x < m_dead_zone) m_left_x = 0.0;
	if (m_left_y > -m_dead_zone && m_left_y < m_dead_zone) m_left_y = 0.0;
	if (m_right_x > -m_dead_zone && m_right_x < m_dead_zone) m_right_x = 0.0;
	if (m_right_y > -m_dead_zone && m_right_y < m_dead_zone) m_right_y = 0.0;

	m_left_x *= m_scaler;
	m_left_y *= m_scaler;
	m_right_x *= m_scaler;
	m_right_y *= m_scaler;
}

void OpenXrRenderer::BeginFrame()
{
	if (!m_app_running)
		return;

	PollEvents();

	if (!m_session_running)
		return;

	// Wait for the next frame
	xrWaitFrame(m_session, &m_frame_wait_info, &m_frame_state);

	// Begin the frame
	xrBeginFrame(m_session, &m_frame_begin_info);

	m_render_layer_info.predictedDisplayTime = m_frame_state.predictedDisplayTime;
	m_render_layer_info.layers.clear();

	//bool sessionActive = (
	//	m_session_state == XR_SESSION_STATE_SYNCHRONIZED ||
	//	m_session_state == XR_SESSION_STATE_VISIBLE ||
	//	m_session_state == XR_SESSION_STATE_FOCUSED);
	//m_should_render = sessionActive && m_frame_state.shouldRender;
	//m_should_render = m_frame_state.shouldRender;
	if (!m_frame_state.shouldRender)
		return;

	// poll actions here because they require a predicted display time, which we've only just obtained.
	PollActions(m_frame_state.predictedDisplayTime);

	// Locate the views from the view configuration within the (reference) space at the display time.
	std::vector<XrView> views(m_view_config_views.size(), { XR_TYPE_VIEW });

	XrViewState viewState{ XR_TYPE_VIEW_STATE };  // Will contain information on whether the position and/or orientation is valid and/or tracked.
	XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
	viewLocateInfo.viewConfigurationType = m_view_config;
	viewLocateInfo.displayTime = m_render_layer_info.predictedDisplayTime;
	viewLocateInfo.space = m_space;
	uint32_t viewCount = 0;
	XrResult result = xrLocateViews(
		m_session, &viewLocateInfo, &viewState,
		static_cast<uint32_t>(views.size()),
		&viewCount, views.data());
	if (result != XR_SUCCESS) return;

	m_render_layer_info.layerProjectionViews.resize(
		viewCount, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW });

	// Per view in the view configuration:
	for (uint32_t i = 0; i < viewCount; i++)
	{
		// offset eye pos
		ApplyEyeOffsets(&views[i], i);

		// Get the width and height and construct the viewport and scissors.
		const uint32_t &width = m_view_config_views[i].recommendedImageRectWidth;
		const uint32_t &height = m_view_config_views[i].recommendedImageRectHeight;
		// Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
		// This also associates the swapchain image with this layer projection view.
		m_render_layer_info.layerProjectionViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		m_render_layer_info.layerProjectionViews[i].pose = views[i].pose;
		m_render_layer_info.layerProjectionViews[i].fov = views[i].fov;
		m_render_layer_info.layerProjectionViews[i].subImage.imageRect.offset.x = 0;
		m_render_layer_info.layerProjectionViews[i].subImage.imageRect.offset.y = 0;
		m_render_layer_info.layerProjectionViews[i].subImage.imageRect.extent.width = static_cast<int32_t>(width);
		m_render_layer_info.layerProjectionViews[i].subImage.imageRect.extent.height = static_cast<int32_t>(height);
		m_render_layer_info.layerProjectionViews[i].subImage.imageArrayIndex = 0;  // Useful for multiview rendering.

		// Compute the view-projection transform.
		if (i < 2)
		{
			XrFovf fov = views[i].fov;
			float tanLeft = tanf(fov.angleLeft);
			float tanRight = tanf(fov.angleRight);
			float tanUp = tanf(fov.angleUp);
			float tanDown = tanf(fov.angleDown);

			float tanWidth = tanRight - tanLeft;
			float tanHeight = tanUp - tanDown;

			m_proj_mat[i] = glm::frustum(
				tanLeft * m_near_clip, tanRight * m_near_clip,
				tanDown * m_near_clip, tanUp * m_near_clip,
				m_near_clip, m_far_clip);

			//
			XrPosef pose = views[i].pose;
			glm::mat4 viewMatrix = glm::mat4(
				1.0f - 2.0f * (pose.orientation.y * pose.orientation.y + pose.orientation.z * pose.orientation.z),
				2.0f * (pose.orientation.x * pose.orientation.y + pose.orientation.w * pose.orientation.z),
				2.0f * (pose.orientation.x * pose.orientation.z - pose.orientation.w * pose.orientation.y),
				0.0f,

				2.0f * (pose.orientation.x * pose.orientation.y - pose.orientation.w * pose.orientation.z),
				1.0f - 2.0f * (pose.orientation.x * pose.orientation.x + pose.orientation.z * pose.orientation.z),
				2.0f * (pose.orientation.y * pose.orientation.z + pose.orientation.w * pose.orientation.x),
				0.0f,

				2.0f * (pose.orientation.x * pose.orientation.z + pose.orientation.w * pose.orientation.y),
				2.0f * (pose.orientation.y * pose.orientation.z - pose.orientation.w * pose.orientation.x),
				1.0f - 2.0f * (pose.orientation.x * pose.orientation.x + pose.orientation.y * pose.orientation.y),
				0.0f,

				pose.position.x,
				pose.position.y,
				pose.position.z,
				1.0f
			);

			m_mv_mat[i] = glm::inverse(viewMatrix);
		}
	}
}

void OpenXrRenderer::EndFrame()
{
	if (!m_app_running || !m_session_running)
		return;
	if (!m_frame_state.shouldRender)
		return;

	// Fill out the XrCompositionLayerProjection structure for usage with xrEndFrame().
	m_render_layer_info.layerProjection.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_CORRECT_CHROMATIC_ABERRATION_BIT;
	m_render_layer_info.layerProjection.space = m_space;
	m_render_layer_info.layerProjection.viewCount = static_cast<uint32_t>(m_render_layer_info.layerProjectionViews.size());
	m_render_layer_info.layerProjection.views = m_render_layer_info.layerProjectionViews.data();
	m_render_layer_info.layers.push_back(
		reinterpret_cast<XrCompositionLayerBaseHeader*>(
			&m_render_layer_info.layerProjection));
	// End the frame
	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = m_frame_state.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = m_env_blend_mode;
	frameEndInfo.layerCount = static_cast<uint32_t>(m_render_layer_info.layers.size());
	frameEndInfo.layers = m_render_layer_info.layers.data();
	XrResult result = xrEndFrame(m_session, &frameEndInfo);
	if (result != XR_SUCCESS)
	{
#ifdef _DEBUG
		DBGPRINT(L"xrEndFrame failed.\n");
#endif
	}
}

void OpenXrRenderer::Draw(const std::vector<flvr::Framebuffer*> &fbos)
{
	if (!m_app_running || !m_session_running)
		return;
	if (!m_frame_state.shouldRender)
		return;

	uint32_t viewCount = m_render_layer_info.layerProjectionViews.size();
	// Per view in the view configuration:
	for (uint32_t i = 0; i < viewCount; i++)
	{
		// Get the width and height and construct the viewport and scissors.
		const uint32_t &width = m_view_config_views[i].recommendedImageRectWidth;
		const uint32_t &height = m_view_config_views[i].recommendedImageRectHeight;

		SwapchainInfo& colorSwapchainInfo = m_swapchain_infos_color[i];
		SwapchainInfo& depthSwapchainInfo = m_swapchain_infos_depth[i];

		// Acquire and wait for an image from the swapchains.
		// Get the image index of an image in the swapchains.
		// The timeout is infinite.
		uint32_t colorImageIndex = 0;
		uint32_t depthImageIndex = 0;
		XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		OPENXR_CHECK(xrAcquireSwapchainImage(colorSwapchainInfo.swapchain, &acquireInfo, &colorImageIndex), "Failed to acquire Image from the Color Swapchian");
		if (m_use_depth)
			OPENXR_CHECK(xrAcquireSwapchainImage(depthSwapchainInfo.swapchain, &acquireInfo, &depthImageIndex), "Failed to acquire Image from the Depth Swapchian");

		XrSwapchainImageWaitInfo waitInfo = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		waitInfo.timeout = XR_INFINITE_DURATION;
		OPENXR_CHECK(xrWaitSwapchainImage(colorSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Color Swapchain");
		if (m_use_depth)
			OPENXR_CHECK(xrWaitSwapchainImage(depthSwapchainInfo.swapchain, &waitInfo), "Failed to wait for Image from the Depth Swapchain");

		// Fill out the XrCompositionLayerProjectionView structure specifying the pose and fov from the view.
		// This also associates the swapchain image with this layer projection view.
		m_render_layer_info.layerProjectionViews[i].subImage.swapchain = colorSwapchainInfo.swapchain;

		//copy buffer
		if (fbos.size() > i)
		{
			//GLuint dest_fbo = (GLuint)(uint64_t)(colorSwapchainInfo.imageViews[colorImageIndex]);
			//glBindFramebuffer(GL_FRAMEBUFFER, dest_fbo);
			//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[i]->id());
			// Read pixels to PBO
			GLuint dest_fbo = (GLuint)(uint64_t)(colorSwapchainInfo.imageViews[colorImageIndex]);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			// Write pixels to destination FBO
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Give the swapchain image back to OpenXR, allowing the compositor to use the image.
		XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		OPENXR_CHECK(xrReleaseSwapchainImage(colorSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Color Swapchain");
		if (m_use_depth)
			OPENXR_CHECK(xrReleaseSwapchainImage(depthSwapchainInfo.swapchain, &releaseInfo), "Failed to release Image back to the Depth Swapchain");
	}
}

bool OpenXrRenderer::CreateInstance()
{
	XrResult result;
	// Define application information
	// The application/engine name and version are user-definied.
	// These may help IHVs or runtimes.
	XrApplicationInfo appInfo = {};
	strncpy(appInfo.applicationName, "FluoRender", XR_MAX_APPLICATION_NAME_SIZE);
	appInfo.applicationVersion = 1;
	strncpy(appInfo.engineName, "FLRENDER", XR_MAX_ENGINE_NAME_SIZE);
	appInfo.engineVersion = 1;
	//there is no way to know which version is supported before creating the instance
	//so, use the lowest version number for now
	appInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0); //XR_CURRENT_API_VERSION;

	// Add additional instance layers/extensions that the application wants.
	// Add both required and requested instance extensions.
	m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
	m_instanceExtensions.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

	// Get all the API Layers from the OpenXR runtime.
	uint32_t apiLayerCount = 0;
	std::vector<XrApiLayerProperties> apiLayerProperties;
	result = xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr);
	if (result != XR_SUCCESS) return false;
	apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
	result = xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data());
	if (result != XR_SUCCESS) return false;

	// Check the requested API layers against the ones from the OpenXR.
	// If found add it to the Active API Layers.
	std::vector<std::string> apiLayers = {};
	std::vector<const char *> activeAPILayers = {};
	for (auto& requestLayer : apiLayers)
	{
		for (auto& layerProperty : apiLayerProperties)
		{
			// strcmp returns 0 if the strings match.
			if (strcmp(requestLayer.c_str(),
				layerProperty.layerName) != 0)
			{
				continue;
			}
			else
			{
				activeAPILayers.push_back(
					requestLayer.c_str());
				break;
			}
		}
	}

	// Get all the Instance Extensions from the OpenXR instance.
	uint32_t extensionCount = 0;
	std::vector<XrExtensionProperties> extensionProperties;
	result = xrEnumerateInstanceExtensionProperties(
		nullptr, 0, &extensionCount, nullptr);
	if (result != XR_SUCCESS) return false;
	extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
	result = xrEnumerateInstanceExtensionProperties(
		nullptr, extensionCount, &extensionCount,
		extensionProperties.data());
	if (result != XR_SUCCESS) return false;

	// Check the requested Instance Extensions against the ones from the OpenXR runtime.
	// If an extension is found add it to Active Instance Extensions.
	// Log error if the Instance Extension is not found.
	for (auto& requestedInstanceExtension : m_instanceExtensions)
	{
		bool found = false;
		for (auto& extensionProperty : extensionProperties) {
			// strcmp returns 0 if the strings match.
			if (strcmp(requestedInstanceExtension.c_str(),
				extensionProperty.extensionName) != 0)
			{
				continue;
			}
			else
			{
				m_activeInstanceExtensions.push_back(requestedInstanceExtension.c_str());
				found = true;
				break;
			}
		}
		if (!found)
		{
#ifdef _DEBUG
			DBGPRINT(L"Failed to find OpenXR instance extension: %s\n",
				requestedInstanceExtension.c_str());
#endif
		}
	}

	// Fill out an XrInstanceCreateInfo structure and create an XrInstance.
	// XR_DOCS_TAG_BEGIN_XrInstanceCreateInfo
	XrInstanceCreateInfo instanceCI{XR_TYPE_INSTANCE_CREATE_INFO};
	instanceCI.createFlags = 0;
	instanceCI.applicationInfo = appInfo;
	instanceCI.enabledApiLayerCount = static_cast<uint32_t>(activeAPILayers.size());
	instanceCI.enabledApiLayerNames = activeAPILayers.data();
	instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
	instanceCI.enabledExtensionNames = m_activeInstanceExtensions.data();
	result = xrCreateInstance(&instanceCI, &m_instance);
	if (result != XR_SUCCESS) return false;

	return true;
}

void OpenXrRenderer::DestroyInstance()
{
	// Destroy the instance
	if (m_instance != XR_NULL_HANDLE)
	{
		xrDestroyInstance(m_instance);
	}
}

void OpenXrRenderer::CreateDebugMessenger()
{
	XrResult result;
	if (IsStringInVector(m_activeInstanceExtensions,
		XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		// Set the userCallback to OpenXRMessageCallbackFunction().
		XrDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{ XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		debugUtilsMessengerCI.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugUtilsMessengerCI.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
		debugUtilsMessengerCI.userCallback = (PFN_xrDebugUtilsMessengerCallbackEXT)OpenXRMessageCallbackFunction;
		debugUtilsMessengerCI.userData = nullptr;

		// Load xrCreateDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
		XrDebugUtilsMessengerEXT debugUtilsMessenger{};
		PFN_xrCreateDebugUtilsMessengerEXT xrCreateDebugUtilsMessengerEXT;
		result = xrGetInstanceProcAddr(m_instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)&xrCreateDebugUtilsMessengerEXT);
		if (result != XR_SUCCESS ||
			!xrCreateDebugUtilsMessengerEXT) return;

		// Finally create and return the XrDebugUtilsMessengerEXT.
		result = xrCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsMessengerCI, &debugUtilsMessenger);
	}
}

void OpenXrRenderer::DestroyDebugMessenger()
{
	if (m_debugUtilsMessenger != XR_NULL_HANDLE)
	{
		// Load xrDestroyDebugUtilsMessengerEXT() function pointer as it is not default loaded by the OpenXR loader.
		PFN_xrDestroyDebugUtilsMessengerEXT xrDestroyDebugUtilsMessengerEXT;
		XrResult result = xrGetInstanceProcAddr(m_instance, "xrDestroyDebugUtilsMessengerEXT",
			(PFN_xrVoidFunction*)&xrDestroyDebugUtilsMessengerEXT);
		if (result != XR_SUCCESS ||
			!xrDestroyDebugUtilsMessengerEXT) return;
		// Destroy the provided XrDebugUtilsMessengerEXT.
		xrDestroyDebugUtilsMessengerEXT(m_debugUtilsMessenger);
	}
}

void OpenXrRenderer::GetInstanceProperties()
{
	// Get the instance's properties and log the runtime name and version.
	XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
	xrGetInstanceProperties(m_instance, &instanceProperties);
	std::string str = instanceProperties.runtimeName;
	std::wstring wstr = s2ws(str);
#ifdef _DEBUG
	DBGPRINT(L"OpenXR Runtime: %s - %d.%d.%d\n",
		wstr.c_str(),
		XR_VERSION_MAJOR(instanceProperties.runtimeVersion),
		XR_VERSION_MINOR(instanceProperties.runtimeVersion),
		XR_VERSION_PATCH(instanceProperties.runtimeVersion));
#endif
}

bool OpenXrRenderer::GetSystemID()
{
	XrResult result;
	// Get the XrSystemId from the instance and the supplied XrFormFactor.
	XrFormFactor formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	XrSystemGetInfo systemGI{ XR_TYPE_SYSTEM_GET_INFO };
	systemGI.formFactor = formFactor;
	result = xrGetSystem(m_instance, &systemGI, &m_sys_id);
	if (result != XR_SUCCESS) return false;

	// Get the System's properties for some general information about the hardware and the vendor.
#ifdef _DEBUG
	XrSystemProperties systemProperties = {XR_TYPE_SYSTEM_PROPERTIES};
	result = xrGetSystemProperties(m_instance, m_sys_id, &systemProperties);
#endif

	return true;
}

bool OpenXrRenderer::GetViewConfigurationViews()
{
	XrResult result;
	// Gets the View Configuration Types.
	// The first call gets the count of the array that will be returned.
	// The next call fills out the array.
	uint32_t viewConfigurationCount = 0;
	result = xrEnumerateViewConfigurations(
		m_instance, m_sys_id, 0, &viewConfigurationCount, nullptr);
	if (result != XR_SUCCESS) return false;
	m_view_configs.resize(viewConfigurationCount);
	result = xrEnumerateViewConfigurations(
		m_instance, m_sys_id, viewConfigurationCount,
		&viewConfigurationCount, m_view_configs.data());
	if (result != XR_SUCCESS) return false;

	// Pick the first application supported View Configuration Type con supported by the hardware.
	for (const XrViewConfigurationType& viewConfiguration : m_app_view_configs)
	{
		if (std::find(m_view_configs.begin(), m_view_configs.end(),
			viewConfiguration) != m_view_configs.end())
		{
			m_view_config = viewConfiguration;
			break;
		}
	}
	if (m_view_config == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM)
	{
		// Failed to find a view configuration type.
		// Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO.
		m_view_config = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	}

	// Gets the View Configuration Views.
	// The first call gets the count of the array that will be returned.
	// The next call fills out the array.
	uint32_t viewConfigurationViewCount = 0;
	result = xrEnumerateViewConfigurationViews(
		m_instance, m_sys_id, m_view_config, 0,
		&viewConfigurationViewCount, nullptr);
	if (result != XR_SUCCESS) return false;
	m_view_config_views.resize(
		viewConfigurationViewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
	result = xrEnumerateViewConfigurationViews(
		m_instance, m_sys_id, m_view_config,
		viewConfigurationViewCount, &viewConfigurationViewCount,
		m_view_config_views.data());
	if (result != XR_SUCCESS) return false;

	uint32_t viewCount = m_view_config_views.size();
	if (viewCount == 0) return false;
	m_size[0] = m_view_config_views[0].recommendedImageRectWidth;
	m_size[1] = m_view_config_views[0].recommendedImageRectHeight;

	return true;
}

bool OpenXrRenderer::GetEnvironmentBlendModes()
{
	XrResult result;
	// Retrieves the available blend modes.
	// The first call gets the count of the array that will be returned.
	// The next call fills out the array.
	uint32_t environmentBlendModeCount = 0;
	result = xrEnumerateEnvironmentBlendModes(m_instance, m_sys_id, m_view_config,
		0, &environmentBlendModeCount, nullptr);
	if (result != XR_SUCCESS) return false;

	m_env_blend_modes.resize(environmentBlendModeCount);
	result = xrEnumerateEnvironmentBlendModes(m_instance, m_sys_id, m_view_config,
		environmentBlendModeCount, &environmentBlendModeCount,
		m_env_blend_modes.data());
	if (result != XR_SUCCESS) return false;

	// Pick the first application supported blend mode supported by the hardware.
	for (const XrEnvironmentBlendMode& environmentBlendMode :
		m_app_env_blend_modes)
	{
		if (std::find(m_env_blend_modes.begin(), m_env_blend_modes.end(),
			environmentBlendMode) != m_env_blend_modes.end())
		{
			m_env_blend_mode = environmentBlendMode;
			break;
		}
	}
	if (m_env_blend_mode == XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM)
	{
#ifdef _DEBUG
		DBGPRINT(L"Failed to find a compatible blend mode. Defaulting to XR_ENVIRONMENT_BLEND_MODE_OPAQUE.\n");
#endif
		m_env_blend_mode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	}

	return true;
}

bool OpenXrRenderer::CreateSession(void* hdc, void* hglrc)
{
	XrResult result;
	// Create an XrSessionCreateInfo structure.
	XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};
	// Create a std::unique_ptr<GraphicsAPI_...> from the instance and system.
	// This call sets up a graphics API that's suitable for use with OpenXR.
	// Get OpenGL graphics requirements
	PFN_xrGetOpenGLGraphicsRequirementsKHR xrGetOpenGLGraphicsRequirementsKHR = nullptr;
	result = xrGetInstanceProcAddr(m_instance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetOpenGLGraphicsRequirementsKHR);
	if (result != XR_SUCCESS) return false;
	XrGraphicsRequirementsOpenGLKHR requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR };
	result = xrGetOpenGLGraphicsRequirementsKHR(m_instance, m_sys_id, &requirements);
	if (result != XR_SUCCESS) return false;
	XrGraphicsBindingOpenGLWin32KHR graphicsBindingOpenGL = { XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
	graphicsBindingOpenGL.hDC = static_cast<HDC>(hdc);
	graphicsBindingOpenGL.hGLRC = static_cast<HGLRC>(hglrc);

	// Fill out the XrSessionCreateInfo structure and create an XrSession.
	sessionCI.next = (void*)&graphicsBindingOpenGL;
	sessionCI.createFlags = 0;
	sessionCI.systemId = m_sys_id;
	result = xrCreateSession(m_instance, &sessionCI, &m_session);
	if (result != XR_SUCCESS) return false;

	return true;
}

void OpenXrRenderer::DestroySession()
{
	// End the session
	if (m_session != XR_NULL_HANDLE)
	{
		xrEndSession(m_session);
		xrDestroySession(m_session);
	}
}

bool OpenXrRenderer::CreateReferenceSpace()
{
	XrResult result;
	// Fill out an XrReferenceSpaceCreateInfo structure and create a reference XrSpace, specifying a Local space with an identity pose as the origin.
	XrReferenceSpaceCreateInfo referenceSpaceCI{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
	referenceSpaceCI.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;// or XR_REFERENCE_SPACE_TYPE_STAGE
	referenceSpaceCI.poseInReferenceSpace = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
	result = xrCreateReferenceSpace(m_session, &referenceSpaceCI, &m_space);
	if (result != XR_SUCCESS) return false;

	return true;
}

void OpenXrRenderer::DestroyReferenceSpace()
{
	if (m_space != XR_NULL_HANDLE)
	{
		xrDestroySpace(m_space);
	}
}

bool OpenXrRenderer::CreateSwapchains()
{
	XrResult result;
	// Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
	uint32_t formatCount = 0;
	result = xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr);
	if (result != XR_SUCCESS) return false;
	std::vector<int64_t> formats(formatCount);
	result = xrEnumerateSwapchainFormats(
		m_session, formatCount, &formatCount, formats.data());
	if (result != XR_SUCCESS) return false;

	std::vector<int64_t> supportSFColor = {
		GL_RGB10_A2,
		GL_RGBA16F,
		// The two below should only be used as a fallback, as they are linear color formats without enough bits for color
		// depth, thus leading to banding.
		GL_RGBA8,
		GL_RGBA8_SNORM,
	};
	std::vector<int64_t> supportSFDepth = {
		GL_DEPTH_COMPONENT32F,
		GL_DEPTH_COMPONENT32,
		GL_DEPTH_COMPONENT24,
		GL_DEPTH_COMPONENT16 };

	const std::vector<int64_t>::const_iterator& itor_color =
		std::find_first_of(formats.begin(), formats.end(),
		std::begin(supportSFColor), std::end(supportSFColor));
	if (itor_color == formats.end())
	{
#ifdef _DEBUG
		DBGPRINT(L"Failed to find color format for Swapchain.\n");
#endif
	}
	const std::vector<int64_t>::const_iterator& itor_depth =
		std::find_first_of(formats.begin(), formats.end(),
		std::begin(supportSFDepth), std::end(supportSFDepth));
	if (itor_depth == formats.end())
	{
#ifdef _DEBUG
		DBGPRINT(L"Failed to find depth format for Swapchain.\n");
#endif
	}

	//Resize the SwapchainInfo to match the number of view in the View Configuration.
	m_swapchain_infos_color.resize(m_view_config_views.size());
	m_swapchain_infos_depth.resize(m_view_config_views.size());

	// Per view, create a color and depth swapchain, and their associated image views.
	for (size_t i = 0; i < m_view_config_views.size(); i++)
	{
		SwapchainInfo& colorSwapchainInfo = m_swapchain_infos_color[i];
		SwapchainInfo& depthSwapchainInfo = m_swapchain_infos_depth[i];

		// Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
		// Color.
		XrSwapchainCreateInfo swapchainCI{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchainCI.createFlags = 0;
		swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.format = *itor_color;
		swapchainCI.sampleCount = m_view_config_views[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
		swapchainCI.width = m_view_config_views[i].recommendedImageRectWidth;
		swapchainCI.height = m_view_config_views[i].recommendedImageRectHeight;
		swapchainCI.faceCount = 1;
		swapchainCI.arraySize = 1;
		swapchainCI.mipCount = 1;
		result = xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain);
		if (result != XR_SUCCESS) return false;
		colorSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

		// Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
		uint32_t colorSwapchainImageCount = 0;
		result = xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr);
		if (result != XR_SUCCESS) return false;
		std::vector<XrSwapchainImageOpenGLKHR> swapchain_images;
		swapchain_images.resize(colorSwapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
		XrSwapchainImageBaseHeader* colorSwapchainImages = reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain_images.data());
		result = xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain,
			colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages);
		if (result != XR_SUCCESS) return false;

		// Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
		for (uint32_t j = 0; j < colorSwapchainImageCount; j++)
		{
			colorSwapchainInfo.imageViews.push_back(CreateImageView(0, (void*)(uint64_t)swapchain_images[j].image));
		}

		// Depth.
		if (m_use_depth)
		{
			swapchainCI.createFlags = 0;
			swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			swapchainCI.format = *itor_depth;
			swapchainCI.sampleCount = m_view_config_views[i].recommendedSwapchainSampleCount;  // Use the recommended values from the XrViewConfigurationView.
			swapchainCI.width = m_view_config_views[i].recommendedImageRectWidth;
			swapchainCI.height = m_view_config_views[i].recommendedImageRectHeight;
			swapchainCI.faceCount = 1;
			swapchainCI.arraySize = 1;
			swapchainCI.mipCount = 1;
			result = xrCreateSwapchain(m_session, &swapchainCI, &depthSwapchainInfo.swapchain);
			if (result != XR_SUCCESS) return false;
			depthSwapchainInfo.swapchainFormat = swapchainCI.format;  // Save the swapchain format for later use.

			uint32_t depthSwapchainImageCount = 0;
			result = xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr);
			swapchain_images.resize(depthSwapchainImageCount, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR });
			XrSwapchainImageBaseHeader* depthSwapchainImages = reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain_images.data());
			result = xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain,
				depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages);
			if (result != XR_SUCCESS) return false;

			for (uint32_t j = 0; j < depthSwapchainImageCount; j++)
			{
				depthSwapchainInfo.imageViews.push_back(CreateImageView(1, (void*)(uint64_t)swapchain_images[j].image));
			}
		}
	}

	return true;
}

void OpenXrRenderer::DestroySwapchains()
{
	// Per view in the view configuration:
	for (size_t i = 0; i < m_view_config_views.size(); i++)
	{
		SwapchainInfo& colorSwapchainInfo = m_swapchain_infos_color[i];
		SwapchainInfo& depthSwapchainInfo = m_swapchain_infos_depth[i];

		// Destroy the color and depth image views from GraphicsAPI.
		for (void*& imageView : colorSwapchainInfo.imageViews)
		{
			DestroyImageView(imageView);
		}
		// Destroy the swapchains.
		xrDestroySwapchain(colorSwapchainInfo.swapchain);

		if (m_use_depth)
		{
			for (void*& imageView : depthSwapchainInfo.imageViews)
			{
				DestroyImageView(imageView);
			}

			xrDestroySwapchain(depthSwapchainInfo.swapchain);
		}
	}
}

void* OpenXrRenderer::CreateImageView(int type, void* tid)
{
	GLuint framebuffer = 0;
	GLenum attachment = GL_COLOR_ATTACHMENT0;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	switch (type)
	{
	case 0://color
		attachment = GL_COLOR_ATTACHMENT0;
		break;
	case 1://depth
		attachment = GL_DEPTH_ATTACHMENT;
		break;
	}
	GLuint tex_id = (GLuint)(uint64_t)tid;
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
		attachment, GL_TEXTURE_2D,
		tex_id, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return (void *)(uint64_t)framebuffer;
}

void OpenXrRenderer::DestroyImageView(void *&imageView)
{
	GLuint framebuffer = (GLuint)(uint64_t)imageView;
	glDeleteFramebuffers(1, &framebuffer);
	imageView = nullptr;
}

void OpenXrRenderer::PollEvents()
{
	// Poll OpenXR for a new event.
	XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };
	auto XrPollEvents = [&]() -> bool
	{
		eventData = { XR_TYPE_EVENT_DATA_BUFFER };
		return xrPollEvent(m_instance, &eventData) == XR_SUCCESS;
	};

	while (XrPollEvents())
	{
		switch (eventData.type)
		{
			// Log the number of lost events from the runtime.
			case XR_TYPE_EVENT_DATA_EVENTS_LOST:
			{
				XrEventDataEventsLost* eventsLost =
					reinterpret_cast<XrEventDataEventsLost*>(&eventData);
#ifdef _DEBUG
				DBGPRINT(L"OPENXR: Events Lost: %d\n", eventsLost->lostEventCount);
#endif
				break;
			}
			// Log that an instance loss is pending and shutdown the application.
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
			{
				XrEventDataInstanceLossPending* instanceLossPending =
					reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
#ifdef _DEBUG
				DBGPRINT(L"OPENXR: Instance Loss Pending at: %llu\n", instanceLossPending->lossTime);
#endif
				m_session_running = false;
				m_app_running = false;
				break;
			}
			// Log that the interaction profile has changed.
			case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
			{
				XrEventDataInteractionProfileChanged* interactionProfileChanged =
					reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
#ifdef _DEBUG
				DBGPRINT(L"OPENXR: Interaction Profile changed for Session: %llu\n", interactionProfileChanged->session);
#endif
				if (interactionProfileChanged->session != m_session)
				{
#ifdef _DEBUG
					DBGPRINT(L"XrEventDataInteractionProfileChanged for unknown Session\n");
#endif
					break;
				}
				RecordCurrentBindings();
				break;
			}
			// Log that there's a reference space change pending.
			case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
			{
				XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending =
					reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
#ifdef _DEBUG
				DBGPRINT(L"OPENXR: Reference Space Change pending for Session: %llu\n", referenceSpaceChangePending->session);
#endif
				if (referenceSpaceChangePending->session != m_session)
				{
#ifdef _DEBUG
					DBGPRINT(L"XrEventDataReferenceSpaceChangePending for unknown Session\n");
#endif
					break;
				}
				break;
			}
			// Session State changes:
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
			{
				XrEventDataSessionStateChanged* sessionStateChanged =
					reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
				if (sessionStateChanged->session != m_session)
				{
#ifdef _DEBUG
					DBGPRINT(L"XrEventDataSessionStateChanged for unknown Session\n");
#endif
					break;
				}

				if (sessionStateChanged->state == XR_SESSION_STATE_READY)
				{
					// SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
					XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
					sessionBeginInfo.primaryViewConfigurationType = m_view_config;
					xrBeginSession(m_session, &sessionBeginInfo);
					m_session_running = true;
				}
				if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING)
				{
					// SessionState is stopping. End the XrSession.
					xrEndSession(m_session);
					m_session_running = false;
				}
				if (sessionStateChanged->state == XR_SESSION_STATE_EXITING)
				{
					// SessionState is exiting. Exit the application.
					m_session_running = false;
					m_app_running = false;
				}
				if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING)
				{
					// SessionState is loss pending. Exit the application.
					// It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
					m_session_running = false;
					m_app_running = false;
				}
				// Store state for reference across the application.
				m_session_state = sessionStateChanged->state;
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void OpenXrRenderer::PollActions(XrTime predictedTime)
{
	XrResult result;
	// Update our action set with up-to-date input data.
	// First, we specify the actionSet we are polling.
	XrActiveActionSet activeActionSet{};
	activeActionSet.actionSet = m_act_set;
	activeActionSet.subactionPath = XR_NULL_PATH;
	// Now we sync the Actions to make sure they have current data.
	XrActionsSyncInfo actionsSyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
	actionsSyncInfo.countActiveActionSets = 1;
	actionsSyncInfo.activeActionSets = &activeActionSet;
	result = xrSyncActions(m_session, &actionsSyncInfo);
	if (result != XR_SUCCESS) return;

	XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	// We pose a single Action, twice - once for each subAction Path.
	actionStateGetInfo.action = m_pose_act;
	// For each hand, get the pose state if possible.
	for (int i = 0; i < 2; i++)
	{
		// Specify the subAction Path.
		actionStateGetInfo.subactionPath = m_hand_paths[i];
		result = xrGetActionStatePose(m_session, &actionStateGetInfo, &m_hand_pose_state[i]);
		if (result != XR_SUCCESS) return;

		if (m_hand_pose_state[i].isActive)
		{
			XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
			XrResult res = xrLocateSpace(m_hand_pose_space[i], m_space, predictedTime, &spaceLocation);
			if (XR_UNQUALIFIED_SUCCESS(res) &&
				(spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
				(spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
				m_hand_pose[i] = spaceLocation.pose;
			}
			else {
				m_hand_pose_state[i].isActive = false;
			}
		}
	}

	//grab
	for (int i = 0; i < 2; i++)
	{
		actionStateGetInfo.action = m_grab_act;
		actionStateGetInfo.subactionPath = m_hand_paths[i];
		result = xrGetActionStateFloat(m_session, &actionStateGetInfo, &m_grab_state[i]);
	}

	// joystick
	for (int i = 0; i < 2; i++)
	{
		actionStateGetInfo.action = m_js_act;
		actionStateGetInfo.subactionPath = m_hand_paths[i];
		result = xrGetActionStateVector2f(m_session, &actionStateGetInfo, &m_js_state[i]);
	}
}

bool OpenXrRenderer::CreateAction(XrAction &xrAction,
		const char *name, XrActionType xrActionType,
		std::vector<const char *> subaction_paths)
{
	XrActionCreateInfo actionCI{ XR_TYPE_ACTION_CREATE_INFO };
	// The type of action: float input, pose, haptic output etc.
	actionCI.actionType = xrActionType;
	// Subaction paths, e.g. left and right hand. To distinguish the same action performed on different devices.
	std::vector<XrPath> subaction_xrpaths;
	for (auto p : subaction_paths) {
		subaction_xrpaths.push_back(CreateXrPath(p));
	}
	actionCI.countSubactionPaths = (uint32_t)subaction_xrpaths.size();
	actionCI.subactionPaths = subaction_xrpaths.data();
	// The internal name the runtime uses for this Action.
	strncpy(actionCI.actionName, name, XR_MAX_ACTION_NAME_SIZE);
	// Localized names are required so there is a human-readable action name to show the user if they are rebinding the Action in an options screen.
	strncpy(actionCI.localizedActionName, name, XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
	XrResult result = xrCreateAction(m_act_set, &actionCI, &xrAction);
	if (result != XR_SUCCESS)
		return false;
	return true;
}

bool OpenXrRenderer::CreateActionSet()
{
	XrResult result;
	XrActionSetCreateInfo actionSetCI{ XR_TYPE_ACTION_SET_CREATE_INFO };
	// The internal name the runtime uses for this Action Set.
	strncpy(actionSetCI.actionSetName, "fluorender-actionset", XR_MAX_ACTION_SET_NAME_SIZE);
	// Localized names are required so there is a human-readable action name to show the user if they are rebinding Actions in an options screen.
	strncpy(actionSetCI.localizedActionSetName, "FluoRender ActionSet", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
	// Set a priority: this comes into play when we have multiple Action Sets, and determines which Action takes priority in binding to a specific input.
	actionSetCI.priority = 0;

	result = xrCreateActionSet(m_instance, &actionSetCI, &m_act_set);
	if (result != XR_SUCCESS) return false;

	// action for joistick pos
	CreateAction(m_js_act, "joystick", XR_ACTION_TYPE_VECTOR2F_INPUT, { "/user/hand/left", "/user/hand/right" });
	// action for grabbing
	CreateAction(m_grab_act, "grab", XR_ACTION_TYPE_FLOAT_INPUT, { "/user/hand/left", "/user/hand/right" });
	// An Action for the position of the palm of the user's hand - appropriate for the location of a grabbing Actions.
	CreateAction(m_pose_act, "palm-pose", XR_ACTION_TYPE_POSE_INPUT, { "/user/hand/left", "/user/hand/right" });
	// For later convenience we create the XrPaths for the subaction path names.
	m_hand_paths[0] = CreateXrPath("/user/hand/left");
	m_hand_paths[1] = CreateXrPath("/user/hand/right");

	return true;
}

bool OpenXrRenderer::SuggestBindings()
{
	auto SuggestBindings = [this](const char* profile_path, std::vector<XrActionSuggestedBinding> bindings) -> bool
	{
		// The application can call xrSuggestInteractionProfileBindings once per interaction profile that it supports.
		XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
		interactionProfileSuggestedBinding.interactionProfile = CreateXrPath(profile_path);
		interactionProfileSuggestedBinding.suggestedBindings = bindings.data();
		interactionProfileSuggestedBinding.countSuggestedBindings = (uint32_t)bindings.size();
		if (xrSuggestInteractionProfileBindings(m_instance, &interactionProfileSuggestedBinding) == XrResult::XR_SUCCESS)
			return true;
		//XR_TUT_LOG("Failed to suggest bindings with " << profile_path);
		return false;
	};

	bool any_ok = false;
	// Each Action here has two paths, one for each SubAction path.
	// generic controller
	any_ok |= SuggestBindings("/interaction_profiles/khr/simple_controller", {
		{m_grab_act, CreateXrPath("/user/hand/left/input/select/click")},
		{m_grab_act, CreateXrPath("/user/hand/right/input/select/click")},
		{m_pose_act, CreateXrPath("/user/hand/left/input/grip/pose")},
		{m_pose_act, CreateXrPath("/user/hand/right/input/grip/pose")},
		{m_js_act, CreateXrPath("/user/hand/left/input/thumbstick")},
		{m_js_act, CreateXrPath("/user/hand/right/input/thumbstick")} });
	// Meta
	any_ok |= SuggestBindings("/interaction_profiles/oculus/touch_controller", {
		{m_grab_act, CreateXrPath("/user/hand/left/input/squeeze/value")},
		{m_grab_act, CreateXrPath("/user/hand/right/input/squeeze/value")},
		{m_pose_act, CreateXrPath("/user/hand/left/input/grip/pose")},
		{m_pose_act, CreateXrPath("/user/hand/right/input/grip/pose")},
		{m_js_act, CreateXrPath("/user/hand/left/input/thumbstick")},
		{m_js_act, CreateXrPath("/user/hand/right/input/thumbstick")} });
	// Windows Mixed Reality
	any_ok |= SuggestBindings("/interaction_profiles/microsoft/motion_controller", {
		{m_grab_act, CreateXrPath("/user/hand/left/input/squeeze/value")},
		{m_grab_act, CreateXrPath("/user/hand/right/input/squeeze/value")},
		{m_pose_act, CreateXrPath("/user/hand/left/input/grip/pose")},
		{m_pose_act, CreateXrPath("/user/hand/right/input/grip/pose")},
		{m_js_act, CreateXrPath("/user/hand/left/input/thumbstick")},
		{m_js_act, CreateXrPath("/user/hand/right/input/thumbstick")} });
	// SteamVR
	any_ok |= SuggestBindings("/interaction_profiles/htc/vive_controller", {
		{m_grab_act, CreateXrPath("/user/hand/left/input/grip/value")},
		{m_grab_act, CreateXrPath("/user/hand/right/input/grip/value")},
		{m_pose_act, CreateXrPath("/user/hand/left/input/grip/pose")},
		{m_pose_act, CreateXrPath("/user/hand/right/input/grip/pose")},
		{m_js_act, CreateXrPath("/user/hand/left/input/thumbstick")},
		{m_js_act, CreateXrPath("/user/hand/right/input/thumbstick")} });
	if (!any_ok) return false;
	return true;
}

void OpenXrRenderer::RecordCurrentBindings()
{
	if (m_session)
	{
		XrResult result;
		// now we are ready to:
		XrInteractionProfileState interactionProfile = { XR_TYPE_INTERACTION_PROFILE_STATE, 0, 0 };
		// for each action, what is the binding?
		result = xrGetCurrentInteractionProfile(m_session, m_hand_paths[0], &interactionProfile);
		if (interactionProfile.interactionProfile)
		{
#ifdef _DEBUG
			DBGPRINT(L"user/hand/left ActiveProfile %s\n",
				s2ws(FromXrPath(interactionProfile.interactionProfile).c_str()));
#endif
		}
		result = xrGetCurrentInteractionProfile(m_session, m_hand_paths[1], &interactionProfile);
		if (interactionProfile.interactionProfile)
		{
#ifdef _DEBUG
			DBGPRINT(L"user/hand/right ActiveProfile %s\n",
				s2ws(FromXrPath(interactionProfile.interactionProfile).c_str()));
#endif
		}
	}
}

XrSpace OpenXrRenderer::CreateActionPoseSpace(XrAction action, const char* path)
{
	XrSpace xrSpace;
	const XrPosef xrPoseIdentity = { {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} };
	// Create frame of reference for a pose action
	XrActionSpaceCreateInfo actionSpaceCI{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
	actionSpaceCI.action = action;
	actionSpaceCI.poseInActionSpace = xrPoseIdentity;
	if (path)
		actionSpaceCI.subactionPath = CreateXrPath(path);
	XrResult result = xrCreateActionSpace(m_session, &actionSpaceCI, &xrSpace);
	return xrSpace;
}

void OpenXrRenderer::CreateActionPoses()
{
	m_hand_pose_space[0] = CreateActionPoseSpace(m_pose_act, "/user/hand/left");
	m_hand_pose_space[1] = CreateActionPoseSpace(m_pose_act, "/user/hand/right");
}

void OpenXrRenderer::AttachActionSet()
{
	// Attach the action set we just made to the session. We could attach multiple action sets!
	XrSessionActionSetsAttachInfo actionSetAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	actionSetAttachInfo.countActionSets = 1;
	actionSetAttachInfo.actionSets = &m_act_set;
	XrResult result = xrAttachSessionActionSets(m_session, &actionSetAttachInfo);
}

void OpenXrRenderer::DestroyActions()
{
	if (m_hand_pose_space[0] != XR_NULL_HANDLE)
	{
		xrDestroySpace(m_hand_pose_space[0]);
	}
	if (m_hand_pose_space[1] != XR_NULL_HANDLE)
	{
		xrDestroySpace(m_hand_pose_space[1]);
	}
	if (m_pose_act != XR_NULL_HANDLE)
	{
		xrDestroyAction(m_pose_act);
	}
	if (m_js_act != XR_NULL_HANDLE)
	{
		xrDestroyAction(m_js_act);
	}
	if (m_act_set != XR_NULL_HANDLE)
	{
		xrDestroyActionSet(m_act_set);
	}
}

void OpenXrRenderer::ApplyEyeOffsets(XrView* views, int eye_index)
{
	if (!views)
		return;
	if (eye_index < 0 || eye_index > 1)
		return;
	float eye_dist = glbin_settings.m_eye_dist;
	// Calculate the offset for each eye
	glm::vec3 offset;
	switch (eye_index)
	{
	case 0://left
		offset = glm::vec3(-eye_dist / 2.0f, 0.0f, 0.0f);
		break;
	case 1://right
		offset = glm::vec3(eye_dist / 2.0f, 0.0f, 0.0f);
		break;
	}

	// Adjust the eye pose
	glm::vec3 pos(
		views->pose.position.x,
		views->pose.position.y,
		views->pose.position.z);
	pos += offset;
	views->pose.position.x = pos.x;
	views->pose.position.y = pos.y;
	views->pose.position.z = pos.z;
}

// Function to convert XrPose to glm::mat4
glm::mat4 OpenXrRenderer::XrPoseToMat4(const XrPosef& pose)
{
	// Extract position
	glm::vec3 position(pose.position.x, pose.position.y, pose.position.z);

	// Extract orientation (quaternion)
	glm::quat orientation(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z);

	// Convert quaternion to rotation matrix
	glm::mat4 rotationMatrix = glm::mat4_cast(orientation);

	// Create transformation matrix
	glm::mat4 transformationMatrix = glm::translate(glm::mat4(1.0f), position) * rotationMatrix;

	return transformationMatrix;
}

XrBool32 OpenXRMessageCallbackFunction(
	XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
	XrDebugUtilsMessageTypeFlagsEXT messageType,
	const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	// Lambda to covert an XrDebugUtilsMessageSeverityFlagsEXT to std::string. Bitwise check to concatenate multiple severities to the output string.
	auto GetMessageSeverityString = [](XrDebugUtilsMessageSeverityFlagsEXT messageSeverity) -> std::string {
		bool separator = false;

		std::string msgFlags;
		if (BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)) {
			msgFlags += "VERBOSE";
			separator = true;
		}
		if (BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)) {
			if (separator) {
				msgFlags += ",";
			}
			msgFlags += "INFO";
			separator = true;
		}
		if (BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) {
			if (separator) {
				msgFlags += ",";
			}
			msgFlags += "WARN";
			separator = true;
		}
		if (BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
			if (separator) {
				msgFlags += ",";
			}
			msgFlags += "ERROR";
		}
		return msgFlags;
		};
	// Lambda to covert an XrDebugUtilsMessageTypeFlagsEXT to std::string. Bitwise check to concatenate multiple types to the output string.
	auto GetMessageTypeString = [](XrDebugUtilsMessageTypeFlagsEXT messageType) -> std::string {
		bool separator = false;

		std::string msgFlags;
		if (BitwiseCheck(messageType, XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)) {
			msgFlags += "GEN";
			separator = true;
		}
		if (BitwiseCheck(messageType, XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)) {
			if (separator) {
				msgFlags += ",";
			}
			msgFlags += "SPEC";
			separator = true;
		}
		if (BitwiseCheck(messageType, XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)) {
			if (separator) {
				msgFlags += ",";
			}
			msgFlags += "PERF";
		}
		return msgFlags;
		};

	// Collect message data.
	std::string functionName = (pCallbackData->functionName) ? pCallbackData->functionName : "";
	std::string messageSeverityStr = GetMessageSeverityString(messageSeverity);
	std::string messageTypeStr = GetMessageTypeString(messageType);
	std::string messageId = (pCallbackData->messageId) ? pCallbackData->messageId : "";
	std::string message = (pCallbackData->message) ? pCallbackData->message : "";

	// String stream final message.
	//std::stringstream errorMessage;
	//errorMessage << functionName << "(" << messageSeverityStr << " / " << messageTypeStr << "): msgNum: " << messageId << " - " << message;

	//// Log and debug break.
	//std::cerr << errorMessage.str() << std::endl;
	//if (BitwiseCheck(messageSeverity, XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
	//	DEBUG_BREAK;
	//}
	return XrBool32();
}
