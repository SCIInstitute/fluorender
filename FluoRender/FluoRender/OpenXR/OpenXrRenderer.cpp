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
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <vector>

OpenXrRenderer::OpenXrRenderer() :
	m_instance(0),
	m_session(0),
	m_space(0),
	m_act_set(0),
	m_act_left(0),
	m_act_right(0),
	m_swap_chain_left(0),
	m_swap_chain_right(0),
	m_size{0, 0},
	m_left_x(0.0f),
	m_left_y(0.0f),
	m_right_x(0.0f),
	m_right_y(0.0f),
	m_dead_zone(0.2f),
	m_scaler(20.0f)
{

}

OpenXrRenderer::~OpenXrRenderer()
{

}

bool OpenXrRenderer::Init()
{
#ifdef _WIN32
	// OpenXR initialization
	XrInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	std::strcpy(instanceCreateInfo.applicationInfo.applicationName, "FluoRender");
	instanceCreateInfo.applicationInfo.applicationVersion = 2;
	std::strcpy(instanceCreateInfo.applicationInfo.engineName, "FLRENDER");
	instanceCreateInfo.applicationInfo.engineVersion = 2;
	instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	xrCreateInstance(&instanceCreateInfo, &m_instance);

	XrSystemGetInfo systemGetInfo = {};
	systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	xrGetSystem(m_instance, &systemGetInfo, &m_sys_id);

	XrSessionCreateInfo sessionCreateInfo = {};
	sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.next = NULL;
	sessionCreateInfo.systemId = m_sys_id;
	xrCreateSession(m_instance, &sessionCreateInfo, &m_session);

	// Create an action set
	XrActionSetCreateInfo actionSetCreateInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };
	std::strcpy(actionSetCreateInfo.actionSetName, "fluo_act_set");
	std::strcpy(actionSetCreateInfo.localizedActionSetName, "FluoRender_act_set");
	actionSetCreateInfo.priority = 0;
	xrCreateActionSet(m_instance, &actionSetCreateInfo, &m_act_set);

	// Create actions for left and right hand controllers
	XrActionCreateInfo actionCreateInfo{ XR_TYPE_ACTION_CREATE_INFO };
	actionCreateInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
	std::strcpy(actionCreateInfo.actionName, "left_hand");
	std::strcpy(actionCreateInfo.localizedActionName, "Left Hand");
	xrCreateAction(m_act_set, &actionCreateInfo, &m_act_left);

	std::strcpy(actionCreateInfo.actionName, "right_hand");
	std::strcpy(actionCreateInfo.localizedActionName, "Right Hand");
	xrCreateAction(m_act_set, &actionCreateInfo, &m_act_right);

	// Suggest bindings for the actions
	XrPath interactionProfilePath;
	xrStringToPath(m_instance, "/interaction_profiles/khr/simple_controller", &interactionProfilePath);

	XrPath leftHandPath, rightHandPath;
	xrStringToPath(m_instance, "/user/hand/left", &leftHandPath);
	xrStringToPath(m_instance, "/user/hand/right", &rightHandPath);

	std::vector<XrActionSuggestedBinding> bindings =
	{
		{m_act_left, leftHandPath},
		{m_act_right, rightHandPath}
	};

	XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggestedBindings.interactionProfile = interactionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
	xrSuggestInteractionProfileBindings(m_instance, &suggestedBindings);

	// Attach the action set to the session
	XrSessionActionSetsAttachInfo attachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attachInfo.actionSets = &m_act_set;
	attachInfo.countActionSets = 1;
	xrAttachSessionActionSets(m_session, &attachInfo);

	// Create a reference space with the desired initial pose
	XrReferenceSpaceCreateInfo spaceCreateInfo = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;  // or XR_REFERENCE_SPACE_TYPE_LOCAL
	spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;  // No rotation
	spaceCreateInfo.poseInReferenceSpace.position.x = 0.0f;
	spaceCreateInfo.poseInReferenceSpace.position.y = 0.0f;
	spaceCreateInfo.poseInReferenceSpace.position.z = 0.0f;

	XrSpace space;
	xrCreateReferenceSpace(m_session, &spaceCreateInfo, &space);

	//frame state
	m_frame.type = XR_TYPE_FRAME_STATE;
	m_frame.predictedDisplayTime = 0;
	m_frame.predictedDisplayPeriod = 0;
	m_frame.shouldRender = XR_TRUE;

	// Get render size
	XrViewConfigurationView views[2];
	uint32_t viewCount = 0;
	xrEnumerateViewConfigurationViews(
		m_instance, m_sys_id,
		XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
		2, &viewCount, views);

	if (viewCount > 0)
	{
		m_size[0] = views[0].recommendedImageRectWidth;
		m_size[1] = views[0].recommendedImageRectHeight;
	}

	//swapchain
	XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
	swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.format = GL_RGBA32F; // Use the appropriate format for your images
	swapchainCreateInfo.sampleCount = 1;
	swapchainCreateInfo.width = m_size[0]; // Set the width of the swapchain
	swapchainCreateInfo.height = m_size[1]; // Set the height of the swapchain
	swapchainCreateInfo.faceCount = 1;
	swapchainCreateInfo.arraySize = 1;
	swapchainCreateInfo.mipCount = 1;

	xrCreateSwapchain(m_session, &swapchainCreateInfo, &m_swap_chain_left);
	xrCreateSwapchain(m_session, &swapchainCreateInfo, &m_swap_chain_right);

	//infos
	m_acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	m_wait_info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	m_wait_info.timeout = XR_INFINITE_DURATION;
	m_release_info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;

	//projection
	m_layer_proj.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;

	return true;
