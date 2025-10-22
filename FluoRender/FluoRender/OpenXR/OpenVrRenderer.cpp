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

#include <OpenVrRenderer.h>
#include <Global.h>
#include <MainSettings.h>
#include <Framebuffer.h>
#include <openvr.h>
#include <array>

OpenVrRenderer::OpenVrRenderer() :
	BaseXrRenderer()
{
}

OpenVrRenderer::~OpenVrRenderer()
{

}

bool OpenVrRenderer::Init(void* v1, void* v2, uint64_t v3)
{
	if (m_initialized)
		return m_initialized;

	//openvr initilization
	vr::EVRInitError vr_error;
	m_vr_system = vr::VR_Init(&vr_error, vr::VRApplication_Scene, 0);
	if (vr_error == vr::VRInitError_None &&
		vr::VRCompositor())
	{
		//get render size
		m_vr_system->GetRecommendedRenderTargetSize(&m_size[0], &m_size[1]);
	}
	else
		return false;

	m_initialized = true;
	return m_initialized;
}

void OpenVrRenderer::Close()
{
	if (!m_initialized)
		return;

	vr::VR_Shutdown();
}

void OpenVrRenderer::GetControllerStates()
{
	//scan all controllers
	for (vr::TrackedDeviceIndex_t deviceIndex = 0;
		deviceIndex < vr::k_unMaxTrackedDeviceCount;
		++deviceIndex)
	{
		if (m_vr_system->IsTrackedDeviceConnected(deviceIndex))
		{
			vr::ETrackedDeviceClass deviceClass = m_vr_system->GetTrackedDeviceClass(deviceIndex);
			if (deviceClass == vr::TrackedDeviceClass_Controller)
			{
				vr::VRControllerState_t state;
				m_vr_system->GetControllerState(deviceIndex, &state, sizeof(state));

				if (m_vr_system->GetControllerRoleForTrackedDeviceIndex(deviceIndex) ==
					vr::TrackedControllerRole_LeftHand)
				{
					//left controller
					m_left_x = state.rAxis[0].x;
					m_left_y = state.rAxis[0].y;
				}
				else
				{
					//right controlelr
					m_right_x = state.rAxis[0].x;
					m_right_y = state.rAxis[0].y;
				}
			}
		}
	}

	if (m_left_x > -m_dead_zone && m_left_x < m_dead_zone) m_left_x = 0.0;
	if (m_left_y > -m_dead_zone && m_left_y < m_dead_zone) m_left_y = 0.0;
	if (m_right_x > -m_dead_zone && m_right_x < m_dead_zone) m_right_x = 0.0;
	if (m_right_y > -m_dead_zone && m_right_y < m_dead_zone) m_right_y = 0.0;
}

void OpenVrRenderer::BeginFrame()
{
	std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> tracked_device_poses;
	vr::VRCompositor()->WaitGetPoses(tracked_device_poses.data(), static_cast<uint32_t>(tracked_device_poses.size()), NULL, 0);

	//get projection matrix
	for (int eye_index = 0; eye_index < 2; ++eye_index)
	{
		vr::EVREye eye = eye_index ? vr::Eye_Right : vr::Eye_Left;
		auto proj_mat = m_vr_system->GetProjectionMatrix(eye, m_near_clip, m_far_clip);
		static int ti[] = { 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15 };
		for (int i = 0; i < 16; ++i)
			glm::value_ptr(m_proj_mat[eye_index])[i] =
			((float*)(proj_mat.m))[ti[i]];
	}

	//get tracking pose matrix
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	m_vr_system->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0,
		trackedDevicePose, vr::k_unMaxTrackedDeviceCount);
	vr::HmdMatrix34_t hmdMatrix = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;
	glm::mat4 modelViewMatrix = glm::mat4(
		hmdMatrix.m[0][0], hmdMatrix.m[1][0], hmdMatrix.m[2][0], 0.0f,
		hmdMatrix.m[0][1], hmdMatrix.m[1][1], hmdMatrix.m[2][1], 0.0f,
		hmdMatrix.m[0][2], hmdMatrix.m[1][2], hmdMatrix.m[2][2], 0.0f,
		hmdMatrix.m[0][3], hmdMatrix.m[1][3], hmdMatrix.m[2][3], 1.0f );
	for (int eye_index = 0; eye_index < 2; ++eye_index)
	{
		m_mv_mat[eye_index] = ApplyEyeOffsets(modelViewMatrix, eye_index);
		m_mv_mat[eye_index] = glm::inverse(m_mv_mat[eye_index]);
	}
}

void OpenVrRenderer::EndFrame()
{
}

void OpenVrRenderer::Draw(const std::vector<std::shared_ptr<flvr::Framebuffer>> &fbos)
{
	for (int eye_index = 0; eye_index < 2; ++eye_index)
	{
		if (fbos.size() <= eye_index)
			break;

		vr::Texture_t eye_tex = {};
		eye_tex.handle = reinterpret_cast<void*>(
			(unsigned long long)(fbos[eye_index]->tex_id(flvr::AttachmentPoint::Color(0))));
		eye_tex.eType = vr::TextureType_OpenGL;
		eye_tex.eColorSpace = vr::ColorSpace_Gamma;
		vr::EVREye eye = eye_index ? vr::Eye_Right : vr::Eye_Left;
		vr::VRCompositor()->Submit(eye, &eye_tex, nullptr);
	}
}

glm::mat4 OpenVrRenderer::ApplyEyeOffsets(const glm::mat4& mv, int eye_index)
{
	if (eye_index < 0 || eye_index > 1)
		return glm::mat4(1.0);
	float eye_dist = static_cast<float>(glbin_settings.m_eye_dist);
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

	// Apply the offsets to the view matrices
	glm::mat4 transl = glm::translate(glm::mat4(1.0f), offset);

	return mv * transl;
}