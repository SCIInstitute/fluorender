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

#include <NoiseReduceAgent.hpp>
//#include <NoiseReduceDlg.h>
#include <AgentFactory.hpp>
#include <BrushToolAgent.hpp>
#include <VolumeData.hpp>
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <CompGenerator.h>

using namespace fluo;

NoiseReduceAgent::NoiseReduceAgent(NoiseReduceDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void NoiseReduceAgent::setupInputs()
{

}

void NoiseReduceAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* NoiseReduceAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void NoiseReduceAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();

/*	if (update_all || FOUND_VALUE(gstNonObjectValues))
	{
		Renderview* view = getObject();
		if (!view) return;
		VolumeData* sel_vol = view->GetCurrentVolume();
		if (!sel_vol) return;

		double max_int;
		sel_vol->getValue(gstMaxInt, max_int);
		//threshold
		dlg_.m_threshold_sldr->SetRange(0, int(max_int*10.0));
		//voxel
		long nx;
		sel_vol->getValue(gstResX, nx);
		dlg_.m_voxel_sldr->SetRange(1, nx);
	}

	if (update_all || FOUND_VALUE(gstThreshold))
	{
		double thresh;
		getValue(gstThreshold, thresh);
		dlg_.m_threshold_sldr->SetValue(int(thresh*10.0 + 0.5));
		dlg_.m_threshold_text->ChangeValue(wxString::Format("%.1f", thresh));
	}
	if (update_all || FOUND_VALUE(gstCompSizeLimit))
	{
		long size;
		getValue(gstCompSizeLimit, size);
		dlg_.m_voxel_sldr->SetValue(size);
		dlg_.m_voxel_text->ChangeValue(wxString::Format("%d", size));
	}
*/}

void NoiseReduceAgent::Preview()
{
	Renderview* view = getObject();
	if (!view) return;
	VolumeData* vd = view->GetCurrentVolume();
	if (!vd) return;

	bool select; long size; double thresh;
	getValue(gstUseSelection, select);
	getValue(gstCompSizeLimit, size);
	getValue(gstThreshold, thresh);

	flrd::ComponentGenerator cg(vd);
	cg.SetUseMask(select);
	vd->AddEmptyMask(cg.GetUseMask() ? 1 : 2, true);
	vd->AddEmptyLabel(0, !cg.GetUseMask());
	cg.ShuffleID();
	double scale;
	vd->getValue(gstIntScale, scale);
	cg.Grow(false, -1, thresh, 0.0, scale);

	flrd::ComponentAnalyzer* ca = view->GetCompAnalyzer();
	ca->SetVolume(vd);
	ca->Analyze(select, true, false);

	flrd::ComponentSelector comp_selector(vd);
	//cell size filter
	comp_selector.SetMinNum(false, 0);
	comp_selector.SetMaxNum(true, size);
	comp_selector.SetAnalyzer(ca);
	comp_selector.CompFull();

	//m_view->Update(39);
	bool bval;
	getValue(gstEnhance, bval);
	if (bval) Enhance();
}

void NoiseReduceAgent::Enhance()
{
	bool enhance;
	getValue(gstEnhance, enhance);

	VolumeData* sel_vol = getObject()->GetCurrentVolume();
	double hdr_r = 0.0;
	double hdr_g = 0.0;
	double hdr_b = 0.0;
	if (enhance && sel_vol)
	{
		Color mask_color;
		sel_vol->getValue(gstSecColor, mask_color);
		if (mask_color.r() > 0.0)
			hdr_r = 0.4;
		if (mask_color.g() > 0.0)
			hdr_g = 0.4;
		if (mask_color.b() > 0.0)
			hdr_b = 0.4;
		Color hdr_color(hdr_r, hdr_g, hdr_b);
		sel_vol->getValue(gstEqualizeR, hdr_r);
		sel_vol->getValue(gstEqualizeG, hdr_g);
		sel_vol->getValue(gstEqualizeB, hdr_b);
		setValue(gstEqualizeSavedR, hdr_r);
		setValue(gstEqualizeSavedG, hdr_g);
		setValue(gstEqualizeSavedB, hdr_b);
		sel_vol->setValue(gstEqualizeR, hdr_color.r());
		sel_vol->setValue(gstEqualizeG, hdr_color.g());
		sel_vol->setValue(gstEqualizeB, hdr_color.b());
	}
	else if (!enhance && sel_vol)
	{
		getValue(gstEqualizeSavedR, hdr_r);
		getValue(gstEqualizeSavedG, hdr_g);
		getValue(gstEqualizeSavedB, hdr_b);
		sel_vol->setValue(gstEqualizeR, hdr_r);
		sel_vol->setValue(gstEqualizeG, hdr_g);
		sel_vol->setValue(gstEqualizeB, hdr_b);
	}
	//if (m_frame)
	//	m_frame->RefreshVRenderViews();
}

void NoiseReduceAgent::BrushErase()
{
	glbin_agtf->getBrushToolAgent()->BrushErase();
}

void NoiseReduceAgent::OnThreshold(Event& event)
{
	double dval;
	getValue(gstThreshold, dval);
	//dlg_.m_threshold_sldr->SetValue(int(dval*10.0 + 0.5));
	//dlg_.m_threshold_text->ChangeValue(wxString::Format("%.1f", dval));

	//change mask threshold
	VolumeData* vd = getObject()->GetCurrentVolume();
	if (!vd) return;
	double max_int;
	vd->getValue(gstMaxInt, max_int);
	vd->updateValue(gstMaskThresh, max_int*dval);
	//m_frame->RefreshVRenderViews();
}

void NoiseReduceAgent::OnCompSizeLimit(Event& event)
{
	//long lval;
	//getValue(gstCompSizeLimit, lval);
	//dlg_.m_voxel_sldr->SetValue(lval);
}

void NoiseReduceAgent::OnEnhance(Event& event)
{
	Enhance();
}