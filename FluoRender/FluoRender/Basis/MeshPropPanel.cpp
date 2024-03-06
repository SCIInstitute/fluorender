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
#include "MeshPropPanel.h"
#include <Global.h>
#include <VRenderFrame.h>
#include <VRenderGLView.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(MeshPropPanel, wxPanel)
	//lighting
	EVT_CHECKBOX(ID_light_chk, MeshPropPanel::OnLightingCheck)
	EVT_COLOURPICKER_CHANGED(ID_diff_picker, MeshPropPanel::OnDiffChange)
	EVT_COLOURPICKER_CHANGED(ID_spec_picker, MeshPropPanel::OnSpecChange)
	EVT_COMMAND_SCROLL(ID_shine_sldr, MeshPropPanel::OnShineChange)
	EVT_COMMAND_SCROLL(ID_alpha_sldr, MeshPropPanel::OnAlphaChange)
	EVT_COMMAND_SCROLL(ID_scale_sldr, MeshPropPanel::OnScaleChange)
	EVT_TEXT(ID_shine_text, MeshPropPanel::OnShineText)
	EVT_TEXT(ID_alpha_text, MeshPropPanel::OnAlphaText)
	EVT_TEXT(ID_scale_text, MeshPropPanel::OnScaleText)
	//shadow
	EVT_CHECKBOX(ID_shadow_chk, MeshPropPanel::OnShadowCheck)
	EVT_COMMAND_SCROLL(ID_shadow_sldr, MeshPropPanel::OnShadowChange)
	EVT_TEXT(ID_shadow_text, MeshPropPanel::OnShadowText)
	//size limiter
	EVT_CHECKBOX(ID_size_chk, MeshPropPanel::OnSizeCheck)
	EVT_COMMAND_SCROLL(ID_size_sldr, MeshPropPanel::OnSizeChange)
	EVT_TEXT(ID_size_text, MeshPropPanel::OnSizeText)
END_EVENT_TABLE()

MeshPropPanel::MeshPropPanel(VRenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size,style, name),
	m_md(0),
	m_view(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

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
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_alpha_sldr = new wxSingleSlider(this, ID_alpha_sldr, 255, 0, 255, 
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, ID_alpha_text, "1.00",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp2);
	sizer_1->Add(20, 5, 0);
	sizer_1->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_1->Add(m_alpha_sldr, 0, wxALIGN_CENTER, 0);
	sizer_1->Add(m_alpha_text, 0, wxALIGN_CENTER, 0);

	m_shadow_chk = new wxCheckBox(this, ID_shadow_chk, "Shadow: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_shadow_sldr = new wxSingleSlider(this, ID_shadow_sldr, 60, 0, 100,
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, ID_shadow_text, "0.60",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp2);
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
		wxDefaultPosition, FromDIP(wxSize(110, 20)));
	m_diff_picker = new wxColourPickerCtrl(this, ID_diff_picker, *wxWHITE, 
		wxDefaultPosition, FromDIP(wxSize(180, 30)));
	sizer_4->Add(st, 0, wxALIGN_LEFT, 0);
	sizer_4->Add(m_diff_picker, 0, wxALIGN_CENTER, 0);

	st = new wxStaticText(this, 0, " Specular Color: ",
		wxDefaultPosition, FromDIP(wxSize(110, 20)));
	m_spec_picker = new wxColourPickerCtrl(this, ID_spec_picker, *wxWHITE, 
		wxDefaultPosition, FromDIP(wxSize(180, 30)));
	sizer_5->Add(st, 0, wxALIGN_LEFT, 0);
	sizer_5->Add(m_spec_picker, 0, wxALIGN_CENTER, 0);

	st = new wxStaticText(this, 0, " Shininess: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_shine_sldr = new wxSingleSlider(this, ID_shine_sldr, 30, 0, 128, 
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_shine_text = new wxTextCtrl(this, ID_shine_text, "30",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_int);
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
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_scale_sldr = new wxSingleSlider(this, ID_scale_sldr, 100, 50, 200, 
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_scale_text = new wxTextCtrl(this, ID_scale_text, "1.00",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_fp2);
	sizer_7->Add(20, 5, 0);
	sizer_7->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_7->Add(m_scale_sldr, 0, wxALIGN_CENTER, 0);
	sizer_7->Add(m_scale_text, 0, wxALIGN_CENTER, 0);

	//size limiter
	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
	m_size_chk = new wxCheckBox(this, ID_size_chk, " Size limit: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_size_sldr = new wxSingleSlider(this, ID_size_sldr, 50, 0, 250,
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_size_text = new wxTextCtrl(this, ID_size_text, "50",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), 0, vald_int);
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
	SetScrollRate(10, 10);
}

MeshPropPanel::~MeshPropPanel()
{
}

void MeshPropPanel::FluoUpdate(const fluo::ValueCollection& values)
{
	if (!m_md)
		return;

	wxString str;
	fluo::Color amb, diff, spec;
	double shine, alpha;
	m_md->GetMaterial(amb, diff, spec, shine, alpha);

	wxColor c;
	c = wxColor(diff.r()*255, diff.g()*255, diff.b()*255);
	m_diff_picker->SetColour(c);
	c = wxColor(spec.r()*255, spec.g()*255, spec.b()*255);
	m_spec_picker->SetColour(c);

	//lighting
	m_light_chk->SetValue(m_md->GetLighting());
	//shine
	m_shine_sldr->SetValue(std::round(shine));
	str = wxString::Format("%.0f", shine);
	m_shine_text->ChangeValue(str);
	//alpha
	m_alpha_sldr->SetValue(std::round(alpha*255));
	str = wxString::Format("%.2f", alpha);
	m_alpha_text->ChangeValue(str);
	//scaling
	double sx, sy, sz;
	m_md->GetScaling(sx, sy, sz);
	m_scale_sldr->SetValue(std::round(sx*100.0));
	str = wxString::Format("%.2f", sx);
	m_scale_text->ChangeValue(str);
	//shadow
	double darkness;
	m_shadow_chk->SetValue(m_md->GetShadowEnable());
	darkness = m_md->GetShadowIntensity();
	m_shadow_sldr->SetValue(std::round(darkness*100.0));
	str = wxString::Format("%.2f", darkness);
	m_shadow_text->ChangeValue(str);
	//size limiter
	m_size_chk->SetValue(m_md->GetLimit());
	int limit = m_md->GetLimitNumber();
	m_size_sldr->SetValue(limit);
	m_size_text->SetValue(wxString::Format("%d", limit));
}

void MeshPropPanel::SetView(VRenderGLView* view)
{
	m_view = view;
}

void MeshPropPanel::SetMeshData(MeshData* md)
{
	m_md = md;
	FluoUpdate();
}

MeshData* MeshPropPanel::GetMeshData()
{
	return m_md;
}

//lighting
void MeshPropPanel::OnLightingCheck(wxCommandEvent& event)
{
	if (m_md && m_view)
	{
		bool val = m_light_chk->GetValue();
		m_md->SetLighting(val);
		for (int i=0; i< m_view->GetMeshNum(); i++)
		{
			MeshData* md = m_view->GetMeshData(i);
			if (md)
				md->SetLighting(val);
		}
		FluoRefresh();
	}
}

void MeshPropPanel::OnDiffChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_md)
	{
		m_md->SetColor(color, MESH_COLOR_DIFF);
		fluo::Color amb = color * 0.3;
		m_md->SetColor(amb, MESH_COLOR_AMB);
		FluoRefresh(true);
	}
}

