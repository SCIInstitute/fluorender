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

#include <MeshPropAgent.hpp>
#include <MeshPropPanel.h>
#include <wx/valnum.h>
#include <wx/colour.h>

using namespace fluo;

MeshPropAgent::MeshPropAgent(MeshPropPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
}

void MeshPropAgent::setObject(MeshData* obj)
{
	InterfaceAgent::setObject(obj);
}

MeshData* MeshPropAgent::getObject()
{
	return dynamic_cast<MeshData*>(InterfaceAgent::getObject());
}

void MeshPropAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
	wxString str;

	if (update_all || FOUND_VALUE(gstColor))
	{
		Color diff;
		getValue(gstColor, diff);
		wxColor c = wxColor(diff.r()*255, diff.g()*255, diff.b()*255);
		panel_.m_diff_picker->SetColour(c);
	}
	if (update_all || FOUND_VALUE(gstMatSpec))
	{
		Color spec;
		getValue(gstMatSpec, spec);
		wxColor c = wxColor(spec.r() * 255, spec.g() * 255, spec.b() * 255);
		panel_.m_spec_picker->SetColour(c);
	}

	//lighting
	if (update_all || FOUND_VALUE(gstShadingEnable))
	{
		bool light_enable;
		getValue(gstShadingEnable, light_enable);
		panel_.m_light_chk->SetValue(light_enable);
	}
	//shine
	if (update_all || FOUND_VALUE(gstMatShine))
	{
		double shine;
		getValue(gstMatShine, shine);
		panel_.m_shine_sldr->SetValue(int(shine));
		str = wxString::Format("%.0f", shine);
		panel_.m_shine_text->ChangeValue(str);
	}
	//alpha
	if (update_all || FOUND_VALUE(gstAlpha))
	{
		double alpha;
		getValue(gstAlpha, alpha);
		panel_.m_alpha_sldr->SetValue(int(alpha * 255));
		str = wxString::Format("%.2f", alpha);
		panel_.m_alpha_text->ChangeValue(str);
	}
	//scaling
	if (update_all || FOUND_VALUE(gstScaleX))
	{
		double sx;
		getValue(gstScaleX, sx);
		panel_.m_scale_sldr->SetValue(int(sx*100.0 + 0.5));
		str = wxString::Format("%.2f", sx);
		panel_.m_scale_text->ChangeValue(str);
	}
	//shadow
	if (update_all || FOUND_VALUE(gstShadowEnable))
	{
		bool shadow_enable;
		getValue(gstShadowEnable, shadow_enable);
		panel_.m_shadow_chk->SetValue(shadow_enable);
	}
	if (update_all || FOUND_VALUE(gstShadowInt))
	{
		double shadow_int;
		getValue(gstShadowInt, shadow_int);
		panel_.m_shadow_sldr->SetValue(int(shadow_int*100.0 + 0.5));
		str = wxString::Format("%.2f", shadow_int);
		panel_.m_shadow_text->ChangeValue(str);
	}
	//size limiter
	if (update_all || FOUND_VALUE(gstLimitEnable))
	{
		bool limit_enable;
		getValue(gstLimitEnable, limit_enable);
		panel_.m_size_chk->SetValue(limit_enable);
	}
	if (update_all || FOUND_VALUE(gstLimit))
	{
		long limit;
		getValue(gstLimit, limit);
		panel_.m_size_sldr->SetValue(limit);
		panel_.m_size_text->SetValue(wxString::Format("%d", limit));
	}
}