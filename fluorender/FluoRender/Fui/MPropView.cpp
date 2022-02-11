/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include "MPropView.h"
#include "VRenderFrame.h"
#include <Renderview.hpp>
#include <MeshData.hpp>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(MPropView, wxPanel)
	//lighting
	EVT_CHECKBOX(ID_light_chk, MPropView::OnLightingCheck)
	EVT_COLOURPICKER_CHANGED(ID_diff_picker, MPropView::OnDiffChange)
	EVT_COLOURPICKER_CHANGED(ID_spec_picker, MPropView::OnSpecChange)
	EVT_COMMAND_SCROLL(ID_shine_sldr, MPropView::OnShineChange)
	EVT_COMMAND_SCROLL(ID_alpha_sldr, MPropView::OnAlphaChange)
	EVT_COMMAND_SCROLL(ID_scale_sldr, MPropView::OnScaleChange)
	EVT_TEXT(ID_shine_text, MPropView::OnShineText)
	EVT_TEXT(ID_alpha_text, MPropView::OnAlphaText)
	EVT_TEXT(ID_scale_text, MPropView::OnScaleText)
	//shadow
	EVT_CHECKBOX(ID_shadow_chk, MPropView::OnShadowCheck)
	EVT_COMMAND_SCROLL(ID_shadow_sldr, MPropView::OnShadowChange)
	EVT_TEXT(ID_shadow_text, MPropView::OnShadowText)
	//size limiter
	EVT_CHECKBOX(ID_size_chk, MPropView::OnSizeCheck)
	EVT_COMMAND_SCROLL(ID_size_sldr, MPropView::OnSizeChange)
	EVT_TEXT(ID_size_text, MPropView::OnSizeText)
END_EVENT_TABLE()

