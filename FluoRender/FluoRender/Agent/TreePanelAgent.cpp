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
		UpdateTree();
	else
	{
		if (update_all || FOUND_VALUE(gstTreeIcons))
			UpdateTreeIcons();
		if (update_all || FOUND_VALUE(gstTreeColors))
			UpdateTreeColors();
	}

	if (update_all || FOUND_VALUE(gstCurrentSelect))
	{
		UpdateTreeSel();
	}

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		flrd::RulerMode rul_mode = glbin_ruler_handler.GetRulerMode();

		m_toolbar->ToggleTool(ID_RulerLocator, rul_mode == flrd::RulerMode::Locator);
		m_toolbar->ToggleTool(ID_RulerLine, rul_mode == flrd::RulerMode::Line);
		bval = rul_mode == flrd::RulerMode::Polyline &&
			(int_mode == InteractiveMode::Ruler ||
				int_mode == InteractiveMode::BrushRuler);
		m_toolbar->ToggleTool(ID_RulerPolyline, bval);
		m_toolbar->ToggleTool(ID_RulerPencil, int_mode == InteractiveMode::Pencil);
		m_toolbar->ToggleTool(ID_RulerEdit, int_mode == InteractiveMode::EditRulerPoint);
		m_toolbar->ToggleTool(ID_RulerDeletePoint, int_mode == InteractiveMode::RulerDelPoint);

		bval = rul_mode == flrd::RulerMode::Locator &&
			sel_mode == flrd::SelectMode::SingleSelect;
		m_toolbar2->ToggleTool(ID_BrushRuler, bval);
		m_toolbar2->ToggleTool(ID_BrushGrow, sel_mode == flrd::SelectMode::Grow);
		m_toolbar2->ToggleTool(ID_BrushAppend, sel_mode == flrd::SelectMode::Append);
		m_toolbar2->ToggleTool(ID_BrushComp, sel_mode == flrd::SelectMode::Segment);
		m_toolbar2->ToggleTool(ID_BrushDiffuse, sel_mode == flrd::SelectMode::Diffuse);
		m_toolbar2->ToggleTool(ID_BrushUnselect, sel_mode == flrd::SelectMode::Eraser);
	}
}

void TreePanelAgent::UpdateData(const UpdateRequest& request)
{

}
