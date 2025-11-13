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
#include <MeshPropPanel.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>
#include <CurrentObjects.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <MeshData.h>
#include <wxSingleSlider.h>
#include <wx/valnum.h>

MeshPropPanel::MeshPropPanel(MainFrame* frame,
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

	wxStaticText* st = 0;
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticBoxSizer *group1 = new wxStaticBoxSizer(
		wxVERTICAL, this, "Material");

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, " Diffuse Color: ",
		wxDefaultPosition, FromDIP(wxSize(110, 20)));
	m_diff_picker = new wxColourPickerCtrl(this, wxID_ANY, *wxWHITE,
		wxDefaultPosition, FromDIP(wxSize(180, 30)));
	m_diff_picker->Bind(wxEVT_COLOURPICKER_CHANGED, &MeshPropPanel::OnDiffChange, this);
	sizer_1->Add(st, 0, wxALIGN_LEFT, 0);
	sizer_1->Add(m_diff_picker, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, " Specular Color: ",
		wxDefaultPosition, FromDIP(wxSize(110, 20)));
	m_spec_picker = new wxColourPickerCtrl(this, wxID_ANY, *wxWHITE,
		wxDefaultPosition, FromDIP(wxSize(180, 30)));
	m_spec_picker->Bind(wxEVT_COLOURPICKER_CHANGED, &MeshPropPanel::OnSpecChange, this);
	sizer_2->Add(st, 0, wxALIGN_LEFT, 0);
	sizer_2->Add(m_spec_picker, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, " Shininess: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_shine_sldr = new wxSingleSlider(this, wxID_ANY, 30, 0, 128,
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_shine_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShineChange, this);
	m_shine_text = new wxTextCtrl(this, wxID_ANY, "30",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_int);
	m_shine_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShineText, this);
	sizer_3->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_3->Add(m_shine_sldr, 0, wxALIGN_CENTER, 0);
	sizer_3->Add(m_shine_text, 0, wxALIGN_CENTER, 0);

	group1->Add(sizer_1, 0, wxALIGN_LEFT);
	group1->Add(sizer_2, 0, wxALIGN_LEFT);
	group1->Add(sizer_3, 0, wxALIGN_LEFT);

	wxBoxSizer* sizer_v1 = new wxBoxSizer(wxVERTICAL);
	sizer_v1->Add(group1, 0, wxALIGN_LEFT);

	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, " Transparency: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_alpha_sldr = new wxSingleSlider(this, wxID_ANY, 255, 0, 255, 
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_alpha_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnAlphaChange, this);
	m_alpha_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_fp2);
	m_alpha_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnAlphaText, this);
	sizer_4->Add(20, 5, 0);
	sizer_4->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_4->Add(m_alpha_sldr, 0, wxALIGN_CENTER, 0);
	sizer_4->Add(m_alpha_text, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	m_shadow_chk = new wxCheckBox(this, wxID_ANY, "Shadow: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_shadow_chk->Bind(wxEVT_CHECKBOX, &MeshPropPanel::OnShadowCheck, this);
	m_shadow_sldr = new wxSingleSlider(this, wxID_ANY, 60, 0, 100,
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_shadow_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShadowChange, this);
	m_shadow_text = new wxTextCtrl(this, wxID_ANY, "0.60",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_fp2);
	m_shadow_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShadowText, this);
	sizer_5->Add(20, 5, 0);
	sizer_5->Add(m_shadow_chk, 0, wxALIGN_CENTER, 0);
	sizer_5->Add(m_shadow_sldr, 0, wxALIGN_CENTER, 0);
	sizer_5->Add(m_shadow_text, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
	m_shadow_dir_chk = new wxCheckBox(this, wxID_ANY, "Dir: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_shadow_dir_sldr = new wxSingleSlider(this, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_shadow_dir_sldr->SetRangeStyle(2);
	m_shadow_dir_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_int);
	m_shadow_dir_chk->Bind(wxEVT_CHECKBOX, &MeshPropPanel::OnShadowDirCheck, this);
	m_shadow_dir_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShadowDirChange, this);
	m_shadow_dir_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShadowDirText, this);
	sizer_6->Add(20, 5, 0);
	sizer_6->Add(m_shadow_dir_chk, 0, wxALIGN_CENTER, 0);
	sizer_6->Add(m_shadow_dir_sldr, 0, wxALIGN_CENTER, 0);
	sizer_6->Add(m_shadow_dir_text, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
	m_light_chk = new wxCheckBox(this, wxID_ANY, "Lighting",
		wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	m_light_chk->Bind(wxEVT_CHECKBOX, &MeshPropPanel::OnLightingCheck, this);
	sizer_7->Add(20, 5, 0);
	sizer_7->Add(m_light_chk, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, " Scaling: ",
		wxDefaultPosition, FromDIP(wxSize(100, 20)));
	m_scale_sldr = new wxSingleSlider(this, wxID_ANY, 100, 50, 200,
		wxDefaultPosition, FromDIP(wxSize(200, 20)), wxSL_HORIZONTAL);
	m_scale_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnScaleChange, this);
	m_scale_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_fp2);
	m_scale_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnScaleText, this);
	sizer_8->Add(20, 5, 0);
	sizer_8->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_8->Add(m_scale_sldr, 0, wxALIGN_CENTER, 0);
	sizer_8->Add(m_scale_text, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_v2 = new wxBoxSizer(wxVERTICAL);
	sizer_v2->Add(5,5);
	sizer_v2->Add(sizer_4, 0, wxALIGN_LEFT);
	sizer_v2->Add(sizer_5, 0, wxALIGN_LEFT);
	sizer_v2->Add(sizer_6, 0, wxALIGN_LEFT);
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
	m_shine_sldr->ChangeValue(std::round(shine));
	str = wxString::Format("%.0f", shine);
	m_shine_text->ChangeValue(str);
	//alpha
	m_alpha_sldr->ChangeValue(std::round(alpha*255));
	str = wxString::Format("%.2f", alpha);
	m_alpha_text->ChangeValue(str);
	//scaling
	double sx, sy, sz;
	m_md->GetScaling(sx, sy, sz);
	m_scale_sldr->ChangeValue(std::round(sx*100.0));
	str = wxString::Format("%.2f", sx);
	m_scale_text->ChangeValue(str);
	//shadow
	double darkness;
	m_shadow_chk->SetValue(m_md->GetShadowEnable());
	darkness = m_md->GetShadowIntensity();
	m_shadow_sldr->ChangeValue(std::round(darkness*100.0));
	str = wxString::Format("%.2f", darkness);
	m_shadow_text->ChangeValue(str);
	//dir
	bool bval = glbin_settings.m_shadow_dir;
	m_shadow_dir_chk->SetValue(bval);
	m_shadow_dir_sldr->Enable(bval);
	m_shadow_dir_text->Enable(bval);
	double deg = r2d(atan2(glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x));
	m_shadow_dir_sldr->ChangeValue(std::round(deg));
	m_shadow_dir_text->ChangeValue(wxString::Format("%.0f", deg));
}

