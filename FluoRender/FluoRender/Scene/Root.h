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
#ifndef _ROOT_H_
#define _ROOT_H_

#include <TreeLayer.h>
#include <vector>
#include <memory>

class RenderView;
class Root : public TreeLayer
{
public:
	Root();
	~Root();

	//view functions
	int GetViewNum();
	std::shared_ptr<RenderView> GetView(int i);
	std::shared_ptr<RenderView> GetViewById(int id);
	std::shared_ptr<RenderView> GetView(const std::wstring& name);
	int GetView(RenderView* view);
	std::shared_ptr<RenderView> GetLastView();
	void AddView(const std::shared_ptr<RenderView>& view);
	void DeleteView(int i);
	void DeleteView(RenderView* view);
	void DeleteView(const std::wstring& name);

private:
	std::vector<std::shared_ptr<RenderView>> m_views;
};

#endif//_ROOT_H_