MPropView::MPropView(VRenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size,style, name),
	m_frame(frame),
	m_md(0),
	m_view(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	wxBoxSizer* sizer_v1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* st = 0;

	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	st = new wxStaticText(this, 0, " Transparency: ",
		wxDefaultPosition, wxSize(100, 20));
	m_alpha_sldr = new wxSlider(this, ID_alpha_sldr, 255, 0, 255, 
		wxDefaultPosition, wxSize(200, 20), wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, ID_alpha_text, "1.00",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	sizer_1->Add(20, 5, 0);
	sizer_1->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_1->Add(m_alpha_sldr, 0, wxALIGN_CENTER, 0);
	sizer_1->Add(m_alpha_text, 0, wxALIGN_CENTER, 0);

	m_shadow_chk = new wxCheckBox(this, ID_shadow_chk, "Shadow: ",
		wxDefaultPosition, wxSize(100, 20));
	m_shadow_sldr = new wxSlider(this, ID_shadow_sldr, 60, 0, 100,
		wxDefaultPosition, wxSize(200, 20), wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, ID_shadow_text, "0.60",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	sizer_2->Add(20, 5, 0);
	sizer_2->Add(m_shadow_chk, 0, wxALIGN_CENTER, 0);
	sizer_2->Add(m_shadow_sldr, 0, wxALIGN_CENTER, 0);
	sizer_2->Add(m_shadow_text, 0, wxALIGN_CENTER, 0);

	m_light_chk = new wxCheckBox(this, ID_light_chk, "Lighting",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	sizer_3->Add(20, 5, 0);
	sizer_3->Add(m_light_chk, 0, wxALIGN_CENTER, 0);

	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Material"),
		wxVERTICAL);

	st = new wxStaticText(this, 0, " Diffuse Color: ",
		wxDefaultPosition, wxSize(110, 20));
	m_diff_picker = new wxColourPickerCtrl(this, ID_diff_picker, *wxWHITE, 
		wxDefaultPosition, wxSize(180, 30));
	sizer_4->Add(st, 0, wxALIGN_LEFT, 0);
	sizer_4->Add(m_diff_picker, 0, wxALIGN_CENTER, 0);

	st = new wxStaticText(this, 0, " Specular Color: ",
		wxDefaultPosition, wxSize(110, 20));
	m_spec_picker = new wxColourPickerCtrl(this, ID_spec_picker, *wxWHITE, 
		wxDefaultPosition, wxSize(180, 30));
	sizer_5->Add(st, 0, wxALIGN_LEFT, 0);
	sizer_5->Add(m_spec_picker, 0, wxALIGN_CENTER, 0);

	st = new wxStaticText(this, 0, " Shininess: ",
		wxDefaultPosition, wxSize(100, 20));
	m_shine_sldr = new wxSlider(this, ID_shine_sldr, 30, 0, 128, 
		wxDefaultPosition, wxSize(200, 20), wxSL_HORIZONTAL);
	m_shine_text = new wxTextCtrl(this, ID_shine_text, "30",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	sizer_6->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_6->Add(m_shine_sldr, 0, wxALIGN_CENTER, 0);
	sizer_6->Add(m_shine_text, 0, wxALIGN_CENTER, 0);

	group1->Add(sizer_4, 0, wxALIGN_LEFT);
	group1->Add(sizer_5, 0, wxALIGN_LEFT);
	group1->Add(sizer_6, 0, wxALIGN_LEFT);

	sizer_v1->Add(group1, 0, wxALIGN_LEFT);

	wxBoxSizer* sizer_v2 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, " Scaling: ",
		wxDefaultPosition, wxSize(100, 20));
	m_scale_sldr = new wxSlider(this, ID_scale_sldr, 100, 50, 200, 
		wxDefaultPosition, wxSize(200, 20), wxSL_HORIZONTAL);
	m_scale_text = new wxTextCtrl(this, ID_scale_text, "1.00",
		wxDefaultPosition, wxSize(50, 20), 0, vald_fp2);
	sizer_7->Add(20, 5, 0);
	sizer_7->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_7->Add(m_scale_sldr, 0, wxALIGN_CENTER, 0);
	sizer_7->Add(m_scale_text, 0, wxALIGN_CENTER, 0);

	//size limiter
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
	m_size_chk = new wxCheckBox(this, ID_size_chk, " Size limit: ",
		wxDefaultPosition, wxSize(100, 20));
	m_size_sldr = new wxSlider(this, ID_size_sldr, 50, 0, 250,
		wxDefaultPosition, wxSize(200, 20), wxSL_HORIZONTAL);
	m_size_text = new wxTextCtrl(this, ID_size_text, "50",
		wxDefaultPosition, wxSize(50, 20), 0, vald_int);
	sizer_8->Add(20, 5, 0);
	sizer_8->Add(m_size_chk, 0, wxALIGN_CENTER);
	sizer_8->Add(m_size_sldr, 0, wxALIGN_CENTER);
	sizer_8->Add(m_size_text, 0, wxALIGN_CENTER);

	sizer_v2->Add(5,5);
	sizer_v2->Add(sizer_1, 0, wxALIGN_LEFT);
	sizer_v2->Add(sizer_2, 0, wxALIGN_LEFT);
	sizer_v2->Add(sizer_3, 0, wxALIGN_LEFT);
	sizer_v2->Add(sizer_7, 0, wxALIGN_LEFT);
	sizer_v2->Add(sizer_8, 0, wxALIGN_LEFT);

	wxBoxSizer* sizer_all = new wxBoxSizer(wxHORIZONTAL);
	sizer_all->Add(sizer_v1, 0, wxALIGN_TOP);
	sizer_all->Add(sizer_v2, 0, wxALIGN_TOP);

	SetSizer(sizer_all);
	Layout();
}

MPropView::~MPropView()
{
}

void MPropView::GetSettings()
{
	if (!m_md)
		return;

	wxString str;
	fluo::Color color;
	double amb, diff, spec;
	double shine, alpha;
	m_md->getValue(gstColor, color);
	m_md->getValue(gstMatAmb, amb);
	m_md->getValue(gstMatDiff, diff);
	m_md->getValue(gstMatSpec, spec);
	m_md->getValue(gstMatShine, shine);
	m_md->getValue(gstAlpha, alpha);

	wxColor c;
	c = wxColor(diff*color.r()*255, diff*color.g()*255, diff*color.b()*255);
	m_diff_picker->SetColour(c);
	c = wxColor(spec*color.r()*255, spec*color.g()*255, spec*color.b()*255);
	m_spec_picker->SetColour(c);

	//lighting
	bool bval;
	m_md->getValue(gstShadingEnable, bval);
	m_light_chk->SetValue(bval);
	//shine
	m_shine_sldr->SetValue(int(shine));
	str = wxString::Format("%.0f", shine);
	m_shine_text->ChangeValue(str);
	//alpha
	m_alpha_sldr->SetValue(int(alpha*255));
	str = wxString::Format("%.2f", alpha);
	m_alpha_text->ChangeValue(str);
	//scaling
	double sx, sy, sz;
	m_md->getValue(gstScaleX, sx);
	m_md->getValue(gstScaleY, sy);
	m_md->getValue(gstScaleZ, sz);
	m_scale_sldr->SetValue(int(sx*100.0+0.5));
	str = wxString::Format("%.2f", sx);
	m_scale_text->ChangeValue(str);
	//shadow
	double dval;
	m_md->getValue(gstShadowEnable, bval);
	m_shadow_chk->SetValue(bval);
	m_md->getValue(gstShadowInt, dval);
	m_shadow_sldr->SetValue(int(dval*100.0+0.5));
	str = wxString::Format("%.2f", dval);
	m_shadow_text->ChangeValue(str);
	//size limiter
	long lval;
	m_md->getValue(gstLimitEnable, bval);
	m_size_chk->SetValue(bval);
	m_md->getValue(gstLimit, lval);
	m_size_sldr->SetValue(lval);
	m_size_text->SetValue(wxString::Format("%d", lval));
}

void MPropView::SetView(fluo::Renderview* view)
{
	m_view = view;
}

void MPropView::SetMeshData(fluo::MeshData* md)
{
	m_md = md;
	GetSettings();
}

fluo::MeshData* MPropView::GetMeshData()
{
	return m_md;
}

void MPropView::RefreshVRenderViews(bool tree)
{
	if (m_frame)
		m_frame->RefreshVRenderViews(tree);
}

//lighting
void MPropView::OnLightingCheck(wxCommandEvent& event)
{
	if (m_md && m_view)
	{
		bool val = m_light_chk->GetValue();
		m_md->setValue(gstShadingEnable, val);
		fluo::MeshList list = m_view->GetMeshList();
		for (auto md : list)
		{
			if (md)
				md->setValue(gstShadingEnable, val);
		}
		RefreshVRenderViews();
	}
}

void MPropView::OnDiffChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_md)
	{
		m_md->setValue(gstColor, color);
		fluo::HSVColor hsv(color);
		m_md->setValue(gstMatAmb, hsv.val());
		RefreshVRenderViews(true);
	}
}

