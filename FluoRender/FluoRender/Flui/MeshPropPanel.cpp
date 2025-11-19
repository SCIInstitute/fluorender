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
#include <wxBoldText.h>
#include <wxUndoableColorPicker.h>
#include <Helper.h>
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

	wxBoldText* st = 0;
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxSize bts(FromDIP(wxSize(80, -1)));
	wxSize tts1(FromDIP(wxSize(40, -1)));

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxBoldText(this, 0, "Main Color:",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_color_text = new wxTextCtrl(this, wxID_ANY, "255 , 255 , 255",
		wxDefaultPosition, wxDefaultSize, wxTE_CENTER);
	m_color_btn = new wxUndoableColorPicker(this, wxID_ANY, *wxRED,
		wxDefaultPosition, wxDefaultSize);
	m_color_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnColorTextChange, this);
	m_color_text->Bind(wxEVT_LEFT_DCLICK, &MeshPropPanel::OnColorTextFocus, this);
	m_color_btn->Bind(wxEVT_COLOURPICKER_CHANGED, &MeshPropPanel::OnColorBtn, this);
	sizer_1->Add(st, 0, wxALIGN_CENTER, 0);
	sizer_1->Add(5, 5, 0);
	sizer_1->Add(m_color_text, 0, wxALIGN_CENTER);
	sizer_1->Add(m_color_btn, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxBoldText(this, 0, "Alpha:",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_alpha_sldr = new wxSingleSlider(this, wxID_ANY, 127, 0, 255, 
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_alpha_text = new wxTextCtrl(this, wxID_ANY, "0.50",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp2);
	m_alpha_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnAlphaChange, this);
	m_alpha_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnAlphaText, this);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(5, 5);
	sizer_2->Add(m_alpha_sldr, 1, wxALIGN_CENTER);
	sizer_2->Add(m_alpha_text, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxBoldText(this, 0, "Shading:",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_shading_chk = new wxCheckBox(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_shading_sldr = new wxSingleSlider(this, wxID_ANY, 255, 0, 255, 
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shading_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp2);
	m_shine_sldr = new wxSingleSlider(this, wxID_ANY, 255, 0, 255, 
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shine_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp2);
	m_shading_chk->Bind(wxEVT_CHECKBOX, &MeshPropPanel::OnShadingCheck, this);
	m_shading_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShadingChange, this);
	m_shading_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShadingText, this);
	m_shine_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShineChange, this);
	m_shine_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShineText, this);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_shading_chk, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_shading_sldr, 1, wxALIGN_CENTER);
	sizer_3->Add(m_shading_text, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	sizer_3->Add(m_shine_sldr, 1, wxALIGN_CENTER);
	sizer_3->Add(m_shine_text, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxBoldText(this, 0, "Shadow:",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_shadow_chk = new wxCheckBox(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize);
	m_shadow_sldr = new wxSingleSlider(this, wxID_ANY, 60, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_text = new wxTextCtrl(this, wxID_ANY, "0.60",
		wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, vald_fp2);
	m_shadow_dir_chk = new wxCheckBox(this, wxID_ANY, "D:",
		wxDefaultPosition, wxDefaultSize);
	m_shadow_dir_sldr = new wxSingleSlider(this, wxID_ANY, 0, -180, 180,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_shadow_dir_sldr->SetRangeStyle(2);
	m_shadow_dir_text = new wxTextCtrl(this, wxID_ANY, "0",
		wxDefaultPosition, FromDIP(wxSize(50, 20)), wxTE_RIGHT, vald_int);
	m_shadow_chk->Bind(wxEVT_CHECKBOX, &MeshPropPanel::OnShadowCheck, this);
	m_shadow_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShadowChange, this);
	m_shadow_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShadowText, this);
	m_shadow_dir_chk->Bind(wxEVT_CHECKBOX, &MeshPropPanel::OnShadowDirCheck, this);
	m_shadow_dir_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnShadowDirChange, this);
	m_shadow_dir_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnShadowDirText, this);
	sizer_4->Add(st, 0, wxALIGN_CENTER);
	sizer_4->Add(m_shadow_chk, 0, wxALIGN_CENTER, 0);
	sizer_4->Add(5, 5);
	sizer_4->Add(m_shadow_sldr, 1, wxALIGN_CENTER, 0);
	sizer_4->Add(m_shadow_text, 0, wxALIGN_CENTER, 0);
	sizer_4->Add(m_shadow_dir_chk, 0, wxALIGN_CENTER, 0);
	sizer_4->Add(m_shadow_dir_sldr, 1, wxALIGN_CENTER, 0);
	sizer_4->Add(m_shadow_dir_text, 0, wxALIGN_CENTER, 0);

	wxBoxSizer* sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxBoldText(this, 0, " Size:",
		wxDefaultPosition, bts, wxALIGN_CENTER);
	m_scale_sldr = new wxSingleSlider(this, wxID_ANY, 100, 50, 200,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_scale_text = new wxTextCtrl(this, wxID_ANY, "1.00",
		wxDefaultPosition, tts1, wxTE_RIGHT, vald_fp2);
	m_scale_sldr->Bind(wxEVT_SCROLL_CHANGED, &MeshPropPanel::OnScaleChange, this);
	m_scale_text->Bind(wxEVT_TEXT, &MeshPropPanel::OnScaleText, this);
	sizer_5->Add(st, 0, wxALIGN_CENTER);
	sizer_5->Add(5, 5);
	sizer_5->Add(m_scale_sldr, 1, wxALIGN_CENTER);
	sizer_5->Add(m_scale_text, 0, wxALIGN_CENTER);

	wxBoxSizer* sizer_v = new wxBoxSizer(wxVERTICAL);
	sizer_v->Add(sizer_1, 1, wxEXPAND);
	sizer_v->Add(sizer_2, 1, wxEXPAND);
	sizer_v->Add(sizer_3, 1, wxEXPAND);
	sizer_v->Add(sizer_4, 1, wxEXPAND);
	sizer_v->Add(sizer_5, 1, wxEXPAND);

	SetSizer(sizer_v);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

MeshPropPanel::~MeshPropPanel()
{
}

void MeshPropPanel::FluoUpdate(const fluo::ValueCollection& vc)
{
	if (FOUND_VALUE(gstNull))
		return;
	if (!m_md)
		return;

	bool update_all = vc.empty() || FOUND_VALUE(gstMeshProps);

	fluo::Color cval;
	double dval;
	wxString str;
	bool bval;

	//color
	if (update_all)
	{
		cval = m_md->GetColor();
		wxColor wxc(cval.r() * 255, cval.g() * 255, cval.b() * 255);
		m_color_text->ChangeValue(wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue()));
		m_color_btn->SetValue(wxc);
	}

	//alpha
	if (update_all)
	{
		dval = m_md->GetAlpha();
		m_alpha_sldr->ChangeValue(std::round(dval * 255.0));
		str = wxString::Format("%.2f", dval);
		m_alpha_text->ChangeValue(str);
	}

	//shading
	if (update_all)
	{
		bval = m_md->GetShading();
		m_shading_chk->SetValue(bval);
		dval = m_md->GetShadingStrength();
		m_shading_sldr->ChangeValue(std::round(dval * 255.0));
		str = wxString::Format("%.2f", dval);
		m_shading_text->ChangeValue(str);
		//shine
		dval = m_md->GetShadingShine();
		m_shine_sldr->ChangeValue(std::round(dval * 255.0));
		str = wxString::Format("%.2f", dval);
		m_shine_text->ChangeValue(str);
	}

	//shadow
	if (update_all)
	{
		bval = m_md->GetShadowEnable();
		m_shadow_chk->SetValue(bval);
		dval = m_md->GetShadowIntensity();
		m_shadow_sldr->ChangeValue(std::round(dval * 100.0));
		str = wxString::Format("%.2f", dval);
		m_shadow_text->ChangeValue(str);
	}
	//dir
	if (update_all || FOUND_VALUE(gstShadowDir))
	{
		bval = glbin_settings.m_shadow_dir;
		m_shadow_dir_chk->SetValue(bval);
		m_shadow_dir_sldr->Enable(bval);
		m_shadow_dir_text->Enable(bval);
		double dirx = glbin_settings.m_shadow_dir_x;
		double diry = glbin_settings.m_shadow_dir_y;
		if (dirx == 0.0 && diry == 0.0)
			dval = 0.0;
		else
			dval = r2d(atan2(glbin_settings.m_shadow_dir_y, glbin_settings.m_shadow_dir_x)) + 45.0;
		m_shadow_dir_sldr->ChangeValue(std::round(dval));
		m_shadow_dir_text->ChangeValue(wxString::Format("%.0f", dval));
	}

	//scaling
	if (update_all)
	{
		double sy, sz;
		m_md->GetScaling(dval, sy, sz);
		m_scale_sldr->ChangeValue(std::round(dval * 100.0));
		str = wxString::Format("%.2f", dval);
		m_scale_text->ChangeValue(str);
	}
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
		deg -= 45.0;
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
	dval -= 45.0;
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

void MeshPropPanel::OnColorChange(const wxColor& c)
{
	if (m_md)
	{
		fluo::Color color(c.Red()/255.0, c.Green()/255.0, c.Blue()/255.0);
		m_md->SetColor(color);
		FluoRefresh(1, { gstTreeColors });
	}
}

void MeshPropPanel::OnColorTextChange(wxCommandEvent& event)
{
	wxString str = m_color_text->GetValue();
	wxColor wxc;
	if (GetColorString(str, wxc) == 3)
	{
		wxString new_str = wxString::Format("%d , %d , %d",
			wxc.Red(), wxc.Green(), wxc.Blue());
		if (str != new_str)
			m_color_text->ChangeValue(new_str);
		m_color_btn->SetValue(wxc);

		OnColorChange(wxc);
	}
}

void MeshPropPanel::OnColorTextFocus(wxMouseEvent& event)
{
	m_color_text->SetSelection(0, -1);
}

void MeshPropPanel::OnColorBtn(wxColourPickerEvent& event)
{
	wxColor c = event.GetColour();
	OnColorChange(c);
}

void MeshPropPanel::OnShadingCheck(wxCommandEvent& event)
{
	assert(m_md && m_view);
	bool val = m_shading_chk->GetValue();
	for (int i=0; i< m_view->GetMeshNum(); i++)
	{
		auto md = m_view->GetMeshData(i);
		if (md)
			md->SetShading(val);
	}
	FluoRefresh(0, {gstMeshProps});
}

void MeshPropPanel::OnShadingChange(wxScrollEvent & event)
{
	double val = m_shading_sldr->GetValue() / 255.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shading_text->GetValue())
		m_shading_text->SetValue(str);
}

void MeshPropPanel::OnShadingText(wxCommandEvent& event)
{
	wxString str = m_shading_text->GetValue();
	double dval;
	str.ToDouble(&dval);
	m_shading_sldr->ChangeValue(std::round(dval * 255.0));

	if (m_md)
	{
		m_md->SetShadingStrength(dval);
		FluoRefresh(0, { gstMeshProps });
	}
}

void MeshPropPanel::OnShineChange(wxScrollEvent & event)
{
	double val = m_shine_sldr->GetValue() / 255.0;
	wxString str = wxString::Format("%.2f", val);
	if (str != m_shine_text->GetValue())
		m_shine_text->SetValue(str);
}

void MeshPropPanel::OnShineText(wxCommandEvent& event)
{
	wxString str = m_shine_text->GetValue();
	double shine;
	str.ToDouble(&shine);
	m_shine_sldr->ChangeValue(std::round(shine * 255.0));

	if (m_md)
	{
		m_md->SetShadingShine(shine);
		FluoRefresh(0, { gstMeshProps });
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
		m_md->SetAlpha(alpha);
		FluoRefresh(0, { gstMeshProps });
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
		FluoRefresh(0, { gstMeshProps });
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
		FluoRefresh(0, { gstMeshProps });
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
		FluoRefresh(0, { gstMeshProps });
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
