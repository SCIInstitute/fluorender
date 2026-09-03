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
#include <memory>

class RenderViewPanel;
class RenderView;

class RenderViewPanelAgent : public Agent
{
public:
	RenderViewPanelAgent(
		RenderViewPanel* panel,
		const std::shared_ptr<RenderView>& view);

	virtual ~RenderViewPanelAgent() = default;

	// Agent interface
	virtual bool Accept(
		const UpdateRequest& request) const override;

	virtual void Update(
		const UpdateRequest& request) override;

	RenderViewPanel* GetPanel() const
	{
		return static_cast<RenderViewPanel*>(GetWindow());
	}

	void SetView(const std::shared_ptr<RenderView>& view)
	{
		m_view = view;
	}

	std::shared_ptr<RenderView> GetView() const
	{
		return m_view.lock();
	}

private:
	void UpdateUI(const UpdateRequest& request);

	void UpdateData(const UpdateRequest& request);

private:
	std::weak_ptr<RenderView> m_view;

	bool m_bg_color_inv = false;
	//rot slider style
	bool m_rot_slider = false;
	int m_pin_by_user = 0;//override pin by scale: 0:by scale; 1:always pin; 2:always not pin
	bool m_pin_by_scale = false;

};

#endif // RenderViewPanelAgent_h
