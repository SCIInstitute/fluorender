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
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <RenderView.h>
#include <VolumeSelector.h>
#include <RulerAlign.h>
#include <Count.h>
#include <BrickTexture.h>
#include <GlobalStates.h>
#include <GridBuilder.h>
#include <MainSettings.h>

#define GM_2_ESTR(x) (1.0 - sqrt(1.0 - (x - 1.0) * (x - 1.0)))

BrushToolDlgAgent::BrushToolDlgAgent(
	BrushToolDlg* dlg) :
	Agent(dlg)
{
	m_segment_timer.setFunc([this]()
		{
			TimerSegment();
		});
}

void BrushToolDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto sel_vol = glbin_current.vol_data.lock();

	//update user interface
	bool update_all = request.values.empty() || request.HasValue(gstCurrentSelect);

	double dval = 0.0;
	int ival = 0;
	bool bval = false;
	//threshold range
	if (sel_vol)
		m_max_value = sel_vol->GetMaxValue();

	if (update_all || request.HasValue(gstSelUndo) || request.HasValue(gstCurrentSelect))
	{
		//need to fix later when pyramid is moved under volumedata
		//if (sel_vol && sel_vol->GetTexture())
		//	dlg->EnableUndo(
		//		sel_vol->GetTexture()->get_undo(),
		//		sel_vol->GetTexture()->get_redo());
		//else
			dlg->EnableUndo(false, false);
	}

	if (update_all || request.HasValue(gstFreehandToolState))
	{
		auto view = glbin_current.render_view.lock();
		InteractiveMode int_mode = view ? view->GetIntMode() : InteractiveMode::Disabled;
		flrd::SelectMode sel_mode = glbin_vol_selector.GetSelectMode();
		dlg->ToggleBrushes(int_mode, sel_mode);
	}

	if (update_all || request.HasValue(gstSelMask) || request.HasValue(gstCurrentSelect))
	{
		bval = glbin_vol_selector.GetCopyMaskVolume() != 0;
		dlg->EnableMask(bval);
	}

	if (update_all || request.HasValue(gstSelOptions))
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
	if (update_all || request.HasValue(gstBrushThreshold))
	{
		flrd::VolumeSelector* vs = &glbin_vol_selector;
		dval = glbin_vol_selector.GetBrushSclTranslate();
		dlg->UpdateBrushThreshold(dval, m_max_value);
	}

	//gm falloff
	if (update_all || request.HasValue(gstBrushGmFalloff))
	{
		dval = glbin_vol_selector.GetBrushGmFalloff();
		dlg->UpdateBrushGmFalloff(GM_2_ESTR(dval));
	}

	//2d influence
	if (update_all || request.HasValue(gstBrush2dInf))
	{
		dval = glbin_vol_selector.GetW2d();
		dlg->UpdateBrush2dInf(dval);
	}

	//size1
	if (update_all || request.HasValue(gstBrushSize1))
	{
		dval = glbin_vol_selector.GetBrushSize1();
		dlg->UpdateBrushSize1(dval);
	}

	//size2
	if (update_all || request.HasValue(gstBrushSize2))
	{
		bval = glbin_vol_selector.GetUseBrushSize2();
		dval = glbin_vol_selector.GetBrushSize2();
		dlg->UpdateBrushSize2(bval, dval);
	}

	//iteration number
	if (update_all || request.HasValue(gstBrushIter))
	{
		ival = glbin_vol_selector.GetBrushIteration();
		dlg->UpdateBrushIter(ival);
	}

	//brush size relation
	if (update_all || request.HasValue(gstBrushSizeRel))
	{
		bval = glbin_vol_selector.GetBrushSizeData();
		dlg->UpdateBrushSizeRel(bval);
	}

	//align center
	if (update_all || request.HasValue(gstAlignCenter))
	{
		bval = glbin_aligner.GetAlignCenter();
		dlg->UpdateAlignCenter(bval);
	}

	//output
	if (update_all || request.HasValue(gstBrushHistoryEnable))
	{
		dlg->UpdateBrushHistoryEnable();
	}

	bool count_result = request.HasValue(gstBrushCountResult);
	bool auto_update = request.HasValue(gstBrushCountAutoUpdate);
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
		std::string titles =
			"Voxel Count\t" \
			"Voxel Count(Int. Weighted)\t" \
			"Average Intensity\t" \
			"Physical Size\t" \
			"Physical Size(Int. Weighted)\n";
		std::wstring values;
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
		values += std::to_wstring(data.voxel_sum) + L"\t";
		values += std::to_wstring(data.voxel_wsum) + L"\t";
		values += std::to_wstring(data.avg_int) + L"\t";
		values += std::to_wstring(data.size) + unit + L"\t";
		values += std::to_wstring(data.wsize) + unit + L"\n";
		auto griddata = GridBuilder::Build(titles, ws2s(values));

		dlg->UpdateGrid(griddata);
	}

	if (request.HasValue(gstBrushSpeedResult))
	{
		if (glbin_vol_selector.m_test_speed)
		{
			std::string titles = "Time\n";
			std::string values = std::to_string(glbin_vol_selector.GetSpanSec()) + "Sec.";
			auto griddata = GridBuilder::Build(titles, values);

			dlg->UpdateGrid(griddata);
		}
	}

	if (request.HasValue(gstTimerSegment))
		m_segment_timer.start(100);

}

void BrushToolDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstSelUndo))
		SelUndo();
	if (request.HasValue(gstSelRedo))
		SelRedo();
	if (request.HasValue(gstBrushGrow))
		BrushGrow();
	if (request.HasValue(gstBrushAppend))
		BrushAppend();
	if (request.HasValue(gstBrushComp))
		BrushComp();
	if (request.HasValue(gstBrushMesh))
		BrushMesh();
	if (request.HasValue(gstBrushSingle))
		BrushSingle();
	if (request.HasValue(gstBrushDiffuse))
		BrushDiffuse();
	if (request.HasValue(gstBrushSolid))
		BrushSolid();
	if (request.HasValue(gstBrushUnsel))
		BrushUnsel();
	if (request.HasValue(gstBrushClear))
		BrushClear();
	if (request.HasValue(gstBrushExtract))
		BrushExtract();
	if (request.HasValue(gstBrushDelete))
		BrushDelete();
	if (request.HasValue(gstMaskCopy))
		MaskCopy();
	if (request.HasValue(gstMaskCopyData))
		MaskCopyData();
	if (request.HasValue(gstMaskPaste))
		MaskPaste(0);
	if (request.HasValue(gstMaskMerge))
		MaskPaste(1);
	if (request.HasValue(gstMaskExclude))
		MaskPaste(2);
	if (request.HasValue(gstMaskIntersect))
		MaskPaste(3);
	if (request.HasValue(gstAlignPca))
		AlignPca();

	if (request.HasValue(gstBrushThreshold))
		SetBrushSclTranslate();
	if (request.HasValue(gstBrushGmFalloff))
		SetBrushGmFalloff();
	if (request.HasValue(gstBrush2dInf))
		SetW2d();
	if (request.HasValue(gstEdgeDetect))
		SetEdgeDetect();
	if (request.HasValue(gstHiddenRemoval))
		SetHiddenRemoval();
	if (request.HasValue(gstSelectGroup))
		SetSelectGroup();
	if (request.HasValue(gstUpdateOrder))
		SetUpdateOrder();
	if (request.HasValue(gstBrushSize1))
		SetBrushSize1();
	if (request.HasValue(gstBrushSize2Enable))
		SetBrushSize2Enable();
	if (request.HasValue(gstBrushSize2))
		SetBrushSize2();
	if (request.HasValue(gstBrushIter))
		SetBrushIteration();
	if (request.HasValue(gstBrushSizeRel))
		SetBrushSizeData();
	if (request.HasValue(gstAlignCenter))
		SetAlignCenter();
	if (request.HasValue(gstAlignAxis))
		SetAlignAxis();
}

BrushToolDlg* BrushToolDlgAgent::GetDialog() const
{
	return static_cast<BrushToolDlg*>(GetOwner());
}

void BrushToolDlgAgent::ApplySettings()
{
	auto dialog = GetDialog();
	if (dialog)
		dialog->LoadPerspective(glbin_settings.m_layout_brush);
}

void BrushToolDlgAgent::PreShutdown()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_settings.m_layout_brush = dialog->SavePerspective();
}

