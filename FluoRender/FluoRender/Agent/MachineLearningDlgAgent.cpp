/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <MachineLearningDlgAgent.h>
#include <MachineLearningDlg.h>
#include <Global.h>
#include <Names.h>
#include <MainSettings.h>

MachineLearningDlgAgent::MachineLearningDlgAgent(
	MachineLearningDlg* dlg) :
	Agent(dlg)
{

}

bool MachineLearningDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MachineLearningDlgAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void MachineLearningDlgAgent::UpdateUI(const UpdateRequest& request)
{
	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	//request panels to update
	UpdateRequest sub_request(request.values, this, request.mode, request.reason);
	Notify(sub_request);
}

void MachineLearningDlgAgent::UpdateData(const UpdateRequest& request)
{

}

MachineLearningDlg* MachineLearningDlgAgent::GetDialog() const
{
	return static_cast<MachineLearningDlg*>(GetWindow());
}

MachineLearningPanelAgent::MachineLearningPanelAgent(
	MachineLearningPanel* panel) :
	Agent(panel)
{

}

bool MachineLearningPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MachineLearningPanelAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void MachineLearningPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	if (update_all || FOUND_VALUE(gstMlTopList))
		panel->PopTopList();
}

void MachineLearningPanelAgent::UpdateData(const UpdateRequest& request)
{

}

MachineLearningPanel* MachineLearningPanelAgent::GetPanel() const
{
	return static_cast<MachineLearningPanel*>(GetWindow());
}

MLCompGenPanelAgent::MLCompGenPanelAgent(
	MLCompGenPanel* panel) :
	Agent(panel)
{

}

bool MLCompGenPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MLCompGenPanelAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void MLCompGenPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	bool bval;

	if (update_all ||
		FOUND_VALUE(gstMlAutoStart) ||
		FOUND_VALUE(gstMlCgAutoStart))
	{
		bval = glbin_settings.m_cg_auto_start;
		panel->SetAutoStart(bval);
	}

	if (update_all || FOUND_VALUE(gstMlAutoLoadTable))
		panel->AutoLoadTable();
}

void MLCompGenPanelAgent::UpdateData(const UpdateRequest& request)
{

}

MLCompGenPanel* MLCompGenPanelAgent::GetPanel() const
{
	return static_cast<MLCompGenPanel*>(GetWindow());
}

MLVolPropPanelAgent::MLVolPropPanelAgent(
	MLVolPropPanel* panel) :
	Agent(panel)
{

}

bool MLVolPropPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MLVolPropPanelAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void MLVolPropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	bool bval;

	if (update_all ||
		FOUND_VALUE(gstMlAutoStart) ||
		FOUND_VALUE(gstMlVpAutoStart))
	{
		bval = glbin_settings.m_vp_auto_start;
		panel->UpdateAutoStart(bval);
	}

	if (update_all || FOUND_VALUE(gstMlVpAutoApply))
	{
		bval = glbin_settings.m_vp_auto_apply;
		panel->UpdateAutoApply(bval);
	}

	if (update_all || FOUND_VALUE(gstMlAutoLoadTable))
		panel->AutoLoadTable();
}

void MLVolPropPanelAgent::UpdateData(const UpdateRequest& request)
{

}

MLVolPropPanel* MLVolPropPanelAgent::GetPanel() const
{
	return static_cast<MLVolPropPanel*>(GetWindow());
}
