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
#include <GridBuilder.h>

ColocalizationDlgAgent::ColocalizationDlgAgent(
	ColocalizationDlg* dlg) :
	Agent(dlg)
{

}

void ColocalizationDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	bool update_all = request.values.empty();

	//settings
	if (update_all || request.HasValue(gstColocalMethod))
	{
		dlg->UpdateColocalMethod(glbin_colocal_def.m_method);
	}

	if (update_all || request.HasValue(gstIntWeighted))
	{
		dlg->UpdateIntWeighted(glbin_colocal_def.m_int_weighted);
	}

	if (update_all || request.HasValue(gstGetRatio))
	{
		dlg->UpdateGetRatio(glbin_colocal_def.m_get_ratio);
	}

	if (update_all || request.HasValue(gstPhysSize))
	{
		dlg->UpdatePhysicalSize(glbin_colocal_def.m_physical_size);
	}

	if (update_all || request.HasValue(gstColocalColormap))
	{
		dlg->UpdateColocalColormap(glbin_colocal_def.m_colormap);
	}

	if (update_all || request.HasValue(gstUseSelection))
	{
		dlg->UpdateUseSelection(glbin_colocal_def.m_use_mask);
	}

	bool colocal_result = request.HasValue(gstColocalResult);
	if (update_all || colocal_result)
	{
		SetOutput();
	}
}

void ColocalizationDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstColocalResult))
	{
		Colocalization();
	}
}

ColocalizationDlg* ColocalizationDlgAgent::GetDialog() const
{
	return static_cast<ColocalizationDlg*>(GetWindow());
}

void ColocalizationDlgAgent::SetUseSelection(bool bval)
{
	glbin_colocal_def.m_use_mask = bval;
	bool auto_update = glbin_colocalizer.GetAutoColocalize();
	if (auto_update)
	{
		UpdateUIToData({ gstColocalResult });
	}
}

void ColocalizationDlgAgent::SetMethod(int ival)
{
	glbin_colocal_def.m_method = ival;
	bool auto_update = glbin_colocalizer.GetAutoColocalize();
	if (auto_update)
	{
		UpdateUIToData({ gstColocalResult });
	}
}

void ColocalizationDlgAgent::SetInWeight(bool bval)
{
	glbin_colocal_def.m_int_weighted = bval;
	bool auto_update = glbin_colocalizer.GetAutoColocalize();
	if (auto_update)
	{
		UpdateUIToData({ gstColocalResult });
	}
}

void ColocalizationDlgAgent::SetRatio(bool bval)
{
	glbin_colocal_def.m_get_ratio = bval;
	bool auto_update = glbin_colocalizer.GetAutoColocalize();
	if (auto_update)
	{
		UpdateUIToData({ gstColocalResult });
	}
}

void ColocalizationDlgAgent::SetPhysical(bool bval)
{
	glbin_colocal_def.m_physical_size = bval;
	bool auto_update = glbin_colocalizer.GetAutoColocalize();
	if (auto_update)
	{
		UpdateUIToData({ gstColocalResult });
	}
}

void ColocalizationDlgAgent::SetColormap(bool bval)
{
	glbin_colocal_def.m_colormap = bval;
	bool auto_update = glbin_colocalizer.GetAutoColocalize();
	if (auto_update)
	{
		UpdateUIToData({ gstColocalResult });
	}
}

void ColocalizationDlgAgent::Colocalization()
{
	glbin_colocalizer.Compute();
	UpdateDataToUI({ gstColocalResult });
}

void ColocalizationDlgAgent::SetOutput()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	std::wstring titles = glbin_colocalizer.GetTitles();
	std::wstring values = glbin_colocalizer.GetValues();
	auto griddata = GridBuilder::Build(ws2s(titles), ws2s(values));
	GridFormatter::ApplyColocalizeColors(griddata);

	dlg->UpdateGrid(griddata);
}