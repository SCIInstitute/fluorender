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

#include "outAdjustAgent.hpp"
#include <Panels/Output_Adjustments/outputAdjust.hpp>

/*
#include <Fui/OutAdjustPanel.h>
#include <wx/valnum.h>
#include <png_resource.h>
#include <img/icons.h>
*/

OutAdjustAgent::OutAdjustAgent(OutputAdjustments *panel) :
	InterfaceAgent(),
	parentPanel(panel)
{

}

void OutAdjustAgent::setObject(fluo::VolumeData* obj)
{
	InterfaceAgent::setObject(obj);
}

fluo::VolumeData* OutAdjustAgent::getObject()
{
	return dynamic_cast<fluo::VolumeData*>(InterfaceAgent::getObject());
}

void OutAdjustAgent::UpdateAllSettings()
{
	double dval = 0.0;
	bool bval = false;

	//values
	//bool result = getValue("gamma r", dval);
	getValue("gamma r", dval);
    parentPanel->setOutRedGammaValue(dval);
	getValue("gamma g", dval);
    parentPanel->setOutGreenGammaValue(dval);
    getValue("gamma b", dval);
    parentPanel->setOutBlueGammaValue(dval);
    
	getValue("brightness r", dval);
    parentPanel->setOutRedLuminValue(dval);
	getValue("brightness g", dval);
    parentPanel->setOutGreenLuminValue(dval);
	getValue("brightness b", dval);
    parentPanel->setOutBlueLuminValue(dval);

	getValue("equalize r", dval);
    parentPanel->setOutRedEqlValue(dval);
	getValue("equalize g", dval);
    parentPanel->setOutGreenEqlValue(dval);
	getValue("equalize b", dval);
    parentPanel->setOutBlueEqlValue(dval);
/*
	parentPanel->m_r_gamma_sldr->SetValue(Gamma2UiS(dval));
	parentPanel->m_r_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	getValue("gamma g", dval);
	parentPanel->m_g_gamma_sldr->SetValue(Gamma2UiS(dval));
	parentPanel->m_g_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	getValue("gamma b", dval);
	parentPanel->m_b_gamma_sldr->SetValue(Gamma2UiS(dval));
	parentPanel->m_b_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	getValue("brightness r", dval);
	parentPanel->m_r_brightness_sldr->SetValue(Brigt2UiS(dval));
	parentPanel->m_r_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	getValue("brightness g", dval);
	parentPanel->m_g_brightness_sldr->SetValue(Brigt2UiS(dval));
	parentPanel->m_g_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	getValue("brightness b", dval);
	parentPanel->m_b_brightness_sldr->SetValue(Brigt2UiS(dval));
	parentPanel->m_b_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	getValue("equalize r", dval);
	parentPanel->m_r_hdr_sldr->SetValue(Equal2UiS(dval));
	parentPanel->m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("equalize g", dval);
	parentPanel->m_g_hdr_sldr->SetValue(Equal2UiS(dval));
	parentPanel->m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("equalize b", dval);
	parentPanel->m_b_hdr_sldr->SetValue(Equal2UiS(dval));
	parentPanel->m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));

	//sync
	//r
	getValue("sync r", bval);
	parentPanel->m_sync_r_chk->ToggleTool(OutAdjustPanel::ID_SyncRChk, bval);
	parentPanel->m_sync_r_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncRChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	//g
	getValue("sync g", bval);
	parentPanel->m_sync_g_chk->ToggleTool(OutAdjustPanel::ID_SyncGChk, bval);
	parentPanel->m_sync_g_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncGChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	//b
	getValue("sync b", bval);
	parentPanel->m_sync_b_chk->ToggleTool(OutAdjustPanel::ID_SyncBChk, bval);
	parentPanel->m_sync_b_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncBChk,
		bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));

	if (result)
		parentPanel->EnableAll();
	else
		parentPanel->DisableAll();

	//parentPanel->Layout();

*/
}

void OutAdjustAgent::OnGammaRChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("gamma r", dval);
    parentPanel->setOutRedGammaValue(dval);
}

void OutAdjustAgent::OnGammaGChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("gamma g", dval);
    parentPanel->setOutGreenGammaValue(dval);
}

void OutAdjustAgent::OnGammaBChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("gamma b", dval);
    parentPanel->setOutBlueGammaValue(dval);
}

void OutAdjustAgent::OnBrightnessRChanged(fluo::Event& event)
{
  double dval = 0.0;
  getValue("brightness r", dval);
  parentPanel->setOutRedLuminValue(dval);

}

void OutAdjustAgent::OnBrightnessGChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("brightness g", dval);
    parentPanel->setOutGreenLuminValue(dval);
}

void OutAdjustAgent::OnBrightnessBChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("brightness b", dval);
    parentPanel->setOutBlueLuminValue(dval);
}

void OutAdjustAgent::OnEqualizeRChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("equalize r", dval);
    parentPanel->setOutRedEqlValue(dval);
}

void OutAdjustAgent::OnEqualizeGChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("equalize g", dval);
    parentPanel->setOutGreenEqlValue(dval);
}

void OutAdjustAgent::OnEqualizeBChanged(fluo::Event& event)
{
	double dval = 0.0;
	getValue("equalize b", dval);
    parentPanel->setOutBlueEqlValue(dval);
}
