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
#include <string>
#include <fstream>
#ifdef _DEBUG
#include <Debug.h>
#endif

HololensRenderer::HololensRenderer() :
	OpenXrRenderer()
{
	m_app_name = "FluoRender";
	m_eng_name = "FLHOLOLENS";
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

	SetExtensions();

	// OpenXR initialization
	if (!CreateInstance())
		return false;

	//load msft specific functions
	LoadFunctions();

#ifdef _DEBUG
	CreateDebugMessenger();
	GetInstanceProperties();
#endif

	if (!GetSystemID())
		return false;

	CreateActionSet();
	SuggestBindings();

	InitDevice();

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

void HololensRenderer::SetExtensions()
{
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
}

bool HololensRenderer::CreateSession(void* hdc, void* hdxrc)
{
	XrResult result;
	// Create an XrSessionCreateInfo structure.
	XrSessionCreateInfo sessionCI{ XR_TYPE_SESSION_CREATE_INFO };
	// Create a std::unique_ptr<GraphicsAPI_...> from the instance and system.
	// This call sets up a graphics API that's suitable for use with OpenXR.
	// Get D3D11 graphics requirements
	PFN_xrGetD3D11GraphicsRequirementsKHR xrGetD3D11GraphicsRequirementsKHR = nullptr;
	result = xrGetInstanceProcAddr(m_instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&xrGetD3D11GraphicsRequirementsKHR);
	if (result != XR_SUCCESS) return false;
	XrGraphicsRequirementsD3D11KHR requirements = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	result = xrGetD3D11GraphicsRequirementsKHR(m_instance, m_sys_id, &requirements);
	if (result != XR_SUCCESS) return false;
	XrGraphicsBindingD3D11KHR graphicsBindingD3D11 = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
	//graphicsBindingOpenGL.hDC = static_cast<HDC>(hdc);
	//graphicsBindingOpenGL.hGLRC = static_cast<HGLRC>(hglrc);

	// Fill out the XrSessionCreateInfo structure and create an XrSession.
	sessionCI.next = (void*)&graphicsBindingD3D11;
	sessionCI.createFlags = 0;
	sessionCI.systemId = m_sys_id;
	result = xrCreateSession(m_instance, &sessionCI, &m_session);
	if (result != XR_SUCCESS) return false;

	// If remoting speech extension is enabled
	if (m_usingRemotingRuntime)
	{
		XrRemotingSpeechInitInfoMSFT speechInitInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_SPEECH_INIT_INFO_MSFT) };
		InitializeSpeechRecognition(speechInitInfo);
		xrInitializeRemotingSpeechMSFT(m_session, &speechInitInfo);
	}

	return true;
}

void HololensRenderer::LoadFunctions()
{
	XrResult result;
	// Get the function address
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingSetContextPropertiesMSFT", (PFN_xrVoidFunction*)&xrRemotingSetContextPropertiesMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingConnectMSFT", (PFN_xrVoidFunction*)&xrRemotingConnectMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingListenMSFT", (PFN_xrVoidFunction*)&xrRemotingListenMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingDisconnectMSFT", (PFN_xrVoidFunction*)&xrRemotingDisconnectMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingGetConnectionStateMSFT", (PFN_xrVoidFunction*)&xrRemotingGetConnectionStateMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingSetSecureConnectionClientCallbacksMSFT", (PFN_xrVoidFunction*)&xrRemotingSetSecureConnectionClientCallbacksMSFT);
	result = xrGetInstanceProcAddr(m_instance, "xrRemotingSetSecureConnectionServerCallbacksMSFT", (PFN_xrVoidFunction*)&xrRemotingSetSecureConnectionServerCallbacksMSFT);

	result = xrGetInstanceProcAddr(m_instance, "xrInitializeRemotingSpeechMSFT", (PFN_xrVoidFunction*)&xrInitializeRemotingSpeechMSFT);
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

void HololensRenderer::InitDevice()
{

}

void HololensRenderer::Disconnect()
{
	XrRemotingDisconnectInfoMSFT disconnectInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_DISCONNECT_INFO_MSFT) };
	xrRemotingDisconnectMSFT(m_instance, m_sys_id, &disconnectInfo);
}

