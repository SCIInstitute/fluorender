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

#include <TreePanelAgent.h>
#include <TreePanel.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <RenderView.h>
#include <VolumeSelector.h>
#include <Ruler.h>
#include <RulerHandler.h>

TreePanelAgent::TreePanelAgent(
	TreePanel* panel) :
	Agent(panel)
{

}

bool TreePanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void TreePanelAgent::Update(
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

void TreePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty();
	bool bval;

	//update icons only
	if (update_all || FOUND_VALUE(gstTreeCtrl) || FOUND_VALUE(gstTreeLayerName))
		panel->UpdateTree();
	else
	{
		if (update_all || FOUND_VALUE(gstTreeIcons))
			panel->UpdateTreeIcons();
		if (update_all || FOUND_VALUE(gstTreeColors))
			panel->UpdateTreeColors();
	}

	if (update_all || FOUND_VALUE(gstCurrentSelect))
	{
		panel->UpdateTreeSel();
	}

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();
		panel->UpdateFreehandToolState(int_mode, sel_mode, rul_mode);
	}
}

void TreePanelAgent::UpdateData(const UpdateRequest& request)
{

}
