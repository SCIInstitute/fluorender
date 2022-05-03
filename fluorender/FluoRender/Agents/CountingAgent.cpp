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

#include <CountingAgent.hpp>
#include <CountingDlg.h>
#include <Global.hpp>
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <SearchVisitor.hpp>
#include <CompGenerator.h>
#include <CompAnalyzer.h>

using namespace fluo;

CountingAgent::CountingAgent(CountingDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void CountingAgent::setObject(VolumeData* obj)
{
	InterfaceAgent::setObject(obj);
}

VolumeData* CountingAgent::getObject()
{
	return dynamic_cast<VolumeData*>(InterfaceAgent::getObject());
}

void CountingAgent::UpdateFui(const ValueCollection &names)
{
	bool bval;
	long lval;
	getValue(gstUseSelection, bval);
	dlg_.m_ca_select_only_chk->SetValue(bval);
	getValue(gstMinValue, lval);
	dlg_.m_ca_min_text->SetValue(wxString::Format("%d", lval));
	getValue(gstMaxValue, lval);
	dlg_.m_ca_max_text->SetValue(wxString::Format("%d", lval));
	getValue(gstUseMax, bval);
	dlg_.m_ca_ignore_max_chk->SetValue(!bval);
	dlg_.m_ca_max_text->Enable(!bval);
}

void CountingAgent::Analyze()
{
	fluo::VolumeData* vd = getObject();
	if (!vd) return;

	bool select;
	getValue(gstUseSelection, select);

	flrd::ComponentGenerator cg(vd);
	cg.SetUseMask(select);
	vd->AddEmptyMask(cg.GetUseMask() ? 2 : 1, true);
	vd->AddEmptyLabel(0, !cg.GetUseMask());
	cg.ShuffleID();
	double scale;
	vd->getValue(gstIntScale, scale);
	cg.Grow(false, -1, 0.0, 0.0, scale);

	flrd::ComponentAnalyzer ca(vd);
	ca.Analyze(select, true, false);
	//m_view->Update(39);

	flrd::CelpList *list = ca.GetCelpList();
	if (!list || list->empty())
		return;

	long min_voxels, max_voxels;
	getValue(gstMinValue, min_voxels);
	getValue(gstMaxValue, max_voxels);
	bool use_max;
	getValue(gstUseMax, use_max);

	int count = 0;
	unsigned int vox = 0;
	for (auto it = list->begin();
		it != list->end(); ++it)
	{
		unsigned int sumi = it->second->GetSizeUi();
		if (sumi > min_voxels &&
			(!use_max ||
			(use_max && sumi < max_voxels)))
		{
			++count;
			vox += sumi;
		}
	}

	if (count > 0)
	{
		//get view of group
		SearchVisitor visitor;
		visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
		visitor.matchClassName("Renderview");
		vd->accept(visitor);
		Renderview* view = visitor.getRenderview();

		dlg_.m_ca_comps_text->SetValue(wxString::Format("%d", count));
		dlg_.m_ca_volume_text->SetValue(wxString::Format("%d", vox));

		double spcx, spcy, spcz;
		vd->getValue(gstSpcX, spcx);
		vd->getValue(gstSpcY, spcy);
		vd->getValue(gstSpcZ, spcz);
		double vol_unit = vox * spcx*spcy*spcz;
		wxString vol_unit_text;
		vol_unit_text = wxString::Format("%.0f", vol_unit);
		vol_unit_text += " ";
		long sb_unit = 1;
		if (view) view->getValue(gstScaleBarUnit, sb_unit);
		switch (sb_unit)
		{
		case 0:
			vol_unit_text += L"nm\u00B3";
			break;
		case 1:
		default:
			vol_unit_text += L"\u03BCm\u00B3";
			break;
		case 2:
			vol_unit_text += L"mm\u00B3";
			break;
		}

		dlg_.m_ca_vol_unit_text->SetValue(vol_unit_text);
	}
}

void CountingAgent::OnUseMax(Event& event)
{
	bool bval;
	getValue(gstUseMax, bval);
	dlg_.m_ca_max_text->Enable(!bval);
}