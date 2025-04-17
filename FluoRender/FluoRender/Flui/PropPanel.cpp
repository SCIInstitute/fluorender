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
#include <PropPanel.h>
#include <Global.h>
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>
#include <RenderView.h>
#include <DataManager.h>

PropBase::PropBase(MainFrame* frame):
	m_frame(frame)
{
}

void PropBase::FluoRefresh(int excl_self,
	const fluo::ValueCollection& vc, const std::set<int>& views)
{
	if (!m_frame)
		return;
	int view_excl = 0;
	if (dynamic_cast<RenderViewPanel*>(this))
		view_excl = excl_self;
	m_frame->RefreshCanvases(views);
	wxWindow* win = dynamic_cast<wxWindow*>(this);
	m_frame->UpdateProps(vc, excl_self, win);//update ui but exclude this
}

void PropBase::SetFocusVRenderViews(wxBasisSlider* slider)
{
	Root* root = glbin_data_manager.GetRoot();
	if (root)
	{
		for (int i = 0; i < root->GetViewNum(); i++)
		{
			RenderView* view = root->GetView(i);
			RenderCanvas* canvas = view->GetRenderCanvas();
			if (canvas)
			{
				canvas->SetFocusedSlider(slider);
			}
		}
	}
}

double PropBase::getDpiScaleFactor()
{
	wxWindow* win = dynamic_cast<wxWindow*>(this);
	if (win)
	{
		double dpi_sf = win->GetDPIScaleFactor();
		double dpi_sf2 = std::round(dpi_sf - 0.1);
		return dpi_sf2 < dpi_sf ? dpi_sf : 1;
	}
	return 1;
}

PropPanel::PropPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropBase(frame),
	wxScrolledWindow(parent, wxID_ANY, pos, size, style, name)
{

}

PropDialog::PropDialog(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropBase(frame),
	wxDialog(parent, wxID_ANY, name, pos, size, style)
{

}