#else
	return false;
#endif
}

void OpenXrRenderer::Close()
{
#ifdef _WIN32
	//
	if (m_act_left != XR_NULL_HANDLE)
	{
		xrDestroyAction(m_act_left);
	}
	if (m_act_right != XR_NULL_HANDLE)
	{
		xrDestroyAction(m_act_right);
	}
	if (m_act_set != XR_NULL_HANDLE)
	{
		xrDestroyActionSet(m_act_set);
	}
	if (m_swap_chain_left != XR_NULL_HANDLE)
	{
		xrDestroySwapchain(m_swap_chain_left);
	}
	if (m_swap_chain_left != XR_NULL_HANDLE)
	{
		xrDestroySwapchain(m_swap_chain_left);
	}
	// End the session
	if (m_session != XR_NULL_HANDLE)
	{
		xrEndSession(m_session);
		xrDestroySession(m_session);
	}
	// Destroy the instance
	if (m_instance != XR_NULL_HANDLE)
	{
		xrDestroyInstance(m_instance);
	}
#endif
}

glm::mat4 OpenXrRenderer::GetProjectionMatrix(int eye_index, float near_clip, float far_clip)
{
	glm::mat4 proj_mat = glm::mat4(1.0f);

#ifdef _WIN32
	XrViewConfigurationType viewType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

	uint32_t viewCount = 0;
	xrEnumerateViewConfigurationViews(m_instance, m_sys_id, viewType, 0, &viewCount, nullptr);
	std::vector<XrViewConfigurationView> views(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	xrEnumerateViewConfigurationViews(m_instance, m_sys_id, viewType, viewCount, &viewCount, views.data());

	if (eye_index < viewCount)
	{
		XrViewConfigurationView view = views[eye_index];

		// Create the projection matrix for the specified eye
		XrFrameState frameState = { XR_TYPE_FRAME_STATE };
		xrWaitFrame(m_session, nullptr, &frameState);

		XrViewLocateInfo viewLocateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
		viewLocateInfo.viewConfigurationType = viewType;
		viewLocateInfo.displayTime = frameState.predictedDisplayTime;
		viewLocateInfo.space = XR_NULL_HANDLE;

		std::vector<XrView> xrViews(viewCount, { XR_TYPE_VIEW });
		xrLocateViews(m_session, &viewLocateInfo, nullptr, viewCount, &viewCount, xrViews.data());

		if (eye_index < xrViews.size())
		{
			XrFovf fov = xrViews[eye_index].fov;
			float tanLeft = tanf(fov.angleLeft);
			float tanRight = tanf(fov.angleRight);
			float tanUp = tanf(fov.angleUp);
			float tanDown = tanf(fov.angleDown);

			float tanWidth = tanRight - tanLeft;
			float tanHeight = tanUp - tanDown;

			proj_mat = glm::frustum(
				tanLeft * near_clip, tanRight * near_clip,
				tanDown * near_clip, tanUp * near_clip,
				near_clip, far_clip);
		}
	}
#endif

	return proj_mat;
}

glm::mat4 OpenXrRenderer::GetModelViewMatrix()
{
	glm::mat4 mv_mat = glm::mat4(1.0f);

#ifdef _WIN32
	XrFrameState frameState = { XR_TYPE_FRAME_STATE };
	xrWaitFrame(m_session, nullptr, &frameState);

	XrViewLocateInfo viewLocateInfo = { XR_TYPE_VIEW_LOCATE_INFO };
	viewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	viewLocateInfo.displayTime = frameState.predictedDisplayTime;
	viewLocateInfo.space = m_space;

	XrViewState viewState = { XR_TYPE_VIEW_STATE };
	uint32_t viewCountOutput;
	std::vector<XrView> views(2, { XR_TYPE_VIEW });
	xrLocateViews(m_session, &viewLocateInfo, &viewState, views.size(), &viewCountOutput, views.data());

	if (viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT &&
		viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT)
	{

		XrPosef pose = views[0].pose;
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

		mv_mat = glm::inverse(viewMatrix);
	}
#endif

	return mv_mat;
}

void OpenXrRenderer::GetControllerStates()
{
#ifdef _WIN32
	// Poll the state for the left hand thumbstick
	XrActionStateGetInfo getInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	getInfo.action = m_act_left;

	XrActionStateFloat stateFloatX{ XR_TYPE_ACTION_STATE_FLOAT };
	XrActionStateFloat stateFloatY{ XR_TYPE_ACTION_STATE_FLOAT };

	// Get the X coordinate
	getInfo.subactionPath = XR_NULL_PATH;
	xrGetActionStateFloat(m_session, &getInfo, &stateFloatX);

	// Get the Y coordinate
	getInfo.subactionPath = XR_NULL_PATH;
	xrGetActionStateFloat(m_session, &getInfo, &stateFloatY);

	if (stateFloatX.isActive && stateFloatY.isActive)
	{
		m_left_x = stateFloatX.currentState;
		m_left_y = stateFloatY.currentState;
	}

	// Poll the state for the right hand thumbstick
	getInfo.action = m_act_right;

	// Get the X coordinate
	getInfo.subactionPath = XR_NULL_PATH;
	xrGetActionStateFloat(m_session, &getInfo, &stateFloatX);

	// Get the Y coordinate
	getInfo.subactionPath = XR_NULL_PATH;
	xrGetActionStateFloat(m_session, &getInfo, &stateFloatY);

	if (stateFloatX.isActive && stateFloatY.isActive) {
		m_right_x = stateFloatX.currentState;
		m_right_y = stateFloatY.currentState;
	}
#endif

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
#ifdef _WIN32
	// Wait for the next frame
	xrWaitFrame(m_session, nullptr, &m_frame);

	// Begin the frame
	XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	xrBeginFrame(m_session, &frameBeginInfo);
#endif
}

void OpenXrRenderer::EndFrame()
{
#ifdef _WIN32
	// End the frame
	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = m_frame.predictedDisplayTime;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.layerCount = 0; // Add your layers here
	frameEndInfo.layers = nullptr;
	xrEndFrame(m_session, &frameEndInfo);
#endif
}

void OpenXrRenderer::DrawLeft(uint32_t left_buffer)
{
#ifdef _WIN32
	// Acquire, copy, and release for left eye
	uint32_t leftImageIndex;
	xrAcquireSwapchainImage(m_swap_chain_left, &m_acquire_info, &leftImageIndex);
	xrWaitSwapchainImage(m_swap_chain_left, &m_wait_info);
	//CopyFramebufferToSwapchainImage(leftFramebuffer, leftImageIndex, width, height);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, left_buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftImageIndex);
	glBlitFramebuffer(0, 0, m_size[0], m_size[1], 0, 0, m_size[0], m_size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	xrReleaseSwapchainImage(m_swap_chain_left, &m_release_info);
#endif
}

void OpenXrRenderer::DrawRight(uint32_t right_buffer)
{
#ifdef _WIN32
	// Acquire, copy, and release for right eye
	uint32_t rightImageIndex;
	xrAcquireSwapchainImage(m_swap_chain_right, &m_acquire_info, &rightImageIndex);
	xrWaitSwapchainImage(m_swap_chain_right, &m_wait_info);
	//CopyFramebufferToSwapchainImage(rightFramebuffer, rightImageIndex, width, height);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, right_buffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightImageIndex);
	glBlitFramebuffer(0, 0, m_size[0], m_size[1], 0, 0, m_size[0], m_size[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	xrReleaseSwapchainImage(m_swap_chain_right, &m_release_info);
#endif
}