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

#include <CalculationDlgAgent.h>
#include <CalculationDlg.h>
#include <Global.h>
#include <Names.h>
#include <VolumeCalculator.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <RenderView.h>
#include <DataManager.h>
#include <CurrentObjects.h>
#include <CombineList.h>

CalculationDlgAgent::CalculationDlgAgent(
	CalculationDlg* dlg) :
	Agent(dlg)
{

}

bool CalculationDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void CalculationDlgAgent::Update(
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

void CalculationDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	bool update_all = request.values.empty();

	std::wstring str;
	if (update_all || request.HasValue(gstVolumeA))
	{
		auto vd = glbin_vol_calculator.GetVolumeA();
		if (vd)
		{
			str = vd->GetName();
			dlg->UpdateVolumeA(str);
		}
	}

	if (update_all || request.HasValue(gstVolumeB))
	{
		auto vd = glbin_vol_calculator.GetVolumeB();
		if (vd)
		{
			str = vd->GetName();
			dlg->UpdateVolumeB(str);
		}
	}
}

void CalculationDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstLoadVolumeA))
	{
		glbin_vol_calculator.SetVolumeA(
			glbin_current.vol_data.lock());
	}
	if (request.HasValue(gstLoadVolumeB))
	{
		glbin_vol_calculator.SetVolumeB(
			glbin_current.vol_data.lock());
	}
	if (request.HasValue(gstCalcSub))
	{
		glbin_vol_calculator.CalculateGroup(1);
	}
	if (request.HasValue(gstCalcAdd))
	{
		glbin_vol_calculator.CalculateGroup(2);
	}
	if (request.HasValue(gstCalcDiv))
	{
		glbin_vol_calculator.CalculateGroup(3);
	}
	if (request.HasValue(gstCalcFill))
	{
		glbin_vol_calculator.SetVolumeB(0);
		glbin_vol_calculator.CalculateGroup(9);
	}
	if (request.HasValue(gstCalcCombine))
	{
		CombineVolumes();
		NotifyViewUpdate({ gstVolumePropPanel, gstListCtrl, gstTreeCtrl, gstCurrentSelect, gstUpdateSync });
	}
}

CalculationDlg* CalculationDlgAgent::GetDialog() const
{
	return static_cast<CalculationDlg*>(GetWindow());
}

void CalculationDlgAgent::CombineVolumes()
{
	auto group = glbin_current.vol_group.lock();
	if (!group)
		return;
	auto view = glbin_current.render_view.lock();
	if (!view)
		return;

	flrd::CombineList Op;
	std::wstring name = group->GetName() + L"_combined";
	Op.SetName(name);
	std::list<std::weak_ptr<VolumeData>> channs;
	for (int i = 0; i < group->GetVolumeNum(); ++i)
	{
		auto vd = group->GetVolumeData(i);
		if (!vd)
			continue;
		channs.push_back(vd);
	}
	if (channs.empty())
		return;

	Op.SetVolumes(channs);
	if (!Op.Execute())
		return;

	auto results = Op.GetResults();
	if (results.empty())
		return;

	std::wstring group_name = L"";
	group = 0;
	std::shared_ptr<VolumeData> volume;
	for (auto it = results.begin(); it != results.end(); ++it)
	{
		auto vd = *it;
		if (vd)
		{
			if (!volume) volume = vd;
			glbin_data_manager.AddVolumeData(vd);
			if (it == results.begin())
			{
				group_name = view->AddGroup(L"");
				group = view->GetGroup(group_name);
			}
			view->AddVolumeData(vd, group_name);
		}
	}
	if (group && volume)
	{
		fluo::Color col = volume->GetGammaColor();
		group->SetGammaAll(col);
		col = volume->GetBrightness();
		group->SetBrightnessAll(col);
		col = volume->GetHdr();
		group->SetHdrAll(col);
	}
	glbin_current.SetVolumeGroup(group);
}