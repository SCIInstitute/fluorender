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

#include <ManipPropPanelAgent.h>
#include <ManipPropPanel.h>
#include <MeshData.h>
#include <Names.h>

ManipPropPanelAgent::ManipPropPanelAgent(
	ManipPropPanel* panel) :
	Agent(panel)
{

}

bool ManipPropPanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void ManipPropPanelAgent::Update(
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

void ManipPropPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;
	auto md = GetData();
	if (!md)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	fluo::Vector vval;

	if (update_all || FOUND_VALUE(gstMeshTranslation))
	{
		vval = md->GetTranslation();
		panel->UpdateMeshTranslation(vval);
	}
	if (update_all || FOUND_VALUE(gstMeshRotation))
	{
		vval = md->GetRotation();
		panel->UpdateMeshRotation(vval);
	}
	if (update_all || FOUND_VALUE(gstMeshScale))
	{
		vval = md->GetScaling();
		panel->UpdateMeshScale(vval);
	}
}

void ManipPropPanelAgent::UpdateData(const UpdateRequest& request)
{

}
