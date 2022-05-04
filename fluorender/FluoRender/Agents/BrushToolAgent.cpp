/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <BrushToolAgent.hpp>
#include <BrushToolDlg.h>
#include <VolumeSelector.h>
#include <VolumeCalculator.h>
#include <RulerAlign.h>
#include <Global.hpp>
#include <Root.hpp>
#include <Distance/Cov.h>
#include <FLIVR/Texture.h>

using namespace fluo;

BrushToolAgent::BrushToolAgent(BrushToolDlg &dlg, TreePanel &panel) :
	InterfaceAgent(),
	dlg_(dlg),
	tree_panel_(panel)
{
}

void BrushToolAgent::setupInputs()
{
	
}

void BrushToolAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* BrushToolAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void BrushToolAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();

	if (update_all || FOUND_VALUE(gstNonObjectValues))
	{
		double dval = 0.0;
		long lval = 0;
		bool bval = false;

		flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
		if (!selector) return;

		VolumeData* vd = getObject()->GetCurrentVolume();
		if (!vd) return;
		//if (m_frame)
		//{
		//	vd = m_frame->GetCurSelVol();
		//	m_frame->GetNoiseCancellingDlg()->GetSettings(m_view);
		//	m_frame->GetCountingDlg()->GetSettings(m_view);
		//	//vr_frame->GetColocalizationDlg()->GetSettings(m_view);
		//}

		//threshold range
		double max_int;
		vd->getValue(gstMaxInt, max_int);
		//falloff
		dlg_.m_brush_scl_translate_sldr->SetRange(0, int(max_int*10.0 + 0.5));
		//m_brush_scl_translate_text->SetValue(wxString::Format("%.1f", m_dft_scl_translate*max_int));
		//selection strength
		dval = selector->GetBrushSclTranslate();
		//m_dft_scl_translate = dval;
		dlg_.m_brush_scl_translate_sldr->SetValue(int(dval*max_int*10.0 + 0.5));
		dlg_.m_brush_scl_translate_text->ChangeValue(wxString::Format("%.1f", dval*max_int));
		//gm falloff
		dval = selector->GetBrushGmFalloff();
		//m_dft_gm_falloff = dval;
		dlg_.m_brush_gm_falloff_sldr->SetValue(int(GM_2_ESTR(dval)*1000.0 + 0.5));
		dlg_.m_brush_gm_falloff_text->ChangeValue(wxString::Format("%.3f", GM_2_ESTR(dval)));
		//2d influence
		dval = selector->GetW2d();
		dlg_.m_brush_2dinfl_sldr->SetValue(int(dval*100.0 + 0.5));
		dlg_.m_brush_2dinfl_text->ChangeValue(wxString::Format("%.2f", dval));
		//edge detect
		bval = selector->GetEdgeDetect();
		dlg_.m_edge_detect_chk->SetValue(bval);
		if (bval)
		{
			dlg_.m_brush_gm_falloff_sldr->Enable();
			dlg_.m_brush_gm_falloff_text->Enable();
		}
		else
		{
			dlg_.m_brush_gm_falloff_sldr->Disable();
			dlg_.m_brush_gm_falloff_text->Disable();
		}
		//hidden removal
		bval = selector->GetHiddenRemoval();
		dlg_.m_hidden_removal_chk->SetValue(bval);
		//select group
		bval = selector->GetSelectGroup();
		dlg_.m_select_group_chk->SetValue(bval);
		//estimate threshold
		bval = selector->GetEstimateThreshold();
		dlg_.m_estimate_thresh_chk->SetValue(bval);
		//brick acuracy
		bval = selector->GetUpdateOrder();
		dlg_.m_accurate_bricks_chk->SetValue(bval);

		//size1
		dval = selector->GetBrushSize1();
		dlg_.m_brush_size1_sldr->SetValue(int(dval));
		dlg_.m_brush_size1_text->ChangeValue(wxString::Format("%.0f", dval));
		//size2
		dlg_.m_brush_size2_chk->SetValue(selector->GetUseBrushSize2());
		if (selector->GetUseBrushSize2())
		{
			dlg_.m_brush_size2_sldr->Enable();
			dlg_.m_brush_size2_text->Enable();
		}
		else
		{
			dlg_.m_brush_size2_sldr->Disable();
			dlg_.m_brush_size2_text->Disable();
		}
		dval = selector->GetBrushSize2();
		dlg_.m_brush_size2_sldr->SetValue(int(dval));
		dlg_.m_brush_size2_text->ChangeValue(wxString::Format("%.0f", dval));

		//iteration number
		lval = selector->GetBrushIteration();
		if (lval <= flrd::VolumeSelector::BRUSH_TOOL_ITER_WEAK)
		{
			dlg_.m_brush_iterw_rb->SetValue(true);
			dlg_.m_brush_iters_rb->SetValue(false);
			dlg_.m_brush_iterss_rb->SetValue(false);
		}
		else if (lval <= flrd::VolumeSelector::BRUSH_TOOL_ITER_NORMAL)
		{
			dlg_.m_brush_iterw_rb->SetValue(false);
			dlg_.m_brush_iters_rb->SetValue(true);
			dlg_.m_brush_iterss_rb->SetValue(false);
		}
		else
		{
			dlg_.m_brush_iterw_rb->SetValue(false);
			dlg_.m_brush_iters_rb->SetValue(false);
			dlg_.m_brush_iterss_rb->SetValue(true);
		}

		//brush size relation
		bval = selector->GetBrushSizeData();
		if (bval)
		{
			dlg_.m_brush_size_data_rb->SetValue(true);
			dlg_.m_brush_size_screen_rb->SetValue(false);
		}
		else
		{
			dlg_.m_brush_size_data_rb->SetValue(false);
			dlg_.m_brush_size_screen_rb->SetValue(true);
		}

		//output
		dlg_.m_history_chk->SetValue(dlg_.m_hold_history);

		UpdateUndoRedo();
		UpdateMaskTb();
	}
}

