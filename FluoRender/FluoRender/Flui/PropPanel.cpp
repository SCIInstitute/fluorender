/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <MainFrame.h>
#include <RenderCanvas.h>
#include <RenderViewPanel.h>

PropPanel::PropPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxScrolledWindow(parent, wxID_ANY, pos, size,style, name),
	m_frame(frame)
{
}

PropPanel::~PropPanel()
{
}

void PropPanel::LoadPerspective()
{

}

void PropPanel::SavePerspective()
{

}

void PropPanel::FluoRefresh(int excl_self,
	const fluo::ValueCollection& vc, const std::set<int>& views)
{
	if (!m_frame)
		return;
	int view_excl = 0;
	if (dynamic_cast<RenderViewPanel*>(this))
		view_excl = excl_self;
	m_frame->RefreshCanvases(views);
	m_frame->UpdateProps(vc, excl_self, this);//update ui but exclude this
}

void PropPanel::SetFocusVRenderViews(wxBasisSlider* slider)
{
	if (m_frame)
	{
		for (int i = 0; i < m_frame->GetViewNum(); i++)
		{
			RenderCanvas* view = m_frame->GetRenderCanvas(i);
			if (view)
			{
				view->SetFocusedSlider(slider);
			}
		}
	}
}

double PropPanel::getDpiScaleFactor()
{
	double dpi_sf = GetDPIScaleFactor();
	double dpi_sf2 = std::round(dpi_sf - 0.1);
	return dpi_sf2 < dpi_sf ? dpi_sf : 1;
}