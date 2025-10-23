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
#ifndef RenderScheduler_h
#define RenderScheduler_h

#include <memory>
#include <string>

class RenderCanvas;
class RenderView;

class RenderScheduler
{
public:
	RenderScheduler(RenderCanvas* canvas, std::shared_ptr<RenderView>& view);

	void requestDraw(const std::string& reason); // Called by canvas, view, or external modules
	void performDraw();                          // Called by canvas during paint event

	RenderCanvas* getCanvas() const;
	std::shared_ptr<RenderView> getView() const;

private:
	RenderCanvas* canvas_;
	std::weak_ptr<RenderView> view_;
	bool draw_pending_;
	std::string last_reason_;
};

class RenderSchedulerManager
{
public:
	void registerScheduler(std::shared_ptr<RenderScheduler> scheduler);
	std::shared_ptr<RenderScheduler> getScheduler(RenderCanvas* canvas);
	void requestGlobalDraw(const std::string& reason); // Optional: broadcast draw
	void removeScheduler(RenderCanvas* canvas);

private:
	std::vector<std::shared_ptr<RenderScheduler>> schedulers_;
};

#endif//RenderScheduler_h
