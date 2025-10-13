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

#ifndef BaseXrRenderer_h
#define BaseXrRenderer_h

#include <glm/glm.hpp>
#include <vector>
#include <memory>

template <typename T>
inline bool BitwiseCheck(const T& value, const T& checkValue) {
	return ((value & checkValue) == checkValue);
}

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

namespace flvr
{
	class Framebuffer;
}
class BaseXrRenderer
{
public:
	virtual ~BaseXrRenderer();

	virtual bool Init(void*, void*, uint64_t) = 0;
	virtual void Close() = 0;

	virtual uint32_t GetSize(int i)
	{
		return m_size[i];
	}

	virtual glm::mat4 GetProjectionMatrix(int eye_index);
	virtual glm::mat4 GetModelViewMatrix(int eye_index);
	virtual glm::mat4 GetGrabMatrix();
	virtual bool GetGrab() { return m_grab[0] || m_grab[1]; }

	virtual void GetControllerStates() = 0;
	virtual float GetControllerLeftThumbstickX() { return m_left_x * m_scaler; }
	virtual float GetControllerLeftThumbstickY() { return m_left_y * m_scaler; }
	virtual float GetControllerRightThumbstickX() { return m_right_x * m_scaler; }
	virtual float GetControllerRightThumbstickY() { return m_right_y * m_scaler; }

	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void Draw(const std::vector<std::shared_ptr<flvr::Framebuffer>> &fbos) = 0;

	virtual void SetClips(float near_clip, float far_clip)
	{
		m_near_clip = near_clip;
		m_far_clip = far_clip;
	}

protected:
	bool m_initialized = false;
	uint32_t m_size[2] = { 0, 0 };
	float m_left_x = 0.0f;
	float m_left_y = 0.0f;
	float m_right_x = 0.0f;
	float m_right_y = 0.0f;
	float m_dead_zone = 0.1f;
	float m_scaler = 20.0f;
	bool m_grab_prev[2] = { false, false };
	bool m_grab[2] = { false, false };

	float m_near_clip = 0.1f;
	float m_far_clip = 1000.0f;
	glm::mat4 m_proj_mat[2] = { glm::mat4(1.0f), glm::mat4(1.0f) };
	glm::mat4 m_mv_mat[2] = { glm::mat4(1.0f), glm::mat4(1.0f) };
	glm::mat4 m_grab_mat = glm::mat4(1.0f);

private:
};

#endif//BaseXrRenderer_h