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

void OutAdjustAgent::objectChanging(void* ptr, void* orig_node, const std::string &exp)
{
	//before change
}

void OutAdjustAgent::objectChanged(void* ptr, void* orig_node, const std::string &exp)
{
	//set values in ui
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
	bool sync_r = false;
	bool sync_g = false;
	bool sync_b = false;
	double dval = 0.0;
	bool bval = false;

	//red
	panel_.m_sync_r_chk->ToggleTool(OutAdjustPanel::ID_SyncRChk, sync_r);
	panel_.m_sync_r_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncRChk,
		sync_r ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	panel_.m_sync_r = sync_r;
	bval = getValue("gamma r", dval);
	panel_.m_r_gamma_sldr->SetValue(Gamma2UIP(dval));
	panel_.m_r_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("brightness r", dval);
	panel_.m_r_brightness_sldr->SetValue(Brightness2UIP(dval));
	panel_.m_r_brightness_text->ChangeValue(wxString::Format("%d", Brightness2UIP(dval)));
	getValue("equalize r", dval);
	panel_.m_r_hdr_sldr->SetValue(Hdr2UIP(dval));
	panel_.m_r_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	//green
	panel_.m_sync_g_chk->ToggleTool(OutAdjustPanel::ID_SyncGChk, sync_g);
	panel_.m_sync_g_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncGChk,
		sync_g ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	panel_.m_sync_g = sync_g;
	getValue("gamma g", dval);
	panel_.m_g_gamma_sldr->SetValue(Gamma2UIP(dval));
	panel_.m_g_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("brightness g", dval);
	panel_.m_g_brightness_sldr->SetValue(Brightness2UIP(dval));
	panel_.m_g_brightness_text->ChangeValue(wxString::Format("%d", int(dval)));
	getValue("equalize g", dval);
	panel_.m_g_hdr_sldr->SetValue(Hdr2UIP(dval));
	panel_.m_g_hdr_text->ChangeValue(wxString::Format("%.2f", dval));
	//blue
	panel_.m_sync_b_chk->ToggleTool(OutAdjustPanel::ID_SyncBChk, sync_b);
	panel_.m_sync_b_chk->SetToolNormalBitmap(OutAdjustPanel::ID_SyncBChk,
		sync_b ? wxGetBitmapFromMemory(link) : wxGetBitmapFromMemory(unlink));
	panel_.m_sync_b = sync_b;
	getValue("gamma b", dval);
	panel_.m_b_gamma_sldr->SetValue(Gamma2UIP(dval));
	panel_.m_b_gamma_text->ChangeValue(wxString::Format("%.2f", dval));
	getValue("brightness b", dval);
	panel_.m_b_brightness_sldr->SetValue(Brightness2UIP(dval));
	panel_.m_b_brightness_text->ChangeValue(wxString::Format("%d", Brightness2UIP(dval)));
	getValue("equalize b", dval);
	panel_.m_b_hdr_sldr->SetValue(Hdr2UIP(dval));
	panel_.m_b_hdr_text->ChangeValue(wxString::Format("%.2f", dval));

	if (bval)
		panel_.EnableAll();
	else
		panel_.DisableAll();

	//panel_.Layout();
}