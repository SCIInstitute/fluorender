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
#include <MeshPropPanel.h>
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

MeshPropPanel::MeshPropPanel(
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size,style, name),
	m_agent(nullptr)
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

MeshPropPanel::~MeshPropPanel()
{
}

//lighting
void MeshPropPanel::OnLightingCheck(wxCommandEvent& event)
{
	bool val = m_light_chk->GetValue();
	m_agent->updateValue(gstShadingEnable, val);
}

void MeshPropPanel::OnDiffChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
	m_agent->updateValue(gstColor, color);
	fluo::HSVColor hsv(color);
	m_agent->updateValue(gstMatAmb, hsv.val());
}

void MeshPropPanel::OnSpecChange(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	fluo::Color color(c.Red() / 255.0, c.Green() / 255.0, c.Blue() / 255.0);
	m_agent->updateValue(gstColor, color);
	fluo::HSVColor hsv(color);
	m_agent->updateValue(gstMatSpec, hsv.val());
}

void MeshPropPanel::OnShineChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition();
	wxString str = wxString::Format("%.0f", val);
	m_shine_text->SetValue(str);
}

void MeshPropPanel::OnShineText(wxCommandEvent& event)
{
	wxString str = m_shine_text->GetValue();
	double shine;
	if (str.ToDouble(&shine))
	{
		m_shine_sldr->SetValue(int(shine));
		m_agent->updateValue(gstMatShine, shine);
	}
}

void MeshPropPanel::OnAlphaChange(wxScrollEvent & event)
{
	double val = (double)event.GetPosition() / 255.0;
	wxString str = wxString::Format("%.2f", val);
	m_alpha_text->SetValue(str);
}

void MeshPropPanel::OnAlphaText(wxCommandEvent& event)
{
	wxString str = m_alpha_text->GetValue();
	double alpha;
	if (str.ToDouble(&alpha))
	{
		m_alpha_sldr->SetValue(int(alpha*255.0 + 0.5));
		m_agent->updateValue(gstAlpha, alpha);
	}
}

void MeshPropPanel::OnScaleChange(wxScrollEvent & event)
{
	double val = event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_scale_text->SetValue(str);
}

void MeshPropPanel::OnScaleText(wxCommandEvent& event)
{
	wxString str = m_scale_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_scale_sldr->SetValue(int(dval*100.0 + 0.5));
		m_agent->updateValue(gstScaleX, dval);
		m_agent->updateValue(gstScaleY, dval);
		m_agent->updateValue(gstScaleZ, dval);
	}
}

//shadow
void MeshPropPanel::OnShadowCheck(wxCommandEvent& event)
{
	bool val = m_shadow_chk->GetValue();
	m_agent->updateValue(gstShadowEnable, val);
}

void MeshPropPanel::OnShadowChange(wxScrollEvent& event)
{
	double val = event.GetPosition() / 100.0;
	wxString str = wxString::Format("%.2f", val);
	m_shadow_text->SetValue(str);
}

void MeshPropPanel::OnShadowText(wxCommandEvent& event)
{
	wxString str = m_shadow_text->GetValue();
	double dval;
	if (str.ToDouble(&dval))
	{
		m_shadow_sldr->SetValue(int(dval*100.0 + 0.5));
		m_agent->updateValue(gstShadowInt, dval);
	}
}

//size limiter
void MeshPropPanel::OnSizeCheck(wxCommandEvent& event)
{
	bool bval = m_size_chk->GetValue();
	m_agent->updateValue(gstLimitEnable, bval);
}

void MeshPropPanel::OnSizeChange(wxScrollEvent& event)
{
	int val = event.GetPosition();
	wxString str = wxString::Format("%d", val);
	m_size_text->SetValue(str);
}

void MeshPropPanel::OnSizeText(wxCommandEvent& event)
{
	wxString str = m_size_text->GetValue();
	long val;
	if (str.ToLong(&val))
	{
		m_size_sldr->SetValue(val);
		m_agent->updateValue(gstLimit, val);
	}
}