void HololensRenderer::ConnectOrListen()
{
	if (!m_usingRemotingRuntime)
	{
		return;
	}

	XrRemotingConnectionStateMSFT connectionState;
	xrRemotingGetConnectionStateMSFT(m_instance, m_sys_id, &connectionState, nullptr);
	if (connectionState != XR_REMOTING_CONNECTION_STATE_DISCONNECTED_MSFT)
	{
		return;
	}

	// Apply remote context properties while disconnected.
	{
		XrRemotingRemoteContextPropertiesMSFT contextProperties;
		contextProperties =
			XrRemotingRemoteContextPropertiesMSFT{ static_cast<XrStructureType>(XR_TYPE_REMOTING_REMOTE_CONTEXT_PROPERTIES_MSFT) };
		contextProperties.enableAudio = false;
		contextProperties.maxBitrateKbps = 20000;
		contextProperties.videoCodec = XR_REMOTING_VIDEO_CODEC_H265_MSFT;
		contextProperties.depthBufferStreamResolution = XR_REMOTING_DEPTH_BUFFER_STREAM_RESOLUTION_HALF_MSFT;
		xrRemotingSetContextPropertiesMSFT(m_instance, m_sys_id, &contextProperties);
	}

	if (m_options.listen)
	{
		if (m_options.secureConnection)
		{
			XrRemotingSecureConnectionServerCallbacksMSFT serverCallbacks;
			serverCallbacks.context = this;
			serverCallbacks.requestServerCertificateCallback = CertificateValidationCallbackStatic;
			serverCallbacks.validateAuthenticationTokenCallback = AuthenticationValidationCallbackStatic;
			serverCallbacks.authenticationRealm = m_options.authenticationRealm.c_str();
			xrRemotingSetSecureConnectionServerCallbacksMSFT(m_instance, m_sys_id, &serverCallbacks);
		}

		XrRemotingListenInfoMSFT listenInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_LISTEN_INFO_MSFT) };
		listenInfo.listenInterface = m_options.host.empty() ? "0.0.0.0" : m_options.host.c_str();
		listenInfo.handshakeListenPort = m_options.port != 0 ? m_options.port : 8265;
		listenInfo.transportListenPort = m_options.transportPort != 0 ? m_options.transportPort : 8266;
		listenInfo.secureConnection = m_options.secureConnection;
		xrRemotingListenMSFT(m_instance, m_sys_id, &listenInfo);
	}
	else
	{
		if (m_options.secureConnection)
		{
			XrRemotingSecureConnectionClientCallbacksMSFT clientCallbacks;
			clientCallbacks.context = this;
			clientCallbacks.requestAuthenticationTokenCallback = AuthenticationRequestCallbackStatic;
			clientCallbacks.validateServerCertificateCallback = CertificateValidationCallbackStatic;
			clientCallbacks.performSystemValidation = true;
			xrRemotingSetSecureConnectionClientCallbacksMSFT(m_instance, m_sys_id, &clientCallbacks);
		}

		XrRemotingConnectInfoMSFT connectInfo{ static_cast<XrStructureType>(XR_TYPE_REMOTING_CONNECT_INFO_MSFT) };
		connectInfo.remoteHostName = m_options.host.empty() ? "127.0.0.1" : m_options.host.c_str();
		connectInfo.remotePort = m_options.port != 0 ? m_options.port : 8265;
		connectInfo.secureConnection = m_options.secureConnection;
		xrRemotingConnectMSFT(m_instance, m_sys_id, &connectInfo);
	}
}

XrResult HololensRenderer::AuthenticationRequestCallback(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest)
{
	const std::string tokenUtf8 = m_options.authenticationToken;
	const uint32_t tokenSize = static_cast<uint32_t>(tokenUtf8.size() + 1); // for null-termination
	if (authenticationTokenRequest->tokenCapacityIn >= tokenSize)
	{
		memcpy(authenticationTokenRequest->tokenBuffer, tokenUtf8.c_str(), tokenSize);
		authenticationTokenRequest->tokenSizeOut = tokenSize;
		return XR_SUCCESS;
	}
	else
	{
		authenticationTokenRequest->tokenSizeOut = tokenSize;
		return XR_ERROR_SIZE_INSUFFICIENT;
	}
}