void MPropView::OnSpecChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_md)
	{
		m_md->setValue(gstColor, color);
		fluo::HSVColor hsv(color);
		m_md->setValue(gstMatSpec, hsv.val());
		RefreshVRenderViews();
	}
}

void MPropView::OnShineChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%.0f", val);
	if (str != m_shine_text->GetValue())
		m_shine_text->SetValue(str);
}

void MPropView::OnShineText(wxCommandEvent& event)
{
	wxString str = m_shine_text->GetValue();
	double shine;
	str.ToDouble(&shine);
	m_shine_sldr->SetValue(int(shine));

	if (m_md)
	{
		m_md->setValue(gstMatShine, shine);
		RefreshVRenderViews();
	}
}

void MPropView::OnAlphaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 255.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_alpha_text->GetValue())
		m_alpha_text->SetValue(str);
}

void MPropView::OnAlphaText(wxCommandEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	double alpha;
	str.ToDouble(&alpha);
	m_alpha_sldr->SetValue(int(alpha*255.0+0.5));

	if (m_md)
	{
		m_md->setValue(gstAlpha, alpha);
		RefreshVRenderViews();
	}
}

void MPropView::OnScaleChange(wxScrollEvent & event)
{
	double val = event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_scale_text->GetValue())
		m_scale_text->SetValue(str);
}

void MPropView::OnScaleText(wxCommandEvent& event)
{
	wxString str = m_scale_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_scale_sldr->SetValue(int(dval*100.0+0.5));

	if (m_md)
	{
		m_md->setValue(gstScaleX, dval);
		m_md->setValue(gstScaleY, dval);
		m_md->setValue(gstScaleZ, dval);
		RefreshVRenderViews();
	}
}

//shadow
void MPropView::OnShadowCheck(wxCommandEvent& event)
{
	if (m_md && m_view)
	{
		bool val = m_shadow_chk->GetValue();
		m_md->setValue(gstShadowEnable, val);
		fluo::MeshList list = m_view->GetMeshList();
		for (auto md : list)
		{
			if (md)
				md->setValue(gstShadowEnable, val);
		}
		RefreshVRenderViews();
	}
}

void MPropView::OnShadowChange(wxScrollEvent& event)
{
	double val = event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shadow_text->GetValue())
		m_shadow_text->SetValue(str);
}

void MPropView::OnShadowText(wxCommandEvent& event)
{
	wxString str = m_shadow_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_shadow_sldr->SetValue(int(dval*100.0+0.5));

	if (m_md && m_view)
	{
		m_md->setValue(gstShadowInt, dval);
		fluo::MeshList list = m_view->GetMeshList();
		for (auto md : list)
		{
			if (md)
				md->setValue(gstShadowInt, dval);
		}
		RefreshVRenderViews();
	}
}

//size limiter
void MPropView::OnSizeCheck(wxCommandEvent& event)
{
	bool bval = m_size_chk->GetValue();
	if (m_md)
	{
		m_md->setValue(gstLimitEnable, bval);
		RefreshVRenderViews();
	}
}

void MPropView::OnSizeChange(wxScrollEvent& event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	if (str != m_size_text->GetValue())
		m_size_text->SetValue(str);
}

void MPropView::OnSizeText(wxCommandEvent& event)
{
	wxString str = m_size_text->GetValue();
	long val;
	str.ToLong(&val);
	m_size_sldr->SetValue(val);

	if (m_md)
	{
		m_md->setValue(gstLimit, val);
		bool bval;
		m_md->getValue(gstLimitEnable, bval);
		if (bval)
			RefreshVRenderViews();
	}
}
