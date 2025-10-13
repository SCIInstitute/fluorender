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

#ifndef OpenVrRenderer_h
#define OpenVrRenderer_h

#include <BaseXrRenderer.h>
#include <glm/gtc/type_ptr.hpp>

namespace flvr
{
	class Framebuffer;
}
namespace vr
{
	class IVRSystem;
}
class OpenVrRenderer : public BaseXrRenderer
{
public:
	OpenVrRenderer();
	virtual ~OpenVrRenderer();

	bool Init(void*, void*, uint64_t) override;
	void Close() override;

	void GetControllerStates() override;

	void BeginFrame() override;
	void EndFrame() override;
	void Draw(const std::vector<std::shared_ptr<flvr::Framebuffer>> &fbos) override;

private:
	vr::IVRSystem* m_vr_system;
	glm::mat4 ApplyEyeOffsets(const glm::mat4& mv, int eye_index);
};
#endif//OpenVrRenderer_h