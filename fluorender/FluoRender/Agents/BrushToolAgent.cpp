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
#include <Global.hpp>
#include <Root.hpp>
#include <FLIVR/Texture.h>

using namespace fluo;

BrushToolAgent::BrushToolAgent(BrushToolDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
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

void BrushToolAgent::UpdateAllSettings()
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
	if (lval <= BRUSH_TOOL_ITER_WEAK)
	{
		dlg_.m_brush_iterw_rb->SetValue(true);
		dlg_.m_brush_iters_rb->SetValue(false);
		dlg_.m_brush_iterss_rb->SetValue(false);
	}
	else if (lval <= BRUSH_TOOL_ITER_NORMAL)
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
	glbin_root->getRvalu(gstSourceVolume, &ref);
	bool bval = ref != nullptr;
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskPaste, bval);
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskMerge, bval);
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskExclude, bval);
	dlg_.m_mask_tb->EnableTool(BrushToolDlg::ID_MaskIntersect, bval);
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
