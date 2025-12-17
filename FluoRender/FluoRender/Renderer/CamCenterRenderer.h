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

#ifndef _CAM_CENTER_RENDERER_H_
#define _CAM_CENTER_RENDERER_H_

#include <BaseRenderer.h>
#include <Names.h>

class RenderView;
namespace flvr
{
	class VertexArray;
	class ShaderProgram;
}
namespace flrd
{
	enum class CamCenterStyle : int
	{
		CenterJack = 0,
		CornerJack,
		Crosshair
	};
	struct CamCenterSettings : public RendererSettings
	{
		std::weak_ptr<RenderView> view;
		CamCenterStyle style;
		double size;
	};
	class CamCenterRenderer : public BaseRenderer
	{
	public:
		CamCenterRenderer() : settings_(std::make_shared<CamCenterSettings>()) {}

		void setSettings(const std::shared_ptr<RendererSettings>& settings) override {
			settings_ = std::dynamic_pointer_cast<CamCenterSettings>(settings);
		}

		std::shared_ptr<RendererSettings> getSettings() override
		{
			return settings_;
		}

		void render() override;

		std::string type() const override {
			return gstCamCenterRenderer;
		}

	private:
		std::shared_ptr<CamCenterSettings> settings_;

	private:
		void drawCrosshair(const std::shared_ptr<flvr::VertexArray>& va, const std::shared_ptr<flvr::ShaderProgram>& shader);
		void drawJack(const std::shared_ptr<flvr::VertexArray>& va, const std::shared_ptr<flvr::ShaderProgram>& shader);
	};
}

#endif//_CAM_CENTER_RENDERER_H_