void BrushToolDlgAgent::SelUndo()
{
	glbin_vol_selector.UndoMask();
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::SelRedo()
{
	glbin_vol_selector.RedoMask();
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::BrushGrow()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Grow);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushAppend()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Append);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushComp()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Segment);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushMesh()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Mesh);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushSingle()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::SingleSelect);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushDiffuse()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Diffuse);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushSolid()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Solid);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushUnsel()
{
	glbin_states.ToggleBrushMode(flrd::SelectMode::Eraser);
	NotifyDataToUI({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
}

void BrushToolDlgAgent::BrushClear()
{
	glbin_vol_selector.Clear();
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::BrushExtract()
{
	glbin_vol_selector.Extract();
	NotifyViewUpdate({ gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
}

void BrushToolDlgAgent::BrushDelete()
{
	glbin_vol_selector.Erase();
	NotifyViewUpdate({ gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
}

void BrushToolDlgAgent::MaskCopy()
{
	glbin_vol_selector.CopyMask(false);
	UpdateDataToUI({ gstSelMask });
}

void BrushToolDlgAgent::MaskCopyData()
{
	glbin_vol_selector.CopyMask(true);
	UpdateDataToUI({ gstSelMask });
}

void BrushToolDlgAgent::MaskPaste(int mode)
{
	glbin_vol_selector.PasteMask(mode);
	NotifyViewUpdate({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
}

void BrushToolDlgAgent::AlignPca()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;
	glbin_aligner.SetVolumeData(vd);
	glbin_aligner.SetView(glbin_current.render_view.lock());
	glbin_aligner.AlignPca(false);
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::TimerSegment()
{
	fluo::ValueCollection vc;

	m_segment_timer.stop();
	if (glbin_vol_selector.GetThUpdate())
		vc.insert(gstBrushThreshold);
	glbin_vol_selector.PopMask();
	glbin_vol_selector.Segment(true, false);
	vc.insert({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
	NotifyViewUpdate(vc);
}

void BrushToolDlgAgent::SetBrushSclTranslate()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetBrushSclTranslate(
			dialog->GetBrushSclTranslate() / m_max_value);
	UpdateDataToUI({ gstBrushThreshold, gstTimerSegment });
}

void BrushToolDlgAgent::SetBrushGmFalloff()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetBrushGmFalloff(
			GM_2_ESTR(dialog->GetBrushGmFalloff()));
	UpdateDataToUI({ gstBrushGmFalloff, gstTimerSegment });
}

void BrushToolDlgAgent::SetW2d()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetW2d(
			dialog->GetW2d());
	UpdateDataToUI({ gstBrush2dInf, gstTimerSegment });
}

void BrushToolDlgAgent::SetEdgeDetect()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetEdgeDetect(
			dialog->GetEdgeDetect());
	UpdateDataToUI({ gstSelOptions, gstTimerSegment });
}

void BrushToolDlgAgent::SetHiddenRemoval()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetHiddenRemoval(
			dialog->GetHiddenRemoval());
	UpdateDataToUI({ gstTimerSegment });
}

void BrushToolDlgAgent::SetSelectGroup()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetSelectGroup(
			dialog->GetSelectGroup());
	UpdateDataToUI({ gstTimerSegment });
}

void BrushToolDlgAgent::SetUpdateOrder()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetUpdateOrder(
			dialog->GetUpdateOrder());
	UpdateDataToUI({ gstTimerSegment });
}

void BrushToolDlgAgent::SetBrushSize1()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetBrushSize(
			dialog->GetBrushSize1(), -1.0);
	NotifyViewUpdate({ gstBrushSize1 });
}

void BrushToolDlgAgent::SetBrushSize2Enable()
{
	auto dialog = GetDialog();
	if (!dialog)
		return;
	bool bval = dialog->GetBrushSize2Enable();
	double dval1 = dialog->GetBrushSize1();
	double dval2 = dialog->GetBrushSize2();
	glbin_vol_selector.SetUseBrushSize2(bval);
	glbin_vol_selector.SetBrushSize(dval1, dval2);
	NotifyViewUpdate({ gstBrushSize2 });
}

void BrushToolDlgAgent::SetBrushSize2()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetBrushSize(-1.0,
			dialog->GetBrushSize2());
	NotifyViewUpdate({ gstBrushSize2 });
}

void BrushToolDlgAgent::SetBrushIteration()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetBrushIteration(
			dialog->GetBrushIteration());
	UpdateDataToUI({ gstTimerSegment });
}

void BrushToolDlgAgent::SetBrushSizeData()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_vol_selector.SetBrushSizeData(
			dialog->GetBrushSizeData());
	UpdateDataToUI({ gstTimerSegment });
}

void BrushToolDlgAgent::SetAlignCenter()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_aligner.SetAlignCenter(
			dialog->GetAlignCenter());
	NotifyViewUpdate({ gstAlignCenter });
}

void BrushToolDlgAgent::SetAlignAxis()
{
	auto dialog = GetDialog();
	if (dialog)
		glbin_aligner.SetAxisType(
			dialog->GetAlignAxis());
}