void BrushToolAgent::UpdateUndoRedo()
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	if (vd && vd->GetTexture())
	{
		dlg_.m_toolbar->EnableTool(BrushToolDlg::ID_BrushUndo,
			vd->GetTexture()->get_undo());
		dlg_.m_toolbar->EnableTool(BrushToolDlg::ID_BrushRedo,
			vd->GetTexture()->get_redo());
	}
}

void BrushToolAgent::UpdateMaskTb()
{
	Referenced* ref;
	glbin_root->getRefValue(gstSourceVolume, &ref);
	bool bval = ref != nullptr;
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskPaste, bval);
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskMerge, bval);
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskExclude, bval);
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskIntersect, bval);
}

//brush ops
void BrushToolAgent::BrushClear()
{
	ValueCollection names{ gstInterMode, gstPaintMode };
	saveValues(names);
	changeValue(gstPaintMode, long(6));
	getObject()->Segment();
	drawValues(names);
}

void BrushToolAgent::BrushErase()
{
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	VolumeData* vd = getObject()->GetCurrentVolume();
	cal->SetVolumeA(vd);
	cal->CalculateGroup(6);
}

void BrushToolAgent::BrushCreate()
{
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	VolumeData* vd = getObject()->GetCurrentVolume();
	cal->SetVolumeA(vd);
	cal->CalculateGroup(5);
}

void BrushToolAgent::MaskCopy()
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	glbin_root->setRefValue(gstSourceVolume, vd);
	glbin_root->setValue(gstSourceMode, long(1));
	UpdateMaskTb();
}

void BrushToolAgent::MaskCopyData()
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	glbin_root->setRefValue(gstSourceVolume, vd);
	glbin_root->setValue(gstSourceMode, long(0));
	UpdateMaskTb();
}

