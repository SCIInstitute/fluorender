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

#include <ColocalizationDlgAgent.h>
#include <ColocalizationDlg.h>
#include <Global.h>
#include <Names.h>
#include <ColocalDefault.h>
#include <Colocalize.h>

ColocalizationDlgAgent::ColocalizationDlgAgent(
	ColocalizationDlg* dlg) :
	Agent(dlg)
{

}

bool ColocalizationDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void ColocalizationDlgAgent::Update(
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

void ColocalizationDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	//settings
	if (update_all || FOUND_VALUE(gstColocalMethod))
	{
		dlg->UpdateColocalMethod(glbin_colocal_def.m_method);
	}

	if (update_all || FOUND_VALUE(gstIntWeighted))
	{
		dlg->UpdateIntWeighted(glbin_colocal_def.m_int_weighted);
	}

	if (update_all || FOUND_VALUE(gstGetRatio))
	{
		dlg->UpdateGetRatio(glbin_colocal_def.m_get_ratio);
	}

	if (update_all || FOUND_VALUE(gstPhysSize))
	{
		dlg->UpdatePhysicalSize(glbin_colocal_def.m_physical_size);
	}

	if (update_all || FOUND_VALUE(gstColocalColormap))
	{
		dlg->UpdateColocalColormap(glbin_colocal_def.m_colormap);
	}

	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		dlg->UpdateUseSelection(glbin_colocal_def.m_use_mask);
	}

	bool colocal_update = false;
	bool colocal_result = FOUND_VALUE(gstColocalResult);
	bool auto_update = FOUND_VALUE(gstColocalAutoUpdate);
	if (update_all || auto_update)
	{
		if (auto_update)
			colocal_update = glbin_colocalizer.GetAutoColocalize();
	}
	if (colocal_result || colocal_update)
	{
		glbin_colocalizer.Compute();
		dlg->SetOutput();
	}
}

void ColocalizationDlgAgent::UpdateData(const UpdateRequest& request)
{

}

ColocalizationDlg* ColocalizationDlgAgent::GetDialog() const
{
	return static_cast<ColocalizationDlg*>(GetWindow());
}
