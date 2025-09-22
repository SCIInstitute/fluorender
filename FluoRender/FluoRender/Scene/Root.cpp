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
#include <Root.h>
#include <RenderView.h>

Root::Root()
{
	type = 0;
}

Root::~Root()
{
	for (auto& view : m_views)
	{
		if (view)
		{
			view->ClearAll();
		}
	}
	m_views.clear();
}

int Root::GetViewNum()
{
	return static_cast<int>(m_views.size());
}

std::shared_ptr<RenderView> Root::GetView(int i)
{
	if (i >= 0 && i < (int)m_views.size())
		return m_views[i];
	else return nullptr;
}

std::shared_ptr<RenderView> Root::GetView(const std::wstring& name)
{
	for (auto& view : m_views)
	{
		if (view && view->GetName() == name)
			return view;
	}
	return nullptr;
}

int Root::GetView(RenderView* view)
{
	if (view)
	{
		for (size_t i = 0; i < m_views.size(); i++)
		{
			if (m_views[i].get() == view)
				return static_cast<int>(i);
		}
	}
	return -1;
}

std::shared_ptr<RenderView> Root::GetLastView()
{
	if (m_views.size())
		return m_views.back();
	else return nullptr;
}

void Root::AddView(const std::shared_ptr<RenderView>& view)
{
	if (view)
	{
		m_views.push_back(view);
	}
}

void Root::DeleteView(int i)
{
	if (i >= 0 && i < (int)m_views.size())
	{
		m_views[i]->ClearAll();
		m_views.erase(m_views.begin() + i);
	}
}

void Root::DeleteView(RenderView* view)
{
	if (view)
	{
		for (size_t i = 0; i < m_views.size(); i++)
		{
			if (m_views[i].get() == view)
			{
				m_views[i]->ClearAll();
				m_views.erase(m_views.begin() + i);
				break;
			}
		}
	}
}

void Root::DeleteView(const std::wstring& name)
{
	for (size_t i = 0; i < m_views.size(); i++)
	{
		if (m_views[i] && m_views[i]->GetName() == name)
		{
			m_views[i]->ClearAll();
			m_views.erase(m_views.begin() + i);
			break;
		}
	}
}

