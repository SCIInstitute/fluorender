/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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
#ifndef RenderViewPanelAgent_h
#define RenderViewPanelAgent_h

#include <Agent.h>
#include <Names.h>
#include <memory>

class RenderViewPanel;
class RenderView;
class RenderCanvasAgent;

class RenderViewPanelAgent : public Agent
{
public:
	RenderViewPanelAgent(
		RenderViewPanel* panel,
		const std::shared_ptr<RenderView>& view);

	virtual ~RenderViewPanelAgent() = default;

	RenderViewPanel* GetPanel() const;

	std::shared_ptr<RenderView> GetView() const
	{
		return m_view.lock();
	}

	void SetRenderCanvasAgent(RenderCanvasAgent* agent)
	{
		m_canvas_agent = agent;
	}
	RenderCanvasAgent* GetRenderCanvasAgent()
	{
		return m_canvas_agent;
	}

	int GetId() const { return m_id; }
	int GetViewId();
	//reset counter
	static void ResetID();

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	std::weak_ptr<RenderView> m_view;

	RenderCanvasAgent* m_canvas_agent = nullptr;

	static int m_max_id;
	int m_id;
	double m_dpi_sf, m_dpi_sf2;

	bool m_bg_color_inv = false;
	//rot slider style
	bool m_rot_slider = false;
	int m_pin_by_user = 0;//override pin by scale: 0:by scale; 1:always pin; 2:always not pin
	bool m_pin_by_scale = false;
	//bit mask for items to save
	bool m_default_saved;

private:
	//update
	void SetChannelMixMode();
	void SetInfo();
	void SetDrawCamCtr();
	void SetLegend();
	void SetDrawColormap();
	void SetDrawScalebar();
	void SetScaleText();
	void SetScaleUnit();
	void Capture();
	void SetBgColor();
	void SetBgColorInvert();
	void SetAov(double val, bool notify);
	void SetProjection();
	void SetCamMode();
	void SetStereography();
	void SetHolography();
	void SetFullScreen();
	void CloseFullScreen();

	void SetDepthAttenEnable(bool val);
	void SetDepthAtten(double val, bool notify);

	void SetCenter();
	void SetScale121();
	void SetScaleFactor(double val);
	void SetScaleMode(int val);

	void SetRotLock(bool val);
	void SetSliderType();
	void SetRotations(const fluo::Vector& val, bool notify);
	void SetZeroRotations();

	void SaveDefault(unsigned int mask = 0xffffffff);
	void LoadSettings();

};

#endif // RenderViewPanelAgent_h
