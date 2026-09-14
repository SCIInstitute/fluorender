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
#include <RenderView.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <GridBuilder.h>

CountingDlgAgent::CountingDlgAgent(
	CountingDlg* dlg) :
	Agent(dlg)
{

}

void CountingDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool update_all = request.values.empty();

	bool bval;
	int ival;

	//selected only
	if (update_all || request.HasValue(gstUseSelection))
	{
		bval = glbin_comp_generator.GetUseSel();
		dlg->UpdateUseSelection(bval);
	}
	//min voxel
	if (update_all || request.HasValue(gstCountMinValue))
	{
		ival = glbin_comp_analyzer.GetMinNum();
		dlg->UpdateCountMinValue(ival);
	}
	//max voxel
	if (update_all || request.HasValue(gstCountMaxValue))
	{
		ival = glbin_comp_analyzer.GetMaxNum();
		dlg->UpdateCountMaxValue(ival);
	}
	//ignore max
	if (update_all || request.HasValue(gstCountUseMax))
	{
		bval = !glbin_comp_analyzer.GetUseMax();
		dlg->UpdateCountUseMax(bval);
	}
	//result
	if (request.HasValue(gstCountResult))
	{
		std::string titles =
			"Components\t" \
			"Voxel Sum\t" \
			"Size\n";
		std::wstring values;
		size_t count = glbin_comp_analyzer.GetCount();
		size_t vox = glbin_comp_analyzer.GetVox();
		double size = glbin_comp_analyzer.GetSize();
		std::wstring unit;
		auto view = glbin_current.render_view.lock();
		if (!view)
			return;
		switch (view->m_sb_unit)
		{
		case 0:
			unit = L"nm\u00B3";
			break;
		case 1:
		default:
			unit = L"\u03BCm\u00B3";
			break;
		case 2:
			unit = L"mm\u00B3";
			break;
		}
		values += std::to_wstring(count) + L"\t";
		values += std::to_wstring(vox) + L"\t";
		values += std::to_wstring(size) + unit + L"\n";
		auto griddata = GridBuilder::Build(titles, ws2s(values));
		dlg->UpdateGrid(griddata);
	}
}

void CountingDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstCountAnalyze))
		Analyze();
}

CountingDlg* CountingDlgAgent::GetDialog() const
{
	return static_cast<CountingDlg*>(GetWindow());
}

void CountingDlgAgent::SetUseSelection(bool bval)
{
	glbin_comp_generator.SetUseSel(bval);
}

void CountingDlgAgent::SetMinNum(int ival)
{
	glbin_comp_analyzer.SetUseMin(true);
	glbin_comp_analyzer.SetMinNum(ival);
}

void CountingDlgAgent::SetMaxNum(int ival)
{
	glbin_comp_analyzer.SetUseMax(true);
	glbin_comp_analyzer.SetMaxNum(ival);
	UpdateDataToUI({ gstCountUseMax });
}

void CountingDlgAgent::SetUseMax(bool bval)
{
	glbin_comp_analyzer.SetUseMax(bval);
}

void CountingDlgAgent::Analyze()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	glbin_comp_generator.SetVolumeData(vd);
	glbin_comp_generator.Compute();
	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze();
	glbin_comp_analyzer.Count();

	NotifyViewUpdate({ gstCountResult, gstMaskMode });
}