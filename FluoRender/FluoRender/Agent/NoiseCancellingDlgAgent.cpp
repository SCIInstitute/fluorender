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

#include <NoiseCancellingDlgAgent.h>
#include <NoiseCancellingDlg.h>
#include <Global.h>
#include <Names.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <ComponentDefault.h>
#include <CompGenerator.h>
#include <CompAnalyzer.h>
#include <CompSelector.h>
#include <VolumeSelector.h>

NoiseCancellingDlgAgent::NoiseCancellingDlgAgent(
	NoiseCancellingDlg* dlg) :
	Agent(dlg)
{

}

void NoiseCancellingDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool update_all = request.values.empty();
	m_max_value = vd->GetMaxValue();

	double dval;
	int ival;
	bool bval;

	if (update_all || request.HasValue(gstNrThresh))
	{
		//threshold
		dval = glbin_comp_def.m_nr_thresh;
		dlg->UpdateNrThresh(dval, m_max_value);
	}

	if (update_all || request.HasValue(gstNrSize))
	{
		//voxel
		ival = glbin_comp_def.m_nr_size;
		auto res = vd->GetResolution();
		dlg->UpdateNrSize(ival, res.intx());
	}

	if (update_all || request.HasValue(gstUseSelection))
	{
		bval = glbin_comp_generator.GetUseSel();
		dlg->UpdateUseSelection(bval);
	}

	if (update_all || request.HasValue(gstNrPreview))
	{
		bval = glbin_comp_def.m_nr_preview;
		dlg->UpdateNrPreview(bval);
	}
}

void NoiseCancellingDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstNrThresh))
		SetMaskThreshold();
	if (request.HasValue(gstNrSize))
		SetNrSize();
	if (request.HasValue(gstUseSelection))
		SetUseSelection();
	if (request.HasValue(gstNrDoPreview))
		Preview();
	if (request.HasValue(gstNrDoErase))
		Erase();
	if (request.HasValue(gstNrPreview))
		SetPreview();
}

NoiseCancellingDlg* NoiseCancellingDlgAgent::GetDialog() const
{
	return static_cast<NoiseCancellingDlg*>(GetWindow());
}

void NoiseCancellingDlgAgent::Preview()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	bool bval = glbin_comp_generator.GetUseSel();
	glbin_comp_generator.SetThresh(glbin_comp_def.m_nr_thresh);
	glbin_comp_generator.SetVolumeData(vd);
	glbin_comp_generator.Compute();

	bool use_min = glbin_comp_analyzer.GetUseMin();
	bool use_max = glbin_comp_analyzer.GetUseMax();
	int min_num = glbin_comp_analyzer.GetMinNum();
	int max_num = glbin_comp_analyzer.GetMaxNum();

	glbin_comp_analyzer.SetUseMin(false);
	glbin_comp_analyzer.SetUseMax(true);
	glbin_comp_analyzer.SetMinNum(0);
	glbin_comp_analyzer.SetMaxNum(glbin_comp_def.m_nr_size);
	glbin_comp_analyzer.SetVolume(vd);
	glbin_comp_analyzer.Analyze();

	//cell size filter
	glbin_comp_selector.SetUseMin(false);
	glbin_comp_selector.SetUseMax(true);
	glbin_comp_selector.SetMinNum(0);
	glbin_comp_selector.SetMaxNum(glbin_comp_def.m_nr_size);
	glbin_comp_selector.CompFull();

	glbin_comp_def.m_nr_preview = true;

	Enhance();

	//restore settings
	glbin_comp_analyzer.SetUseMin(use_min);
	glbin_comp_analyzer.SetUseMax(use_max);
	glbin_comp_analyzer.SetMinNum(min_num);
	glbin_comp_analyzer.SetMaxNum(max_num);
	glbin_comp_selector.SetUseMin(use_min);
	glbin_comp_selector.SetUseMax(use_max);
	glbin_comp_selector.SetMinNum(min_num);
	glbin_comp_selector.SetMaxNum(max_num);

	NotifyViewUpdate({ gstNull });
}

void NoiseCancellingDlgAgent::Enhance()
{
	auto vd = glbin_current.vol_data.lock();
	if (!vd)
		return;

	if (glbin_comp_def.m_nr_preview)
	{
		fluo::Color mask_color = vd->GetMaskColor();
		double hdr_r = 0.0;
		double hdr_g = 0.0;
		double hdr_b = 0.0;
		if (mask_color.r() > 0.0)
			hdr_r = 0.4;
		if (mask_color.g() > 0.0)
			hdr_g = 0.4;
		if (mask_color.b() > 0.0)
			hdr_b = 0.4;
		fluo::Color hdr_color(hdr_r, hdr_g, hdr_b);
		glbin_comp_def.m_nr_hdr_r = vd->GetHdr().r();
		glbin_comp_def.m_nr_hdr_g = vd->GetHdr().g();
		glbin_comp_def.m_nr_hdr_b = vd->GetHdr().b();
		vd->SetHdr(hdr_color);
	}
	else if (!glbin_comp_def.m_nr_preview)
	{
		fluo::Color c(
			glbin_comp_def.m_nr_hdr_r,
			glbin_comp_def.m_nr_hdr_g,
			glbin_comp_def.m_nr_hdr_b);
		vd->SetHdr(c);
	}
	NotifyViewUpdate({ gstNull });
}

void NoiseCancellingDlgAgent::SetMaskThreshold()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	double val = dlg->GetThreshold();
	glbin_comp_def.m_nr_thresh = val / m_max_value;
	//change mask threshold
	auto vd = glbin_current.vol_data.lock();
	if (vd)
		vd->SetMaskThreshold(glbin_comp_def.m_nr_thresh);
	NotifyViewUpdate({ gstNull });
}

void NoiseCancellingDlgAgent::SetNrSize()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	glbin_comp_def.m_nr_size = dlg->GetNrSize();
}

void NoiseCancellingDlgAgent::SetUseSelection()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	glbin_comp_generator.SetUseSel(dlg->GetUseSelection());
	NotifyViewUpdate({ gstUseSelection });
}

void NoiseCancellingDlgAgent::Erase()
{
	glbin_vol_selector.Erase();
	NotifyViewUpdate({ gstNull });
}

void NoiseCancellingDlgAgent::SetPreview()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	glbin_comp_def.m_nr_preview = dlg->GetPreview();
	NotifyViewUpdate({ gstNull });
}