void BrushToolAgent::MaskPaste(int op)
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	Referenced* ref;
	glbin_root->getRefValue(gstSourceVolume, &ref);
	VolumeData* vd_src = dynamic_cast<VolumeData*>(ref);
	if (!vd_src || vd == vd_src)
		return;
	long lval;
	glbin_root->getValue(gstSourceMode, lval);
	//undo/redo
	if (flvr::Texture::mask_undo_num_ > 0 &&
		vd->GetTexture())
		vd->GetTexture()->push_mask();
	if (lval == 0)
	{
		Nrrd* data = vd_src->GetData(false);
		long bits;
		vd_src->getValue(gstBits, bits);
		if (bits == 16)
		{
			double scale;
			vd_src->getValue(gstIntScale, scale);
			vd->AddMask16(data, op, scale);
		}
		else
			vd->AddMask(data, op);
	}
	else
		vd->AddMask(vd_src->GetMask(false), op);
	UpdateUndoRedo();
}

void BrushToolAgent::SetBrushSclTranslate(double v)
{
	//set translate
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (!selector) return;

	selector->SetBrushSclTranslate(v);
	if (selector->GetThUpdate())
	{
		selector->PopMask();
		getObject()->Segment();
		//m_view->Update(39);
	}
}

void BrushToolAgent::SetBrushGmFalloff(double v)
{
	//set gm falloff
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (!selector) return;

	selector->SetBrushGmFalloff(v);
	if (selector->GetThUpdate())
	{
		selector->PopMask();
		getObject()->Segment();
		//m_view->Update(39);
	}
}

void BrushToolAgent::SetW2d(double v)
{
	//set 2d weight
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (!selector) return;

	selector->SetW2d(v);
	if (selector->GetThUpdate())
	{
		selector->PopMask();
		getObject()->Segment();
		//m_view->Update(39);
	}
}

void BrushToolAgent::SetEdgeDetect(bool v)
{
	//set edge detect
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (!selector) return;

	selector->SetEdgeDetect(v);
	if (selector->GetThUpdate())
	{
		selector->PopMask();
		getObject()->Segment();
		//m_view->Update(39);
	}
}

void BrushToolAgent::SetHiddenRemoval(bool v)
{
	//set hidden removal
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetHiddenRemoval(v);
}

void BrushToolAgent::SetSelectGroup(bool v)
{
	//set select group
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (!selector) return;

	selector->SetSelectGroup(v);
	if (selector->GetThUpdate())
	{
		selector->PopMask();
		getObject()->Segment();
		//m_view->Update(39);
	}
}

void BrushToolAgent::SetEstimateThreshold(bool v)
{
	//set estimate threshold
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetEstimateThreshold(v);
}

void BrushToolAgent::SetUpdateOrder(bool v)
{
	//set update order
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetUpdateOrder(v);
}

void BrushToolAgent::SetBrushSize(double v1, double v2)
{
	//set size
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetBrushSize(v1, v2);
	//	long lval;
	//	m_view->getValue(gstInterMode, lval);
	//	if (lval == 2)
	//		m_view->Update(39);
}

void BrushToolAgent::SetUseBrushSize2(bool v)
{
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetUseBrushSize2(v);
}

void BrushToolAgent::SetBrushIteration(int v)
{
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetBrushIteration(v);
}

void BrushToolAgent::SetBrushSizeData(bool v)
{
	flrd::VolumeSelector* selector = getObject()->GetVolumeSelector();
	if (selector) selector->SetBrushSizeData(v);
}

void BrushToolAgent::AlignPca(int type, bool ac)
{
	VolumeData* vd = getObject()->GetCurrentVolume();
	flrd::RulerAlign* aligner = getObject()->GetRulerAlign();
	if (vd && vd->GetTexture())
	{
		flrd::Cov cover(vd);
		if (cover.Compute(0))
		{
			std::vector<double> cov = cover.GetCov();
			fluo::Point center = cover.GetCenter();
			aligner->SetCovMat(cov);
			aligner->AlignPca(type, false);
			if (ac)
			{
				double tx, ty, tz;
				getValue(gstObjCtrX, tx);
				getValue(gstObjCtrY, ty);
				getValue(gstObjCtrZ, tz);
				tx = tx - center.x();
				ty = center.y() - ty;
				tz = center.z() - tz;
				updateValue(gstObjTransX, tx);
				updateValue(gstObjTransY, ty);
				updateValue(gstObjTransZ, tz);
			}
		}
	}
}

