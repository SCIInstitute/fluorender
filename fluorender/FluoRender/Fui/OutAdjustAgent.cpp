/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#include <Fui/OutAdjustAgent.h>
#include <Fui/OutAdjustPanel.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>

using namespace FUI;

OutAdjustAgent::OutAdjustAgent(OutAdjustPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{

}

void OutAdjustAgent::setObject(FL::Node* obj)
{
	InterfaceAgent::setObject(obj);
}

FL::Node* OutAdjustAgent::getObject()
{
	return dynamic_cast<FL::Node*>(InterfaceAgent::getObject());
}

void OutAdjustAgent::UpdateAllSettings()
{
	double dval = 0.0;
	bool bval = false;

	//values
	bool result = getValue("gamma r", dval);
	panel_.m_r_gamma_sldr->SetValue(Gamma2UiS(dval));
	panel_.m_r_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	getValue("gamma g", dval);
	panel_.m_g_gamma_sldr->SetValue(Gamma2UiS(dval));
	panel_.m_g_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	getValue("gamma b", dval);
	panel_.m_b_gamma_sldr->SetValue(Gamma2UiS(dval));
	panel_.m_b_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	getValue("brightness r", dval);
	panel_.m_r_brightness_sldr->SetValue(Brigt2UiS(dval));
	panel_.m_r_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	getValue("brightness g", dval);
	panel_.m_g_brightness_sldr->SetValue(Brigt2UiS(dval));
	panel_.m_g_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	getValue("brightness b", dval);
	panel_.m_b_brightness_sldr->SetValue(Brigt2UiS(dval));
	panel_.m_b_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	getValue("equalize r", dval);
	panel_.m_r_hdr_sldr->SetValue(Equal2UiS(dval));
	panel_.m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("equalize g", dval);
	panel_.m_g_hdr_sldr->SetValue(Equal2UiS(dval));
	panel_.m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("equalize b", dval);
	panel_.m_b_hdr_sldr->SetValue(Equal2UiS(dval));
	panel_.m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));

	//sync
	//r
	getValue("sync r", bval);
	panel_.m_sync_r_chk->ToggleTool(OutAdjustPanel::ID_SyncRChk, bval);
	panel_.m_sync_r_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncRChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	//g
	getValue("sync g", bval);
	panel_.m_sync_g_chk->ToggleTool(OutAdjustPanel::ID_SyncGChk, bval);
	panel_.m_sync_g_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncGChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	//b
	getValue("sync b", bval);
	panel_.m_sync_b_chk->ToggleTool(OutAdjustPanel::ID_SyncBChk, bval);
	panel_.m_sync_b_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncBChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));

	if (result)
		panel_.EnableAll();
	else
		panel_.DisableAll();

	//panel_.Layout();
}

void OutAdjustAgent::OnGammaRChanged(FL::Event& event)
{
	double dval;
	getValue("gamma r", dval);
	panel_.m_r_gamma_sldr->SetValue(Gamma2UiS(dval));
	panel_.m_r_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
}

void OutAdjustAgent::OnGammaGChanged(FL::Event& event)
{
	double dval;
	getValue("gamma g", dval);
	panel_.m_g_gamma_sldr->SetValue(Gamma2UiS(dval));
	panel_.m_g_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
}

void OutAdjustAgent::OnGammaBChanged(FL::Event& event)
{
	double dval;
	getValue("gamma b", dval);
	panel_.m_b_gamma_sldr->SetValue(Gamma2UiS(dval));
	panel_.m_b_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
}

void OutAdjustAgent::OnBrightnessRChanged(FL::Event& event)
{
	double dval;
	getValue("brightness r", dval);
	panel_.m_r_brightness_sldr->SetValue(Brigt2UiS(dval));
	panel_.m_r_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
}

void OutAdjustAgent::OnBrightnessGChanged(FL::Event& event)
{
	double dval;
	getValue("brightness g", dval);
	panel_.m_g_brightness_sldr->SetValue(Brigt2UiS(dval));
	panel_.m_g_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
}

void OutAdjustAgent::OnBrightnessBChanged(FL::Event& event)
{
	double dval;
	getValue("brightness b", dval);
	panel_.m_b_brightness_sldr->SetValue(Brigt2UiS(dval));
	panel_.m_b_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
}

void OutAdjustAgent::OnEqualizeRChanged(FL::Event& event)
{
	double dval;
	getValue("equalize r", dval);
	panel_.m_r_hdr_sldr->SetValue(Equal2UiS(dval));
	panel_.m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutAdjustAgent::OnEqualizeGChanged(FL::Event& event)
{
	double dval;
	getValue("equalize g", dval);
	panel_.m_g_hdr_sldr->SetValue(Equal2UiS(dval));
	panel_.m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
}

void OutAdjustAgent::OnEqualizeBChanged(FL::Event& event)
{
	double dval;
	getValue("equalize b", dval);
	panel_.m_b_hdr_sldr->SetValue(Equal2UiS(dval));
	panel_.m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
}