XrResult XRAPI_CALL
HololensRenderer::AuthenticationRequestCallbackStatic(XrRemotingAuthenticationTokenRequestMSFT* authenticationTokenRequest)
{
	if (!authenticationTokenRequest->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(authenticationTokenRequest->context)
		->AuthenticationRequestCallback(authenticationTokenRequest);
}

XrResult HololensRenderer::AuthenticationValidationCallback(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation)
{
	const std::string tokenUtf8 = m_options.authenticationToken;
	authenticationTokenValidation->tokenValidOut =
		(authenticationTokenValidation->token != nullptr && tokenUtf8 == authenticationTokenValidation->token);
	return XR_SUCCESS;
}

XrResult XRAPI_CALL
HololensRenderer::AuthenticationValidationCallbackStatic(XrRemotingAuthenticationTokenValidationMSFT* authenticationTokenValidation)
{
	if (!authenticationTokenValidation->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(authenticationTokenValidation->context)
		->AuthenticationValidationCallback(authenticationTokenValidation);
}

XrResult HololensRenderer::CertificateRequestCallback(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest)
{
	const std::string subjectNameUtf8 = m_options.subjectName;
	const std::string passPhraseUtf8 = m_options.keyPassphrase;

	const uint32_t certStoreSize = static_cast<uint32_t>(m_certificateStore.size());
	const uint32_t subjectNameSize = static_cast<uint32_t>(subjectNameUtf8.size() + 1); // for null-termination
	const uint32_t passPhraseSize = static_cast<uint32_t>(passPhraseUtf8.size() + 1);   // for null-termination

	serverCertificateRequest->certStoreSizeOut = certStoreSize;
	serverCertificateRequest->subjectNameSizeOut = subjectNameSize;
	serverCertificateRequest->keyPassphraseSizeOut = passPhraseSize;
	if (serverCertificateRequest->certStoreCapacityIn < certStoreSize ||
		serverCertificateRequest->subjectNameCapacityIn < subjectNameSize ||
		serverCertificateRequest->keyPassphraseCapacityIn < passPhraseSize)
	{
		return XR_ERROR_SIZE_INSUFFICIENT;
	}

	// All buffers have sufficient size, so fill in the data
	memcpy(serverCertificateRequest->certStoreBuffer, m_certificateStore.data(), certStoreSize);
	memcpy(serverCertificateRequest->subjectNameBuffer, subjectNameUtf8.c_str(), subjectNameSize);
	memcpy(serverCertificateRequest->keyPassphraseBuffer, passPhraseUtf8.c_str(), passPhraseSize);

	return XR_SUCCESS;
}

XrResult XRAPI_CALL HololensRenderer::CertificateValidationCallbackStatic(XrRemotingServerCertificateRequestMSFT* serverCertificateRequest)
{
	if (!serverCertificateRequest->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(serverCertificateRequest->context)
		->CertificateRequestCallback(serverCertificateRequest);
}

XrResult HololensRenderer::CertificateValidationCallback(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation)
{
	if (!serverCertificateValidation->systemValidationResult)
	{
		return XR_ERROR_RUNTIME_FAILURE; // We requested system validation to be performed
	}

	serverCertificateValidation->validationResultOut = *serverCertificateValidation->systemValidationResult;
	if (m_options.allowCertificateNameMismatch && serverCertificateValidation->validationResultOut.nameValidationResult ==
		XR_REMOTING_CERTIFICATE_NAME_VALIDATION_RESULT_MISMATCH_MSFT)
	{
		serverCertificateValidation->validationResultOut.nameValidationResult =
			XR_REMOTING_CERTIFICATE_NAME_VALIDATION_RESULT_MATCH_MSFT;
	}
	if (m_options.allowUnverifiedCertificateChain)
	{
		serverCertificateValidation->validationResultOut.trustedRoot = true;
	}

	return XR_SUCCESS;
}

XrResult XRAPI_CALL
HololensRenderer::CertificateValidationCallbackStatic(XrRemotingServerCertificateValidationMSFT* serverCertificateValidation)
{
	if (!serverCertificateValidation->context)
	{
		return XR_ERROR_RUNTIME_FAILURE;
	}

	return reinterpret_cast<HololensRenderer*>(serverCertificateValidation->context)
		->CertificateValidationCallback(serverCertificateValidation);
}

void HololensRenderer::InitializeSpeechRecognition(XrRemotingSpeechInitInfoMSFT& speechInitInfo)
{
	// Specify the speech recognition language.
	strcpy_s(speechInitInfo.language, "en-US");

	// Initialize the dictionary.
	m_dictionaryEntries = { "Red", "Blue", "Green", "Aquamarine", "Default" };
	speechInitInfo.dictionaryEntries = m_dictionaryEntries.data();
	speechInitInfo.dictionaryEntriesCount = static_cast<uint32_t>(m_dictionaryEntries.size());

	// Initialize the grammar file if it exists.
	if (LoadGrammarFile(m_grammarFileContent))
	{
		speechInitInfo.grammarFileSize = static_cast<uint32_t>(m_grammarFileContent.size());
		speechInitInfo.grammarFileContent = m_grammarFileContent.data();
	}
}

bool HololensRenderer::LoadGrammarFile(std::vector<uint8_t>& grammarFileContent)
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	char executablePath[MAX_PATH];
	if (GetModuleFileNameA(NULL, executablePath, ARRAYSIZE(executablePath)) == 0)
	{
		return false;
	}

	std::filesystem::path filename(executablePath);
	filename = filename.replace_filename("OpenXRSpeechGrammar.xml");

	if (!std::filesystem::exists(filename))
	{
		return false;
	}

	std::string grammarFilePath{ filename.string() };
	std::ifstream grammarFileStream(grammarFilePath, std::ios::binary);
	const size_t grammarFileSize = std::filesystem::file_size(filename);
	if (!grammarFileStream.good() || grammarFileSize == 0)
	{
		return false;
	}

	grammarFileContent.resize(grammarFileSize);
	grammarFileStream.read(reinterpret_cast<char*>(grammarFileContent.data()), grammarFileSize);
	if (grammarFileStream.fail())
	{
		return false;
	}

	return true;
#else
	return false;
#endif
}
