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

#define GM_2_ESTR(x) (1.0 - sqrt(1.0 - (x - 1.0) * (x - 1.0)))

BrushToolDlgAgent::BrushToolDlgAgent(
	BrushToolDlg* dlg) :
	Agent(dlg)
{

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
			"Physical Size\n";
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
}

void BrushToolDlgAgent::UpdateData(const UpdateRequest& request)
{
	fluo::ValueCollection vc = request.values;
	if (request.HasValue(gstSelUndo))
	{
		glbin_vol_selector.UndoMask();
	}
	if (request.HasValue(gstSelRedo))
	{
		glbin_vol_selector.RedoMask();
	}
	if (request.HasValue(gstBrushGrow))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Grow);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushAppend))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Append);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushComp))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Segment);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushMesh))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Mesh);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushSingle))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::SingleSelect);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushDiffuse))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Diffuse);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushSolid))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Solid);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushUnsel))
	{
		glbin_states.ToggleBrushMode(flrd::SelectMode::Eraser);
		vc.insert({ gstFreehandToolState, gstBrushSize1, gstBrushSize2, gstBrushIter });
	}
	if (request.HasValue(gstBrushClear))
	{
		glbin_vol_selector.Clear();
		vc.insert({ gstNull });
	}
	if (request.HasValue(gstBrushExtract))
	{
		glbin_vol_selector.Extract();
		vc.insert({ gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
	}
	if (request.HasValue(gstBrushDelete))
	{
		glbin_vol_selector.Erase();
		vc.insert({ gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
	}
	if (request.HasValue(gstMaskCopy))
	{
		glbin_vol_selector.CopyMask(false);
		vc.insert({ gstSelMask });
	}
	if (request.HasValue(gstMaskCopyData))
	{
		glbin_vol_selector.CopyMask(true);
		vc.insert({ gstSelMask });
	}
	if (request.HasValue(gstMaskPaste))
	{
		glbin_vol_selector.PasteMask(0);
		vc.insert({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
	}
	if (request.HasValue(gstMaskMerge))
	{
		glbin_vol_selector.PasteMask(1);
		vc.insert({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
	}
	if (request.HasValue(gstMaskExclude))
	{
		glbin_vol_selector.PasteMask(2);
		vc.insert({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
	}
	if (request.HasValue(gstMaskIntersect))
	{
		glbin_vol_selector.PasteMask(3);
		vc.insert({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
	}
	if (request.HasValue(gstAlignPca))
	{
		auto vd = glbin_current.vol_data.lock();
		if (!vd)
			return;
		glbin_aligner.SetVolumeData(vd);
		glbin_aligner.SetView(glbin_current.render_view.lock());
		glbin_aligner.AlignPca(false);
		vc.insert({ gstNull });
	}
	if (request.HasValue(gstTimerSegment))
	{
		if (glbin_vol_selector.GetThUpdate())
			vc.insert(gstBrushThreshold);
		glbin_vol_selector.PopMask();
		glbin_vol_selector.Segment(true, false);
		vc.insert({ gstSelUndo, gstBrushCountAutoUpdate, gstColocalAutoUpdate });
	}

	NotifyViewUpdate(vc);
}

BrushToolDlg* BrushToolDlgAgent::GetDialog() const
{
	return static_cast<BrushToolDlg*>(GetWindow());
}

void BrushToolDlgAgent::SetBrushSclTranslate(double dval)
{
	//set translate
	glbin_vol_selector.SetBrushSclTranslate(dval / m_max_value);
}

void BrushToolDlgAgent::SetBrushGmFalloff(double dval)
{
	//set gm falloffd
	glbin_vol_selector.SetBrushGmFalloff(GM_2_ESTR(dval));
}

void BrushToolDlgAgent::SetW2d(double dval)
{
	//set 2d weight
	glbin_vol_selector.SetW2d(dval);
}

void BrushToolDlgAgent::SetEdgeDetect(bool bval)
{
	//set edge detect
	glbin_vol_selector.SetEdgeDetect(bval);
}

void BrushToolDlgAgent::SetHiddenRemoval(bool bval)
{
	//set hidden removal
	glbin_vol_selector.SetHiddenRemoval(bval);
}

void BrushToolDlgAgent::SetSelectGroup(bool bval)
{
	//set select group
	glbin_vol_selector.SetSelectGroup(bval);
}

void BrushToolDlgAgent::SetUpdateOrder(bool bval)
{
	glbin_vol_selector.SetUpdateOrder(bval);
}

void BrushToolDlgAgent::SetBrushSize1(double dval)
{
	//set size1
	glbin_vol_selector.SetBrushSize(dval, -1.0);
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::SetBrushSize2Enable(bool bval, double dval1, double dval2)
{
	glbin_vol_selector.SetUseBrushSize2(bval);
	glbin_vol_selector.SetBrushSize(dval1, dval2);
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::SetBrushSize2(double dval)
{
	//set size2
	glbin_vol_selector.SetBrushSize(-1.0, dval);
	NotifyViewUpdate({ gstNull });
}

void BrushToolDlgAgent::SetBrushIteration(int ival)
{
	glbin_vol_selector.SetBrushIteration(ival);
}

void BrushToolDlgAgent::SetBrushSizeData(bool bval)
{
	glbin_vol_selector.SetBrushSizeData(bval);
}

void BrushToolDlgAgent::SetAlignCenter(bool bval)
{
	glbin_aligner.SetAlignCenter(bval);
	NotifyViewUpdate({ gstAlignCenter });
}

void BrushToolDlgAgent::SetAlignAxis(int ival)
{
	glbin_aligner.SetAxisType(ival);
}
