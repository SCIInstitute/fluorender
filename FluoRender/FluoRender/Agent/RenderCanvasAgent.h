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
#ifndef RenderCanvasAgent_h
#define RenderCanvasAgent_h

#include <Agent.h>

#include <memory>

class RenderCanvas;
class RenderView;

struct UpdateRequest;

class RenderCanvasAgent : public Agent
{
public:
	RenderCanvasAgent(
		RenderCanvas* canvas,
		const std::shared_ptr<RenderView>& view);

	virtual ~RenderCanvasAgent() = default;

	// Agent interface
	virtual bool Accept(
		const UpdateRequest& request) const override;

	virtual void Update(
		const UpdateRequest& request) override;

	void RequestDraw();

	void PerformDraw();

	RenderCanvas* GetCanvas() const
	{
		return static_cast<RenderCanvas*>(GetWindow());
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
	RenderCanvasAgent* GetRenderSender(
		const UpdateRequest& request) const;

	void SyncRotations(
		RenderCanvasAgent* sender);

	void SyncCamera(
		RenderCanvasAgent* sender);

private:
	std::weak_ptr<RenderView> m_view;

	bool draw_pending_ = false;
};

#endif // RenderCanvasAgent_h
