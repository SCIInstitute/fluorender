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

#ifndef _CLIPPING_BOX_RENDERER_H_
#define _CLIPPING_BOX_RENDERER_H_

#include <BaseRenderer.h>
#include <ClippingBox.h>
#include <Color.h>
#include <Names.h>

namespace flvr
{
	class VertexArray;
	class ShaderProgram;
	class Framebuffer;
}
class RenderView;
namespace flrd
{
	enum class FaceWinding : int
	{
		Front,
		Back,
		Off
	};

	enum class ClippingRenderMode : int
	{
		None = 0,
		ColoredFront,
		ColoredBack,
		FrameAll,
		FrameFront,
		//FrameBack,
		TransFront,
		TransBack,
	};

	struct ClippingBoxSettings : public RendererSettings
	{
		ClippingRenderMode mode = ClippingRenderMode::ColoredFront;
		FaceWinding winding = FaceWinding::Front;
		uint8_t plane_mask = -1; // 6 bits, one per plane (XYZ min/max)
		bool draw_face = true;
		bool draw_border = true;
		std::weak_ptr<RenderView> view;
		fluo::Color color;
		double alpha = 0.3;
	};

	class ClippingBoxRenderer : public BaseRenderer {
	public:
		ClippingBoxRenderer() : settings_(std::make_shared<ClippingBoxSettings>()) {}

		void setData(const fluo::ClippingBox& box)
		{
			box_ = box;
		}

		void setSettings(const std::shared_ptr<RendererSettings>& settings) override {
			settings_ = std::dynamic_pointer_cast<ClippingBoxSettings>(settings);
		}

		std::shared_ptr<RendererSettings> getSettings() override
		{
			return settings_;
		}

		void render() override;

		std::string type() const override {
			return gstClippingBoxRenderer;
		}

	private:
		void drawPlane(int index,
			const std::shared_ptr<flvr::VertexArray>& va_clipp,
			const std::shared_ptr<flvr::ShaderProgram>& shader1,
			const std::shared_ptr<flvr::ShaderProgram>& shader2,
			const std::shared_ptr<flvr::Framebuffer>& data_buffer);

		fluo::Color getPlaneColor(int index);

		fluo::ClippingBox box_;
		std::shared_ptr<ClippingBoxSettings> settings_;
	};
}

#endif//_CLIPPING_BOX_RENDERER_H_