void MeshPropPanel::OnSpecChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_md)
	{
		m_md->SetColor(color, MESH_COLOR_SPEC);
		FluoRefresh();
	}
}

void MeshPropPanel::OnShineChange(wxScrollEvent & event)
{
	double val = m_shine_sldr->GetValue();
	wxString str = wxString::Format("%.0f", val);
	if (str != m_shine_text->GetValue())
		m_shine_text->SetValue(str);
}

void MeshPropPanel::OnShineText(wxCommandEvent& event)
{
	wxString str = m_shine_text->GetValue();
	double shine;
	str.ToDouble(&shine);
	m_shine_sldr->SetValue(std::round(shine));

	if (m_md)
	{
		m_md->SetFloat(shine, MESH_FLOAT_SHN);
		FluoRefresh();
	}
}

void MeshPropPanel::OnAlphaChange(wxScrollEvent & event)
{
	double val = m_alpha_sldr->GetValue() / 255.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_alpha_text->GetValue())
		m_alpha_text->SetValue(str);
}

void MeshPropPanel::OnAlphaText(wxCommandEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	double alpha;
	str.ToDouble(&alpha);
	m_alpha_sldr->SetValue(std::round(alpha*255.0));

	if (m_md)
	{
		m_md->SetFloat(alpha, MESH_FLOAT_ALPHA);
		FluoRefresh();
	}
}

void MeshPropPanel::OnScaleChange(wxScrollEvent & event)
{
	double val = m_scale_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_scale_text->GetValue())
		m_scale_text->SetValue(str);
}

void MeshPropPanel::OnScaleText(wxCommandEvent& event)
{
	wxString str = m_scale_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_scale_sldr->SetValue(std::round(dval*100.0));

	if (m_md)
	{
		m_md->SetScaling(dval, dval, dval);
		FluoRefresh();
	}
}

//shadow
void MeshPropPanel::OnShadowCheck(wxCommandEvent& event)
{
	if (m_md && m_view)
	{
		bool val = m_shadow_chk->GetValue();
		m_md->SetShadowEnable(val);
		for (int i=0; i< m_view->GetMeshNum(); i++)
		{
			MeshData* md = m_view->GetMeshData(i);
			if (md)
				md->SetShadowEnable(val);
		}
		FluoRefresh();
	}
}

void MeshPropPanel::OnShadowChange(wxScrollEvent& event)
{
	double val = m_shadow_sldr->GetValue() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shadow_text->GetValue())
		m_shadow_text->SetValue(str);
}

void MeshPropPanel::OnShadowText(wxCommandEvent& event)
{
	wxString str = m_shadow_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_shadow_sldr->SetValue(std::round(dval*100.0));

	if (m_md && m_view)
	{
		m_md->SetShadowIntensity(dval);
		for (int i=0; i< m_view->GetMeshNum(); i++)
		{
			MeshData* md = m_view->GetMeshData(i);
			if (md)
				md->SetShadowIntensity(dval);
		}
		FluoRefresh();
	}
}

//size limiter
void MeshPropPanel::OnSizeCheck(wxCommandEvent& event)
{
	bool bval = m_size_chk->GetValue();
	if (m_md)
	{
		m_md->SetLimit(bval);
		FluoRefresh();
	}
}

void MeshPropPanel::OnSizeChange(wxScrollEvent& event)
{
	int val = m_size_sldr->GetValue();
	wxString str = wxString::Format("%d", val);
	if (str != m_size_text->GetValue())
		m_size_text->SetValue(str);
}

void MeshPropPanel::OnSizeText(wxCommandEvent& event)
{
	wxString str = m_size_text->GetValue();
	long val;
	str.ToLong(&val);
	m_size_sldr->SetValue(val);

	if (m_md)
	{
		m_md->SetLimitNumer(val);
		if (m_md->GetLimit())
			FluoRefresh();
	}
}