//update
void BrushToolAgent::Update(int mode)
{
	switch (mode)
	{
	case 0:
	default:
		UpdateSize();
		break;
	case 1:
		UpdateSpeed();
	}
}

void BrushToolAgent::UpdateSize()
{
	//GridData data;
	//fluo::VolumeData* sel_vol = 0;
	//if (!m_frame)
	//	return;
	//sel_vol = m_frame->GetCurSelVol();
	//if (!sel_vol)
	//	return;

	//flrd::CountVoxels counter(sel_vol);
	//counter.SetUseMask(true);
	//counter.Count();
	//data.voxel_sum = counter.GetSum();
	//double scale;
	//sel_vol->getValue(gstIntScale, scale);
	//data.voxel_wsum = counter.GetWeightedSum() * scale;
	//if (data.voxel_sum)
	//{
	//	data.avg_int = data.voxel_wsum / data.voxel_sum;
	//	long bits;
	//	sel_vol->getValue(gstBits, bits);
	//	if (bits == 8)
	//		data.avg_int *= 255.0;
	//	else if (bits == 16)
	//	{
	//		double maxint;
	//		sel_vol->getValue(gstMaxInt, maxint);
	//		data.avg_int *= maxint;
	//	}
	//}
	//double spcx, spcy, spcz;
	//sel_vol->getValue(gstSpcX, spcx);
	//sel_vol->getValue(gstSpcY, spcy);
	//sel_vol->getValue(gstSpcZ, spcz);
	//double vvol = spcx * spcy * spcz;
	//vvol = vvol == 0.0 ? 1.0 : vvol;
	//data.size = data.voxel_sum * vvol;
	//data.wsize = data.voxel_wsum * vvol;
	//wxString unit;
	//if (m_view)
	//{
	//	long lval;
	//	m_view->getValue(gstScaleBarUnit, lval);
	//	switch (lval)
	//	{
	//	case 0:
	//		unit = L"nm\u00B3";
	//		break;
	//	case 1:
	//	default:
	//		unit = L"\u03BCm\u00B3";
	//		break;
	//	case 2:
	//		unit = L"mm\u00B3";
	//		break;
	//	}
	//}

	//SetOutput(data, unit);
}

void BrushToolAgent::UpdateSpeed()
{
	//if (!m_selector || !m_selector->m_test_speed)
	//	return;
	//GridData data;
	//data.size = m_selector->GetSpanSec();
	//data.wsize = data.size;
	//wxString unit = "Sec.";
	//SetOutput(data, unit);
}

void BrushToolAgent::OnInterModeChanged(Event& event)
{
	dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushAppend, false);
	dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushDiffuse, false);
	dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushDesel, false);
	dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushSolid, false);
	dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_Grow, false);
	long lval;
	getValue(gstInterMode, lval);
	if (lval != 2 &&
		lval != 7 &&
		lval != 10 &&
		lval != 12)
		return;
	if (lval == 10 ||
		lval == 12)
	{
		dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_Grow, true);
		return;
	}
	getValue(gstBrushState, lval);
	switch (lval)
	{
	case 2://append
		dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushAppend, true);
		break;
	case 3://erase
		dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushDesel, true);
		break;
	case 4://diffuse
		dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushDiffuse, true);
		break;
	case 8://solid
		dlg_.m_toolbar->ToggleTool(BrushToolDlg::ID_BrushSolid, true);
		break;
	}
}

void BrushToolAgent::OnSelUndo(Event& event)
{
	UpdateUndoRedo();
}

void BrushToolAgent::OnSelRedo(Event& event)
{
	UpdateUndoRedo();
}
