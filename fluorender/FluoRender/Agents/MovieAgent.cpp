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

#include <MovieAgent.hpp>
#include <MoviePanel.h>
#include <Global.hpp>
#include <AgentFactory.hpp>

using namespace fluo;

MovieAgent::MovieAgent(MoviePanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
}

void MovieAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
	std::string name = obj->getName();
	int i = panel_.m_views_cmb->FindString(name);
	panel_.m_views_cmb->SetSelection(i);
}

Renderview* MovieAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void MovieAgent::UpdateAllSettings()
{
	bool bval;
	getValue(gstMovTimeSeqEnable, bval);

	SetCurrentTime(m_start_frame);
	bool bval;
	m_view->getValue(gstDrawCropFrame, bval);
	SetCrop(bval);
	AddScriptToList();
	GetScriptSettings();
	glbin_agtf->findFirst(gstRecorderAgent)->asRecorderAgent()->UpdateAllSettings();
}

void MovieAgent::OnMovTimeSeqEnable(Event& event)
{
	bool bval;
	long lval;
	double dval;
	getValue(gstMovTimeSeqEnable, bval);
	if (bval)
	{
		getValue(gstMovSeqMode, lval);
		if (lval == 0)
			setValue(gstMovSeqMode, long(1));
	}
	else
	{
		panel_.m_seq_chk->SetValue(false);
		panel_.m_bat_chk->SetValue(false);
		setValue(gstBeginFrame, long(0));
		getValue(gstMovRotAng, dval);
		setValue(gstEndFrame, long(dval));
	}
	long bf, ef;
	double fps;
	getValue(gstBeginFrame, bf);
	getValue(gstEndFrame, ef);
	getValue(gstMovFps, fps);
	dval = (ef - bf + 1) / fps;
	setValue(gstMovLength, dval);
	panel_.m_start_frame_text->ChangeValue(wxString::Format("%d", bf));
	panel_.m_end_frame_text->ChangeValue(wxString::Format("%d", ef));
	panel_.m_movie_len_text->ChangeValue(wxString::Format("%.2f", dval));
}

void MovieAgent::OnMovSeqMode(Event& event)
{
	long lval;
	getValue(gstMovSeqMode, lval);
	if (lval == 1)
	{
		panel_.m_seq_chk->SetValue(true);
		panel_.m_bat_chk->SetValue(false);
		getObject()->Get4DSeqRange();
	}
	else if (lval == 2)
	{
		panel_.m_seq_chk->SetValue(false);
		panel_.m_bat_chk->SetValue(true);
		getObject()->Get3DBatRange();
	}
}

void MovieAgent::OnMovRotEnable(Event& event)
{
	bool bval;
	getValue(gstMovRotEnable, bval);
	panel_.m_x_rd->Enable(bval);
	panel_.m_y_rd->Enable(bval);
	panel_.m_z_rd->Enable(bval);
	panel_.m_degree_text->Enable(bval);
	panel_.m_rot_chk->SetValue(bval);
}

void MovieAgent::OnMovRotAxis(Event& event)
{
	long lval;
	getValue(gstMovRotAxis, lval);
	if (lval == 0)
		panel_.m_x_rd->SetValue(true);
	else if (lval == 1)
		panel_.m_y_rd->SetValue(true);
	else if (lval == 2)
		panel_.m_z_rd->SetValue(true);
}

void MovieAgent::OnMovRotAng(Event& event)
{
	double dval;
	getValue(gstMovRotAng, dval);
	panel_.m_degree_text->SetValue(wxString::Format("%d", dval));
}
