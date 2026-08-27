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

#include <BrushToolDlgAgent.h>
#include <BrushToolDlg.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <RenderView.h>
#include <VolumeSelector.h>
#include <RulerAlign.h>
#include <Count.h>

#define GM_2_ESTR(x) (1.0 - sqrt(1.0 - (x - 1.0) * (x - 1.0)))

BrushToolDlgAgent::BrushToolDlgAgent(
	BrushToolDlg* dlg) :
	Agent(dlg)
{

}

bool BrushToolDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
	//return
	//	FOUND_VALUE(gstAnnotMemoText) ||
	//	FOUND_VALUE(gstAnnotMemoReadOnly);
}

void BrushToolDlgAgent::Update(
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

void BrushToolDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel_vol = glbin_current.vol_data.lock();

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty() || FOUND_VALUE(gstCurrentSelect);

	double dval = 0.0;
	int ival = 0;
	bool bval = false;
	//threshold range
	if (sel_vol)
		m_max_value = sel_vol->GetMaxValue();

	if (update_all || FOUND_VALUE(gstSelUndo) || FOUND_VALUE(gstCurrentSelect))
	{
		if (sel_vol && sel_vol->GetTexture())
			dlg->EnableUndo(
				sel_vol->GetTexture()->get_undo(),
				sel_vol->GetTexture()->get_redo());
		else
			dlg->EnableUndo(false, false);
	}

	if (update_all || FOUND_VALUE(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		dlg->ToggleBrushes(int_mode, sel_mode);
	}

	if (update_all || FOUND_VALUE(gstSelMask) || FOUND_VALUE(gstCurrentSelect))
	{
		bval = glbin_vol_selector.GetCopyMaskVolume() != 0;
		dlg->EnableMask(bval);
	}

	if (update_all || FOUND_VALUE(gstSelOptions))
	{
		//edge detect
		bval = glbin_vol_selector.GetEdgeDetect();
		dlg->UpdateEdgeDetect(bval);
		//hidden removal
		bval = glbin_vol_selector.GetHiddenRemoval();
		dlg->UpdateHiddenRemoval(bval);
		//select group
		bval = glbin_vol_selector.GetSelectGroup();
		dlg->UpdateSelectGroup(bval);
		//brick acuracy
		bval = glbin_vol_selector.GetUpdateOrder();
		dlg->UpdateUpdateOrder(bval);
	}

	//selection strength
	if (update_all || FOUND_VALUE(gstBrushThreshold))
	{
		flrd::VolumeSelector* vs = &glbin_vol_selector;
		dval = glbin_vol_selector.GetBrushSclTranslate();
		dlg->UpdateBrushThreshold(dval, m_max_value);
	}

	//gm falloff
	if (update_all || FOUND_VALUE(gstBrushGmFalloff))
	{
		dval = glbin_vol_selector.GetBrushGmFalloff();
		dlg->UpdateBrushGmFalloff(GM_2_ESTR(dval));
	}

	//2d influence
	if (update_all || FOUND_VALUE(gstBrush2dInf))
	{
		dval = glbin_vol_selector.GetW2d();
		dlg->UpdateBrush2dInf(dval);
	}

	//size1
	if (update_all || FOUND_VALUE(gstBrushSize1))
	{
		dval = glbin_vol_selector.GetBrushSize1();
		dlg->UpdateBrushSize1(dval);
	}

	//size2
	if (update_all || FOUND_VALUE(gstBrushSize2))
	{
		bval = glbin_vol_selector.GetUseBrushSize2();
		dval = glbin_vol_selector.GetBrushSize2();
		dlg->UpdateBrushSize2(bval, dval);
	}

	//iteration number
	if (update_all || FOUND_VALUE(gstBrushIter))
	{
		ival = glbin_vol_selector.GetBrushIteration();
		dlg->UpdateBrushIter(ival);
	}

	//brush size relation
	if (update_all || FOUND_VALUE(gstBrushSizeRel))
	{
		bval = glbin_vol_selector.GetBrushSizeData();
		dlg->UpdateBrushSizeRel(bval);
	}

	//align center
	if (update_all || FOUND_VALUE(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		dlg->UpdateAlignCenter(bval);
	}

	//output
	if (update_all || FOUND_VALUE(gstBrushHistoryEnable))
	{
		dlg->UpdateBrushHistoryEnable();
	}

	bool count_result = FOUND_VALUE(gstBrushCountResult);
	bool auto_update = FOUND_VALUE(gstBrushCountAutoUpdate);
	bool count_update = false;
	if (sel_vol &&
		(count_result ||
			auto_update))
	{
		if (auto_update)
			count_update = glbin_vol_selector.GetAutoPaintSize();
		else
			count_update = true;
	}
	if (count_update)
	{
		BrushGridData data;
		flrd::CountVoxels counter;
		counter.SetVolumeData(sel_vol);
		counter.Count();
		data.voxel_sum = counter.GetSum();
		double scale = sel_vol->GetScalarScale();
		data.voxel_wsum = counter.GetWeightedSum() * scale;
		if (data.voxel_sum)
		{
			data.avg_int = data.voxel_wsum / data.voxel_sum;
			if (sel_vol->GetBits() == 8)
				data.avg_int *= 255.0;
			else if (sel_vol->GetBits() == 16)
				data.avg_int *= sel_vol->GetMaxValue();
		}
		auto spc = sel_vol->GetSpacing();
		double vvol = spc.x() * spc.y() * spc.z();
		vvol = vvol == 0.0 ? 1.0 : vvol;
		data.size = data.voxel_sum * vvol;
		data.wsize = data.voxel_wsum * vvol;
		std::wstring unit;
		auto view = glbin_current.render_view.lock();
		if (view)
		{
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
		}
		dlg->SetOutput(data, unit);
	}

	if (FOUND_VALUE(gstBrushSpeedResult))
	{
		if (glbin_vol_selector.m_test_speed)
		{
			BrushGridData data;
			data.size = glbin_vol_selector.GetSpanSec();
			data.wsize = data.size;
			std::wstring unit = L"Sec.";
			dlg->SetOutput(data, unit);
		}
	}
}

void BrushToolDlgAgent::UpdateData(const UpdateRequest& request)
{

}

BrushToolDlg* BrushToolDlgAgent::GetDialog() const
{
	return static_cast<BrushToolDlg*>(GetWindow());
}
