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

#include <OutAdjustAgent.hpp>
#include <AgentFactory.hpp>
//#include <OutAdjustPanel.h>
#include <VolumeFactory.hpp>
//#include <png_resource.h>
#include <img/icons.h>

using namespace fluo;

OutAdjustAgent::OutAdjustAgent(OutAdjustPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{

}

void OutAdjustAgent::setupInputs()
{

}

void OutAdjustAgent::setObject(Node* obj)
{
	InterfaceAgent::setObject(obj);
}

Node* OutAdjustAgent::getObject()
{
	return dynamic_cast<Node*>(InterfaceAgent::getObject());
}

void OutAdjustAgent::UpdateFui(const ValueCollection &names)
{
/*	double dval = 0.0;
	bool bval = false;
	if (getValue(gstGammaR, dval))
		panel_.EnableAll();
	else
	{
		panel_.DisableAll();
		return;
	}

	bool update_all = names.empty();

	//values
	if (update_all || FOUND_VALUE(gstGammaR))
	{
		getValue(gstGammaR, dval);
		panel_.m_r_gamma_sldr->SetValue(Gamma2UiS(dval));
		panel_.m_r_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	}
	if (update_all || FOUND_VALUE(gstGammaG))
	{
		getValue(gstGammaG, dval);
		panel_.m_g_gamma_sldr->SetValue(Gamma2UiS(dval));
		panel_.m_g_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	}
	if (update_all || FOUND_VALUE(gstGammaB))
	{
		getValue(gstGammaB, dval);
		panel_.m_b_gamma_sldr->SetValue(Gamma2UiS(dval));
		panel_.m_b_gamma_text->ChangeValue(wxString::Format("%.2f", Gamma2UiT(dval)));
	}
	if (update_all || FOUND_VALUE(gstBrightnessR))
	{
		getValue(gstBrightnessR, dval);
		panel_.m_r_brightness_sldr->SetValue(Brigt2UiS(dval));
		panel_.m_r_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	}
	if (update_all || FOUND_VALUE(gstBrightnessG))
	{
		getValue(gstBrightnessG, dval);
		panel_.m_g_brightness_sldr->SetValue(Brigt2UiS(dval));
		panel_.m_g_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	}
	if (update_all || FOUND_VALUE(gstBrightnessB))
	{
		getValue(gstBrightnessB, dval);
		panel_.m_b_brightness_sldr->SetValue(Brigt2UiS(dval));
		panel_.m_b_brightness_text->ChangeValue(wxString::Format("%.0f", Brigt2UiT(dval)));
	}
	if (update_all || FOUND_VALUE(gstEqualizeR))
	{
		getValue(gstEqualizeR, dval);
		panel_.m_r_hdr_sldr->SetValue(Equal2UiS(dval));
		panel_.m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	}
	if (update_all || FOUND_VALUE(gstEqualizeG))
	{
		getValue(gstEqualizeG, dval);
		panel_.m_g_hdr_sldr->SetValue(Equal2UiS(dval));
		panel_.m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	}
	if (update_all || FOUND_VALUE(gstEqualizeB))
	{
		getValue(gstEqualizeB, dval);
		panel_.m_b_hdr_sldr->SetValue(Equal2UiS(dval));
		panel_.m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	}

	//sync
	//r
	if (update_all || FOUND_VALUE(gstSyncR))
	{
		getValue(gstSyncR, bval);
		panel_.m_sync_r_chk->ToggleTool(OutAdjustPanel::ID_SyncRChk, bval);
		panel_.m_sync_r_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncRChk,
			bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	}
	//g
	if (update_all || FOUND_VALUE(gstSyncG))
	{
		getValue(gstSyncG, bval);
		panel_.m_sync_g_chk->ToggleTool(OutAdjustPanel::ID_SyncGChk, bval);
		panel_.m_sync_g_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncGChk,
			bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	}
	//b
	if (update_all || FOUND_VALUE(gstSyncB))
	{
		getValue(gstSyncB, bval);
		panel_.m_sync_b_chk->ToggleTool(OutAdjustPanel::ID_SyncBChk, bval);
		panel_.m_sync_b_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncBChk,
			bval ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	}

	//panel_.Layout();
*/}

void OutAdjustAgent::ResetRed()
{
	fluo::ValueCollection names{ gstGammaR, gstBrightnessR, gstEqualizeR };
	glbin_volf->propValuesFromDefault(this, names);
}

void OutAdjustAgent::ResetGreen()
{
	fluo::ValueCollection names{ gstGammaB, gstBrightnessB, gstEqualizeB };
	glbin_volf->propValuesFromDefault(this, names);
}

void OutAdjustAgent::ResetBlue()
{
	fluo::ValueCollection names{ gstGammaB, gstBrightnessB, gstEqualizeB };
	glbin_volf->propValuesFromDefault(this, names);
}

void OutAdjustAgent::SaveDefault()
{
	std::string ss[] = {
		gstGammaR, gstGammaG, gstGammaB,
		gstBrightnessR, gstBrightnessG, gstBrightnessB,
		gstEqualizeR, gstEqualizeG, gstEqualizeB };
	fluo::ValueCollection names(std::begin(ss), std::end(ss));//values to save
	glbin_volf->propValuesToDefault(this, names);
	glbin_volf->writeDefault(names);
}

void OutAdjustAgent::OnGammaRChanged(Event& event)
{
}

void OutAdjustAgent::OnGammaGChanged(Event& event)
{
}

void OutAdjustAgent::OnGammaBChanged(Event& event)
{
}

void OutAdjustAgent::OnBrightnessRChanged(Event& event)
{
}

void OutAdjustAgent::OnBrightnessGChanged(Event& event)
{
}

void OutAdjustAgent::OnBrightnessBChanged(Event& event)
{
}

void OutAdjustAgent::OnEqualizeRChanged(Event& event)
{
}

void OutAdjustAgent::OnEqualizeGChanged(Event& event)
{
}

void OutAdjustAgent::OnEqualizeBChanged(Event& event)
{
}