void MeshPropPanel::SetView(RenderView* view)
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

void MeshPropPanel::EnableShadowDir(bool bval)
{
	if (glbin_settings.m_shadow_dir == bval)
		return;

	glbin_settings.m_shadow_dir = bval;
	if (bval)
	{
		wxString str;
		str = m_shadow_dir_text->GetValue();
		double deg;
		str.ToDouble(&deg);
		glbin_settings.m_shadow_dir_x = cos(d2r(deg));
		glbin_settings.m_shadow_dir_y = sin(d2r(deg));
	}
	else
	{
		glbin_settings.m_shadow_dir_x = 0.0;
		glbin_settings.m_shadow_dir_y = 0.0;
	}
	FluoRefresh(0, { gstShadowDir }, { glbin_current.GetViewId() });
}

void MeshPropPanel::SetShadowDir(double dval, bool notify)
{
	double dir_x = cos(d2r(dval));
	double dir_y = sin(d2r(dval));
	if (glbin_settings.m_shadow_dir_x == dir_x &&
		glbin_settings.m_shadow_dir_y == dir_y)
		return;

	glbin_settings.m_shadow_dir_x = dir_x;
	glbin_settings.m_shadow_dir_y = dir_y;
	if (notify)
		FluoRefresh(0, { gstShadowDir }, { glbin_current.GetViewId() });
	else
		FluoRefresh(0, { gstNull }, { glbin_current.GetViewId() });
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
			auto md = m_view->GetMeshData(i);
			if (md)
				md->SetLighting(val);
		}
		FluoRefresh(3, {gstNull});
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
		FluoRefresh(1, { gstTreeColors });
	}
}

void MeshPropPanel::OnSpecChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
	if (m_md)
	{
		m_md->SetColor(color, MESH_COLOR_SPEC);
		FluoRefresh(3, { gstNull });
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
	m_shine_sldr->ChangeValue(std::round(shine));

	if (m_md)
	{
		m_md->SetFloat(shine, MESH_FLOAT_SHN);
		FluoRefresh(3, { gstNull });
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
	m_alpha_sldr->ChangeValue(std::round(alpha*255.0));

	if (m_md)
	{
		m_md->SetFloat(alpha, MESH_FLOAT_ALPHA);
		FluoRefresh(3, { gstNull });
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
	m_scale_sldr->ChangeValue(std::round(dval*100.0));

	if (m_md)
	{
		m_md->SetScaling(dval, dval, dval);
		FluoRefresh(3, { gstNull });
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
			auto md = m_view->GetMeshData(i);
			if (md)
				md->SetShadowEnable(val);
		}
		FluoRefresh(3, { gstNull });
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
	m_shadow_sldr->ChangeValue(std::round(dval*100.0));

	if (m_md && m_view)
	{
		m_md->SetShadowIntensity(dval);
		for (int i=0; i< m_view->GetMeshNum(); i++)
		{
			auto md = m_view->GetMeshData(i);
			if (md)
				md->SetShadowIntensity(dval);
		}
		FluoRefresh(3, { gstNull });
	}
}

void MeshPropPanel::OnShadowDirCheck(wxCommandEvent& event)
{
	bool bval = m_shadow_dir_chk->GetValue();
	EnableShadowDir(bval);
}

void MeshPropPanel::OnShadowDirChange(wxScrollEvent& event)
{
	double deg = m_shadow_dir_sldr->GetValue();
	wxString str = wxString::Format("%.0f", deg);
	if (str != m_shadow_dir_text->GetValue())
		m_shadow_dir_text->SetValue(str);
	SetShadowDir(deg, false);
}

void MeshPropPanel::OnShadowDirText(wxCommandEvent& event)
{
	wxString str = m_shadow_dir_text->GetValue();
	double deg;
	str.ToDouble(&deg);
	m_shadow_dir_sldr->ChangeValue(std::round(deg));
	SetShadowDir(deg, false);
}
