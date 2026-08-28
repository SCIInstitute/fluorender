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

#include <CountingDlgAgent.h>
#include <CountingDlg.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>

CountingDlgAgent::CountingDlgAgent(
	CountingDlg* dlg) :
	Agent(dlg)
{

}

bool CountingDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void CountingDlgAgent::Update(
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

void CountingDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool update_all = request.values.empty();
	m_max_value = vd->GetMaxValue();

	bool bval;
	int ival;

	//selected only
	if (update_all || FOUND_VALUE(gstUseSelection))
	{
		bval = glbin_comp_generator.GetUseSel();
		dlg->UpdateUseSelection(bval);
	}
	//min voxel
	if (update_all || FOUND_VALUE(gstCountMinValue))
	{
		ival = glbin_comp_analyzer.GetMinNum();
		dlg->UpdateCountMinValue(ival);
	}
	//max voxel
	if (update_all || FOUND_VALUE(gstCountMaxValue))
	{
		ival = glbin_comp_analyzer.GetMaxNum();
		dlg->UpdateCountMaxValue(ival);
	}
	//ignore max
	if (update_all || FOUND_VALUE(gstCountUseMax))
	{
		bval = !glbin_comp_analyzer.GetUseMax();
		dlg->UpdateCountUseMax(bval);
	}
	//result
	if (FOUND_VALUE(gstCountResult))
		dlg->OutputSize();
}

void CountingDlgAgent::UpdateData(const UpdateRequest& request)
{

}

CountingDlg* CountingDlgAgent::GetDialog() const
{
	return static_cast<CountingDlg*>(GetWindow());
}
