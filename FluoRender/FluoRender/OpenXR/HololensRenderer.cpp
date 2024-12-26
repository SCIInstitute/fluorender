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
#include <HololensRenderer.h>
#include <Global.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <vector>
#include <algorithm>
#include <filesystem>
#ifdef _DEBUG
#include <Debug.h>
#endif

HololensRenderer::HololensRenderer() :
	OpenXrRenderer()
{
}

HololensRenderer::~HololensRenderer()
{

}

bool HololensRenderer::Init(void* hdc, void* hglrc)
{
	if (m_initialized)
		return m_initialized;

	if (!m_options.isStandalone)
	{
		m_usingRemotingRuntime = EnableRemotingXR();

		if (m_usingRemotingRuntime)
		{
			PrepareRemotingEnvironment();
		}
		else
		{
#ifdef _DEBUG
			DBGPRINT(L"RemotingXR runtime not available. Running with default OpenXR runtime.\n");
#endif
		}
	}
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

void HololensRenderer::Close()
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

void HololensRenderer::GetControllerStates()
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

void HololensRenderer::BeginFrame()
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

void HololensRenderer::EndFrame()
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

void HololensRenderer::Draw(const std::vector<flvr::Framebuffer*> &fbos)
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

bool HololensRenderer::EnableRemotingXR()
{
	wchar_t executablePath[MAX_PATH];
	if (GetModuleFileNameW(NULL, executablePath, ARRAYSIZE(executablePath)) == 0)
	{
		return false;
	}

	std::filesystem::path filename(executablePath);
	filename = filename.replace_filename("RemotingXR.json");

	if (std::filesystem::exists(filename))
	{
		SetEnvironmentVariableW(L"XR_RUNTIME_JSON", filename.c_str());
		return true;
	}

	return false;
}

void HololensRenderer::PrepareRemotingEnvironment()
{
	if (!m_options.secureConnection) {
		return;
	}

	if (m_options.authenticationToken.empty()) {
		throw std::logic_error("Authentication token must be specified for secure connections.");
	}

	if (m_options.listen) {
		if (m_options.certificateStore.empty() || m_options.subjectName.empty()) {
			throw std::logic_error("Certificate store and subject name must be specified for secure listening.");
		}

		constexpr size_t maxCertStoreSize = 1 << 20;
		std::ifstream certStoreStream(m_options.certificateStore, std::ios::binary);
		certStoreStream.seekg(0, std::ios_base::end);
		const size_t certStoreSize = certStoreStream.tellg();
		if (!certStoreStream.good() || certStoreSize == 0 || certStoreSize > maxCertStoreSize) {
			throw std::logic_error("Error reading certificate store.");
		}
		certStoreStream.seekg(0, std::ios_base::beg);
		m_certificateStore.resize(certStoreSize);
		certStoreStream.read(reinterpret_cast<char*>(m_certificateStore.data()), certStoreSize);
		if (certStoreStream.fail()) {
			throw std::logic_error("Error reading certificate store.");
		}
	}
}

bool HololensRenderer::CreateInstance()
{
	XrResult result;
	// Define application information
	// The application/engine name and version are user-definied.
	// These may help IHVs or runtimes.
	XrApplicationInfo appInfo = {};
	strncpy(appInfo.applicationName, "FluoRender", XR_MAX_APPLICATION_NAME_SIZE);
	appInfo.applicationVersion = 1;
	strncpy(appInfo.engineName, "FLHOLOLENS", XR_MAX_ENGINE_NAME_SIZE);
	appInfo.engineVersion = 1;
	//there is no way to know which version is supported before creating the instance
	//so, use the lowest version number for now
	appInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0); //XR_CURRENT_API_VERSION;

	// Add additional instance layers/extensions that the application wants.
	// Add both required and requested instance extensions.
	m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
	// D3D11 extension is required for this sample, so check if it's supported.
	m_instanceExtensions.push_back(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
	if (m_usingRemotingRuntime)
	{
		// If using the remoting runtime, the remoting extension must be present as well
		m_instanceExtensions.push_back(XR_MSFT_HOLOGRAPHIC_REMOTING_EXTENSION_NAME);
		m_instanceExtensions.push_back(XR_MSFT_HOLOGRAPHIC_REMOTING_FRAME_MIRRORING_EXTENSION_NAME);
		m_instanceExtensions.push_back(XR_MSFT_HOLOGRAPHIC_REMOTING_SPEECH_EXTENSION_NAME);
	}
	// Additional optional extensions for enhanced functionality. Track whether enabled in m_optionalExtensions.
	m_instanceExtensions.push_back(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
	m_instanceExtensions.push_back(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
	m_instanceExtensions.push_back(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);

	// Get all the API Layers from the OpenXR runtime.
	uint32_t apiLayerCount = 0;
	std::vector<XrApiLayerProperties> apiLayerProperties;
	result = xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr);
	if (result != XR_SUCCESS) return false;
	apiLayerProperties.resize(apiLayerCount, { XR_TYPE_API_LAYER_PROPERTIES });
	result = xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data());
	if (result != XR_SUCCESS) return false;

	// Check the requested API layers against the ones from the OpenXR.
	// If found add it to the Active API Layers.
	std::vector<std::string> apiLayers = {};
	std::vector<const char*> activeAPILayers = {};
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
	extensionProperties.resize(extensionCount, { XR_TYPE_EXTENSION_PROPERTIES });
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
	XrInstanceCreateInfo instanceCI{ XR_TYPE_INSTANCE_CREATE_INFO };
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
